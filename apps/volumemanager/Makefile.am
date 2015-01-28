# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in


bin_PROGRAMS = ferris-volume-manager \
	ferris-internal-hal-mount-volume \
	ferris-internal-hal-els \
	ferris-internal-hal-copyto


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



ferris_volume_manager_SOURCES = ferris-volume-manager.cpp 
ferris_volume_manager_LDADD = -lm -lpopt
ferris_volume_manager_LDFLAGS = @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_hal_mount_volume_SOURCES = ferris-internal-hal-mount-volume.cpp
ferris_internal_hal_mount_volume_LDADD = -lm -lpopt
ferris_internal_hal_mount_volume_LDFLAGS = @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_hal_els_SOURCES = ferris-internal-hal-els.cpp
ferris_internal_hal_els_LDADD = -lm -lpopt
ferris_internal_hal_els_LDFLAGS = @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_hal_copyto_SOURCES = ferris-internal-hal-copyto.cpp
ferris_internal_hal_copyto_LDADD = -lm -lpopt
ferris_internal_hal_copyto_LDFLAGS = @QTDBUS_LIBS@ @LIBFERRIS_LA@ 




