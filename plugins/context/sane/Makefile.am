MOC_DEP_SRCS=libferrissane_moc.cpp

CLEANFILES = $(MOC_DEP_SRCS)
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
%_moc.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@


plugindir = $(libdir)/ferris/plugins/context

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@KSANE_CFLAGS@                                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferrissane.la libferrissane_factory.la

libferrissane_factory_la_SOURCES = libferrissane_factory.cpp
libferrissane_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrissane_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrissane_la_SOURCES = mocimpls.cpp libferrissane.cpp 
libferrissane_la_LDFLAGS = @KSANE_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

