
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisdtl.la libferrisdtl_factory.la
libferrisdtl_factory_la_SOURCES = libferrisdtl_factory.cpp
libferrisdtl_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdtl_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisdtl_la_SOURCES         = libferrisdtl.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	$(GNOME_INCLUDEDIR)                             \
	@DTL_CFLAGS@                                    \
	-I@COMMONSQLDBDIR@                              \
	@CFLAGS@

libferrisdtl_la_LDFLAGS = @GLIB_LIBS@    \
	libferrisdtlshared.la            \
	@DTL_LIBS@                       \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisdtlshared.la
libferrisdtlshared_la_SOURCES = libferrisdtlshared.cpp libferrisdtlshared.hh
