EXTRA_DIST = 

bin_PROGRAMS = FerrisXalanTransform

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@XML4C_CFLAGS@                                  \
	@XALAN_CFLAGS@                                  \
	@CFLAGS@

FerrisXalanTransform_SOURCES = FerrisXalanTransform.cpp 
FerrisXalanTransform_LDADD = -lpopt @LIBFERRIS_LA@ @LIBFERRISXSLT_LA@
FerrisXalanTransform_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 



lib_LTLIBRARIES        = libferrisxslt.la

ferrisxsltincludedir=$(includedir)/FerrisXSLT
ferrisxsltinclude_HEADERS = \
	libferrisxslt.hh

libferrisxslt_la_SOURCES      =  \
	xslt_base.cpp            \
	xslt_base.hh             \
	xslt_shell.cpp           \
	xslt_ferris.cpp		 \
	xslt_string.cpp		 \
	xslt_fs.cpp

libferrisxslt_la_LIBADD       = $(LIBS_ALL)  @GLIB_LIBS@ @XML4C_LIBS@ @XALAN_LIBS@ @LIBFERRIS_LA@
libferrisxslt_la_LDFLAGS      = -version-info 1:0:0 -luuid

