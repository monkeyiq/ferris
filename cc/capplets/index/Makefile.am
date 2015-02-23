
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                                    \
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-index

ferris_capplet_index_SOURCES = ferris-capplet-index.cpp 
ferris_capplet_index_LDADD = -lpopt @LIBFERRIS_LA@ @LIBFERRISUI_LA@
ferris_capplet_index_LDFLAGS = @GTK_LIBS@


