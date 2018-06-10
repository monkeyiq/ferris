plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@LIBPLATINUM_CFLAGS@                            \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisplatinum.la libferrisplatinum_factory.la

libferrisplatinum_factory_la_SOURCES = libferrisplatinum_factory.cpp
libferrisplatinum_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisplatinum_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisplatinum_la_SOURCES = libferrisplatinum.cpp 
libferrisplatinum_la_LDFLAGS = @LIBPLATINUM_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

