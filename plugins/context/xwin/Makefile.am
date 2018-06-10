if FERRIS_HAVE_ECORE
ECORE_SOURCES=libferrisxwin_ecore.cpp libferrisxwin_ecore.hh
endif

EXTRA_DIST=klipper_interface.xml

if FERRIS_HAVE_KDE
BUILT_SOURCES = klipper_interface.cpp klipper_interface.h klipper_interface_public.hh

klipper_interface.cpp klipper_interface.h: klipper_interface.xml
	qdbusxml2cpp -c klipper -p klipper_interface $(srcdir)/klipper_interface.xml 

klipper_interface_public.hh: klipper_interface.h
	$(MOC) klipper_interface.h -o klipper_interface_public.hh

KLIPPER_SOURCES=libferrisxwin_klipper.cpp libferrisxwin_klipper.hh \
	klipper_interface.cpp klipper_interface_public.hh
endif

if FERRIS_BUILD_XWIN_CONTEXT
plugindir = $(libdir)/ferris/plugins/context

plugin_LTLIBRARIES               = libferrisxwin.la libferrisxwin_factory.la
libferrisxwin_factory_la_SOURCES = libferrisxwin_factory.cpp
libferrisxwin_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisxwin_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisxwin_la_SOURCES         = libferrisxwin.cpp $(ECORE_SOURCES) $(KLIPPER_SOURCES)

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@ECORE_CFLAGS@                                  \
	@QTDBUS_CFLAGS@ @QTNETWORK_CFLAGS@              \
	@CFLAGS@

libferrisxwin_la_LDFLAGS =                           \
	@ECORE_LIBS@                                 \
	@QTDBUS_LIBS@                                \
	@LIBFERRIS_PLUGIN_LT_LINE@                   

endif
