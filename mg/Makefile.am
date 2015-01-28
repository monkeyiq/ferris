EXTRA_DIST = 

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	@SIGC_CFLAGS@                                   \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@CFLAGS@					\
	@COMPILE_SYMBOLS_AS_PUBLIC@

noinst_LTLIBRARIES = libferrismg.la
##libcustomalloc_la_CFLAGS = -O2
libferrismg_la_SOURCES =                \
	sysfuncs.h pathmax.h longlong.h \
	memlib.c memlib.h               \
	stem.c                    \
	stem.h                    \
	bitio_gen.c  bitio_gen.h  \
	bitio_mem.c  bitio_mem.h  \
	bitio_mems.c bitio_mems.h \
	bitio_m.h bitio_m_mem.h bitio_m_mems.h bitio_m_random.h \
	bitio_random.c bitio_random.h 

libferrismg_la_LIBADD  = $(LIBS_ALL)  @GLIB_LIBS@
libferrismg_la_LDFLAGS = -version-info 1:0:0 @LIBFERRIS_LIBS@ 

