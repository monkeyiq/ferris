
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxtsearch2.la libftxidxtsearch2_factory.la
libftxidxtsearch2_la_SOURCES = libftxidxtsearch2.cpp
libftxidxtsearch2_factory_la_SOURCES = libftxidxtsearch2_factory.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@TSEARCH2_CFLAGS@                               \
	@CFLAGS@

libftxidxtsearch2_la_LDFLAGS = @TSEARCH2_LIBS@ \
	@LIBFERRIS_PLUGIN_LT_LINE@
libftxidxtsearch2_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

