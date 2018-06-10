
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES              = libferrisldap.la libferrisldap_factory.la
libferrisldap_factory_la_SOURCES = libferrisldap_factory.cpp
libferrisldap_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisldap_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisldap_la_SOURCES         = libferrisldap.cpp

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LDAP_CFLAGS@                                   \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

libferrisldap_la_LDFLAGS =                               \
	@GLIB_LIBS@                                      \
	@LDAP_LIBS@                                      \
	libferrisldapshared.la                          \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisldapshared.la
libferrisldapshared_la_SOURCES = libferrisldapshared.cpp libferrisldapshared.hh
