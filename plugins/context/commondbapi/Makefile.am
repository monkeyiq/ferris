INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@GLIB_CFLAGS@                                   \
	@CFLAGS@

noinst_LTLIBRARIES = libcommondbapi.la
libcommondbapi_la_SOURCES = libcommondbapi.cpp libcommondbapi.hh

EXTRA_DIST = gdbm.h
