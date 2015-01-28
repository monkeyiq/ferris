MOC_DEP_SRCS=libferrisfacebook_shared_moc.cpp

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
	@CFLAGS@

plugin_LTLIBRARIES = libferrisfacebook.la libferrisfacebook_factory.la
libferrisfacebook_factory_la_SOURCES = libferrisfacebook_factory.cpp
libferrisfacebook_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisfacebook_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisfacebook_la_SOURCES         = libferrisfacebook.cpp libferrisfacebook_shared.hh
libferrisfacebook_la_LDFLAGS = @CTXFACEBOOK_LA@ @SHAREDWEBPHOTOS_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@


noinst_LTLIBRARIES = libferrisfacebook_shared.la
nodist_libferrisfacebook_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisfacebook_shared_la_SOURCES = \
	libferrisfacebook_shared.cpp libferrisfacebook_shared.hh


noinst_PROGRAMS = facebooktest
facebooktest_SOURCES = facebooktest.cpp libferrisfacebook_shared.hh
facebooktest_LDADD = -lpopt @LIBFERRIS_LA@ 
facebooktest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXFACEBOOK_LA@ @SHAREDWEBPHOTOS_LA@

