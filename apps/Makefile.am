NULL=

if HAVE_LIBPQXX
PQXX_DEPENDANT_CODE = fca
endif

if HAVE_LIBPLATINUM
UPNPDIR = upnpserver
endif

SUBDIRS = \
	fileclip cat importdesktop  \
	fmkdir                      \
	fmodestr2octal              \
	oprocdeamon                 \
	fcompress                   \
	fulltextindex               \
	ftouch                      \
	fschema                     \
	fmedallion                  \
	ferriscd                    \
	ai                          \
	namespaces                  \
	cursors                     \
	$(PQXX_DEPENDANT_CODE)      \
	ls                          \
	copy                        \
	mv rm                       \
	xml                         \
	head tail                   \
	fileactions		    \
	ferrisredirect              \
	rdf xmmsdaemon              \
	metadataserver              \
	phpsearchinterface          \
	frunner                     \
	volumemanager               \
	du                          \
	scripts                     \
	$(UPNPDIR)                  \
	tools rest yuibrowser


bin_SCRIPTS = $(srcdir)/*.sh $(srcdir)/*.pl libferris-googleearth ferris-rdfproc fedit ferris-internal-extract-subtitles-to-format
EXTRA_DIST = $(srcdir)/*.sh $(srcdir)/*.pl libferris-googleearth ferris-rdfproc fedit ferris-internal-extract-subtitles-to-format

#
# These were moved to plugins/context/webphotos so that they are closer to the shared
# library they depend on.
#
# bin_PROGRAMS = \
# 	ferris-webphoto-upload \
# 	ferris-webphoto-remote-url-to-eaindex-predicate

# INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
# 	@STLPORT_CFLAGS@                                \
# 	@SIGC_CFLAGS@                                   \
# 	-I. 						\
# 	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
# 	-I$(top_builddir)				\
#         -I$(includedir)                                 \
# 	-I/usr/local/include \
# 	@GLIB_CFLAGS@ \
# 	@QT_CFLAGS@ @QTNETWORK_CFLAGS@ \
# 	@CFLAGS@

# ferris_webphoto_upload_SOURCES = ferris-webphoto-upload.cpp
# ferris_webphoto_upload_LDADD = -lpopt @LIBFERRIS_LA@ @SHAREDWEBPHOTOS_LA@
# ferris_webphoto_upload_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@

# ferris_webphoto_remote_url_to_eaindex_predicate_SOURCES = ferris-webphoto-remote-url-to-eaindex-predicate.cpp
# ferris_webphoto_remote_url_to_eaindex_predicate_LDADD = -lpopt @LIBFERRIS_LA@ @SHAREDWEBPHOTOS_LA@
# ferris_webphoto_remote_url_to_eaindex_predicate_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@



