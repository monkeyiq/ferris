
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES              = libferristdb.la libferristdb_factory.la
libferristdb_factory_la_SOURCES = libferristdb_factory.cpp
libferristdb_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferristdb_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferristdb_la_SOURCES         = libferristdb.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@TDB_CFLAGS@                                    \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferristdb_la_LDFLAGS =                               \
	@GLIB_LIBS@                                     \
	@TDB_LIBS@                                      \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@

noinst_PROGRAMS = tdbmaker

tdbmaker_SOURCES = tdbmaker.cpp
tdbmaker_LDADD = -lpopt @LIBFERRIS_LA@ -ltdb
tdbmaker_LDFLAGS = @GLIB_LIBS@
