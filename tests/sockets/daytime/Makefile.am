
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = daytimeclient

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

daytimeclient_SOURCES = daytimeclient.cpp
daytimeclient_LDADD =  -lpopt @LIBFERRIS_LA@ 
daytimeclient_LDFLAGS = @GLIB_LIBS@


