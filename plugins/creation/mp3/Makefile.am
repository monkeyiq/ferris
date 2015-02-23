
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@MP3_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationmp3.la 
libcreationmp3_la_SOURCES = libcreationmp3.cpp
libcreationmp3_la_LDFLAGS = @GLIB_LIBS@ @MP3_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
