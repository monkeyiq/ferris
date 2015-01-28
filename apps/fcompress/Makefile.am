
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fcompress

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@BZIP2_CFLAGS@ @LIBZ_CFLAGS@                    \
	@CFLAGS@

fcompress_SOURCES = fcompress.cpp
fcompress_LDADD = -lpopt @LIBFERRIS_LA@ 
fcompress_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @BZIP2_LIBS@ @LIBZ_LIBS@

