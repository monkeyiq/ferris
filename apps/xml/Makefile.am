# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

NULL=

if HAVE_XALAN
XALANTESTS = XSLTransform CGITransform
endif

if HAVE_XQILLA
PROGRAMS_NEEDING_XQILLA=ferris-xqilla
endif

SUBDIRS = $(XALANTESTS)

if HAVE_XALAN
XALAN_SOURCES=ferris-filesystem-to-xsltfs-sheets
endif

bin_PROGRAMS = \
	$(XALAN_SOURCES) \
	$(PROGRAMS_NEEDING_XQILLA) \
	ferris-xml-edit

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@GLIB_CFLAGS@ \
	@CFLAGS@

ferris_filesystem_to_xsltfs_sheets_SOURCES = ferris-filesystem-to-xsltfs-sheets.cpp
ferris_filesystem_to_xsltfs_sheets_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_filesystem_to_xsltfs_sheets_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ferris_xml_edit_SOURCES = ferris-xml-edit.cpp
ferris_xml_edit_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_xml_edit_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ferris_xqilla_SOURCES = ferris-xqilla.cpp
ferris_xqilla_LDADD   = -lpopt @LIBFERRIS_LA@ 
ferris_xqilla_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @XQILLA_LIBS@
ferris_xqilla_CXXFLAGS = $(INCLUDES) @XQILLA_CFLAGS@



XML_FILES = $(srcdir)/*.xfragment
EXTRA_DIST = $(XML_FILES)
XML_INSTALL_DIR = $(datadir)/ferris/ferris-filesystem-to-xsltfs-sheets

xmldir = $(XML_INSTALL_DIR)
xml_DATA = $(XML_FILES)

