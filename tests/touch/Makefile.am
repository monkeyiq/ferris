NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = touchtest

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@CFLAGS@

touchtest_SOURCES = touchtest.cpp
touchtest_LDADD   = -lpopt @LIBFERRIS_LA@ 
touchtest_LDFLAGS = 




