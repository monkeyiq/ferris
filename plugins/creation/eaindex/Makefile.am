
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = \
	libcreationeaindex.la         \
	libcreationeaindexdb4tree.la  \
	libcreationeaindexnull.la 

libcreationeaindex_la_SOURCES = libcreationeaindex.cpp
libcreationeaindex_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@

libcreationeaindexdb4tree_la_SOURCES = libcreationeaindexdb4tree.cpp
libcreationeaindexdb4tree_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@

libcreationeaindexnull_la_SOURCES = libcreationeaindexnull.cpp
libcreationeaindexnull_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
