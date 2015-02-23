if HAVE_STRIGI

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@STRIGI_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisstrigi.la 
libferrisstrigi_la_SOURCES = libferrisstrigi.cpp
libferrisstrigi_la_LDFLAGS = @GLIB_LIBS@ @STRIGI_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
