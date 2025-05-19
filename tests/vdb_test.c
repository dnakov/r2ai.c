#include "r_vdb.h"      // SUT
#include "r_util.h"     // RList, r_list_free, etc.
#include "r_common.h"   // for VDBDIM definition
#include <assert.h>
#include <string.h>
#include <stdio.h>      // For printf
#include <stdlib.h>     // For calloc, free, abort
#include <math.h>       // For fabsf, sqrtf

// If VDBDIM is not defined via r_common.h or a direct include from r2ai.c context,
// define it here for the test. It should match the one used in vdb_embed.inc.c
#ifndef VDBDIM
#define VDBDIM 16 // Default dimension if not provided by includes
#endif
#ifndef R2AI_MAX_WORD_SIZE
#define R2AI_MAX_WORD_SIZE 32 // From vdb_embed.inc.c
#endif


static const float epsilon = 0.0001f;

// Helper to print test names
static void print_test_name(const char *name) {
	printf("[TEST] %s\n", name);
}

// Helper to create a simple embedding vector for testing query_embedding
static float* create_sample_embedding(int dimension, float fill_value) {
	float *vec = (float*)calloc(dimension, sizeof(float));
	if (!vec) return NULL;
	for (int i = 0; i < dimension; i++) {
		vec[i] = fill_value * (i + 1); // Simple varying values
	}
	return vec;
}

// Test for r_vdb_new() and r_vdb_free()
static void test_vdb_creation_and_freeing() {
	print_test_name("test_vdb_creation_and_freeing");

	RVdb *db = r_vdb_new(VDBDIM);
	assert(db != NULL);
	assert(db->dimension == VDBDIM);
	assert(db->root == NULL);
	assert(db->size == 0);
	assert(db->tokens != NULL); 
	assert(r_list_length(db->tokens) == 0);

	r_vdb_free(db);
	r_vdb_free(NULL); // Should not crash

	RVdb *db1 = r_vdb_new(VDBDIM);
	assert(db1 != NULL);
	RVdb *db2 = r_vdb_new(VDBDIM + 1); 
	assert(db2 != NULL);
	assert(db2->dimension == VDBDIM + 1);
	r_vdb_free(db1);
	r_vdb_free(db2);
}

// Test for r_vdb_insert() basic functionality
static void test_vdb_insert_basic() {
	print_test_name("test_vdb_insert_basic");
	RVdb *db = r_vdb_new(VDBDIM);
	assert(db != NULL);

	r_vdb_insert(db, "Hello world", NULL); 
	assert(db->size == 1);
	assert(db->root != NULL);

	r_vdb_insert(db, "Another document", NULL);
	assert(db->size == 2);

	r_vdb_insert(db, "Hello world", NULL); // Duplicate
	assert(db->size == 3); 

	r_vdb_insert(db, "", NULL); // Empty string
	assert(db->size == 4); 

	r_vdb_insert(db, NULL, NULL);
	assert(db->size == 4); 

	r_vdb_free(db);
}

// Test for r_vdb_query() and r_vdb_query_embedding()
static void test_vdb_query() {
	print_test_name("test_vdb_query");
	RVdb *db = r_vdb_new(VDBDIM);
	assert(db != NULL);

	RVdbResultSet *rs_empty = r_vdb_query(db, "query", 3);
	assert(rs_empty != NULL);
	assert(rs_empty->size == 0);
	r_vdb_result_free(rs_empty);

	float *empty_query_vec = create_sample_embedding(VDBDIM, 0.0f);
	rs_empty = r_vdb_query_embedding(db, empty_query_vec, 3);
	assert(rs_empty != NULL);
	assert(rs_empty->size == 0);
	r_vdb_result_free(rs_empty);
	free(empty_query_vec);

	const char *text1 = "This is a test document."; 
	const char *text2 = "Another example for testing.";
	const char *text3 = "Radare2 is a cool tool.";  
	const char *text4 = "Yet another example."; 

	r_vdb_insert(db, text1, NULL);
	r_vdb_insert(db, text2, NULL);
	r_vdb_insert(db, text3, NULL);
	r_vdb_insert(db, text4, NULL);
	assert(db->size == 4);

	RVdbResultSet *rs1 = r_vdb_query(db, text3, 3);
	assert(rs1 != NULL);
	assert(rs1->size > 0); 
	if (rs1->size > 0) {
		assert(strcmp(rs1->results[0].node->text, text3) == 0);
		assert(fabsf(rs1->results[0].dist_sq) < epsilon); 
	}
	r_vdb_result_free(rs1);

	RVdbResultSet *rs2 = r_vdb_query(db, "An example to test", 3);
	assert(rs2 != NULL);
	assert(rs2->size > 0);
	if (rs2->size > 0) {
		bool found_text2 = false;
		bool found_text4 = false;
		for (size_t i = 0; i < rs2->size; i++) {
			if (strcmp(rs2->results[i].node->text, text2) == 0) found_text2 = true;
			if (strcmp(rs2->results[i].node->text, text4) == 0) found_text4 = true;
		}
		assert(found_text2 || found_text4); 
	}
	r_vdb_result_free(rs2);

	RVdbResultSet *rs3 = r_vdb_query(db, "Quantum physics lecture", 3);
	assert(rs3 != NULL);
	for (size_t i = 0; i < rs3->size; i++) {
		assert(rs3->results[i].dist_sq >= 0.0f);
	}
	r_vdb_result_free(rs3);

	RVdbResultSet *rs_k1 = r_vdb_query(db, text1, 1);
	assert(rs_k1 != NULL);
	assert(rs_k1->size == 1 || (rs_k1->size == 0 && db->size == 0)); 
	r_vdb_result_free(rs_k1);

	RVdbResultSet *rs_k_all = r_vdb_query(db, text1, db->size);
	assert(rs_k_all != NULL);
	assert(rs_k_all->size == db->size);
	r_vdb_result_free(rs_k_all);
	
	RVdbResultSet *rs_k_more = r_vdb_query(db, text1, db->size + 5); 
	assert(rs_k_more != NULL);
	assert(rs_k_more->size == db->size);
	r_vdb_result_free(rs_k_more);

	RVdbResultSet *rs_empty_q = r_vdb_query(db, "", 3);
	assert(rs_empty_q != NULL);
	assert(rs_empty_q->size <= db->size);
	r_vdb_result_free(rs_empty_q);

	RVdbResultSet *rs_null_q = r_vdb_query(db, NULL, 3);
	assert(rs_null_q != NULL);
	assert(rs_null_q->size == 0); 
	r_vdb_result_free(rs_null_q);

	float *dummy_vec = create_sample_embedding(VDBDIM, 0.1f);
	assert(dummy_vec != NULL);
	RVdbResultSet *rs_qe1 = r_vdb_query_embedding(db, dummy_vec, 3);
	assert(rs_qe1 != NULL);
	assert(rs_qe1->size <= db->size);
	for (size_t i = 0; i < rs_qe1->size; i++) {
		assert(rs_qe1->results[i].dist_sq >= 0.0f);
	}
	r_vdb_result_free(rs_qe1);
	free(dummy_vec);

	float *zero_vec = (float*)calloc(VDBDIM, sizeof(float));
	assert(zero_vec != NULL);
	RVdbResultSet *rs_qe_zero = r_vdb_query_embedding(db, zero_vec, 3);
	assert(rs_qe_zero != NULL);
	r_vdb_result_free(rs_qe_zero);
	free(zero_vec);
	
	RVdbResultSet *rs_qe_null = r_vdb_query_embedding(db, NULL, 3);
	assert(rs_qe_null != NULL);
	assert(rs_qe_null->size == 0);
	r_vdb_result_free(rs_qe_null);

	r_vdb_free(db);
}

// Test for RVdbResultSet management
static void test_vdb_result_free() {
	print_test_name("test_vdb_result_free");
	RVdb *db = r_vdb_new(VDBDIM);
	r_vdb_insert(db, "data", NULL);
	RVdbResultSet *rs = r_vdb_query(db, "data", 1);
	assert(rs != NULL);
	r_vdb_result_free(rs);

	r_vdb_result_free(NULL); 

	r_vdb_free(db);
}

// Test specific TF-IDF behaviors by observing query results
static void test_vdb_tfidf_effects() {
	print_test_name("test_vdb_tfidf_effects");
	RVdb *db = r_vdb_new(VDBDIM);

	const char *text_stopwords_only = "this is a a this the"; 
	const char *text_unique_word = "uniqueXword document";
	const char *text_common_word = "document common";
	const char *text_repeated_unique = "uniqueXword uniqueXword uniqueXword";

	r_vdb_insert(db, text_stopwords_only, NULL); 
	r_vdb_insert(db, text_unique_word, NULL);    
	r_vdb_insert(db, text_common_word, NULL);    
	r_vdb_insert(db, text_repeated_unique, NULL); 

	RVdbResultSet *rs = r_vdb_query(db, "uniqueXword", 4);
	assert(rs != NULL && rs->size > 0);
	if (rs->size >= 2) {
		bool found_b = false, found_d = false;
		if (strcmp(rs->results[0].node->text, text_unique_word) == 0 || strcmp(rs->results[1].node->text, text_unique_word) == 0) {
			found_b = true;
		}
		if (strcmp(rs->results[0].node->text, text_repeated_unique) == 0 || strcmp(rs->results[1].node->text, text_repeated_unique) == 0) {
			found_d = true;
		}
		assert(found_b && found_d);
        float dist_b = -1.0f, dist_d = -1.0f;
        for(size_t i=0; i < rs->size; ++i) {
            if (strcmp(rs->results[i].node->text, text_unique_word) == 0) dist_b = rs->results[i].dist_sq;
            if (strcmp(rs->results[i].node->text, text_repeated_unique) == 0) dist_d = rs->results[i].dist_sq;
        }
        assert(dist_d >=0 && dist_b >=0); 
        assert(dist_d <= dist_b + epsilon);
	}
	r_vdb_result_free(rs);

	rs = r_vdb_query(db, "this a", 1);
	assert(rs != NULL && rs->size > 0);
	assert(strcmp(rs->results[0].node->text, text_stopwords_only) == 0);
	assert(fabsf(rs->results[0].dist_sq) < epsilon);
	r_vdb_result_free(rs);

	r_vdb_free(db);
}

// Test compute_embedding() behavior with special text inputs
static void test_vdb_special_texts() {
	print_test_name("test_vdb_special_texts");
	RVdb *db = r_vdb_new(VDBDIM);
	assert(db != NULL);

	// 1. Text with only punctuation and special characters
	const char *text_punct_only = "!@#$%^&*()_+[]{};':\",./<>?`~";
	r_vdb_insert(db, text_punct_only, NULL); // Should not crash
	assert(db->size == 1); // Should insert, likely resulting in a zero or near-zero vector

	// Query for it
	RVdbResultSet *rs_punct = r_vdb_query(db, text_punct_only, 1);
	assert(rs_punct != NULL);
	assert(rs_punct->size == 1); // Should find itself
	if (rs_punct->size == 1) {
		assert(strcmp(rs_punct->results[0].node->text, text_punct_only) == 0);
		// Distance should be near zero as it's an exact match of its (likely zero) embedding
		assert(fabsf(rs_punct->results[0].dist_sq) < epsilon);
	}
	r_vdb_result_free(rs_punct);

	// 2. Text with words exceeding R2AI_MAX_WORD_SIZE
	char long_word_buf[R2AI_MAX_WORD_SIZE * 2 + 1];
	for (int i = 0; i < R2AI_MAX_WORD_SIZE * 2; i++) {
		long_word_buf[i] = 'a';
	}
	long_word_buf[R2AI_MAX_WORD_SIZE * 2] = '\0';
	
	char text_with_long_word[R2AI_MAX_WORD_SIZE * 2 + 20];
	snprintf(text_with_long_word, sizeof(text_with_long_word), "prefix %s suffix", long_word_buf);

	r_vdb_insert(db, text_with_long_word, NULL); // Should not crash
	assert(db->size == 2); // Should insert, word might be truncated by str_to_list

	// Query for it
	RVdbResultSet *rs_long_word = r_vdb_query(db, text_with_long_word, 1);
	assert(rs_long_word != NULL);
	assert(rs_long_word->size > 0); // Should find itself
	if (rs_long_word->size > 0) {
		// The stored text is complete, but the embedding is based on truncated words.
		// Exact match query will use truncated words for query embedding too.
		assert(strcmp(rs_long_word->results[0].node->text, text_with_long_word) == 0);
		assert(fabsf(rs_long_word->results[0].dist_sq) < epsilon);
	}
	r_vdb_result_free(rs_long_word);

	// Query for just the long word (or its truncated part)
	// The `str_to_list` function in `vdb_embed.inc.c` truncates words to R2AI_MAX_WORD_SIZE-1.
	char truncated_long_word[R2AI_MAX_WORD_SIZE];
	strncpy(truncated_long_word, long_word_buf, R2AI_MAX_WORD_SIZE -1);
	truncated_long_word[R2AI_MAX_WORD_SIZE -1] = '\0';

	RVdbResultSet *rs_query_truncated = r_vdb_query(db, truncated_long_word, 1);
	assert(rs_query_truncated != NULL);
	assert(rs_query_truncated->size > 0);
	// Expect the document containing the long word to be the top result
	if (rs_query_truncated->size > 0) {
		assert(strcmp(rs_query_truncated->results[0].node->text, text_with_long_word) == 0);
	}
	r_vdb_result_free(rs_query_truncated);

	// 3. Text that is a mix of normal words and only punctuation
	const char *text_mixed_punct = "normal word !@# another !@#$";
	r_vdb_insert(db, text_mixed_punct, NULL);
	assert(db->size == 3);

	RVdbResultSet *rs_mixed = r_vdb_query(db, text_mixed_punct, 1);
	assert(rs_mixed != NULL);
	assert(rs_mixed->size > 0);
	if (rs_mixed->size > 0) {
		assert(strcmp(rs_mixed->results[0].node->text, text_mixed_punct) == 0);
		assert(fabsf(rs_mixed->results[0].dist_sq) < epsilon);
	}
	r_vdb_result_free(rs_mixed);

	// Check interaction with text_punct_only (Node 1)
	// Querying for "normal word" should find text_mixed_punct, not text_punct_only
	RVdbResultSet *rs_normal_q = r_vdb_query(db, "normal word", 2);
	assert(rs_normal_q != NULL);
	assert(rs_normal_q->size > 0);
	bool found_mixed = false;
	bool found_punct_only_unexpectedly = false;
	for (size_t i = 0; i < rs_normal_q->size; i++) {
		if (strcmp(rs_normal_q->results[i].node->text, text_mixed_punct) == 0) {
			found_mixed = true;
		}
		if (strcmp(rs_normal_q->results[i].node->text, text_punct_only) == 0) {
			found_punct_only_unexpectedly = true;
		}
	}
	assert(found_mixed);
	// The text_punct_only might be found if "normal word" query also results in near-zero embedding,
	// but it should be much further than text_mixed_punct.
	// For this test, simply ensuring found_mixed is enough. If rs_normal_q->size is 1, it must be text_mixed_punct.
	if (rs_normal_q->size == 1) {
		assert(strcmp(rs_normal_q->results[0].node->text, text_mixed_punct) == 0);
	} else if (rs_normal_q->size > 1) {
		// If both are found, text_mixed_punct should be closer (smaller distance)
		if (found_mixed && found_punct_only_unexpectedly) {
			float dist_mixed = -1.0f, dist_punct = -1.0f;
			for (size_t i = 0; i < rs_normal_q->size; i++) {
				if (strcmp(rs_normal_q->results[i].node->text, text_mixed_punct) == 0) dist_mixed = rs_normal_q->results[i].dist_sq;
				if (strcmp(rs_normal_q->results[i].node->text, text_punct_only) == 0) dist_punct = rs_normal_q->results[i].dist_sq;
			}
			assert(dist_mixed < dist_punct || fabsf(dist_mixed - dist_punct) < epsilon);
		}
	}
	r_vdb_result_free(rs_normal_q);

	r_vdb_free(db);
}


int main(int argc, char **argv) {
	test_vdb_creation_and_freeing();
	test_vdb_insert_basic();
	test_vdb_query();
	test_vdb_result_free();
	test_vdb_tfidf_effects();
	test_vdb_special_texts(); // Added new test function call

	printf("[PASS] All vdb tests completed.\n");
	return 0;
}
