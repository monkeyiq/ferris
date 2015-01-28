
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrissqlplus.la libferrissqlplus_factory.la
libferrissqlplus_factory_la_SOURCES = libferrissqlplus_factory.cpp
libferrissqlplus_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrissqlplus_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrissqlplus_la_SOURCES         = libferrissqlplus.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	$(GNOME_INCLUDEDIR)                             \
	@XML4C_CFLAGS@                                  \
	@SQLPLUS_CFLAGS@                                \
	-I@COMMONSQLDBDIR@                              \
	@CFLAGS@

libferrissqlplus_la_LDFLAGS = @GLIB_LIBS@               \
	@SQLPLUS_LIBS@ libferrissqlplusshared.la        \
	@XML4C_LIBS@                                    \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrissqlplusshared.la
libferrissqlplusshared_la_SOURCES = libferrissqlplusshared.cpp libferrissqlplusshared.hh

