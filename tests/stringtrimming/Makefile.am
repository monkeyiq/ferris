
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = stringtrimming

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
        @GTK_CFLAGS@ \
	@CFLAGS@

stringtrimming_SOURCES = stringtrimming.cpp
stringtrimming_LDFLAGS = $(GTK_LIBS)
stringtrimming_LDADD   = -lpopt @LIBFERRIS_LA@ 

