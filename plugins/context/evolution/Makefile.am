
if HAVE_EVOLUTION

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                  = libferrisevolution.la libferrisevolution_factory.la
libferrisevolution_factory_la_SOURCES = libferrisevolution_factory.cpp
libferrisevolution_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisevolution_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisevolution_la_SOURCES         = libferrisevolution.cpp
libferrisevolution_la_LDFLAGS =                         \
	@EVOLUTION_LIBS@                                \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@EVOLUTION_CFLAGS@                              \
	@CFLAGS@


endif
