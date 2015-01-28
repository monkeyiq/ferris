
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxyahoo.la libftxidxyahoo_factory.la
libftxidxyahoo_la_SOURCES = libftxidxyahoo.cpp
libftxidxyahoo_factory_la_SOURCES = libftxidxyahoo_factory.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

libftxidxyahoo_la_LDFLAGS         = @LIBFERRIS_PLUGIN_LT_LINE@
libftxidxyahoo_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

