
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fhead

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

fhead_SOURCES = fhead.cpp
fhead_LDADD = -lpopt @LIBFERRIS_LA@ 

