MOC_DEP_SRCS=libferrisferrisrest_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@                                 \
	@QJSON_CFLAGS@                                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisferrisrest.la libferrisferrisrest_factory.la
libferrisferrisrest_factory_la_SOURCES = libferrisferrisrest_factory.cpp
libferrisferrisrest_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisferrisrest_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisferrisrest_la_SOURCES = libferrisferrisrest.cpp libferrisferrisrest_shared.hh
libferrisferrisrest_la_LDFLAGS = @CTXFERRISREST_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QJSON_LIBS@


noinst_LTLIBRARIES = libferrisferrisrest_shared.la
nodist_libferrisferrisrest_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisferrisrest_shared_la_SOURCES = \
	libferrisferrisrest_shared.cpp libferrisferrisrest_shared.hh

