
EXTRA_DIST = 

if HAVE_TDB
TDB = libcreationtdb_factory.cpp
endif

if HAVE_DB4
DB4 = libcreationdb4_factory.cpp
endif

if HAVE_GDBM
GDBM = libcreationgdbm_factory.cpp
endif

if HAVE_MAGICK
MAGICKIMAGE = libcreationmagick_factory.cpp
endif

if HAVE_DBXML
DBXMLDIR = libcreationdbxml_factory.cpp
endif

noinst_LTLIBRARIES = libcreation_factories.la
libcreation_factories_la_SOURCES =                   \
	libcreationdevsource_factory.cpp             \
	libcreationeaindex_factory.cpp               \
	libcreationeaindexdb4tree_factory.cpp        \
	libcreationfulltextindex_factory.cpp         \
	libcreationmarkup_factory.cpp                \
	libcreationmng_factory.cpp                   \
	libcreationmp3_factory.cpp                   \
	libcreationmpeg2_factory.cpp                 \
	libcreationogg_factory.cpp                   \
	libcreationshellscript_factory.cpp           \
	libcreationwav_factory.cpp                   \
	$(TDB)                                       \
	$(GDBM)                                      \
	$(DB4)                                       \
	$(MAGICKIMAGE)                               \
	$(DBXMLDIR)                                  \
	libcreationsopranoredland_factory.cpp

libcreation_factories_la_LDFLAGS = @LIBFERRIS_CREATIONPLUGIN_LT_LINE@
