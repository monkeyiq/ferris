
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@GCJ_CFLAGS@                                    \
	@CFLAGS@                                        

COMMON_GCJFLAGS = --encoding=ISO-8859-1 -Wno-deprecated
#BUILT_SOURCES = MyAnalyzer.h
#%.h: %.java
#	$(GCJ) -C $(COMMON_GCJFLAGS) $(AM_GCJFLAGS) $(GCJFLAGS) $<
#	gcjh  --classpath `pwd`  `echo $< | sed "s/.java//g"`;

###################################################################
###################################################################
###################################################################

lib_LTLIBRARIES      = libferrisgcj.la

ferrisincludedir=$(includedir)/Ferris
ferrisinclude_HEADERS =          
libferrisgcj_la_SOURCES = FerrisGCJ.cpp FerrisGCJ_private.hh 

libferrisgcj_la_LIBADD = @GCJ_LIBS@               


libferrisgcj_la_LDFLAGS =              	  \
	@GLIB_LIBS@                 	  \
	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@


##########################################################################
##########################################################################
##########################################################################

