NULL=

if HAVE_LIBPQXX
PQXX_DEPENDANT_CODE = fca
endif

if HAVE_LIBPLATINUM
UPNPDIR = upnpserver
endif

SUBDIRS = \
	importdesktop      \
	fulltextindex               \
	$(PQXX_DEPENDANT_CODE)      \
	xml                         \
	rdf                         \
	metadataserver              \
	phpsearchinterface          \
	volumemanager               \
	scripts                     \
	$(UPNPDIR)                  \
	tools rest yuibrowser       \
	fcreate


bin_SCRIPTS = $(srcdir)/*.sh $(srcdir)/*.pl libferris-googleearth ferris-rdfproc fedit ferris-internal-extract-subtitles-to-format
EXTRA_DIST = $(srcdir)/*.sh $(srcdir)/*.pl libferris-googleearth ferris-rdfproc fedit ferris-internal-extract-subtitles-to-format






bin_PROGRAMS = ferrisls fmkdir fcompress fcat ftee ferriscp \
	ferrismv ferrisrm ftail fhead ftouch \
	fmedallion ferris-get-emblems-within-range \
	fschema ferriscdexe fdu \
	fclipcopy fclipcut fcliplink fclippaste fclipredo fclipundo \
	ferris-file-action ferris-set-file-action-for-type \
	ferris-redirect ferris-internal-file-to-fifo-command \
	fagent fagentcreate fnamespace ferris-runner \
	ferris-out-of-proc-notification-deamon \
	fmodestr2octal ferris-cursor



AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib \
	@SIGC_CFLAGS@ @GLIB_CFLAGS@ @CFLAGS@
AM_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ -lpopt @LIBFERRIS_LA@ 


SHELL_FILES = ferriscd
shelldir    = @DOTFILE_ETC@/profile.d/
shell_DATA  = $(SHELL_FILES)
EXTRA_DIST = $(SHELL_FILES)

bin_SCRIPTS = $(srcdir)/*.sh
EXTRA_DIST = $(srcdir)/*.sh



ferrisls_SOURCES  = ferrisls.cpp
fmkdir_SOURCES    = fmkdir.cpp
fcompress_SOURCES = fcompress.cpp
fcat_SOURCES      = fcat.cpp
fcat_LDADD        = $(top_builddir)/lib/libgnu.la
ftee_SOURCES      = ftee.cpp
ftee_LDADD        = $(top_builddir)/lib/libgnu.la
ferriscp_SOURCES  = ferriscp.cpp
ferrismv_SOURCES  = ferrismv.cpp
ferrisrm_SOURCES  = ferrisrm.cpp
ftail_SOURCES     = ftail.cpp
fhead_SOURCES     = fhead.cpp
ftouch_SOURCES    = ftouch.cpp
fmedallion_SOURCES = fmedallion.cpp
ferris_get_emblems_within_range_SOURCES = ferris-get-emblems-within-range.cpp
fschema_SOURCES     = fschema.cpp
ferriscdexe_SOURCES = ferriscdexe.cpp
fdu_SOURCES         = fdu.cpp 
fclipcopy_SOURCES   = fclipcopy.cpp
fclipcut_SOURCES    = fclipcut.cpp
fcliplink_SOURCES   = fcliplink.cpp
fclippaste_SOURCES  = fclippaste.cpp
fclipredo_SOURCES   = fclipredo.cpp
fclipundo_SOURCES   = fclipundo.cpp
ferris_file_action_SOURCES = ferris-file-action.cpp
ferris_set_file_action_for_type_SOURCES = ferris-set-file-action-for-type.cpp
ferris_redirect_SOURCES = ferris-redirect.cpp
ferris_internal_file_to_fifo_command_SOURCES = ferris-internal-file-to-fifo-command.cpp
fagent_SOURCES = fagent.cpp
fagentcreate_SOURCES = fagentcreate.cpp
fnamespace_SOURCES = fnamespace.cpp
ferris_runner_SOURCES = ferris-runner.cpp
ferris_out_of_proc_notification_deamon_SOURCES = ferris-out-of-proc-notification-deamon.cpp


fmodestr2octal_SOURCES = fmodestr2octal.cpp
ferris_cursor_SOURCES = ferris-cursor.cpp





if HAVE_XMMS
bin_PROGRAMS+=ferris-internal-xmms-server
endif

ferris_internal_xmms_server_SOURCES = ferris-internal-xmms-server.cpp
ferris_internal_xmms_server_CXXFLAGS = $(INCLUDES) @LIBXMMS_CFLAGS@
ferris_internal_xmms_server_LDFLAGS = @LIBXMMS_LIBS@ 
