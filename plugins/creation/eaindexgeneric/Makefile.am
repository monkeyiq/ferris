
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libcreationeaindexgeneric.la 
libcreationeaindexgeneric_la_SOURCES = libcreationeaindexgeneric.cpp
libcreationeaindexgeneric_la_LDFLAGS = @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
