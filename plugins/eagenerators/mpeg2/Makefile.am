if HAVE_LIBMPG2

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBMPG2_CFLAGS@				\
	@CFLAGS@

plugin_LTLIBRARIES = libferrismpeg2.la 
libferrismpeg2_la_SOURCES = libferrismpeg2.cpp #video_out.c
libferrismpeg2_la_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_EAPLUGIN_LT_LINE@

#	@LIBMPG2_LIBS@

endif
