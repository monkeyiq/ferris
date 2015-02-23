
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

XML_FILES = query.fxsql query.xsl queryform.fxsql queryform.xsl
EXTRA_DIST = $(XML_FILES)
HTTP_INSTALL_DIR = /var/www/html/ferrisql


xmldir = $(HTTP_INSTALL_DIR)
xml_DATA = $(XML_FILES)

if FERRIS_HAVE_CGICC

cgibindir = /var/www/cgi-bin
cgibin_PROGRAMS = CGIFerrisTransform

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@XERCESC_CFLAGS@                                \
	@XALAN_CFLAGS@                                  \
	@CFLAGS@

CGIFerrisTransform_SOURCES = CGIFerrisTransform.cpp
CGIFerrisTransform_LDADD = -lpopt @LIBFERRIS_LA@ 
CGIFerrisTransform_LDFLAGS = @GLIB_LIBS@ @XERCESC_LIBS@ @XALAN_LIBS@ \
	-L/usr/lib -lcgicc


endif

