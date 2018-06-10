
plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@PULSEAUDIO_CFLAGS@                             \
	@CFLAGS@

plugin_LTLIBRARIES = libferrispulseaudio.la libferrispulseaudio_factory.la

libferrispulseaudio_factory_la_SOURCES = libferrispulseaudio_factory.cpp
libferrispulseaudio_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispulseaudio_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrispulseaudio_la_SOURCES = libferrispulseaudio.cpp 
libferrispulseaudio_la_LDFLAGS = @PULSEAUDIO_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

