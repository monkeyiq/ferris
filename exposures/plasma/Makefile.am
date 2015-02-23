plugindir = /usr/lib64/kde4
desktopdir      = /usr/share/kde4/services/
operationsdir   = /usr/share/kde4/apps/plasma/services/
desktop_DATA    = $(srcdir)/plasma-engine-libferris.desktop
operations_DATA = $(srcdir)/libferris.operations
EXTRA_DIST = $(desktop_DATA) $(operations_DATA) 

MOC_DEP_SRCS=plasma_engine_libferris_moc.cpp

CLEANFILES = $(MOC_DEP_SRCS)
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@PLASMA_CFLAGS@                                 \
	@CFLAGS@

plugin_LTLIBRARIES = libplasma_engine_libferris.la
libplasma_engine_libferris_la_SOURCES = mocimpls.cpp plasma_engine_libferris.cpp plasma_engine_libferris.hh
libplasma_engine_libferris_la_LDFLAGS = @PLASMA_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @LIBFERRIS_LA@ 

