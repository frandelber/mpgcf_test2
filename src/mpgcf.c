/*
 * Copyright (C) 2026 mpgcf authors
 *
 * This file is part of mpgcf.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "mpgcf.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* maximum number of terms */
static size_t mpgcf_default_max_terms = MPGCF_DEFAULT_MAX_TERMS;

/* precision */
static mp_bitcnt_t mpgcf_default_f_precision = MPGCF_DEFAULT_F_PRECISION;

void mpgcf_init(mpgcf_t cf)
{
	mpq_init(cf->gcf_b0);
	cf->gcf_a = NULL;
	cf->gcf_b = NULL;
	cf->gcf_size = 0;
	cf->gcf_alloc = 0;
}

void mpgcf_clear(mpgcf_t cf)
{
	size_t i;

	mpq_clear(cf->gcf_b0);
	for (i = 0; i < cf->gcf_size; i++) {
		mpq_clear(cf->gcf_a[i]);
		mpq_clear(cf->gcf_b[i]);
	}
	free(cf->gcf_a);
	free(cf->gcf_b);
	cf->gcf_a = NULL;
	cf->gcf_b = NULL;
	cf->gcf_size = 0;
	cf->gcf_alloc = 0;
}

void mpgcf_reset(mpgcf_t cf)
{
	mpgcf_clear(cf);
	mpgcf_init(cf);
}

size_t mpgcf_get_default_max_terms(void)
{
	return mpgcf_default_max_terms;
}

void mpgcf_set_default_max_terms(size_t max_terms)
{
	if (max_terms == 0)
		mpgcf_default_max_terms = MPGCF_DEFAULT_MAX_TERMS;
	else
		mpgcf_default_max_terms = max_terms;
}

mp_bitcnt_t mpgcf_get_default_f_precision(void)
{
	return mpgcf_default_f_precision;
}

void mpgcf_set_default_f_precision(mp_bitcnt_t prec)
{
	mpgcf_default_f_precision = prec;
}

static void mpgcf_reserve(mpgcf_t cf, size_t min_capacity)
{
	size_t new_alloc;
	mpq_t *new_a, *new_b;

	if (cf->gcf_alloc >= min_capacity)
		return;

	new_alloc = cf->gcf_alloc == 0 ? 4 : cf->gcf_alloc * 2;
	if (new_alloc < min_capacity)
		new_alloc = min_capacity;

	new_a = realloc(cf->gcf_a, new_alloc * sizeof(mpq_t));
	if (!new_a)
		abort();
	new_b = realloc(cf->gcf_b, new_alloc * sizeof(mpq_t));
	if (!new_b)
		abort();

	cf->gcf_a = new_a;
	cf->gcf_b = new_b;
	cf->gcf_alloc = new_alloc;
}

void mpgcf_set(mpgcf_t dest, const mpgcf_t src)
{
	size_t i;

	mpgcf_reset(dest);
	mpq_set(dest->gcf_b0, src->gcf_b0);
	mpgcf_reserve(dest, src->gcf_size);
	for (i = 0; i < src->gcf_size; i++) {
		mpq_init(dest->gcf_a[i]);
		mpq_init(dest->gcf_b[i]);
		mpq_set(dest->gcf_a[i], src->gcf_a[i]);
		mpq_set(dest->gcf_b[i], src->gcf_b[i]);
	}
	dest->gcf_size = src->gcf_size;
}

void mpgcf_set_b0(mpgcf_t cf, const mpq_t b0)
{
	mpq_set(cf->gcf_b0, b0);
}

void mpgcf_set_b0_si(mpgcf_t cf, long num, unsigned long den)
{
	mpq_set_si(cf->gcf_b0, num, den);
	mpq_canonicalize(cf->gcf_b0);
}

void mpgcf_append(mpgcf_t cf, const mpq_t a, const mpq_t b)
{
	mpgcf_reserve(cf, cf->gcf_size + 1);
	mpq_init(cf->gcf_a[cf->gcf_size]);
	mpq_init(cf->gcf_b[cf->gcf_size]);
	mpq_set(cf->gcf_a[cf->gcf_size], a);
	mpq_set(cf->gcf_b[cf->gcf_size], b);
	cf->gcf_size += 1;
}

void mpgcf_append_si(mpgcf_t cf, long a_num, unsigned long a_den,
		      long b_num, unsigned long b_den)
{
	mpq_t a, b;

	mpq_init(a);
	mpq_init(b);
	mpq_set_si(a, a_num, a_den);
	mpq_canonicalize(a);
	mpq_set_si(b, b_num, b_den);
	mpq_canonicalize(b);
	mpgcf_append(cf, a, b);
	mpq_clear(a);
	mpq_clear(b);
}

size_t mpgcf_length(const mpgcf_t cf)
{
	return cf->gcf_size;
}

int mpgcf_get_term(const mpgcf_t cf, size_t i, mpq_t a, mpq_t b)
{
	if (i >= cf->gcf_size)
		return -1;
	mpq_set(a, cf->gcf_a[i]);
	mpq_set(b, cf->gcf_b[i]);
	return 0;
}

int mpgcf_set_term(mpgcf_t cf, size_t i, const mpq_t a, const mpq_t b)
{
	if (i >= cf->gcf_size)
		return -1;
	mpq_set(cf->gcf_a[i], a);
	mpq_set(cf->gcf_b[i], b);
	return 0;
}

int mpgcf_set_term_si(mpgcf_t cf, size_t i, long a_num, unsigned long a_den,
		       long b_num, unsigned long b_den)
{
	mpq_t a, b;
	int rc;

	if (i >= cf->gcf_size)
		return -1;

	mpq_init(a);
	mpq_init(b);
	mpq_set_si(a, a_num, a_den);
	mpq_canonicalize(a);
	mpq_set_si(b, b_num, b_den);
	mpq_canonicalize(b);

	rc = mpgcf_set_term(cf, i, a, b);

	mpq_clear(a);
	mpq_clear(b);
	return rc;
}

int mpgcf_set_fpstr(mpgcf_t cf, const char* str)
{
	int ret;
	mpq_t rop;

	mpq_init(rop);

        const char* dot = strchr(str, '.');
        if (dot == NULL) {
                /*
		 * XXX implement this, for now return -1
		 * mpq_set_str(rop, str, 10);
		 * mpgcf_set_b0_si(mpgcf_t cf, str, 1)
		 */
		return -1;
        }

        size_t int_len = dot - str;
        size_t decimal_places = strlen(dot + 1);

        // allocate memory for "numerator/denominator" str
        // add one byte to buf_size to safely hold a leading '0' if needed
        size_t buf_size = strlen(str) + 1 + decimal_places + 3;
        char* frac_str = malloc(buf_size);
        if (!frac_str)
                return -1;

        size_t write_pos = 0;
        if (int_len == 0) {
                // if no integer part write a '0' first
                frac_str[write_pos++] = '0';
        } else {
                strncpy(frac_str, str, int_len);
                write_pos += int_len;
        }
        frac_str[write_pos] = '\0';

        // append fractional digits
        strcpy(&frac_str[write_pos], dot + 1);
        write_pos += decimal_places;

        // append division sign '/'
        frac_str[write_pos++] = '/';
        // append denominator '1'
        frac_str[write_pos++] = '1';
        // append zeros directly from the absolute write pointer
        for (size_t i = 0; i < decimal_places; i++) {
                frac_str[write_pos++] = '0';
        }
        frac_str[write_pos] = '\0';

        ret = mpq_set_str(rop, frac_str, 10);

        mpq_canonicalize(rop);

        free(frac_str);

	ret = mpgcf_set_q(cf, rop);

	mpq_clear(rop);

	return ret;
}

int mpgcf_set_q(mpgcf_t cf, const mpq_t value)
{
	mpz_t p, q, b0, r;
	size_t count;
	int truncated = 0;
	size_t effective_max;

	if (!cf || !value)
		return -1;

	// XXX
	//if (cf.mpgcf_terms == 0)
	//	cf.mpgcf_terms = mpgcf_default_max_terms;

	effective_max = mpgcf_default_max_terms;

	mpgcf_reset(cf);

	mpz_init(p);
	mpz_init(q);
	mpz_init(b0);
	mpz_init(r);

	mpz_set(p, mpq_numref(value));
	mpz_set(q, mpq_denref(value));

	/* b0 = floor(p/q); r = p - b0*q, so p/q = b0 + r/q, 0 <= r < q */
	mpz_fdiv_qr(b0, r, p, q);

	{
		mpq_t qb0;

		mpq_init(qb0);
		mpq_set_z(qb0, b0);
		mpgcf_set_b0(cf, qb0);
		mpq_clear(qb0);
	}

	count = 0;
	/* repeatedly invert q/r -> next term, standard Euclidean CF */
	while (mpz_sgn(r) != 0 && (effective_max == 0 || count < effective_max)) {
		mpz_t next_b, next_r;

		mpz_init(next_b);
		mpz_init(next_r);

		/* p/q = b0 + r/q  =>  next fraction to expand is q/r */
		mpz_fdiv_qr(next_b, next_r, q, r);

		{
			mpq_t a_term, b_term;

			mpq_init(a_term);
			mpq_init(b_term);
			mpq_set_ui(a_term, 1, 1);
			mpq_set_z(b_term, next_b);
			mpgcf_append(cf, a_term, b_term);
			mpq_clear(a_term);
			mpq_clear(b_term);
		}

		mpz_set(q, r);
		mpz_set(r, next_r);
		mpz_clear(next_b);
		mpz_clear(next_r);
		count++;
	}

	/* check if we stopped because max_terms was reached */
	if (effective_max > 0 && mpz_sgn(r) != 0 && count == effective_max)
		truncated = 1;

	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(b0);
	mpz_clear(r);

	return truncated ? 1 : 0;
}

int mpgcf_set_f(mpgcf_t cf, const mpf_t value)
{
	mpf_t x, frac, inv;
	mpq_t b0, a_term, b_term;
	size_t count;
	int truncated = 0;
	size_t effective_max;

	if (!cf || !value)
		return -1;

	/* default if set to 0 */
	effective_max = (mpgcf_default_max_terms == 0) ? MPGCF_DEFAULT_MAX_TERMS : mpgcf_default_max_terms;

	mpgcf_reset(cf);

	mpf_init(x);
	mpf_init(frac);
	mpf_init(inv);
	mpq_init(b0);
	mpq_init(a_term);
	mpq_init(b_term);

	mpf_set(x, value);
	count = 0;

	mpf_floor(frac, x);
	mpq_set_f(b0, frac);
	mpq_canonicalize(b0);
	mpgcf_set_b0(cf, b0);

	mpf_sub(frac, x, frac);
	mpq_set_ui(a_term, 1, 1);

	while (count < effective_max) {
		if (mpf_cmp_ui(frac, 0) == 0)
			break;

		mpf_ui_div(inv, 1, frac);
		mpf_floor(frac, inv);
		mpq_set_f(b_term, frac);
		mpq_canonicalize(b_term);

		mpgcf_append(cf, a_term, b_term);
		mpf_sub(frac, inv, frac);

		count++;
	}

	if (mpf_cmp_ui(frac, 0) != 0 && count == effective_max)
		truncated = 1;

	mpf_clear(x);
	mpf_clear(frac);
	mpf_clear(inv);
	mpq_clear(b0);
	mpq_clear(a_term);
	mpq_clear(b_term);

	return truncated ? 1 : 0;
}

int mpgcf_get_q(mpq_t result, const mpgcf_t cf)
{
	mpq_t x, tmp;
	size_t n = cf->gcf_size;

	if (n == 0) {
		mpq_set(result, cf->gcf_b0);
		return 0;
	}

	mpq_init(x);
	mpq_init(tmp);

	/* x = b_n */
	mpq_set(x, cf->gcf_b[n - 1]);

	/*
	 * walk backwards: x_k = b_k + a_{k+1} / x_{k+1},
	 * for k = n-1 .. 1.  In 0-based array terms, 
	 * that is: for j = n-2 downto 0: x = b[j] + a[j+1] / x
	 */
	if (n >= 2) {
		for(size_t j = n - 2; 1; j--) {
			if (mpq_sgn(x) == 0) {
				mpq_clear(x);
				mpq_clear(tmp);
				return -1;
			}
			mpq_div(tmp, cf->gcf_a[j + 1], x);
			mpq_add(x, cf->gcf_b[j], tmp);

			if (j == 0)
				break;
		}
	}

	if (mpq_sgn(x) == 0) {
		mpq_clear(x);
		mpq_clear(tmp);
		return -1;
	}

	/* result = b0 + a_1 / x */
	mpq_div(tmp, cf->gcf_a[0], x);
	mpq_add(result, cf->gcf_b0, tmp);

	mpq_clear(x);
	mpq_clear(tmp);
	return 0;
}

static void skip_ws(const char **p)
{
	while (isspace((unsigned char)**p))
		(*p)++;
}

/*
 * parse mpq token starting at *p
 * token is [0-9], optional leading '+'/'-', and optional '/'
 * 
 * @return on success, advances *p, sets out, and returns 0, returns -1 on failure
 */
static int parse_mpq_token(const char **p, mpq_t out)
{
	const char *start;
	const char *cur;
	char *buf;
	size_t len;
	int rc;

	skip_ws(p);
	start = *p;
	cur = start;

	if (*cur == '+' || *cur == '-')
		cur++;

	if (!isdigit((unsigned char)*cur))
		return -1;

	while (isdigit((unsigned char)*cur))
		cur++;

	/*
	 * Only treat '/' as the start of a "p/q" denominator if a
	 * digit actually follows it.  This lookahead matters for
	 * instr notation, where a bare integer token can itself be
	 * followed by a '/' that is the a/b separator rather than
	 * part of the number, e.g. the "1" in "1/(7 + ...)".
	 */
	if (*cur == '/' && isdigit((unsigned char)*(cur + 1))) {
		cur++;
		while (isdigit((unsigned char)*cur))
			cur++;
	}

	len = (size_t)(cur - start);
	buf = malloc(len + 1);
	if (!buf)
		return -1;
	memcpy(buf, start, len);
	buf[len] = '\0';

	rc = mpq_set_str(out, buf, 10);
	free(buf);
	if (rc != 0)
		return -1;

	mpq_canonicalize(out);
	*p = cur;
	return 0;
}

static int expect_char(const char **p, char c)
{
	skip_ws(p);
	if (**p != c)
		return -1;
	(*p)++;
	return 0;
}

int mpgcf_set_bnstr(mpgcf_t cf, const char *str, const char **endptr)
{
	const char *p = str;
	mpgcf_t tmp;
	mpq_t b0;

	mpgcf_init(tmp);
	mpq_init(b0);

	skip_ws(&p);
	if (expect_char(&p, '[') != 0)
		goto fail;

	skip_ws(&p);
	if (*p == ';') {
		/* implicitly set b0 = 0 */
		mpq_set_ui(b0, 0, 1);
	} else {
		if (parse_mpq_token(&p, b0) != 0)
			goto fail;
	}
	mpgcf_set_b0(tmp, b0);

	skip_ws(&p);
	if (*p == ';') {
		p++; /* consume ';' */
		skip_ws(&p);

		/* loop as long as there is an opening '(' or a start of an mpq number */
		while (*p == '(' || isdigit((unsigned char)*p) || *p == '+' || *p == '-') {
			mpq_t a, b;
			int is_generalized = (*p == '(');

			mpq_init(a);
			mpq_init(b);

			if (is_generalized) {
				p++; /* consume '(' */
				if (parse_mpq_token(&p, a) != 0) {
					mpq_clear(a);
					mpq_clear(b);
					goto fail;
				}
				if (expect_char(&p, ',') != 0) {
					mpq_clear(a);
					mpq_clear(b);
					goto fail;
				}
				if (parse_mpq_token(&p, b) != 0) {
					mpq_clear(a);
					mpq_clear(b);
					goto fail;
				}
				if (expect_char(&p, ')') != 0) {
					mpq_clear(a);
					mpq_clear(b);
					goto fail;
				}
			} else {
				/* implicitly set a = 1 */
				mpq_set_ui(a, 1, 1);
				if (parse_mpq_token(&p, b) != 0) {
					mpq_clear(a);
					mpq_clear(b);
					goto fail;
				}
			}

			mpgcf_append(tmp, a, b);
			mpq_clear(a);
			mpq_clear(b);

			skip_ws(&p);
			if (*p == ',') {
				p++;
				skip_ws(&p);
				continue;
			}
			break;
		}
	}

	if (expect_char(&p, ']') != 0)
		goto fail;

	mpgcf_set(cf, tmp);
	mpgcf_clear(tmp);
	mpq_clear(b0);
	if (endptr)
		*endptr = p;
	return 0;

fail:
	mpgcf_clear(tmp);
	mpq_clear(b0);
	return -1;
}

int mpgcf_set_bnstr_general(mpgcf_t cf, const char *str, const char **endptr)
{
	const char *p = str;
	mpgcf_t tmp;
	mpq_t b0;

	mpgcf_init(tmp);
	mpq_init(b0);

	if (expect_char(&p, '[') != 0)
		goto fail;

	if (parse_mpq_token(&p, b0) != 0)
		goto fail;
	mpgcf_set_b0(tmp, b0);

	skip_ws(&p);
	if (*p == ';') {
		p++; /* consume ';' */
		skip_ws(&p);

		/* term list is optional after ';', ie: "[ b0 ; ]" is valid. */
		while (*p == '(') {
			mpq_t a, b;

			p++; /* consume '(' */
			mpq_init(a);
			mpq_init(b);

			if (parse_mpq_token(&p, a) != 0) {
				mpq_clear(a);
				mpq_clear(b);
				goto fail;
			}

			if (expect_char(&p, ',') != 0) {
				mpq_clear(a);
				mpq_clear(b);
				goto fail;
			}

			if (parse_mpq_token(&p, b) != 0) {
				mpq_clear(a);
				mpq_clear(b);
				goto fail;
			}

			if (expect_char(&p, ')') != 0) {
				mpq_clear(a);
				mpq_clear(b);
				goto fail;
			}

			mpgcf_append(tmp, a, b);
			mpq_clear(a);
			mpq_clear(b);

			skip_ws(&p);
			if (*p == ',') {
				p++;
				skip_ws(&p);
				continue;
			}
			break;
		}
	}

	if (expect_char(&p, ']') != 0)
		goto fail;

	mpgcf_set(cf, tmp);
	mpgcf_clear(tmp);
	mpq_clear(b0);
	if (endptr)
		*endptr = p;
	return 0;

fail:
	mpgcf_clear(tmp);
	mpq_clear(b0);
	return -1;
}

int mpgcf_out_bnstr(FILE *stream, const mpgcf_t cf)
{
	int total = 0;
	int rc;
	size_t i;

	rc = fprintf(stream, "[ ");
	if (rc < 0)
		return rc;
	total += rc;

	rc = mpq_out_str(stream, 10, cf->gcf_b0);
	if (rc < 0)
		return rc;
	total += rc;

	if (cf->gcf_size > 0) {
		rc = fprintf(stream, " ; ");
		if (rc < 0)
			return rc;
		total += rc;

		for (i = 0; i < cf->gcf_size; i++) {
			rc = fprintf(stream, "%s(", i == 0 ? "" : ", ");
			if (rc < 0)
				return rc;
			total += rc;

			rc = mpq_out_str(stream, 10, cf->gcf_a[i]);
			if (rc < 0)
				return rc;
			total += rc;

			rc = fprintf(stream, ", ");
			if (rc < 0)
				return rc;
			total += rc;

			rc = mpq_out_str(stream, 10, cf->gcf_b[i]);
			if (rc < 0)
				return rc;
			total += rc;

			rc = fprintf(stream, ")");
			if (rc < 0)
				return rc;
			total += rc;
		}
	}

	rc = fprintf(stream, " ]");
	if (rc < 0)
		return rc;
	total += rc;

	return total;
}

char *mpgcf_get_bnstr(const mpgcf_t cf)
{
	char *buf;
	size_t cap;
	FILE *f;
	long size;

	f = tmpfile();
	if (!f)
		return NULL;

	if (mpgcf_out_bnstr(f, cf) < 0) {
		fclose(f);
		return NULL;
	}

	size = ftell(f);
	if (size < 0) {
		fclose(f);
		return NULL;
	}

	cap = (size_t)size;
	buf = malloc(cap + 1);
	if (!buf) {
		fclose(f);
		return NULL;
	}

	rewind(f);
	if (fread(buf, 1, cap, f) != cap) {
		free(buf);
		fclose(f);
		return NULL;
	}
	buf[cap] = '\0';
	fclose(f);
	return buf;
}

/*
 * parse "a_i / ( b_i [+ <nested>] )", appending (a_i, b_i) to tmp
 * and recursing to pick up any further terms nested inside the
 * parens.  Every b_i is parenthesized, including the innermost
 * (last) one: without that, a bare trailing "an/bn" would be
 * indistinguishable from a single GMP "p/q" rational literal (e.g.
 * "1/15" the number one-fifteenth vs. "1" then separator then "15")
 * whenever bn happens to start with a digit, which for a classical
 * (all a_i = 1) continued fraction is always.
 */
static int parse_nested_instr(const char **p, mpgcf_t tmp)
{
	mpq_t a, b;
	int rc = -1;

	mpq_init(a);
	mpq_init(b);

	if (parse_mpq_token(p, a) != 0)
		goto out;

	if (expect_char(p, '/') != 0)
		goto out;

	if (expect_char(p, '(') != 0)
		goto out;

	if (parse_mpq_token(p, b) != 0)
		goto out;

	mpgcf_append(tmp, a, b);

	skip_ws(p);
	if (**p == '+') {
		(*p)++; /* consume '+' */
		skip_ws(p);
		if (parse_nested_instr(p, tmp) != 0)
			goto out;
	}

	if (expect_char(p, ')') != 0)
		goto out;

	rc = 0;
out:
	mpq_clear(a);
	mpq_clear(b);
	return rc;
}

int mpgcf_set_instr(mpgcf_t cf, const char *str, const char **endptr)
{
	const char *p = str;
	mpgcf_t tmp;
	mpq_t b0;

	mpgcf_init(tmp);
	mpq_init(b0);

	if (parse_mpq_token(&p, b0) != 0)
		goto fail;
	mpgcf_set_b0(tmp, b0);

	skip_ws(&p);
	if (*p == '+') {
		p++; /* consume '+' */
		skip_ws(&p);
		if (parse_nested_instr(&p, tmp) != 0)
			goto fail;
	}

	mpgcf_set(cf, tmp);
	mpgcf_clear(tmp);
	mpq_clear(b0);
	if (endptr)
		*endptr = p;
	return 0;

fail:
	mpgcf_clear(tmp);
	mpq_clear(b0);
	return -1;
}

/*
 * write "a_i/(b_i [+ <recurse>])" for term i, always parenthesizing
 * b_i (including the last term -- see parse_nested_instr for why).
 * Sets *err on the first I/O failure and unwinds without further
 * writes.
 */
static int out_instr_nested(FILE *stream, const mpgcf_t cf, size_t i, int *err)
{
	int total = 0;
	int rc;
	int last = (i + 1 == cf->gcf_size);

	rc = mpq_out_str(stream, 10, cf->gcf_a[i]);
	if (rc < 0) {
		*err = 1;
		return total;
	}
	total += rc;

	rc = fprintf(stream, "/(");
	if (rc < 0) {
		*err = 1;
		return total;
	}
	total += rc;

	rc = mpq_out_str(stream, 10, cf->gcf_b[i]);
	if (rc < 0) {
		*err = 1;
		return total;
	}
	total += rc;

	if (!last) {
		rc = fprintf(stream, " + ");
		if (rc < 0) {
			*err = 1;
			return total;
		}
		total += rc;

		total += out_instr_nested(stream, cf, i + 1, err);
		if (*err)
			return total;
	}

	rc = fprintf(stream, ")");
	if (rc < 0) {
		*err = 1;
		return total;
	}
	total += rc;

	return total;
}

int mpgcf_out_instr(FILE *stream, const mpgcf_t cf)
{
	int total = 0;
	int rc;
	int err = 0;

	rc = mpq_out_str(stream, 10, cf->gcf_b0);
	if (rc < 0)
		return rc;
	total += rc;

	if (cf->gcf_size > 0) {
		rc = fprintf(stream, " + ");
		if (rc < 0)
			return rc;
		total += rc;

		total += out_instr_nested(stream, cf, 0, &err);
		if (err)
			return -1;
	}

	return total;
}

char *mpgcf_get_instr(const mpgcf_t cf)
{
	char *buf;
	size_t cap;
	FILE *f;
	long size;

	f = tmpfile();
	if (!f)
		return NULL;

	if (mpgcf_out_instr(f, cf) < 0) {
		fclose(f);
		return NULL;
	}

	size = ftell(f);
	if (size < 0) {
		fclose(f);
		return NULL;
	}

	cap = (size_t)size;
	buf = malloc(cap + 1);
	if (!buf) {
		fclose(f);
		return NULL;
	}

	rewind(f);
	if (fread(buf, 1, cap, f) != cap) {
		free(buf);
		fclose(f);
		return NULL;
	}
	buf[cap] = '\0';
	fclose(f);
	return buf;
}

int mpgcf_inp_instr(FILE *stream, mpgcf_t cf)
{
	char *buf = NULL;
	size_t cap = 0, len = 0;
	long start;
	int c;
	int rc;
	const char *endptr = NULL;
	long consumed, unconsumed;

	start = ftell(stream);

	/*
	 * There is no fixed terminator for instr notation (unlike
	 * bnstr's closing ']'), so we can't know where the value
	 * ends without parsing it -- and parsing needs the whole
	 * thing available as a string first. Slurp everything the
	 * stream has to offer, parse it with mpgcf_set_instr(), then
	 * seek the stream back over whatever trailing bytes weren't
	 * part of the parsed value, leaving it positioned just past
	 * the CF (as if only the CF had been read). This rollback
	 * only works for a seekable stream (a regular file, as
	 * returned by e.g. fopen() or tmpfile()); for a
	 * non-seekable stream (a pipe, etc.) the whole stream is
	 * necessarily consumed.
	 */
	while ((c = fgetc(stream)) != EOF) {
		if (len + 1 >= cap) {
			size_t new_cap = cap == 0 ? 64 : cap * 2;
			char *new_buf = realloc(buf, new_cap);

			if (!new_buf) {
				free(buf);
				return -1;
			}
			buf = new_buf;
			cap = new_cap;
		}
		buf[len++] = (char)c;
	}
	if (ferror(stream)) {
		free(buf);
		return -1;
	}
	if (!buf) {
		/* stream was already at EOF; nothing to parse */
		return -1;
	}
	buf[len] = '\0';

	rc = mpgcf_set_instr(cf, buf, &endptr);
	if (rc != 0) {
		free(buf);
		if (start >= 0)
			fseek(stream, start, SEEK_SET);
		return -1;
	}

	consumed = (long)(endptr - buf);
	unconsumed = (long)len - consumed;
	free(buf);

	if (unconsumed > 0)
		fseek(stream, -unconsumed, SEEK_CUR); /* no-op failure on unseekable streams */

	return 0;
}

