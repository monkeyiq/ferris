%_moc.cpp: %.hh
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

%_moc_impl.cpp: %.cpp
	$(MOC) $(DEFINES) $(INCPATH) $< -o $@

# have to make these!
# FerrisWebServices_private_moc.cpp FerrisQt_moc_impl.cpp
mocimpls.cpp: FerrisWebServices_private_moc.cpp FerrisQt_moc_impl.cpp  MetadataServer_moc_impl.cpp
	touch mocimpls.cpp


# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@LOKI_CFLAGS@                                   \
	@FSTREAM_CFLAGS@                                \
	-I/usr/local/witme-file-3.35/include            \
	@FAMPP2_CFLAGS@                                 \
	@XML4C_CFLAGS@                                  \
	@XALAN_CFLAGS@                                  \
	@DB4_CFLAGS@                                    \
	@GNOMEVFS_CFLAGS@                               \
	@LIBFILE_CFLAGS@                                \
	@EFSD_CFLAGS@                                   \
	@KDE_CFLAGS@                                    \
	@LIBMAGIC_CFLAGS@                               \
	@OPENSSL_CFLAGS@                                \
	@LIBZ_CFLAGS@                                   \
	@BZIP2_CFLAGS@                                  \
	@STLDB4_CFLAGS@                                 \
	@LIBRPM_CFLAGS@                                 \
	@XQILLA_CFLAGS@                                 \
	@SOPRANO_CFLAGS@                                \
	@NEPOMUK_CFLAGS@                                \
	@LIBPQXX_CFLAGS@                                \
	@LIBTEXTCAT_CFLAGS@                             \
	@GPGME_CFLAGS@                                  \
	@BOOST_CFLAGS@                                  \
	-DBUILDING_LIBFERRIS                            \
	@DBUS_CFLAGS@                                   \
	@IMLIB_CFLAGS@		                        \
	@QTDBUS_CFLAGS@                                 \
	@LIBSELINUX_CFLAGS@                             \
	@CURL_CFLAGS@                                   \
	@XERCES_INTERNAL_HEADERS_CFLAGS@                \
	@QTNETWORK_CFLAGS@                              \
	@QJSON_CFLAGS@                                  \
	@CFLAGS@                                        


if HAVE_XML4C
XMLHH = FerrisDOM.hh
XMLCPP = FerrisDOM.cpp FerrisDOM_private.hh libferrisxmlshared.cpp libferrisxmlshared.hh
endif

if HAVE_XALAN
XALANCPP = FerrisXalan_private.hh FerrisXalan.cpp
endif

if FERRIS_HAVE_LIBRPM
LIBRPM_FILES=RPMContext.cpp RPMContext_private.hh
endif


if HAVE_CURL
CURL_FILES=FerrisCurl.cpp FerrisCurl_private.hh
endif

if HAVE_XQILLA
XQILLA_DEP_SRCS=XQillaContext.cpp XQillaContext_private.hh XQilla_private.hh XQilla.cpp
endif

if HAVE_SVMLIGHT
SVMLIGHT_DEP_SRCS=AgentSVMLight.cpp
endif


if FERRIS_HAVE_GPGME
GPG_DEP_SRCS=FerrisGPGContext.cpp FerrisGPGContext_private.hh
endif


if HAVE_XMMS
XMMS_DEP_SOURCES = WrapXMMS.cpp
endif

if HAVE_LIBPQXX
FCA_PQXX_DEP_SOURCES = FCA.cpp
endif



#COMMON_GCJFLAGS = --encoding=ISO-8859-1 -Wno-deprecated
#BUILT_SOURCES = MyAnalyzer.h
#%.h: %.java
#	$(GCJ) -C $(COMMON_GCJFLAGS) $(AM_GCJFLAGS) $(GCJFLAGS) $<
#	gcjh  --classpath `pwd`  `echo $< | sed "s/.java//g"`;

##############################
##############################
##############################

if HAVE_SWIG_PERL
SWIG_PERL_LTLIBNAMES = libferrisperl.la
libferrisperl_wrap.cpp libferrisperl.pm: libferrisperl.i
	swig -c++ -DLIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER \
		-I/usr/local/include -I/usr/include `pkg-config gtk+-2.0 ferrisstreams --cflags | sed s/-pthread//g ` \
		-perl5 -module libferrisperl  -o $(srcdir)/libferrisperl_wrap.cpp $(srcdir)/libferrisperl.i
	sed -i -e 's/#define SWIGPERL/#include <config.h>   #define SWIGPERL /g' $(srcdir)/libferrisperl_wrap.cpp

#-proxy -shadow

libferrisperl_la_SOURCES = libferrisperl_wrap.cpp
libferrisperl_la_LIBADD  = libferris.la 
libferrisperl_la_CXXFLAGS =   \
	$(INCLUDES)           \
	`perl -MExtUtils::Embed -e ccopts -e ldopts` \
	@SWIG_PERL_CFLAGS@  

libferrisperl_la_LDFLAGS = \
	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@ \
	`perl -MExtUtils::Embed -e ccopts -e ldopts` 

perlmoduledir = @SWIG_PERL_PMDIR@
perlmodule_DATA = libferrisperl.pm

endif



##############################
##############################
##############################

if HAVE_SWIG_PYTHON
SWIG_PYTHON_LTLIBNAMES = libferrispython.la
libferrispython_wrap.cpp: libferrispython.i
	swig -c++ -I/usr/local/include -I/usr/include `pkg-config gtk+-2.0 ferrisstreams --cflags` \
		-python -module libferrispython  -o libferrispython_wrap.cpp libferrispython.i
libferrispython.py: libferrispython_wrap.cpp

libferrispython_la_SOURCES = libferrispython_wrap.cpp
libferrispython_la_LIBADD  = libferris.la
libferrispython_la_CXXFLAGS = \
	$(INCLUDES)         \
	@SWIG_PYTHON_CFLAGS@

install-exec-local:
	cd @SWIG_PYTHON_LIBDIR@;                             \
	cp $(LIBDIR)/libferrispython.so _libferrispython.so; \
	mv $(LIBDIR)/libferrispython.so* /tmp;               \
	mv $(LIBDIR)/libferrispython.la /tmp;

libferrispython_la_LDFLAGS = \
	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@

pythonmoduledir = @SWIG_PYTHON_MODULEDIR@
pythonmodule_DATA = libferrispython.py

endif

##############################
##############################
##############################

if HAVE_SWIG_OCAML

libferrisocaml_wrap.c: libferrisocaml.i libferris.i
	swig -c++ -DLIBFERRIS_INTERNAL_COMPILING_SWIG_WRAPPER -I/usr/local/include -I/usr/include `pkg-config gtk+-2.0 ferrisstreams --cflags` \
		-ocaml -module libferrisocaml  -o libferrisocaml_wrap.cpp libferrisocaml.i
	echo "#include <assert.h>"  >|libferrisocaml_wrap.c
	cat libferrisocaml_wrap.cpp >>libferrisocaml_wrap.c

libferrisocaml.so: libferrisocaml_wrap.c
	ocamlc -cc 'g++' -g -c  -ccopt "-I." -ccopt "$(SIGC_CFLAGS)" -ccopt "$(LOKI_CFLAGS)" -ccopt "$(GLIB_CFLAGS)" -ccopt "$(STLDB4_CFLAGS)" -ccopt "$(OPENSSL_CFLAGS)" -ccopt "$(SWIG_OCAML_CFLAGS)" -ccopt -g -ccopt "-xc++ " libferrisocaml_wrap.c
	g++ -shared -o libferrisocaml.so libferrisocaml_wrap.o \
$(SIGC_LIBS) $(LOKI_LIBS) $(GLIB_LIBS) $(STLDB4_LIBS) $(OPENSSL_LIBS)
	OCAMLRUNPARAM="l=32000000" ocamldlgen \
			libferrisocaml.ml libferrisocaml.so > libferrisocaml_dynamic.ml
	mv -f           libferrisocaml_dynamic.ml libferrisocaml.ml
	OCAMLRUNPARAM="l=32000000" ocamlfind \
			ocamlc -g -c -package dl libferrisocaml.ml

libferrisocaml.cmi: libferrisocaml.so
libferrisocaml.cmo: libferrisocaml.so

ocamlsharedlibdir   = @LIBDIR@
ocamlsharedlib_DATA = libferrisocaml.so

SWIG_OCAML_LTLIBNAMES =
#SWIG_OCAML_LTLIBNAMES      = libferrisocaml.la
#libferrisocaml_la_SOURCES  = libferrisocaml_wrap.o
#libferrisocaml_la_LIBADD   = libferris.la
#libferrisocaml_la_CXXFLAGS =   \
#	$(INCLUDES)         \
#	@SWIG_OCAML_CFLAGS@

#libferrisocaml_la_LDFLAGS = \
#	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@


ocamlmoduledir = @SWIG_OCAML_HEADERDIR@
ocamlmodule_DATA = libferrisocaml.cmi libferrisocaml.cmo

endif



###################################################################
###################################################################
###################################################################

if HAVE_DBUS
DBUS_SOURCES = 	DBus.cpp DBus_private.hh
endif

lib_LTLIBRARIES      = libferris.la \
	$(SWIG_PERL_LTLIBNAMES) $(SWIG_PYTHON_LTLIBNAMES)  \
	$(SWIG_OCAML_LTLIBNAMES)
EXTRA_DIST= libferris.i     \
	libferrisperl.i     \
	perl_std_list.i     \
	libferrispython.i   \
	libferrisocaml.i    \
	libferrisocaml_wrap.c

BUILT_SOURCES = libferrisperl_wrap.cpp libferrispython_wrap.cpp libferrisocaml_wrap.c 

#	libferrispython.py  libferrisperl.pm libferrisocaml.cmi libferrisocaml.cmo


ferrisincludedir=$(includedir)/Ferris
ferrisinclude_HEADERS =          \
	Ferris.hh                \
	Versioned.hh             \
	Native.hh                \
	ContextContext.hh        \
	FerrisException.hh       \
	SignalStreams.hh         \
	FerrisHandle.hh          \
	FerrisEvent.hh           \
	timber.hh                \
	Enamel.hh                \
	SM.hh                    \
	EAGenerators.hh          \
	MatchedEAGenerators.hh   \
	General.hh		 \
	FilteredContext.hh       \
	Regex.hh                 \
	Trimming.hh              \
	TypeDecl.hh              \
	Context.hh               \
	ContextPopt.hh           \
	FerrisPopt.hh            \
	ValueRestorer.hh         \
	Runner.hh                \
	Ferrisls.hh              \
	Attribute.hh             \
	Resolver.hh              \
	Math.hh                  \
	ChainedViewContext.hh    \
	$(XMLHH)                 \
	BuildDependentMethods.hh \
	EAN.hh                   \
	FerrisBackup.hh          \
	FerrisCopy.hh            \
	FerrisRemove.hh          \
	FerrisSmartPointerChecker.hh \
	FerrisMove.hh            \
	ClipAPI.hh               \
	CursorAPI.hh             \
	Configuration.hh         \
	FerrisSTL.hh             \
	FerrisVersioning.hh      \
	ContextIterator.hh       \
	ContextSetCompare.hh     \
	Shell.hh                 \
	Mime.hh                  \
	ChildStreamServer.hh     \
	Hashing.hh               \
	AsyncIO.hh               \
	Runner_FunctorType.hh    \
	Chmod.hh                 \
	Daemon.hh                \
	Debug.hh                 \
	Iterator.hh              \
	FullTextIndexer.hh       \
	FullTextQuery.hh         \
	EAIndexer.hh             \
	EAQuery.hh               \
	FCA.hh                   \
	SchemaSupport.hh         \
	SchemaManips.hh          \
	Medallion.hh             \
	Cache.hh                 \
	Personalities.hh         \
	Agent.hh                 \
	Numeric.hh               \
	DublinCore.hh            \
	FullTextIndexerMetaInterface.hh \
	All.hh                          \
	EAIndexerMetaInterface.hh       \
	HiddenSymbolSupport.hh          \
	FerrisBitMagic.hh               \
	FerrisStdHashMap.hh             \
	FerrisFileActions.hh		\
	SyncDelayer.hh			\
	FerrisBoost.hh                  \
	WrapXMMS.hh                     \
	ContextPlugin.hh                \
	Ferrisls_AggregateData.hh       \
	FerrisSlaveProcess.hh           \
	TimeDelayedAction.hh

libferris_la_SOURCES =           \
	mocimpls.cpp             \
	Ferris.cpp               \
	Ferris_private.hh        \
	Image.cpp                \
	Image_private.hh         \
	Versioned.cpp            \
	Native.cpp               \
	ContextContext.cpp       \
	RootContext.cpp          \
	RootContext_private.hh   \
	FerrisException.cpp      \
	SignalStreams.cpp        \
	FerrisHandle.cpp         \
	FerrisEvent.cpp          \
	timber.cpp               \
	Enamel.cpp               \
	Enamel_priv.hh 		 \
	SM.cpp                   \
	SM_private.hh            \
	MatchedEAGenerators.cpp  \
	General.cpp              \
	FilteredContext.cpp      \
	Regex.cpp                \
	FerrisContextPlugin.cpp  \
	FerrisOpenSSL.cpp        \
	Context.cpp              \
	Context_private.hh       \
	FerrisPopt.cpp           \
	ContextPopt.cpp          \
	Runner.cpp               \
	Ferrisls.cpp             \
	Ferrisls_RDF.cpp         \
	Ferrisls_XML.cpp         \
	Ferrisls_JSON.cpp        \
	Attribute.cpp 		 \
	Attribute_private.hh     \
	FilteredContext_private.hh \
	Resolver.cpp 		 \
	Resolver_private.hh 	 \
	Math.cpp		 \
	Ferrisls_AggregateData.cpp  \
	ChainedViewContext.cpp   \
	FerrisOpenSSL.hh         \
	MainPageDocs.hh          \
	$(XMLCPP)                \
	$(XALANCPP)              \
	xfsutil.cpp              \
	xfsutil.hh               \
	EAN.cpp                  \
	FerrisBackup.cpp         \
	StatfsUtilities.cpp      \
	StatfsUtilities.hh       \
	FerrisCopy.cpp		 \
	FerrisCopy_private.hh    \
	FerrisRemove.cpp         \
	FerrisRemove_private.hh  \
	LinkContextScheme.cpp    \
	LinkContextScheme.hh     \
	LinkContextSchemeClients.cpp \
	LinkContextSchemeClients.hh  \
	FerrisMove.cpp               \
	ClipAPI.cpp                  \
	CursorAPI.cpp                \
	Configuration.cpp            \
	Configuration_private.hh     \
	FerrisVersioning.cpp         \
	ContextIterator.cpp          \
	ContextSetCompare.cpp        \
	ContextSetCompare_private.hh \
	Shell.cpp                    \
	Mime.cpp                     \
	FerrisKDE.cpp FerrisKDE.hh \
	ChildStreamServer.cpp        \
	AsyncIO.cpp                  \
	Chmod.cpp                    \
	PluginOutOfProcNotificationEngine.hh  \
	PluginOutOfProcNotificationEngine.cpp \
	common-ferris-out-of-proc-notification-deamon.hh  \
	common-ferris-out-of-proc-notification-deamon.cpp \
	Daemon.cpp               \
	Debug.cpp                \
	FullTextIndexer.cpp      \
	FullTextIndexer_private.hh \
	FullTextIndexerMetaInterface.cpp \
	FullTextQuery.cpp        \
	EAIndexer.cpp            \
	EAIndexer_private.hh     \
	FullTextIndexer_private.hh \
	EAQuery.cpp                \
	FerrisAsTextPlugin.hh      \
	FerrisAsTextPlugin.cpp     \
	FerrisCreationPlugin.hh    \
	FerrisCreationPlugin.cpp   \
	FerrisEAGeneratorPlugin.hh         \
	FerrisEAGeneratorPlugin.cpp        \
	FerrisEAGeneratorPlugin_private.hh \
	$(FCA_PQXX_DEP_SOURCES)    \
	FullTextContext.cpp        \
	FullTextContext.hh         \
	EAQueryContext.cpp         \
	EAQueryContext.hh          \
	RPM.cpp                    \
	RPM_private.hh             \
	$(LIBRPM_FILES)            \
	SpiritTime.cpp             \
	SchemaSupport.cpp          \
	SchemaManips.cpp           \
	Medallion.cpp              \
	Medallion_private.hh       \
	Cache.cpp                  \
	EtagereContext.cpp         \
	EtagereContext_private.hh  \
	FerrisDevContext.cpp         \
	FerrisDevContext_private.hh  \
	$(GPG_DEP_SRCS)              \
	FerrisGPG.cpp FerrisGPG_private.hh \
	Personalities.cpp            \
	Agent.cpp                    \
	Agent_private.hh             \
	$(XQILLA_DEP_SRCS)           \
	$(SVMLIGHT_DEP_SRCS)         \
	FerrisRDFCore.cpp FerrisRDFCore.hh \
	FerrisSemantic.cpp FerrisSemantic.hh \
	DublinCore.cpp               \
	BranchContext_private.hh BranchContext.cpp BranchContext_Clients.cpp \
	BranchRemembranceHistoryInternalContext.cpp \
	EAIndexerSQLCommon_private.hh  \
	EAIndexerSQLCommon.cpp         \
	EAIndexerCustomFerris.cpp      \
	FullTextIndexerCustomFerris.cpp \
	EAIndexerMetaInterface.cpp     \
	FactoriesCreationCommon_private.hh   \
	FactoriesCreationCommon_private.cpp  \
	CacheManager.cpp \
	CacheManager_private.hh \
	SpiritFFilter.cpp \
	FerrisFileActions.cpp \
	RemembranceContext.cpp \
	FullTextIndexerSyntheticDocID_private.hh \
	FullTextIndexerSyntheticDocID.cpp \
	SyncDelayer.cpp                   \
	FerrisBoost.cpp                   \
	FerrisCLucene.hh                  \
	$(XMMS_DEP_SOURCES)               \
	MetadataServer.cpp MetadataServer_private.hh \
	FSParser.cpp FSParser_private.hh  \
	$(DBUS_SOURCES)                   \
	$(CURL_FILES)                     \
	ForwardEAIndexInterface.hh ForwardEAIndexInterface.cpp \
	FerrisSlaveProcess.cpp FerrisSlaveProcess_private.hh \
	FerrisQt.cpp FerrisQt_private.hh \
	FerrisWebServices.cpp FerrisWebServices_private.hh \
	TimeDelayedAction.cpp

libferris_la_LIBADD =            \
	-lfam -lfampp            \
	-lcrypto -lssl           \
	@LOKI_LIBS@              \
	@FSTREAM_LIBS@           \
	@XML4C_LIBS@             \
	@XALAN_LIBS@             \
	@STLPORT_LIB@            \
	@DB4_LIBS@               \
	@GNOMEVFS_LIBS@          \
	@LIBFILE_LIBS@           \
	@EFSD_LIBS@              \
	@KDE_LIBS@               \
	@LIBMAGIC_LIBS@          \
	@LIBCAP_LIBS@            \
	@OPENSSL_LIBS@           \
	@LIBZ_LIBS@              \
	@BZIP2_LIBS@             \
	@STLDB4_LIBS@            \
	@UUID_LIBS@              \
	@LIBRPM_LIBS@            \
	@LIBTEXTCAT_LIBS@        \
	@GPGME_LIBS@             \
	@XQILLA_LIBS@            \
	@SOPRANO_LIBS@           \
	@NEPOMUK_LIBS@           \
	@LIBPQXX_LIBS@           \
	@DBUS_LIBS@              \
	@IMLIB_LIBS@             \
	@QTDBUS_LIBS@            \
	@LIBSELINUX_LIBS@        \
	@CURL_LIBS@              \
	-lpopt                   \
	@QJSON_LIBS@             \
	@BOOST_LIBS@            


libferris_la_LDFLAGS =              	  \
	@GLIB_LIBS@                 	  \
	@FAMPP2_LIBS@                     \
	@CUSTOMALLOC_LA@                  \
	@LIBFERRISMG_LA@                  \
	@LIBFERRISINDEXING_LA@            \
	@LIBASTEXT_PLUGIN_FACTORIES_LA@   \
	@LIBCREATION_PLUGIN_FACTORIES_LA@ \
	@LIBEAGEN_PLUGIN_FACTORIES_LA@    \
	@LIBIMAGEMAGICKNAMESPACEGLUE_LA@ @MAGICK_LIBS@ \
	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@


##########################################################################
##########################################################################
##########################################################################

include $(top_srcdir)/flymake.mk
