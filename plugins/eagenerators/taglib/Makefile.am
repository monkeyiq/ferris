if HAVE_TAGLIB

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@TAGLIB_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferristaglib.la 
libferristaglib_la_SOURCES = libferristaglib.cpp
libferristaglib_la_LDFLAGS = @GLIB_LIBS@ @TAGLIB_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
