#include <assert.h>
#include <string.h>
#include "r2ai.h"
#include <stdio.h>

static void test_parse_and_convert(void) {
    const char *json = "[{'type':'function','function':{'name':'echo','description':'Echo','parameters':{'type':'object','properties':{'text':{'type':'string'}},'required':['text']}}}]";
    /* Replace single quotes with double quotes for valid JSON */
    char *j = strdup(json);
    for (char *p = j; *p; p++) {
        if (*p == '\'') *p = '"';
    }

    R2AI_Tools *tools = r2ai_tools_parse(j);
    assert(tools);
    assert(tools->n_tools == 1);
    assert(tools->tools[0].name && strcmp(tools->tools[0].name, "echo") == 0);

    char *oj = r2ai_tools_to_openai_json(tools);
    assert(oj && strstr(oj, "\"name\":\"echo\""));

    char *aj = r2ai_tools_to_anthropic_json(tools);
    assert(aj && strstr(aj, "\"name\":\"echo\""));

    r2ai_tools_free(tools);
    free(j);
    free(oj);
    free(aj);
}

static void test_invalid_json(void) {
    R2AI_Tools *tools = r2ai_tools_parse("{invalid}");
    assert(!tools);
}

static void test_get_global(void) {
    const R2AI_Tools *tools = r2ai_get_tools();
    assert(tools);
    assert(tools->n_tools >= 1);
}

int main(void) {
    test_parse_and_convert();
    test_invalid_json();
    test_get_global();
    printf("All tests passed\n");
    return 0;
}
