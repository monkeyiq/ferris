EXTRA_DIST = 

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include \
	@SQLITE3_CFLAGS@ \
	@GLIB_CFLAGS@ \
	@CFLAGS@

lib_LTLIBRARIES        = libferrissqlitevtable.la

libferrissqlitevtable_la_SOURCES      =  \
	libferrissqlitevtable.cpp libferrissqlitevtable.hh

libferrissqlitevtable_la_LIBADD       = $(LIBS_ALL) @GLIB_LIBS@ @SIGC_LIBS@ @LIBFERRIS_LA@  @SQLITE3_LIBS@
libferrissqlitevtable_la_LDFLAGS      = -version-info 1:0:0 

