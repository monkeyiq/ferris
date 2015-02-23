MOC_DEP_SRCS=libferriswebphotos_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(srcdir)/..                                  \
	-I$(top_builddir)				\
	-I$(includedir)					\
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@                                 \
	@CFLAGS@

plugin_LTLIBRARIES = libferriswebphotos.la libferriswebphotos_factory.la
libferriswebphotos_factory_la_SOURCES = libferriswebphotos_factory.cpp
libferriswebphotos_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriswebphotos_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferriswebphotos_la_SOURCES         = libferriswebphotos.cpp libferriswebphotos_shared.hh
libferriswebphotos_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/webphotos/libferriswebphotos_shared.la      \
	@QT_LIBS@                                                                    \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferriswebphotos_shared.la
nodist_libferriswebphotos_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferriswebphotos_shared_la_SOURCES = \
	libferriswebphotos_shared.cpp libferriswebphotos_shared.hh 


noinst_PROGRAMS = webphotostest
webphotostest_SOURCES = webphotostest.cpp
webphotostest_LDADD = -lpopt @LIBFERRIS_LA@ 
webphotostest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ \
	\$(top_builddir)/plugins/context/webphotos/libferriswebphotos_shared.la


bin_PROGRAMS = \
	ferris-webphoto-upload \
	ferris-webphoto-remote-url-to-eaindex-predicate

ferris_webphoto_upload_SOURCES = ferris-webphoto-upload.cpp
ferris_webphoto_upload_LDADD = -lpopt @LIBFERRIS_LA@ @SHAREDWEBPHOTOS_LA@
ferris_webphoto_upload_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@

ferris_webphoto_remote_url_to_eaindex_predicate_SOURCES = ferris-webphoto-remote-url-to-eaindex-predicate.cpp
ferris_webphoto_remote_url_to_eaindex_predicate_LDADD = -lpopt @LIBFERRIS_LA@ @SHAREDWEBPHOTOS_LA@
ferris_webphoto_remote_url_to_eaindex_predicate_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@

