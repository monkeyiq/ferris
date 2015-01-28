
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferris-out-of-proc-notification-deamon

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@ \
	@CFLAGS@

ferris_out_of_proc_notification_deamon_SOURCES = ferris-out-of-proc-notification-deamon.cpp
ferris_out_of_proc_notification_deamon_LDADD = -lpopt @LIBFERRIS_LA@ 
ferris_out_of_proc_notification_deamon_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

