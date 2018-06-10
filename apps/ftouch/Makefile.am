
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ftouch

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

ftouch_SOURCES = ftouch.cpp
ftouch_LDADD = -lpopt @LIBFERRIS_LA@ 
ftouch_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 


