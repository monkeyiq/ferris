
if FERRIS_HAVE_AMAROK
plugindir = $(libdir)/ferris/plugins/context

EXTRA_DIST = tracklist_interface.xml player_interface.xml
BUILT_SOURCES = player_interface.cpp player_interface.h tracklist_interface.cpp tracklist_interface.h

player_interface.cpp player_interface.h: player_interface.xml
	qdbusxml2cpp -N -m -i dbus-extra-types.hh -c player -p player_interface $(srcdir)/player_interface.xml 
	$(MOC) player_interface.h -o player_interface.moc

tracklist_interface.cpp tracklist_interface.h: tracklist_interface.xml
	qdbusxml2cpp -N -m -c tracklist -p tracklist_interface $(srcdir)/tracklist_interface.xml 
	$(MOC) tracklist_interface.h -o tracklist_interface.moc



INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@LIBAMAROK_CFLAGS@                                \
	@GLIB_CFLAGS@                                   \
	@QT_CFLAGS@ @QTNETWORK_CFLAGS@                  \
	@QTDBUS_CFLAGS@                                 \
	@CFLAGS@

plugin_LTLIBRARIES                = libferrisamarok.la libferrisamarok_factory.la
libferrisamarok_factory_la_SOURCES = libferrisamarok_factory.cpp
libferrisamarok_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisamarok_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@



libferrisamarok_la_SOURCES         = \
	libferrisamarok.cpp \
	dbus-extra-types.hh \
	player_interface.cpp \
	tracklist_interface.cpp

libferrisamarok_la_LDFLAGS =                              \
	@GLIB_LIBS@                 	                \
	@LIBAMAROK_LIBS@                                  \
	@QTDBUS_LIBS@                                \
	@LIBFERRIS_PLUGIN_LT_LINE@                        

endif
