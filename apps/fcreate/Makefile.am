MAINTAINERCLEANFILES = Makefile.in

if HAVE_XALAN
TARGETS = fcreate
endif

bin_PROGRAMS = $(TARGETS)

INCLUDES = @GLIB_CFLAGS@ @CFLAGS@ @XML4C_CFLAGS@ @XALAN_CFLAGS@                   

fcreate_SOURCES = fcreate.cpp fcreate.hh ferriscreatecommon.cpp ferriscreatecommon.hh
fcreate_LDADD   = -lpopt 
fcreate_LDFLAGS = @GTK_LIBS@ @XML4C_LIBS@ @XALAN_LIBS@ @LIBFERRIS_LA@ 


