
plugindir = @FERRIS_AS_TEXT_PLUGIN_DIR@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBDJVULIBRE_CFLAGS@ @IMLIB_CFLAGS@            \
	@CFLAGS@

plugin_LTLIBRARIES = libastextman.la 
libastextman_la_SOURCES = libastextman.cpp
libastextman_la_LDFLAGS = @GLIB_LIBS@    \
	@LIBDJVULIBRE_LIBS@ @IMLIB_LIBS@ \
	@LIBFERRIS_ASTEXTPLUGIN_LT_LINE@
