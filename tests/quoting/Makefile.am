NULL=

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in


bin_PROGRAMS = ftestshellquote

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

ftestshellquote_SOURCES = ftestshellquote.cpp
ftestshellquote_LDADD   = -lpopt @LIBFERRIS_LA@ 
ftestshellquote_LDFLAGS = @GLIB_LIBS@



