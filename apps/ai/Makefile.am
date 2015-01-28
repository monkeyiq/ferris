
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fagent fagentcreate

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

fagent_SOURCES = fagent.cpp
fagent_LDADD = -lpopt @LIBFERRIS_LA@ 
fagent_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

fagentcreate_SOURCES = fagentcreate.cpp
fagentcreate_LDADD = -lpopt @LIBFERRIS_LA@ 
fagentcreate_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@ 

