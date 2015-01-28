
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = echoservertls

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

echoservertls_SOURCES = echoservertls.cpp
echoservertls_LDADD =  -lpopt @LIBFERRIS_LA@ 
echoservertls_LDFLAGS = @GLIB_LIBS@


