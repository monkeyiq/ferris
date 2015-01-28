
EXTRA_DIST = 

if HAVE_LIBDJVULIBRE
DJVU = libastextdjvu_factory.cpp
endif

if HAVE_PDFTOTEXT
PDF = libastextpdf_factory.cpp
endif

if HAVE_LYNX
HTML = libastexthtml_factory.cpp
endif

if HAVE_WV
WV = libastextwv_factory.cpp
endif

noinst_LTLIBRARIES = libastext_factories.la
libastext_factories_la_SOURCES =                    \
	libastextman_factory.cpp                    \
	$(DJVU)                                     \
	$(PDF)                                      \
	$(HTML)	                                    \
	$(WV)					    \
	libastextextern_factory.cpp

libastext_factories_la_LDFLAGS = @LIBFERRIS_ASTEXTPLUGIN_LT_LINE@


