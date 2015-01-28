
if FERRIS_HAVE_FIREFOX_MOUNT_ABILITY
plugindir = $(libdir)/ferris/plugins/context

SUBDIRS = firefox-extension

plugin_LTLIBRARIES                = libferrisfirefox.la libferrisfirefox_factory.la
libferrisfirefox_factory_la_SOURCES = libferrisfirefox_factory.cpp
libferrisfirefox_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisfirefox_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisfirefox_la_SOURCES         = libferrisfirefox.cpp 

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@FIREFOX_MOUNT_CFLAGS@                                  \
	@CFLAGS@

libferrisfirefox_la_LDFLAGS =                           \
	@FIREFOX_MOUNT_LIBS@                                  \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

endif
