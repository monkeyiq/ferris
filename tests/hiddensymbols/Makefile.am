if BUILD_HIDDENSYMBOL_TEST

NULL=
lib_LTLIBRARIES = libferrishsrtest.la

INCLUDES = -I$(top_builddir)/lib -I$(top_srcdir)/lib                                               \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_srcdir)/Ferris                          \
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	-I$(top_srcdir)/plugins/context/                \
	-I/usr/local/include                            \
	@GLIB_CFLAGS@                                   \
	@LOKI_CFLAGS@                                   \
	@FSTREAM_CFLAGS@                                \
	-I/usr/local/witme-file-3.35/include            \
	@FAMPP2_CFLAGS@                                 \
	@XML4C_CFLAGS@                                  \
	@XALAN_CFLAGS@                                  \
	@DB4_CFLAGS@                                    \
	@GNOMEVFS_CFLAGS@                               \
	@LIBFILE_CFLAGS@                                \
	@EFSD_CFLAGS@                                   \
	@LIBMAGIC_CFLAGS@                               \
	@OPENSSL_CFLAGS@                                \
	@LIBZ_CFLAGS@                                   \
	@BZIP2_CFLAGS@                                  \
	@STLDB4_CFLAGS@                                 \
	@LIBRPM_CFLAGS@                                 \
	@XQILLA_CFLAGS@                                 \
	@REDLAND_CFLAGS@                                \
	@LIBPQXX_CFLAGS@                                \
	@LIBTEXTCAT_CFLAGS@                             \
	@GPGME_CFLAGS@                                  \
	@BOOST_CFLAGS@                                  \
	-DBUILDING_LIBFERRIS                            \
	@DBUS_CFLAGS@                                   \
	@IMLIB_CFLAGS@		                        \
	@QTDBUS_CFLAGS@                                \
	@LIBSELINUX_CFLAGS@                             \
	@CURL_CFLAGS@                                   \
	@XERCES_INTERNAL_HEADERS_CFLAGS@                \
	@CFLAGS@                                        

plugindir = /tmp/ferris/plugins/context
plugin_LTLIBRARIES               = libhsrmodule.la
libhsrmodule_la_SOURCES         = libhsrmodule.cpp libhsrmodule.hh
libhsrmodule_la_LDFLAGS =                            \
	@GLIB_LIBS@                 	             \
	@LIBFERRIS_PLUGIN_LT_LINE@                        




libferrishsrtest_la_SOURCES =           \
	libferrishsrtest.cpp libferrishsrtest.hh

libferrishsrtest_la_LDFLAGS = \
	@GLIB_LIBS@                 	  \
	@FAMPP2_LIBS@                     \
	@CUSTOMALLOC_LA@                  \
	@LIBFERRISMG_LA@                  \
	@LIBFERRISINDEXING_LA@            \
	@LIBASTEXT_PLUGIN_FACTORIES_LA@   \
	@LIBCREATION_PLUGIN_FACTORIES_LA@ \
	@LIBEAGEN_PLUGIN_FACTORIES_LA@    \
	@LIBIMAGEMAGICKNAMESPACEGLUE_LA@ @MAGICK_LIBS@ \
	-version-info @LIBFERRIS_LT_CURRENT@:@LIBFERRIS_LT_REVISION@:@LIBFERRIS_LT_AGE@ \
	@LIBFERRIS_LA@ \
	libhsrmodule.la \
	\$(top_builddir)/plugins/context/xml/libxml.la

noinst_PROGRAMS = hsrtest
hsrtest_SOURCES = hsrtest.cpp
hsrtest_LDADD = -lpopt @LIBFERRIS_LA@  libferrishsrtest.la
hsrtest_LDFLAGS = $(GLIB_LIBS) @SIGC_LIBS@


endif
