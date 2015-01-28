
plugindir = $(libdir)/ferris/plugins/eaindexers

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

plugin_LTLIBRARIES = libeaindexlucene.la libeaindexlucene_factory.la
libeaindexlucene_la_SOURCES = libeaindexlucene.cpp
libeaindexlucene_factory_la_SOURCES = libeaindexlucene_factory.cpp

COMMON_GCJFLAGS = --encoding=ISO-8859-1 -Wno-deprecated

libeaindexlucene_la_LDFLAGS = @GLIB_LIBS@    \
	@GCJ_LIBS@                       \
	@GCJLUCENE_LIBS@                 \
	@FERRISGCJ_LA@                   \
	@LIBFERRIS_PLUGIN_LT_LINE@

libeaindexlucene_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

