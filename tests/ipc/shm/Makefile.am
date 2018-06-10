
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

XML_FILES = 
EXTRA_DIST = $(XML_FILES)

xmldir = ${datadir}/ferris/xml
xml_DATA = $(XML_FILES)

bin_PROGRAMS = fshmput fshmget shmrawget shmrawput

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

fshmput_SOURCES = fshmput.cpp
fshmput_LDADD = -lpopt @LIBFERRIS_LA@ 
fshmput_LDFLAGS = @GLIB_LIBS@

fshmget_SOURCES = fshmget.cpp
fshmget_LDADD = -lpopt @LIBFERRIS_LA@ 
fshmget_LDFLAGS = @GLIB_LIBS@

shmrawget_SOURCES = shmrawget.cpp
shmrawget_LDADD = -lpopt @LIBFERRIS_LA@ 
shmrawget_LDFLAGS = @GLIB_LIBS@

shmrawput_SOURCES = shmrawput.cpp
shmrawput_LDADD = -lpopt @LIBFERRIS_LA@ 
shmrawput_LDFLAGS = @GLIB_LIBS@

