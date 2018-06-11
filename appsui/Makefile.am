


AM_CPPFLAGS = @GTK_CFLAGS@ @CFLAGS@
AM_LDFLAGS = @LIBFERRIS_LA@ @LIBFERRISUI_LA@ -lpopt @GTK_LIBS@

if HAVE_XALAN_GLADE
UITARGETS = gfcreate
endif


bin_PROGRAMS = gfcp gfrm gfmv gfproperties gfdl gfindexadd \
	ferris-volume-manager-setup-volume-wizard $(UITARGETS)

gfcp_SOURCES = gfcp.cpp
gfrm_SOURCES = gfrm.cpp
gfmv_SOURCES = gfmv.cpp
gfproperties_SOURCES = gfproperties.cpp
gfdl_SOURCES = gfdl.cpp
gfindexadd_SOURCES = gfindexadd.cpp
ferris_volume_manager_setup_volume_wizard_SOURCES = ferris-volume-manager-setup-volume-wizard.cpp

gfcreate_CXXFLAGS = $(AM_CPPFLAGS) @GLADE_CFLAGS@ @XML4C_CFLAGS@ @XALAN_CFLAGS@ -I$(top_srcdir)/apps/fcreate
gfcreate_SOURCES = gfcreate.cpp gfcreate.hh 
gfcreate_LDFLAGS = $(AM_LDFLAGS) @XML4C_LIBS@ @XALAN_LIBS@ @GTK_LIBS@ @GLADE_LIBS@ @FCREATESHAREDGTK_LA@
