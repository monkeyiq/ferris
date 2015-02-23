
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@DB4_CFLAGS@                                    \
	@GTK_CFLAGS@                                    \
	@CFLAGS@

bin_PROGRAMS = ferris-first-time-user-ui

ferris_first_time_user_ui_SOURCES = ferris-first-time-user-ui.cpp 
ferris_first_time_user_ui_LDADD   = -lpopt @LIBFERRIS_LA@ 
ferris_first_time_user_ui_LDFLAGS = @GTK_LIBS@ @DB4_LIBS@

