#include <r_util.h>
#include <r_util/r_json.h>
#include "messages.h" // SUT
#include "r2ai.h"     // For R2AI_Message and other related structs
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper to print test names
static void print_test_name(const char *name) {
	printf("[TEST] %s\n", name);
}

// --- Test r2ai_message_free ---
static void test_r2ai_message_free_extended() {
	print_test_name("test_r2ai_message_free_extended");
	R2AI_Message msg;

	// 1. Test freeing a message with NULL role, NULL content, no tool calls, no content blocks
	memset(&msg, 0, sizeof(R2AI_Message)); // Initialize all to NULL/0
	r2ai_message_free(&msg); // Should not crash

	// 2. Test freeing a message with tool calls
	memset(&msg, 0, sizeof(R2AI_Message));
	msg.role = strdup("user");
	msg.json = strdup("content");
	msg.n_tool_calls = 2;
	msg.tool_calls = R_NEWS0(R2AI_Tool_Call, 2);
	assert(msg.tool_calls);
	msg.tool_calls[0].id = strdup("call_id_1");
	msg.tool_calls[0].name = strdup("tool_name_1");
	msg.tool_calls[0].arguments = strdup("{\"arg1\":\"val1\"}");
	msg.tool_calls[1].id = strdup("call_id_2");
	msg.tool_calls[1].name = NULL; // Test NULL name
	msg.tool_calls[1].arguments = strdup("{\"arg2\":\"val2\"}");
	r2ai_message_free(&msg); // Should free all strdup'd fields and tool_calls array

	// 3. Test freeing a message with content_blocks
	memset(&msg, 0, sizeof(R2AI_Message));
	msg.role = strdup("assistant");
	msg.content_blocks = R_NEW0(R2AI_Content_Blocks);
	assert(msg.content_blocks);
	msg.content_blocks->n_blocks = 1;
	msg.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 1);
	assert(msg.content_blocks->blocks);
	msg.content_blocks->blocks[0].type = strdup("text");
	msg.content_blocks->blocks[0].text = strdup("This is text in a block.");
	msg.content_blocks->blocks[0].id = strdup("block_id_1");
	// Other fields in block can be NULL
	r2ai_message_free(&msg); // Should free block fields, blocks array, and content_blocks struct

	// 4. Test freeing a partially filled message
	memset(&msg, 0, sizeof(R2AI_Message));
	msg.role = strdup("user");
	// msg.json is NULL
	msg.n_tool_calls = 1;
	msg.tool_calls = R_NEWS0(R2AI_Tool_Call, 1);
	msg.tool_calls[0].id = strdup("partial_call");
	// msg.tool_calls[0].name is NULL
	// msg.tool_calls[0].arguments is NULL
	r2ai_message_free(&msg);

	// 5. Test freeing a message with everything (role, content, tool_calls, content_blocks)
	memset(&msg, 0, sizeof(R2AI_Message));
	msg.role = strdup("multi_role");
	msg.json = strdup("multi_content");
	msg.reasoning_content = strdup("multi_reasoning");
	msg.tool_call_id = strdup("multi_tool_call_id");

	msg.n_tool_calls = 1;
	msg.tool_calls = R_NEWS0(R2AI_Tool_Call, 1);
	msg.tool_calls[0].id = strdup("multi_tc_id");
	msg.tool_calls[0].name = strdup("multi_tc_name");
	msg.tool_calls[0].arguments = strdup("multi_tc_args");

	msg.content_blocks = R_NEW0(R2AI_Content_Blocks);
	msg.content_blocks->n_blocks = 1;
	msg.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 1);
	msg.content_blocks->blocks[0].type = strdup("multi_cb_type");
	msg.content_blocks->blocks[0].text = strdup("multi_cb_text");
	msg.content_blocks->blocks[0].data = strdup("multi_cb_data");
	msg.content_blocks->blocks[0].thinking = strdup("multi_cb_thinking");
	msg.content_blocks->blocks[0].signature = strdup("multi_cb_signature");
	msg.content_blocks->blocks[0].id = strdup("multi_cb_id");
	msg.content_blocks->blocks[0].name = strdup("multi_cb_name");
	msg.content_blocks->blocks[0].input = strdup("multi_cb_input");
	r2ai_message_free(&msg);
}


// --- Test r2ai_msgs_new and r2ai_msgs_free ---
static void test_r2ai_msgs_new_and_free_extended() {
	print_test_name("test_r2ai_msgs_new_and_free_extended");

	// 1. Test r2ai_msgs_new()
	R2AI_Messages *msgs = r2ai_msgs_new();
	assert(msgs != NULL);
	assert(msgs->n_messages == 0);
	assert(msgs->cap_messages > 0); // Initial capacity should be set
	assert(msgs->messages != NULL);
	assert(msgs->system_prompt == NULL);
	r2ai_msgs_free(msgs); // Frees the R2AI_Messages struct itself

	// 2. Test freeing NULL msgs
	r2ai_msgs_free(NULL); // Should not crash

	// 3. Test freeing the global `conversation`
	r2ai_conversation_init("System P"); // Initializes the static `conversation`
	R2AI_Messages *global_conv = r2ai_conversation_get();
	assert(global_conv != NULL);
	r2ai_messasge_add(global_conv, "user", "test message", 0);
	assert(global_conv->n_messages == 1);
	
	R2AI_Messages *before_free_ptr = global_conv;
	r2ai_msgs_free(global_conv);
	
	R2AI_Messages *after_free_ptr = r2ai_conversation_get();
	assert(after_free_ptr == before_free_ptr); 
	assert(after_free_ptr->n_messages == 0);   
	assert(strcmp(after_free_ptr->system_prompt, "System P") == 0); 
	
	r2ai_conversation_free(); 
	assert(r2ai_conversation_get() == NULL);
}

// --- Test r2ai_msgs_clear ---
static void test_r2ai_msgs_clear_extended() {
	print_test_name("test_r2ai_msgs_clear_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();
	assert(msgs != NULL);

	r2ai_messasge_add(msgs, "user", "Message 1", 0);
	R2AI_Message complex_msg;
	memset(&complex_msg, 0, sizeof(R2AI_Message));
	complex_msg.role = strdup("assistant");
	complex_msg.json = strdup("Complex Content");
	complex_msg.n_tool_calls = 1;
	complex_msg.tool_calls = R_NEWS0(R2AI_Tool_Call, 1);
	complex_msg.tool_calls[0].id = strdup("tc_clear_id");
	complex_msg.tool_calls[0].name = strdup("tc_clear_name");
	complex_msg.tool_calls[0].arguments = strdup("tc_clear_args");
	r2ai_messasge_add_msg(msgs, &complex_msg); 
	r2ai_message_free(&complex_msg); 

	assert(msgs->n_messages == 2);
	msgs->system_prompt = strdup("Test System Prompt for Clear");

	r2ai_msgs_clear(msgs);
	assert(msgs->n_messages == 0);
	assert(msgs->system_prompt != NULL && strcmp(msgs->system_prompt, "Test System Prompt for Clear") == 0);

	r2ai_msgs_free(msgs); 
}


// --- Test r2ai_msgs_add (r2ai_messasge_add and r2ai_messasge_add_msg) ---
static void test_r2ai_msgs_add_extended() {
	print_test_name("test_r2ai_msgs_add_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();
	assert(msgs != NULL);
	size_t initial_capacity = msgs->cap_messages;

	for (size_t i = 0; i < initial_capacity + 5; i++) {
		char content[32];
		snprintf(content, sizeof(content), "Message %zu", i);
		r2ai_messasge_add(msgs, "user", content, 0);
	}
	assert(msgs->n_messages == initial_capacity + 5);
	assert(msgs->cap_messages > initial_capacity); 

	R2AI_Message msg_with_blocks;
	memset(&msg_with_blocks, 0, sizeof(R2AI_Message));
	msg_with_blocks.role = strdup("user_blocks");
	msg_with_blocks.content_blocks = R_NEW0(R2AI_Content_Blocks);
	msg_with_blocks.content_blocks->n_blocks = 1;
	msg_with_blocks.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 1);
	msg_with_blocks.content_blocks->blocks[0].type = strdup("text_block");
	msg_with_blocks.content_blocks->blocks[0].text = strdup("Block text content");
	
	r2ai_messasge_add_msg(msgs, &msg_with_blocks);
	assert(msgs->n_messages == initial_capacity + 5 + 1);
	R2AI_Message *added_blocks_msg = &msgs->messages[msgs->n_messages - 1];
	assert(strcmp(added_blocks_msg->role, "user_blocks") == 0);
	assert(added_blocks_msg->content_blocks != NULL);
	assert(added_blocks_msg->content_blocks->n_blocks == 1);
	assert(strcmp(added_blocks_msg->content_blocks->blocks[0].type, "text_block") == 0);
	assert(strcmp(added_blocks_msg->content_blocks->blocks[0].text, "Block text content") == 0);
	free(msg_with_blocks.content_blocks->blocks[0].text);
	msg_with_blocks.content_blocks->blocks[0].text = strdup("Modified original");
	assert(strcmp(added_blocks_msg->content_blocks->blocks[0].text, "Block text content") == 0);
	r2ai_message_free(&msg_with_blocks); 

	R2AI_Message msg_with_tc;
	memset(&msg_with_tc, 0, sizeof(R2AI_Message));
	msg_with_tc.role = strdup("user_tc");
	msg_with_tc.n_tool_calls = 1;
	msg_with_tc.tool_calls = R_NEWS0(R2AI_Tool_Call, 1);
	msg_with_tc.tool_calls[0].id = strdup("tc_add_id");
	msg_with_tc.tool_calls[0].name = strdup("tc_add_name");
	msg_with_tc.tool_calls[0].arguments = strdup("tc_add_args");

	r2ai_messasge_add_msg(msgs, &msg_with_tc);
	assert(msgs->n_messages == initial_capacity + 5 + 1 + 1);
	R2AI_Message *added_tc_msg = &msgs->messages[msgs->n_messages - 1];
	assert(strcmp(added_tc_msg->role, "user_tc") == 0);
	assert(added_tc_msg->n_tool_calls == 1);
	assert(strcmp(added_tc_msg->tool_calls[0].id, "tc_add_id") == 0);
	free(msg_with_tc.tool_calls[0].name);
	msg_with_tc.tool_calls[0].name = strdup("Modified original TC name");
	assert(strcmp(added_tc_msg->tool_calls[0].name, "tc_add_name") == 0);
	r2ai_message_free(&msg_with_tc);

	r2ai_messasge_add(msgs, NULL, "Content with NULL role", 0); 
	assert(strcmp(msgs->messages[msgs->n_messages - 1].role, "user") == 0);
	assert(strcmp(msgs->messages[msgs->n_messages - 1].json, "Content with NULL role") == 0);

	r2ai_messasge_add(msgs, "assistant", NULL, 0); 
	assert(strcmp(msgs->messages[msgs->n_messages - 1].role, "assistant") == 0);
	assert(msgs->messages[msgs->n_messages - 1].json == NULL);

	r2ai_msgs_free(msgs);
}

// --- Test r2ai_msgs_add_tool_call ---
static void test_r2ai_msgs_add_tool_call_extended() {
	print_test_name("test_r2ai_msgs_add_tool_call_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();
	r2ai_messasge_add(msgs, "assistant", "Initial assistant message", 0);
	R2AI_Message *msg_ptr = &msgs->messages[0]; 

	assert(msg_ptr->n_tool_calls == 0);
	assert(msg_ptr->tool_calls == NULL);
	r2ai_msgs_add_tool_call(msg_ptr, "id1", "name1", "args1");
	assert(msg_ptr->n_tool_calls == 1);
	assert(msg_ptr->tool_calls != NULL);
	assert(strcmp(msg_ptr->tool_calls[0].id, "id1") == 0);
	assert(strcmp(msg_ptr->tool_calls[0].name, "name1") == 0);
	assert(strcmp(msg_ptr->tool_calls[0].arguments, "args1") == 0);

	r2ai_msgs_add_tool_call(msg_ptr, "id2", "name2", "args2");
	r2ai_msgs_add_tool_call(msg_ptr, "id3", "name3", "args3");
	assert(msg_ptr->n_tool_calls == 3);
	assert(strcmp(msg_ptr->tool_calls[1].id, "id2") == 0);
	assert(strcmp(msg_ptr->tool_calls[2].name, "name3") == 0);

	r2ai_msgs_add_tool_call(msg_ptr, NULL, "name4_null_id", "args4_null_id");
	assert(msg_ptr->n_tool_calls == 4);
	assert(msg_ptr->tool_calls[3].id == NULL);
	assert(strcmp(msg_ptr->tool_calls[3].name, "name4_null_id") == 0);

	r2ai_msgs_add_tool_call(msg_ptr, "id5_null_name", NULL, "args5_null_name");
	assert(msg_ptr->n_tool_calls == 5);
	assert(msg_ptr->tool_calls[4].name == NULL);
	assert(strcmp(msg_ptr->tool_calls[4].id, "id5_null_name") == 0);

	r2ai_msgs_add_tool_call(msg_ptr, "id6_null_args", "name6_null_args", NULL);
	assert(msg_ptr->n_tool_calls == 6);
	assert(msg_ptr->tool_calls[5].arguments == NULL);
	assert(strcmp(msg_ptr->tool_calls[5].id, "id6_null_args") == 0);

	r2ai_msgs_free(msgs);
}

// --- Test r2ai_msgs_from_response and r2ai_msgs_from_json (JSON Parsing) ---
static void test_json_parsing_extended() {
	print_test_name("test_json_parsing_extended");
	R2AI_Messages *msgs;
	R2AI_Message *parsed_msg;

	const char *json_multi_choice = "{ \"choices\": [ { \"message\": { \"role\": \"assistant\", \"content\": \"Choice 1\" } }, { \"message\": { \"role\": \"assistant\", \"content\": \"Choice 2\" } } ] }";
	msgs = r2ai_msgs_from_response(json_multi_choice);
	assert(msgs != NULL && msgs->n_messages == 1);
	parsed_msg = &msgs->messages[0];
	assert(strcmp(parsed_msg->role, "assistant") == 0);
	assert(strcmp(parsed_msg->json, "Choice 1") == 0);
	r2ai_msgs_free(msgs);

	const char *json_missing_content = "{ \"choices\": [ { \"message\": { \"role\": \"assistant\" } } ] }"; 
	msgs = r2ai_msgs_from_response(json_missing_content);
	assert(msgs != NULL && msgs->n_messages == 1);
	parsed_msg = &msgs->messages[0];
	assert(strcmp(parsed_msg->role, "assistant") == 0);
	assert(parsed_msg->json == NULL);
	r2ai_msgs_free(msgs);

	const char *json_missing_role = "{ \"choices\": [ { \"message\": { \"content\": \"No role here\" } } ] }"; 
	msgs = r2ai_msgs_from_response(json_missing_role);
	assert(msgs != NULL && msgs->n_messages == 1);
	parsed_msg = &msgs->messages[0];
	assert(strcmp(parsed_msg->role, "assistant") == 0); 
	assert(strcmp(parsed_msg->json, "No role here") == 0);
	r2ai_msgs_free(msgs);
	
	const char *json_missing_message = "{ \"choices\": [ { } ] }"; 
	msgs = r2ai_msgs_from_response(json_missing_message);
	assert(msgs != NULL && msgs->n_messages == 1); 
	parsed_msg = &msgs->messages[0];
	assert(strcmp(parsed_msg->role, "assistant") == 0); 
	assert(parsed_msg->json == NULL);
	r2ai_msgs_free(msgs);

	const char *json_missing_choices = "{ }"; 
	msgs = r2ai_msgs_from_response(json_missing_choices);
	assert(msgs != NULL && msgs->n_messages == 0); 
	r2ai_msgs_free(msgs);

	const char *json_with_blocks_anthropic = R"({ "role": "user", "content": [ 
		{ "type": "text", "text": "Hello block" }, 
		{ "type": "tool_use", "id": "tool_id_123", "name": "get_cmd_output", "input": { "command": "pd 10" } },
		{ "type": "image_url", "image_url": { "url": "data:image/jpeg;base64,ABCD" } } 
	] })"; // This is an Anthropic style message, not a full response object
	msgs = r2ai_msgs_new();
	// Use r2ai_msgs_from_json with new_msg = true to test parsing a single message object
	parsed_msg = r2ai_msgs_from_json(json_with_blocks_anthropic, msgs, true); 
	assert(parsed_msg != NULL); // new_msg = true returns the parsed message
	assert(strcmp(parsed_msg->role, "user") == 0);
	assert(parsed_msg->json == NULL); 
	assert(parsed_msg->content_blocks != NULL);
	assert(parsed_msg->content_blocks->n_blocks == 3);
	assert(strcmp(parsed_msg->content_blocks->blocks[0].type, "text") == 0);
	assert(strcmp(parsed_msg->content_blocks->blocks[0].text, "Hello block") == 0);
	assert(strcmp(parsed_msg->content_blocks->blocks[1].type, "tool_use") == 0);
	assert(strcmp(parsed_msg->content_blocks->blocks[1].id, "tool_id_123") == 0);
	assert(strcmp(parsed_msg->content_blocks->blocks[1].name, "get_cmd_output") == 0);
	assert(strstr(parsed_msg->content_blocks->blocks[1].input, "\"command\":\"pd 10\"") != NULL); 
	assert(strcmp(parsed_msg->content_blocks->blocks[2].type, "image_url") == 0);
	assert(strstr(parsed_msg->content_blocks->blocks[2].text, "data:image/jpeg;base64,ABCD") != NULL); 
	r2ai_message_free(parsed_msg); // Free the message returned by r2ai_msgs_from_json when new_msg=true
	r2ai_msgs_free(msgs);


	const char *json_with_tool_calls_openai = R"({ "choices": [ { "message": { "role": "assistant", "tool_calls": [
		{ "id": "tc_id1", "type": "function", "function": { "name": "func1", "arguments": "{\"arg\":\"val1\"}" } },
		{ "id": "tc_id2", "type": "function", "function": { "name": "func2", "arguments": "{\"arg\":\"val2\"}" } }
	] } } ] })";
	msgs = r2ai_msgs_from_response(json_with_tool_calls_openai);
	assert(msgs != NULL && msgs->n_messages == 1);
	parsed_msg = &msgs->messages[0];
	assert(strcmp(parsed_msg->role, "assistant") == 0);
	assert(parsed_msg->n_tool_calls == 2);
	assert(strcmp(parsed_msg->tool_calls[0].id, "tc_id1") == 0);
	assert(strcmp(parsed_msg->tool_calls[0].name, "func1") == 0);
	assert(strcmp(parsed_msg->tool_calls[0].arguments, "{\"arg\":\"val1\"}") == 0);
	assert(strcmp(parsed_msg->tool_calls[1].name, "func2") == 0);
	r2ai_msgs_free(msgs);

	const char *json_tc_missing_fields = R"({ "choices": [ { "message": { "role": "assistant", "tool_calls": [
		{ "id": "tc_id_ok", "type": "function", "function": { "name": "func_ok", "arguments": "{}" } },
		{ "type": "function", "function": { "name": "func_no_id", "arguments": "{}" } },
		{ "id": "tc_id_no_func_obj", "type": "function" },
		{ "id": "tc_id_no_name", "type": "function", "function": { "arguments": "{}" } },
		{ "id": "tc_id_no_args", "type": "function", "function": { "name": "func_no_args" } }
	] } } ] })";
	msgs = r2ai_msgs_from_response(json_tc_missing_fields);
	assert(msgs != NULL && msgs->n_messages == 1);
	parsed_msg = &msgs->messages[0];
	assert(parsed_msg->n_tool_calls == 5); 
	assert(strcmp(parsed_msg->tool_calls[0].id, "tc_id_ok") == 0); 
	assert(parsed_msg->tool_calls[1].id == NULL); 
	assert(strcmp(parsed_msg->tool_calls[1].name, "func_no_id") == 0);
	assert(parsed_msg->tool_calls[2].id != NULL && strcmp(parsed_msg->tool_calls[2].id, "tc_id_no_func_obj") == 0);
	assert(parsed_msg->tool_calls[2].name == NULL && parsed_msg->tool_calls[2].arguments == NULL);
	assert(parsed_msg->tool_calls[3].name == NULL); 
	assert(parsed_msg->tool_calls[4].arguments == NULL); 
	r2ai_msgs_free(msgs);

	msgs = r2ai_msgs_from_response("{ \"invalid_json\": ");
	assert(msgs != NULL && msgs->n_messages == 0); 
	r2ai_msgs_free(msgs);

    // Test unknown content block type & malformed tool_use/tool_result
    const char *json_unknown_malformed_blocks = R"({ "role": "user", "content": [
        { "type": "text", "text": "Valid text block" },
        { "type": "future_feature_block", "data": "some data" }, 
        { "type": "tool_use", "name": "tool_no_id", "input": {} }, 
        { "type": "tool_use", "id": "id_no_name", "input": {} },   
        { "type": "tool_result", "content": "result content no id" }, 
        { "type": "text", "text": "Another valid text block" }
    ]})";
    msgs = r2ai_msgs_new();
    parsed_msg = r2ai_msgs_from_json(json_unknown_malformed_blocks, msgs, true);
    assert(parsed_msg != NULL);
    assert(parsed_msg->content_blocks != NULL);
    // Expected: text, tool_use(no_id), tool_use(no_name), tool_result(no_id), text. Unknown block is skipped.
    assert(parsed_msg->content_blocks->n_blocks == 5); 
    assert(strcmp(parsed_msg->content_blocks->blocks[0].type, "text") == 0);
    assert(strcmp(parsed_msg->content_blocks->blocks[0].text, "Valid text block") == 0);
    // Block 1: tool_use without id
    assert(strcmp(parsed_msg->content_blocks->blocks[1].type, "tool_use") == 0);
    assert(parsed_msg->content_blocks->blocks[1].id == NULL);
    assert(strcmp(parsed_msg->content_blocks->blocks[1].name, "tool_no_id") == 0);
    // Block 2: tool_use without name
    assert(strcmp(parsed_msg->content_blocks->blocks[2].type, "tool_use") == 0);
    assert(strcmp(parsed_msg->content_blocks->blocks[2].id, "id_no_name") == 0);
    assert(parsed_msg->content_blocks->blocks[2].name == NULL);
    // Block 3: tool_result without tool_use_id
    assert(strcmp(parsed_msg->content_blocks->blocks[3].type, "tool_result") == 0);
    assert(parsed_msg->content_blocks->blocks[3].id == NULL); // tool_use_id is stored in block->id
    assert(strcmp(parsed_msg->content_blocks->blocks[3].text, "result content no id") == 0);
    // Block 4: another valid text block
    assert(strcmp(parsed_msg->content_blocks->blocks[4].type, "text") == 0);
    assert(strcmp(parsed_msg->content_blocks->blocks[4].text, "Another valid text block") == 0);
    r2ai_message_free(parsed_msg);
    r2ai_msgs_free(msgs);
}

// --- Test r2ai_msgs_to_json (OpenAI JSON Serialization) ---
static void test_r2ai_msgs_to_json_extended() {
	print_test_name("test_r2ai_msgs_to_json_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();
	RJson *rj;
	char *json_str;

	R2AI_Message msg_with_blocks;
	memset(&msg_with_blocks, 0, sizeof(R2AI_Message));
	msg_with_blocks.role = strdup("user");
	msg_with_blocks.json = strdup("This is the main content."); 
	msg_with_blocks.content_blocks = R_NEW0(R2AI_Content_Blocks);
	msg_with_blocks.content_blocks->n_blocks = 1;
	msg_with_blocks.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 1);
	msg_with_blocks.content_blocks->blocks[0].type = strdup("text");
	msg_with_blocks.content_blocks->blocks[0].text = strdup("Block content (should not appear)");
	r2ai_messasge_add_msg(msgs, &msg_with_blocks);
	r2ai_message_free(&msg_with_blocks);

	rj = r2ai_msgs_to_json(msgs);
	json_str = r_json_to_string(rj);
	assert(strstr(json_str, "\"content\":\"This is the main content.\"") != NULL);
	assert(strstr(json_str, "Block content (should not appear)") == NULL);
	free(json_str);
	r_json_free(rj);

	r2ai_msgs_clear(msgs);
	R2AI_Message msg_reasoning;
	memset(&msg_reasoning, 0, sizeof(R2AI_Message));
	msg_reasoning.role = strdup("tool"); 
	msg_reasoning.json = strdup("Tool output content");
	msg_reasoning.reasoning_content = strdup("Some reasoning here"); 
	msg_reasoning.tool_call_id = strdup("tc_id_for_tool_msg");
	r2ai_messasge_add_msg(msgs, &msg_reasoning);
	r2ai_message_free(&msg_reasoning);

	rj = r2ai_msgs_to_json(msgs);
	json_str = r_json_to_string(rj);
	assert(strstr(json_str, "\"role\":\"tool\"") != NULL);
	assert(strstr(json_str, "\"content\":\"Tool output content\"") != NULL);
	assert(strstr(json_str, "\"tool_call_id\":\"tc_id_for_tool_msg\"") != NULL);
	assert(strstr(json_str, "Some reasoning here") == NULL); 
	free(json_str);
	r_json_free(rj);

	r2ai_msgs_clear(msgs);
	R2AI_Message msg_multi_tc;
	memset(&msg_multi_tc, 0, sizeof(R2AI_Message));
	msg_multi_tc.role = strdup("assistant");
	r2ai_msgs_add_tool_call(&msg_multi_tc, "id1", "name1", "args1");
	r2ai_msgs_add_tool_call(&msg_multi_tc, "id2", "name2", "args2");
	r2ai_messasge_add_msg(msgs, &msg_multi_tc);
	r2ai_message_free(&msg_multi_tc);

	rj = r2ai_msgs_to_json(msgs);
	json_str = r_json_to_string(rj);
	assert(strstr(json_str, "\"tool_calls\":[") != NULL);
	assert(strstr(json_str, "\"id\":\"id1\"") != NULL);
	assert(strstr(json_str, "\"name\":\"name1\"") != NULL);
	assert(strstr(json_str, "\"arguments\":\"args1\"") != NULL); 
	assert(strstr(json_str, "\"id\":\"id2\"") != NULL);
	free(json_str);
	r_json_free(rj);
	
	r2ai_msgs_clear(msgs);
	r2ai_messasge_add(msgs, NULL, "Content for NULL role", 0); 
	r2ai_messasge_add(msgs, "assistant", NULL, 0); 

	rj = r2ai_msgs_to_json(msgs);
	json_str = r_json_to_string(rj);
	assert(strstr(json_str, "{\"role\":\"user\",\"content\":\"Content for NULL role\"}") != NULL);
	assert(strstr(json_str, "{\"role\":\"assistant\",\"content\":null}") != NULL);
	free(json_str);
	r_json_free(rj);

	r2ai_msgs_free(msgs);
}


// --- Test r2ai_msgs_to_anthropic_json ---
static void test_r2ai_msgs_to_anthropic_json_extended() {
	print_test_name("test_r2ai_msgs_to_anthropic_json_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();
	RJson *rj_anthropic;
	char *json_str_anthropic;

	R2AI_Message msg_blocks_anth;
	memset(&msg_blocks_anth, 0, sizeof(R2AI_Message));
	msg_blocks_anth.role = strdup("user");
	msg_blocks_anth.content_blocks = R_NEW0(R2AI_Content_Blocks);
	msg_blocks_anth.content_blocks->n_blocks = 2;
	msg_blocks_anth.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 2);
	msg_blocks_anth.content_blocks->blocks[0].type = strdup("text");
	msg_blocks_anth.content_blocks->blocks[0].text = strdup("Anthropic text block.");
	msg_blocks_anth.content_blocks->blocks[1].type = strdup("tool_use"); 
	msg_blocks_anth.content_blocks->blocks[1].id = strdup("anth_tool_id");
	msg_blocks_anth.content_blocks->blocks[1].name = strdup("anth_tool_name");
	msg_blocks_anth.content_blocks->blocks[1].input = strdup("{\"command\":\"ls\"}"); 
	r2ai_messasge_add_msg(msgs, &msg_blocks_anth);
	r2ai_message_free(&msg_blocks_anth);

	rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
	json_str_anthropic = r_json_to_string(rj_anthropic);
	assert(strstr(json_str_anthropic, "\"role\":\"user\"") != NULL);
	assert(strstr(json_str_anthropic, "\"content\":[") != NULL);
	assert(strstr(json_str_anthropic, "{\"type\":\"text\",\"text\":\"Anthropic text block.\"}") != NULL);
	assert(strstr(json_str_anthropic, "{\"type\":\"tool_use\",\"id\":\"anth_tool_id\",\"name\":\"anth_tool_name\",\"input\":{\"command\":\"ls\"}}") != NULL);
	free(json_str_anthropic);
	r_json_free(rj_anthropic);

	r2ai_msgs_clear(msgs);
	r2ai_messasge_add(msgs, "user", "Simple user message.", 0);
	r2ai_messasge_add(msgs, "assistant", "Simple assistant reply.", 0);
	rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
	json_str_anthropic = r_json_to_string(rj_anthropic);
	assert(strstr(json_str_anthropic, "{\"role\":\"user\",\"content\":\"Simple user message.\"}") != NULL);
	assert(strstr(json_str_anthropic, "{\"role\":\"assistant\",\"content\":\"Simple assistant reply.\"}") != NULL);
	free(json_str_anthropic);
	r_json_free(rj_anthropic);

	r2ai_msgs_clear(msgs);
	R2AI_Message msg_tool_result;
	memset(&msg_tool_result, 0, sizeof(R2AI_Message));
	msg_tool_result.role = strdup("tool");
	msg_tool_result.tool_call_id = strdup("original_tool_id");
	msg_tool_result.json = strdup("Output from the tool call."); 
	r2ai_messasge_add_msg(msgs, &msg_tool_result);
	r2ai_message_free(&msg_tool_result);

	rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
	json_str_anthropic = r_json_to_string(rj_anthropic);
	assert(strstr(json_str_anthropic, "\"role\":\"user\"") != NULL); 
	assert(strstr(json_str_anthropic, "\"content\":[{\"type\":\"tool_result\",\"tool_use_id\":\"original_tool_id\",\"content\":\"Output from the tool call.\"}]") != NULL);
	free(json_str_anthropic);
	r_json_free(rj_anthropic);

	r2ai_msgs_clear(msgs);
	R2AI_Message msg_assistant_tc;
	memset(&msg_assistant_tc, 0, sizeof(R2AI_Message));
	msg_assistant_tc.role = strdup("assistant");
	msg_assistant_tc.json = strdup("Optional text part from assistant."); 
	r2ai_msgs_add_tool_call(&msg_assistant_tc, "as_tc_id1", "as_tc_name1", "{\"param\":1}");
	r2ai_messasge_add_msg(msgs, &msg_assistant_tc);
	r2ai_message_free(&msg_assistant_tc);

	rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
	json_str_anthropic = r_json_to_string(rj_anthropic);
	assert(strstr(json_str_anthropic, "\"role\":\"assistant\"") != NULL);
	assert(strstr(json_str_anthropic, "\"content\":[") != NULL);
	assert(strstr(json_str_anthropic, "{\"type\":\"text\",\"text\":\"Optional text part from assistant.\"}") != NULL);
	assert(strstr(json_str_anthropic, "{\"type\":\"tool_use\",\"id\":\"as_tc_id1\",\"name\":\"as_tc_name1\",\"input\":{\"param\":1}}") != NULL);
	free(json_str_anthropic);
	r_json_free(rj_anthropic);

	r2ai_msgs_clear(msgs);
	R2AI_Message msg_null_fields;
	memset(&msg_null_fields, 0, sizeof(R2AI_Message));
	msg_null_fields.role = strdup("user"); 
	r2ai_messasge_add_msg(msgs, &msg_null_fields);
	r2ai_message_free(&msg_null_fields);

	rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
	json_str_anthropic = r_json_to_string(rj_anthropic);
	assert(strstr(json_str_anthropic, "{\"role\":\"user\",\"content\":\"\"}") != NULL);
	free(json_str_anthropic);
	r_json_free(rj_anthropic);

    // New: User message with tool_calls
    r2ai_msgs_clear(msgs);
    R2AI_Message user_msg_with_tc;
    memset(&user_msg_with_tc, 0, sizeof(R2AI_Message));
    user_msg_with_tc.role = strdup("user");
    user_msg_with_tc.json = strdup("User text with tool calls.");
    r2ai_msgs_add_tool_call(&user_msg_with_tc, "user_tc_id", "user_tc_name", "{}");
    r2ai_messasge_add_msg(msgs, &user_msg_with_tc);
    r2ai_message_free(&user_msg_with_tc);

    rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
    json_str_anthropic = r_json_to_string(rj_anthropic);
    // Current behavior: user messages with tool_calls are serialized as plain text content.
    // The tool_calls are ignored for user role in Anthropic conversion.
    // If `json` is present, it's used. If `json` is NULL but `content_blocks` exist, they are used.
    // If both `json` and `tool_calls` exist for a user message, `json` takes precedence.
    // r_cons_printf("Anthropic User with TC: %s\n", json_str_anthropic);
    assert(strstr(json_str_anthropic, "{\"role\":\"user\",\"content\":\"User text with tool calls.\"}") != NULL);
    assert(strstr(json_str_anthropic, "tool_use") == NULL); // No tool_use blocks from user's tool_calls
    free(json_str_anthropic);
    r_json_free(rj_anthropic);

    // New: "tool" role message with existing content_blocks
    r2ai_msgs_clear(msgs);
    R2AI_Message tool_msg_with_blocks;
    memset(&tool_msg_with_blocks, 0, sizeof(R2AI_Message));
    tool_msg_with_blocks.role = strdup("tool");
    tool_msg_with_blocks.tool_call_id = strdup("tool_id_for_blocks_test");
    // tool_msg_with_blocks.json = strdup("This would usually become tool_result content"); // Leave this out
    tool_msg_with_blocks.content_blocks = R_NEW0(R2AI_Content_Blocks);
    tool_msg_with_blocks.content_blocks->n_blocks = 1;
    tool_msg_with_blocks.content_blocks->blocks = R_NEWS0(R2AI_Content_Block, 1);
    tool_msg_with_blocks.content_blocks->blocks[0].type = strdup("text"); // User provides a text block as "tool output"
    tool_msg_with_blocks.content_blocks->blocks[0].text = strdup("Pre-existing text block for tool result.");
    r2ai_messasge_add_msg(msgs, &tool_msg_with_blocks);
    r2ai_message_free(&tool_msg_with_blocks);
    
    rj_anthropic = r2ai_msgs_to_anthropic_json(msgs, NULL);
    json_str_anthropic = r_json_to_string(rj_anthropic);
    // Current behavior: if role is "tool", it creates a "tool_result" block.
    // If msg->json exists, it's used as content for that block.
    // If msg->json is NULL but msg->content_blocks exist, the *first* block's text is used as content.
    // r_cons_printf("Anthropic Tool with Blocks: %s\n", json_str_anthropic);
    assert(strstr(json_str_anthropic, "\"role\":\"user\"") != NULL);
    assert(strstr(json_str_anthropic, "\"content\":[{\"type\":\"tool_result\",\"tool_use_id\":\"tool_id_for_blocks_test\",\"content\":\"Pre-existing text block for tool result.\"}]") != NULL);
    free(json_str_anthropic);
    r_json_free(rj_anthropic);


	r2ai_msgs_free(msgs);
}

// --- Test r2ai_delete_last_messages ---
static void test_r2ai_delete_last_messages_extended() {
	print_test_name("test_r2ai_delete_last_messages_extended");
	R2AI_Messages *msgs = r2ai_msgs_new();

	r2ai_messasge_add(msgs, "user", "Msg1", 0);
	r2ai_messasge_add(msgs, "assistant", "Msg2", 0);
	r2ai_messasge_add(msgs, "user", "Msg3", 0);
	r2ai_messasge_add(msgs, "assistant", "Msg4", 0);
	assert(msgs->n_messages == 4);

	r2ai_delete_last_messages(msgs, 0);
	assert(msgs->n_messages == 4);

	r2ai_delete_last_messages(msgs, 1);
	assert(msgs->n_messages == 3);
	assert(strcmp(msgs->messages[msgs->n_messages - 1].json, "Msg3") == 0);

	r2ai_delete_last_messages(msgs, 2);
	assert(msgs->n_messages == 1);
	assert(strcmp(msgs->messages[0].json, "Msg1") == 0);

	r2ai_delete_last_messages(msgs, 5);
	assert(msgs->n_messages == 0);

	r2ai_delete_last_messages(msgs, 1);
	assert(msgs->n_messages == 0);

	r2ai_msgs_free(msgs);

	r2ai_delete_last_messages(NULL, 1); 
}

// --- Test r_json_to_pj and r_json_to_string (indirectly, but one specific case) ---
static void test_r_json_to_pj_and_string() { // Renamed for clarity
	print_test_name("test_r_json_to_pj_and_string");
	PJ *pj = pj_new();
	assert(pj != NULL);

	pj_o(pj);
	pj_ks(pj, "initial_key", "initial_value");
	pj_end(pj);

	RJson *rj_to_append = r_json_new_object();
	r_json_object_set_string(rj_to_append, "appended_key", "appended_value");

	PJ *converted_pj = r_json_to_pj(rj_to_append);
	assert(converted_pj != NULL);
	char *converted_str = pj_string(converted_pj);
	assert(strstr(converted_str, "\"appended_key\":\"appended_value\"") != NULL);

	pj_free(pj);
	pj_free(converted_pj);
	free(converted_str);
	r_json_free(rj_to_append);

	RJson *rj_for_string = r_json_new_object();
	r_json_object_set_string(rj_for_string, "key_str", "value_str");
	r_json_object_set_number(rj_for_string, "key_num", 123);
	RJson *arr_str = r_json_new_array();
	r_json_array_append_string(arr_str, "item1");
	r_json_object_set(rj_for_string, "key_arr", arr_str);

	char *stringified = r_json_to_string(rj_for_string);
	assert(strstr(stringified, "\"key_str\":\"value_str\"") != NULL);
	assert(strstr(stringified, "\"key_num\":123") != NULL);
	assert(strstr(stringified, "\"key_arr\":[\"item1\"]") != NULL);
	free(stringified);
	r_json_free(rj_for_string);
}


// --- Original tests from the file (kept for reference/integration) ---
static void test_msgs_add_and_delete(void) {
	print_test_name("test_msgs_add_and_delete (original)");
	R2AI_Messages *msgs = r2ai_msgs_new();
	assert(msgs != NULL);
	assert(msgs->n_messages == 0);

	r2ai_messasge_add(msgs, "user", "Hello", 0);
	assert(msgs->n_messages == 1);
	assert(strcmp(msgs->messages[0].role, "user") == 0);
	assert(strcmp(msgs->messages[0].json, "Hello") == 0);

	r2ai_messasge_add(msgs, "assistant", "Hi there", 0);
	assert(msgs->n_messages == 2);

	r2ai_delete_last_messages(msgs, 1);
	assert(msgs->n_messages == 1);
	assert(strcmp(msgs->messages[0].json, "Hello") == 0);

	r2ai_delete_last_messages(msgs, 1);
	assert(msgs->n_messages == 0);

	r2ai_msgs_free(msgs);
}

static void test_json_roundtrip(void) {
	print_test_name("test_json_roundtrip (original)");
	const char *response_json = "{ \"id\": \"chatcmpl-foo\", \"choices\": [ { \"index\": 0, \"message\": { \"role\": \"assistant\", \"content\": \"Hello, I am R2AI!\" }, \"finish_reason\": \"stop\" } ], \"model\": \"gpt-foo\", \"created\": 123, \"object\": \"chat.completion\", \"usage\": { \"prompt_tokens\": 1, \"completion_tokens\": 2, \"total_tokens\": 3 } }";

	R2AI_Messages *msgs_from_json = r2ai_msgs_from_response(response_json);
	assert(msgs_from_json != NULL);
	assert(msgs_from_json->n_messages == 1);
	R2AI_Message *msg = &msgs_from_json->messages[0];
	assert(strcmp(msg->role, "assistant") == 0);
	assert(strcmp(msg->json, "Hello, I am R2AI!") == 0);

	RJson *rj_to_json = r2ai_msgs_to_json(msgs_from_json);
	char *json_output_str = r_json_to_string(rj_to_json);

	assert(strstr(json_output_str, "\"role\":\"assistant\"") != NULL);
	assert(strstr(json_output_str, "\"content\":\"Hello, I am R2AI!\"") != NULL);

	free(json_output_str);
	r_json_free(rj_to_json);
	r2ai_msgs_free(msgs_from_json);
}


int main(int argc, char **argv) {
	// Original tests
	test_msgs_add_and_delete();
	test_json_roundtrip();

	// Extended tests
	test_r2ai_message_free_extended();
	test_r2ai_msgs_new_and_free_extended();
	test_r2ai_msgs_clear_extended();
	test_r2ai_msgs_add_extended();
	test_r2ai_msgs_add_tool_call_extended();
	test_json_parsing_extended(); // This now includes the new parsing tests
	test_r2ai_msgs_to_json_extended();
	test_r2ai_msgs_to_anthropic_json_extended(); // This now includes the new Anthropic edge case tests
	test_r2ai_delete_last_messages_extended();
	test_r_json_to_pj_and_string(); 

	printf("[PASS] All messages tests completed.\n");
	return 0;
}
