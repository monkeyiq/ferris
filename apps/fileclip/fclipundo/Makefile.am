
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fclipundo

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

fclipundo_SOURCES = fclipundo.cpp
fclipundo_LDADD   = -lpopt @LIBFERRIS_LA@ 
fclipundo_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

