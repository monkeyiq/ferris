
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fmodestr2octal

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

fmodestr2octal_SOURCES = fmodestr2octal.cpp
fmodestr2octal_LDADD = -lpopt @LIBFERRIS_LA@ 
fmodestr2octal_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

