
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = XSLTransform

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@XALAN_CFLAGS@                                  \
	@CFLAGS@

XSLTransform_SOURCES = XSLTransform.cpp
XSLTransform_LDADD = -lpopt @LIBFERRIS_LA@ 
XSLTransform_LDFLAGS = @GLIB_LIBS@ @XALAN_LIBS@

