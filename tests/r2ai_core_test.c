#include <r_core.h>
#include <r_config.h>
#include <r_cmd.h>    // For RCmd
#include <r_util.h>   // For r_file_rm, r_file_slurp, r_file_mkstemp
#include "r2ai.h"     // SUT and for R2AIArgs
#include "messages.h" // For R2AI_Messages, r2ai_msgs_free, etc.
#include <assert.h>
#include <string.h>
#include <stdio.h>    // For printf (test names)
#include <stdlib.h>   // For free
#include <sys/stat.h> // For mkdir
#include <unistd.h>   // For rmdir, unlink

// Helper to print test names
static void print_test_name(const char *name) {
	printf("[TEST] %s\n", name);
}

// Helper to create and initialize a minimal RCore for tests
static RCore* create_test_core() {
	RCore *core = r_core_new();
	if (core) {
		r_config_init(core->config);
		// r2ai_init might be called if tests need full r2ai setup
		// For VDB tests, r2ai_init will be called to setup core->rcmd and other necessities.
	}
	return core;
}

// Helper to destroy RCore
static void destroy_test_core(RCore *core) {
	if (core) {
		r_core_free(core);
	}
}

// --- Conversation Management Tests ---

static void test_conversation_init_free_get() {
	print_test_name("test_conversation_init_free_get");
	r2ai_conversation_init(NULL); 
	R2AI_Messages *conv1 = r2ai_conversation_get();
	assert(conv1 != NULL);
	assert(conv1->n_messages == 0); 

	r2ai_conversation_free();
	assert(r2ai_conversation_get() == NULL);

	r2ai_conversation_init("New System Prompt");
	R2AI_Messages *conv2 = r2ai_conversation_get();
	assert(conv2 != NULL);
	assert(conv2->n_messages == 0); 
	assert(conv2->system_prompt != NULL && strcmp(conv2->system_prompt, "New System Prompt") == 0);

	r2ai_conversation_free(); 
}

static void test_create_conversation() {
	print_test_name("test_create_conversation");

	R2AI_Messages *msgs_null = create_conversation(NULL);
	assert(msgs_null != NULL);
	assert(msgs_null->n_messages == 0);
	assert(msgs_null->messages == NULL); 
	assert(msgs_null->system_prompt == NULL);
	r2ai_msgs_free(msgs_null);

	const char *user_msg_text = "Hello, R2AI!";
	R2AI_Messages *msgs_non_empty = create_conversation(user_msg_text);
	assert(msgs_non_empty != NULL);
	assert(msgs_non_empty->n_messages == 1);
	assert(msgs_non_empty->messages != NULL);
	assert(strcmp(msgs_non_empty->messages[0].json, user_msg_text) == 0);
	assert(strcmp(msgs_non_empty->messages[0].role, "user") == 0);
	assert(msgs_non_empty->system_prompt == NULL);
	r2ai_msgs_free(msgs_non_empty);
}

static void test_r2ai_delete_last_messages_on_global_conv() {
	print_test_name("test_r2ai_delete_last_messages_on_global_conv");

	r2ai_conversation_init(NULL);
	R2AI_Messages *conv = r2ai_conversation_get();
	assert(conv != NULL);

	r2ai_messasge_add(conv, "user", "Message 1", 0);
	r2ai_messasge_add(conv, "assistant", "Message 2", 0);
	r2ai_messasge_add(conv, "user", "Message 3", 0);
	assert(conv->n_messages == 3);

	r2ai_delete_last_messages(conv, 1);
	assert(conv->n_messages == 2);
	assert(strcmp(conv->messages[conv->n_messages - 1].json, "Message 2") == 0); 

	r2ai_delete_last_messages(conv, 1);
	assert(conv->n_messages == 1);
	assert(strcmp(conv->messages[conv->n_messages - 1].json, "Message 1") == 0);

	r2ai_messasge_add(conv, "assistant", "Message A", 0);
	r2ai_messasge_add(conv, "user", "Message B", 0);
	assert(conv->n_messages == 3); 

	r2ai_delete_last_messages(conv, 2);
	assert(conv->n_messages == 1);
	assert(strcmp(conv->messages[0].json, "Message 1") == 0);

	r2ai_delete_last_messages(conv, 5);
	assert(conv->n_messages == 0);

	r2ai_delete_last_messages(conv, 1);
	assert(conv->n_messages == 0);

	r2ai_conversation_free();
}


// --- r2ai_llmcall() and r2ai() Tests ---
char *mock_r2ai_provider_testapi(R2AIArgs *args) {
	if (args->api_key == NULL || strcmp(args->api_key, "") == 0) {
		if (args->error) *args->error = strdup("API key missing for testapi");
		return NULL;
	}
	if (args->messages == NULL || args->messages->n_messages == 0) {
		if (args->error) *args->error = strdup("No messages provided for testapi");
		return NULL;
	}
	if (args->system_prompt && strcmp(args->system_prompt, "Test System Prompt from Config") == 0) {
		return strdup("Mocked response including system prompt: Test System Prompt from Config");
	}
	if (!(args->model && strcmp(args->model, "testmodel_from_config")==0)) {
		if (args->error) *args->error = strdup ("Model not picked from config");
		return NULL;
	}
	if (args->max_tokens != 1234) {
		if (args->error) *args->error = strdup ("Max tokens not picked from config");
		return NULL;
	}
	if (fabs(args->temperature - 0.5) > 0.01) {
		if (args->error) *args->error = strdup ("Temperature not picked from config");
		return NULL;
	}
	return strdup("Mocked response from testapi");
}


static void test_r2ai_function_inputs_and_config() {
	print_test_name("test_r2ai_function_inputs_and_config");
	RCore *core = create_test_core();
	assert(core != NULL);

	r_config_set(core->config, "r2ai.api", "testapi"); 
	r_config_set(core->config, "r2ai.model", "testmodel_from_config");
	r_config_set(core->config, "r2ai.system", "Test System Prompt from Config");
	r_config_set(core->config, "r2ai.maxtokens", "1234");
	r_config_set(core->config, "r2ai.temperature", "0.5");
    r_config_set(core->config, "r2ai.apikey", "testkey123"); 

	R2AIPlugin r2ai_plugin_testapi = {
		.name = "testapi", .description = "Mock test API", .call = mock_r2ai_provider_testapi,
	};
	r2ai_plugin_add(&r2ai_plugin_testapi);

	R2AIArgs args_r2ai = {0};
	args_r2ai.core = core;

	char *res_r2ai = r2ai(&args_r2ai);
	assert(res_r2ai == NULL);
	assert(args_r2ai.error != NULL); 
	free(args_r2ai.error); args_r2ai.error = NULL;

	args_r2ai.input = "This is a test input.";
	r_config_set(core->config, "r2ai.apikey", ""); 
	
	res_r2ai = r2ai(&args_r2ai);
	assert(res_r2ai == NULL);
	assert(args_r2ai.error != NULL);
	assert(strstr(args_r2ai.error, "API key missing") != NULL);
	free(args_r2ai.error); args_r2ai.error = NULL;
	r_config_set(core->config, "r2ai.apikey", "testkey123"); 

	R2AIArgs args_llmcall = {0};
	args_llmcall.core = core;

	args_llmcall.provider = "testapi"; 
	args_llmcall.messages = create_conversation("Hello from llmcall test");
	char *old_apikey = r_config_get(core->config, "r2ai.apikey"); 
	r_config_set(core->config, "r2ai.apikey", ""); 

	char *res_llmcall = r2ai_llmcall(&args_llmcall);
	assert(res_llmcall == NULL);
	assert(args_llmcall.error != NULL);
	assert(strstr(args_llmcall.error, "API key missing") != NULL);
	free(args_llmcall.error); args_llmcall.error = NULL;
	r2ai_msgs_free(args_llmcall.messages); args_llmcall.messages = NULL;
	r_config_set(core->config, "r2ai.apikey", old_apikey); 

	args_llmcall.provider = "testapi";
	args_llmcall.api_key = "testkey123"; 
	args_llmcall.messages = create_conversation(NULL); 
	assert(args_llmcall.messages->n_messages == 0);

	res_llmcall = r2ai_llmcall(&args_llmcall);
	assert(res_llmcall == NULL);
	assert(args_llmcall.error != NULL);
	assert(strstr(args_llmcall.error, "No messages provided") != NULL);
	free(args_llmcall.error); args_llmcall.error = NULL;
	r2ai_msgs_free(args_llmcall.messages); args_llmcall.messages = NULL;

	args_llmcall.provider = "testapi";
	args_llmcall.api_key = "testkey123";
	args_llmcall.messages = create_conversation("A message");
	args_llmcall.system_prompt = NULL; 

	res_llmcall = r2ai_llmcall(&args_llmcall);
	assert(res_llmcall != NULL);
	assert(strstr(res_llmcall, "Mocked response including system prompt: Test System Prompt from Config") != NULL);
	free(res_llmcall);
	r2ai_msgs_free(args_llmcall.messages); args_llmcall.messages = NULL;

	args_llmcall.provider = "testapi";
	args_llmcall.api_key = "testkey123";
	args_llmcall.messages = create_conversation("Another message for config checks");
	args_llmcall.system_prompt = "Generic prompt"; 
	
	res_llmcall = r2ai_llmcall(&args_llmcall);
	assert(res_llmcall != NULL);
	assert(strcmp(res_llmcall, "Mocked response from testapi") == 0); 
	assert(args_llmcall.error == NULL);
	free(res_llmcall);
	r2ai_msgs_free(args_llmcall.messages); args_llmcall.messages = NULL;

    // New: Test non-existent provider
    args_llmcall.provider = "provider_does_not_exist_abc123";
    args_llmcall.api_key = "anykey";
    args_llmcall.messages = create_conversation("Message for non-existent provider");
    args_llmcall.system_prompt = "System prompt";

    res_llmcall = r2ai_llmcall(&args_llmcall);
    assert(res_llmcall == NULL);
    assert(args_llmcall.error != NULL);
    // printf("Non-existent provider error: %s\n", args_llmcall.error);
    assert(strstr(args_llmcall.error, "Unsupported provider") != NULL || strstr(args_llmcall.error, "No such plugin") != NULL);
    free(args_llmcall.error); args_llmcall.error = NULL;
    r2ai_msgs_free(args_llmcall.messages); args_llmcall.messages = NULL;

    r2ai_plugin_del(&r2ai_plugin_testapi);
	destroy_test_core(core);
}

// --- Plugin Lifecycle Tests ---

static void test_r2ai_init_fini() {
	print_test_name("test_r2ai_init_fini");
	RCore *core = create_test_core();
	assert(core != NULL);
	core->rcmd = r_cmd_new (core); 
	assert(core->rcmd != NULL);

	int res_init = r2ai_init(core->rcmd, NULL); 
	assert(res_init); 

	assert(strcmp(r_config_get(core->config, "r2ai.api"), "openai") == 0); 
	assert(strcmp(r_config_get(core->config, "r2ai.model"), "gpt-4-turbo-preview") == 0); 
	assert(strlen(r_config_get(core->config, "r2ai.system")) > 0); 
	assert(strlen(r_config_get(core->config, "r2ai.prompt")) > 0); 

	R2AI_Messages *conv_after_init = r2ai_conversation_get();
	assert(conv_after_init != NULL);
	assert(strcmp(conv_after_init->system_prompt, r_config_get(core->config, "r2ai.system")) == 0);

	int res_fini = r2ai_fini(core->rcmd, NULL); 
	assert(res_fini); 

	assert(r_config_get(core->config, "r2ai.api") != NULL); 
	assert(r2ai_conversation_get() == NULL);

	destroy_test_core(core);
}

// --- VDB Wrapper Function Tests ---
static void test_r2ai_vdb_wrappers() {
    print_test_name("test_r2ai_vdb_wrappers");
    RCore *core = create_test_core();
    assert(core != NULL);
    core->rcmd = r_cmd_new(core); // Needed for r2ai_init
    assert(core->rcmd != NULL);

    // Setup: r2ai_init and config
    r2ai_init(core->rcmd, NULL); // This initializes the static db in r2ai.c
    r_config_set_b(core->config, "r2ai.data", true);

    // Use a temporary directory for r2ai.data.path
    char *tmp_data_path_template = "/tmp/r2ai_test_data_XXXXXX";
    char *tmp_data_path = r_file_mkdtemp(tmp_data_path_template);
    assert(tmp_data_path != NULL);
    r_config_set(core->config, "r2ai.data.path", tmp_data_path);
    // printf("Using temp data path: %s\n", tmp_data_path);


    // 1. Test r2ai_vdb_add(core, "text")
    char *add_res = r2ai_vdb_add(core, "sample text for r2ai vdb");
    // r2ai_vdb_add returns char*, but its content isn't strictly defined for success/failure yet.
    // We primarily check for no crash.
    // printf("r2ai_vdb_add result: %s\n", add_res ? add_res : "NULL");
    free(add_res); // It might return a status message or NULL

    // 2. Test r2ai_vdb_query(core, "text", k)
    char *query_res = r2ai_vdb_query(core, "sample text", 1);
    assert(query_res != NULL);
    // printf("r2ai_vdb_query result: %s\n", query_res);
    // Expect a JSON string. If db is small, it might be empty "[]" or contain the result.
    assert(strstr(query_res, "sample text for r2ai vdb") != NULL || strcmp(query_res, "[]") == 0);
    free(query_res);

    // Test query with k=0 or k<0 (should default to a sensible k, e.g., 3 as per r2ai.c)
    query_res = r2ai_vdb_query(core, "sample text", 0);
    assert(query_res != NULL);
    free(query_res);
    query_res = r2ai_vdb_query(core, "sample text", -1);
    assert(query_res != NULL);
    free(query_res);

    // 3. Test r2ai_vdb_add(core, NULL) - refresh from path
    const char *dummy_file_content = "text from dummy file for refresh";
    char dummy_file_path[256];
    snprintf(dummy_file_path, sizeof(dummy_file_path), "%s/test_data.txt", tmp_data_path);
    
    FILE *f = fopen(dummy_file_path, "w");
    assert(f != NULL);
    fprintf(f, "%s\n", dummy_file_content);
    fclose(f);

    add_res = r2ai_vdb_add(core, NULL); // Trigger refresh
    // printf("r2ai_vdb_add (refresh) result: %s\n", add_res ? add_res : "NULL");
    free(add_res);

    // Query for the content from the dummy file
    query_res = r2ai_vdb_query(core, "dummy file refresh", 1); // Query for part of the content
    assert(query_res != NULL);
    // printf("r2ai_vdb_query (after refresh) result: %s\n", query_res);
    assert(strstr(query_res, dummy_file_content) != NULL);
    free(query_res);

    // Clean up dummy file
    unlink(dummy_file_path);

    // 4. Test r2ai_vdb_delete(core, "id")
    // The "id" for r2ai_vdb_delete is the text content itself from the file.
    // Let's try to delete "text from dummy file for refresh" which was added from the file.
    // First, ensure it's there.
    query_res = r2ai_vdb_query(core, "dummy file refresh", 1);
    assert(query_res != NULL && strstr(query_res, dummy_file_content) != NULL);
    free(query_res);
    
    // Create the file again, as refresh might have cleared non-file entries or re-added.
    // The VDB in r2ai.c is cleared and re-populated entirely from files on refresh.
    // So, after the refresh, "sample text for r2ai vdb" (added directly) would be gone.
    // Let's re-add the dummy file and its content to test deletion from a file-backed entry.
    f = fopen(dummy_file_path, "w");
    assert(f != NULL);
    fprintf(f, "%s\n", dummy_file_content);
    fclose(f);
    add_res = r2ai_vdb_add(core, NULL); // Refresh again to load the file
    free(add_res);

    char *delete_res = r2ai_vdb_delete(core, dummy_file_content); // Delete by content
    // printf("r2ai_vdb_delete result: %s\n", delete_res ? delete_res : "NULL");
    // The function returns "1" on success (line removed), "0" if not found.
    assert(delete_res != NULL && strcmp(delete_res, "1") == 0); // Expect success
    free(delete_res);

    // Verify it's deleted by querying (should not be found)
    query_res = r2ai_vdb_query(core, "dummy file refresh", 1);
    assert(query_res != NULL);
    // printf("r2ai_vdb_query (after delete) result: %s\n", query_res);
    assert(strstr(query_res, dummy_file_content) == NULL); // Should not be found
    free(query_res);

    // Test deleting non-existent entry
    delete_res = r2ai_vdb_delete(core, "this text does not exist in any file");
    assert(delete_res != NULL && strcmp(delete_res, "0") == 0); // Expect "0" (not found)
    free(delete_res);

    // Cleanup
    r_file_rm_rf(tmp_data_path); // Remove the temporary directory and its contents
    free(tmp_data_path);
    r2ai_fini(core->rcmd, NULL); // Frees the static db
    destroy_test_core(core);
}


int main(int argc, char **argv) {
	test_conversation_init_free_get();
	test_create_conversation();
	test_r2ai_delete_last_messages_on_global_conv();

	test_r2ai_function_inputs_and_config(); // This now includes non-existent provider test

	test_r2ai_init_fini();
    test_r2ai_vdb_wrappers(); // Added new test suite

	printf("[PASS] All r2ai_core tests completed.\n");
	return 0;
}
