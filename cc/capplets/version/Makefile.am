
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                    		\
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-version

ferris_capplet_version_SOURCES = ferris-capplet-version.cpp 
ferris_capplet_version_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_capplet_version_LDFLAGS = @GTK_LIBS@


