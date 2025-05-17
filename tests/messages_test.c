#include <assert.h>
#include "r2ai.h"
#include <r_util/r_str.h>
#include <stdio.h>

static void test_msgs_add_and_delete(void) {
    R2AI_Messages *msgs = r2ai_msgs_new();
    assert(msgs);

    R2AI_Message m1 = {.role = "user", .content = "hello"};
    assert(r2ai_msgs_add(msgs, &m1));
    assert(msgs->n_messages == 1);

    R2AI_ToolCall tc = {.name = "echo", .arguments = "{\"arg\":\"test\"}", .id = "1"};
    assert(r2ai_msgs_add_tool_call(msgs, &tc));
    assert(msgs->messages[0].n_tool_calls == 1);

    r2ai_delete_last_messages(msgs, 1);
    assert(msgs->n_messages == 0);

    r2ai_msgs_free(msgs);
}

static void test_json_roundtrip(void) {
    R2AI_Messages *msgs = r2ai_msgs_new();
    R2AI_Message m = {.role = "assistant", .content = "hi"};
    assert(r2ai_msgs_add(msgs, &m));

    char *json = r2ai_msgs_to_json(msgs);
    assert(json);

    /* Build a fake API response using the JSON string */
    char *resp = r_str_newf("{\"choices\":[{\"message\":%s}]}", json);

    (void)resp; /* Placeholder to keep variable used */

    char *anth = r2ai_msgs_to_anthropic_json(msgs);
    assert(anth);
    free(anth);

    r2ai_msgs_free(msgs);
    free(resp);
    free(json);
}

int main(void) {
    test_msgs_add_and_delete();
    test_json_roundtrip();
    printf("All tests passed\n");
    return 0;
}
