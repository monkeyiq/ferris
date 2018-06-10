
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fclipredo

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

fclipredo_SOURCES = fclipredo.cpp
fclipredo_LDADD   = -lpopt @LIBFERRIS_LA@
fclipredo_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

