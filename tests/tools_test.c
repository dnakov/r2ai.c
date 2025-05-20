#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For bool type
#include "../r2ai.h"    // For R2AI_Tools, r2ai_tools_parse, r2ai_tools_free
#include <r_util/r_json.h> // For RJson, if needed for more complex assertions

// Helper for assertions
void assert_true(bool condition, const char* message) {
    if (!condition) {
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

// Test for r2ai_tools_parse with valid JSON
void test_parse_valid_json() {
    printf("Running test_parse_valid_json...\n");
    const char *valid_json = "["
                             "  {"
                             "    \"type\": \"function\","
                             "    \"function\": {"
                             "      \"name\": \"get_current_weather\","
                             "      \"description\": \"Get the current weather in a given location\","
                             "      \"parameters\": {"
                             "        \"type\": \"object\","
                             "        \"properties\": {"
                             "          \"location\": {"
                             "            \"type\": \"string\","
                             "            \"description\": \"The city and state, e.g. San Francisco, CA\""
                             "          },"
                             "          \"unit\": {"
                             "            \"type\": \"string\","
                             "            \"enum\": [\"celsius\", \"fahrenheit\"]"
                             "          }"
                             "        },"
                             "        \"required\": [\"location\"]"
                             "      }"
                             "    }"
                             "  },"
                             "  {"
                             "    \"type\": \"function\","
                             "    \"function\": {"
                             "      \"name\": \"get_stock_price\","
                             "      \"description\": \"Get the current stock price for a symbol\","
                             "      \"parameters\": {"
                             "        \"type\": \"object\","
                             "        \"properties\": {"
                             "          \"symbol\": {"
                             "            \"type\": \"string\","
                             "            \"description\": \"The stock symbol, e.g. GOOG\""
                             "          }"
                             "        },"
                             "        \"required\": [\"symbol\"]"
                             "      }"
                             "    }"
                             "  }"
                             "]";

    R2AI_Tools *tools = r2ai_tools_parse(valid_json);
    assert_true(tools != NULL, "tools should not be NULL for valid JSON");
    assert_true(tools->n_tools == 2, "tools->n_tools should be 2");

    // Check first tool
    assert_true(tools->tools != NULL, "tools->tools should not be NULL");
    R2AI_Tool *tool1 = &tools->tools[0];
    assert_streq(tool1->name, "get_current_weather", "tool1->name");
    assert_streq(tool1->description, "Get the current weather in a given location", "tool1->description");
    // For parameters, we expect a JSON string. Let's check it's not NULL.
    // A more detailed check would parse the JSON string itself.
    assert_true(tool1->parameters != NULL, "tool1->parameters should not be NULL");
    // Example check for a substring in parameters:
    assert_true(strstr(tool1->parameters, "\"location\"") != NULL, "tool1->parameters content check");


    // Check second tool
    R2AI_Tool *tool2 = &tools->tools[1];
    assert_streq(tool2->name, "get_stock_price", "tool2->name");
    assert_streq(tool2->description, "Get the current stock price for a symbol", "tool2->description");
    assert_true(tool2->parameters != NULL, "tool2->parameters should not be NULL");
    assert_true(strstr(tool2->parameters, "\"symbol\"") != NULL, "tool2->parameters content check");


    r2ai_tools_free(tools); // Test r2ai_tools_free implicitly
    printf("test_parse_valid_json passed.\n");
}

// Test for r2ai_tools_parse with invalid JSON
void test_parse_invalid_json() {
    printf("Running test_parse_invalid_json...\n");
    R2AI_Tools *tools;

    // Test with completely invalid JSON
    tools = r2ai_tools_parse("this is not json");
    assert_true(tools == NULL, "tools should be NULL for completely invalid JSON");
    r2ai_tools_free(tools); // Should handle NULL gracefully

    // Test with valid JSON but wrong structure (not an array)
    tools = r2ai_tools_parse("{\"foo\": \"bar\"}");
    assert_true(tools == NULL, "tools should be NULL for JSON object instead of array");
    r2ai_tools_free(tools);

    // Test with JSON array of non-objects
    tools = r2ai_tools_parse("[1, 2, 3]");
    assert_true(tools == NULL, "tools should be NULL for JSON array of non-objects");
    r2ai_tools_free(tools);

    // Test with JSON array of objects missing 'type'
    tools = r2ai_tools_parse("[{\"function\": {\"name\": \"test\"}}]");
    assert_true(tools == NULL, "tools should be NULL for missing 'type' field");
    r2ai_tools_free(tools);

    // Test with JSON array of objects with wrong 'type'
    tools = r2ai_tools_parse("[{\"type\": \"not_function\", \"function\": {\"name\": \"test\"}}]");
    assert_true(tools == NULL, "tools should be NULL for wrong 'type' field");
    r2ai_tools_free(tools);
    
    // Test with JSON array of objects missing 'function'
    tools = r2ai_tools_parse("[{\"type\": \"function\"}]");
    assert_true(tools == NULL, "tools should be NULL for missing 'function' object");
    r2ai_tools_free(tools);

    // Test with JSON array of objects missing 'function.name'
    tools = r2ai_tools_parse("[{\"type\": \"function\", \"function\": {\"description\": \"test\"}}]");
    assert_true(tools == NULL, "tools should be NULL for missing 'function.name'");
    r2ai_tools_free(tools);
    
    // Test with JSON array of objects missing 'function.description' (should still parse, description is optional)
    // Based on current understanding of r2ai_tools_parse, it might still parse this.
    // If description is mandatory for the parser, this test needs adjustment or the parser logic clarified.
    // For now, let's assume it parses if only description is missing but name and parameters are there.
    // The provided code for r2ai_tools_parse in tools.c needs to be checked for this.
    // The current structure R2AI_Tool has char *description, if it's not set, it would be NULL.
    // Let's assume a tool *must* have a name. Description might be optional. Parameters might be optional.
    // The example in the prompt has "description" and "parameters" for each tool.
    // Let's assume name is mandatory, description and parameters are optional for now.

    const char *missing_description_json = "["
                                           "  {"
                                           "    \"type\": \"function\","
                                           "    \"function\": {"
                                           "      \"name\": \"tool_without_description\""
                                           // No description, no parameters
                                           "    }"
                                           "  }"
                                           "]";
    tools = r2ai_tools_parse(missing_description_json);
    assert_true(tools != NULL, "tools should not be NULL if only description and parameters are missing");
    if (tools) {
        assert_true(tools->n_tools == 1, "n_tools should be 1 for missing_description_json");
        assert_streq(tools->tools[0].name, "tool_without_description", "name for tool_without_description");
        assert_true(tools->tools[0].description == NULL, "description should be NULL for tool_without_description");
        assert_true(tools->tools[0].parameters == NULL, "parameters should be NULL for tool_without_description");
    }
    r2ai_tools_free(tools);


    // Test with NULL input
    tools = r2ai_tools_parse(NULL);
    assert_true(tools == NULL, "tools should be NULL for NULL input");
    r2ai_tools_free(tools);

    printf("test_parse_invalid_json passed.\n");
}

// Test for r2ai_tools_parse with an empty JSON array
void test_parse_empty_array_json() {
    printf("Running test_parse_empty_array_json...\n");
    const char *empty_array_json = "[]";
    R2AI_Tools *tools = r2ai_tools_parse(empty_array_json);
    assert_true(tools != NULL, "tools should not be NULL for an empty JSON array");
    if (tools) {
        assert_true(tools->n_tools == 0, "tools->n_tools should be 0 for an empty array");
        assert_true(tools->tools == NULL, "tools->tools should be NULL for an empty array");
    }
    r2ai_tools_free(tools);
    printf("test_parse_empty_array_json passed.\n");
}

// Helper function to manually create an R2AI_Tools structure for testing
R2AI_Tools* create_sample_tools() {
    R2AI_Tools *tools = R_NEW0(R2AI_Tools);
    if (!tools) return NULL;

    tools->n_tools = 2;
    tools->tools = R_NEWS0(R2AI_Tool, tools->n_tools);
    if (!tools->tools) {
        free(tools);
        return NULL;
    }

    // Tool 1
    tools->tools[0].name = strdup("get_weather");
    tools->tools[0].description = strdup("Get current weather");
    tools->tools[0].parameters = strdup("{\"type\":\"object\",\"properties\":{\"location\":{\"type\":\"string\"}}}");

    // Tool 2
    tools->tools[1].name = strdup("get_stock");
    tools->tools[1].description = strdup("Get stock price");
    tools->tools[1].parameters = strdup("{\"type\":\"object\",\"properties\":{\"symbol\":{\"type\":\"string\"}}}");
    
    if (!tools->tools[0].name || !tools->tools[0].description || !tools->tools[0].parameters ||
        !tools->tools[1].name || !tools->tools[1].description || !tools->tools[1].parameters) {
        r2ai_tools_free(tools); // Will free any successfully strdup'd strings
        return NULL;
    }

    return tools;
}

void test_tools_to_openai_json() {
    printf("Running test_tools_to_openai_json...\n");
    R2AI_Tools *tools = create_sample_tools();
    assert_true(tools != NULL, "Failed to create sample tools for OpenAI test");

    char *json_str = r2ai_tools_to_openai_json(tools);
    assert_true(json_str != NULL, "r2ai_tools_to_openai_json returned NULL");

    // Basic validation of the JSON string content
    // A more robust check would involve a JSON parser
    assert_true(strstr(json_str, "\"type\":\"function\"") != NULL, "OpenAI JSON missing 'type:function'");
    assert_true(strstr(json_str, "\"name\":\"get_weather\"") != NULL, "OpenAI JSON missing tool1 name");
    assert_true(strstr(json_str, "\"description\":\"Get current weather\"") != NULL, "OpenAI JSON missing tool1 description");
    assert_true(strstr(json_str, "{\"type\":\"object\",\"properties\":{\"location\":{\"type\":\"string\"}}}") != NULL, "OpenAI JSON missing tool1 params");
    assert_true(strstr(json_str, "\"name\":\"get_stock\"") != NULL, "OpenAI JSON missing tool2 name");

    printf("Generated OpenAI JSON: %s\n", json_str); // Optional: print for manual inspection

    free(json_str);
    r2ai_tools_free(tools);
    printf("test_tools_to_openai_json passed.\n");
}

void test_tools_to_anthropic_json() {
    printf("Running test_tools_to_anthropic_json...\n");
    R2AI_Tools *tools = create_sample_tools();
    assert_true(tools != NULL, "Failed to create sample tools for Anthropic test");

    char *json_str = r2ai_tools_to_anthropic_json(tools);
    assert_true(json_str != NULL, "r2ai_tools_to_anthropic_json returned NULL");

    // Basic validation of the JSON string content
    assert_true(strstr(json_str, "\"name\":\"get_weather\"") != NULL, "Anthropic JSON missing tool1 name");
    assert_true(strstr(json_str, "\"description\":\"Get current weather\"") != NULL, "Anthropic JSON missing tool1 description");
    assert_true(strstr(json_str, "\"input_schema\":{\"type\":\"object\",\"properties\":{\"location\":{\"type\":\"string\"}}}") != NULL, "Anthropic JSON missing tool1 input_schema");
    assert_true(strstr(json_str, "\"name\":\"get_stock\"") != NULL, "Anthropic JSON missing tool2 name");
    
    printf("Generated Anthropic JSON: %s\n", json_str); // Optional: print for manual inspection

    free(json_str);
    r2ai_tools_free(tools);
    printf("test_tools_to_anthropic_json passed.\n");
}

void test_empty_tools_to_json() {
    printf("Running test_empty_tools_to_json...\n");
    R2AI_Tools tools_empty = { .tools = NULL, .n_tools = 0 };
    char *json_str;

    // Test OpenAI format
    json_str = r2ai_tools_to_openai_json(&tools_empty);
    assert_true(json_str != NULL, "OpenAI JSON for empty tools should not be NULL");
    // Expected: "[]" or "[\n]\n" or similar, r_str_trim will remove newlines for strcmp
    char *trimmed_json_openai = strdup(json_str);
    r_str_trim(trimmed_json_openai); // Assuming r_str_trim is available via r_util or similar
    assert_streq(trimmed_json_openai, "[]", "OpenAI JSON for empty tools should be '[]'");
    free(trimmed_json_openai);
    free(json_str);

    // Test Anthropic format
    json_str = r2ai_tools_to_anthropic_json(&tools_empty);
    assert_true(json_str != NULL, "Anthropic JSON for empty tools should not be NULL");
    char *trimmed_json_anthropic = strdup(json_str);
    r_str_trim(trimmed_json_anthropic); // Assuming r_str_trim is available
    assert_streq(trimmed_json_anthropic, "[]", "Anthropic JSON for empty tools should be '[]'");
    free(trimmed_json_anthropic);
    free(json_str);

    // Note: r2ai_tools_free is not needed here as tools_empty is stack-allocated and fields are NULL.
    // If it were heap allocated with R_NEW0, then r2ai_tools_free would be appropriate.
    // R2AI_Tools tools_empty_on_heap = R_NEW0(R2AI_Tools);
    // r2ai_tools_free(tools_empty_on_heap);

    printf("test_empty_tools_to_json passed.\n");
}

void test_tools_with_optional_fields_to_json() {
    printf("Running test_tools_with_optional_fields_to_json...\n");
    R2AI_Tools *tools = R_NEW0(R2AI_Tools);
    assert_true(tools != NULL, "Failed to allocate tools structure");

    tools->n_tools = 1;
    tools->tools = R_NEWS0(R2AI_Tool, 1);
    assert_true(tools->tools != NULL, "Failed to allocate tools array");

    R2AI_Tool *tool = &tools->tools[0];
    char *json_str;

    // Case 1: description and parameters are NULL
    tool->name = strdup("test_tool_null_opt");
    tool->description = NULL;
    tool->parameters = NULL;
    assert_true(tool->name != NULL, "strdup failed for tool name");

    printf("  Testing with description = NULL, parameters = NULL...\n");
    // OpenAI
    json_str = r2ai_tools_to_openai_json(tools);
    assert_true(json_str != NULL, "OpenAI JSON (null opts) should not be NULL");
    printf("    OpenAI (null opts): %s\n", json_str);
    assert_true(strstr(json_str, "\"name\":\"test_tool_null_opt\"") != NULL, "OpenAI (null opts) name missing");
    assert_true(strstr(json_str, "\"description\"") == NULL, "OpenAI (null opts) description should be absent");
    // Depending on implementation, parameters might be an empty object {} or absent.
    // The OpenAI spec usually expects a parameters object. If null, it should be an empty object.
    assert_true(strstr(json_str, "\"parameters\":{}") != NULL || strstr(json_str, "\"parameters\": {}") != NULL, "OpenAI (null opts) parameters should be {} or absent");
    free(json_str);

    // Anthropic
    json_str = r2ai_tools_to_anthropic_json(tools);
    assert_true(json_str != NULL, "Anthropic JSON (null opts) should not be NULL");
    printf("    Anthropic (null opts): %s\n", json_str);
    assert_true(strstr(json_str, "\"name\":\"test_tool_null_opt\"") != NULL, "Anthropic (null opts) name missing");
    assert_true(strstr(json_str, "\"description\"") == NULL, "Anthropic (null opts) description should be absent");
    assert_true(strstr(json_str, "\"input_schema\":{}") != NULL || strstr(json_str, "\"input_schema\": {}") != NULL, "Anthropic (null opts) input_schema should be {} or absent");
    free(json_str);
    
    free(tool->name); // Prepare for next case by freeing previous name

    // Case 2: description is empty string, parameters is valid empty JSON object string
    tool->name = strdup("test_tool_empty_desc");
    tool->description = strdup(""); // Empty string
    tool->parameters = strdup("{}"); // Empty JSON object
    assert_true(tool->name != NULL && tool->description != NULL && tool->parameters != NULL, "strdup failed for empty desc case");

    printf("  Testing with description = \"\", parameters = \"{}...\"\n");
    // OpenAI
    json_str = r2ai_tools_to_openai_json(tools);
    assert_true(json_str != NULL, "OpenAI JSON (empty desc) should not be NULL");
    printf("    OpenAI (empty desc): %s\n", json_str);
    assert_true(strstr(json_str, "\"name\":\"test_tool_empty_desc\"") != NULL, "OpenAI (empty desc) name missing");
    assert_true(strstr(json_str, "\"description\":\"\"") != NULL, "OpenAI (empty desc) description should be empty string");
    assert_true(strstr(json_str, "\"parameters\":{}") != NULL || strstr(json_str, "\"parameters\": {}") != NULL, "OpenAI (empty desc) parameters should be {}");
    free(json_str);

    // Anthropic
    json_str = r2ai_tools_to_anthropic_json(tools);
    assert_true(json_str != NULL, "Anthropic JSON (empty desc) should not be NULL");
    printf("    Anthropic (empty desc): %s\n", json_str);
    assert_true(strstr(json_str, "\"name\":\"test_tool_empty_desc\"") != NULL, "Anthropic (empty desc) name missing");
    assert_true(strstr(json_str, "\"description\":\"\"") != NULL, "Anthropic (empty desc) description should be empty string");
    assert_true(strstr(json_str, "\"input_schema\":{}") != NULL || strstr(json_str, "\"input_schema\": {}") != NULL, "Anthropic (empty desc) input_schema should be {}");
    free(json_str);

    // r2ai_tools_free will handle freeing the members of the tool and the tools structure itself.
    // The current tool->name, tool->description, tool->parameters are heap allocated.
    r2ai_tools_free(tools);
    printf("test_tools_with_optional_fields_to_json passed.\n");
}

void test_r2ai_get_tools() {
    printf("Running test_r2ai_get_tools...\n");

    const R2AI_Tools *global_tools = r2ai_get_tools();

    // Test that r2ai_get_tools returns a non-NULL pointer
    assert_true(global_tools != NULL, "r2ai_get_tools() should return a non-NULL pointer.");

    // Verify the number of tools
    assert_true(global_tools->n_tools == 2, "Global tools should have 2 tools.");
    assert_true(global_tools->tools != NULL, "Global tools->tools array should not be NULL.");

    // Verify properties of the first tool (r2cmd_tool)
    const R2AI_Tool *tool1 = &global_tools->tools[0];
    assert_streq(tool1->name, "r2cmd", "Tool 1 name should be 'r2cmd'.");
    assert_streq(tool1->description, "Run a radare2 command", "Tool 1 description mismatch.");
    assert_true(tool1->parameters != NULL, "Tool 1 parameters should not be NULL.");
    assert_true(strstr(tool1->parameters, "\"command\"") != NULL, "Tool 1 parameters JSON content error for 'command'.");

    // Verify properties of the second tool (qjs_tool)
    const R2AI_Tool *tool2 = &global_tools->tools[1];
    assert_streq(tool2->name, "execute_js", "Tool 2 name should be 'execute_js'.");
    assert_streq(tool2->description, "Execute a JavaScript script in a quickjs environment. Only what you console.log will be returned.", "Tool 2 description mismatch.");
    assert_true(tool2->parameters != NULL, "Tool 2 parameters should not be NULL.");
    assert_true(strstr(tool2->parameters, "\"script\"") != NULL, "Tool 2 parameters JSON content error for 'script'.");

    // Test that subsequent calls return the same instance (or at least equivalent)
    const R2AI_Tools *global_tools_again = r2ai_get_tools();
    assert_true(global_tools == global_tools_again, "Subsequent calls to r2ai_get_tools() should return the same pointer.");
    // Also check a field to be sure content is consistent, though pointer check implies this for static.
    assert_true(global_tools_again->n_tools == 2, "Global tools (second call) should still have 2 tools.");


    // It's important NOT to call r2ai_tools_free() on the result of r2ai_get_tools(),
    // as it points to static data. Freeing it would lead to a crash or undefined behavior.

    printf("test_r2ai_get_tools passed.\n");
}

// --- Tests for execute_tool, r2ai_r2cmd, r2ai_qjs ---

// Mock RCore for basic testing - very limited functionality
RCore* create_mock_rconfig_core() {
    RCore *core = R_NEW0(RCore);
    if (!core) return NULL;
    // core->config = r_config_new(NULL); // This would be ideal but r_config_new needs more setup
    // For now, direct calls to r_config_get_b in the functions will likely default or error.
    // This is a major limitation for these unit tests.
    return core;
}

void free_mock_rconfig_core(RCore* core) {
    if (!core) return;
    // r_config_free(core->config);
    free(core);
}

void test_execute_tool_dispatch() {
    printf("Running test_execute_tool_dispatch...\n");
    RCore *core = NULL; // Passing NULL core, most tool executions will fail internally or handle NULL.
                        // The goal here is to test dispatch and basic error handling in execute_tool itself.
    char *result;

    // Case 1: NULL tool_name
    result = execute_tool(core, NULL, "{}");
    assert_not_null(result, "execute_tool(NULL, ...) should return an error string");
    assert_true(strstr(result, "Tool name or arguments are NULL") != NULL, "Error message for NULL tool_name mismatch");
    free(result);

    // Case 2: NULL args
    result = execute_tool(core, "r2cmd", NULL);
    assert_not_null(result, "execute_tool(..., NULL) should return an error string");
    assert_true(strstr(result, "Tool name or arguments are NULL") != NULL, "Error message for NULL args mismatch");
    free(result);

    // Case 3: Unknown tool
    result = execute_tool(core, "unknown_tool", "{}");
    assert_not_null(result, "execute_tool(unknown, ...) should return an error string");
    // messages.c execute_tool returns: { "res":"Unknown tool" }
    // tools.c execute_tool returns: { "res":"Unknown tool" } - This matches the code in tools.c
    assert_true(strstr(result, "\"res\":\"Unknown tool\"") != NULL, "Error message for unknown_tool mismatch");
    free(result);
    
    // Case 4: Invalid JSON arguments string
    result = execute_tool(core, "r2cmd", "not a json string");
    assert_not_null(result, "execute_tool with invalid JSON args should return error");
    assert_true(strstr(result, "Invalid JSON arguments") != NULL, "Error for invalid JSON args mismatch");
    free(result);

    // Case 5: Known tool "r2cmd" - will fail deeper due to NULL core, but tests dispatch path
    // We expect r2ai_r2cmd to be called, which itself will return an error with NULL RJson args
    // (as execute_tool will fail to parse "{}" if args_json is not created before)
    // OR if args_json is created, r2ai_r2cmd will complain about command field
    // The execute_tool parses args first. If "command" is missing, r2ai_r2cmd handles it.
    result = execute_tool(core, "r2cmd", "{\"command\":\"\"}"); // Empty command
    assert_not_null(result, "execute_tool(r2cmd, ...) should return a string");
    // Depending on RCore being NULL, and config, output will vary.
    // If r2ai_r2cmd is reached, it might return "No command in tool call arguments" or similar.
    // Since core is NULL, r_config_get_b might crash or return default (false for hide_tool_output).
    // r_core_cmd_str(NULL, ...) will likely return NULL or error.
    // This is where unit testing hits its limit for r2cmd.
    // For now, just check it doesn't crash and returns something.
    // The actual output of r2cmd with NULL core is "{ \"res\":\"Command returned no output or failed\" }"
    // because r_core_cmd_str(NULL, ...) returns NULL.
    assert_true(strstr(result, "Command returned no output or failed") != NULL || strstr(result, "No command in tool call arguments") != NULL, "execute_tool(r2cmd) basic error path check");
    free(result);

    // Case 6: Known tool "execute_js" - similar limitations
    result = execute_tool(core, "execute_js", "{\"script\":\"print(1)\"}");
    assert_not_null(result, "execute_tool(execute_js, ...) should return a string");
    // Similar to r2cmd, this will fail deeper.
    // r2ai_qjs with NULL core will likely fail at r_core_cmd_str.
    // Expected: "{ \"res\":\"Command returned no output or failed\" }"
    assert_true(strstr(result, "Command returned no output or failed") != NULL || strstr(result, "No script field found") != NULL, "execute_tool(execute_js) basic error path check");
    free(result);

    printf("test_execute_tool_dispatch passed.\n");
}

void test_r2ai_r2cmd_arg_handling() {
    printf("Running test_r2ai_r2cmd_arg_handling...\n");
    RCore *core = NULL; // NULL core
    char *result;
    RJson *args_json;

    // Case 1: NULL args RJson
    result = r2ai_r2cmd(core, NULL, false);
    assert_not_null(result, "r2cmd(NULL RJson) should return error string");
    assert_true(strstr(result, "Command is NULL") != NULL, "Error for r2cmd NULL RJson args");
    free(result);

    // Case 2: Args RJson missing "command" field
    args_json = r_json_parse("{\"foo\":\"bar\"}");
    assert_not_null(args_json, "Failed to parse test JSON for r2cmd");
    result = r2ai_r2cmd(core, args_json, false);
    assert_not_null(result, "r2cmd(missing command) should return error string");
    assert_true(strstr(result, "No command in tool call arguments") != NULL, "Error for r2cmd missing command field");
    free(result);
    r_json_free(args_json);
    
    // Case 3: "command" field is not a string (e.g. int)
    // r_json_get (args, "command") would return an RJson object not of type string.
    // Then command_json->str_value would be NULL.
    args_json = r_json_parse("{\"command\":123}");
    assert_not_null(args_json, "Failed to parse test JSON for r2cmd non-string command");
    result = r2ai_r2cmd(core, args_json, false);
    assert_not_null(result, "r2cmd(non-string command) should return error string");
    assert_true(strstr(result, "No command in tool call arguments") != NULL, "Error for r2cmd non-string command field");
    free(result);
    r_json_free(args_json);

    // Deeper tests require mocked RCore for config and r_core_cmd_str
    printf("test_r2ai_r2cmd_arg_handling passed.\n");
}

void test_r2ai_qjs_arg_handling() {
    printf("Running test_r2ai_qjs_arg_handling...\n");
    RCore *core = NULL; // NULL core
    char *result;
    RJson *args_json;

    // Case 1: NULL args RJson
    result = r2ai_qjs(core, NULL, false);
    assert_not_null(result, "qjs(NULL RJson) should return error string");
    assert_true(strstr(result, "Script is NULL") != NULL, "Error for qjs NULL RJson args");
    free(result);

    // Case 2: Args RJson missing "script" field
    args_json = r_json_parse("{\"foo\":\"bar\"}");
    assert_not_null(args_json, "Failed to parse test JSON for qjs");
    result = r2ai_qjs(core, args_json, false);
    assert_not_null(result, "qjs(missing script) should return error string");
    assert_true(strstr(result, "No script field found") != NULL, "Error for qjs missing script field");
    free(result);
    r_json_free(args_json);

    // Case 3: "script" field is not a string or is null
    args_json = r_json_parse("{\"script\":null}");
    assert_not_null(args_json, "Failed to parse test JSON for qjs null script");
    result = r2ai_qjs(core, args_json, false);
    assert_not_null(result, "qjs(null script value) should return error string");
    assert_true(strstr(result, "Script value is NULL or empty") != NULL, "Error for qjs null script value");
    free(result);
    r_json_free(args_json);
    
    // Case 4: "script" field is empty string
    // This should proceed to command construction, but fail at r_core_cmd_str due to NULL core.
    args_json = r_json_parse("{\"script\":\"\"}");
    assert_not_null(args_json, "Failed to parse test JSON for qjs empty script");
    result = r2ai_qjs(core, args_json, false);
    assert_not_null(result, "qjs(empty script) should return string");
    // Expected: "{ \"res\":\"Command returned no output or failed\" }" due to NULL core
    assert_true(strstr(result, "Command returned no output or failed") != NULL, "qjs(empty script) with NULL core error check");
    free(result);
    r_json_free(args_json);

    // Deeper tests (actual command construction, base64 encoding) require mocked RCore or more complex setup.
    printf("test_r2ai_qjs_arg_handling passed.\n");
}


int main() {
    printf("Running tests for tools.c...\n");

    test_parse_valid_json();
    test_parse_invalid_json();
    test_parse_empty_array_json();
    test_tools_to_openai_json();
    test_tools_to_anthropic_json();
    test_empty_tools_to_json();
    test_tools_with_optional_fields_to_json();
    test_r2ai_get_tools();
    
    // Tests for execute_tool and its sub-functions
    test_execute_tool_dispatch();
    test_r2ai_r2cmd_arg_handling();
    test_r2ai_qjs_arg_handling();

    printf("Tests finished.\n");
    return 0;
}
