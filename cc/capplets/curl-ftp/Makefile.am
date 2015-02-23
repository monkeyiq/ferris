
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                    		\
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-curl-ftp

ferris_capplet_curl_ftp_SOURCES = ferris-capplet-curl-ftp.cpp
ferris_capplet_curl_ftp_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_capplet_curl_ftp_LDFLAGS = @GTK_LIBS@


