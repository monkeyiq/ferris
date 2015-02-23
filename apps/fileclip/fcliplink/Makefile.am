
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fcliplink

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

fcliplink_SOURCES = fcliplink.cpp
fcliplink_LDADD = -lpopt @LIBFERRIS_LA@ 
fcliplink_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

