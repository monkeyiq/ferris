
plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@SOPRANO_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES =  libferrissoprano.la libferrissoprano_factory.la

libferrissoprano_factory_la_SOURCES = libferrissoprano_factory.cpp
libferrissoprano_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@ 
libferrissoprano_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrissoprano_la_SOURCES = libferrissoprano.cpp
libferrissoprano_la_LDFLAGS = @SOPRANO_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
