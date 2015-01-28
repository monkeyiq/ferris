
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = simpleroot

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

simpleroot_SOURCES = simpleroot.cpp
simpleroot_LDADD =  -lpopt @LIBFERRIS_LA@ 
simpleroot_LDFLAGS = @GLIB_LIBS@

