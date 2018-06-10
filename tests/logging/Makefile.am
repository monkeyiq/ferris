
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = logtest interlace

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@CFLAGS@

logtest_SOURCES = logtest.cpp
logtest_LDADD   = -lpopt @LIBFERRIS_LA@ 
logtest_LDFLAGS = @GLIB_LIBS@

interlace_SOURCES = interlace.cpp
interlace_LDADD   = -lpopt @LIBFERRIS_LA@ 
interlace_LDFLAGS = @GLIB_LIBS@


