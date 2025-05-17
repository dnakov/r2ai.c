#include <assert.h>
#include "r2ai.h"
#include <r_util/r_json.h>
#include <stdio.h>

static const char *openai_tools_json =
    "[{\"type\":\"function\",\"function\":{\"name\":\"r2cmd\",\"description\":\"Run command\",\"parameters\":{\"type\":\"object\",\"properties\":{\"command\":{\"type\":\"string\"}},\"required\":[\"command\"]}}},"
    "{\"type\":\"function\",\"function\":{\"name\":\"execute_js\",\"description\":\"Exec js\",\"parameters\":{\"type\":\"object\",\"properties\":{\"script\":{\"type\":\"string\"}},\"required\":[\"script\"]}}}]";

static void test_tools_parse_and_convert(void) {
    char *copy = strdup(openai_tools_json);
    R2AI_Tools *tools = r2ai_tools_parse(copy);
    free(copy);
    assert(tools);
    assert(tools->n_tools == 2);
    assert(tools->tools);
    assert(tools->tools[0].name && strcmp(tools->tools[0].name, "r2cmd") == 0);
    assert(tools->tools[1].name && strcmp(tools->tools[1].name, "execute_js") == 0);

    char *json = r2ai_tools_to_openai_json(tools);
    assert(json);
    RJson *rj = r_json_parse(json);
    assert(rj && rj->type == R_JSON_ARRAY);
    r_json_free(rj);
    free(json);

    char *anth = r2ai_tools_to_anthropic_json(tools);
    assert(anth);
    rj = r_json_parse(anth);
    assert(rj && rj->type == R_JSON_ARRAY);
    r_json_free(rj);
    free(anth);

    r2ai_tools_free(tools);
}

static void test_get_tools(void) {
    const R2AI_Tools *tools = r2ai_get_tools();
    assert(tools);
    assert(tools->n_tools == 2);
    assert(tools->tools);
    assert(strcmp(tools->tools[0].name, "r2cmd") == 0);
}

int main(void) {
    test_tools_parse_and_convert();
    test_get_tools();
    printf("All tools tests passed\n");
    return 0;
}
