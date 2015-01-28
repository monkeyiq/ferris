MOC_DEP_SRCS=libferrisboxcom_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisboxcom.la libferrisboxcom_factory.la
libferrisboxcom_factory_la_SOURCES = libferrisboxcom_factory.cpp
libferrisboxcom_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisboxcom_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisboxcom_la_SOURCES         = libferrisboxcom.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@ @QTNETWORK_CFLAGS@              \
	@CFLAGS@

libferrisboxcom_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/boxcom/libferrisboxcom_shared.la           \
	@QT_LIBS@                                                                   \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisboxcom_shared.la
nodist_libferrisboxcom_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisboxcom_shared_la_SOURCES = \
	libferrisboxcom_shared.cpp libferrisboxcom_shared.hh 


noinst_PROGRAMS = boxcomtest
boxcomtest_SOURCES = boxcomtest.cpp
boxcomtest_LDADD = -lpopt @LIBFERRIS_LA@ 
boxcomtest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXBOXCOM_LA@

