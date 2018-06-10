
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = cleanuptest singlecontext resolveagain filters chainedFilter simpleSort

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

EXTRA_DIST = vmdebug.cpp

cleanuptest_SOURCES = cleanuptest.cpp
cleanuptest_LDADD = -lpopt @LIBFERRIS_LA@ 
cleanuptest_LDFLAGS = $(GLIB_LIBS)

singlecontext_SOURCES = singlecontext.cpp
singlecontext_LDADD = -lpopt @LIBFERRIS_LA@ 
singlecontext_LDFLAGS = $(GLIB_LIBS)

resolveagain_SOURCES = resolveagain.cpp
resolveagain_LDADD = -lpopt @LIBFERRIS_LA@ 
resolveagain_LDFLAGS = $(GLIB_LIBS)

filters_SOURCES = filters.cpp
filters_LDADD = -lpopt @LIBFERRIS_LA@ 
filters_LDFLAGS = $(GLIB_LIBS)

chainedFilter_SOURCES = chainedFilter.cpp
chainedFilter_LDADD = -lpopt @LIBFERRIS_LA@ 
chainedFilter_LDFLAGS = $(GLIB_LIBS)

simpleSort_SOURCES = simpleSort.cpp
simpleSort_LDADD = -lpopt @LIBFERRIS_LA@ 
simpleSort_LDFLAGS = $(GLIB_LIBS)

