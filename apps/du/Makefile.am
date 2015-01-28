bin_PROGRAMS = fdu

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

fdu_SOURCES = fdu.cpp 
fdu_LDADD = -lm -lpopt
fdu_LDFLAGS = @LIBFERRIS_LA@ 

