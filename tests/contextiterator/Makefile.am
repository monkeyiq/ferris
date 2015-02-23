
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = 	forwardsweep    backwardsweep    \
		forwardsweepcre backwardsweepcre \
		knightrider     randomover       \
		itemremovedfromunder

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@GLIB_CFLAGS@ \
	@CFLAGS@

forwardsweep_SOURCES = forwardsweep.cpp
forwardsweep_LDADD   = -lpopt @LIBFERRIS_LA@ 
forwardsweep_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

backwardsweep_SOURCES = backwardsweep.cpp
backwardsweep_LDADD   = -lpopt @LIBFERRIS_LA@ 
backwardsweep_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

forwardsweepcre_SOURCES = forwardsweepcre.cpp
forwardsweepcre_LDADD   = -lpopt @LIBFERRIS_LA@ 
forwardsweepcre_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

backwardsweepcre_SOURCES = backwardsweepcre.cpp
backwardsweepcre_LDADD   = -lpopt @LIBFERRIS_LA@ 
backwardsweepcre_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

knightrider_SOURCES = knightrider.cpp
knightrider_LDADD   = -lpopt @LIBFERRIS_LA@ 
knightrider_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

randomover_SOURCES = randomover.cpp
randomover_LDADD   = -lpopt @LIBFERRIS_LA@ 
randomover_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

itemremovedfromunder_SOURCES = itemremovedfromunder.cpp
itemremovedfromunder_LDADD   = -lpopt @LIBFERRIS_LA@ 
itemremovedfromunder_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

