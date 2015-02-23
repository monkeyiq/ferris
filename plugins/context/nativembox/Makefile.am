
plugindir = $(libdir)/ferris/plugins/context

noinst_PROGRAMS = mbox

plugin_LTLIBRARIES = libnativembox.la libnativembox_factory.la
libnativembox_factory_la_SOURCES = libnativembox_factory.cpp
libnativembox_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libnativembox_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libnativembox_la_SOURCES = libnativembox.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)                             \
	@CFLAGS@

mbox_SOURCES = mbox.cpp
mbox_LDADD   = -lpopt @LIBFERRIS_LA@ 
mbox_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
