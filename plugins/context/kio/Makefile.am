MOC_DEP_SRCS=libferriskio_shared_moc.cpp

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
	@KDE_EAGENERATOR_CFLAGS@                        \
	@CFLAGS@

plugin_LTLIBRARIES = libferriskio.la libferriskio_factory.la
libferriskio_factory_la_SOURCES = libferriskio_factory.cpp
libferriskio_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriskio_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferriskio_la_SOURCES = libferriskio.cpp libferriskio_shared.hh
libferriskio_la_LDFLAGS = @CTXKIO_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @KDE_EAGENERATOR_LIBS@


noinst_LTLIBRARIES = libferriskio_shared.la
nodist_libferriskio_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferriskio_shared_la_SOURCES = \
	libferriskio_shared.cpp libferriskio_shared.hh

