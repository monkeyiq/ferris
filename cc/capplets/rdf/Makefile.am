
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SOPRANO_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                                    \
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-rdf

ferris_capplet_rdf_SOURCES = ferris-capplet-rdf.cpp 
ferris_capplet_rdf_LDADD = -lpopt @LIBFERRIS_LA@ @LIBFERRISUI_LA@
ferris_capplet_rdf_LDFLAGS = @GTK_LIBS@ @SOPRANO_LIBS@


