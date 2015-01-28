
plugindir = $(libdir)/ferris/plugins/context

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@DBXML_CFLAGS@                                  \
	@CFLAGS@

plugin_LTLIBRARIES =  libferrisdbxml.la libferrisdbxml_factory.la

libferrisdbxml_factory_la_SOURCES = libferrisdbxml_factory.cpp
libferrisdbxml_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdbxml_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisdbxml_la_SOURCES = libferrisdbxml.cpp dbxmlwrappers.hh
libferrisdbxml_la_LDFLAGS = @DBXML_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
