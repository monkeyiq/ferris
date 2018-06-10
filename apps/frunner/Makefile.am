
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = 

bin_PROGRAMS = ferris-runner

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/eagenerators/           \
	-I/usr/local/include                            \
	@CFLAGS@



ferris_runner_SOURCES = ferris-runner.cpp
ferris_runner_LDADD = -lm -lpopt
ferris_runner_LDFLAGS = @LIBFERRIS_LA@ 

