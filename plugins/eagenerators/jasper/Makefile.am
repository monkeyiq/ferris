if HAVE_LIBJASPER

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@GTK_CFLAGS@                                    \
	@LIBJASPER_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisjasper.la 
libferrisjasper_la_SOURCES = libferrisjasper.cpp
libferrisjasper_la_LDFLAGS = @GTK_LIBS@  \
	@LIBJASPER_LIBS@                 \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
