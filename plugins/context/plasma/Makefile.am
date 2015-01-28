MOC_DEP_SRCS=libferrisplasma_moc.cpp

CLEANFILES = $(MOC_DEP_SRCS)
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
%_moc.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@


plugindir = $(libdir)/ferris/plugins/context

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@PLASMA_CFLAGS@                                 \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisplasma.la libferrisplasma_factory.la

libferrisplasma_factory_la_SOURCES = libferrisplasma_factory.cpp
libferrisplasma_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisplasma_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisplasma_la_SOURCES = mocimpls.cpp libferrisplasma.cpp 
libferrisplasma_la_LDFLAGS = @PLASMA_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

