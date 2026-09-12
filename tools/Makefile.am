AM_CPPFLAGS = -I$(top_srcdir)/src

bin_PROGRAMS = gcf fpstr2cf

gcf_SOURCES = gcf.c
gcf_LDADD = $(top_builddir)/src/libmpgcf.la -lgmp

fpstr2cf_SOURCES = fpstr2cf.c
fpstr2cf_LDADD = $(top_builddir)/src/libmpgcf.la -lgmp

# Test program (optional, if you want to run as a test)
check_PROGRAMS = fpstr2cf
TESTS = fpstr2cf

# Clean up
CLEANFILES = *~

