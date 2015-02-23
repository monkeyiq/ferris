
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@TDB_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationtdb.la 
libcreationtdb_la_SOURCES = libcreationtdb.cpp
libcreationtdb_la_LDFLAGS = @GLIB_LIBS@ @TDB_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
