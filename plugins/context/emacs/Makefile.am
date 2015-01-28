
if FERRIS_HAVE_EMACS_MOUNT_ABILITY
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                = libferrisemacs.la libferrisemacs_factory.la
libferrisemacs_factory_la_SOURCES = libferrisemacs_factory.cpp
libferrisemacs_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisemacs_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisemacs_la_SOURCES         = libferrisemacs.cpp 

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@EMACS_MOUNT_CFLAGS@                                  \
	@CFLAGS@

libferrisemacs_la_LDFLAGS =                           \
	@EMACS_MOUNT_LIBS@                                  \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

endif
