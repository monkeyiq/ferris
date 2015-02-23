MOC_DEP_SRCS=libferrisqthttp_moc_impl.cpp

%_moc_impl.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)


plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	@CONTEXT_PLUGIN_CFLAGS@                         \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisqthttp.la libferrisqthttp_factory.la 

libferrisqthttp_la_SOURCES         = libferrisqthttp.cpp mocimpls.cpp
libferrisqthttp_la_LDFLAGS = @GLIB_LIBS@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

libferrisqthttp_factory_la_SOURCES = libferrisqthttp_factory.cpp
libferrisqthttp_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisqthttp_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

