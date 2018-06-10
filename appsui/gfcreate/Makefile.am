MAINTAINERCLEANFILES = Makefile.in


if HAVE_XALAN_GLADE
UITARGETS = gfcreate
endif

bin_PROGRAMS = $(UITARGETS)

INCLUDES =                                              \
	@GLIB_CFLAGS@                                   \
	@XML4C_CFLAGS@ @XALAN_CFLAGS@                   \
	@GTK_CFLAGS@  @GLADE_CFLAGS@ -DHAVE_GTK2=1      \
	@CFLAGS@ -I$(top_srcdir)/apps/fcreate

gfcreate_SOURCES = \
	gfcreate.cpp gfcreate.hh \
	$(top_srcdir)/apps/fcreate/ferriscreatecommon.cpp \
        $(top_srcdir)../../apps/fcreate/ferriscreatecommon.hh
gfcreate_LDADD   = -lpopt 
gfcreate_CFLAGS  = $(INCLUDES) @GTK_CFLAGS@
gfcreate_LDFLAGS = @GTK_LIBS@ @XML4C_LIBS@ @XALAN_LIBS@ @GTK_LIBS@ @GLADE_LIBS@ \
	@LIBFERRIS_LA@ @LIBFERRISUI_LA@


