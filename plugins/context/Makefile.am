

EXTRA_DIST = FerrisContextPlugin.hh gdbm.h SpiritContext.hh

INCLUDES = -I$(top_srcdir)/plugins/context @CFLAGS@

BUILT_SOURCES=

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
BUILT_SOURCES = $(MOC_DEP_SRCS)
CLEANFILES = $(MOC_DEP_SRCS)




noinst_LTLIBRARIES = libcommondbapi.la libcommonsqldbapi.la
libcommondbapi_la_SOURCES = libcommondbapi.cpp libcommondbapi.hh
libcommonsqldbapi_la_SOURCES = libcommonsqldbapi.cpp libcommonsqldbapi.hh

plugindir = $(libdir)/ferris/plugins/context

##############################
##############################
##############################

noinst_PROGRAMS = \
	libffilterspirittest \
	libfulltextbooleanspirittest
plugin_LTLIBRARIES = \
	libffilterspirit.la libffilterspirit_factory.la \
	libfulltextbooleanspirit.la libfulltextbooleanspirit_factory.la \
	libferrisrecordfile.la libferrisrecordfile_factory.la


libffilterspirit_factory_la_SOURCES = libffilterspirit_factory.cpp
libffilterspirit_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libffilterspirit_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libffilterspirit_la_SOURCES = libffilterspirit.cpp 
libffilterspirit_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libffilterspirittest_SOURCES = libffilterspirittest.cpp
libffilterspirittest_LDADD = -lpopt @LIBFERRIS_LA@  libffilterspirit.la
libffilterspirittest_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@


libfulltextbooleanspirit_factory_la_SOURCES = libfulltextbooleanspirit_factory.cpp
libfulltextbooleanspirit_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libfulltextbooleanspirit_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libfulltextbooleanspirit_la_SOURCES = libfulltextbooleanspirit.cpp 
libfulltextbooleanspirit_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libfulltextbooleanspirittest_SOURCES = libfulltextbooleanspirittest.cpp
libfulltextbooleanspirittest_LDADD = -lpopt @LIBFERRIS_LA@  libfulltextbooleanspirit.la
libfulltextbooleanspirittest_LDFLAGS = @GLIB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

##############################
##############################
##############################

if HAVE_DB4
plugin_LTLIBRARIES+=libferrisdb4.la libferrisdb4_factory.la
endif

libferrisdb4_factory_la_SOURCES = libferrisdb4_factory.cpp
libferrisdb4_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdb4_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisdb4_la_SOURCES         = libferrisdb4.cpp
libferrisdb4_la_INCLUDES = $(INCLUDES) @DB4_CFLAGS@ @GLIB_CFLAGS@ 
libferrisdb4_la_LIBADD =                                \
	@COMMONDB_LA@                                   \
	@LIBFERRIS_PLUGIN_LIBADD@
libferrisdb4_la_LDFLAGS =                               \
	@GLIB_LIBS@                                     \
	@DB4_LIBS@                                      \
	@LIBFERRIS_PLUGIN_LT_LINE@                      

##############################
##############################
##############################

libferrisrecordfile_factory_la_SOURCES = libferrisrecordfile_factory.cpp
libferrisrecordfile_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisrecordfile_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisrecordfile_la_SOURCES         = libferrisrecordfile.cpp
libferrisrecordfile_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

##############################
##############################
##############################

if HAVE_EVOLUTION
plugin_LTLIBRARIES+=libferrisevolution.la libferrisevolution_factory.la
endif

libferrisevolution_factory_la_SOURCES = libferrisevolution_factory.cpp
libferrisevolution_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisevolution_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisevolution_la_SOURCES         = libferrisevolution.cpp
libferrisevolution_la_LDFLAGS =                         \
	@EVOLUTION_LIBS@                                \
	@LIBFERRIS_PLUGIN_LT_LINE@                      \
	@COMMONDB_LA@
libferrisevolution_la_INCLUDES = $(INCLUDES) @EVOLUTION_CFLAGS@

##############################
##############################
##############################

if HAVE_LIBPQXX
noinst_LTLIBRARIES += libferrispostgresqlshared.la
plugin_LTLIBRARIES += libferrispostgresql.la libferrispostgresql_factory.la
endif

libferrispostgresql_factory_la_SOURCES = libferrispostgresql_factory.cpp
libferrispostgresql_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispostgresql_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrispostgresql_la_SOURCES         = libferrispostgresql.cpp
libferrispostgresql_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/libferrispostgresqlshared.la \
	@LIBPQXX_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libferrispostgresql_la_INCLUDES = $(INCLUDES) @LIBPQXX_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@

libferrispostgresqlshared_la_SOURCES = libferrispostgresqlshared.cpp libferrispostgresqlshared.hh
libferrispostgresqlshared_la_INCLUDES = $(INCLUDES) @LIBPQXX_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@

##############################
##############################
##############################

EXTRA_DIST+=klipper_interface.xml
if FERRIS_HAVE_ECORE
ECORE_SOURCES=libferrisxwin_ecore.cpp libferrisxwin_ecore.hh
endif


if FERRIS_HAVE_KDE
BUILT_SOURCES+= klipper_interface.cpp klipper_interface.h klipper_interface_public.hh

klipper_interface.cpp klipper_interface.h: klipper_interface.xml
	qdbusxml2cpp -c klipper -p klipper_interface $(srcdir)/klipper_interface.xml 

klipper_interface_public.hh: klipper_interface.h
	$(MOC) klipper_interface.h -o klipper_interface_public.hh

KLIPPER_SOURCES=libferrisxwin_klipper.cpp libferrisxwin_klipper.hh \
	klipper_interface.cpp klipper_interface_public.hh
endif


if FERRIS_BUILD_XWIN_CONTEXT
plugin_LTLIBRARIES               += libferrisxwin.la libferrisxwin_factory.la
endif

libferrisxwin_factory_la_SOURCES = libferrisxwin_factory.cpp
libferrisxwin_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisxwin_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisxwin_la_SOURCES         = libferrisxwin.cpp $(ECORE_SOURCES) $(KLIPPER_SOURCES)
libferrisxwin_la_INCLUDES        = @ECORE_CFLAGS@ @QTDBUS_CFLAGS@ @QTNETWORK_CFLAGS@ @CFLAGS@
libferrisxwin_la_LDFLAGS         = @ECORE_LIBS@ @QTDBUS_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@                   



##############################
##############################
##############################

if FERRIS_HAVE_XSLTFS_MOUNT_ABILITY
plugin_LTLIBRARIES                += libferrisxsltfs.la libferrisxsltfs_factory.la
endif

libferrisxsltfs_factory_la_SOURCES = libferrisxsltfs_factory.cpp
libferrisxsltfs_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisxsltfs_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisxsltfs_la_SOURCES         = libferrisxsltfs.cpp 
libferrisxsltfs_la_INCLUDES = @XSLTFS_MOUNT_CFLAGS@ @CFLAGS@
libferrisxsltfs_la_LDFLAGS = @XSLTFS_MOUNT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@                        

##############################
##############################
##############################


if FERRIS_HAVE_ANNODEX
plugin_LTLIBRARIES += libferrisannodex.la libferrisannodex_factory.la
endif

libferrisannodex_factory_la_SOURCES = libferrisannodex_factory.cpp
libferrisannodex_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisannodex_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisannodex_la_SOURCES   = libferrisannodex.cpp 
libferrisannodex_la_INCLUDES  = @ANNODEX_CFLAGS@ @CMML_CFLAGS@ @CFLAGS@
libferrisannodex_la_LDFLAGS   = @ANNODEX_LIBS@ @CMML_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@                   

##############################
##############################
##############################

if FERRIS_HAVE_EMACS_MOUNT_ABILITY
plugin_LTLIBRARIES += libferrisemacs.la libferrisemacs_factory.la
endif

libferrisemacs_factory_la_SOURCES = libferrisemacs_factory.cpp
libferrisemacs_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisemacs_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisemacs_la_SOURCES         = libferrisemacs.cpp 
libferrisemacs_la_INCLUDES        = @EMACS_MOUNT_CFLAGS@ @CFLAGS@
libferrisemacs_la_LDFLAGS         = @EMACS_MOUNT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@                   

##############################
##############################
##############################

plugin_LTLIBRARIES += libferriswiki.la libferriswiki_factory.la

libferriswiki_factory_la_SOURCES = libferriswiki_factory.cpp
libferriswiki_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriswiki_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferriswiki_la_SOURCES = libferriswiki.cpp 
libferriswiki_la_LDFLAGS = @QTNETWORK_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ \
	\$(top_builddir)/plugins/context/libferriswikishared.la
libferriswiki_la_INCLUDES = -I/usr/local/lib @QTNETWORK_CFLAGS@ @CFLAGS@

noinst_LTLIBRARIES += libferriswikishared.la
nodist_libferriswikishared_la_SOURCES = libferriswikishared_moc.cpp
libferriswikishared_la_SOURCES = libferriswikishared.cpp libferriswikishared.hh 

##############################
##############################
##############################


plugin_LTLIBRARIES += libferrispastebin.la libferrispastebin_factory.la

libferrispastebin_factory_la_SOURCES = libferrispastebin_factory.cpp
libferrispastebin_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispastebin_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrispastebin_la_SOURCES = libferrispastebin.cpp 
libferrispastebin_la_LDFLAGS = @QTNETWORK_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ \
	\$(top_builddir)/plugins/context/libferrispastebinshared.la
libferrispastebin_la_INCLUDES = -I/usr/local/lib @QTNETWORK_CFLAGS@ @CFLAGS@

noinst_LTLIBRARIES += libferrispastebinshared.la
nodist_libferrispastebinshared_la_SOURCES = libferrispastebinshared_moc.cpp
libferrispastebinshared_la_SOURCES = libferrispastebinshared.cpp libferrispastebinshared.hh 


##############################
##############################
##############################

# pastebin


if HAVE_PCCTS
PCCTSDIR = pccts
endif

if HAVE_EDB
EDBDIR = edb
endif

if HAVE_LIBMPEG3
MPEG3DIR = libmpeg3
endif

if HAVE_XML4C
XMLDIR = xml
endif

if HAVE_SQLPLUS
SQLPLUSDIR = sqlplus
endif

if HAVE_DVDREAD
DVDREADDIR = dvdread
endif

if HAVE_IPC_CONTEXT
IPCDIR = ipc
endif


if HAVE_LDAP
LDAPDIR = ldap
endif

if HAVE_EET
EETDIR = eet
endif

if HAVE_GDBM
GDBMDIR = gdbm
endif

if HAVE_TDB
TDBDIR = tdb
endif

if HAVE_DBXML
DBXMLDIR = dbxml
endif

if HAVE_SOPRANO
SOPRANO = soprano
endif

if HAVE_DTL
DTL = dtl
endif

if HAVE_SOCKETPP
SOCKETS = sockets
endif

if HAVE_GPHOTO2
GPHOTO2 = gphoto2
endif

if BUILD_DBUS_MOUNTING
DBUSDIR = dbus
HALDIR = hal
endif

if HAVE_QTSQL
QTSQLDIR = qtsql
endif

if HAVE_QTNETWORK
QTFTPDIR = qtftp
QTHTTPDIR = qthttp
endif

if HAVE_GSTREAMER
GSTREAMERDIR = gstreamer
endif

if HAVE_LIBPLATINUM
UPNPDIR = upnp
endif

if FERRIS_HAVE_PLASMA
PLASMADIR = plasma
endif

if FERRIS_HAVE_SANE
SANEDIR = sane
endif

if FERRIS_HAVE_QPRINTER
QPRINTERDIR = printer
endif

if HAVE_PULSEAUDIO
PULSEDIR = pulseaudio
endif

if FERRIS_BUILD_KDE_EAGENERATOR
KIODIR = kio
endif

SUBDIRS =                                           \
	$(MPEG3DIR) $(XMLDIR)  $(EDBDIR)            \
	nativembox $(SOCKETS) external       \
	$(SQLPLUSDIR) $(DVDREADDIR)                 \
	$(IPCDIR)  $(LDAPDIR) $(EETDIR)    \
	$(GDBMDIR) $(TDBDIR) $(DBXMLDIR) $(SOPRANO) \
	$(DTL) $(GPHOTO2) bibtex \
	obby xmms amarok \
	$(DBUSDIR) $(HALDIR) $(QTSQLDIR)  \
	google webphotos facebook vimeo \
	$(GSTREAMERDIR) $(UPNPDIR) $(QTFTPDIR) $(QTHTTPDIR) \
	$(SANEDIR) $(QPRINTERDIR) $(PLASMADIR) $(PULSEDIR) \
	identica zoneminder ferrisrest \
	$(KIODIR) firefox

