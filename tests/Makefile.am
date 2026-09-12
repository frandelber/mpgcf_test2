AM_CPPFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = test_mpgcf

test_mpgcf_SOURCES = test_mpgcf.c
test_mpgcf_LDADD = $(top_builddir)/src/libmpgcf.la -lgmp

TESTS = test_mpgcf
