
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libcreationmarkup.la 
libcreationmarkup_la_SOURCES = libcreationmarkup.cpp
libcreationmarkup_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
