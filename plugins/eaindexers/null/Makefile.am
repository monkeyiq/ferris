
plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexnull.la libeaindexnull_factory.la
libeaindexnull_la_SOURCES         = libeaindexnull.cpp
libeaindexnull_factory_la_SOURCES = libeaindexnull_factory.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

libeaindexnull_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libeaindexnull_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

