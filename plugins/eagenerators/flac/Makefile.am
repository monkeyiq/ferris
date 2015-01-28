if HAVE_FLAC

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I/usr/include                                  \
	-I$(includedir)					\
	@FLAC_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisflac.la 
libferrisflac_la_SOURCES = libferrisflac.cpp
libferrisflac_la_LDFLAGS = @GLIB_LIBS@ @FLAC_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
