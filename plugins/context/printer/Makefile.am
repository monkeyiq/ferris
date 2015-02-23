MOC_DEP_SRCS=libferrisprinter_moc.cpp

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
	@QPRINTER_CFLAGS@                                  \
	@CFLAGS@

plugin_LTLIBRARIES = libferrisprinter.la libferrisprinter_factory.la

libferrisprinter_factory_la_SOURCES = libferrisprinter_factory.cpp
libferrisprinter_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisprinter_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisprinter_la_SOURCES = mocimpls.cpp libferrisprinter.cpp 
libferrisprinter_la_LDFLAGS = @QPRINTER_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

