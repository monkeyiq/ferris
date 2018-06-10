
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libcreationmpeg2.la 
libcreationmpeg2_la_SOURCES = libcreationmpeg2.cpp
libcreationmpeg2_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
