# mpgcf usage guide

`mpgcf` represents *generalized continued fractions*

```
b0 + a1 / (b1 + a2 / (b2 + a3 / (b3 + ... + an / bn)))
```

where `b0` and every `(a_i, b_i)` pair is an arbitrary-precision
rational (`mpq_t`, from GNU MP). A classical *simple* continued
fraction is just the special case where every `a_i == 1`.

See `man 3 mpgcf` (installed from `doc/mpgcf.3`) for the full
function-by-function API reference. This document walks through
common tasks.

## Linking

```sh
cc myprog.c -lmpgcf -lgmp -o myprog
```

or, if the installed `pkg-config` file is available:

```sh
cc myprog.c $(pkg-config --cflags --libs mpgcf) -o myprog
```

## The external text format

```
[ b_0 ; (a_1, b_1), (a_2, b_2), ..., (a_n, b_n) ]
```

- Every number (`b_0`, `a_i`, `b_i`) is written in plain GMP rational
  syntax: optional leading `-`, digits, optional `/digits`
  denominator. Examples: `3`, `-7`, `2/5`, `-1/3`.
- Whitespace around brackets, the semicolon, parentheses, and commas
  is ignored, so `[3;(1,7),(1,15)]` and
  `[ 3 ; (1, 7), (1, 15) ]` parse identically.
- The `; (a_1,b_1), ...` part is optional: `[ 3 ]` is a valid
  continued fraction with zero terms (i.e. just the integer 3).

## Building a continued fraction by hand

```c
#include <mpgcf.h>

mpgcf_t cf;
mpgcf_init(cf);

mpgcf_set_b0_si(cf, 3, 1);          /* b0 = 3/1        */
mpgcf_append_si(cf, 1, 1, 7, 1);    /* (a1,b1) = (1,7) */
mpgcf_append_si(cf, 1, 1, 15, 1);   /* (a2,b2) = (1,15)*/

mpgcf_out_str(stdout, cf);          /* [ 3 ; (1, 7), (1, 15) ] */
putchar('\n');

mpgcf_clear(cf);
```

## Parsing from text

```c
mpgcf_t cf;
mpgcf_init(cf);

if (mpgcf_set_str(cf, "[ 3 ; (1,7), (1,15), (1,1), (1,292) ]", NULL) != 0)
	fprintf(stderr, "malformed continued fraction\n");

mpgcf_clear(cf);
```

`mpgcf_set_str` never partially applies a malformed input: on
failure `cf` is reset to `[ 0 ]`.

## Evaluating to a rational

```c
mpq_t value;
mpq_init(value);

if (mpgcf_eval(value, cf) == 0)
	gmp_printf("%Qd\n", value);   /* e.g. 103993/33102 */
else
	fprintf(stderr, "division by zero while evaluating\n");

mpq_clear(value);
```

## Expanding a rational into a simple continued fraction

```c
mpq_t r;
mpq_init(r);
mpq_set_str(r, "355/113", 10);
mpq_canonicalize(r);

mpgcf_t cf;
mpgcf_init(cf);
mpgcf_from_mpq(cf, r, 0 /* no term limit */);

mpgcf_out_str(stdout, cf);
putchar('\n');
/* => [ 3 ; (1, 7), (1, 16) ] */

mpgcf_clear(cf);
mpq_clear(r);
```

Pass a nonzero `max_terms` to truncate the expansion early (useful
for irrational numbers approximated as a rational, or for very large
denominators where you only want the first few partial quotients).

## Updating a term in place by index

Use `mpgcf_set_term` (or the small-integer convenience wrapper
`mpgcf_set_term_si`) to overwrite an existing `(a_i, b_i)` pair
without changing the total number of terms. This is different from
`mpgcf_append`, which always adds a brand new term at the end.

```c
mpq_t a, b;
mpq_init(a);
mpq_init(b);
mpq_set_si(a, 1, 1);
mpq_set_si(b, 16, 1);

/* overwrite the term at index 1 (the second (a_i, b_i) pair) */
if (mpgcf_set_term(cf, 1, a, b) != 0)
	fprintf(stderr, "index out of range\n");

/* equivalent, without building mpq_t values by hand */
mpgcf_set_term_si(cf, 1, 1, 1, 16, 1);

mpq_clear(a);
mpq_clear(b);
```

Both functions return nonzero and leave `cf` untouched if `i` is
`>= mpgcf_length(cf)`.

## Getting a heap string instead of printing

```c
char *s = mpgcf_get_str(cf);
if (s) {
	puts(s);
	free(s);
}
```

## Copying and resetting

```c
mpgcf_t a, b;
mpgcf_init(a);
mpgcf_set_str(a, "[ 2 ; (1,3), (1,4) ]", NULL);

mpgcf_init(b);
mpgcf_set(b, a);      /* deep copy */

mpgcf_reset(a);       /* a becomes [ 0 ]; b is unaffected */

mpgcf_clear(a);
mpgcf_clear(b);
```

## Error handling summary

| Function            | Failure indicator                                    |
|---------------------|-------------------------------------------------------|
| `mpgcf_set_str`     | returns negative value; `cf` reset to `[ 0 ]`          |
| `mpgcf_eval`        | returns `-1` on division by zero; result untouched     |
| `mpgcf_get_term`    | returns nonzero if `i` is out of range                 |
| `mpgcf_set_term`    | returns nonzero if `i` is out of range; `cf` untouched |
| `mpgcf_get_str`     | returns `NULL` on allocation/I-O failure               |
