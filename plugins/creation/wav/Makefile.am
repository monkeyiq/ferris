
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@WAV_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationwav.la 
libcreationwav_la_SOURCES = libcreationwav.cpp
libcreationwav_la_LDFLAGS = @GLIB_LIBS@ @WAV_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
