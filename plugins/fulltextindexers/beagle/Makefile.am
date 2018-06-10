if BUILD_BEAGLE_FTXIDX

plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxbeagle.la libftxidxbeagle_factory.la
libftxidxbeagle_la_SOURCES = libftxidxbeagle.cpp
libftxidxbeagle_factory_la_SOURCES = libftxidxbeagle_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@BEAGLE_CFLAGS@                                 \
	@CFLAGS@

libftxidxbeagle_la_LDFLAGS         = @BEAGLE_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libftxidxbeagle_factory_la_LDFLAGS =               @LIBFERRIS_PLUGIN_LT_LINE@


endif
