if HAVE_WV

plugindir = @FERRIS_AS_TEXT_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libastextwv.la 
libastextwv_la_SOURCES = libastextwv.cpp
libastextwv_la_LDFLAGS = @GLIB_LIBS@   \
	@LIBFERRIS_ASTEXTPLUGIN_LT_LINE@

endif
