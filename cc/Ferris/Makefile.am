
CC_DIR_FILES = $(srcdir)/*.directory
CC_DESKTOP_FILES = $(srcdir)/*.desktop
CC_FILES = $(CC_DIR_FILES) $(CC_DESKTOP_FILES)
CC_ICONS = $(srcdir)/*png

EXTRA_DIST = $(CC_DIR_FILES) $(CC_DESKTOP_FILES) $(CC_ICONS)

CC_TARGET_DIR = $(DESTDIR)/usr/share/control-center/capplets/Ferris/
CC_TARGET_ICON_DIR = $(DESTDIR)/usr/share/control-center-plus/icons/

install-data-local:
	install -d $(CC_TARGET_DIR)/;
	install -d $(CC_TARGET_ICON_DIR)/;

	@for f in $(CC_FILES); do \
		echo install of file $$f; \
		install -v $$f $(CC_TARGET_DIR)/; \
	done

	@for f in $(CC_DIR_FILES); do \
		install -v $$f $(CC_TARGET_DIR)/; \
	done

	@for f in $(CC_ICONS); do \
		install -v $$f $(CC_TARGET_ICON_DIR)/; \
	done

