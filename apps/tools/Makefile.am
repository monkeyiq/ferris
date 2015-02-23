bin_PROGRAMS = ftest fchown fchmod fstat freadlink

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@CFLAGS@

ftest_SOURCES = ftest.cpp 
ftest_LDADD = -lm -lpopt
ftest_LDFLAGS = @LIBFERRIS_LA@ 

fchown_SOURCES = fchown.cpp 
fchown_LDADD = -lm -lpopt
fchown_LDFLAGS = @LIBFERRIS_LA@ 

fchmod_SOURCES = fchmod.cpp 
fchmod_LDADD = -lm -lpopt
fchmod_LDFLAGS = @LIBFERRIS_LA@ 

fstat_SOURCES = fstat.cpp 
fstat_LDADD = -lm -lpopt
fstat_LDFLAGS = @LIBFERRIS_LA@ 

freadlink_SOURCES = freadlink.cpp 
freadlink_LDADD = -lm -lpopt
freadlink_LDFLAGS = @LIBFERRIS_LA@ 


