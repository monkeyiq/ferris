
plugindir = $(libdir)/ferris/plugins/context

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@QTDBUS_CFLAGS@                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

plugin_LTLIBRARIES               = libferrishal.la libferrishal_factory.la
libferrishal_factory_la_SOURCES = libferrishal_factory.cpp
libferrishal_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrishal_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrishal_la_SOURCES         = libferrishal.cpp  libferrishal_private.hh


libferrishal_la_LDFLAGS =                            \
	@GLIB_LIBS@                 	             \
	@QTDBUS_LIBS@                                 \
	@LIBFERRIS_PLUGIN_LT_LINE@                        

