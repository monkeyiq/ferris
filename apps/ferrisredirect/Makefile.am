
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferris-redirect ferris-internal-file-to-fifo-command

dist_bin_SCRIPTS = ferris-file-to-fifo
# EXTRA_DIST = $(dist_bin_SCRIPTS)


INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

ferris_redirect_SOURCES = ferris-redirect.cpp
ferris_redirect_LDADD = -lpopt @LIBFERRIS_LA@ 

ferris_internal_file_to_fifo_command_SOURCES = ferris-internal-file-to-fifo-command.cpp
ferris_internal_file_to_fifo_command_LDADD = -lpopt @LIBFERRIS_LA@ 

