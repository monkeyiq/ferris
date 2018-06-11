MAINTAINERCLEANFILES = Makefile.in

if HAVE_XALAN
TARGETS = fcreate
endif

bin_PROGRAMS = $(TARGETS)

INCLUDES = @GLIB_CFLAGS@ @CFLAGS@ @XML4C_CFLAGS@ @XALAN_CFLAGS@                   

noinst_LTLIBRARIES = libfcreate.la
if HAVE_GTK2
if HAVE_XALAN_GLADE
noinst_LTLIBRARIES += libfcreategtk.la
endif
endif
libfcreate_la_SOURCES = ferriscreatecommon.cpp ferriscreatecommon.hh
libfcreategtk_la_SOURCES = ferriscreatecommon.cpp ferriscreatecommon.hh
libfcreategtk_la_CXXFLAGS = -DHAVE_GTK2=1 $(INCLUDES) @GTK_CFLAGS@ @GLADE_CFLAGS@ @XML4C_CFLAGS@ @XALAN_CFLAGS@ 


fcreate_SOURCES = fcreate.cpp fcreate.hh 
fcreate_LDADD   = -lpopt 
fcreate_LDFLAGS = @GTK_LIBS@ @XML4C_LIBS@ @XALAN_LIBS@ @LIBFERRIS_LA@ libfcreate.la

