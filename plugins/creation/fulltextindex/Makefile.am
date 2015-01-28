plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libcreationfulltextindex.la
libcreationfulltextindex_la_SOURCES = libcreationfulltextindex.cpp
libcreationfulltextindex_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@ @FTXIDXCUSTOMFERRIS_LA@


