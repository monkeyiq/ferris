
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_srcdir)/src                             \
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@GTK_CFLAGS@                                    \
	-I/usr/local/witme-file-3.35/include            \
	@FAMPP2_CFLAGS@                                 \
	-DBUILDING_LIBFERRIS                            \
	@CFLAGS@

lib_LTLIBRARIES      = libferrisui.la

###################################################################
###################################################################
###################################################################

ferrisuiincludedir=$(includedir)/FerrisUI
ferrisuiinclude_HEADERS =	        \
	All.hh                          \
	GtkFerris.hh                    \
	gtkferriscellrenderertext.hh    \
	gtkferristreestore.hh           \
	TreeStoreDriver.hh              \
	FerrisCopyUI.hh                 \
	FerrisRemoveUI.hh               \
	FerrisOptionMenu.hh             \
	ContextPropertiesEditor.hh      \
	UAsyncIO.hh                     \
	EditStringList.hh               \
	Menus.hh                        \
	MedallionEditor.hh              \
	EditSQLColumns.hh		\
	SubprocessButtonAndLabel.hh

libferrisui_la_SOURCES =                \
	GtkFerris.cpp                   \
	gtkferriscellrenderertext.cpp   \
	gtkferristreestore.cpp          \
	TreeStoreDriver.cpp             \
	FerrisCopyUI.cpp                \
	FerrisRemoveUI.cpp		\
	GtkFerris_private.hh            \
	FerrisOptionMenu.cpp            \
	ContextPropertiesEditor.cpp     \
	UAsyncIO.cpp                    \
	EditStringList.cpp              \
	Menus.cpp                       \
	MedallionEditor.cpp             \
	gtkcellrendererprogress.c gtkcellrendererprogress.h \
	EditSQLColumns.cpp 		\
	SubprocessButtonAndLabel.cpp

#libferrisui_la_LIBADD  = #$(top_builddir)/src/libferris.la
libferrisui_la_LDFLAGS = @GTK_LIBS@                

###################################################################
###################################################################
###################################################################

