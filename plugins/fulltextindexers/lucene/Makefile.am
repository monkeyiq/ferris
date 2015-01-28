
plugindir = $(libdir)/ferris/plugins/fulltextindexers

noinst_LTLIBRARIES = libftxidxluceneanalyzer.la
libftxidxluceneanalyzer_la_SOURCES = MyAnalyzer.java

plugin_LTLIBRARIES = libftxidxlucene.la libftxidxlucene_factory.la
libftxidxlucene_la_SOURCES = libftxidxlucene.cpp 
libftxidxlucene_factory_la_SOURCES = libftxidxlucene_factory.cpp 

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(top_srcdir)/FerrisGCJ                       \
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@GCJ_CFLAGS@                                    \
	@GCJLUCENE_CFLAGS@                              \
	@CFLAGS@

GCJFLAGS = @GCJLUCENE_GCJFLAGS@ @GCJFLAGS@

libftxidxlucene_la_LDFLAGS = @GLIB_LIBS@    \
	@GCJ_LIBS@                       \
	@GCJLUCENE_LIBS@                 \
	@FERRISGCJ_LA@                   \
	@FTXIDXLUCENEANALYZER_LA@        \
	@LIBFERRIS_PLUGIN_LT_LINE@

libftxidxlucene_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

