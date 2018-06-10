
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisdvdread.la libferrisdvdread_factory.la
libferrisdvdread_factory_la_SOURCES = libferrisdvdread_factory.cpp
libferrisdvdread_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdvdread_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisdvdread_la_SOURCES = libferrisdvdread.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@DVDREAD_CFLAGS@                                \
	@CFLAGS@

libferrisdvdread_la_LDFLAGS = @DVDREAD_LIBS@ @GLIB_LIBS@ \
	@LIBFERRIS_PLUGIN_LT_LINE@
