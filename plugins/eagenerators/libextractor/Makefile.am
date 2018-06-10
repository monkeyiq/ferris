if HAVE_LIBEXTRACTOR

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBEXTRACTOR_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisextractor.la 
libferrisextractor_la_SOURCES = libferrisextractor.cpp
libferrisextractor_la_LDFLAGS = @GLIB_LIBS@ @LIBEXTRACTOR_LIBS@        \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
