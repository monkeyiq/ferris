
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = streamtimes

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@ \
	@CFLAGS@

streamtimes_SOURCES = streamtimes.cpp
streamtimes_LDADD = -lpopt @LIBFERRIS_LA@ 
streamtimes_LDFLAGS = $(GTK_LIBS)

