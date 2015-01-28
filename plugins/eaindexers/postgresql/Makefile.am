
plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexpostgresql.la libeaindexpostgresql_factory.la
libeaindexpostgresql_la_SOURCES         = libeaindexpostgresql.cpp
libeaindexpostgresql_factory_la_SOURCES = libeaindexpostgresql_factory.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@LIBPQXX_CFLAGS@                                \
	@CFLAGS@

libeaindexpostgresql_la_LDFLAGS = @GLIB_LIBS@   \
	@LIBPQXX_LIBS@                          \
	@LIBFERRIS_PLUGIN_LT_LINE@

libeaindexpostgresql_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

