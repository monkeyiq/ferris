
plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexxapian.la libeaindexxapian_factory.la
libeaindexxapian_la_SOURCES = libeaindexxapian.cpp
libeaindexxapian_factory_la_SOURCES = libeaindexxapian_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@XAPIAN_CFLAGS@                                 \
	@CFLAGS@

libeaindexxapian_la_LDFLAGS = @GLIB_LIBS@    \
	@XAPIAN_LIBS@                      \
	@SHAREDXAPIANEA_LA@                \
	@LIBFERRIS_PLUGIN_LT_LINE@

libeaindexxapian_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

