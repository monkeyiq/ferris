
if HAVE_GIMP
GIMPDIR = libgimp_factory.cpp
endif

if HAVE_IMLIB2
IM2DIR = libimlib2_factory.cpp
endif

if HAVE_MAGICK
MAGICDIR = libmagick_factory.cpp
endif

if HAVE_LIBPNG
PNGDIR = libferrispng_factory.cpp
endif

if HAVE_LIBJPEG
JPEGDIR = libferrisjpeg_factory.cpp
endif

if HAVE_ID3
ID3DIR = libferrisid3_factory.cpp
endif

if HAVE_FLAC
FLACDIR = libferrisflac_factory.cpp
endif

if HAVE_TAGLIB
TAGLIBDIR = libferristaglib_factory.cpp
endif

if HAVE_OGGZ
OGGZDIR = libferrisoggz_factory.cpp
endif

if HAVE_LIBMPEG3
MPEG3DIR = libferrismpeg3_factory.cpp
endif

if HAVE_LIBATTR
ATTRDIR = libxfsnative_factory.cpp
endif

if HAVE_LIBA52
A52 = libferrisa52_factory.cpp
endif

if HAVE_LIBMPG2
MPG2 = libferrismpeg2_factory.cpp
endif

if HAVE_LIBJASPER
JASPER = libferrisjasper_factory.cpp
endif

if HAVE_LIBDJVULIBRE
DJVU = libferrisdjvulibre_factory.cpp
endif


if HAVE_LIBEXIF
EXIF = libferrisexifea_factory.cpp
endif

if HAVE_XMP
XMP = libferrisxmp_factory.cpp
endif

if HAVE_LIBEXTRACTOR
LIBEXTRACTOR = libferrisextractor_factory.cpp
endif

# if FERRIS_BUILD_KDE_EAGENERATOR
# KDE3METADATA = libferriskde3metadata_factory.cpp
# endif

if FERRIS_BUILD_XINE_EAGENERATOR
XINEEA = libferrisxineea_factory.cpp
endif

if HAVE_FSPOT
FSPOT = libferrisfspot_factory.cpp
endif

if HAVE_STRIGI
STRIGI = libferrisstrigi_factory.cpp
endif

if HAVE_SOPRANO
SOPRANO = libferrissoprano_factory.cpp
endif

if HAVE_MEDIAINFO
MEDIAINFO = libferrismediainfoea_factory.cpp
endif

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib 						\
	@STLPORT_CFLAGS@                                \
	-I..                                            \
	-I$(top_srcdir)/plugins/eagenerators 		\
	-I$(includedir)					\
	@CFLAGS@

noinst_LTLIBRARIES = libferriseagenerator_factories.la
libferriseagenerator_factories_la_SOURCES = \
	$(ID3DIR) $(IM2DIR) $(MAGICDIR) $(PNGDIR)  \
	$(JPEGDIR) $(MPEG3DIR) $(ATTRDIR) $(GIMPDIR) \
	$(A52) $(MPG2) $(JASPER) $(DJVU) \
	$(EXIF) $(XMP) $(LIBEXTRACTOR) $(KDE3METADATA) $(XINEEA) \
	$(FSPOT) $(STRIGI) $(FLACDIR) $(OGGZDIR) $(TAGLIBDIR) \
	$(SOPRANO) $(MEDIAINFO) libferrissubtitles_factory.cpp

libferriseagenerator_factories_la_LDFLAGS = @LIBFERRIS_EAPLUGIN_LT_LINE@

