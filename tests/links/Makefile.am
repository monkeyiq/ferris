NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = linkplay

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

linkplay_SOURCES = linkplay.cpp
linkplay_LDADD   = -lpopt @LIBFERRIS_LA@ 
linkplay_LDFLAGS = @GLIB_LIBS@




