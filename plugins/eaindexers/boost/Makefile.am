
if HAVE_BOOST_IPC

plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexboostmmap.la libeaindexboostmmap_factory.la
libeaindexboostmmap_la_SOURCES         = libeaindexboostmmap.cpp mmap_dynamic_bitset.hpp
libeaindexboostmmap_factory_la_SOURCES = libeaindexboostmmap_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-fpermissive                                    \
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

libeaindexboostmmap_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libeaindexboostmmap_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

endif
