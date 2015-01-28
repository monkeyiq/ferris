
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferris-file-action ferris-set-file-action-for-type

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@CFLAGS@

ferris_file_action_SOURCES = ferris-file-action.cpp
ferris_file_action_LDADD = -lpopt @LIBFERRIS_LA@ 

ferris_set_file_action_for_type_SOURCES = ferris-set-file-action-for-type.cpp
ferris_set_file_action_for_type_LDADD = -lpopt @LIBFERRIS_LA@ 

