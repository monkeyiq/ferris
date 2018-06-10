
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@SOPRANO_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationsopranoredland.la 
libcreationsopranoredland_la_SOURCES = libcreationsopranoredland.cpp
libcreationsopranoredland_la_LDFLAGS = @SOPRANO_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
