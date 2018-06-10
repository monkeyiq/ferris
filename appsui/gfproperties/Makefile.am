
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gfproperties

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GTK_CFLAGS@                                    \
	@CFLAGS@

gfproperties_SOURCES = gfproperties.cpp
gfproperties_LDADD = \
	@LIBFERRIS_LA@ \
	@LIBFERRISUI_LA@ \
	-lpopt
gfproperties_LDFLAGS = $(GTK_LIBS)

