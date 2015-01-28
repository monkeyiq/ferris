if HAVE_GIMP

plugindir = $(libdir)/ferris/plugins/eagenerators

plugin_LTLIBRARIES = libgimp.la 
libgimp_la_SOURCES = libgimp.cpp

INCLUDES =						\
	@STLPORT_CFLAGS@                                \
	-I$(srcdir)/..                                  \
	-I$(top_srcdir)/src 				\
	-I$(includedir)					\
	@GTK_CFLAGS@                                    \
        -DREGEX_MALLOC                                  \
        -DG_LOG_DOMAIN=\"Ferris\"                       \
        @GIMP_THREAD_FLAGS@                             \
        @GIMP_MP_FLAGS@                 		\
	@CFLAGS@

#`gimp-config --cflags` -g -O0 -c MatchedEAGeneratorsGIMP.cpp


libgimp_la_LDFLAGS = @GTK_LIBS@                             \
	`gimp-config --libs`                                \
	$(GIMP_THREAD_LIBS)                                 \
	$(GIMP_MP_LIBS)

endif
