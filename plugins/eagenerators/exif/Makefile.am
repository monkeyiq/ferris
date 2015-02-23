if HAVE_LIBEXIF

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBEXIF_CFLAGS@	                        \
	@LIBJPEG_CFLAGS@	                        \
	@EPEG_CFLAGS@                                   \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisexifea.la 
libferrisexifea_la_SOURCES = libferrisexifea.cpp \
	jpeg-data.c  jpeg-data.h  jpeg-marker.c  jpeg-marker.h
libferrisexifea_la_LDFLAGS = @GLIB_LIBS@ @LIBEXIF_LIBS@ \
	@LIBJPEG_LIBS@                                  \
	@EPEG_LIBS@                                     \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
