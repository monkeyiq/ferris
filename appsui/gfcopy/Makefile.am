
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gfcp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@GTK_CFLAGS@ \
	@CFLAGS@

gfcp_SOURCES = gfcp.cpp
gfcp_LDADD = \
	@LIBFERRIS_LA@ \
	@LIBFERRISUI_LA@ \
	-lpopt
gfcp_LDFLAGS = @GTK_LIBS@


