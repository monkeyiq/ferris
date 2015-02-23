
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@GDBM_CFLAGS@                                   \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationgdbm.la 
libcreationgdbm_la_SOURCES = libcreationgdbm.cpp
libcreationgdbm_la_LDFLAGS = @GLIB_LIBS@ @GDBM_LIBS@ \
	@LIBFERRIS_CREATIONPLUGIN_LT_LINE@
