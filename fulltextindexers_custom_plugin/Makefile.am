
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxcustomferris.la
libftxcustomferris_la_SOURCES = libftxcustomferris.cpp libftxcustomferris.hh

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir) 				\
	-I$(includedir)					\
	@CFLAGS@

libftxcustomferris_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

