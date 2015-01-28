NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = timeparse

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

timeparse_SOURCES = timeparse.cpp
timeparse_LDADD   = -lpopt @LIBFERRIS_LA@ 
timeparse_LDFLAGS = @GLIB_LIBS@




