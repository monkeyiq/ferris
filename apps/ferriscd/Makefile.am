
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ferriscdexe 

SHELL_FILES = ferriscd
shelldir    = @DOTFILE_ETC@/profile.d/
shell_DATA  = $(SHELL_FILES)

EXTRA_DIST = $(SHELL_FILES)


INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

ferriscdexe_SOURCES = ferriscdexe.cpp
ferriscdexe_LDADD = -lpopt @LIBFERRIS_LA@ 
ferriscdexe_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@

