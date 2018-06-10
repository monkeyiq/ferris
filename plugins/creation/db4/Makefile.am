
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@DB4_CFLAGS@                                    \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationdb4.la 
libcreationdb4_la_SOURCES = libcreationdb4.cpp
libcreationdb4_la_LDFLAGS = @GLIB_LIBS@ @DB4_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
