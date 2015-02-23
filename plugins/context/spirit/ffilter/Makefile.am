
plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib  -I$(srcdir)/.. -I$(srcdir)/../.. -I$(top_srcdir)/src -I$(includedir) \
	-I/foooooooooo \
	-I/usr/local/lib                                \
	@CFLAGS@


plugin_LTLIBRARIES = libffilterspirit.la libffilterspirit_factory.la

libffilterspirit_factory_la_SOURCES = libffilterspirit_factory.cpp
libffilterspirit_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libffilterspirit_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libffilterspirit_la_SOURCES = libffilterspirit.cpp 
libffilterspirit_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

noinst_PROGRAMS = ffiltertest
ffiltertest_SOURCES = ffiltertest.cpp
ffiltertest_LDADD = -lpopt @LIBFERRIS_LA@  libffilterspirit.la
ffiltertest_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

