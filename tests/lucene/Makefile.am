
if HAVE_GCJLUCENE

NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = IndexFiles SearchFiles

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@GCJ_CFLAGS@                                    \
	@GCJLUCENE_CFLAGS@                              \
	@CFLAGS@

IndexFiles_SOURCES = IndexFiles.cpp
IndexFiles_LDADD   = -lpopt @LIBFERRIS_LA@ 
IndexFiles_LDFLAGS = @GLIB_LIBS@ @GCJ_LIBS@ @GCJLUCENE_LIBS@

SearchFiles_SOURCES = SearchFiles.cpp
SearchFiles_LDADD   = -lpopt @LIBFERRIS_LA@ 
SearchFiles_LDFLAGS = @GLIB_LIBS@  @GCJ_LIBS@ @GCJLUCENE_LIBS@





endif
