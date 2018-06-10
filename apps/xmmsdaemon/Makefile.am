
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

if HAVE_XMMS
XMMS_SERVERCOMMAND = ferris-internal-xmms-server
endif

bin_PROGRAMS = $(XMMS_SERVERCOMMAND) 

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(top_srcdir)/ego -I../ego			\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@CFLAGS@ 

ferris_internal_xmms_server_SOURCES = ferris-internal-xmms-server.cpp
ferris_internal_xmms_server_CXXFLAGS = @LIBXMMS_CFLAGS@
ferris_internal_xmms_server_LDADD = -lm -lpopt
ferris_internal_xmms_server_LDFLAGS = @LIBXMMS_LIBS@ 


