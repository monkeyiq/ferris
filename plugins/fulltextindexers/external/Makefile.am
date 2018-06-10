
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxexternal.la libftxidxexternal_factory.la
libftxidxexternal_la_SOURCES = libftxidxexternal.cpp
libftxidxexternal_factory_la_SOURCES = libftxidxexternal_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

libftxidxexternal_la_LDFLAGS         = @LIBFERRIS_PLUGIN_LT_LINE@
libftxidxexternal_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


perlplugindir = /usr/local/ferris/perl/
perlplugin_SCRIPTS = FerrisFulltextExternalIndex.pm FerrisFulltextExternalIndexExample.pl
EXTRA_DIST = FerrisFulltextExternalIndex.pm FerrisFulltextExternalIndexExample.pl
