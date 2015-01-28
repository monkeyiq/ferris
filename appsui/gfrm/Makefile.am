
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gfrm

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@GTK_CFLAGS@ \
	@CFLAGS@

gfrm_SOURCES = gfrm.cpp
gfrm_LDADD = \
	@LIBFERRIS_LA@ \
	@LIBFERRISUI_LA@ \
	-lpopt
gfrm_LDFLAGS = $(GTK_LIBS)

