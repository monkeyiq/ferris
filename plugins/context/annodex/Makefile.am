
if FERRIS_HAVE_ANNODEX
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES               = libferrisannodex.la libferrisannodex_factory.la
libferrisannodex_factory_la_SOURCES = libferrisannodex_factory.cpp
libferrisannodex_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisannodex_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisannodex_la_SOURCES         = libferrisannodex.cpp 

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@ANNODEX_CFLAGS@                                \
	@CMML_CFLAGS@                                   \
	@CFLAGS@

libferrisannodex_la_LDFLAGS =                           \
	@ANNODEX_LIBS@                                  \
	@CMML_LIBS@                                     \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

endif
