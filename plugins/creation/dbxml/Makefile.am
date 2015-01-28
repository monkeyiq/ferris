
plugindir = @FERRIS_CREATION_PLUGIN_DIR@

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@DBXML_CFLAGS@                                  \
	@CFLAGS@

plugin_LTLIBRARIES = libcreationdbxml.la 
libcreationdbxml_la_SOURCES = libcreationdbxml.cpp
libcreationdbxml_la_LDFLAGS = @DBXML_LIBS@ @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
