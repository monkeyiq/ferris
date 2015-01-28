if HAVE_QTSQL

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisqtsql.la libferrisqtsql_factory.la
libferrisqtsql_factory_la_SOURCES = libferrisqtsql_factory.cpp
libferrisqtsql_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisqtsql_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisqtsql_la_SOURCES         = libferrisqtsql.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@QTSQL_CFLAGS@                                \
	-I@COMMONSQLDBDIR@                              \
	@CFLAGS@

libferrisqtsql_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/qtsql/libferrisqtsqlshared.la            \
	@QTSQL_LIBS@ \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisqtsqlshared.la
libferrisqtsqlshared_la_SOURCES = libferrisqtsqlshared.cpp libferrisqtsqlshared.hh

endif
