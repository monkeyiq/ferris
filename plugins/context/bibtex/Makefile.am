
if FERRIS_HAVE_LIBBTPARSE

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                  = libferrisbibtex.la libferrisbibtex_factory.la
libferrisbibtex_factory_la_SOURCES = libferrisbibtex_factory.cpp
libferrisbibtex_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisbibtex_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisbibtex_la_SOURCES         = libferrisbibtex.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBBTPARSE_CFLAGS@                             \
	@CFLAGS@

libferrisbibtex_la_LDFLAGS =                         \
	@LIBBTPARSE_LIBS@                            \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

endif
