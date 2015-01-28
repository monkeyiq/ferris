
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxnull.la libftxidxnull_factory.la
libftxidxnull_la_SOURCES = libftxidxnull.cpp
libftxidxnull_factory_la_SOURCES = libftxidxnull_factory.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

libftxidxnull_la_LDFLAGS         = @LIBFERRIS_PLUGIN_LT_LINE@
libftxidxnull_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

