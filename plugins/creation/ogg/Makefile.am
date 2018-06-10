
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@OGG_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationogg.la 
libcreationogg_la_SOURCES = libcreationogg.cpp
libcreationogg_la_LDFLAGS = @GLIB_LIBS@ @OGG_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
