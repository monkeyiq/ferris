
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gfdl

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GTK_CFLAGS@                                    \
	@CFLAGS@

gfdl_SOURCES = gfdl.cpp
gfdl_LDADD = \
	@LIBFERRIS_LA@ \
	@LIBFERRISUI_LA@ \
	-lpopt
gfdl_LDFLAGS = $(GTK_LIBS)

