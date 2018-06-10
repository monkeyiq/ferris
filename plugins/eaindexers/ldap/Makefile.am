
plugindir = $(libdir)/ferris/plugins/eaindexers

plugin_LTLIBRARIES = libeaindexldap.la libeaindexldap_factory.la
libeaindexldap_la_SOURCES = libeaindexldap.cpp libeaindexldap.hh
libeaindexldap_factory_la_SOURCES = libeaindexldap_factory.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I..                                            \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@LDAP_CFLAGS@                                   \
	@CFLAGS@

libeaindexldap_la_LDFLAGS = @GLIB_LIBS@    \
	@LDAP_LIBS@                      \
	@LIBFERRIS_PLUGIN_LT_LINE@
libeaindexldap_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

