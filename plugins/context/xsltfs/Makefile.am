
if FERRIS_HAVE_XSLTFS_MOUNT_ABILITY
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                = libferrisxsltfs.la libferrisxsltfs_factory.la
libferrisxsltfs_factory_la_SOURCES = libferrisxsltfs_factory.cpp
libferrisxsltfs_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisxsltfs_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisxsltfs_la_SOURCES         = libferrisxsltfs.cpp 

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                   \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@XSLTFS_MOUNT_CFLAGS@                           \
	@CFLAGS@

libferrisxsltfs_la_LDFLAGS =                              \
	@XSLTFS_MOUNT_LIBS@                               \
	@LIBFERRIS_PLUGIN_LT_LINE@                        

endif
