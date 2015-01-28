if HAVE_LIBJPEG

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBJPEG_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisjpeg.la 
libferrisjpeg_la_SOURCES = libferrisjpeg.cpp
libferrisjpeg_la_LDFLAGS = @GLIB_LIBS@   \
	@LIBJPEG_LIBS@                   \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
