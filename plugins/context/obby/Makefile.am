
if FERRIS_HAVE_OBBY_MOUNT_ABILITY
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES                = libferrisobby.la libferrisobby_factory.la
libferrisobby_factory_la_SOURCES = libferrisobby_factory.cpp
libferrisobby_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisobby_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisobby_la_SOURCES         = libferrisobby.cpp gselectorglib.hh

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@OBBY_MOUNT_CFLAGS@                             \
	@CFLAGS@

libferrisobby_la_LDFLAGS =                              \
	libferrisobbyshared.la                          \
	@OBBY_MOUNT_LIBS@                               \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

noinst_LTLIBRARIES = libferrisobbyshared.la
libferrisobbyshared_la_SOURCES = libferrisobbyshared.cpp libferrisobbyshared.hh

endif
