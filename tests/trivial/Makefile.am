
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = test_setStrAttr

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   

test_setStrAttr_SOURCES = test_setStrAttr.cpp
test_setStrAttr_LDFLAGS = @STLPORT_LDFLAGS@ @GLIB_LIBS@        
test_setStrAttr_LDADD = -lpopt @LIBFERRIS_LA@         



