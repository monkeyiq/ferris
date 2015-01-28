
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES              = libferrisgdbm.la libferrisgdbm_factory.la
libferrisgdbm_factory_la_SOURCES = libferrisgdbm_factory.cpp
libferrisgdbm_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgdbm_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgdbm_la_SOURCES         = libferrisgdbm.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I@COMMONDBDIR@                                 \
	@GDBM_CFLAGS@                                   \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferrisgdbm_la_LDFLAGS =                               \
	@GLIB_LIBS@                                     \
	@GDBM_LIBS@                                      \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@

noinst_PROGRAMS = gdbmaker

gdbmaker_SOURCES = gdbmaker.cpp
gdbmaker_LDADD = -lpopt @LIBFERRIS_LA@ -lgdbm
gdbmaker_LDFLAGS = @GLIB_LIBS@
