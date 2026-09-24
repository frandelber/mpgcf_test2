# mpgcf

Generalized Continued Fraction library. (MP GCF).

A C library for Generalized Continued Fractions built on top of [GNU MP](https://gmplib.org/).

A continued fraction is represented as

```
b0 + a1 / (b1 + a2 / (b2 + a3 / (b3 + ... + an / bn)))
```
ie:
```
1/2 + (2/3) / ((3/4) + (4/5) / ((5/6) + (6/7) / (7/8)))
```

Where `b0` and every `(a_i, b_i)` pair are arbitrary-precision rationals.
Simple continued fractions are the special case `a_i == 1` for all `i`.

The library can:

- parse and print the standard textual form:
    * generalized continued fraction
    `[ b_0 ; (a_1, b_1), (a_2, b_2), ..., (a_n, b_n) ]`
    * simple continued fraction
    `[ b_0 ; b_1, b_2, ..., b_n ]`
- build continued fractions term-by-term programmatically
- overwrite an existing term in place by index (`mpgcf_set_term`)
- evaluate a continued fraction down to a single fraction
- expand an arbitrary rational into its simple continued fraction
  (optionally truncated to a maximum number of terms)

The C source follows the
[Linux kernel coding style](https://www.kernel.org/doc/html/latest/process/coding-style.html)
(tabs, K&R-derived brace placement, etc).

## Requirements

- A C99 compiler
- [GMP](https://gmplib.org/) (`libgmp-dev` / `gmp-devel`)
- GNU Autotools (autoconf, automake, libtool)
    only needed if building from a git checkout rather than a release tarball

## Building

From a release tarball:

```sh
./configure
make
make check
sudo make install
```

From a git checkout (no `configure` script yet):

```sh
./autogen.sh
./configure
make
make check
sudo make install
```

`make check` builds and runs the test suite under `tests/`.

### Useful configure options

- `--prefix=DIR` — install location (default `/usr/local`)
- `--with-gmp=DIR` — point to a non-standard GMP install

## Using the library

```c
#include <mpgcf.h>
#include <gmp.h>
#include <stdio.h>

int main(void)
{
	mpgcf_t cf;
	mpq_t value;

	mpgcf_init(cf);
	mpgcf_set_str(cf, "[ 3 ; (1,7), (1,15), (1,1), (1,292) ]", NULL);

	mpq_init(value);
	mpgcf_eval(value, cf);
	gmp_printf("value = %Qd\n", value); /* 103993/33102 */

	mpgcf_out_str(stdout, cf);
	putchar('\n');                      /* [ 3 ; (1, 7), (1, 15), (1, 1), (1, 292) ] */

	mpq_clear(value);
	mpgcf_clear(cf);
	return 0;
}
```

Compile and link with:

```sh
cc myprog.c -lmpgcf -lgmp -o myprog
# or, using the installed pkg-config file:
cc myprog.c $(pkg-config --cflags --libs mpgcf) -o myprog
```

See `doc/USAGE.md` for a fuller walkthrough and `man 3 mpgcf`
(installed from `doc/mpgcf.3`) for the complete API reference.

## Project layout

```
src/    library source (mpgcf.h, mpgcf.c)
tests/  automated test suite, run via `make check`
tools/  sample `gfc` binary to parse bracket notation GCF and print the rational result
doc/    man page (mpgcf.3) and usage guide (USAGE.md)
```

## License

mpgcf is free software: you can redistribute it and/or modify it under the terms of the **GNU Lesser General Public License** as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

See the [`LICENSE`](LICENSE) file for the full text. The `COPYING` and `COPYING.LESSER` files are included as part of the standard LGPLv3 packaging convention.

## Contact
I am Francisco Delgado Bertuzzi.
If you have any comment please reach out via my [Contact Form](https://forms.gle/xMNMESoKr22uLf529)
To protect my inbox from spam my direct email is hidden.


