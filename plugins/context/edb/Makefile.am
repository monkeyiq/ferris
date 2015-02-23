
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libedb.la libedb_factory.la
libedb_factory_la_SOURCES = libedb_factory.cpp
libedb_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libedb_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libedb_la_SOURCES = libedb.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)                             \
	@EDB_CFLAGS@                                    \
	@CFLAGS@

libedb_la_LDFLAGS = @GLIB_LIBS@       \
	@EDB_LIBS@                    \
	@LIBFERRIS_PLUGIN_LT_LINE@
