
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = dot-desktop-files

bin_PROGRAMS = ferris-import-desktop-file

bin_SCRIPTS = $(srcdir)/*.sh
EXTRA_DIST = $(srcdir)/*.sh


INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

ferris_import_desktop_file_SOURCES = ferris-import-desktop-file.cpp
ferris_import_desktop_file_LDADD   = -lpopt @LIBFERRIS_LA@ 
ferris_import_desktop_file_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

