MOC_DEP_SRCS=libferrisidentica_shared_moc.cpp

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
	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisidentica.la libferrisidentica_factory.la
libferrisidentica_factory_la_SOURCES = libferrisidentica_factory.cpp
libferrisidentica_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisidentica_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisidentica_la_SOURCES = libferrisidentica.cpp libferrisidentica_shared.hh
libferrisidentica_la_LDFLAGS = @CTXIDENTICA_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@


noinst_LTLIBRARIES = libferrisidentica_shared.la
nodist_libferrisidentica_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisidentica_shared_la_SOURCES = \
	libferrisidentica_shared.cpp libferrisidentica_shared.hh


# noinst_PROGRAMS = identicatest
# identicatest_SOURCES = identicatest.cpp libferrisidentica_shared.hh
# identicatest_LDADD = -lpopt @LIBFERRIS_LA@ 
# identicatest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXIDENTICA_LA@

