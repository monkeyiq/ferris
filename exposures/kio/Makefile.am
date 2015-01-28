plugindir = /usr/lib64/kde4
protocoldir      = /usr/share/kde4/services/
protocol_DATA    = $(srcdir)/kio_ferris.protocol
EXTRA_DIST = $(protocol_DATA)

MOC_DEP_SRCS=kio_ferris_moc.cpp

CLEANFILES = $(MOC_DEP_SRCS)
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

INCLUDES =						\
	@KDE_CFLAGS@                                 \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	-I/usr/local/lib                                \
	@CFLAGS@

plugin_LTLIBRARIES = libkio_ferris.la
libkio_ferris_la_SOURCES = mocimpls.cpp kio_ferris.cpp kio_ferris.hh
libkio_ferris_la_LDFLAGS = @KDE_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @LIBFERRIS_LA@ 

