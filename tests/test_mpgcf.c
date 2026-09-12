/*
 * Copyright (C) 2026 mpgcf authors
 *
 * This file is part of mpgcf.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mpgcf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
static int checks;

#define CHECK(cond, msg)						\
	do {								\
		checks++;						\
		if (!(cond)) {						\
			failures++;					\
			fprintf(stderr, "FAIL %s:%d: %s\n",		\
				__FILE__, __LINE__, msg);		\
		}							\
	} while (0)

static void test_basic_parse_and_print(void)
{
	mpgcf_t cf;
	char *s;

	mpgcf_init(cf);
	mpgcf_set_b0_si(cf, 3, 1);
	mpgcf_append_si(cf, 1, 1, 7, 1);
	mpgcf_append_si(cf, 1, 1, 15, 1);
	mpgcf_append_si(cf, 1, 1, 1, 1);
	mpgcf_append_si(cf, 1, 1, 292, 1);

	s = mpgcf_get_bnstr(cf);
	CHECK(s != NULL, "mpgcf_get_bnstr returned NULL");
	if (s) {
		CHECK(strcmp(s, "[ 3 ; (1, 7), (1, 15), (1, 1), (1, 292) ]") == 0,
		      "pi approximation printed incorrectly");
		free(s);
	}

	CHECK(mpgcf_length(cf) == 4, "expected 4 terms");

	mpgcf_clear(cf);
}

static void test_roundtrip_parse_print(void)
{
	static const char *inputs[] = {
		"[ 0 ]",
		"[ -3/2 ]",
		"[ 1 ; (1, 2) ]",
		"[ 3 ; (1, 7), (1, 15), (1, 1), (1, 292) ]",
		"[ -1 ; (2/3, -5/7), (1, 4) ]",
	};
	size_t i;

	for (i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
		mpgcf_t cf;
		char *s;
		int rc;

		mpgcf_init(cf);
		rc = mpgcf_set_bnstr(cf, inputs[i], NULL);
		CHECK(rc == 0, "parse failed for a valid roundtrip input");

		s = mpgcf_get_bnstr(cf);
		CHECK(s != NULL, "get_str returned NULL in roundtrip test");
		if (s) {
			if (strcmp(s, inputs[i]) != 0) {
				fprintf(stderr,
					"  roundtrip mismatch: got '%s' want '%s'\n",
					s, inputs[i]);
				failures++;
			}
			checks++;
			free(s);
		}
		mpgcf_clear(cf);
	}
}

static void test_whitespace_tolerance(void)
{
	mpgcf_t cf;
	int rc;
	mpq_t val, expect;

	mpgcf_init(cf);
	rc = mpgcf_set_bnstr(cf, "[3;(1,7),(1,15)]", NULL);
	CHECK(rc == 0, "compact form without spaces should parse");

	mpq_init(val);
	mpq_init(expect);
	CHECK(mpgcf_get_q(val, cf) == 0, "eval failed on compact form");
	/* 3 + 1/(7 + 1/15) = 3 + 1/(106/15) = 3 + 15/106 = 333/106 */
	mpq_set_str(expect, "333/106", 10);
	mpq_canonicalize(expect);
	CHECK(mpq_equal(val, expect) != 0, "compact-form value mismatch");

	mpq_clear(val);
	mpq_clear(expect);
	mpgcf_clear(cf);
}

static void test_parse_rejects_garbage(void)
{
	mpgcf_t cf;
	static const char *bad_inputs[] = {
		"",
		"[",
		"[ ]",
		"[ 1",
		"[ 1 ; (1,2)",
		"[ 1 ; (1) ]",
		"[ 1 ; (1,2,3) ]",
		"not a fraction",
		"[ 1 ; (a, b) ]",
	};
	size_t i;

	for (i = 0; i < sizeof(bad_inputs) / sizeof(bad_inputs[0]); i++) {
		int rc;

		mpgcf_init(cf);
		rc = mpgcf_set_bnstr(cf, bad_inputs[i], NULL);
		CHECK(rc != 0, "malformed input was incorrectly accepted");
		mpgcf_clear(cf);
	}
}

static void test_eval_simple(void)
{
	mpgcf_t cf;
	mpq_t result, expect;

	/* [ 0 ; (1,1), (1,1), (1,1) ] -> 1/(1+1/(1+1/1)) = 2/3 */
	mpgcf_init(cf);
	mpgcf_set_b0_si(cf, 0, 1);
	mpgcf_append_si(cf, 1, 1, 1, 1);
	mpgcf_append_si(cf, 1, 1, 1, 1);
	mpgcf_append_si(cf, 1, 1, 1, 1);

	mpq_init(result);
	mpq_init(expect);
	CHECK(mpgcf_get_q(result, cf) == 0, "eval failed for simple case");
	mpq_set_str(expect, "2/3", 10);
	mpq_canonicalize(expect);
	CHECK(mpq_equal(result, expect) != 0, "simple eval value mismatch");

	mpq_clear(result);
	mpq_clear(expect);
	mpgcf_clear(cf);
}

static void test_eval_no_terms(void)
{
	mpgcf_t cf;
	mpq_t result;

	mpgcf_init(cf);
	mpgcf_set_b0_si(cf, 5, 3);
	mpq_init(result);
	CHECK(mpgcf_get_q(result, cf) == 0, "eval failed with no terms");
	CHECK(mpq_cmp_si(result, 5, 3) == 0,
	      "eval with no terms should equal b0");
	mpq_clear(result);
	mpgcf_clear(cf);
}

static void test_eval_division_by_zero(void)
{
	mpgcf_t cf;
	mpq_t result;

	/* [ 1 ; (1, 0) ] -> 1 + 1/0, should be detected */
	mpgcf_init(cf);
	mpgcf_set_b0_si(cf, 1, 1);
	mpgcf_append_si(cf, 1, 1, 0, 1);
	mpq_init(result);
	CHECK(mpgcf_get_q(result, cf) != 0,
	      "eval should report failure on division by zero");
	mpq_clear(result);
	mpgcf_clear(cf);
}

static void test_from_mpq_exact(void)
{
	mpq_t value, result;
	mpgcf_t cf;

	/*
	 * 355/113 is a famous close rational approximation of pi;
	 * its simple continued fraction is [3; (1,7),(1,16)] since
	 * 355/113 = 3 + 1/(7 + 1/16). Verify from_mpq recovers it
	 * exactly and that eval inverts it.
	 */
	mpq_init(value);
	mpq_set_str(value, "355/113", 10);
	mpq_canonicalize(value);

	mpgcf_init(cf);
	mpgcf_set_default_max_terms(0);
	mpgcf_set_q(cf, value);

	{
		char *s = mpgcf_get_bnstr(cf);

		CHECK(s != NULL, "from_mpq get_str returned NULL");
		if (s) {
			CHECK(strcmp(s, "[ 3 ; (1, 7), (1, 16) ]") == 0,
			      "355/113 continued fraction expansion mismatch");
			free(s);
		}
	}

	mpq_init(result);
	CHECK(mpgcf_get_q(result, cf) == 0, "eval of from_mpq result failed");
	CHECK(mpq_equal(result, value) != 0,
	      "from_mpq then eval did not round-trip to original value");

	mpq_clear(value);
	mpq_clear(result);
	mpgcf_clear(cf);
}

static void test_from_mpq_max_terms(void)
{
	mpq_t value;
	mpgcf_t cf;

	mpq_init(value);
	mpq_set_str(value, "355/113", 10);
	mpq_canonicalize(value);

	mpgcf_init(cf);

	mpgcf_set_default_max_terms(1);
	mpgcf_set_q(cf, value);
	CHECK(mpgcf_length(cf) == 1, "max_terms=1 should stop after 1 term");

	mpq_clear(value);
	mpgcf_clear(cf);
}

static void test_from_mpq_integer(void)
{
	mpq_t value, result;
	mpgcf_t cf;

	mpq_init(value);
	mpq_set_si(value, 7, 1);

	mpgcf_init(cf);
	mpgcf_set_default_max_terms(0);
	mpgcf_set_q(cf, value);
	CHECK(mpgcf_length(cf) == 0,
	      "integer value should expand with zero terms");

	mpq_init(result);
	CHECK(mpgcf_get_q(result, cf) == 0, "eval of integer expansion failed");
	CHECK(mpq_equal(result, value) != 0, "integer round-trip mismatch");

	mpq_clear(value);
	mpq_clear(result);
	mpgcf_clear(cf);
}

static void test_copy_and_reset(void)
{
	mpgcf_t a, b;
	char *sa, *sb;

	mpgcf_init(a);
	mpgcf_set_bnstr(a, "[ 2 ; (1, 3), (1, 4) ]", NULL);

	mpgcf_init(b);
	mpgcf_set(b, a);

	sa = mpgcf_get_bnstr(a);
	sb = mpgcf_get_bnstr(b);
	CHECK(sa != NULL && sb != NULL && strcmp(sa, sb) == 0,
	      "copied continued fraction should print identically");
	free(sa);
	free(sb);

	mpgcf_reset(a);
	CHECK(mpgcf_length(a) == 0, "reset should clear terms");
	sa = mpgcf_get_bnstr(a);
	CHECK(sa != NULL && strcmp(sa, "[ 0 ]") == 0,
	      "reset continued fraction should print as [ 0 ]");
	free(sa);

	/* b must be unaffected by resetting a (deep copy check) */
	CHECK(mpgcf_length(b) == 2, "independent copy should be unaffected");

	mpgcf_clear(a);
	mpgcf_clear(b);
}

static void test_set_term(void)
{
	mpgcf_t cf;
	mpq_t a, b, got_a, got_b;
	char *s;
	int rc;

	mpgcf_init(cf);
	mpgcf_set_b0_si(cf, 3, 1);
	mpgcf_append_si(cf, 1, 1, 7, 1);
	mpgcf_append_si(cf, 1, 1, 15, 1);

	mpq_init(a);
	mpq_init(b);
	mpq_set_si(a, 2, 1);
	mpq_set_si(b, 9, 1);

	rc = mpgcf_set_term(cf, 1, a, b);
	CHECK(rc == 0, "set_term should succeed for a valid index");

	mpq_init(got_a);
	mpq_init(got_b);
	CHECK(mpgcf_get_term(cf, 1, got_a, got_b) == 0,
	      "get_term should succeed after set_term");
	CHECK(mpq_cmp_si(got_a, 2, 1) == 0, "set_term did not update a_i");
	CHECK(mpq_cmp_si(got_b, 9, 1) == 0, "set_term did not update b_i");

	/* length must be unchanged: set_term overwrites, never grows */
	CHECK(mpgcf_length(cf) == 2, "set_term must not change term count");

	s = mpgcf_get_bnstr(cf);
	CHECK(s != NULL && strcmp(s, "[ 3 ; (1, 7), (2, 9) ]") == 0,
	      "printed form should reflect the overwritten term");
	free(s);

	/* out-of-range index must fail and leave the term list untouched */
	rc = mpgcf_set_term(cf, 5, a, b);
	CHECK(rc != 0, "set_term should fail for an out-of-range index");
	CHECK(mpgcf_length(cf) == 2,
	      "failed set_term must not alter the term count");

	/* the _si convenience wrapper should behave the same way */
	rc = mpgcf_set_term_si(cf, 0, 5, 1, 11, 1);
	CHECK(rc == 0, "set_term_si should succeed for a valid index");
	s = mpgcf_get_bnstr(cf);
	CHECK(s != NULL && strcmp(s, "[ 3 ; (5, 11), (2, 9) ]") == 0,
	      "set_term_si did not update the printed form correctly");
	free(s);

	rc = mpgcf_set_term_si(cf, 9, 1, 1, 1, 1);
	CHECK(rc != 0, "set_term_si should fail for an out-of-range index");

	mpq_clear(a);
	mpq_clear(b);
	mpq_clear(got_a);
	mpq_clear(got_b);
	mpgcf_clear(cf);
}

/*
 * check parsing simple continuous fraction
 */
static void test_basic_parse_and_print_cf(void)
{
	mpgcf_t cf;
	char *s;
	int rc;

	s = "[ 3 ; 7, 15, 1, 292 ]";

	mpgcf_init(cf);
	rc = mpgcf_set_bnstr(cf, s, NULL);
	CHECK(rc == 0, "parse failed for a valid simple continuous fraction");

	s = mpgcf_get_bnstr(cf);
	CHECK(s != NULL, "mpgcf_get_bnstr returned NULL");

	CHECK(mpgcf_length(cf) == 4, "expected 4 terms");

	mpgcf_clear(cf);
}

/*
 * check parsing without first item, ie: b_0
 */
static void test_parse_and_print_no_b0(void)
{
	mpgcf_t cf;
	char *s;
	int rc;

	s = "[ ; 7, 15, 1, 292 ]";

	mpgcf_init(cf);
	rc = mpgcf_set_bnstr(cf, s, NULL);
	CHECK(rc == 0, "parse failed for a continued fraction without first item");

	// XXX check output is equal input
	s = mpgcf_get_bnstr(cf);
	CHECK(s != NULL, "mpgcf_get_bnstr returned NULL");

	mpgcf_clear(cf);
}

/* check negative items */
static void test_parse_and_print_negative(void)
{
	mpgcf_t cf;
	char *s, *ret;
	int rc;

	s = "[ -1; -2, 3, -4, -5 ]";

	mpgcf_init(cf);
	rc = mpgcf_set_bnstr(cf, s, NULL);
	CHECK(rc == 0, "parse failed for a continued fraction with negative item");

	ret = mpgcf_get_bnstr(cf);
	CHECK(s != NULL, "mpgcf_get_bnstr returned NULL");

	printf("%s\n", s);
	printf("%s\n", ret);

	mpgcf_clear(cf);
}

static void test_parse_and_print_fraction(void)
{
	mpgcf_t cf;
	char *s, *ret;
	int rc;

	s = "[ -1/2; -2, (1/2, 3/4), -3/4, (4/5, 6/7) ]";

	mpgcf_init(cf);
	rc = mpgcf_set_bnstr(cf, s, NULL);
	CHECK(rc == 0, "parse failed for a continued fraction with negative item");

	ret = mpgcf_get_bnstr(cf);
	CHECK(s != NULL, "mpgcf_get_bnstr returned NULL");

	printf("%s\n", s);
	printf("%s\n", ret);

	mpgcf_clear(cf);
}

int main(void)
{
	test_basic_parse_and_print();
	test_roundtrip_parse_print();
	test_whitespace_tolerance();
	test_parse_rejects_garbage();
	test_eval_simple();
	test_eval_no_terms();
	test_eval_division_by_zero();
	test_from_mpq_exact();
	test_from_mpq_max_terms();
	test_from_mpq_integer();
	test_copy_and_reset();
	test_set_term();
	test_basic_parse_and_print_cf();
	test_parse_and_print_no_b0();
	test_parse_and_print_negative();
	test_parse_and_print_fraction();

	printf("%d/%d checks passed\n", checks - failures, checks);

	if (failures > 0) {
		fprintf(stderr, "%d check(s) FAILED\n", failures);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
