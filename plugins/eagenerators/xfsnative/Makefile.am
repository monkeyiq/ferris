if HAVE_LIBATTR

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBATTR_CFLAGS@                                \
	@CFLAGS@

plugin_LTLIBRARIES = libxfsnative.la 
libxfsnative_la_SOURCES   	= libxfsnative.cpp
libxfsnative_la_LDFLAGS = @GLIB_LIBS@       \
	@LIBATTR_LIBS@                      \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
