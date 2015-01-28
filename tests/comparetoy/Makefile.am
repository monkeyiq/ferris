
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS = comparetoy

INCLUDES =                                              \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@CFLAGS@

comparetoy_SOURCES = comparetoy.cpp
comparetoy_LDFLAGS = @GLIB_LIBS@
comparetoy_LDADD = -lpopt @LIBFERRIS_LA@ 


