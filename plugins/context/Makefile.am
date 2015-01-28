
EXTRA_DIST = FerrisContextPlugin.hh

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

if HAVE_DB4
DB4DIR = db4
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
	commondbapi commonsqldbapi                  \
	$(MPEG3DIR) $(XMLDIR)  $(EDBDIR)            \
	spirit nativembox $(SOCKETS) external       \
	$(SQLPLUSDIR) $(DVDREADDIR)                 \
	$(IPCDIR) $(DB4DIR) $(LDAPDIR) $(EETDIR)    \
	$(GDBMDIR) $(TDBDIR) $(DBXMLDIR) $(SOPRANO) \
	$(DTL) $(GPHOTO2) evolution postgresql bibtex \
	xwin annodex emacs firefox obby xsltfs xmms amarok \
	$(DBUSDIR) $(HALDIR) $(QTSQLDIR) recordfile \
	google webphotos facebook vimeo \
	$(GSTREAMERDIR) $(UPNPDIR) $(QTFTPDIR) $(QTHTTPDIR) \
	$(SANEDIR) $(QPRINTERDIR) $(PLASMADIR) $(PULSEDIR) \
	wiki pastebin identica zoneminder ferrisrest \
	$(KIODIR)

