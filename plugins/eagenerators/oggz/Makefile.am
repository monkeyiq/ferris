if HAVE_OGGZ

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@OGGZ_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisoggz.la 
libferrisoggz_la_SOURCES = libferrisoggz.cpp
libferrisoggz_la_LDFLAGS = @GLIB_LIBS@ @OGGZ_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
