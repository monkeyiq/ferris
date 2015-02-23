
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES =  libxml.la libxml_factory.la
libxml_factory_la_SOURCES = libxml_factory.cpp
libxml_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libxml_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libxml_la_SOURCES = libxml.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@XML4C_CFLAGS@                                  \
	@GTK_CFLAGS@  					\
	@CFLAGS@

libxml_la_LDFLAGS = @GLIB_LIBS@ @XML4C_LIBS@  \
	@LIBFERRIS_PLUGIN_LT_LINE@


