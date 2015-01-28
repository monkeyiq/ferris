
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                    		\
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-locale

ferris_capplet_locale_SOURCES = ferris-capplet-locale.cpp 
ferris_capplet_locale_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_capplet_locale_LDFLAGS = @GTK_LIBS@


