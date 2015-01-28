if HAVE_LIBPNG

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBPNG_CFLAGS@	                                \
	@CFLAGS@

plugin_LTLIBRARIES = libferrispng.la 
libferrispng_la_SOURCES = libferrispng.cpp
libferrispng_la_LDFLAGS = @GLIB_LIBS@       \
	@LIBPNG_LIBS@                       \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif

