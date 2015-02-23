
plugindir = $(libdir)/ferris/plugins/fulltextindexers

plugin_LTLIBRARIES = libftxidxstrigi.la libftxidxstrigi_factory.la
libftxidxstrigi_la_SOURCES = libftxidxstrigi.cpp
libftxidxstrigi_factory_la_SOURCES = libftxidxstrigi_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@DBUS_CFLAGS@					\
	@STRIGI_CFLAGS@                                    \
	@CFLAGS@

libftxidxstrigi_la_LDFLAGS = @STRIGI_LIBS@ @DBUS_LIBS@     \
	-L/usr/local/lib -lsearchclient \
	@LIBFERRIS_PLUGIN_LT_LINE@
libftxidxstrigi_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

