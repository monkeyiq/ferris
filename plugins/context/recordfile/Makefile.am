plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisrecordfile.la libferrisrecordfile_factory.la
libferrisrecordfile_factory_la_SOURCES = libferrisrecordfile_factory.cpp
libferrisrecordfile_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisrecordfile_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisrecordfile_la_SOURCES         = libferrisrecordfile.cpp
libferrisrecordfile_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	-I@COMMONSQLDBDIR@                              \
	@CFLAGS@


