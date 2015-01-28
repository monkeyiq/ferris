
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fcat ftee

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@


fcat_SOURCES = fcat.cpp
fcat_LDADD = -lpopt @LIBFERRIS_LA@ 
fcat_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ftee_SOURCES = ftee.cpp
ftee_LDADD = -lpopt @LIBFERRIS_LA@ 
ftee_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

