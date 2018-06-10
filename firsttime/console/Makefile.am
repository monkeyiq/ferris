
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@DB4_CFLAGS@                                    \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

skelsetupdir = $(bindir)
skelsetup_SCRIPTS = ferris-setup-from-skels.sh
EXTRA_DIST = ferris-setup-from-skels.sh

bin_PROGRAMS = ferris-first-time-user

ferris_first_time_user_SOURCES = ferris-first-time-user.cpp 
ferris_first_time_user_LDADD   = -lpopt @LIBFERRIS_LA@ @SHAREDSQLPLUS_LA@ 
ferris_first_time_user_LDFLAGS = @GLIB_LIBS@ @DB4_LIBS@

