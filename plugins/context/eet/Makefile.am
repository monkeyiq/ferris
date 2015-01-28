
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES              = libferriseet.la libferriseet_factory.la
libferriseet_factory_la_SOURCES = libferriseet_factory.cpp
libferriseet_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriseet_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferriseet_la_SOURCES         = libferriseet.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@EET_CFLAGS@                                    \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferriseet_la_LDFLAGS =                               \
	@GLIB_LIBS@                                     \
	@EET_LIBS@                                      \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@
