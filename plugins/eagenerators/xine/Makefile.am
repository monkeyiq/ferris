if FERRIS_BUILD_XINE_EAGENERATOR

plugindir = $(libdir)/ferris/plugins/eagenerators

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@XINE_EAGENERATOR_CFLAGS@                       \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisxineea.la 
libferrisxineea_la_SOURCES = libferrisxineea.cpp
libferrisxineea_la_LDFLAGS = @XINE_EAGENERATOR_LIBS@ @LIBFERRIS_EAPLUGIN_LT_LINE@

endif
