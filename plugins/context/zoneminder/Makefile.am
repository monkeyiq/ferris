MOC_DEP_SRCS=libferriszoneminder_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@                                 \
	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferriszoneminder.la libferriszoneminder_factory.la
libferriszoneminder_factory_la_SOURCES = libferriszoneminder_factory.cpp
libferriszoneminder_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriszoneminder_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferriszoneminder_la_SOURCES = libferriszoneminder.cpp libferriszoneminder_shared.hh
libferriszoneminder_la_LDFLAGS = @CTXZONEMINDER_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@


noinst_LTLIBRARIES = libferriszoneminder_shared.la
nodist_libferriszoneminder_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferriszoneminder_shared_la_SOURCES = \
	libferriszoneminder_shared.cpp libferriszoneminder_shared.hh

