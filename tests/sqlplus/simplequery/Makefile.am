
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST=$(srcdir)/*.xml
noinst_PROGRAMS = sqlsimplequery

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

sqlsimplequery_SOURCES = sqlsimplequery.cpp
sqlsimplequery_LDADD =  -lpopt @LIBFERRIS_LA@ 
sqlsimplequery_LDFLAGS = @GLIB_LIBS@

