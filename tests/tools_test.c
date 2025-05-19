#include <r_core.h>
#include <r_cons.h>
#include <r_util.h> // For r_str_newf, r_file_slurp, etc.
#include <r_util/r_json.h> // For RJson
#include "tools.h"  // SUT
#include <assert.h>
#include <string.h>
#include <stdlib.h> // For free

// Forward declarations for helper functions if any, or static test functions
static RCore* create_test_core(void);
static void destroy_test_core(RCore *core);
static void print_test_name(const char *name);

static void test_r2ai_get_tools(void);
static void test_r2ai_tools_parse(void);
static void test_r2ai_tools_to_openai_json(void);
static void test_r2ai_tools_to_anthropic_json(void);
static void test_r2ai_tools_free(void);
static void test_execute_tool(void);

// Helper to create and configure a minimal RCore for tests
static RCore* create_test_core() {
	RCore *core = r_core_new();
	if (core) {
		r_config_set_b (core->config, "scr.interactive", false);
		r_config_set_b (core->config, "r2ai.auto.ask_to_execute", false);
		r_config_set_b (core->config, "r2ai.auto.hide_tool_output", true);
	}
	return core;
}

static void destroy_test_core(RCore *core) {
	if (core) {
		r_core_free(core);
	}
}

static void print_test_name(const char *name) {
	r_cons_printf("[TEST] %s\n", name);
}

// Test for r2ai_get_tools()
static void test_r2ai_get_tools(void) {
	print_test_name("test_r2ai_get_tools");
	R2AI_Tools *tools = r2ai_get_tools();

	assert(tools != NULL);
	assert(tools->n_tools == 2); // Expected: r2cmd, execute_js

	int r2cmd_found = 0;
	int execute_js_found = 0;
	for (int i = 0; i < tools->n_tools; i++) {
		if (strcmp(tools->tools[i].name, "r2cmd") == 0) {
			r2cmd_found = 1;
			assert(strcmp(tools->tools[i].description, "Execute radare2 commands and get the output back. The output is a string.") == 0);
			assert(tools->tools[i].parameters_json != NULL); // parameters_json should be a string
			// For default tools, parameters_json is generated from a C struct, then parsed to R2AI_Tool_Params
			// Let's check the R2AI_Tool_Params part
			assert(tools->tools[i].parameters != NULL);
			assert(strcmp(tools->tools[i].parameters->type, "object") == 0);
			assert(tools->tools[i].parameters->n_properties == 1);
			assert(strcmp(tools->tools[i].parameters->properties[0].name, "command") == 0);
			assert(strcmp(tools->tools[i].parameters->properties[0].type, "string") == 0);
			assert(tools->tools[i].parameters->properties[0].description != NULL);
			assert(tools->tools[i].parameters->n_required == 1);
			assert(strcmp(tools->tools[i].parameters->required[0], "command") == 0);

		} else if (strcmp(tools->tools[i].name, "execute_js") == 0) {
			execute_js_found = 1;
			assert(strcmp(tools->tools[i].description, "Execute javascript code and get the output back. The output is a string.") == 0);
			assert(tools->tools[i].parameters_json != NULL);
			assert(tools->tools[i].parameters != NULL);
			assert(strcmp(tools->tools[i].parameters->type, "object") == 0);
			assert(tools->tools[i].parameters->n_properties == 1);
			assert(strcmp(tools->tools[i].parameters->properties[0].name, "script") == 0);
			assert(strcmp(tools->tools[i].parameters->properties[0].type, "string") == 0);
			assert(tools->tools[i].parameters->properties[0].description != NULL);
			assert(tools->tools[i].parameters->n_required == 1);
			assert(strcmp(tools->tools[i].parameters->required[0], "script") == 0);
		}
	}
	assert(r2cmd_found);
	assert(execute_js_found);
}

// Test for r2ai_tools_parse()
static void test_r2ai_tools_parse(void) {
	print_test_name("test_r2ai_tools_parse");
	R2AI_Tools *tools;

	const char *valid_json_string_prop = R"([
		{
			"type": "function",
			"function": {
				"name": "get_weather",
				"description": "Get current weather",
				"parameters": {
					"type": "object",
					"properties": {
						"location": { "type": "string", "description": "Location to get weather for" }
					},
					"required": ["location"]
				}
			}
		}
	])";
	tools = r2ai_tools_parse(valid_json_string_prop);
	assert(tools != NULL);
	assert(tools->n_tools == 1);
	assert(strcmp(tools->tools[0].name, "get_weather") == 0);
	assert(tools->tools[0].parameters_json != NULL);
	assert(strstr(tools->tools[0].parameters_json, "\"type\":\"string\"") != NULL);
	// Also check the parsed R2AI_Tool_Params
	assert(tools->tools[0].parameters != NULL);
	assert(strcmp(tools->tools[0].parameters->properties[0].type, "string") == 0);
	r2ai_tools_free(tools);

	// Test diverse parameter types
	const char *json_diverse_params = R"([
		{
			"type": "function", "function": { "name": "type_tester", "description": "Test diverse types",
				"parameters": {
					"type": "object",
					"properties": {
						"p_int": { "type": "integer", "description": "An integer" },
						"p_bool": { "type": "boolean", "description": "A boolean" },
						"p_num": { "type": "number", "description": "A number" },
						"p_arr": { "type": "array", "description": "An array", "items": { "type": "string" } },
						"p_obj": { "type": "object", "description": "An object", 
								   "properties": { "nested_str": { "type": "string" } } }
					}, "required": ["p_int", "p_bool"]
				}
			}
		}
	])";
	tools = r2ai_tools_parse(json_diverse_params);
	assert(tools != NULL);
	assert(tools->n_tools == 1);
	assert(strcmp(tools->tools[0].name, "type_tester") == 0);
	assert(tools->tools[0].parameters_json != NULL);
	// Check raw parameters_json string
	assert(strstr(tools->tools[0].parameters_json, "\"p_int\":{\"type\":\"integer\"") != NULL);
	assert(strstr(tools->tools[0].parameters_json, "\"p_bool\":{\"type\":\"boolean\"") != NULL);
	assert(strstr(tools->tools[0].parameters_json, "\"p_num\":{\"type\":\"number\"") != NULL);
	assert(strstr(tools->tools[0].parameters_json, "\"p_arr\":{\"type\":\"array\",\"description\":\"An array\",\"items\":{\"type\":\"string\"}}") != NULL);
	assert(strstr(tools->tools[0].parameters_json, "\"p_obj\":{\"type\":\"object\",\"description\":\"An object\",\"properties\":{\"nested_str\":{\"type\":\"string\"}}}") != NULL);
	
	// Check parsed R2AI_Tool_Params
	R2AI_Tool_Params *params = tools->tools[0].parameters;
	assert(params != NULL);
	assert(params->n_properties == 5);
	assert(params->n_required == 2);
	for (int i = 0; i < params->n_properties; i++) {
		const char *pname = params->properties[i].name;
		const char *ptype = params->properties[i].type;
		if (strcmp(pname, "p_int") == 0) assert(strcmp(ptype, "integer") == 0);
		else if (strcmp(pname, "p_bool") == 0) assert(strcmp(ptype, "boolean") == 0);
		else if (strcmp(pname, "p_num") == 0) assert(strcmp(ptype, "number") == 0);
		else if (strcmp(pname, "p_arr") == 0) assert(strcmp(ptype, "array") == 0);
		else if (strcmp(pname, "p_obj") == 0) assert(strcmp(ptype, "object") == 0);
	}
	r2ai_tools_free(tools);


	// ... (rest of the original test_r2ai_tools_parse cases)
	const char *valid_json_multi = R"([
		{
			"type": "function",
			"function": {
				"name": "tool1", "description": "desc1",
				"parameters": {"type": "object", "properties": {"p1": {"type": "integer"}}, "required": ["p1"]}
			}
		},
		{
			"type": "function",
			"function": {
				"name": "tool2", "description": "desc2",
				"parameters": {"type": "object", "properties": {"p2": {"type": "boolean"}}}
			}
		}
	])";
	tools = r2ai_tools_parse(valid_json_multi);
	assert(tools != NULL);
	assert(tools->n_tools == 2);
	r2ai_tools_free(tools);

	tools = r2ai_tools_parse("[]");
	assert(tools != NULL && tools->n_tools == 0);
	r2ai_tools_free(tools);

	tools = r2ai_tools_parse("[{\"name\": \"tool1\""); // Invalid
	assert(tools == NULL);

	tools = r2ai_tools_parse("{\"name\": \"tool1\"}"); // Not an array
	assert(tools == NULL);
}

// Test for r2ai_tools_to_openai_json()
static void test_r2ai_tools_to_openai_json(void) {
	print_test_name("test_r2ai_tools_to_openai_json");
	char *json_str;

	R2AI_Tools tools_container = {0}; // Use a container to call r2ai_tools_free easily
	tools_container.n_tools = 1;
	tools_container.tools = (R2AI_Tool *)calloc(1, sizeof(R2AI_Tool));
	R2AI_Tool *tool = &tools_container.tools[0];

	tool->name = strdup("tool_A_openai");
	tool->description = strdup("Description for tool A openai");
	// Manually set parameters_json with diverse types
	tool->parameters_json = strdup(R"({
		"type": "object",
		"properties": {
			"p_str": { "type": "string" },
			"p_int": { "type": "integer" },
			"p_bool": { "type": "boolean" },
			"p_num": { "type": "number" },
			"p_arr": { "type": "array", "items": { "type": "string" } },
			"p_obj": { "type": "object", "properties": { "n": { "type": "string" } } }
		},
		"required": ["p_str"]
	})");
	// r2ai_tools_to_openai_json currently does not re-parse parameters_json if it exists.
	// It directly uses it. If parameters_json is NULL, it would try to build from R2AI_Tool_Params.
	// For this test, we are testing the direct use of parameters_json.

	json_str = r2ai_tools_to_openai_json(&tools_container);
	assert(json_str != NULL);
	// r_cons_printf("OpenAI JSON (Diverse Params):\n%s\n", json_str);
	assert(strstr(json_str, "\"name\":\"tool_A_openai\"") != NULL);
	assert(strstr(json_str, "\"description\":\"Description for tool A openai\"") != NULL);
	// The parameters part should be exactly what we set in tool->parameters_json
	assert(strstr(json_str, "\"parameters\":{") != NULL); // Start of parameters object
	assert(strstr(json_str, "\"p_str\":{\"type\":\"string\"}") != NULL);
	assert(strstr(json_str, "\"p_int\":{\"type\":\"integer\"}") != NULL);
	assert(strstr(json_str, "\"p_bool\":{\"type\":\"boolean\"}") != NULL);
	assert(strstr(json_str, "\"p_num\":{\"type\":\"number\"}") != NULL);
	assert(strstr(json_str, "\"p_arr\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}") != NULL);
	assert(strstr(json_str, "\"p_obj\":{\"type\":\"object\",\"properties\":{\"n\":{\"type\":\"string\"}}}") != NULL);
	assert(strstr(json_str, "\"required\":[\"p_str\"]") != NULL);
	
	free(json_str);
	r2ai_tools_free(&tools_container); // This will free tool_A_openai and its parameters_json

	// Test with a tool that has R2AI_Tool_Params populated but parameters_json is NULL
	// This will test the serialization from R2AI_Tool_Params struct
	tools_container.n_tools = 1;
	tools_container.tools = (R2AI_Tool *)calloc(1, sizeof(R2AI_Tool));
	tool = &tools_container.tools[0];
	tool->name = strdup("tool_B_params_struct");
	tool->description = strdup("Tool B from struct");
	tool->parameters_json = NULL; // Ensure this is NULL
	tool->parameters = (R2AI_Tool_Params *)calloc(1, sizeof(R2AI_Tool_Params));
	assert(tool->parameters);
	tool->parameters->type = strdup("object");
	tool->parameters->n_properties = 2;
	tool->parameters->properties = (R2AI_Tool_Param_Property *)calloc(2, sizeof(R2AI_Tool_Param_Property));
	assert(tool->parameters->properties);
	tool->parameters->properties[0].name = strdup("prop_s");
	tool->parameters->properties[0].type = strdup("string");
	tool->parameters->properties[0].description = strdup("A string prop");
	tool->parameters->properties[1].name = strdup("prop_i");
	tool->parameters->properties[1].type = strdup("integer");
	tool->parameters->properties[1].description = strdup("An integer prop");
	tool->parameters->n_required = 1;
	tool->parameters->required = (char **)calloc(1, sizeof(char*));
	tool->parameters->required[0] = strdup("prop_s");

	json_str = r2ai_tools_to_openai_json(&tools_container);
	assert(json_str != NULL);
	// r_cons_printf("OpenAI JSON (From Struct):\n%s\n", json_str);
	assert(strstr(json_str, "\"name\":\"tool_B_params_struct\"") != NULL);
	assert(strstr(json_str, "\"parameters\":{\"type\":\"object\",\"properties\":{\"prop_s\":{\"type\":\"string\",\"description\":\"A string prop\"},\"prop_i\":{\"type\":\"integer\",\"description\":\"An integer prop\"}},\"required\":[\"prop_s\"]}") != NULL);
	free(json_str);
	r2ai_tools_free(&tools_container); // Frees tool_B_params_struct and its R2AI_Tool_Params

	// ... (rest of original test_r2ai_tools_to_openai_json cases for empty tools etc.)
	R2AI_Tools empty_tools = {0};
	json_str = r2ai_tools_to_openai_json(&empty_tools);
	assert(json_str != NULL && strcmp(json_str, "[]") == 0);
	free(json_str);
}

// Test for r2ai_tools_to_anthropic_json()
static void test_r2ai_tools_to_anthropic_json(void) {
	print_test_name("test_r2ai_tools_to_anthropic_json");
	char *json_str;

	R2AI_Tools tools_container = {0};
	tools_container.n_tools = 1;
	tools_container.tools = (R2AI_Tool *)calloc(1, sizeof(R2AI_Tool));
	R2AI_Tool *tool = &tools_container.tools[0];

	tool->name = strdup("tool_A_anthropic");
	tool->description = strdup("Description for tool A anthropic");
	// Manually set parameters_json with diverse types for Anthropic's "input_schema"
	tool->parameters_json = strdup(R"({
		"type": "object",
		"properties": {
			"p_str_anth": { "type": "string" },
			"p_int_anth": { "type": "integer" },
			"p_bool_anth": { "type": "boolean" }
		},
		"required": ["p_str_anth", "p_int_anth"]
	})");
	// Similar to OpenAI, if parameters_json is set, it's used directly for input_schema.

	json_str = r2ai_tools_to_anthropic_json(&tools_container);
	assert(json_str != NULL);
	// r_cons_printf("Anthropic JSON (Diverse Params):\n%s\n", json_str);
	assert(strstr(json_str, "\"name\":\"tool_A_anthropic\"") != NULL);
	assert(strstr(json_str, "\"description\":\"Description for tool A anthropic\"") != NULL);
	// The input_schema part should be exactly what we set in tool->parameters_json
	assert(strstr(json_str, "\"input_schema\":{") != NULL);
	assert(strstr(json_str, "\"p_str_anth\":{\"type\":\"string\"}") != NULL);
	assert(strstr(json_str, "\"p_int_anth\":{\"type\":\"integer\"}") != NULL);
	assert(strstr(json_str, "\"p_bool_anth\":{\"type\":\"boolean\"}") != NULL);
	assert(strstr(json_str, "\"required\":[\"p_str_anth\",\"p_int_anth\"]") != NULL);

	free(json_str);
	r2ai_tools_free(&tools_container);

	// Test with a tool that has R2AI_Tool_Params populated but parameters_json is NULL
	tools_container.n_tools = 1;
	tools_container.tools = (R2AI_Tool *)calloc(1, sizeof(R2AI_Tool));
	tool = &tools_container.tools[0];
	tool->name = strdup("tool_B_anth_struct");
	tool->description = strdup("Tool B Anthropic from struct");
	tool->parameters_json = NULL;
	tool->parameters = (R2AI_Tool_Params *)calloc(1, sizeof(R2AI_Tool_Params));
	assert(tool->parameters);
	tool->parameters->type = strdup("object");
	tool->parameters->n_properties = 1;
	tool->parameters->properties = (R2AI_Tool_Param_Property *)calloc(1, sizeof(R2AI_Tool_Param_Property));
	assert(tool->parameters->properties);
	tool->parameters->properties[0].name = strdup("prop_bool_anth");
	tool->parameters->properties[0].type = strdup("boolean");
	tool->parameters->properties[0].description = strdup("A boolean prop for Anthropic");
	// No required fields for this test case
	tool->parameters->n_required = 0;
	tool->parameters->required = NULL;

	json_str = r2ai_tools_to_anthropic_json(&tools_container);
	assert(json_str != NULL);
	// r_cons_printf("Anthropic JSON (From Struct):\n%s\n", json_str);
	assert(strstr(json_str, "\"name\":\"tool_B_anth_struct\"") != NULL);
	assert(strstr(json_str, "\"input_schema\":{\"type\":\"object\",\"properties\":{\"prop_bool_anth\":{\"type\":\"boolean\",\"description\":\"A boolean prop for Anthropic\"}}") != NULL);
	// Check that "required" is not present or is an empty array if not specified
	assert(strstr(json_str, "\"required\"") == NULL || strstr(json_str, "\"required\":[]") != NULL);

	free(json_str);
	r2ai_tools_free(&tools_container);

	// ... (rest of original test_r2ai_tools_to_anthropic_json cases for empty tools etc.)
	R2AI_Tools empty_tools_anth = {0};
	json_str = r2ai_tools_to_anthropic_json(&empty_tools_anth);
	assert(json_str != NULL && strcmp(json_str, "[]") == 0);
	free(json_str);
}


// Test for r2ai_tools_free()
static void test_r2ai_tools_free(void) {
	print_test_name("test_r2ai_tools_free");

	R2AI_Tools *tools = (R2AI_Tools *)calloc(1, sizeof(R2AI_Tools));
	assert(tools != NULL);
	tools->n_tools = 2;
	tools->tools = (R2AI_Tool *)calloc(2, sizeof(R2AI_Tool));
	assert(tools->tools != NULL);

	tools->tools[0].name = strdup("tool0_name");
	tools->tools[0].description = strdup("tool0_desc");
	tools->tools[0].parameters_json = strdup("{\"type\":\"object\"}"); // Added parameters_json
	tools->tools[0].parameters = (R2AI_Tool_Params *)calloc(1, sizeof(R2AI_Tool_Params)); // Also test freeing this
	assert(tools->tools[0].parameters != NULL);
	tools->tools[0].parameters->type = strdup("object");
	tools->tools[0].parameters->n_properties = 1;
	tools->tools[0].parameters->properties = (R2AI_Tool_Param_Property *)calloc(1, sizeof(R2AI_Tool_Param_Property));
	assert(tools->tools[0].parameters->properties != NULL);
	tools->tools[0].parameters->properties[0].name = strdup("param0");
	tools->tools[0].parameters->properties[0].type = strdup("string");

	tools->tools[1].name = strdup("tool1_name");
	tools->tools[1].description = NULL;
	tools->tools[1].parameters_json = NULL;
	tools->tools[1].parameters = NULL;


	r2ai_tools_free(tools);

	const char *json_for_free_test = R"([
		{
			"type": "function",
			"function": {
				"name": "test_free", "description": "testing free",
				"parameters": {"type": "object", "properties": {"p1": {"type": "string"}}, "required": ["p1"]}
			}
		}
	])";
	tools = r2ai_tools_parse(json_for_free_test);
	assert(tools != NULL);
	// Parsed tools will have both parameters_json and parameters struct populated
	assert(tools->tools[0].parameters_json != NULL);
	assert(tools->tools[0].parameters != NULL);
	r2ai_tools_free(tools);

	r2ai_tools_free(NULL);
}

// Test for execute_tool()
static void test_execute_tool(void) {
	print_test_name("test_execute_tool");
	RCore *core = create_test_core();
	if (!core) {
		r_cons_printf("Failed to create RCore for test_execute_tool\n");
		return;
	}
	char *res;

	res = execute_tool(core, "r2cmd", "{\"command\":\"?V\"}");
	assert(res != NULL);
	assert(strstr(res, R2_VERSION) != NULL || strstr(res, "radare2") != NULL); 
	free(res);

	res = execute_tool(core, "r2cmd", "{\"command\":\"?E hello\"}");
	assert(res != NULL && strcmp(res, "hello\n") == 0);
	free(res);
	
	res = execute_tool(core, "r2cmd", "{\"command\":\"invalidcommandthatdoesnotexist\"}");
	assert(res != NULL); // Output might be empty or error from r2
	free(res);

	res = execute_tool(core, "r2cmd", "{\"command\":?V}"); 
	assert(res != NULL && strstr(res, "Invalid JSON arguments") != NULL);
	free(res);

	res = execute_tool(core, "r2cmd", "{\"cmd\":\"?V\"}"); 
	assert(res != NULL && strstr(res, "Missing 'command' field in arguments") != NULL);
	free(res);

	res = execute_tool(core, "execute_js", "{\"script\":\"console.log('hello from js')\"}");
	assert(res != NULL);
	if (strstr(res, "qjs: command not found") == NULL && strstr(res, "Failed to execute script") == NULL) {
		assert(strcmp(res, "hello from js\n") == 0);
	} else {
		r_cons_printf("[WARN] qjs not found or script execution failed, skipping execute_js content check.\n");
	}
	free(res);

	res = execute_tool(core, "execute_js", "{\"script\":\"console.log('hello'); throw new Error('test error');\"}");
	assert(res != NULL);
	if (strstr(res, "qjs: command not found") == NULL && strstr(res, "Failed to execute script") == NULL) {
		assert(strstr(res, "Error: test error") != NULL || strstr(res, "Exception") != NULL); 
	}
	free(res);

	res = execute_tool(core, "execute_js", "{\"script\":console.log('hi')}");
	assert(res != NULL && strstr(res, "Invalid JSON arguments") != NULL);
	free(res);

	res = execute_tool(core, "execute_js", "{\"code\":\"console.log('hi')\"}");
	assert(res != NULL && strstr(res, "Missing 'script' field in arguments") != NULL);
	free(res);

	res = execute_tool(core, "unknown_tool", "{\"arg\":\"val\"}");
	assert(res != NULL && strcmp(res, "{ \"res\":\"Unknown tool\" }") == 0);
	free(res);

	res = execute_tool(core, "r2cmd", NULL);
	assert(res != NULL && strstr(res, "Missing JSON arguments") != NULL);
	free(res);

	destroy_test_core(core);
}


int main(int argc, char **argv) {
	r_cons_new();

	test_r2ai_get_tools();
	test_r2ai_tools_parse();
	test_r2ai_tools_to_openai_json();
	test_r2ai_tools_to_anthropic_json();
	test_r2ai_tools_free();
	test_execute_tool();

	r_cons_printf("[PASS] All tools tests completed.\n");
	r_cons_free();
	return 0;
}
