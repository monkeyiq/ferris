

EXTRA_DIST = FerrisContextPlugin.hh gdbm.h SpiritContext.hh

INCLUDES = -I$(top_srcdir)/plugins/context @CFLAGS@

MOC_DEP_SRCS=
BUILT_SOURCES=
CLEANFILES=

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@
%_moc_impl.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

stub.c: $(noinst_LTLIBRARIES)
	touch stub.c


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
libferrisdb4_la_CXXFLAGS = $(INCLUDES) @DB4_CFLAGS@ @GLIB_CFLAGS@ 
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
libferrisevolution_la_CXXFLAGS = $(INCLUDES) @EVOLUTION_CFLAGS@

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
libferrispostgresql_la_CXXFLAGS = $(INCLUDES) @LIBPQXX_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@

libferrispostgresqlshared_la_SOURCES = libferrispostgresqlshared.cpp libferrispostgresqlshared.hh
libferrispostgresqlshared_la_CXXFLAGS = $(INCLUDES) @LIBPQXX_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@

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
libferrisxwin_la_CXXFLAGS        = @ECORE_CFLAGS@ @QTDBUS_CFLAGS@ @QTNETWORK_CFLAGS@ @CFLAGS@
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
libferrisxsltfs_la_CXXFLAGS = @XSLTFS_MOUNT_CFLAGS@ @CFLAGS@
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
libferrisannodex_la_CXXFLAGS  = @ANNODEX_CFLAGS@ @CMML_CFLAGS@ @CFLAGS@
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
libferrisemacs_la_CXXFLAGS        = @EMACS_MOUNT_CFLAGS@ @CFLAGS@
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
libferriswiki_la_CXXFLAGS = -I/usr/local/lib @QTNETWORK_CFLAGS@ @CFLAGS@

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
libferrispastebin_la_CXXFLAGS = -I/usr/local/lib @QTNETWORK_CFLAGS@ @CFLAGS@

noinst_LTLIBRARIES += libferrispastebinshared.la
nodist_libferrispastebinshared_la_SOURCES = libferrispastebinshared_moc.cpp
libferrispastebinshared_la_SOURCES = libferrispastebinshared.cpp libferrispastebinshared.hh 

##############################
##############################
##############################

MOC_DEP_SRCS+=libferrisgoogle_shared_moc.cpp
GOOGLEINCLUDES = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ @QTNETWORK_CFLAGS@ @CFLAGS@

plugin_LTLIBRARIES += libferrisgoogle.la libferrisgoogle_factory.la

libferrisgoogle_factory_la_SOURCES = libferrisgoogle_factory.cpp
libferrisgoogle_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgoogle_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgoogle_la_SOURCES         = libferrisgoogle.cpp
libferrisgoogle_la_CXXFLAGS = $(GOOGLEINCLUDES)
libferrisgoogle_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/libferrisgoogle_shared.la           \
	@QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@

noinst_LTLIBRARIES += libferrisgoogle_shared.la
nodist_libferrisgoogle_shared_la_SOURCES = libferrisgoogle_shared_moc.cpp
libferrisgoogle_shared_la_SOURCES = libferrisgoogle_shared.cpp libferrisgoogle_shared.hh 
libferrisgoogle_shared_la_CXXFLAGS = $(GOOGLEINCLUDES)

noinst_PROGRAMS += googletest
googletest_SOURCES = googletest.cpp
googletest_LDADD = -lpopt @LIBFERRIS_LA@  libferrisgoogle_shared.la
googletest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXGOOGLE_LA@



##############################
##############################
##############################

MOC_DEP_SRCS=libferriswebphotos_shared_moc.cpp

WEBPHOTOSINCLUDES = @GLIB_CFLAGS@ @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ @CFLAGS@

plugin_LTLIBRARIES += libferriswebphotos.la libferriswebphotos_factory.la
libferriswebphotos_factory_la_SOURCES = libferriswebphotos_factory.cpp
libferriswebphotos_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriswebphotos_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferriswebphotos_la_SOURCES         = libferriswebphotos.cpp libferriswebphotos_shared.hh stub.c
libferriswebphotos_la_LDFLAGS = \
	\$(top_builddir)/plugins/context/libferriswebphotos_shared.la      \
	@QT_LIBS@                                                          \
	@LIBFERRIS_PLUGIN_LT_LINE@
libferriswebphotos_la_CXXFLAGS = $(WEBPHOTOSINCLUDES)

noinst_LTLIBRARIES += libferriswebphotos_shared.la
nodist_libferriswebphotos_shared_la_SOURCES = libferriswebphotos_shared_moc.cpp
libferriswebphotos_shared_la_SOURCES = \
	libferriswebphotos_shared.cpp libferriswebphotos_shared.hh 
libferriswebphotos_shared_la_CXXFLAGS = $(WEBPHOTOSINCLUDES)


noinst_PROGRAMS += webphotostest
webphotostest_SOURCES = webphotostest.cpp
webphotostest_LDADD = -lpopt @LIBFERRIS_LA@ 
webphotostest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ \
	\$(top_builddir)/plugins/context/libferriswebphotos_shared.la
webphotostest_CXXFLAGS = $(WEBPHOTOSINCLUDES)

bin_PROGRAMS = ferris-webphoto-upload ferris-webphoto-remote-url-to-eaindex-predicate

ferris_webphoto_upload_SOURCES = ferris-webphoto-upload.cpp
ferris_webphoto_upload_LDADD = -lpopt @LIBFERRIS_LA@ libferriswebphotos_shared.la
ferris_webphoto_upload_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@
ferris_webphoto_upload_CXXFLAGS = $(WEBPHOTOSINCLUDES)

ferris_webphoto_remote_url_to_eaindex_predicate_SOURCES = ferris-webphoto-remote-url-to-eaindex-predicate.cpp
ferris_webphoto_remote_url_to_eaindex_predicate_LDADD = -lpopt @LIBFERRIS_LA@ @SHAREDWEBPHOTOS_LA@
ferris_webphoto_remote_url_to_eaindex_predicate_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ @QT_LIBS@ @QTNETWORK_LIBS@
ferris_webphoto_remote_url_to_eaindex_predicate_CXXFLAGS = $(WEBPHOTOSINCLUDES)



##############################
##############################
##############################

# MOC_DEP_SRCS=libferrisfacebook_shared_moc.cpp

# FACEBOOKINCLUDES = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ @CFLAGS@

# plugin_LTLIBRARIES += libferrisfacebook.la libferrisfacebook_factory.la
# libferrisfacebook_factory_la_SOURCES  = libferrisfacebook_factory.cpp
# libferrisfacebook_factory_la_LIBADD   = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
# libferrisfacebook_factory_la_LDFLAGS  = @LIBFERRIS_PLUGIN_LT_LINE@
# libferrisfacebook_factory_la_CXXFLAGS = $(FACEBOOKINCLUDES)

# libferrisfacebook_la_SOURCES  = libferrisfacebook.cpp libferrisfacebook_shared.hh
# libferrisfacebook_la_LDFLAGS  = @CTXFACEBOOK_LA@ @SHAREDWEBPHOTOS_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
# libferrisfacebook_la_CXXFLAGS = $(FACEBOOKINCLUDES)

# noinst_LTLIBRARIES += libferrisfacebook_shared.la
# nodist_libferrisfacebook_shared_la_SOURCES = libferrisfacebook_shared_moc.cpp
# libferrisfacebook_shared_la_SOURCES = \
# 	libferrisfacebook_shared.cpp libferrisfacebook_shared.hh
# libferrisfacebook_shared_la_CXXFLAGS = $(FACEBOOKINCLUDES)

# noinst_PROGRAMS += facebooktest
# facebooktest_SOURCES = facebooktest.cpp libferrisfacebook_shared.hh
# facebooktest_LDADD = -lpopt @LIBFERRIS_LA@ 
# facebooktest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXFACEBOOK_LA@ @SHAREDWEBPHOTOS_LA@
# facebooktest_CXXFLAGS = $(FACEBOOKINCLUDES)


##############################
##############################
##############################

MOC_DEP_SRCS=libferrisvimeo_shared_moc.cpp

VIMEOINCLUDES = \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QTDBUS_CFLAGS@                                 \
	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@                  \
	@CFLAGS@

plugin_LTLIBRARIES += libferrisvimeo.la libferrisvimeo_factory.la

libferrisvimeo_factory_la_SOURCES = libferrisvimeo_factory.cpp
libferrisvimeo_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisvimeo_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisvimeo_la_SOURCES = libferrisvimeo.cpp libferrisvimeo_shared.hh
libferrisvimeo_la_LDFLAGS = @CTXVIMEO_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@
libferrisvimeo_la_CXXFLAGS = $(VIMEOINCLUDES)

noinst_LTLIBRARIES += libferrisvimeo_shared.la
nodist_libferrisvimeo_shared_la_SOURCES = libferrisvimeo_shared_moc.cpp
libferrisvimeo_shared_la_SOURCES = \
	libferrisvimeo_shared.cpp libferrisvimeo_shared.hh
libferrisvimeo_shared_la_CXXFLAGS = $(VIMEOINCLUDES)


noinst_PROGRAMS += vimeotest
vimeotest_SOURCES = vimeotest.cpp libferrisvimeo_shared.hh
vimeotest_LDADD = -lpopt @LIBFERRIS_LA@ libferrisvimeo_shared.la
vimeotest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXVIMEO_LA@ @QOAUTH_LIBS@ @QJSON_LIBS@
vimeotest_CXXFLAGS = $(VIMEOINCLUDES)



##############################
##############################
##############################


plugin_LTLIBRARIES += libxml.la libxml_factory.la
libxml_factory_la_SOURCES = libxml_factory.cpp
libxml_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libxml_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libxml_la_SOURCES   = libxml.cpp
libxml_la_LDFLAGS   = @GLIB_LIBS@ @XML4C_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libxml_la_CXXFLAGS  = @XML4C_CFLAGS@ @GTK_CFLAGS@ @CFLAGS@

##############################
##############################
##############################

if HAVE_EDB
plugin_LTLIBRARIES += libedb.la libedb_factory.la
endif

libedb_factory_la_SOURCES = libedb_factory.cpp
libedb_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libedb_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libedb_la_SOURCES = libedb.cpp
libedb_la_LDFLAGS = @GLIB_LIBS@ @EDB_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libedb_la_CXXFLAGS = $(GNOME_INCLUDEDIR) @EDB_CFLAGS@ @CFLAGS@


##############################
##############################
##############################


if HAVE_GDBM
plugin_LTLIBRARIES += libferrisgdbm.la libferrisgdbm_factory.la
noinst_PROGRAMS += gdbmaker
endif

libferrisgdbm_factory_la_SOURCES = libferrisgdbm_factory.cpp
libferrisgdbm_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgdbm_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgdbm_la_SOURCES  = libferrisgdbm.cpp
libferrisgdbm_la_CXXFLAGS = -I@COMMONDBDIR@ @GDBM_CFLAGS@ @GLIB_CFLAGS@ @CFLAGS@
libferrisgdbm_la_LDFLAGS  = @GLIB_LIBS@ @GDBM_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @COMMONDB_LA@

gdbmaker_SOURCES = gdbmaker.cpp
gdbmaker_LDADD = -lpopt @LIBFERRIS_LA@ -lgdbm
gdbmaker_LDFLAGS = @GLIB_LIBS@
gdbmaker_CXXFLAGS = -I@COMMONDBDIR@ @GDBM_CFLAGS@ @GLIB_CFLAGS@ @CFLAGS@

##############################
##############################
##############################

MOC_DEP_SRCS+=libferrisqtftp_moc_impl.cpp


libferrisqtftp_moc_generator.cpp: libferrisqtftp_moc_impl.cpp
	touch libferrisqtftp_moc_generator.cpp

if HAVE_QTNETWORK
plugin_LTLIBRARIES += libferrisqtftp.la libferrisqtftp_factory.la 
endif

libferrisqtftp_la_SOURCES  = libferrisqtftp.cpp libferrisqtftp_moc_generator.cpp
libferrisqtftp_la_LDFLAGS  = @GLIB_LIBS@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libferrisqtftp_la_CXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @CFLAGS@

libferrisqtftp_factory_la_SOURCES = libferrisqtftp_factory.cpp
libferrisqtftp_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisqtftp_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


##############################
##############################
##############################

MOC_DEP_SRCS+=libferrisqthttp_moc_impl.cpp

libferrisqthttp_moc_generator.cpp: libferrisqthttp_moc_impl.cpp
	touch libferrisqthttp_moc_generator.cpp

if HAVE_QTNETWORK
plugin_LTLIBRARIES += libferrisqthttp.la libferrisqthttp_factory.la 
endif

libferrisqthttp_la_SOURCES  = libferrisqthttp.cpp libferrisqthttp_moc_generator.cpp
libferrisqthttp_la_LDFLAGS  = @GLIB_LIBS@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libferrisqthttp_la_CXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @CFLAGS@
libferrisqthttp_factory_la_SOURCES = libferrisqthttp_factory.cpp
libferrisqthttp_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisqthttp_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


##############################
##############################
##############################

# MOC_DEP_SRCS+=libferrissane_moc_impl.cpp

# sane_moc_generator.cpp: libferrissane_moc_impl.cpp
# 	touch sane_moc_generator.cpp


# plugin_LTLIBRARIES += libferrissane.la libferrissane_factory.la

# libferrissane_factory_la_SOURCES = libferrissane_factory.cpp
# libferrissane_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
# libferrissane_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

# libferrissane_la_SOURCES = sane_moc_generator.cpp libferrissane.cpp 
# libferrissane_la_LDFLAGS = @KSANE_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
# libferrissane_la_CXXFLAGS = @KSANE_CFLAGS@ @CFLAGS@


##############################
##############################
##############################

MOC_DEP_SRCS+=libferriskio_shared_moc.cpp


KIOCXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @KDE_EAGENERATOR_CFLAGS@ @CFLAGS@

if FERRIS_BUILD_KDE_EAGENERATOR
plugin_LTLIBRARIES += libferriskio.la libferriskio_factory.la
noinst_LTLIBRARIES += libferriskio_shared.la
endif

libferriskio_factory_la_SOURCES = libferriskio_factory.cpp
libferriskio_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriskio_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferriskio_la_SOURCES = libferriskio.cpp libferriskio_shared.hh
libferriskio_la_LDFLAGS = @CTXKIO_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @KDE_EAGENERATOR_LIBS@
libferriskio_la_CXXFLAGS = $(KIOCXXFLAGS)

nodist_libferriskio_shared_la_SOURCES = libferriskio_shared_moc.cpp
libferriskio_shared_la_SOURCES = libferriskio_shared.cpp libferriskio_shared.hh
libferriskio_shared_la_CXXFLAGS = $(KIOCXXFLAGS)


##############################
##############################
##############################

# MOC_DEP_SRCS+=libferrisidentica_shared_moc.cpp

# IDENTICACXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ \
# 	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@ @CFLAGS@

# plugin_LTLIBRARIES += libferrisidentica.la libferrisidentica_factory.la

# libferrisidentica_factory_la_SOURCES = libferrisidentica_factory.cpp
# libferrisidentica_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
# libferrisidentica_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

# libferrisidentica_la_SOURCES = libferrisidentica.cpp libferrisidentica_shared.hh
# libferrisidentica_la_LDFLAGS = @CTXIDENTICA_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@
# libferrisidentica_la_CXXFLAGS = $(IDENTICACXXFLAGS)


# noinst_LTLIBRARIES += libferrisidentica_shared.la
# nodist_libferrisidentica_shared_la_SOURCES = libferrisidentica_shared_moc.cpp
# libferrisidentica_shared_la_SOURCES = \
# 	libferrisidentica_shared.cpp libferrisidentica_shared.hh
# libferrisidentica_shared_la_CXXFLAGS = $(IDENTICACXXFLAGS)


# noinst_PROGRAMS += identicatest
# identicatest_SOURCES = identicatest.cpp libferrisidentica_shared.hh
# identicatest_LDADD = -lpopt @LIBFERRIS_LA@ 
# identicatest_LDFLAGS = @QT_LIBS@ @QTNETWORK_LIBS@ @CTXIDENTICA_LA@



##############################
##############################
##############################


MOC_DEP_SRCS+=libferriszoneminder_shared_moc.cpp


ZMCXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ @QJSON_CFLAGS@ @QOAUTH_CFLAGS@ @CFLAGS@

plugin_LTLIBRARIES += libferriszoneminder.la libferriszoneminder_factory.la

libferriszoneminder_factory_la_SOURCES = libferriszoneminder_factory.cpp
libferriszoneminder_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferriszoneminder_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferriszoneminder_la_SOURCES = libferriszoneminder.cpp libferriszoneminder_shared.hh
libferriszoneminder_la_CXXFLAGS = $(ZMCXXFLAGS)
libferriszoneminder_la_LDFLAGS = @CTXZONEMINDER_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QOAUTH_LIBS@ @QJSON_LIBS@


noinst_LTLIBRARIES += libferriszoneminder_shared.la
nodist_libferriszoneminder_shared_la_SOURCES = libferriszoneminder_shared_moc.cpp
libferriszoneminder_shared_la_SOURCES = libferriszoneminder_shared.cpp libferriszoneminder_shared.hh
libferriszoneminder_shared_la_CXXFLAGS = $(ZMCXXFLAGS)


##############################
##############################
##############################

MOC_DEP_SRCS+=libferrisferrisrest_shared_moc.cpp

RESTCXXFLAGS = @QT_CFLAGS@ @QTNETWORK_CFLAGS@ @QTDBUS_CFLAGS@ @QJSON_CFLAGS@ @CFLAGS@

plugin_LTLIBRARIES += libferrisferrisrest.la libferrisferrisrest_factory.la
noinst_LTLIBRARIES += libferrisferrisrest_shared.la

libferrisferrisrest_factory_la_SOURCES = libferrisferrisrest_factory.cpp
libferrisferrisrest_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisferrisrest_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrisferrisrest_la_SOURCES = libferrisferrisrest.cpp libferrisferrisrest_shared.hh
libferrisferrisrest_la_LDFLAGS = @CTXFERRISREST_LA@ @QT_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @QJSON_LIBS@
libferrisferrisrest_la_CXXFLAGS = $(RESTCXXFLAGS)

nodist_libferrisferrisrest_shared_la_SOURCES = libferrisferrisrest_shared_moc.cpp
libferrisferrisrest_shared_la_SOURCES = libferrisferrisrest_shared.cpp libferrisferrisrest_shared.hh
libferrisferrisrest_shared_la_CXXFLAGS = $(RESTCXXFLAGS)


##############################
##############################
##############################

# MOC_DEP_SRCS+=libferrisplasma_moc_impl.cpp

# libferrisplasma_moc_generator.cpp: libferrisplasma_moc_impl.cpp
# 	touch libferrisplasma_moc_generator.cpp

# PLAMSACXXFLAGS = @PLASMA_CFLAGS@ @CFLAGS@

# if FERRIS_HAVE_PLASMA
# plugin_LTLIBRARIES += libferrisplasma.la libferrisplasma_factory.la
# endif

# libferrisplasma_factory_la_SOURCES = libferrisplasma_factory.cpp
# libferrisplasma_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
# libferrisplasma_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

# libferrisplasma_la_SOURCES = libferrisplasma.cpp  libferrisplasma_moc_generator.cpp
# libferrisplasma_la_LDFLAGS = @PLASMA_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
# libferrisplasma_la_CXXFLAGS = $(PLAMSACXXFLAGS)



##############################
##############################
##############################

if HAVE_PULSEAUDIO
plugin_LTLIBRARIES += libferrispulseaudio.la libferrispulseaudio_factory.la
endif

libferrispulseaudio_factory_la_SOURCES = libferrispulseaudio_factory.cpp
libferrispulseaudio_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrispulseaudio_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@

libferrispulseaudio_la_SOURCES = libferrispulseaudio.cpp 
libferrispulseaudio_la_LDFLAGS = @PULSEAUDIO_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libferrispulseaudio_la_CXXFLAGS = @PULSEAUDIO_CFLAGS@ @CFLAGS@


##############################
##############################
##############################

# if HAVE_QTSQL
# plugin_LTLIBRARIES += libferrisqtsql.la libferrisqtsql_factory.la
# noinst_LTLIBRARIES += libferrisqtsqlshared.la
# endif

# libferrisqtsql_factory_la_SOURCES = libferrisqtsql_factory.cpp
# libferrisqtsql_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
# libferrisqtsql_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
# libferrisqtsql_la_SOURCES   = libferrisqtsql.cpp
# libferrisqtsql_la_CXXFLAGS  = @QTSQL_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@
# libferrisqtsql_la_LDFLAGS = \
# 	\$(top_builddir)/plugins/context/qtsql/libferrisqtsqlshared.la            \
# 	@QTSQL_LIBS@ \
# 	@LIBFERRIS_PLUGIN_LT_LINE@

# libferrisqtsqlshared_la_SOURCES = libferrisqtsqlshared.cpp libferrisqtsqlshared.hh
# libferrisqtsqlshared_la_CXXFLAGS  = @QTSQL_CFLAGS@ -I@COMMONSQLDBDIR@ @CFLAGS@




##############################
##############################
##############################

if HAVE_GPHOTO2
plugin_LTLIBRARIES += libferrisgphoto2.la libferrisgphoto2_factory.la
endif


libferrisgphoto2_factory_la_SOURCES = libferrisgphoto2_factory.cpp
libferrisgphoto2_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgphoto2_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgphoto2_la_SOURCES         = libferrisgphoto2.cpp
libferrisgphoto2_la_CXXFLAGS = -I@COMMONDBDIR@ @GPHOTO2_CFLAGS@ @GLIB_CFLAGS@ @CFLAGS@
libferrisgphoto2_la_LDFLAGS = @GLIB_LIBS@ @GPHOTO2_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@ @COMMONDB_LA@



##############################
##############################
##############################

GSTREAMERCXXFLAGS = @GSTREAMER_CFLAGS@ @CFLAGS@

if HAVE_GSTREAMER
plugin_LTLIBRARIES += libferrisgstreamer.la libferrisgstreamer_factory.la
noinst_LTLIBRARIES += libferrisgstreamer_shared.la
noinst_PROGRAMS += gstreamertest
endif

libferrisgstreamer_factory_la_SOURCES = libferrisgstreamer_factory.cpp
libferrisgstreamer_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisgstreamer_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@


libferrisgstreamer_la_SOURCES = libferrisgstreamer.cpp libferrisgstreamer_shared.hh
libferrisgstreamer_la_LDFLAGS = @CTXGSTREAMER_LA@ @GSTREAMER_LIBS@ @LIBFERRIS_PLUGIN_LT_LINE@
libferrisgstreamer_la_CXXFLAGS = $(GSTREAMERCXXFLAGS)
libferrisgstreamer_shared_la_SOURCES = \
	libferrisgstreamer_shared.cpp libferrisgstreamer_shared.hh
libferrisgstreamer_shared_la_CXXFLAGS = $(GSTREAMERCXXFLAGS)

gstreamertest_SOURCES = gstreamertest.cpp libferrisgstreamer_shared.hh
gstreamertest_LDADD = -lpopt @LIBFERRIS_LA@ 
gstreamertest_LDFLAGS = @GSTREAMER_LIBS@ @CTXGSTREAMER_LA@
gstreamertest_CXXFLAGS = $(GSTREAMERCXXFLAGS)




##############################
##############################
##############################


if BUILD_DBUS_MOUNTING
plugin_LTLIBRARIES += libferrisdbus.la libferrisdbus_factory.la
endif

DBUSCXXFLAGS = @QTDBUS_CFLAGS@ @GLIB_CFLAGS@ @CFLAGS@

libferrisdbus_factory_la_SOURCES = libferrisdbus_factory.cpp
libferrisdbus_factory_la_LIBADD  = @LIBFERRIS_PLUGIN_FACTORY_LIBADD@
libferrisdbus_factory_la_LDFLAGS = @LIBFERRIS_PLUGIN_LT_LINE@
libferrisdbus_la_SOURCES  = libferrisdbus.cpp 
libferrisdbus_la_CXXFLAGS = $(DBUSCXXFLAGS)
libferrisdbus_la_LDFLAGS =                            \
	@GLIB_LIBS@              	              \
	@QTDBUS_LIBS@                                 \
	@LIBFERRIS_PLUGIN_LT_LINE@                        




##############################
##############################
##############################




##############################
##############################
##############################


# next:
#       
#             



if HAVE_LIBMPEG3
MPEG3DIR = libmpeg3
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


if HAVE_GSTREAMER
GSTREAMERDIR = gstreamer
endif

if HAVE_LIBPLATINUM
UPNPDIR = upnp
endif

if FERRIS_HAVE_PLASMA
PLASMADIR = plasma
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
	$(MPEG3DIR)             \
	nativembox $(SOCKETS) external       \
	$(SQLPLUSDIR) $(DVDREADDIR)                 \
	$(IPCDIR)  $(LDAPDIR) $(EETDIR)    \
	$(TDBDIR) $(DBXMLDIR) $(SOPRANO) \
	$(DTL) bibtex \
	obby xmms amarok \
	$(HALDIR)   \
	$(UPNPDIR)  \
	$(QPRINTERDIR)   \
	firefox


BUILT_SOURCES += $(MOC_DEP_SRCS)
CLEANFILES += $(MOC_DEP_SRCS)
