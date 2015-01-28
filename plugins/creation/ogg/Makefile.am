
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@OGG_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationogg.la 
libcreationogg_la_SOURCES = libcreationogg.cpp
libcreationogg_la_LDFLAGS = @GLIB_LIBS@ @OGG_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
