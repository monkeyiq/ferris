
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

SERVICE_FILES = $(top_builddir)/DBusGlue/*.service
servicedir    = $(datarootdir)/dbus-1/services
service_DATA  = $(SERVICE_FILES)

XML_FILES  = $(top_srcdir)/DBusGlue/*.xml
EXTRA_DIST = $(XML_FILES) $(SERVICE_FILES)

BUILT_SOURCES = broker_adaptor.h broker_adaptor.cpp broker_interface.h broker_interface.cpp \
	worker_adaptor.h worker_adaptor.cpp worker_interface.h worker_interface.cpp \
	org_freedesktop_DBus.h                 org_freedesktop_DBus.cpp \
	org_freedesktop_DBus_Introspectable.h  org_freedesktop_DBus_Introspectable.cpp \
	org_freedesktop_Hal_Device.h           org_freedesktop_Hal_Device.cpp \
	org_freedesktop_Hal_Device_Volume.h    org_freedesktop_Hal_Device_Volume.cpp \
	org_freedesktop_Hal_Manager.h          org_freedesktop_Hal_Manager.cpp \
	com_libferris_Volume_Manager.h         com_libferris_Volume_Manager.cpp  \
	com_libferris_Volume_Manager_adaptor.h com_libferris_Volume_Manager_adaptor.cpp


broker_adaptor.h broker_adaptor.cpp broker_interface.h broker_interface.cpp: ferris_internal_metadata_broker_introspect.xml
	qdbusxml2cpp -N -m -c broker -p broker_interface $(srcdir)/ferris_internal_metadata_broker_introspect.xml 
	$(MOC) broker_interface.h -o broker_interface.moc
	qdbusxml2cpp -N -m -c broker -a broker_adaptor $(srcdir)/ferris_internal_metadata_broker_introspect.xml 
	$(MOC) broker_adaptor.h -o broker_adaptor.moc

worker_adaptor.h worker_adaptor.cpp worker_interface.h worker_interface.cpp: ferris_internal_metadata_worker_introspect.xml
	qdbusxml2cpp -N -m -c worker -p worker_interface $(srcdir)/ferris_internal_metadata_worker_introspect.xml 
	$(MOC) worker_interface.h -o worker_interface.moc
	qdbusxml2cpp -N -m -c worker -a worker_adaptor $(srcdir)/ferris_internal_metadata_worker_introspect.xml 
	$(MOC) worker_adaptor.h -o worker_adaptor.moc


org_freedesktop_DBus.h org_freedesktop_DBus.cpp: org.freedesktop.DBus.xml
	qdbusxml2cpp -N -m -c fdoDBus -p org_freedesktop_DBus $(srcdir)/org.freedesktop.DBus.xml
	$(MOC) org_freedesktop_DBus.h -o org_freedesktop_DBus.moc

org_freedesktop_DBus_Introspectable.h org_freedesktop_DBus_Introspectable.cpp: org.freedesktop.DBus.Introspectable.xml
	qdbusxml2cpp -N -m -c fdoDBusIntrospectable -p org_freedesktop_DBus_Introspectable $(srcdir)/org.freedesktop.DBus.Introspectable.xml
	$(MOC) org_freedesktop_DBus_Introspectable.h -o org_freedesktop_DBus_Introspectable.moc

org_freedesktop_Hal_Device.h org_freedesktop_Hal_Device.cpp: org.freedesktop.Hal.Device.xml
	qdbusxml2cpp -N -m -c fdoHalDevice -p org_freedesktop_Hal_Device $(srcdir)/org.freedesktop.Hal.Device.xml
	$(MOC) org_freedesktop_Hal_Device.h -o org_freedesktop_Hal_Device.moc

org_freedesktop_Hal_Device_Volume.h org_freedesktop_Hal_Device_Volume.cpp: org.freedesktop.Hal.Device.Volume.xml
	qdbusxml2cpp -N -m -c fdoHalDeviceVolume -p org_freedesktop_Hal_Device_Volume $(srcdir)/org.freedesktop.Hal.Device.Volume.xml
	$(MOC) org_freedesktop_Hal_Device_Volume.h -o org_freedesktop_Hal_Device_Volume.moc

org_freedesktop_Hal_Manager.h org_freedesktop_Hal_Manager.cpp: org.freedesktop.Hal.Manager.xml
	qdbusxml2cpp -N -m -c fdoHalManager -p org_freedesktop_Hal_Manager $(srcdir)/org.freedesktop.Hal.Manager.xml
	$(MOC) org_freedesktop_Hal_Manager.h -o org_freedesktop_Hal_Manager.moc


com_libferris_Volume_Manager.h com_libferris_Volume_Manager.cpp com_libferris_Volume_Manager_adaptor.h com_libferris_Volume_Manager_adaptor.cpp: com.libferris.Volume.Manager.xml
	qdbusxml2cpp -N -m -c fVolumeManager -p com_libferris_Volume_Manager $(srcdir)/com.libferris.Volume.Manager.xml 
	$(MOC) com_libferris_Volume_Manager.h -o com_libferris_Volume_Manager.moc
	qdbusxml2cpp -N -m -c fVolumeManager -a com_libferris_Volume_Manager_adaptor $(srcdir)/com.libferris.Volume.Manager.xml 
	$(MOC) com_libferris_Volume_Manager_adaptor.h -o com_libferris_Volume_Manager_adaptor.moc



METADATA_COMMANDS = force-header-generation
noinst_PROGRAMS = $(METADATA_COMMANDS) 

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/eagenerators/           \
	-I/usr/local/include                            \
	@QTDBUS_CFLAGS@                                \
	@CFLAGS@

force_header_generation_LDADD = -lm -lpopt
force_header_generation_LDFLAGS = @QTDBUS_LIBS@ #@LIBFERRIS_LA@ 
force_header_generation_SOURCES = \
	broker_adaptor.h broker_interface.h \
	worker_adaptor.h worker_interface.h \
	org_freedesktop_DBus.h \
	org_freedesktop_DBus_Introspectable.h \
	org_freedesktop_Hal_Device.h \
	org_freedesktop_Hal_Device_Volume.h \
	org_freedesktop_Hal_Manager.h \
	force-header-generation.cpp

