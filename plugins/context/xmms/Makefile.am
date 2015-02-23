
if HAVE_XMMS
plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBXMMS_CFLAGS@                                \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

plugin_LTLIBRARIES                = libferrisxmms.la libferrisxmms_factory.la
libferrisxmms_factory_la_SOURCES = libferrisxmms_factory.cpp
libferrisxmms_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisxmms_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisxmms_la_SOURCES         = libferrisxmms.cpp 


libferrisxmms_la_LDFLAGS =                              \
	@GLIB_LIBS@                 	                \
	@LIBXMMS_LIBS@                                  \
	@LIBFERRIS_PLUGIN_LT_LINE@                        

endif
