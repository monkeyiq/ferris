MOC_DEP_SRCS=libferriswikishared_moc.cpp

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
	@QTNETWORK_CFLAGS@                              \
	@CFLAGS@

plugin_LTLIBRARIES = libferriswiki.la libferriswiki_factory.la

libferriswiki_factory_la_SOURCES = libferriswiki_factory.cpp
libferriswiki_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriswiki_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferriswiki_la_SOURCES = libferriswiki.cpp 
libferriswiki_la_LDFLAGS = @QTNETWORK_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ \
	\$(top_builddir)/plugins/context/wiki/libferriswikishared.la

noinst_LTLIBRARIES = libferriswikishared.la
nodist_libferriswikishared_la_SOURCES = $(MOC_DEP_SRCS)
libferriswikishared_la_SOURCES = libferriswikishared.cpp libferriswikishared.hh 
