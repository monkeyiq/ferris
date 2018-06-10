if HAVE_LIBA52

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBA52_CFLAGS@					\
	@CFLAGS@

plugin_LTLIBRARIES = libferrisa52.la 
libferrisa52_la_SOURCES = libferrisa52.cpp
libferrisa52_la_LDFLAGS = @GLIB_LIBS@   \
	@LIBA52_LIBS@                   \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
