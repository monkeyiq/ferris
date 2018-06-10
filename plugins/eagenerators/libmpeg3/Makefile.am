if HAVE_LIBMPEG3

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@GTK_CFLAGS@                                    \
	@LIBMPEG3_CFLAGS@                               \
	@CFLAGS@

plugin_LTLIBRARIES = libferrismpeg3.la 
libferrismpeg3_la_SOURCES = libferrismpeg3.cpp
libferrismpeg3_la_LDFLAGS = @GTK_LIBS@       \
	@LIBMPEG3_LIBS@                      \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
