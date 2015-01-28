
PROJECT = libferrismount
VERSION = 0.5

all: $(PROJECT).xpi

FFOX_ADDON_FILES = \
	chrome/libferrismount/content/about.xul \
	chrome/libferrismount/content/contents.rdf \
	chrome/libferrismount/content/libferrismountOverlay.js \
	chrome/libferrismount/content/libferrismountOverlay.xul \
	\
	skin/classic/libferrismount/libferris.png \
	skin/classic/libferrismount/contents.rdf \
	chrome.manifest		\
	install.rdf		

EXTRA_DIST = $(FFOX_ADDON_FILES)
CLEANFILES = $(PROJECT).xpi

$(PROJECT).xpi: $(OUTER_XPI_FILES)
	echo "Setting up pwd:`pwd`"
	if [ x"$(srcdir)" != "x." ]; then \
		cp -a $(srcdir)/skin . ;\
		cp -a $(srcdir)/chrome . ;\
		cp -a $(srcdir)/install.rdf . ;\
		cp -a $(srcdir)/chrome.manifest . ;\
	fi;
	zip -q9r $(PROJECT).xpi .


