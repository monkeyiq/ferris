
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libmpeg3.la libmpeg3_factory.la
libmpeg3_factory_la_SOURCES = libmpeg3_factory.cpp
libmpeg3_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libmpeg3_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libmpeg3_la_SOURCES = libmpeg3.cpp
libmpeg3_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)                             \
	-I/usr/local/src/libmpeg3-1.3                   \
	@GTK_CFLAGS@                    		\
	@CFLAGS@
