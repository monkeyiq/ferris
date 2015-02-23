if HAVE_SOPRANO

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@SOPRANO_CFLAGS@	                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferrissoprano.la 
libferrissoprano_la_SOURCES = libferrissoprano.cpp
libferrissoprano_la_LDFLAGS = @GLIB_LIBS@ @SOPRANO_LIBS@ \
	libferrissopranoeashared.la                      \
	@LIBFERRIS_EAPLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrissopranoeashared.la
libferrissopranoeashared_la_SOURCES = libferrissopranoeashared.cpp libferrissopranoeashared.hh

endif
