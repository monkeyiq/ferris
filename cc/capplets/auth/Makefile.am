
# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES =                                              \
	@STLPORT_CFLAGS@                                \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/src	\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	@GTK_CFLAGS@                     		\
	@SQLPLUS_CFLAGS@                                \
	-I@SQLPLUSMODDIR@				\
	@LIBPQXX_CFLAGS@                                \
	-I@PGCTXMODDIR@                                 \
	-I@ZMCTXMODDIR@                                 \
	-I@FERRISRESTCTXMODDIR@                         \
	@DTL_CFLAGS@                                    \
	-I@DTLMODDIR@				        \
	@LDAP_CFLAGS@                                   \
	-I@LDAPMODDIR@				        \
	@OBBY_CFLAGS@                                   \
	-I@OBBYMODDIR@                                  \
	-I@WIKIDIR@                                     \
	@QT_CFLAGS@                                     \
	@QTNETWORK_CFLAGS@                              \
	@QJSON_CFLAGS@ @QOAUTH_CFLAGS@                  \
	@CFLAGS@

bin_PROGRAMS = ferris-capplet-auth
ferris_capplet_auth_SOURCES = ferris-capplet-auth.cpp 

ferris_capplet_auth_LDADD   = -lpopt @LIBFERRIS_LA@ @LIBFERRISUI_LA@ \
	@SHAREDSQLPLUS_LA@                          \
	@SHAREDPGCTX_LA@                            \
	@SHAREDZMCTX_LA@                            \
	@SHAREDFERRISRESTCTX_LA@                    \
	@SHAREDDTL_LA@                              \
	@SHAREDLDAP_LA@                             \
	@SHAREDWEBPHOTOS_LA@                        \
	@CTXGOOGLE_LA@                              \
	@CTXFACEBOOK_LA@                            \
	@CTXVIMEO_LA@                               \
	@CTXIDENTICA_LA@                            \
	@SHAREDOBBY_LA@                             \
        @SHAREDWIKI_LA@                             

ferris_capplet_auth_LDFLAGS = @GTK_LIBS@ \
	@SQLPLUS_LIBS@                   \
	@LIBPQXX_LIBS@                   \
	@DTL_LIBS@                       \
	@LDAP_LIBS@                      \
	@QT_LIBS@                        \
	@QTNETWORK_LIBS@                 \
        @QOAUTH_LIBS@ @QJSON_LIBS@       \
	@OBBY_LIBS@                      


