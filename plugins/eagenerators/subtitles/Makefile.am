
plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@CFLAGS@

plugin_LTLIBRARIES = libferrissubtitles.la 
libferrissubtitles_la_SOURCES = libferrissubtitles.cpp
libferrissubtitles_la_LDFLAGS = @GLIB_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@


