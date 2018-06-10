
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                      		\
	@DB4_CFLAGS@                                    \
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-logging

ferris_capplet_logging_SOURCES = ferris-capplet-logging.cpp 
ferris_capplet_logging_LDADD = -lpopt @LIBFERRIS_LA@ @LIBFERRISUI_LA@
ferris_capplet_logging_LDFLAGS = @GTK_LIBS@ @DB4_LIBS@


