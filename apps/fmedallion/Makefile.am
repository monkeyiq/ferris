
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = fmedallion ferris-get-emblems-within-range

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

fmedallion_SOURCES = fmedallion.cpp
fmedallion_LDADD = -lpopt @LIBFERRIS_LA@ 
fmedallion_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

ferris_get_emblems_within_range_SOURCES = ferris-get-emblems-within-range.cpp
ferris_get_emblems_within_range_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_get_emblems_within_range_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

