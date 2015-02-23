AUTOMAKE_OPTIONS = dist-bzip2 dist-xz subdir-objects 
ACLOCAL_AMFLAGS = -I m4

NULL=

if HAVE_GTK2
FERRISUIDIR = FerrisUI appsui
endif

if USE_CUSTOMALLOC
CUSTOMALLOCDIR = customalloc
endif

if HAVE_XALAN
XSLTFUNCTIONSDIR = xsltfunctions
endif

if HAVE_GCJ
GCJDIR = FerrisGCJ
endif

if HAVE_QTGUI
FERRISQTDIR = FerrisQtGui
endif

SUBDIRS = lib ThirdParty $(CUSTOMALLOCDIR)              \
	DBusGlue                                        \
	$(GCJDIR) mg Indexing                           \
	factories                                       \
	FerrisGlue                                      \
	Ferris fulltextindexers_custom_plugin eaindexers_custom_plugin apps \
	$(FERRISQTDIR) 					\
	$(FERRISUIDIR)                                  \
	plugins                                         \
	tests dot-ferris                                \
	cc media firsttime $(XSLTFUNCTIONSDIR)          \
	noarch migration perl exposures patches

bin_SCRIPTS = ferris-config

EXTRA_DIST = 		\
ferris-config.in        \
ferris.pc.in            \
ferris.pc               \
ferrisui.pc.in          \
ferrisui.pc             \
ferrisxslt.pc.in        \
ferrisxslt.pc           \
m4/gnulib-cache.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ferris.pc ferrisui.pc ferrisxslt.pc


