/*
 * Copyright (C) 2026 mpgcf authors
 *
 * gcf.c
 * This file is part of mpgcf.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mpgcf.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

int main(int argc, char **argv)
{
	mpgcf_t cf;
	mpq_t result;
	const char *input_str;
	const char *end_ptr = NULL;
	char *output_str = NULL;

	if (argc < 2) {
		fprintf(stderr, "libmpgcf %s\n", VERSION);
		fprintf(stderr, "gcf - parse continued fraction and print it in bracket notation, "
			"in-fix notation and as a fraction\n");
		fprintf(stderr, "Copyright (C) 2026 mpgcf authors\n");
		fprintf(stderr, "license: LGPL-3.0-or-later\n");
		fprintf(stderr, "project: https://github.com/mpgcf/mpgcf\n\n");
		fprintf(stderr, "usage: %s \"[ b0 ; (a1,b1), ... ]\" or \"[ b0 ; b1, b2, ... ]\"\n", argv[0]);
		fprintf(stderr, "examples:\n");
		fprintf(stderr, "  %s \"[ 3; (1,7), (10,15), (1,1) ]\"\n", argv[0]);
		fprintf(stderr, "  %s \"[ 4; 7/2, 15, 2/3, 292 ]\"\n", argv[0]);
		fprintf(stderr, "  %s \"[ 5; (1/2,4/7), (3/1,1/5), (3/1,65434/1) ]\"\n", argv[0]);
		return EXIT_FAILURE;
	}

	/*
	if (argc > 1 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
		printf("mpgcf version %s\n", VERSION);
		return 0;
	}
	*/

	input_str = argv[1];
	end_ptr = input_str;

	mpgcf_init(cf);
	mpq_init(result);

	printf("input: %s\n", input_str);

	if (mpgcf_set_bnstr(cf, input_str, &end_ptr) != 0) {
		fprintf(stderr, "error: parsing. end: %s\n", end_ptr);
		mpgcf_clear(cf);
		mpq_clear(result);
		return EXIT_FAILURE;
	}

	output_str = mpgcf_get_bnstr(cf);
	if (output_str) {
		printf("terms: %zu\n", mpgcf_length(cf));
		printf("mpgcf_get_bnstr:\n%s\n", output_str);
		free(output_str);
	} else {
		// XXX error
	}

	output_str = mpgcf_get_instr(cf);
	if (output_str) {
		printf("mpgcf_get_inst:\n%s\n", output_str);
		free(output_str);
	} else {
		// error
	}

	if (mpgcf_get_q(result, cf) != 0) {
		fprintf(stderr, "error: division by zero\n");
		mpgcf_clear(cf);
		mpq_clear(result);
		return EXIT_FAILURE;
	}

	puts("rational:");
	mpq_out_str(stdout, 10, result);
	puts("");

	mpf_set_default_prec(256);
	mpf_t fp;
	mpf_init(fp);
    	mpf_set_q(fp, result);

	puts("irational:");
	mpf_out_str(stdout, 10, 80, fp);
	puts("");

	mpf_clear(fp);
	mpgcf_clear(cf);
	mpq_clear(result);

	return EXIT_SUCCESS;
}

