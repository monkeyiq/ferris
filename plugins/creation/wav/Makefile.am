
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@WAV_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationwav.la 
libcreationwav_la_SOURCES = libcreationwav.cpp
libcreationwav_la_LDFLAGS = @GLIB_LIBS@ @WAV_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
