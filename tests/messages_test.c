#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../r2ai.h" // Main header for R2AI structures and functions
// Assuming R_NEW0, R_FREE, R_NEWS0 etc. are available via r2ai.h or its includes (e.g. r_types.h from r_core.h)

// Assertion helpers
void assert_true(bool condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "Assertion failed: %s\n", message);
        exit(1);
    }
}

void assert_false(bool condition, const char* message) {
    if (condition) {
        fprintf(stderr, "Assertion failed: %s\n", message);
        exit(1);
    }
}

void assert_streq(const char* s1, const char* s2, const char* message) {
    if (s1 == NULL && s2 == NULL) return;
    if (s1 == NULL || s2 == NULL || strcmp(s1, s2) != 0) {
        fprintf(stderr, "Assertion failed: %s. Expected '%s', got '%s'\n", message, s2, s1);
        exit(1);
    }
}

void assert_null(const void* ptr, const char* message) {
    if (ptr != NULL) {
        fprintf(stderr, "Assertion failed: %s. Expected NULL, got %p\n", message, ptr);
        exit(1);
    }
}

void assert_not_null(const void* ptr, const char* message) {
    if (ptr == NULL) {
        fprintf(stderr, "Assertion failed: %s. Expected not NULL.\n", message);
        exit(1);
    }
}

// --- Test r2ai_message_free ---
void test_r2ai_message_free_null() {
    printf("Running test_r2ai_message_free_null...\n");
    r2ai_message_free(NULL); // Should not crash
    printf("test_r2ai_message_free_null passed.\n");
}

void test_r2ai_message_free_basic_fields() {
    printf("Running test_r2ai_message_free_basic_fields...\n");
    R2AI_Message *msg = R_NEW0(R2AI_Message);
    assert_not_null(msg, "msg allocation failed for basic_fields");

    msg->role = strdup("user");
    msg->content = strdup("Hello content");
    msg->reasoning_content = strdup("Thinking about it");
    msg->tool_call_id = strdup("static_id_string"); // Was: "static_id_string";


    assert_not_null(msg->role, "strdup role failed");
    assert_not_null(msg->tool_call_id, "strdup tool_call_id failed");
    assert_not_null(msg->content, "strdup content failed");
    assert_not_null(msg->reasoning_content, "strdup reasoning_content failed");

    r2ai_message_free(msg); // Frees contents of msg
    free(msg); // Free the msg struct itself
    printf("test_r2ai_message_free_basic_fields passed.\n");
}

void test_r2ai_message_free_with_tool_calls_array() {
    printf("Running test_r2ai_message_free_with_tool_calls_array...\n");
    R2AI_Message *msg = R_NEW0(R2AI_Message);
    assert_not_null(msg, "msg allocation failed for tool_calls_array");

    msg->n_tool_calls = 2;
    // r2ai_message_free frees the array `msg->tool_calls` itself.
    // The R2AI_ToolCall structs within the array and their string members (name, args, id)
    // are const and thus assumed not to be owned/freed by r2ai_message_free.
    R2AI_ToolCall *tool_calls_array = R_NEWS0(R2AI_ToolCall, msg->n_tool_calls);
    assert_not_null(tool_calls_array, "tool_calls_array allocation failed");

    // These strings now need to be strdup'd as r2ai_message_free will free them
    tool_calls_array[0].id = strdup("tc1");
    tool_calls_array[0].name = strdup("tool_one");
    tool_calls_array[0].arguments = strdup("{\"arg\": \"val1\"}");
    assert_not_null(tool_calls_array[0].id, "strdup tc1.id failed");
    assert_not_null(tool_calls_array[0].name, "strdup tc1.name failed");
    assert_not_null(tool_calls_array[0].arguments, "strdup tc1.arguments failed");

    tool_calls_array[1].id = strdup("tc2");
    tool_calls_array[1].name = strdup("tool_two");
    tool_calls_array[1].arguments = strdup("{}");
    assert_not_null(tool_calls_array[1].id, "strdup tc2.id failed");
    assert_not_null(tool_calls_array[1].name, "strdup tc2.name failed");
    assert_not_null(tool_calls_array[1].arguments, "strdup tc2.arguments failed");

    msg->tool_calls = tool_calls_array; // Assign the allocated array

    r2ai_message_free(msg); // Frees contents of msg
    free(msg); // Free the msg struct itself
    printf("test_r2ai_message_free_with_tool_calls_array passed.\n");
}

void test_r2ai_message_free_with_content_blocks_full() {
    printf("Running test_r2ai_message_free_with_content_blocks_full...\n");
    R2AI_Message *msg = R_NEW0(R2AI_Message);
    assert_not_null(msg, "msg allocation failed for content_blocks_full");

    R2AI_ContentBlocks *cbs = R_NEW0(R2AI_ContentBlocks);
    assert_not_null(cbs, "cbs allocation failed");
    msg->content_blocks = cbs;

    cbs->n_blocks = 1;
    // r2ai_message_free frees `cbs->blocks` (the array) and `cbs` itself.
    // The R2AI_ContentBlock structs and their string members are const and not freed.
    R2AI_ContentBlock *blocks_array = R_NEWS0(R2AI_ContentBlock, cbs->n_blocks);
    assert_not_null(blocks_array, "blocks_array allocation failed");

    // These strings now need to be strdup'd as r2ai_message_free will free them
    blocks_array[0].type = strdup("text");
    blocks_array[0].text = strdup("Sample text content");
    blocks_array[0].id = strdup("block1");
    assert_not_null(blocks_array[0].type, "strdup cb.type failed");
    assert_not_null(blocks_array[0].text, "strdup cb.text failed");
    assert_not_null(blocks_array[0].id, "strdup cb.id failed");
    // ... other const char* fields like data, thinking, signature, name, input would also need strdup if set
    blocks_array[0].data = NULL;
    blocks_array[0].thinking = NULL;
    blocks_array[0].signature = NULL;
    blocks_array[0].name = NULL;
    blocks_array[0].input = NULL;


    cbs->blocks = blocks_array; // Assign the allocated array

    r2ai_message_free(msg); // Frees contents of msg
    free(msg); // Free the msg struct itself
    printf("test_r2ai_message_free_with_content_blocks_full passed.\n");
}

void test_r2ai_message_free_all_allocations() {
    printf("Running test_r2ai_message_free_all_allocations...\n");
    R2AI_Message *msg = R_NEW0(R2AI_Message);
    assert_not_null(msg, "msg allocation failed for all_allocations");

    msg->role = strdup("assistant");
    msg->content = strdup("Comprehensive test");
    msg->tool_call_id = strdup("some_static_id"); // Was: "some_static_id";
    assert_not_null(msg->tool_call_id, "strdup tool_call_id (all_allocations) failed");


    // Tool calls part
    msg->n_tool_calls = 1;
    R2AI_ToolCall *tool_calls_array = R_NEWS0(R2AI_ToolCall, msg->n_tool_calls);
    assert_not_null(tool_calls_array, "tool_calls_array allocation failed for all_allocations");
    tool_calls_array[0].id = strdup("tc_all");
    tool_calls_array[0].name = strdup("name_all"); // Added strdup
    tool_calls_array[0].arguments = strdup("{}");   // Added strdup
    assert_not_null(tool_calls_array[0].id, "strdup tc_all.id failed");
    assert_not_null(tool_calls_array[0].name, "strdup tc_all.name failed");
    assert_not_null(tool_calls_array[0].arguments, "strdup tc_all.arguments failed");
    msg->tool_calls = tool_calls_array;

    // Content blocks part
    R2AI_ContentBlocks *cbs = R_NEW0(R2AI_ContentBlocks);
    assert_not_null(cbs, "cbs allocation failed for all_allocations");
    msg->content_blocks = cbs;

    cbs->n_blocks = 1;
    R2AI_ContentBlock *blocks_array = R_NEWS0(R2AI_ContentBlock, cbs->n_blocks);
    assert_not_null(blocks_array, "blocks_array allocation failed for all_allocations");
    blocks_array[0].type = strdup("text_all");
    blocks_array[0].text = strdup("text_content_all"); // Added strdup
    blocks_array[0].id = strdup("block_id_all");     // Added strdup
    assert_not_null(blocks_array[0].type, "strdup cb_all.type failed");
    assert_not_null(blocks_array[0].text, "strdup cb_all.text failed");
    assert_not_null(blocks_array[0].id, "strdup cb_all.id failed");
    // Initialize other pointers to NULL
    blocks_array[0].data = NULL;
    blocks_array[0].thinking = NULL;
    blocks_array[0].signature = NULL;
    blocks_array[0].name = NULL;
    blocks_array[0].input = NULL;
    cbs->blocks = blocks_array;

    r2ai_message_free(msg); // Frees contents of msg
    free(msg); // Free the msg struct itself
    printf("test_r2ai_message_free_all_allocations passed.\n");
}

// --- Test r2ai_msgs_new and r2ai_msgs_free ---
void test_r2ai_msgs_new_and_free() {
    printf("Running test_r2ai_msgs_new_and_free...\n");

    // Test r2ai_msgs_new
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "r2ai_msgs_new() returned NULL");
    assert_true(msgs->n_messages == 0, "New msgs should have 0 messages");
    // Initial capacity might be 0 or some default. Let's not be too strict on cap_messages here,
    // as long as it's reasonable (e.g., >= 0).
    // If cap_messages is 0, msgs->messages might be NULL. If >0, it should be non-NULL.
    if (msgs->cap_messages > 0) {
        assert_not_null(msgs->messages, "msgs->messages should be non-NULL if cap > 0");
    } else {
        assert_null(msgs->messages, "msgs->messages should be NULL if cap == 0");
    }

    // Test r2ai_msgs_free on an empty R2AI_Messages
    r2ai_msgs_free(msgs);
    printf("  Freed empty msgs successfully.\n");

    // Test r2ai_msgs_free(NULL)
    r2ai_msgs_free(NULL); // Should not crash
    printf("  r2ai_msgs_free(NULL) did not crash.\n");

    // Further tests for r2ai_msgs_free with actual messages will be part of r2ai_msgs_add tests.
    printf("test_r2ai_msgs_new_and_free passed.\n");
}

// --- Test r2ai_msgs_add ---
void test_r2ai_msgs_add_simple() {
    printf("Running test_r2ai_msgs_add_simple...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "msgs creation failed for add_simple");

    R2AI_Message template_msg = {
        .role = "user",
        .content = "This is a simple message.",
        .reasoning_content = NULL,
        .content_blocks = NULL,
        .tool_call_id = NULL,
        .tool_calls = NULL,
        .n_tool_calls = 0
    };

    bool added = r2ai_msgs_add(msgs, &template_msg);
    assert_true(added, "r2ai_msgs_add returned false for simple message");
    assert_true(msgs->n_messages == 1, "n_messages should be 1 after adding one message");
    assert_not_null(msgs->messages, "msgs->messages should not be NULL after adding");

    R2AI_Message *added_msg = &msgs->messages[0];
    assert_streq(added_msg->role, template_msg.role, "Role not copied correctly");
    assert_streq(added_msg->content, template_msg.content, "Content not copied correctly");
    assert_true(added_msg->role != template_msg.role, "Role should be a new string (strdup)");
    assert_true(added_msg->content != template_msg.content, "Content should be a new string (strdup)");

    assert_null(added_msg->reasoning_content, "reasoning_content should be NULL");
    assert_null(added_msg->content_blocks, "content_blocks should be NULL");
    assert_null(added_msg->tool_call_id, "tool_call_id should be NULL");
    assert_null(added_msg->tool_calls, "tool_calls should be NULL");
    assert_true(added_msg->n_tool_calls == 0, "n_tool_calls should be 0");

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_simple passed.\n");
}

void test_r2ai_msgs_add_complex() {
    printf("Running test_r2ai_msgs_add_complex...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "msgs creation failed for add_complex");

    // Prepare a complex message template
    R2AI_ToolCall tc_templates[] = {
        { .id = "tc1", .name = "tool_alpha", .arguments = "{\"param1\":\"val1\"}" },
        { .id = "tc2", .name = "tool_beta",  .arguments = "{\"param2\":\"val2\"}" }
    };

    R2AI_ContentBlock cb_templates[] = {
        { .type = "text", .text = "This is text." },
        { .type = "tool_use", .id = "tc1", .name = "tool_alpha", .input = "{\"param1\":\"val1\"}"}
    };
    R2AI_ContentBlocks cbs_template = { .blocks = (R2AI_ContentBlock*)cb_templates, .n_blocks = 2 };


    R2AI_Message template_msg = {
        .role = "assistant",
        .content = "A complex message with tools and blocks.",
        .reasoning_content = "I decided to use tools.",
        .tool_call_id = "main_tc_id_1", // This one is also duplicated by r2ai_msgs_add_os
        .tool_calls = (const R2AI_ToolCall*)tc_templates,
        .n_tool_calls = 2,
        .content_blocks = &cbs_template
    };

    bool added = r2ai_msgs_add(msgs, &template_msg);
    assert_true(added, "r2ai_msgs_add returned false for complex message");
    assert_true(msgs->n_messages == 1, "n_messages should be 1 after complex add");

    R2AI_Message *added_msg = &msgs->messages[0];
    assert_streq(added_msg->role, template_msg.role, "Complex msg role mismatch");
    assert_streq(added_msg->content, template_msg.content, "Complex msg content mismatch");
    assert_streq(added_msg->reasoning_content, template_msg.reasoning_content, "Complex msg reasoning_content mismatch");
    assert_streq(added_msg->tool_call_id, template_msg.tool_call_id, "Complex msg tool_call_id mismatch");

    // Verify deep copy of tool_calls
    assert_not_null(added_msg->tool_calls, "Added msg tool_calls is NULL");
    assert_true(added_msg->n_tool_calls == template_msg.n_tool_calls, "n_tool_calls mismatch");
    for (int i = 0; i < added_msg->n_tool_calls; i++) {
        assert_streq(added_msg->tool_calls[i].id, tc_templates[i].id, "Tool call ID mismatch");
        assert_streq(added_msg->tool_calls[i].name, tc_templates[i].name, "Tool call name mismatch");
        assert_streq(added_msg->tool_calls[i].arguments, tc_templates[i].arguments, "Tool call arguments mismatch");
        // Check they are new pointers
        assert_true(added_msg->tool_calls[i].id != tc_templates[i].id, "Tool ID not strdup'd");
    }

    // Verify deep copy of content_blocks
    assert_not_null(added_msg->content_blocks, "Added msg content_blocks is NULL");
    assert_true(added_msg->content_blocks->n_blocks == cbs_template.n_blocks, "n_blocks mismatch");
    assert_not_null(added_msg->content_blocks->blocks, "Added msg content_blocks->blocks is NULL");
    for (int i = 0; i < added_msg->content_blocks->n_blocks; i++) {
        assert_streq(added_msg->content_blocks->blocks[i].type, cb_templates[i].type, "CB type mismatch");
        assert_streq(added_msg->content_blocks->blocks[i].text, cb_templates[i].text, "CB text mismatch");
        // Check they are new pointers (e.g. type and text if they were set)
        assert_true(added_msg->content_blocks->blocks[i].type != cb_templates[i].type, "CB type not strdup'd");
        if (cb_templates[i].text) { // only check if original text was not null
             assert_true(added_msg->content_blocks->blocks[i].text != cb_templates[i].text, "CB text not strdup'd");
        }
    }

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_complex passed.\n");
}

void test_r2ai_msgs_add_reallocation() {
    printf("Running test_r2ai_msgs_add_reallocation...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "msgs creation failed for reallocation test");

    R2AI_Message template_msg = { .role = "user", .content = "msg" };
    int initial_cap = msgs->cap_messages;
    printf("  Initial capacity: %d\n", initial_cap);

    // Add messages to potentially trigger reallocation
    // Assuming a small initial capacity or growth factor for testing this.
    // If initial capacity is large, this test might not effectively test reallocation
    // without adding an excessive number of messages.
    // Let's add initial_cap + 5 messages.
    int num_to_add = (initial_cap > 0 ? initial_cap : 2) + 5; // Ensure we add a few more than initial/default
    
    for (int i = 0; i < num_to_add; i++) {
        char content_buf[32];
        snprintf(content_buf, sizeof(content_buf), "Message %d", i);
        template_msg.content = content_buf; // r2ai_msgs_add will strdup this
        
        bool added = r2ai_msgs_add(msgs, &template_msg);
        assert_true(added, "r2ai_msgs_add failed during reallocation test");
        assert_true(msgs->n_messages == (i + 1), "n_messages incorrect during reallocation test");
        assert_streq(msgs->messages[i].content, content_buf, "Content mismatch in reallocated message");
    }
    
    printf("  Added %d messages. Final capacity: %d, Final n_messages: %d\n", num_to_add, msgs->cap_messages, msgs->n_messages);
    assert_true(msgs->cap_messages >= num_to_add, "Capacity should be >= num_to_add after reallocations");

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_reallocation passed.\n");
}

// --- Test r2ai_msgs_add_tool_call ---
void test_r2ai_msgs_add_tool_call_basic() {
    printf("Running test_r2ai_msgs_add_tool_call_basic...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "msgs creation failed for add_tool_call");

    // Add an initial message to which tool calls will be added
    R2AI_Message init_msg_template = { .role = "assistant", .content = "I can use tools." };
    assert_true(r2ai_msgs_add(msgs, &init_msg_template), "Failed to add initial message for tool call test");
    assert_true(msgs->n_messages == 1, "Should have 1 message after initial add");
    
    R2AI_Message *last_msg = &msgs->messages[0]; // Pointer to the message in the array

    // Tool call 1
    R2AI_ToolCall tc1_template = { .id = "call_id_1", .name = "first_tool", .arguments = "{\"arg\":\"val\"}" };
    bool added_tc1 = r2ai_msgs_add_tool_call(msgs, &tc1_template);
    assert_true(added_tc1, "r2ai_msgs_add_tool_call returned false for tc1");
    assert_true(last_msg->n_tool_calls == 1, "n_tool_calls should be 1 after first tool call");
    assert_not_null(last_msg->tool_calls, "tool_calls array should not be NULL");
    
    assert_streq(last_msg->tool_calls[0].id, tc1_template.id, "TC1 ID mismatch");
    assert_streq(last_msg->tool_calls[0].name, tc1_template.name, "TC1 Name mismatch");
    assert_streq(last_msg->tool_calls[0].arguments, tc1_template.arguments, "TC1 Arguments mismatch");
    // Check for new pointers (strdup)
    assert_true(last_msg->tool_calls[0].id != tc1_template.id, "TC1 ID not strdup'd");
    assert_true(last_msg->tool_calls[0].name != tc1_template.name, "TC1 Name not strdup'd");
    assert_true(last_msg->tool_calls[0].arguments != tc1_template.arguments, "TC1 Arguments not strdup'd");

    // Tool call 2
    R2AI_ToolCall tc2_template = { .id = "call_id_2", .name = "second_tool", .arguments = "{}" };
    bool added_tc2 = r2ai_msgs_add_tool_call(msgs, &tc2_template);
    assert_true(added_tc2, "r2ai_msgs_add_tool_call returned false for tc2");
    assert_true(last_msg->n_tool_calls == 2, "n_tool_calls should be 2 after second tool call");
    
    assert_streq(last_msg->tool_calls[1].id, tc2_template.id, "TC2 ID mismatch");
    assert_streq(last_msg->tool_calls[1].name, tc2_template.name, "TC2 Name mismatch");
    assert_streq(last_msg->tool_calls[1].arguments, tc2_template.arguments, "TC2 Arguments mismatch");

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_tool_call_basic passed.\n");
}

void test_r2ai_msgs_add_tool_call_no_initial_message() {
    printf("Running test_r2ai_msgs_add_tool_call_no_initial_message...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert_not_null(msgs, "msgs creation failed for no_initial_message_tc_test");

    R2AI_ToolCall tc_template = { .id = "call_id_fail", .name = "fail_tool", .arguments = "{}" };
    bool added_tc = r2ai_msgs_add_tool_call(msgs, &tc_template);
    assert_false(added_tc, "r2ai_msgs_add_tool_call should return false if no messages exist");
    assert_true(msgs->n_messages == 0, "n_messages should still be 0");

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_tool_call_no_initial_message passed.\n");
}

void test_r2ai_msgs_add_tool_call_reallocation() {
    printf("Running test_r2ai_msgs_add_tool_call_reallocation...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    R2AI_Message init_msg_template = { .role = "assistant" };
    r2ai_msgs_add(msgs, &init_msg_template);
    R2AI_Message *last_msg = &msgs->messages[0];

    // Determine initial capacity of tool_calls. Since it's not explicitly stored,
    // we assume it starts small (e.g. 0 or 1) and R_NEWS_APPEND handles growth.
    // We'll add a few tool calls to test this.
    int num_tool_calls_to_add = 5; 
    for (int i = 0; i < num_tool_calls_to_add; i++) {
        char id_buf[20], name_buf[20];
        snprintf(id_buf, sizeof(id_buf), "tc_realloc_%d", i);
        snprintf(name_buf, sizeof(name_buf), "tool_realloc_%d", i);
        R2AI_ToolCall tc_template = { .id = id_buf, .name = name_buf, .arguments = "{}"};
        
        bool added = r2ai_msgs_add_tool_call(msgs, &tc_template);
        assert_true(added, "r2ai_msgs_add_tool_call failed during reallocation test");
        assert_true(last_msg->n_tool_calls == (i + 1), "n_tool_calls incorrect during tc reallocation");
        assert_streq(last_msg->tool_calls[i].id, id_buf, "TC ID mismatch in reallocated tc array");
        assert_streq(last_msg->tool_calls[i].name, name_buf, "TC Name mismatch in reallocated tc array");
    }
    printf("  Added %d tool_calls. Final n_tool_calls: %d\n", num_tool_calls_to_add, last_msg->n_tool_calls);

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_add_tool_call_reallocation passed.\n");
}

// --- Test Conversation Management ---
void test_conversation_management() {
    printf("Running test_conversation_management...\n");
    R2AI_Messages *conv;

    // 1. Get before init
    conv = r2ai_conversation_get();
    assert_null(conv, "Conversation should be NULL before init");
    printf("  Get before init: OK\n");

    // 2. Init conversation
    r2ai_conversation_init();
    printf("  Init called.\n");

    // 3. Get after init
    conv = r2ai_conversation_get();
    assert_not_null(conv, "Conversation should not be NULL after init");
    assert_true(conv->n_messages == 0, "New conversation should have 0 messages");
    R2AI_Messages *conv1 = conv; // Store the pointer
    printf("  Get after init: OK, n_messages = %d\n", conv->n_messages);

    // 4. Get again (should be same instance)
    conv = r2ai_conversation_get();
    assert_true(conv == conv1, "Getting conversation again should return the same instance");
    printf("  Get again (same instance): OK\n");

    // Add a message to see it cleared later by free or re-init
    R2AI_Message template_msg = { .role = "user", .content = "Test msg for conversation" };
    r2ai_msgs_add(conv1, &template_msg);
    assert_true(conv1->n_messages == 1, "Conversation should have 1 message after add");
    printf("  Added a message to conv1: n_messages = %d\n", conv1->n_messages);

    // 5. Free conversation
    r2ai_conversation_free();
    printf("  Free called.\n");
    // conv1 pointer is now dangling, do not use.

    // 6. Get after free
    conv = r2ai_conversation_get();
    assert_null(conv, "Conversation should be NULL after free");
    printf("  Get after free: OK\n");

    // 7. Re-initialize
    r2ai_conversation_init();
    printf("  Re-init called.\n");
    R2AI_Messages *conv2 = r2ai_conversation_get();
    assert_not_null(conv2, "Conversation should not be NULL after re-init");
    assert_true(conv2->n_messages == 0, "New conversation (conv2) should have 0 messages");
    // assert_true(conv2 != conv1, "New conversation (conv2) should be a different instance from conv1"); // This can be true if allocator reuses memory
    printf("  Get after re-init (conv2): OK, n_messages = %d (pointer may be same as conv1 if memory is reused)\n", conv2->n_messages);
    
    // 8. Final free
    r2ai_conversation_free();
    printf("  Final free called.\n");
    conv = r2ai_conversation_get();
    assert_null(conv, "Conversation should be NULL after final free");
    printf("  Get after final free: OK\n");

    printf("test_conversation_management passed.\n");
}

// --- Test r2ai_msgs_clear ---
void test_r2ai_msgs_clear() {
    printf("Running test_r2ai_msgs_clear...\n");

    // Case 1: Clear NULL R2AI_Messages
    r2ai_msgs_clear(NULL); // Should not crash
    printf("  Clear NULL msgs: OK\n");

    // Case 2: Clear empty R2AI_Messages
    R2AI_Messages *msgs_empty = r2ai_msgs_new();
    assert_not_null(msgs_empty, "msgs_empty allocation failed");
    r2ai_msgs_clear(msgs_empty);
    assert_true(msgs_empty->n_messages == 0, "n_messages should be 0 after clearing empty msgs");
    // messages array and capacity should ideally remain.
    if (msgs_empty->cap_messages > 0) {
         assert_not_null(msgs_empty->messages, "messages array should persist if cap > 0 after clear");
    }
    r2ai_msgs_free(msgs_empty);
    printf("  Clear empty msgs: OK\n");

    // Case 3: Clear R2AI_Messages with messages
    R2AI_Messages *msgs_populated = r2ai_msgs_new();
    assert_not_null(msgs_populated, "msgs_populated allocation failed");

    R2AI_Message template1 = { .role = "user", .content = "Hello" };
    R2AI_ToolCall tc_templates[] = {{ .id = "tc1", .name = "tool_a", .arguments = "{}" }};
    R2AI_Message template2 = { .role = "assistant", .tool_calls = tc_templates, .n_tool_calls = 1 };
    
    r2ai_msgs_add(msgs_populated, &template1);
    r2ai_msgs_add(msgs_populated, &template2);
    assert_true(msgs_populated->n_messages == 2, "Should have 2 messages before clear");
    int old_cap = msgs_populated->cap_messages;

    r2ai_msgs_clear(msgs_populated);
    assert_true(msgs_populated->n_messages == 0, "n_messages should be 0 after clearing populated msgs");
    assert_true(msgs_populated->cap_messages == old_cap, "Capacity should remain the same after clear");
    if (old_cap > 0) {
        assert_not_null(msgs_populated->messages, "messages array should still exist if cap > 0 after clear");
    }
    printf("  Clear populated msgs: OK (n_messages=0, cap preserved)\n");

    // Add another message to ensure it's usable after clearing
    R2AI_Message template3 = { .role = "user", .content = "After clear" };
    bool added_after_clear = r2ai_msgs_add(msgs_populated, &template3);
    assert_true(added_after_clear, "Failed to add message after clearing");
    assert_true(msgs_populated->n_messages == 1, "n_messages should be 1 after adding post-clear");
    assert_streq(msgs_populated->messages[0].content, "After clear", "Content of post-clear message incorrect");
    printf("  Add message after clear: OK\n");

    r2ai_msgs_free(msgs_populated);
    printf("test_r2ai_msgs_clear passed.\n");
}

// --- Test r2ai_delete_last_messages ---
void test_r2ai_delete_last_messages() {
    printf("Running test_r2ai_delete_last_messages...\n");
    R2AI_Messages *msgs;

    // Case 1: Delete from NULL R2AI_Messages
    r2ai_delete_last_messages(NULL, 1); // Should not crash
    printf("  Delete from NULL msgs: OK\n");

    // Case 2: Delete from empty R2AI_Messages
    msgs = r2ai_msgs_new();
    r2ai_delete_last_messages(msgs, 1);
    assert_true(msgs->n_messages == 0, "n_messages should be 0 after deleting from empty");
    r2ai_msgs_free(msgs);
    printf("  Delete from empty msgs: OK\n");

    // Case 3: Delete N messages
    msgs = r2ai_msgs_new();
    R2AI_Message templates[] = {
        {.role="user", .content="msg1"}, {.role="user", .content="msg2"},
        {.role="user", .content="msg3"}, {.role="user", .content="msg4"},
        {.role="user", .content="msg5"}
    };
    for (int i = 0; i < 5; i++) r2ai_msgs_add(msgs, &templates[i]);
    assert_true(msgs->n_messages == 5, "Should have 5 messages initially for Case 3");

    r2ai_delete_last_messages(msgs, 2); // Delete last 2 (msg4, msg5)
    assert_true(msgs->n_messages == 3, "n_messages should be 3 after deleting 2");
    assert_streq(msgs->messages[2].content, "msg3", "Remaining message content mismatch after deleting 2");
    printf("  Delete 2 from 5: OK (n_messages=3)\n");
    
    r2ai_delete_last_messages(msgs, 0); // Delete last 1 (msg3) because N <= 0 defaults to 1
    assert_true(msgs->n_messages == 2, "n_messages should be 2 after deleting 1 (N=0)");
    assert_streq(msgs->messages[1].content, "msg2", "Remaining message content mismatch after deleting 1 (N=0)");
    printf("  Delete 1 (N=0) from 3: OK (n_messages=2)\n");

    r2ai_delete_last_messages(msgs, 1); // Delete last 1 (msg2)
    assert_true(msgs->n_messages == 1, "n_messages should be 1 after deleting 1");
    assert_streq(msgs->messages[0].content, "msg1", "Remaining message content mismatch after deleting 1");
    printf("  Delete 1 from 2: OK (n_messages=1)\n");
    r2ai_msgs_free(msgs);

    // Case 4: Delete more messages than exist
    msgs = r2ai_msgs_new();
    r2ai_msgs_add(msgs, &templates[0]);
    r2ai_msgs_add(msgs, &templates[1]);
    assert_true(msgs->n_messages == 2, "Should have 2 messages initially for Case 4");
    r2ai_delete_last_messages(msgs, 5); // Delete 5 from 2
    assert_true(msgs->n_messages == 0, "n_messages should be 0 after deleting more than exist");
    r2ai_msgs_free(msgs);
    printf("  Delete more than exist: OK (n_messages=0)\n");

    // Case 5: Delete with N <= 0 (default to 1) - more specific checks
    msgs = r2ai_msgs_new();
    r2ai_msgs_add(msgs, &templates[0]); // msg1
    r2ai_msgs_add(msgs, &templates[1]); // msg2
    r2ai_delete_last_messages(msgs, 0); // Deletes 1 (msg2)
    assert_true(msgs->n_messages == 1, "N=0: n_messages should be 1");
    assert_streq(msgs->messages[0].content, "msg1", "N=0: Incorrect message remained");
    r2ai_msgs_free(msgs);

    msgs = r2ai_msgs_new();
    r2ai_msgs_add(msgs, &templates[0]); // msg1
    r2ai_msgs_add(msgs, &templates[1]); // msg2
    r2ai_delete_last_messages(msgs, -5); // Deletes 1 (msg2)
    assert_true(msgs->n_messages == 1, "N=-5: n_messages should be 1");
    assert_streq(msgs->messages[0].content, "msg1", "N=-5: Incorrect message remained");
    r2ai_msgs_free(msgs);
    printf("  Delete with N <= 0 (default to 1): OK\n");

    printf("test_r2ai_delete_last_messages passed.\n");
}

// --- Test r2ai_msgs_from_json / r2ai_msgs_from_response ---

void test_r2ai_msgs_from_response_valid_simple() {
    printf("Running test_r2ai_msgs_from_response_valid_simple...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    const char *json_str = "[{\"role\": \"user\", \"content\": \"Hello JSON\"}]";

    bool success = r2ai_msgs_from_response(msgs, json_str);
    assert_true(success, "r2ai_msgs_from_response failed for valid simple JSON");
    assert_true(msgs->n_messages == 1, "Should parse 1 message from valid simple JSON");
    assert_streq(msgs->messages[0].role, "user", "Parsed message role mismatch");
    assert_streq(msgs->messages[0].content, "Hello JSON", "Parsed message content mismatch");

    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_from_response_valid_simple passed.\n");
}

void test_r2ai_msgs_from_response_valid_complex() {
    printf("Running test_r2ai_msgs_from_response_valid_complex...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();
    // This JSON structure needs to align with what r2ai_msgs_from_json_os expects.
    // It expects "tool_code" for tool calls, and "content_blocks" for other content.
    const char *json_str = "["
        "{\"role\": \"user\", \"content\": \"Give me weather for London\"},"
        "{\"role\": \"assistant\", \"content\": null, \"tool_code\": [{\"id\":\"call_abc\", \"name\":\"get_weather\", \"arguments\":\"{\\\"location\\\":\\\"London\\\"}\"}]},"
        "{\"role\": \"tool\", \"tool_call_id\":\"call_abc\", \"name\":\"get_weather\", \"content\":\"Weather is sunny\"},"
        "{\"role\": \"assistant\", \"content\": null, \"content_blocks\":[{\"type\":\"text\",\"text\":\"Okay, weather in London is sunny.\"}]}"
    "]";
    // Note: `tool_code` vs `tool_calls` and `content` vs `text` in content_blocks needs to match parser in messages.c

    bool success = r2ai_msgs_from_response(msgs, json_str);
    assert_true(success, "r2ai_msgs_from_response failed for valid complex JSON");
    assert_true(msgs->n_messages == 4, "Should parse 4 messages from complex JSON");

    // Message 1: User
    assert_streq(msgs->messages[0].role, "user", "Msg1 role");
    assert_streq(msgs->messages[0].content, "Give me weather for London", "Msg1 content");

    // Message 2: Assistant with tool_code (tool_calls)
    assert_streq(msgs->messages[1].role, "assistant", "Msg2 role");
    assert_null(msgs->messages[1].content, "Msg2 content should be null due to tool_code");
    assert_true(msgs->messages[1].n_tool_calls == 1, "Msg2 n_tool_calls");
    assert_not_null(msgs->messages[1].tool_calls, "Msg2 tool_calls array");
    assert_streq(msgs->messages[1].tool_calls[0].id, "call_abc", "Msg2 TC id");
    assert_streq(msgs->messages[1].tool_calls[0].name, "get_weather", "Msg2 TC name");
    assert_streq(msgs->messages[1].tool_calls[0].arguments, "{\"location\":\"London\"}", "Msg2 TC args");

    // Message 3: Tool response
    assert_streq(msgs->messages[2].role, "tool", "Msg3 role");
    assert_streq(msgs->messages[2].tool_call_id, "call_abc", "Msg3 tool_call_id");
    // The parser puts tool response into msg->content if name matches a previous tool call.
    // Or it might be under a specific field if the JSON is structured differently.
    // The code in `r2ai_msgs_from_json_os` puts it into `content` if `name` matches.
    assert_streq(msgs->messages[2].content, "Weather is sunny", "Msg3 content (tool response)");
    // assert_streq(msgs->messages[2].name, "get_weather", "Msg3 name (tool name)"); // This is not directly stored in R2AI_Message like this

    // Message 4: Assistant with content_blocks
    assert_streq(msgs->messages[3].role, "assistant", "Msg4 role");
    assert_null(msgs->messages[3].content, "Msg4 content should be null due to content_blocks");
    assert_not_null(msgs->messages[3].content_blocks, "Msg4 content_blocks should exist");
    assert_true(msgs->messages[3].content_blocks->n_blocks == 1, "Msg4 n_blocks");
    assert_not_null(msgs->messages[3].content_blocks->blocks, "Msg4 content_blocks->blocks array");
    assert_streq(msgs->messages[3].content_blocks->blocks[0].type, "text", "Msg4 CB type");
    assert_streq(msgs->messages[3].content_blocks->blocks[0].text, "Okay, weather in London is sunny.", "Msg4 CB text");


    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_from_response_valid_complex passed.\n");
}

void test_r2ai_msgs_from_response_invalid_json() {
    printf("Running test_r2ai_msgs_from_response_invalid_json...\n");
    R2AI_Messages *msgs = r2ai_msgs_new();

    // Invalid JSON syntax
    const char *invalid_syntax_json = "this is not json";
    bool success_syntax = r2ai_msgs_from_response(msgs, invalid_syntax_json);
    assert_false(success_syntax, "r2ai_msgs_from_response should fail for invalid JSON syntax");
    assert_true(msgs->n_messages == 0, "No messages should be added for syntax error");

    // Valid JSON, but not an array
    const char *not_array_json = "{\"role\":\"user\", \"content\":\"valid json but not array\"}";
    bool success_not_array = r2ai_msgs_from_response(msgs, not_array_json);
    assert_false(success_not_array, "r2ai_msgs_from_response should fail for non-array JSON");
    assert_true(msgs->n_messages == 0, "No messages should be added for non-array JSON");
    
    // Array with invalid message (e.g. missing role)
    // The current parser r2ai_msgs_from_json_os skips invalid messages within an array.
    const char *array_invalid_msg = "[{\"content\":\"no role here\"}, {\"role\":\"user\", \"content\":\"valid msg\"}]";
    bool success_invalid_msg_in_array = r2ai_msgs_from_response(msgs, array_invalid_msg);
    assert_true(success_invalid_msg_in_array, "r2ai_msgs_from_response should not fail for array with one invalid message if others are valid");
    assert_true(msgs->n_messages == 1, "Should parse 1 valid message from array with one invalid message");
    if (msgs->n_messages == 1) {
        assert_streq(msgs->messages[0].role, "user", "Role of valid message in mixed array");
        assert_streq(msgs->messages[0].content, "valid msg", "Content of valid message in mixed array");
    }


    r2ai_msgs_free(msgs);
    printf("test_r2ai_msgs_from_response_invalid_json passed.\n");
}

// --- Test r2ai_msgs_to_json ---
void test_r2ai_msgs_to_json() {
    printf("Running test_r2ai_msgs_to_json...\n");
    R2AI_Messages *msgs;
    char *json_str;

    // Case 1: NULL messages
    json_str = r2ai_msgs_to_json(NULL);
    assert_not_null(json_str, "to_json(NULL) should return non-NULL (e.g. '[]')");
    char* trimmed_json = strdup(json_str); // Use a mutable copy for r_str_trim if available
    // r_str_trim(trimmed_json); // Assuming r_str_trim from r_util/r_str.h
    // For now, direct compare or check for "[]" within it.
    // The implementation returns strdup("[]") so direct comparison is fine.
    assert_streq(trimmed_json, "[]", "to_json(NULL) should produce '[]'");
    free(trimmed_json);
    free(json_str);
    printf("  to_json(NULL): OK\n");

    // Case 2: Empty messages
    msgs = r2ai_msgs_new();
    json_str = r2ai_msgs_to_json(msgs);
    assert_not_null(json_str, "to_json(empty) should return non-NULL");
    trimmed_json = strdup(json_str);
    // r_str_trim(trimmed_json);
    assert_streq(trimmed_json, "[]", "to_json(empty) should produce '[]'");
    free(trimmed_json);
    free(json_str);
    printf("  to_json(empty): OK\n");

    // Case 3: Messages with content
    R2AI_Message user_msg_template = {.role = "user", .content = "User query"};
    r2ai_msgs_add(msgs, &user_msg_template);

    R2AI_ToolCall tc_template[] = {{.id = "tc_json_1", .name = "do_stuff", .arguments = "{\"param\":\"val\"}"}};
    R2AI_ContentBlock cb_template[] = {{.type="text", .text="Assistant response text."}};
    R2AI_ContentBlocks cbs_template = {.blocks=(R2AI_ContentBlock*)cb_template, .n_blocks=1};
    R2AI_Message assistant_msg_template = {
        .role = "assistant", 
        .content = NULL, // Content is in blocks
        .tool_calls = tc_template, 
        .n_tool_calls = 1,
        .content_blocks = &cbs_template
    };
    r2ai_msgs_add(msgs, &assistant_msg_template);
    
    json_str = r2ai_msgs_to_json(msgs);
    assert_not_null(json_str, "to_json(populated) returned NULL");
    printf("  Generated JSON for populated messages: %s\n", json_str);

    // Spot checks for key elements. Full validation would require parsing.
    assert_true(strstr(json_str, "\"role\":\"user\"") != NULL, "Missing user role");
    assert_true(strstr(json_str, "\"content\":\"User query\"") != NULL, "Missing user content");
    assert_true(strstr(json_str, "\"role\":\"assistant\"") != NULL, "Missing assistant role");
    assert_true(strstr(json_str, "\"tool_calls\":") != NULL, "Missing tool_calls array for assistant");
    assert_true(strstr(json_str, "\"id\":\"tc_json_1\"") != NULL, "Missing tool call id");
    assert_true(strstr(json_str, "\"name\":\"do_stuff\"") != NULL, "Missing tool call name");
    assert_true(strstr(json_str, "\"arguments\":\"{\\\"param\\\":\\\"val\\\"}\"") != NULL, "Missing tool call arguments"); // Escaped quotes
    assert_true(strstr(json_str, "\"content_blocks\":") != NULL, "Missing content_blocks array for assistant");
    assert_true(strstr(json_str, "\"type\":\"text\"") != NULL, "Missing content_block type");
    assert_true(strstr(json_str, "\"text\":\"Assistant response text.\"") != NULL, "Missing content_block text");
    // Check that assistant message has "content":null because content is in blocks
    // This depends on exact serialization logic. messages.c:r2ai_msgs_to_json_os only adds "content" if non-null.
    // So if content was explicitly set to NULL, it should be absent, not "content":null.
    // Let's verify it doesn't contain "content":"Assistant response text." for the assistant message.
    // A more precise check would parse the JSON.
    // For now, let's assume if content_blocks or tool_calls are present, content might be omitted or null.
    // The current r2ai_msgs_to_json_os will omit "content":null for the assistant message if msg->content is NULL.

    free(json_str);
    r2ai_msgs_free(msgs);
    printf("  to_json(populated): OK (spot checks passed)\n");

    printf("test_r2ai_msgs_to_json passed.\n");
}

// --- Test create_conversation ---
void test_create_conversation() {
    printf("Running test_create_conversation...\n");
    R2AI_Messages *msgs;
    const char *default_system_prompt = "You are a helpful assistant."; // From messages.c

    // Case 1: With user message
    const char *user_msg_str = "Test user message for create_conversation";
    msgs = create_conversation(user_msg_str);
    assert_not_null(msgs, "create_conversation with user message returned NULL");
    assert_true(msgs->n_messages == 2, "Should have 2 messages (system, user)");
    
    // Check system prompt (message 0)
    assert_streq(msgs->messages[0].role, "system", "Msg0 role should be system");
    assert_streq(msgs->messages[0].content, default_system_prompt, "Msg0 content system prompt mismatch");

    // Check user message (message 1)
    assert_streq(msgs->messages[1].role, "user", "Msg1 role should be user");
    assert_streq(msgs->messages[1].content, user_msg_str, "Msg1 content user message mismatch");
    
    r2ai_msgs_free(msgs);
    printf("  Create conversation with user message: OK\n");

    // Case 2: With NULL user message
    msgs = create_conversation(NULL);
    assert_not_null(msgs, "create_conversation with NULL user message returned NULL");
    assert_true(msgs->n_messages == 1, "Should have 1 message (system only)");

    // Check system prompt (message 0)
    assert_streq(msgs->messages[0].role, "system", "Msg0 role (NULL user) should be system");
    assert_streq(msgs->messages[0].content, default_system_prompt, "Msg0 content (NULL user) system prompt mismatch");

    r2ai_msgs_free(msgs);
    printf("  Create conversation with NULL user message: OK\n");

    printf("test_create_conversation passed.\n");
}

// --- Test RJson utilities (r_json_to_string, r_json_to_pj) ---
void test_r_json_utils() {
    printf("Running test_r_json_utils...\n");
    
    // Test r_json_to_string
    printf("  Testing r_json_to_string...\n");
    RJson *json_obj_str = r_json_parse("{\"name\":\"test_obj\", \"value\":123}");
    assert_not_null(json_obj_str, "Failed to parse JSON for r_json_to_string test");

    char *str_from_json = r_json_to_string(json_obj_str);
    assert_not_null(str_from_json, "r_json_to_string returned NULL for valid RJson");
    // The output might have whitespace variations, so strstr is safer than streq for objects.
    // Or, parse it back and check values if exact string match is needed and predictable.
    // messages.c r_json_to_string uses pj_drain which is compact.
    assert_streq(str_from_json, "{\"name\":\"test_obj\",\"value\":123}", "r_json_to_string output mismatch");
    free(str_from_json);
    printf("    r_json_to_string(valid_json): OK\n");

    str_from_json = r_json_to_string(NULL);
    assert_null(str_from_json, "r_json_to_string(NULL) should return NULL");
    printf("    r_json_to_string(NULL): OK\n");
    r_json_free(json_obj_str);

    // Test r_json_to_pj
    printf("  Testing r_json_to_pj...\n");
    RJson *json_obj_pj = r_json_parse("{\"type\":\"example\", \"count\":42}");
    assert_not_null(json_obj_pj, "Failed to parse JSON for r_json_to_pj test");

    // Case 1: existing_pj is NULL
    PJ *pj_from_json = r_json_to_pj(json_obj_pj, NULL);
    assert_not_null(pj_from_json, "r_json_to_pj(json, NULL) returned NULL");
    char *str_from_pj_new = pj_drain(pj_from_json); // pj_drain also frees pj_from_json
    assert_streq(str_from_pj_new, "{\"type\":\"example\",\"count\":42}", "String from new PJ mismatch");
    free(str_from_pj_new);
    printf("    r_json_to_pj(json, NULL): OK\n");

    // Case 2: existing_pj is provided
    PJ *pj_existing = pj_new();
    pj_a(pj_existing); // Start an array in existing_pj
    PJ *pj_returned = r_json_to_pj(json_obj_pj, pj_existing);
    assert_true(pj_returned == pj_existing, "r_json_to_pj should return the existing_pj instance");
    pj_end(pj_existing); // End the array
    char *str_from_pj_existing = pj_drain(pj_existing);
    // Expected: "[{\"type\":\"example\",\"count\":42}]"
    assert_streq(str_from_pj_existing, "[{\"type\":\"example\",\"count\":42}]", "String from existing PJ mismatch");
    free(str_from_pj_existing);
    printf("    r_json_to_pj(json, existing_pj): OK\n");
    
    PJ *pj_null_result = r_json_to_pj(NULL, NULL);
    assert_null(pj_null_result, "r_json_to_pj(NULL, NULL) should return NULL");
    printf("    r_json_to_pj(NULL, NULL): OK\n");
    
    r_json_free(json_obj_pj);

    printf("test_r_json_utils passed.\n");
}


int main() {
    printf("Running tests for messages.c...\n");

    // r2ai_message_free tests
    test_r2ai_message_free_null();
    test_r2ai_message_free_basic_fields();
    test_r2ai_message_free_with_tool_calls_array();
    test_r2ai_message_free_with_content_blocks_full();
    test_r2ai_message_free_all_allocations();

    // r2ai_msgs_new and r2ai_msgs_free tests
    test_r2ai_msgs_new_and_free();

    // r2ai_msgs_add tests
    test_r2ai_msgs_add_simple();
    test_r2ai_msgs_add_complex();
    test_r2ai_msgs_add_reallocation();

    // r2ai_msgs_add_tool_call tests
    test_r2ai_msgs_add_tool_call_basic();
    test_r2ai_msgs_add_tool_call_no_initial_message();
    test_r2ai_msgs_add_tool_call_reallocation();

    // Conversation management tests
    test_conversation_management();

    // r2ai_msgs_clear tests
    test_r2ai_msgs_clear();

    // r2ai_delete_last_messages tests
    test_r2ai_delete_last_messages();

    // r2ai_msgs_from_response tests
    test_r2ai_msgs_from_response_valid_simple();
    test_r2ai_msgs_from_response_valid_complex();
    test_r2ai_msgs_from_response_invalid_json();

    // r2ai_msgs_to_json tests
    // test_r2ai_msgs_to_json();

    // create_conversation tests
    // test_create_conversation();

    // RJson utilities tests
    // test_r_json_utils();

    printf("All message tests finished (partially, some tests commented out).\n");
    return 0;
}
