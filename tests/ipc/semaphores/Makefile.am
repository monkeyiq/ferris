
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = semrawcreate fgetsem fmodifysem

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

semrawcreate_SOURCES = semrawcreate.cpp
semrawcreate_LDADD =  -lpopt @LIBFERRIS_LA@ 
semrawcreate_LDFLAGS = @GLIB_LIBS@

fgetsem_SOURCES = fgetsem.cpp
fgetsem_LDADD =  -lpopt @LIBFERRIS_LA@ 
fgetsem_LDFLAGS = @GLIB_LIBS@

fmodifysem_SOURCES = fmodifysem.cpp
fmodifysem_LDADD =  -lpopt @LIBFERRIS_LA@ 
fmodifysem_LDFLAGS = @GLIB_LIBS@


