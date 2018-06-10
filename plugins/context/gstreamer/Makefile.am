plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@GSTREAMER_CFLAGS@                              \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisgstreamer.la libferrisgstreamer_factory.la
libferrisgstreamer_factory_la_SOURCES = libferrisgstreamer_factory.cpp
libferrisgstreamer_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgstreamer_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisgstreamer_la_SOURCES = libferrisgstreamer.cpp libferrisgstreamer_shared.hh
libferrisgstreamer_la_LDFLAGS = @CTXGSTREAMER_LA@ @GSTREAMER_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisgstreamer_shared.la
libferrisgstreamer_shared_la_SOURCES = \
	libferrisgstreamer_shared.cpp libferrisgstreamer_shared.hh

noinst_PROGRAMS = gstreamertest
gstreamertest_SOURCES = gstreamertest.cpp libferrisgstreamer_shared.hh
gstreamertest_LDADD = -lpopt @LIBFERRIS_LA@ 
gstreamertest_LDFLAGS = @GSTREAMER_LIBS@ @CTXGSTREAMER_LA@

