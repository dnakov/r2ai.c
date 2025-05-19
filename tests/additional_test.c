#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "r2ai.h"
#include "markdown.h"

static void test_markdown(void) {
    RMarkdownTheme def = r2ai_markdown_theme_default();
    r2ai_markdown_set_theme(&def);
    const RMarkdownTheme *t = r2ai_markdown_get_theme();
    assert(strcmp(t->bold, def.bold) == 0);
    char *out = r2ai_markdown("**bold** `code`");
    assert(out && strstr(out, "bold"));
    free(out);
}

static void test_json_helpers(void) {
    char json_text[] = "{\"foo\":123,\"bar\":[\"x\",true]}";
    RJson *json = r_json_parse(json_text);
    assert(json);
    PJ *pj = r_json_to_pj(json, NULL);
    char *raw = pj_drain(pj);
    assert(raw && strstr(raw, "foo"));
    RJson *json2 = r_json_parse(raw);
    assert(json2);
    char *round = r_json_to_string(json);
    assert(round && strstr(round, "bar"));
    free(round);
    r_json_free(json2);
    free(raw);
    r_json_free(json);
}

static void test_conversation(void) {
    R2AI_Messages *msgs = create_conversation("hi");
    assert(msgs && msgs->n_messages == 1);
    assert(strcmp(msgs->messages[0].role, "user") == 0);
    char *json = r2ai_msgs_to_json(msgs);
    assert(json && strstr(json, "hi"));
    free(json);
    r2ai_msgs_free(msgs);
}

static void test_vdb(void) {
    RVdb *db = r_vdb_new(8);
    assert(db);
    r_vdb_insert(db, "hello world");
    r_vdb_insert(db, "test data");
    RVdbResultSet *rs = r_vdb_query(db, "hello", 1);
    assert(rs && rs->size > 0 && rs->results[0].node);
    r_vdb_result_free(rs);
    r_vdb_free(db);
}

int main(void) {
    test_markdown();
    test_json_helpers();
    test_conversation();
    test_vdb();
    printf("All additional tests passed\n");
    return 0;
}
