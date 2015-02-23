
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = contextsweep 

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

contextsweep_SOURCES = contextsweep.cpp
contextsweep_LDADD = -lpopt @LIBFERRIS_LA@ 
contextsweep_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@


