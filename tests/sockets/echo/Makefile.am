
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = echoclient

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

echoclient_SOURCES = echoclient.cpp
echoclient_LDADD =  -lpopt @LIBFERRIS_LA@ 
echoclient_LDFLAGS = @GLIB_LIBS@


