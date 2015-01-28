MOC_DEP_SRCS=libferrisgoogle_shared_moc.cpp

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)

plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES = libferrisgoogle.la libferrisgoogle_factory.la
libferrisgoogle_factory_la_SOURCES = libferrisgoogle_factory.cpp
libferrisgoogle_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgoogle_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgoogle_la_SOURCES         = libferrisgoogle.cpp

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

libferrisgoogle_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/google/libferrisgoogle_shared.la           \
	@QT_LIBS@                                                                   \
	@LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES = libferrisgoogle_shared.la
nodist_libferrisgoogle_shared_la_SOURCES = $(MOC_DEP_SRCS)
libferrisgoogle_shared_la_SOURCES = \
	libferrisgoogle_shared.cpp libferrisgoogle_shared.hh 


noinst_PROGRAMS = googletest
googletest_SOURCES = googletest.cpp
googletest_LDADD = -lpopt @LIBFERRIS_LA@ 
googletest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXGOOGLE_LA@

