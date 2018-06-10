
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferrisls

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@GLIB_CFLAGS@ \
	@CFLAGS@

ferrisls_SOURCES = ls.cpp
ferrisls_LDADD = -lpopt @LIBFERRIS_LA@ 
ferrisls_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

