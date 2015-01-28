# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in


if FERRIS_HAVE_CGICC

cgibindir = /var/www/cgi-bin/ferris
cgibin_PROGRAMS = rest

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

rest_SOURCES = rest.cpp 
rest_LDADD = -lm -lpopt
rest_LDFLAGS = @LIBFERRIS_LA@ \
	-L/usr/lib -lcgicc


endif
