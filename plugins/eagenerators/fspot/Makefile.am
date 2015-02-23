if HAVE_FSPOT

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@FSPOT_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisfspot.la 
libferrisfspot_la_SOURCES = libferrisfspot.cpp
libferrisfspot_la_LDFLAGS = @GLIB_LIBS@ @FSPOT_LIBS@ \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

endif
