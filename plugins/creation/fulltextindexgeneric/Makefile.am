
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libcreationfulltextindexgeneric.la
libcreationfulltextindexgeneric_la_SOURCES = libcreationfulltextindexgeneric.cpp
libcreationfulltextindexgeneric_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@

