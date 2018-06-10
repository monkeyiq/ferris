
plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexsoprano.la libeaindexsoprano_factory.la
libeaindexsoprano_la_SOURCES = libeaindexsoprano.cpp
libeaindexsoprano_factory_la_SOURCES = libeaindexsoprano_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@SOPRANO_CFLAGS@                                \
	@CFLAGS@

libeaindexsoprano_la_LDFLAGS = @GLIB_LIBS@    \
	@SOPRANO_LIBS@                      \
	@LIBFERRIS_PLUGIN_LT_LINE@

libeaindexsoprano_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

