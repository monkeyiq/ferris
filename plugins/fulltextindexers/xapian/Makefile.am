
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxxapian.la libftxidxxapian_factory.la
libftxidxxapian_la_SOURCES = libftxidxxapian.cpp
libftxidxxapian_factory_la_SOURCES = libftxidxxapian_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@XAPIAN_CFLAGS@                                    \
	@CFLAGS@

libftxidxxapian_la_LDFLAGS = @XAPIAN_LIBS@      \
	libferrisxapianeashared.la            \
	@LIBFERRIS_PLUGIN_LT_LINE@
libftxidxxapian_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


noinst_LTLIBRARIES = libferrisxapianeashared.la
libferrisxapianeashared_la_SOURCES = libferrisxapianeashared.cpp libferrisxapianeashared.hh
