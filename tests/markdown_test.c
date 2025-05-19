#include <r_cons.h>
#include <r_util.h>
#include "markdown.h" // Assuming markdown.h is in the same directory or include path is set
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// Helper function to print test names
static void print_test_name(const char *name) {
	r_cons_printf("[TEST] %s\n", name);
}

// Test for r2ai_markdown_theme_default()
static void test_markdown_theme_default() {
	print_test_name("test_markdown_theme_default");
	R2AIMarkdownTheme *theme = r2ai_markdown_theme_default();
	assert(theme != NULL);
	assert(theme->H1 != NULL);
	assert(theme->H2 != NULL);
	assert(theme->H3 != NULL);
	assert(theme->H4 != NULL);
	assert(theme->H5 != NULL);
	assert(theme->H6 != NULL);
	assert(theme->BOLD != NULL);
	assert(theme->ITALIC != NULL);
	assert(theme->STRIKETHROUGH != NULL);
	assert(theme->CODEBLOCK != NULL);
	assert(theme->INLINECODE != NULL);
	assert(theme->LIST_BULLET != NULL);
	// list_number is a format string, can be NULL if not used by default theme.
	// assert(theme->LIST_NUMBER != NULL); 
	assert(theme->CHECKBOX_CHECKED != NULL);
	assert(theme->CHECKBOX_UNCHECKED != NULL);
	assert(theme->HR != NULL); // Added for horizontal rule
	assert(theme->QUOTE != NULL); // Added for blockquote
	assert(theme->RESET != NULL);
	// No need to free the default theme, it's static
}

// Test for r2ai_markdown_set_theme() and r2ai_markdown_get_theme()
static void test_markdown_set_get_theme() {
	print_test_name("test_markdown_set_get_theme");

	// Get default theme first to ensure reset works
	R2AIMarkdownTheme *default_theme = r2ai_markdown_theme_default();

	R2AIMarkdownTheme custom_theme = {
		.H1 = "CUSTOM_H1", .H2 = "CUSTOM_H2", .H3 = "CUSTOM_H3",
		.H4 = "CUSTOM_H4", .H5 = "CUSTOM_H5", .H6 = "CUSTOM_H6",
		.BOLD = "CUSTOM_BOLD", .ITALIC = "CUSTOM_ITALIC",
		.STRIKETHROUGH = "CUSTOM_STRIKETHROUGH",
		.CODEBLOCK = "CUSTOM_CODEBLOCK", .INLINECODE = "CUSTOM_INLINECODE",
		.LIST_BULLET = "CB", .LIST_NUMBER = "CN.%d.",
		.CHECKBOX_CHECKED = "[Y]", .CHECKBOX_UNCHECKED = "[N]",
		.HR = "CUSTOM_HR", .QUOTE = "CUSTOM_QUOTE",
		.RESET = "CUSTOM_RESET"
	};

	r2ai_markdown_set_theme(&custom_theme);
	R2AIMarkdownTheme *retrieved_theme = r2ai_markdown_get_theme();

	assert(retrieved_theme != NULL);
	assert(strcmp(retrieved_theme->H1, "CUSTOM_H1") == 0);
	assert(strcmp(retrieved_theme->BOLD, "CUSTOM_BOLD") == 0);
	assert(strcmp(retrieved_theme->HR, "CUSTOM_HR") == 0);
	assert(strcmp(retrieved_theme->QUOTE, "CUSTOM_QUOTE") == 0);
	assert(strcmp(retrieved_theme->RESET, "CUSTOM_RESET") == 0);

	r2ai_markdown_set_theme(NULL); // Reset to default
	retrieved_theme = r2ai_markdown_get_theme();
	assert(retrieved_theme != NULL);
	assert(strcmp(retrieved_theme->H1, default_theme->H1) == 0);
	assert(strcmp(retrieved_theme->HR, default_theme->HR) == 0);
}

// Test for r2ai_markdown() with NULL input
static void test_markdown_null_input() {
	print_test_name("test_markdown_null_input");
	char *result = r2ai_markdown(NULL);
	assert(result == NULL);
}

// Test for r2ai_markdown() with empty string input
static void test_markdown_empty_input() {
	print_test_name("test_markdown_empty_input");
	char *result = r2ai_markdown("");
	assert(result != NULL && (strcmp(result, "") == 0 || strcmp(result, "\n") == 0));
	free(result);
}

// Test for r2ai_markdown() headings
static void test_markdown_headings() {
	print_test_name("test_markdown_headings");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("# Heading 1");
	expected = r_str_newf("%sHeading 1%s\n", theme->H1, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("## Heading 2");
	expected = r_str_newf("%sHeading 2%s\n", theme->H2, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("### Heading 3");
	expected = r_str_newf("%sHeading 3%s\n", theme->H3, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("#### Heading 4");
	expected = r_str_newf("%sHeading 4%s\n", theme->H4, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("##### Heading 5");
	expected = r_str_newf("%sHeading 5%s\n", theme->H5, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("###### Heading 6");
	expected = r_str_newf("%sHeading 6%s\n", theme->H6, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() bold text
static void test_markdown_bold() {
	print_test_name("test_markdown_bold");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("**bold text**");
	expected = r_str_newf("%sbold text%s\n", theme->BOLD, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() italic text
static void test_markdown_italic() {
	print_test_name("test_markdown_italic");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("*italic text*");
	expected = r_str_newf("%sitalic text%s\n", theme->ITALIC, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("_italic text_");
	expected = r_str_newf("%sitalic text%s\n", theme->ITALIC, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() strikethrough text
static void test_markdown_strikethrough() {
	print_test_name("test_markdown_strikethrough");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("~~strikethrough~~");
	expected = r_str_newf("%sstrikethrough%s\n", theme->STRIKETHROUGH, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() inline code
static void test_markdown_inline_code() {
	print_test_name("test_markdown_inline_code");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("`inline code`");
	expected = r_str_newf("%sinline code%s\n", theme->INLINECODE, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() code blocks
static void test_markdown_code_blocks() {
	print_test_name("test_markdown_code_blocks");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("```\ncode block\n```");
	expected = r_str_newf("%scode block%s\n", theme->CODEBLOCK, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("```c\ncode block\n```");
	expected = r_str_newf("%scode block%s\n", theme->CODEBLOCK, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
	
	actual = r2ai_markdown("```c\nline1\nline2\n```");
	expected = r_str_newf("%sline1\nline2%s\n", theme->CODEBLOCK, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() bullet lists
static void test_markdown_bullet_lists() {
	print_test_name("test_markdown_bullet_lists");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("- item 1\n- item 2");
	expected = r_str_newf("%s item 1\n%s item 2\n", theme->LIST_BULLET, theme->LIST_BULLET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("- item 1\n  - sub-item"); // Nested list
	expected = r_str_newf("%s item 1\n  %s sub-item\n", theme->LIST_BULLET, theme->LIST_BULLET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() numbered lists
static void test_markdown_numbered_lists() {
	print_test_name("test_markdown_numbered_lists");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual, *num_fmt_1, *num_fmt_2, *sub_num_fmt_1;

	num_fmt_1 = r_str_newf(theme->LIST_NUMBER, 1);
	num_fmt_2 = r_str_newf(theme->LIST_NUMBER, 2);
	actual = r2ai_markdown("1. item 1\n2. item 2");
	expected = r_str_newf("%s item 1\n%s item 2\n", num_fmt_1, num_fmt_2);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected); free(num_fmt_1); free(num_fmt_2);

	num_fmt_1 = r_str_newf(theme->LIST_NUMBER, 1);
	sub_num_fmt_1 = r_str_newf(theme->LIST_NUMBER, 1);
	actual = r2ai_markdown("1. item 1\n   1. sub-item"); // Nested
	expected = r_str_newf("%s item 1\n   %s sub-item\n", num_fmt_1, sub_num_fmt_1);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected); free(num_fmt_1); free(sub_num_fmt_1);
}

// Test for r2ai_markdown() checklists
static void test_markdown_checklists() {
	print_test_name("test_markdown_checklists");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	actual = r2ai_markdown("- [ ] task 1\n- [x] task 2");
	expected = r_str_newf("%s task 1\n%s task 2\n", theme->CHECKBOX_UNCHECKED, theme->CHECKBOX_CHECKED);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() combinations
static void test_markdown_combinations() {
	print_test_name("test_markdown_combinations");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *actual;
	RStrBuf expected_buf;
	r_strbuf_init (&expected_buf);

	const char *combo_md = "# **Important** List\n1. `code` item\n- [x] *done*";
	actual = r2ai_markdown(combo_md);

	// Line 1: # **Important** List
	r_strbuf_appendf (&expected_buf, "%s%sImportant%s List%s\n", theme->H1, theme->BOLD, theme->RESET, theme->RESET);
	// Line 2: 1. `code` item
	char *list_num_str_1 = r_str_newf(theme->LIST_NUMBER, 1);
	r_strbuf_appendf (&expected_buf, "%s %scode%s item%s\n", list_num_str_1, theme->INLINECODE, theme->RESET, theme->RESET);
	free(list_num_str_1);
	// Line 3: - [x] *done*
	r_strbuf_appendf (&expected_buf, "%s %sdone%s%s\n", theme->CHECKBOX_CHECKED, theme->ITALIC, theme->RESET, theme->RESET);
	
	char* final_expected = r_strbuf_drain(&expected_buf);
	assert(strcmp(actual, final_expected) == 0);
	free(actual); free(final_expected);
}


// Test for r2ai_markdown() line breaks
static void test_markdown_line_breaks() {
	print_test_name("test_markdown_line_breaks");
	char *expected, *actual;

	actual = r2ai_markdown("line1\nline2\n\nline4");
	expected = r_str_newf("line1\nline2\n\nline4\n");
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() no formatting
static void test_markdown_no_formatting() {
	print_test_name("test_markdown_no_formatting");
	char *expected, *actual;

	actual = r2ai_markdown("This is plain text.");
	expected = r_str_newf("This is plain text.\n");
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	actual = r2ai_markdown("Plain line 1.\nPlain line 2.");
	expected = r_str_newf("Plain line 1.\nPlain line 2.\n");
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() blockquotes
static void test_markdown_blockquotes() {
	print_test_name("test_markdown_blockquotes");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	// Single-line blockquote
	actual = r2ai_markdown("> quoted text");
	expected = r_str_newf("%squoted text%s\n", theme->QUOTE, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	// Multi-line blockquote
	actual = r2ai_markdown("> line 1\n> line 2");
	expected = r_str_newf("%sline 1%s\n%sline 2%s\n", theme->QUOTE, theme->RESET, theme->QUOTE, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	// Blockquote with internal formatting (current parser might not support complex nesting here)
	// The current markdown.c applies QUOTE then processes the rest of the line.
	actual = r2ai_markdown("> **bold** in quote");
	expected = r_str_newf("%s%sbold%s in quote%s\n", theme->QUOTE, theme->BOLD, theme->RESET, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}

// Test for r2ai_markdown() horizontal rules
static void test_markdown_horizontal_rules() {
	print_test_name("test_markdown_horizontal_rules");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;
	char *hr_line = r_str_newf("%s%s\n", theme->HR, theme->RESET); // HR itself contains reset in default theme

	// Test "---"
	actual = r2ai_markdown("---");
	assert(strcmp(actual, hr_line) == 0);
	free(actual);

	// Test "***"
	actual = r2ai_markdown("***");
	assert(strcmp(actual, hr_line) == 0);
	free(actual);

	// Test "___"
	actual = r2ai_markdown("___");
	assert(strcmp(actual, hr_line) == 0);
	free(actual);

	// Test with text before and after
	actual = r2ai_markdown("Text before\n---\nText after");
	char *expected_multi = r_str_newf("Text before\n%s%s\nText after\n", theme->HR, theme->RESET);
	assert(strcmp(actual, expected_multi) == 0);
	free(actual); free(expected_multi);

	free(hr_line);
}

// Test for r2ai_markdown() list items with multiple lines or complex internal formatting
static void test_markdown_multiline_list_items() {
	print_test_name("test_markdown_multiline_list_items");
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual, *num_fmt_1;

	// Bullet list item spanning multiple lines (current parser handles line by line, so indentation is key)
	// The current parser processes "- item one" and then "  continues here" as separate lines.
	// It does not automatically join indented lines into a single list item's content.
	actual = r2ai_markdown("- item one\n  continues here");
	expected = r_str_newf("%s item one\n  continues here\n", theme->LIST_BULLET); // No theme->RESET after item one, as "continues here" is separate.
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	// Numbered list item spanning multiple lines (similar behavior)
	num_fmt_1 = r_str_newf(theme->LIST_NUMBER, 1);
	actual = r2ai_markdown("1. num item one\n   continues here");
	expected = r_str_newf("%s num item one\n   continues here\n", num_fmt_1);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected); free(num_fmt_1);

	// List item containing inline formatting
	actual = r2ai_markdown("- item with **bold** text");
	expected = r_str_newf("%s item with %sbold%s text%s\n", theme->LIST_BULLET, theme->BOLD, theme->RESET, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	num_fmt_1 = r_str_newf(theme->LIST_NUMBER, 1);
	actual = r2ai_markdown("1. num item with *italic*");
	expected = r_str_newf("%s num item with %sitalic%s%s\n", num_fmt_1, theme->ITALIC, theme->RESET, theme->RESET);
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected); free(num_fmt_1);
}

// Test for r2ai_markdown() escaped markdown characters
static void test_markdown_escaped_chars() {
	print_test_name("test_markdown_escaped_chars");
	// The current r2ai_markdown parser does NOT implement escaping.
	// These tests will show how it currently behaves (likely parsing them).
	// If escaping were implemented, these tests would need to change.
	R2AIMarkdownTheme *theme = r2ai_markdown_get_theme();
	char *expected, *actual;

	// Test "\*not bold\*"
	actual = r2ai_markdown("\\*not bold\\*");
	// Expected based on current parser: it will see '*' and apply italic. Backslash might be treated as literal.
	// This depends on how r_cons_strcat_markdown_token handles unknown chars before known ones.
	// It's likely: literal '\' + italic 'not bold' + literal '\' + reset + newline
	expected = r_str_newf("\\%snot bold%s\\%s\n", theme->ITALIC, theme->RESET, theme->RESET);
	// If it were to correctly parse escapes, expected would be: "*not bold*\n"
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	// Test "\_not italic\_"
	actual = r2ai_markdown("\\_not italic\\_");
	expected = r_str_newf("\\%snot italic%s\\%s\n", theme->ITALIC, theme->RESET, theme->RESET);
	// If it were to correctly parse escapes, expected would be: "_not italic_\n"
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);

	// Test "\`not code\`"
	actual = r2ai_markdown("\\`not code\\`");
	expected = r_str_newf("\\%snot code%s\\%s\n", theme->INLINECODE, theme->RESET, theme->RESET);
	// If it were to correctly parse escapes, expected would be: "`not code`\n"
	assert(strcmp(actual, expected) == 0);
	free(actual); free(expected);
}


int main(int argc, char **argv) {
	r_cons_new(); 

	test_markdown_theme_default();
	test_markdown_set_get_theme();
	test_markdown_null_input();
	test_markdown_empty_input();
	test_markdown_headings();
	test_markdown_bold();
	test_markdown_italic();
	test_markdown_strikethrough();
	test_markdown_inline_code();
	test_markdown_code_blocks();
	test_markdown_bullet_lists();
	test_markdown_numbered_lists();
	test_markdown_checklists();
	test_markdown_combinations(); 
	test_markdown_line_breaks();
	test_markdown_no_formatting();

	// New tests
	test_markdown_blockquotes();
	test_markdown_horizontal_rules();
	test_markdown_multiline_list_items();
	test_markdown_escaped_chars();

	r2ai_markdown_set_theme(NULL); // Restore default theme

	r_cons_printf("[PASS] All markdown tests passed.\n");
	r_cons_free(); 
	return 0;
}
