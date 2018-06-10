MOC_DEP_SRCS=libferrisvimeo_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@                                 \
	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisvimeo.la libferrisvimeo_factory.la
libferrisvimeo_factory_la_SOURCES = libferrisvimeo_factory.cpp
libferrisvimeo_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisvimeo_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisvimeo_la_SOURCES = libferrisvimeo.cpp libferrisvimeo_shared.hh
libferrisvimeo_la_LDFLAGS = @CTXVIMEO_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@


noinst_LTLIBRARIES = libferrisvimeo_shared.la
nodist_libferrisvimeo_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisvimeo_shared_la_SOURCES = \
	libferrisvimeo_shared.cpp libferrisvimeo_shared.hh


noinst_PROGRAMS = vimeotest
vimeotest_SOURCES = vimeotest.cpp libferrisvimeo_shared.hh
vimeotest_LDADD = -lpopt @LIBFERRIS_LA@ 
vimeotest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXVIMEO_LA@ @QOAUTH_LIBS@ @QJSON_LIBS@

