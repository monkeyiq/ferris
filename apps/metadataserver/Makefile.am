if HAVE_QTDBUS

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST =

MOC_DEP_SRCS=ferris-internal-metadata-broker.moc  ferris-internal-metadata-worker.moc ferris-internal-metadata-broker-direct-client.moc metadata-clients-common.moc

CLEANFILES = $(MOC_DEP_SRCS)
mocimpls.cpp: $(MOC_DEP_SRCS)
	touch mocimpls.cpp
ferris-internal-metadata-broker.moc: ferris-internal-metadata-broker.cpp
	$(MOC) $(srcdir)/ferris-internal-metadata-broker.cpp -o ferris-internal-metadata-broker.moc

ferris-internal-metadata-worker.moc: ferris-internal-metadata-worker.cpp
	$(MOC) $(srcdir)/ferris-internal-metadata-worker.cpp -o ferris-internal-metadata-worker.moc

ferris-internal-metadata-broker-direct-client.moc: ferris-internal-metadata-broker-direct-client.cpp
	$(MOC) $(srcdir)/ferris-internal-metadata-broker-direct-client.cpp -o ferris-internal-metadata-broker-direct-client.moc

metadata-clients-common.moc: metadata-clients-common.cpp
	$(MOC) $(srcdir)/metadata-clients-common.hh -o metadata-clients-common.moc

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/eagenerators/           \
	-I$(top_srcdir)/DBusGlue/ 	                \
	-I$(top_builddir)/DBusGlue/ 	                \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@QT_CFLAGS@ @QTNETWORK_CFLAGS@                  \
	@QTDBUS_CFLAGS@                                 \
	@CFLAGS@


METADATA_COMMANDS = \
	ferris-internal-metadata-broker ferris-internal-metadata-worker \
	ferris-internal-metadata-broker-direct-client ferris-internal-metadata-worker-direct-client

bin_PROGRAMS = $(METADATA_COMMANDS) 

ferris_internal_metadata_broker_SOURCES = mocimpls.cpp ferris-internal-metadata-broker.cpp metadata-clients-common.cpp metadata-clients-common.hh
ferris_internal_metadata_broker_LDADD = -lm -lpopt
ferris_internal_metadata_broker_LDFLAGS = @GLIB_LIBS@ @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_metadata_worker_SOURCES = mocimpls.cpp ferris-internal-metadata-worker.cpp metadata-clients-common.cpp metadata-clients-common.hh
ferris_internal_metadata_worker_LDADD = -lm -lpopt
ferris_internal_metadata_worker_LDFLAGS = @GLIB_LIBS@ @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_metadata_broker_direct_client_SOURCES = mocimpls.cpp ferris-internal-metadata-broker-direct-client.cpp
ferris_internal_metadata_broker_direct_client_LDADD = -lm -lpopt
ferris_internal_metadata_broker_direct_client_LDFLAGS = @GLIB_LIBS@ @QTDBUS_LIBS@ @LIBFERRIS_LA@ 

ferris_internal_metadata_worker_direct_client_SOURCES = mocimpls.cpp ferris-internal-metadata-worker-direct-client.cpp
ferris_internal_metadata_worker_direct_client_LDADD = -lm -lpopt
ferris_internal_metadata_worker_direct_client_LDFLAGS = @GLIB_LIBS@ @QTDBUS_LIBS@ @LIBFERRIS_LA@ 


endif
