
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = socketopsclient

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

socketopsclient_SOURCES = socketopsclient.cpp
socketopsclient_LDADD =   -lpopt @LIBFERRIS_LA@ 
socketopsclient_LDFLAGS = @GLIB_LIBS@


