
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                  = libferrisgphoto2.la libferrisgphoto2_factory.la
libferrisgphoto2_factory_la_SOURCES = libferrisgphoto2_factory.cpp
libferrisgphoto2_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgphoto2_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgphoto2_la_SOURCES         = libferrisgphoto2.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@GPHOTO2_CFLAGS@                                \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferrisgphoto2_la_LDFLAGS =                           \
	@GLIB_LIBS@                                     \
	@GPHOTO2_LIBS@                                  \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@
