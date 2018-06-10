
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@MAGICK_CFLAGS@                                 \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationmagick.la 
libcreationmagick_la_SOURCES = libcreationmagick.cpp
libcreationmagick_la_LDFLAGS = @GLIB_LIBS@ @MAGICK_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
