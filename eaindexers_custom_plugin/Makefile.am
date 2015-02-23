
plugindir = $(libdir)/ferris/plugins/eaindexers

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(top_srcdir)/Indexing 			\
	-I$(top_srcdir)         			\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libeaidxcustomferris.la libeaidxcustomferrisdb4tree.la

libeaidxcustomferris_la_SOURCES = libeaidxcustomferris.cpp libeaidxcustomferris.hh
libeaidxcustomferris_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@ @LIBFERRISINDEXING_LA@ @FTXIDXCUSTOMFERRIS_LA@

libeaidxcustomferrisdb4tree_la_SOURCES = libeaidxcustomferrisdb4tree.cpp libeaidxcustomferrisdb4tree.hh
libeaidxcustomferrisdb4tree_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@ @LIBFERRISINDEXING_LA@ @FTXIDXCUSTOMFERRIS_LA@

