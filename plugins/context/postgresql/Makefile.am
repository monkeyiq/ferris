if HAVE_LIBPQXX

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrispostgresql.la libferrispostgresql_factory.la
libferrispostgresql_factory_la_SOURCES = libferrispostgresql_factory.cpp
libferrispostgresql_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispostgresql_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrispostgresql_la_SOURCES         = libferrispostgresql.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@LIBPQXX_CFLAGS@                                \
	-I@COMMONSQLDBDIR@                              \
	@CFLAGS@

libferrispostgresql_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/postgresql/libferrispostgresqlshared.la            \
	@LIBPQXX_LIBS@                          \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrispostgresqlshared.la
libferrispostgresqlshared_la_SOURCES = libferrispostgresqlshared.cpp libferrispostgresqlshared.hh

endif
