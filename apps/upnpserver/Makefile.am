bin_PROGRAMS = ferris-upnp-server

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@LIBPLATINUM_CFLAGS@                            \
	@CFLAGS@

ferris_upnp_server_SOURCES = ferris-upnp-server.cpp 
ferris_upnp_server_LDADD = -lm -lpopt @LIBPLATINUM_LIBS@
ferris_upnp_server_LDFLAGS = @LIBFERRIS_LA@ 

