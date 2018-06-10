
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES              = libferrisdb4.la libferrisdb4_factory.la
libferrisdb4_factory_la_SOURCES = libferrisdb4_factory.cpp
libferrisdb4_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdb4_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisdb4_la_SOURCES         = libferrisdb4.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@DB4_CFLAGS@                                    \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferrisdb4_la_LIBADD =                                \
	@COMMONDB_LA@                                   \
	@LIBFERRIS_PLUGIN_LIBADD@

libferrisdb4_la_LDFLAGS =                               \
	@GLIB_LIBS@                                     \
	@DB4_LIBS@                                      \
	@LIBFERRIS_PLUGIN_LT_LINE@                      
