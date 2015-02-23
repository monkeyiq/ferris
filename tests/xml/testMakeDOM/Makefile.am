
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = testMakeDOM

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@XML4C_CFLAGS@                                  \
	@CFLAGS@

testMakeDOM_SOURCES = testMakeDOM.cpp
testMakeDOM_LDADD = -lpopt @LIBFERRIS_LA@ 
testMakeDOM_LDFLAGS = @GLIB_LIBS@ @XML4C_LIBS@

