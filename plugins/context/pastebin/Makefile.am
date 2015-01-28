MOC_DEP_SRCS=libferrispastebinshared_moc.cpp

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
	@QTNETWORK_CFLAGS@                              \
	@CFLAGS@

plugin_LTLIBRARIES = libferrispastebin.la libferrispastebin_factory.la

libferrispastebin_factory_la_SOURCES = libferrispastebin_factory.cpp
libferrispastebin_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispastebin_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrispastebin_la_SOURCES = libferrispastebin.cpp 
libferrispastebin_la_LDFLAGS = @QTNETWORK_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ \
	\$(top_builddir)/plugins/context/pastebin/libferrispastebinshared.la

noinst_LTLIBRARIES = libferrispastebinshared.la
nodist_libferrispastebinshared_la_SOURCES = $(MOC_DEP_SRCS)
libferrispastebinshared_la_SOURCES = libferrispastebinshared.cpp libferrispastebinshared.hh 
