
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES        = libipc.la libipc_factory.la
libipc_factory_la_SOURCES = libipc_factory.cpp
libipc_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libipc_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libipc_la_SOURCES         = libipc.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@IPC_CONTEXT_CFLAGS@                            \
	@CFLAGS@

libipc_la_LDFLAGS =                                     \
	@GLIB_LIBS@                                     \
	@IPC_CONTEXT_LIBS@                              \
	@LIBFERRIS_PLUGIN_LT_LINE@
