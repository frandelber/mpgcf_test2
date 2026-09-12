/*
 * Copyright (C) 2026 mpgcf authors
 *
 * fpstr2cf
 * This file is part of mpgcf.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <mpgcf.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define PI "3.141592653589793238462643383279502884197"

static void print_usage(const char *progname)
{
	fprintf(stderr, "libmpgcf %s\n", VERSION);
	fprintf(stderr, "mpf2cf - convert floating-point number to continued fraction\n");
	fprintf(stderr, "Copyright (C) 2026 mpgcf authors\n");
	fprintf(stderr, "license: LGPL-3.0-or-later\n");
	fprintf(stderr, "project: https://github.com/frandelber/mpgcf\n\n");
	fprintf(stderr, "usage: %s [OPTIONS] [number]\n", progname);
	fprintf(stderr, "generate continued fraction from a floating-point number.\n\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -t N, --terms N    Try to produce N terms (default: 0 = as many as\n");
	fprintf(stderr, "                     the input's own precision naturally supports).\n");
	fprintf(stderr, "                     Working precision is grown as needed to reach N,\n");
	fprintf(stderr, "                     up to an internal safety cap; if N still can't be\n");
	fprintf(stderr, "                     reached, the best available result is shown and\n");
	fprintf(stderr, "                     marked truncated.\n");
	fprintf(stderr, "  -h, --help         Show this help message\n\n");
	fprintf(stderr, "if no number is provided, Pi (3.141592653589793i...) is used.\n");
	fprintf(stderr, "examples:\n");
	fprintf(stderr, "  %s 3.14159\n", progname);
	fprintf(stderr, "  %s -t 10 2.71828\n", progname);
	fprintf(stderr, "  %s -t 5\n", progname);
	fprintf(stderr, "report bugs to: https://github.com/frandelber/mpgcf/issues\n");
}

int main(int argc, char *argv[])
{
	// mpf_t fvalue;
	mpq_t qvalue;
	mpgcf_t cf;
	int result;
	size_t max_terms = 0;
	const char *num_str = NULL;
	const char *strvalue;
	mp_bitcnt_t prec;
	mp_bitcnt_t used_prec = 0;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--terms") == 0) {
			if (i + 1 < argc) {
				max_terms = (size_t)atoi(argv[++i]);
			} else {
				fprintf(stderr, "error: -t/--terms requires a number\n");
				print_usage(argv[0]);
				return 1;
			}
		} else if (argv[i][0] != '-') {
			num_str = argv[i];
		} else {
			fprintf(stderr, "error: Unknown option '%s'\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}

	strvalue = (num_str == NULL) ? PI : num_str;

	mpq_init(qvalue);
	mpgcf_init(cf);

	mpgcf_set_default_max_terms(max_terms);

	// result = mpgcf_set_q(cf, qvalue);
	result = mpgcf_set_fpstr(cf, strvalue);

	// XXX
	// gmp_printf("input: %.15Ff\n", qvalue);
	// printf("precision: %lu bits\n", (unsigned long)used_prec);

	puts("bracket notation:");
	mpgcf_out_bnstr(stdout, cf);
	puts("");

	puts("in-fix notation:");
	mpgcf_out_instr(stdout, cf);
	puts("");

	// 
	// printf("truncated: %s\n", result == SFT_OK ? "no" : "yes");

	{
		mpq_t eval_result;
		mpq_init(eval_result);
		if (mpgcf_get_q(eval_result, cf) == 0) {
			char *str = mpq_get_str(NULL, 10, eval_result);
			printf("rational: %s\n", str);
			free(str);
		}
		mpq_clear(eval_result);
	}

	mpq_clear(qvalue);
	mpgcf_clear(cf);

	return 0;
}

