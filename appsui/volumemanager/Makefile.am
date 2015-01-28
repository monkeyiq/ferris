# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in


bin_PROGRAMS = ferris-volume-manager-setup-volume-wizard


INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@QTDBUS_CFLAGS@                                \
	@GTK_CFLAGS@ \
	@CFLAGS@


ferris_volume_manager_setup_volume_wizard_SOURCES = ferris-volume-manager-setup-volume-wizard.cpp
ferris_volume_manager_setup_volume_wizard_LDADD = -lm -lpopt
ferris_volume_manager_setup_volume_wizard_LDFLAGS = @QTDBUS_LIBS@ @LIBFERRIS_LA@ @LIBFERRISUI_LA@ @GTK_LIBS@



