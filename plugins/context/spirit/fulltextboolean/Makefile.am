
plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib  -I$(srcdir)/.. -I$(srcdir)/../.. -I$(top_srcdir)/src -I$(includedir) \
	-I/foooooooooo \
	-I/usr/local/lib                                \
	@CFLAGS@


plugin_LTLIBRARIES = libfulltextbooleanspirit.la libfulltextbooleanspirit_factory.la

libfulltextbooleanspirit_factory_la_SOURCES = libfulltextbooleanspirit_factory.cpp
libfulltextbooleanspirit_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libfulltextbooleanspirit_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libfulltextbooleanspirit_la_SOURCES = libfulltextbooleanspirit.cpp 
libfulltextbooleanspirit_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

noinst_PROGRAMS = testapp
testapp_SOURCES = testapp.cpp
testapp_LDADD = -lpopt @LIBFERRIS_LA@  libfulltextbooleanspirit.la
testapp_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

