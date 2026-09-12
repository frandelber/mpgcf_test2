/*
 * Copyright (C) 2026 mpgcf authors
 *
 * This file is part of mpgcf.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef MPGCF_H
#define MPGCF_H

#include <gmp.h>
#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * generalized continued fraction of the form
 *
 *   b0 + a1 / (b1 + a2 / (b2 + a3 / (b3 + ... + an / bn)))
 *
 * is represented as a leading term b0 (mpq_t) followed by n pairs
 * (a_i, b_i), i = 1..n, each also mpq_t.
 * for a classical simple continued fraction all a_i are equal to 1.
 *
 * bracket notation is used to parse/printei by the library:
 *
 *   [ b_0 ; (a_1, b_1), (a_2, b_2), ..., (a_n, b_n) ]
 *
 * where each of b_0, a_i, b_i is written in GMP rational syntax
 * ("p" or "p/q", with an optional leading '-').
 */


/* default maximum number of terms per CF */
#define MPGCF_DEFAULT_MAX_TERMS 20

/* default precision (bits) used for the mpf_t built inside
 * mpgcf_set_f_terms() -- see mpgcf_set_default_f_precision()
 * */
#define MPGCF_DEFAULT_F_PRECISION 1024 /* 128 bytes */
// #define MPGCF_DEFAULT_F_PRECISION 2048 /* 256 bytes */

struct mpgcf_struct {
	mpq_t gcf_b0;		/* leading term */
	mpq_t *gcf_a;		/* array of n numerator terms a_1..a_n */
	mpq_t *gcf_b;		/* array of n denominator terms b_1..b_n */
	size_t gcf_size;	/* number of (a_i, b_i) pairs */
	size_t gcf_alloc;	/* allocated capacity of a[] and b[] (internal) */
};

typedef struct mpgcf_struct mpgcf_t[1];

/* initialize CF to the empty continued fraction "[ 0 ]". */
void mpgcf_init(mpgcf_t cf);

/*
 * free all memory owned by CF.
 */
void mpgcf_clear(mpgcf_t cf);

/* set dst to a copy of src */
void mpgcf_set(mpgcf_t dst, const mpgcf_t src);

/*
 * reset CF back to the empty continued fraction "[ 0 ]", discarding  all terms
 */
void mpgcf_reset(mpgcf_t cf);

/*
 * get maximum number of terms
 */
size_t mpgcf_get_default_max_terms(void);

/*
 * default maximum number of terms
 * @param max_terms The new default value (0 means no limit)
 */
void mpgcf_set_default_max_terms(size_t max_terms); 

/*
 * get the default precision (bits) that mpgcf_set_f_terms() starts
 * its search from.
 */
mp_bitcnt_t mpgcf_get_default_f_precision(void);

/*
 * default precision (bits) that mpgcf_set_f_terms() starts its
 * search from. Defaults to MPGCF_DEFAULT_F_PRECISION (1024 bits,
 * 128 bytes of mantissa). Unlike mpgcf_set_default_max_terms(),
 * @param prec The new default precision, in bits
 */
void mpgcf_set_default_f_precision(mp_bitcnt_t prec);

/* set the leading term b0. */
void mpgcf_set_b0(mpgcf_t cf, const mpq_t b0);
void mpgcf_set_b0_si(mpgcf_t cf, long num, unsigned long den);

/* append a new term (a_i, b_i) to the end of the term list. */
void mpgcf_append(mpgcf_t cf, const mpq_t a, const mpq_t b);
void mpgcf_append_si(mpgcf_t cf, long a_num, unsigned long a_den,
		      long b_num, unsigned long b_den);

/* number of (a_i, b_i) terms currently stored (not counting b0). */
size_t mpgcf_length(const mpgcf_t cf);

/*
 * access to the i-th term (0-based, 0 .. length-1). Returns nonzero
 * leaves a/b untouched if I is out of range.
 * @return nonzero
 */
int mpgcf_get_term(const mpgcf_t cf, size_t i, mpq_t a, mpq_t b);

/*
 * overwrite the i-th existing term (0-based, 0 .. length-1) with a
 * new (a, b) pair.  Returns 0 on success, or nonzero (leaving CF
 * unmodified) if I is out of range.  Unlike mpgcf_append, this does
 * not grow the term list -- use it to update a term that has
 * already been created.
 */
int mpgcf_set_term(mpgcf_t cf, size_t i, const mpq_t a, const mpq_t b);
int mpgcf_set_term_si(mpgcf_t cf, size_t i, long a_num, unsigned long a_den,
		       long b_num, unsigned long b_den);

/*
 * build simple continued fraction from fraction
 * ie: 351/2322
 * @return 0 on success, 1 if truncated due to max_terms, -1 on error.
 */
int mpgcf_set_q(mpgcf_t cf, const mpq_t value);

/*
 * build simple continued fraction from floating point string number
 * ie: 3.14159
 * @return 0 on success, 1 if truncated due to max_terms, -1 on error.
 */
int mpgcf_set_fpstr(mpgcf_t cf, const char* str);

/*
 * build simple continued fraction expansion of the
 * floating-point value (all a_i = 1), using the library default max_terms.
 *
 * Each term is produced by a subtraction that cancels out however
 * many bits value and the term's integer part have in common -- that
 * precision is gone for good, not recoverable. Once continuing would
 * mean reusing value's own rounding noise as if it were more digits
 * of value, the expansion stops on its own (reported the same way as
 * hitting max_terms) rather than emitting terms that don't actually
 * reflect value. A higher-precision value (see mpf_init2()) supports
 * more terms.
 * @param cf: dest
 * @param value: orig
 * @return 0 on success, 1 truncated (by max_terms or exhausted
 * precision), -1 on error
 */
int mpgcf_set_f(mpgcf_t cf, const mpf_t value);

/*
 * build the classical simple continued fraction expansion of a
 * computed value into CF, adaptively growing the precision that
 * value is computed at until at least WANT_TERMS terms come out
 * reliable (see mpgcf_set_f() for what "reliable" means), rather
 * than requiring the caller to guess a precision up front.
 *
 * There's no fixed relationship between term count and the bits of
 * precision needed to support it -- it depends on how large that
 * particular value's own partial quotients happen to be (e.g.
 * exp(2)'s quotients grow roughly linearly with term index, so it
 * needs progressively *more* bits per additional term, not a
 * constant amount) -- so this can't be precomputed by a formula and
 * has to be discovered by actually expanding the value and checking.
 *
 * Since only the caller knows how to (re)compute their value at an
 * arbitrary precision, that's supplied as a callback: COMPUTE(out,
 * prec, ctx) must set OUT to the value, computed to at least PREC
 * bits of precision (out is already mpf_init2()'d to PREC by this
 * function; COMPUTE must not clear or reinitialize it). CTX is
 * passed through unchanged, for any state COMPUTE needs (e.g. which
 * constant to compute, a working buffer, etc.), and may be NULL if
 * unneeded.
 *
 * The search starts at mpgcf_get_default_f_precision() bits and
 * doubles each time mpgcf_set_f() doesn't yet yield WANT_TERMS
 * terms, stopping either once it does, or once precision would
 * exceed MAX_PREC bits (MAX_PREC == 0 means no cap -- be aware this
 * can then in principle grow without bound, and COMPUTE gets
 * correspondingly more expensive at each doubling).
 *
 * @param cf: dest, left holding whatever mpgcf_set_f() last
 * produced (i.e. the best available expansion, even on failure)
 * @param compute: callback that computes the target value to a
 * given precision, as described above
 * @param ctx: opaque pointer passed through to COMPUTE unchanged
 * @param want_terms: how many (a_i, b_i) terms to reliably obtain
 * @param max_prec: precision cap in bits (0 means no cap)
 * @param out_prec: if non-NULL, set on return to the precision (in
 * bits) that was actually used to build CF
 * @return 0 if WANT_TERMS terms were reliably obtained; 1 if the
 * search was capped by MAX_PREC first (CF then has as many terms as
 * that precision could support, which is fewer than WANT_TERMS);
 * -1 on error (CF or compute is NULL)
 */
int mpgcf_set_f_terms(mpgcf_t cf,
		       void (*compute)(mpf_t out, mp_bitcnt_t prec, void *ctx),
		       void *ctx, size_t want_terms, mp_bitcnt_t max_prec,
		       mp_bitcnt_t *out_prec);

/*
 * evaluate CF to a single rational, stored into result.
 * @return 0 on success, -1 if a division by zero was encountered
 * while collapsing the fraction (in which case @result is untouched).
 */
int mpgcf_get_q(mpq_t result, const mpgcf_t cf);

/*
 * bnstr, bracket notation string
 * in bracket notation format "[ b0 ; (a1, b1), (a2, b2), ..., (an, bn) ]",
 * parse a continued fraction from the 0-terminated string str,
 * Whitespace around tokens and delimiters is ignored.  The list of
 * (a_i, b_i) terms (and the surrounding "; ... " part) is optional,
 * so "[ b0 ]" is also accepted, as is "[ b0 ; ]".
 * On success, CF (which must already be initialized) is set to the
 * parsed value and 0 is returned.  On failure a negative value is
 * returned and CF is left unmodified (reset to empty).
 * If endptr is non-NULL, *endptr is set to point just past the closing ']' on success.
 */
int mpgcf_set_bnstr(mpgcf_t cf, const char *str, const char **endptr);

/*
 * bnstr, bracket notation string
 * print @cf to @stream in the standard format. 
 * @return number of characters written, or a negative value on error
 */
int mpgcf_out_bnstr(FILE *stream, const mpgcf_t cf);

/*
 * bnstr, bracket notation string
 * format @cf into a newly malloc 0-terminated string, the caller must free().
 * @return NULL on allocation failure
 */
char *mpgcf_get_bnstr(const mpgcf_t cf);

/*
 * instr, in-order (nested) mathematical notation string
 *
 * unlike bnstr, which lists every (a_i, b_i) pair flatly, instr
 * writes the continued fraction the way it is normally written in
 * mathematics, with each level of the fraction nested inside the
 * previous one:
 *
 *   b0 + a1/(b1 + a2/(b2 + a3/(b3 + ... + an/(bn))))
 *
 * every b_i, including the innermost (last) one, is wrapped in its
 * own "(...)" -- there are as many '(' as there are terms.  This is
 * required, not just cosmetic: without a paren around bn, a bare
 * trailing "an/bn" (e.g. "1/15") would be indistinguishable from a
 * single GMP "p/q" rational literal, which is exactly the syntax
 * used for b0, a_i and b_i themselves.  If CF has no (a_i, b_i)
 * terms, the string is simply the value of b0 with no "+".  As with
 * bnstr, each of b0, a_i, b_i is written in GMP rational syntax
 * ("p" or "p/q", optional leading '-'), and whitespace around
 * tokens/operators is ignored on parse.
 *
 * mpgcf_set_instr parses such a string into CF (which must already
 * be initialized).  On success 0 is returned and, if endptr is
 * non-NULL, *endptr is set to point just past the last character
 * consumed (i.e. just past b0, or just past the final matching ')'
 * / bn if there are terms).  On failure a negative value is
 * returned and CF is left unmodified (reset to empty).
 */
int mpgcf_set_instr(mpgcf_t cf, const char *str, const char **endptr);

/*
 * instr, in-order (nested) mathematical notation string
 * print @cf to @stream in the nested "b0 + a1/(b1 + ... )" form.
 * @return number of characters written, or a negative value on error
 */
int mpgcf_out_instr(FILE *stream, const mpgcf_t cf);

/*
 * instr, in-order (nested) mathematical notation string
 * read a value in instr notation from @stream and parse it into CF
 * (which must already be initialized), mirroring mpgcf_out_instr.
 * Unlike mpgcf_set_instr, there is no explicit terminator to stop
 * at, so this reads the stream through to EOF to find one, and then
 * -- for a seekable stream, e.g. a regular file -- seeks back over
 * any trailing bytes that came after the parsed value, leaving the
 * stream positioned just past it. For a non-seekable stream (a
 * pipe, etc.) any such trailing bytes are unavoidably consumed.
 * @return 0 on success. On failure a negative value is returned, CF
 * is left unmodified (reset to empty), and the stream position is
 * restored to where it started if the stream is seekable.
 */
int mpgcf_inp_instr(FILE *stream, mpgcf_t cf);

/*
 * instr, in-order (nested) mathematical notation string
 * format @cf into a newly malloc'd 0-terminated string, in the
 * nested "b0 + a1/(b1 + ... )" form, the caller must free().
 * @return NULL on allocation failure
 */
char *mpgcf_get_instr(const mpgcf_t cf);

#ifdef __cplusplus
}
#endif

#endif /* MPGCF_H */

