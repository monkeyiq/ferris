
MAINTAINERCLEANFILES = Makefile.in

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_srcdir)/src                             \
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@QTGUI_CFLAGS@                                  \
	-DBUILDING_LIBFERRIS                            \
	@CFLAGS@

lib_LTLIBRARIES      = libferrisqtgui.la

EXTRA_DIST = $(srcdir)/*.qml

###################################################################
###################################################################
###################################################################

%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

%_moc_impl.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

mocimpls.cpp: FerrisFileSystemModel_moc.cpp FerrisSortFilterProxyModel_moc.cpp modeltest_moc_impl.cpp
	touch mocimpls.cpp


ferrisqtguiincludedir=$(includedir)/Ferrisqtgui
libferrisqtgui_la_LDFLAGS = @QTGUI_LIBS@                

ferrisqtguiinclude_HEADERS =	        \
	All.hh                          \
	FerrisFileSystemModel.hh        \
	FerrisSortFilterProxyModel.hh   \
	FerrisQtGui.hh 

libferrisqtgui_la_SOURCES =             \
	mocimpls.cpp                    \
	FerrisFileSystemModel.cpp       \
	FerrisSortFilterProxyModel.cpp  \
	FerrisQtGui.cpp


###################################################################
###################################################################
###################################################################

noinst_PROGRAMS = modeltest # modeltestqml

modeltest_SOURCES = modeltest.cpp FerrisFileSystemModel.hh
modeltest_LDADD   = -lpopt @LIBFERRIS_LA@ @LIBFERRISQTGUI_LA@ 
modeltest_LDFLAGS = @QT_LIBS@  @QTGUI_LIBS@

modeltestqml_SOURCES = modeltestqml.cpp FerrisFileSystemModel.hh
modeltestqml_LDADD   = -lpopt @LIBFERRIS_LA@ @LIBFERRISQTGUI_LA@ 
modeltestqml_LDFLAGS = @QT_LIBS@  @QTGUI_LIBS@


