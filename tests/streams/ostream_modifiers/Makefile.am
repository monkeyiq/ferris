
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = ostream_modifiers

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@ \
	@CFLAGS@

ostream_modifiers_SOURCES = ostream_modifiers.cpp
ostream_modifiers_LDADD = -lpopt @LIBFERRIS_LA@ 
ostream_modifiers_LDFLAGS = $(GTK_LIBS)

