
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libexternal.la libexternal_factory.la libssh_factory.la
libexternal_factory_la_SOURCES = libexternal_factory.cpp mapping.cpp mapping.hh
libexternal_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libexternal_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libexternal_la_SOURCES = libexternal.cpp mapping.cpp mapping.hh
libssh_factory_la_SOURCES = libssh_factory.cpp
libssh_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libssh_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)                             \
	@XML4C_CFLAGS@                                  \
	@CFLAGS@

libexternal_la_LDFLAGS = @GLIB_LIBS@                    \
	@XML4C_LIBS@                                    \
	@LIBFERRIS_PLUGIN_LT_LINE@

SUBDIRS = extfs_modules

install-exec-local:
	cd $(DESTDIR)/$(plugindir); \
	ln -sf libexternal.so libssh.so; 
