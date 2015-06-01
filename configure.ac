PACKAGE=libferris
AC_INIT(libferris, 1.5.30)
LIBFERRIS_BETA=30
LIBFERRIS_MINOR=5
LIBFERRIS_MAJOR=1

dnl
dnl  /usr/local/src/git/gnulib/gnulib-tool --import strverscmp readlinkat extern-inline
dnl 

AC_SUBST(LIBFERRIS_MAJOR)
AC_SUBST(LIBFERRIS_MINOR)
AC_SUBST(LIBFERRIS_BETA)

FERRIS_VERSION=$LIBFERRIS_MAJOR.$LIBFERRIS_MINOR.$LIBFERRIS_BETA
LIBFERRIS_FULLV=$FERRIS_VERSION
AC_SUBST(LIBFERRIS_FULLV)
AC_SUBST(FERRIS_VERSION)
AC_DEFINE_UNQUOTED(FERRIS_VERSION, "$FERRIS_VERSION", [] )

VERSION=$FERRIS_VERSION
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [] )

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(Ferris/Ferris.cpp)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([1.9 tar-ustar])
AM_MAINTAINER_MODE


cd ${srcdir}; absolute_srcdir=`pwd`; cd -;

on_osx=`uname -a | grep -q -v '^Darwin'; echo $?`


dnl
dnl The rules are that 1.0.x will be stable, and 1.1.x will be development.
dnl I assume to break ABI if not API between 1.0.x and 1.2.x so the major/minor
dnl form an ABI sentinal and the BETA releases are to be ABI compatible for the stable tree.
dnl ABI will probably break for each development snapshot.
dnl
if test x$LIBFERRIS_MAJOR = x0; then
	LIBFERRIS_LT_CURRENT=$LIBFERRIS_MINOR
else
	LIBFERRIS_LT_CURRENT=$LIBFERRIS_MAJOR$LIBFERRIS_MINOR
fi
LIBFERRIS_LT_REVISION=$LIBFERRIS_BETA
LIBFERRIS_LT_AGE=0
if test x$LIBFERRIS_MINOR = x1 || test x$LIBFERRIS_MINOR = x9; then
	echo Development tree...
	LIBFERRIS_LT_REVISION=$LIBFERRIS_MINOR$LIBFERRIS_BETA
fi

AC_SUBST(LIBFERRIS_LT_CURRENT)
AC_SUBST(LIBFERRIS_LT_REVISION)
AC_SUBST(LIBFERRIS_LT_AGE)

LIBFERRIS_PLUGIN_LT_LINE=" -version-info $LIBFERRIS_LT_CURRENT:0:0 "
AC_SUBST(LIBFERRIS_PLUGIN_LT_LINE)
LIBFERRIS_EAPLUGIN_LT_LINE=" -version-info $LIBFERRIS_LT_CURRENT:0:0 "
AC_SUBST(LIBFERRIS_EAPLUGIN_LT_LINE)
LIBFERRIS_ASTEXTPLUGIN_LT_LINE=" -version-info $LIBFERRIS_LT_CURRENT:0:0 "
AC_SUBST(LIBFERRIS_ASTEXTPLUGIN_LT_LINE)
LIBFERRIS_CREATIONPLUGIN_LT_LINE=" -version-info $LIBFERRIS_LT_CURRENT:0:0 "
AC_SUBST(LIBFERRIS_CREATIONPLUGIN_LT_LINE)

dnl
dnl This is made throughout, as each text passes it appends its
dnl to-text factory to be statically linked into libferris
dnl
dnl LIBASTEXT_PLUGIN_FACTORY_PREFIX="\$(top_builddir)/plugins/astext"
LIBASTEXT_PLUGIN_FACTORIES_LA=" \$(top_builddir)/factories/astext/libastext_factories.la "
dnl
dnl This is made throughout, as each text passes it appends its
dnl creation factory to be statically linked into libferris
dnl
dnl LIBCREATION_PLUGIN_FACTORY_PREFIX="\$(top_builddir)/plugins/creation"
LIBCREATION_PLUGIN_FACTORIES_LA=" \$(top_builddir)/factories/creation/libcreation_factories.la "
dnl
dnl This is made throughout, as each test passes it appends its
dnl ea factory to be statically linked into libferris
dnl
dnl LIBEAGEN_PLUGIN_FACTORY_PREFIX="\$(top_builddir)/plugins/eagenerators"
LIBEAGEN_PLUGIN_FACTORIES_LA=" \$(top_builddir)/factories/eagenerators/libferriseagenerator_factories.la "


RPM_BUILD_ROOT=`eval echo ${RPM_BUILD_ROOT}`
AC_SUBST(RPM_BUILD_ROOT)

LDFLAGS=" $LDFLAGS -L/usr/local/lib "


dnl libtool versioning
LT_RELEASE=$LIBFERRIS_MAJOR.$LIBFERRIS_MINOR
AC_SUBST(LT_RELEASE)

AC_CANONICAL_HOST
AC_ISC_POSIX

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AM_PROG_GCJ
gl_EARLY


AC_STDC_HEADERS
AC_SYS_LARGEFILE
dnl AC_ARG_PROGRAM
AC_CONFIG_MACRO_DIR([m4])
AC_C_RESTRICT

AC_ARG_ENABLE(fastinstall,
  [AS_HELP_STRING([--enable-fastinstall],
                  [dont call AC_DISABLE_FAST_INSTALL (default=yes)])],[],[])

m4_pattern_allow([^AC_DISABLE_FAST_INSTALL$])
if test x$enable_fastinstall = xno; then
	echo "disable fast install for inline testing...";
	AC_DISABLE_FAST_INSTALL
else
	echo "fast install left intact...";
fi

LT_INIT([disable-static])
LT_LANG([C++])
LT_OUTPUT

dnl AC_DISABLE_STATIC
dnl AM_PROG_LIBTOOL


FERRIS_CF_X86=" -fomit-frame-pointer -fno-exceptions -pipe -s -mpentium -mcpu=pentium -ffast-math -fexpensive-optimizations -march=i686 -funroll-all-loops "
FERRIS_CF=" -O3 -DNDEBUG -I/nothing -DG_DISABLE_ASSERT -ffast-math "
FERRIS_LD=""
smartptr_assert=""

###############################################################################
## Debug,profile or production CFLAGS #########################################
###############################################################################
AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug],
 [compile with -g and -O0 debug information. Invalid smart pointer dereferences will give a backtrace (default=no)])],
[],[])
if test x$enable_debug = xyes; then
	echo "setting debug mode to on..."
	FERRIS_CF=" -O0 -g -pipe -fno-inline -fno-default-inline "; #-Wall "; 
	FERRIS_CF=" -O0 -g -pipe -fno-inline ";
	smartptr_assert="if( !val ) { ::Ferris::BackTrace(); assert(val); }";
else
	echo "setting debug mode to off..."
fi


AC_SUBST( FERRIS_SMARTPTR_ASSERT, $smartptr_assert )
dnl #AC_DEFINE_UNQUOTED(FERRIS_SMARTPTR_CHECKINGPOLICY, $ferris_smartptr_checkingpolicy)

AC_ARG_ENABLE(profile,
  [AS_HELP_STRING([--enable-profile], [compile with profile debug information (default=no)])],
[],[])
if test x$enable_profile = xyes; then
	echo "setting profile mode to on..."
	FERRIS_CF=" -O0 -g -pg -fprofile-arcs -ftest-coverage "; 
else
	echo "setting profile mode to off..."
fi



AC_ARG_ENABLE(stlport-stlport-nocustomio,
  [AS_HELP_STRING([--enable-stlport-nocustomio], [compile with _STLP_NO_CUSTOM_IO where you can (default=yes)])],
[],[enable_stlport_nocustomio=yes])

if test x$enable_stlport_nocustomio = xyes; then
	echo setting _STLP_NO_CUSTOM_IO to on...
	FERRIS_CF=" $FERRIS_CF -D_STLP_NO_CUSTOM_IO "; 
else
	echo not using _STLP_NO_CUSTOM_IO...
fi



AC_ARG_ENABLE(vmdebug,
  [AS_HELP_STRING([--enable-vmdebug], [compile with extra printouts for VM debug msgs (default=no)])],
[],[])

if test x$enable_vmdebug = xyes; then
	echo setting vm debug mode to on...
	AC_DEFINE( FERRIS_DEBUG_VM, 1, [Extra debug information on VM system] )
else
	echo setting vm debug mode to off...
fi



AC_ARG_ENABLE(resolvedebug,
  [AS_HELP_STRING([--enable-resolvedebug], [compile with extra printouts for resolve() debug msgs (default=no)])],
[],[])

if test x$enable_resolvedebug = xyes; then
	echo "setting resolve() debug mode to on..."
	AC_DEFINE( FERRIS_DEBUG_RESOLVE,1,[Debuging resolve() method] )
else
	echo "setting resolve() debug mode to off..."
fi



CONTEXT_PLUGIN_CFLAGS=
COMPILE_SYMBOLS_AS_PUBLIC=
AC_ARG_ENABLE(hiddensymbols,
  [AS_HELP_STRING([--enable-hiddensymbols], [use hidden symbols for private APIs (default=no)])],
[],[])
if test x$enable_hiddensymbols = xyes; then
	echo setting hidden symbol support...;
#	FERRIS_CF=" $FERRIS_CF -DGCC_HASCLASSVISIBILITY -fvisibility=default -fvisibility-inlines-hidden ";
	FERRIS_CF=" $FERRIS_CF -DGCC_HASCLASSVISIBILITY -DBUILDING_LIBFERRIS -fvisibility=hidden  -fvisibility-inlines-hidden ";
	AC_DEFINE(GCC_HASCLASSVISIBILITY,1, [] )
	COMPILE_SYMBOLS_AS_PUBLIC=" -DGCC_HASCLASSVISIBILITY -DBUILDING_LIBFERRIS -fvisibility=default "
	CONTEXT_PLUGIN_CFLAGS=" -DBUILDING_LIBFERRIS "
fi

AC_SUBST(COMPILE_SYMBOLS_AS_PUBLIC)
AC_SUBST(CONTEXT_PLUGIN_CFLAGS)


AC_ARG_ENABLE(hiddensymbolstests,
  [AS_HELP_STRING([--enable-hiddensymbolstests], [compile some test programs using hidden symbols for private APIs (default=no)])],
[],[])
AM_CONDITIONAL(BUILD_HIDDENSYMBOL_TEST, test x$enable_hiddensymbolstests = xyes)

gl_INIT
gl_EXTERN_INLINE
LDADD += lib/libgnu.a

dnl ###############################################################################
dnl ###############################################################################
dnl ###############################################################################
dnl # Build FCA support
dnl ###############################################################################

dnl AC_ARG_ENABLE(fca,
dnl  [AS_HELP_STRING([--enable-fca], [build support for performing Formal Concept Analysis (default=no)])],
dnl [],[])
dnl 
dnl if test x$enable_fca = xyes; then
dnl 	echo Building Formal Concept Analysis code...
dnl  	AC_DEFINE(HAVE_FCA)
dnl  	AC_DEFINE(BUILD_FCA)
dnl fi


###############################################################################
###############################################################################
###############################################################################
# Test for stlport 4.5
###############################################################################

AM_FERRIS_STLPORT_OPTIONAL( 4.5.0 )


###############################################################################
###############################################################################
###############################################################################

AC_CHECK_FUNCS([posix_fallocate posix_fadvise truncate ftruncate])

###############################################################################
###############################################################################
###############################################################################
# Test for readlinkat(2)
###############################################################################

AC_TRY_LINK([ #include <unistd.h> ],
	[
	    int dirfd = 1;
	    const char *path;
	    char *buf;
	    size_t bufsiz;
	    int rc = readlinkat( dirfd, path, buf, bufsiz);
	],
       	[
		AC_DEFINE(FERRIS_HAVE_READLINKAT, 1, [] )
		have_readlinkat=yes;
	],
	[
		have_readlinkat=no;
	])
AM_CONDITIONAL(FERRIS_HAVE_READLINKAT, test x"$have_readlinkat" = xyes)

###############################################################################
###############################################################################
###############################################################################
# Test for sqrtl(3)
###############################################################################

AC_TRY_LINK([ #include <math.h> ],
	[
		long double v = 100;
		long double x = sqrtl( v );
	],
       	[
		AC_DEFINE(FERRIS_HAVE_SQRTL, 1, [] )
		have_sqrtl=yes;
	],
	[
		have_sqrtl=no;
	])
AM_CONDITIONAL(FERRIS_HAVE_SQRTL, test x"$have_sqrtl" = xyes)


###############################################################################
###############################################################################
###############################################################################
# Test for xml4c (xerces-c)
###############################################################################

AM_FERRIS_XERCESC3( 3.0.0 )

###############################################################################
###############################################################################
###############################################################################
# Test for xalan
###############################################################################

AM_FERRIS_XALAN( 1.10.0, [], [echo "not building Xalan-C support..."] )

AM_CONDITIONAL(FERRIS_HAVE_XSLTFS_MOUNT_ABILITY, test x"$have_xalan" = xyes, [] )
XSLTFS_MOUNT_CFLAGS=
XSLTFS_MOUNT_LIBS=
AC_SUBST(XSLTFS_MOUNT_CFLAGS)
AC_SUBST(XSLTFS_MOUNT_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for redland RDF library
###############################################################################

dnl package=redland
dnl AC_CHECK_PROG( have_redland, redland-config, yes, no )
dnl if test "$have_redland" = yes; then
dnl 	REDLAND_LIBS="   `redland-config --libs`   `rasqal-config --libs`   `raptor-config --libs`"
dnl 	REDLAND_CFLAGS=" `redland-config --cflags` `rasqal-config --cflags` `rasqal-config --cflags`"
dnl 	AC_DEFINE(HAVE_REDLAND)
dnl dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/redland/libferrisredland_factory.la "

dnl dnl 	PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl dnl 	LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/redland/libcreationredland_factory.la "
dnl else
dnl   AC_MSG_ERROR([Fatal Error: The redland RDF library is required... http://www.redland.opensource.ac.uk/])	
dnl fi

dnl dnl
dnl dnl -ldb-4.5 -lcurl -lxslt
dnl dnl
dnl AC_ARG_WITH( redland-extra-client-libs,
dnl  [  --with-redland-extra-client-libs=extra libraries to link for static redland],
dnl  [REDLAND_LIBS="$REDLAND_LIBS $withval "
dnl  ])

dnl AM_CONDITIONAL(HAVE_REDLAND, test x"$have_redland" = xyes)
dnl AC_SUBST(REDLAND_LIBS)
dnl AC_SUBST(REDLAND_CFLAGS)

have_redland=no
AM_CONDITIONAL(HAVE_REDLAND, test x"$have_redland" = xyes)

###############################################################################
###############################################################################
###############################################################################
# Test for making adobe XMP support
###############################################################################

have_xmp=no
if test "$have_redland" = yes; then
	have_xmp=yes
fi
AM_CONDITIONAL(HAVE_XMP, test x"$have_xmp" = xyes)
AC_SUBST(XMP_LIBS)
AC_SUBST(XMP_CFLAGS)

###############################################################################
## check ssh/fsh shell name ###################################################
###############################################################################

SSHCMD="ssh"
AC_CHECK_PROG( have_fsh, fsh, yes, no )
if test x$have_fsh = xyes; then
SSHCMD="fsh"
fi

AC_ARG_WITH(sshcmd,
  [AC_HELP_STRING([--with-sshcmd=ssh],[use ssh/fsh command])],
[SSHCMD="$withval"],[SSHCMD=ssh])
AC_SUBST(SSHCMD)

###############################################################################
###############################################################################
###############################################################################

dnl Checks for libraries.
AC_CHECK_LIB(fam, FAMOpen,,
  echo ""
  echo "fam 2.6.4+ is needed by ferris"
  echo ""
  echo "Its best to get fam from your Linux distribution mirror"
  echo ""
  echo "-- /usr/lib/fam -- "
  ls -l /usr/lib/libfam*
  echo ""
  echo "-- /usr/local/lib/fam -- "
  ls -l /usr/local/lib/libfam*

  AC_MSG_ERROR([Fatal Error: no libfam found.])	
)
AM_FERRIS_FAMPP2(3.5.0)


AC_CHECK_LIB("cap_FIXME_XFS_CONFLICT", cap_clear,
  [
AC_DEFINE(FERRIS_HAVE_LIBCAP, 1, [] )
LIBCAP_CFLAGS=" "
LIBCAP_LIBS=" -lcap"
AC_SUBST(LIBCAP_CFLAGS)
AC_SUBST(LIBCAP_LIBS)
], [])



AC_ARG_ENABLE(libz,
  [AS_HELP_STRING([--enable-libz],
                  [enable libz support (default=auto)])],[],[enable_libz=check])
if test x$enable_libz != xno; then

	AC_CHECK_LIB("z", zlibVersion,
	  [
		AM_CONDITIONAL(FERRIS_HAVE_LIBZ, test "yes" = yes )
		AC_DEFINE(FERRIS_HAVE_LIBZ, 1, [] )
		LIBZ_CFLAGS=" "
		LIBZ_LIBS=" -lz "
		AC_SUBST(LIBZ_CFLAGS)
		AC_SUBST(LIBZ_LIBS)
	], [
		AM_CONDITIONAL(FERRIS_HAVE_LIBZ, test x"no" = xyes)
	])
fi



AC_ARG_ENABLE(libbz2,
  [AS_HELP_STRING([--enable-libbz2],
                  [enable libbz2 support (default=auto)])],[],[enable_libbz2=check])
if test x$enable_libbz2 != xno; then

	AC_CHECK_LIB("bz2", BZ2_bzCompress,
	[
		AM_CONDITIONAL(FERRIS_HAVE_BZIP2, test x"yes" = xyes)
		AC_DEFINE(FERRIS_HAVE_BZIP2, 1, [] )
		BZIP2_CFLAGS=" "
		BZIP2_LIBS=" -lbz2 "
		AC_SUBST(BZIP2_CFLAGS)
		AC_SUBST(BZIP2_LIBS)
	], [
		AM_CONDITIONAL(FERRIS_HAVE_BZIP2, test x"no" = xyes)
	])
fi




AC_CHECK_LIB("uuid", uuid_generate,
[
	UUID_LIBS=" -luuid "
	AC_SUBST(UUID_LIBS)
], [
	AC_MSG_ERROR([Fatal Error: no libuuid (from e2fsprogs-devel) found.])	
])


###############################################################################
###############################################################################
###############################################################################
# Test for libtextcat 2.1 or better
###############################################################################

AC_LANG_C

LIBTEXTCAT_CFLAGS=
LIBTEXTCAT_LIBS=

AC_ARG_ENABLE(libtextcat,
  [AS_HELP_STRING([--enable-libtextcat],
                  [enable libtextcat support (default=auto)])],[],[enable_libtextcat=check])

if test x$enable_libtextcat != xno; then

	LIBTEXTCAT_CFLAGS=""
	LIBTEXTCAT_LIBS="-ltextcat"

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $LIBTEXTCAT_CFLAGS "
	CFLAGS_cache=$CFLAGS
	CFLAGS=" $LIBTEXTCAT_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LIBTEXTCAT_LIBS "

	AC_TRY_COMPILE([
	#include <textcat.h>
	#include <string.h>
		],
		[
	    char* buf = "The createfp program allows you to easily create your own document"
        	"fingerprints. Just feed it an example document on standard input, and"
	        "store the standard output:";
    
	    void* h = textcat_Init( "/etc/libtextcat.conf" );
	    printf( "Language: %s\n", textcat_Classify(h, buf, strlen(buf)) );
	    textcat_Done(h);
		],
       		[have_libtextcat=yes],[have_libtextcat=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	CFLAGS=$CFLAGS_cache
fi

if test x"$have_libtextcat" = xno; then
  LIBTEXTCAT_CFLAGS=" "
  LIBTEXTCAT_LIBS=" "
  echo "libtextcat 2.1+ not found or symlink from /etc/libtextcat.conf to .../langclass/conf.txt"
  echo " not found... no text language guessing for you."
else
  echo "Found libtextcat 2.1, text language guessing support being built..."
  AC_DEFINE(HAVE_LIBTEXTCAT, 1, [] )
fi;

AM_CONDITIONAL(HAVE_LIBTEXTCAT, test x"$have_libtextcat" = xyes)
AC_SUBST(LIBTEXTCAT_CFLAGS)
AC_SUBST(LIBTEXTCAT_LIBS) 

###############################################################################
###############################################################################
###############################################################################
# Test for libsvmlight
###############################################################################

package=svmlight
version=5.0.0

AC_ARG_ENABLE(svmlight,
  [AS_HELP_STRING([--enable-svmlight],
                  [enable svmlight support (default=auto)])],[],[enable_svmlight=check])
if test x$enable_svmlight != xno; then

	PKG_CHECK_MODULES(SVMLIGHT, $package >= $version, 
	[
		have_svmlight=yes
		AC_DEFINE(HAVE_SVMLIGHT,1, [] )
		echo "building agent using detected svmlight..."
	],
	[
		have_svmlight=no
		echo "no svmlight library detected (version $version or better). "
		echo "  src available at http://svmlight.joachims.org/"
	])
fi
AM_CONDITIONAL(HAVE_SVMLIGHT, test x"$have_svmlight" = xyes)
AC_SUBST(SVMLIGHT_CFLAGS)
AC_SUBST(SVMLIGHT_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for gstreamer
###############################################################################

package=gstreamer-0.10
version=0.10.13

AC_ARG_ENABLE(gstreamer,
  [AS_HELP_STRING([--enable-gstreamer],
                  [enable gstreamer support (default=auto)])],[],[enable_gstreamer=check])
if test x$enable_gstreamer != xno; then

   	PKG_CHECK_MODULES(GSTREAMER, [$package >= $version gstreamer-app-0.10], 
	[
		have_gstreamer=yes
		AC_DEFINE(HAVE_GSTREAMER,1, [] )
		echo "building gstreamer stuff..."
	],
	[
		have_gstreamer=no
		echo "no gstreamer library detected (gstreamer-0.10). "
	])
fi
AM_CONDITIONAL(HAVE_GSTREAMER, test x"$have_gstreamer" = xyes)
AC_SUBST(GSTREAMER_CFLAGS)
AC_SUBST(GSTREAMER_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for libexif (digital camera metadata and thumbnails)
###############################################################################

package=libexif
version=0.5.12

AC_ARG_ENABLE(libexif,
  [AS_HELP_STRING([--enable-libexif],
                  [enable libexif support (default=auto)])],[],[enable_libexif=check])
if test x$enable_libexif != xno; then

	PKG_CHECK_MODULES(LIBEXIF, $package >= $version, 
	[
		have_libexif=yes
		AC_DEFINE(HAVE_LIBEXIF,1, [] )
		echo "building image metadata and thumbnail support using detected libexif..."
	],
	[
		have_libexif=no
		echo "no libexif found. download at http://sourceforge.net/project/showfiles.php?group_id=12272"
	])
fi
AM_CONDITIONAL(HAVE_LIBEXIF, test x"$have_libexif" = xyes)
AC_SUBST(LIBEXIF_CFLAGS)
AC_SUBST(LIBEXIF_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for epeg
###############################################################################

AC_ARG_ENABLE(epeg,
  [AS_HELP_STRING([--enable-epeg],
                  [enable epeg support (default=auto)])],[],[enable_epeg=check])
if test x$enable_epeg != xno; then

	AC_CHECK_PROG( have_epeg, epeg-config, yes, no )
fi

if test "$have_epeg" = yes; then
	EPEG_LIBS=" `epeg-config --libs` "
	EPEG_CFLAGS=" `epeg-config --cflags` "
	AC_DEFINE(HAVE_EPEG,1, [] )
fi
AM_CONDITIONAL(HAVE_EPEG, test "$have_epeg" = yes)
AC_SUBST(EPEG_LIBS)
AC_SUBST(EPEG_CFLAGS)

###############################################################################
###############################################################################
###############################################################################
# Test for socket++
###############################################################################

AM_FERRIS_SOCKETPP( 1.12.10 )

###############################################################################
###############################################################################
###############################################################################
# Test for openssl
###############################################################################

AC_LANG_CPLUSPLUS
OPENSSL_CFLAGS=""
OPENSSL_LIBS=""

openssl_pkgcfg_modules="openssl"

AC_CHECK_PROG( have_openssl, pkg-config --exists ${openssl_pkgcfg_modules}, yes, no )
if test "$have_openssl" = yes; then
	OPENSSL_CFLAGS=" `pkg-config --cflags ${openssl_pkgcfg_modules}` "
	OPENSSL_LIBS="   `pkg-config --libs   ${openssl_pkgcfg_modules}` "
fi

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS=" $CXXFLAGS $OPENSSL_CFLAGS "
LDFLAGS_cache=$LDFLAGS
LDFLAGS=" $LDFLAGS $OPENSSL_LIBS "

AC_TRY_LINK([
	#include <openssl/ssl.h>
	],
	[
		int fixme;
	],
       	[have_openssl=yes],[have_openssl=no])
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

if test x"$have_openssl" = xno; then
  echo ""
  echo "Please install the openssl"
  echo "version 0.9.6b or better"
  AC_MSG_ERROR([Fatal Error: no openssl found.])	
fi;

AM_CONDITIONAL(HAVE_OPENSSL, test "$have_openssl" = yes)
AC_SUBST(OPENSSL_CFLAGS)
AC_SUBST(OPENSSL_LIBS) 
AC_DEFINE(HAVE_OPENSSL,1, [] )

###############################################################################
###############################################################################
###############################################################################
# See what will be used for mime type determination
###############################################################################

AC_LANG_CPLUSPLUS
MIMETYPE_ENGINE_CHOSEN=""
MIMETYPE_ENGINE_DESC="none"

CXXFLAGS_cache=$CXXFLAGS
LIBFILE_CFLAGS=""

AC_ARG_WITH(libfile-mime, 
AC_HELP_STRING( [--with-libfile-mime=DIR],[use libfile for mimetype determination (default=auto)]),
[ 
	if test x$withval != xno && test x$withval != xyes; then
		LIBFILE_CFLAGS="-I$withval/include"
	fi
	try_libfile_mime=$withval 
], 
   [ try_libfile_mime=check ] 
)
CXXFLAGS=" $LIBFILE_CFLAGS "
if test x"$try_libfile_mime" != xno; then
	AC_CHECK_LIB("file", libfile_new,
	[
		MIMETYPE_ENGINE_DESC="libfile"
		MIMETYPE_ENGINE_CHOSEN=yes
		FERRIS_HAVE_LIBFILE=yes
		AC_DEFINE(HAVE_LIBFILE,1, [] )
		LIBFILE_LIBS="-lfile" 
	] , )
fi
AC_SUBST(LIBFILE_CFLAGS)
AC_SUBST(LIBFILE_LIBS)
echo "libfile cflags:$LIBFILE_CFLAGS"
echo "libfile libs:$LIBFILE_LIBS"
CXXFLAGS=$CXXFLAGS_cache



CXXFLAGS_cache=$CXXFLAGS
GNOMEVFS_CFLAGS=""

AC_ARG_WITH(gnome-vfs-mime, 
 AC_HELP_STRING( [--with-gnome-vfs-mime], [use gnome-vfs2 for mimetype determination (default=auto)]),
		[ try_gnome_vfs_mime=$withval ], [ try_gnome_vfs_mime=check ] )
if test x"$try_gnome_vfs_mime" != xno; then
	if test x"$FERRIS_HAVE_LIBFILE" = xyes; then
		if test x"$try_gnome_vfs_mime" = xyes; then
			AC_MSG_ERROR([Asked to build gnome-vfs2 mime determination functions but mimetype engine already selected. You may only have one mimetype engine. Selected engine: $MIMETYPE_ENGINE_DESC])	
		fi
	fi
	PKG_CHECK_MODULES(GNOMEVFS, gnome-vfs-2.0 gnome-vfs-module-2.0, [ have_gvfs=yes ], [ have_gvfs=no ])
	if test "$have_gvfs" = yes; then
		echo Building gnome-vfs support functions
		MIMETYPE_ENGINE_DESC="gnome-vfs"
		MIMETYPE_ENGINE_CHOSEN=yes
		FERRIS_HAVE_GNOMEVFS=yes
		AC_DEFINE(HAVE_GNOMEVFS, 1, [] )
		GNOMEVFS_CFLAGS=" `pkg-config --cflags  gnome-vfs-2.0 gnome-vfs-module-2.0` "
		GNOMEVFS_LIBS="   `pkg-config --libs    gnome-vfs-2.0 gnome-vfs-module-2.0` "
	fi
fi
AC_SUBST(GNOMEVFS_CFLAGS)
AC_SUBST(GNOMEVFS_LIBS)


AC_ARG_WITH(kde-mime, 
 AC_HELP_STRING( [--with-kde-mime], [use KDE4 for mimetype determination (default=auto)]),
	[ try_kde_mime=$withval ], [ try_kde_mime=check ]  )

if test x"$try_kde_mime" != xno; then
	if test x"$MIMETYPE_ENGINE_CHOSEN" = xyes; then
		if test x"$try_kde_mime" = xyes; then
			AC_MSG_ERROR([Asked to build KDE4 mime determination functions but mimetype engine already selected. You may only have one mimetype engine. Selected engine: $MIMETYPE_ENGINE_DESC ])	
		fi
	fi

	echo "checking if KDE4 support stuff should be built..."
	AM_FERRIS_KDE( 4.0.0 )
	if test x"$have_kde" = xyes; then
		echo "Building kde support funtions"
		MIMETYPE_ENGINE_DESC="KDE4"
		MIMETYPE_ENGINE_CHOSEN=yes
	else
		echo "KDE not found..."
	fi
fi


AM_FERRIS_PLASMA( 4.0.0 )

AM_FERRIS_KSANE( 0.0.0 )

AM_FERRIS_QPRINTER( 0.0.0 )

AC_ARG_WITH(efsd-mime, 
 AC_HELP_STRING( [--with-efsd-mime], [use efsd for mimetype determination (default=auto)]),
	[ try_efsd_mime=$withval ], [ try_efsd_mime=check ] )
if test x"$try_efsd_mime" != xno; then
	if test x"$MIMETYPE_ENGINE_CHOSEN" = xyes; then
		if test x"$try_efsd_mime" = xyes; then
			AC_MSG_ERROR([Asked to build efsd mime determination functions but mimetype engine already selected. You may only have one mimetype engine. Selected engine: $MIMETYPE_ENGINE_DESC ])	
		fi
	fi
	AC_CHECK_PROG( have_efsd, efsd-config --cflags, yes, no )
	if test "$have_efsd" = yes; then
		echo Building efsd support functions
		MIMETYPE_ENGINE_DESC="efsd"
		MIMETYPE_ENGINE_CHOSEN=yes
		FERRIS_HAVE_EFSD=yes
		AC_DEFINE(HAVE_EFSD, 1, [] )
		EFSD_CFLAGS=" `efsd-config --cflags` "
		EFSD_LIBS="   `efsd-config --libs  ` "
	fi
fi
AC_SUBST(EFSD_CFLAGS)
AC_SUBST(EFSD_LIBS)

dnl
dnl Try to find the trusty libmagic library and default to that.
dnl

LIBMAGIC_CFLAGS=
LIBMAGIC_LIBS=

if test x"$MIMETYPE_ENGINE_CHOSEN" = x; then
	LIBMAGIC_CFLAGS="  "
	LIBMAGIC_LIBS=" -lmagic "

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $LIBMAGIC_CFLAGS"
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS="$LDFLAGS $LIBMAGIC_LIBS"

	AC_TRY_LINK([
		#include <magic.h>
	],
	[
		 magic_t cookie = magic_open( MAGIC_NONE );
		 return 0;
	 ],
	 [have_libmagic=yes], [have_libmagic=no])
 
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache

	if test x"$have_libmagic" = xyes; then
		echo Building support for libmagic MIME type detection...
		MIMETYPE_ENGINE_DESC="libmagic"
		MIMETYPE_ENGINE_CHOSEN=yes
		HAVE_LIBMAGIC=yes
		AC_DEFINE(HAVE_LIBMAGIC,1, [] )
	else
		LIBMAGIC_CFLAGS="  "
		LIBMAGIC_LIBS=" "
	fi
fi

AC_SUBST(LIBMAGIC_CFLAGS)
AC_SUBST(LIBMAGIC_LIBS)


if test x"$MIMETYPE_ENGINE_CHOSEN" = x; then
AC_MSG_ERROR([You must select which mime type engine to use from
efsd, gnome-vfs2, KDE4, libfile or have libmagic installed as default choice.
If you have passed a suitable --with arg to configure then that engine could 
not be found or used on your computer. Please see --help for exact arguments
for enabling mime engines.])
fi

echo "chosen engine for mime: $MIMETYPE_ENGINE_DESC..."
AC_LANG_C
AC_DEFINE_UNQUOTED(MIMETYPE_ENGINE_DESC, "$MIMETYPE_ENGINE_DESC", [] )

#
# because the mimetype names for some types may vary depending on
# which mime engine is in use, mimetypes that ferris needs internally
# are defined here.
#
MIMETYPE_DIRECTORY="x-directory/normal";
AC_SUBST(MIMETYPE_DIRECTORY)
AC_DEFINE_UNQUOTED(MIMETYPE_DIRECTORY, "$MIMETYPE_DIRECTORY", [] )
###############################################################################
###############################################################################
###############################################################################
###############################################################################

AM_FERRIS_LOKI( 2.2.0 )
AM_FERRIS_STREAMS( 0.4.0 )
AM_FERRIS_SIGC2( 2.0.0 )

###############################################################################
###############################################################################
###############################################################################
# Check if we should add Signal6<> templates because the current 
# sigc++ 1.2 doesn't have it
###############################################################################

AC_DEFINE(SIGCXX_HAS_LESS_THAN_FIVE_SIGNAL_ARGS_AS_MAX,1, [] )

###############################################################################
###############################################################################
###############################################################################
# Test for mysql++ (aka sqlplus)
###############################################################################

package="mysql++"
version=2.1.1

AC_ARG_ENABLE(mysqlpp,
  [AS_HELP_STRING([--enable-mysqlpp],
                  [enable mysqlpp support (default=auto)])],[],[enable_mysqlpp=check])
if test x$enable_mysqlpp != xno; then

	PKG_CHECK_MODULES(SQLPLUS, $package >= $version, 
	[
		have_sqlplus=yes
	],
	[

	SQLPLUS_CFLAGS="-I/usr/include/mysql++ -I/usr/include/mysql "
	SQLPLUS_LIBS="-lmysqlpp"

	AC_LANG_CPLUSPLUS
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $SQLPLUS_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $SQLPLUS_LIBS "

	AC_TRY_RUN([
		#include <mysql++.h>

	int main( int, char** ) {

	  if( MYSQLPP_LIB_VERSION < MYSQLPP_VERSION(2,1,1) )
	    exit(1);
	  return 0;
	}
		],
       		[have_sqlplus=yes],[have_sqlplus=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	AC_LANG_C
	])
fi

if test x"$have_sqlplus" = xyes; then
	have_sqlplus=yes
	AC_DEFINE(HAVE_SQLPLUS, 1, [] )
	AC_DEFINE(FERRIS_HAVE_SQLPLUS, 1, [] )
	echo "$package library detected and used..."
else
	have_sqlplus=no
	echo "no $package library detected (version $version or better). "
	SQLPLUS_CFLAGS=
	SQLPLUS_LIBS=
fi

AM_CONDITIONAL(HAVE_SQLPLUS, test x"$have_sqlplus" = xyes)
AC_SUBST(SQLPLUS_CFLAGS)
AC_SUBST(SQLPLUS_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for DTL ODBC wrapper
###############################################################################

TESTING_FEATURE="mount ODBC using DTL";
AM_FERRIS_DTL( 3.6.2, 
	[ echo "Adding ability to: $TESTING_FEATURE ..."; ],
	[ echo "Not adding ability to: $TESTING_FEATURE ..."; ] )


###############################################################################
###############################################################################
###############################################################################
# Test for libpqxx library
###############################################################################

AM_FERRIS_PQXX( 2.4.3, 
[ echo "building ea index module for native postgresql (libpqxx)..." ],
[ echo "NOT building ea index module for native postgresql (libpqxx)..." ])



#
# fulltext indexing with tsearch2
#

TSEARCH2_CFLAGS=""
TSEARCH2_LIBS=""

build_tsearch2=no
AC_ARG_ENABLE(postgresql-tsearch2-indexing,
  [AS_HELP_STRING([--enable-postgresql-tsearch2-indexing], [Build support for fulltext indexes with PostgreSQL/TSearch2 (default=yes, if pqxx is found)])],
[],
[
if test x$have_libpqxx = xyes; then
	enable_postgresql_tsearch2_indexing=yes
fi
])
if test x$enable_postgresql_tsearch2_indexing = xyes; then
	echo "Building support for fulltext indexes with PostgreSQL/TSearch2..."
	build_tsearch2=yes
	AC_DEFINE(BUILD_PGTSEARCH_FTXIDX,1, [] )
	TSEARCH2_CFLAGS=" $LIBPQXX_CFLAGS "
	TSEARCH2_LIBS=" $LIBPQXX_LIBS "
else
	echo "postgresql-tsearch2-indexing is off by default..."
fi
AM_CONDITIONAL(BUILD_PGTSEARCH_FTXIDX, test x$build_tsearch2 = xyes)
AC_SUBST(TSEARCH2_CFLAGS)
AC_SUBST(TSEARCH2_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for xapian
###############################################################################

have_xapian=no
AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(xapian,
  [AS_HELP_STRING([--enable-xapian],
                  [enable xapian support (default=auto)])],[],[enable_xapian=check])
if test x$enable_xapian != xno; then

	AC_PATH_GENERIC( XAPIAN, "0.8.3", [ have_xapianinit=yes ], [have_xapianinit=no] )

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $XAPIAN_CFLAGS $XAPIAN_CXXFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $STLPORT_LIBS $LDFLAGS $XAPIAN_LIBS "

	AC_TRY_LINK([
		#include <xapian.h>
		#include <algorithm>
		#include <iostream>
		#include <string>

		using namespace Xapian;
		using namespace std;
		],
		[
		    string dbname = "/tmp/foobar";
		    WritableDatabase database;
		    try {
	        	// Open the database
	        	database = Auto::open( dbname, DB_CREATE_OR_OPEN);
		    } catch (const Error &error) {
	        	cerr << "Exception: "  << error.get_msg() << endl;
		        exit(1);
		    }	
		],
	       	[have_xapian=yes],
		[have_xapian=no])
	AC_LANG_C
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

if test x$have_xapian = xyes; then
 	echo "building fulltext index module using xapian..."
	AC_DEFINE(HAVE_XAPIAN,1, [] )
else
 	echo "NOT building fulltext index module using xapian..."

	if test x$have_xapianinit = xyes; then
		echo "xapian-config was found but it doesnt appear to use STLPort..." 
		echo "please install a xapian compiled against STLPort for this option."
	fi
	XAPIAN_CFLAGS=" "
	XAPIAN_LIBS=" "
fi

AM_CONDITIONAL(HAVE_XAPIAN, test "$have_xapian" = yes)
AC_SUBST(XAPIAN_LIBS)
AC_SUBST(XAPIAN_CFLAGS)

###############################################################################
###############################################################################
###############################################################################
# Test for BOOST library
###############################################################################

TESTING_FEATURE="Boost C++ library";
AM_FERRIS_BOOST( 1.33.1, 
	[ echo "Found boost library ..."; ],
	[ AC_MSG_ERROR([ERROR: boost 1.33.1 (or 1.34.1 or maybe later) is required for building libferris]); exit; ] )

AC_ARG_ENABLE(boostipc,
  [AS_HELP_STRING([--enable-boostipc], [assume boost has IPC support and build code that uses it (default=no)])],
[],[])

have_boostipc=yes
AM_CONDITIONAL(HAVE_BOOST_IPC, test x"$have_boostipc" = xyes)


###############################################################################
###############################################################################
###############################################################################
# Test for libcurl
###############################################################################

AC_ARG_ENABLE(curl,
  [AS_HELP_STRING([--enable-curl],
                  [enable curl support (default=auto)])],[],[enable_curl=check])
if test x$enable_curl != xno; then

	AC_CHECK_PROG( have_curl, curl-config, yes, no )
	if test "$have_curl" = yes; then
		CURL_LIBS=" `curl-config --libs` -lcurl "
		CURL_CFLAGS=" `curl-config --cflags` "
	fi

fi

AM_CONDITIONAL(HAVE_CURL, test "$have_curl" = yes)
AC_SUBST(CURL_LIBS)
AC_SUBST(CURL_CFLAGS)
AC_DEFINE(CURL_LEAVES_SIG_ALARM_ON,1,[])

###############################################################################
###############################################################################
###############################################################################
# Test for imlib2
###############################################################################

AC_ARG_ENABLE(imlib2,
  [AS_HELP_STRING([--enable-imlib2],
                  [enable imlib2 support (default=auto)])],[],[enable_imlib2=check])
if test x$enable_imlib2 != xno; then

	AC_CHECK_PROG( have_imlib2, imlib2-config, yes, no )
	if test "$have_imlib2" = yes; then
		IMLIB_LIBS=" `imlib2-config --libs` "
		IMLIB_CFLAGS=" `imlib2-config --cflags` "
		AC_DEFINE(HAVE_IMLIB2,1, [] )
	fi
fi

AM_CONDITIONAL(HAVE_IMLIB2, test "$have_imlib2" = yes)
AC_SUBST(IMLIB_LIBS)
AC_SUBST(IMLIB_CFLAGS)

###############################################################################
###############################################################################
###############################################################################
# Test for berkeley db4 (www.sleepycat.com)
# We only use the STLdb4 wrapper for db4 files now.
###############################################################################

AC_LANG_CPLUSPLUS
have_db4=yes
DB4_CFLAGS=""
DB4_LIBS=""
AM_CONDITIONAL(HAVE_DB4, test x"$have_db4" = xyes)
AC_DEFINE(FERRIS_HAVE_DB4,1, [] )
dnl PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/db4/libcreationdb4_factory.la "
AC_SUBST(DB4_CFLAGS)
AC_SUBST(DB4_LIBS)

dnl DB4_CFLAGS=" -I/usr/include/db4 "
dnl DB4_LIBS=" -ldb-4.0 -ldb_cxx-4.0 "

dnl CXXFLAGS_cache=$CXXFLAGS
dnl CXXFLAGS="$CXXFLAGS $DB4_CFLAGS"
dnl LDFLAGS_cache=$LDFLAGS
dnl LDFLAGS="$LDFLAGS $DB4_LIBS"

dnl AC_TRY_LINK([
dnl 	#include <db_cxx.h>
dnl         ],
dnl 	[
dnl 	int fixme;
dnl 	],
dnl        	[have_db4=yes; FERRIS_CF=" $FERRIS_CF -DFERRIS_HAVE_DB4 "],
dnl 	[have_db4=no; AC_MSG_ERROR([Fatal Error: standard db4 C++ API still used and needed in some places. Please install standard C++ db4 API]) ])
dnl LDFLAGS=$LDFLAGS_cache
dnl CXXFLAGS=$CXXFLAGS_cache

dnl if test x"$have_db4" = xyes; then
dnl 	AC_DEFINE(FERRIS_HAVE_DB4)
dnl 	PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl 	LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/db4/libcreationdb4_factory.la "
dnl fi
dnl AM_CONDITIONAL(HAVE_DB4, test x"$have_db4" = xyes)
AC_SUBST(DB4_CFLAGS)
AC_SUBST(DB4_LIBS)

###############################################################################
###############################################################################
###############################################################################
##  test for stldb4 library ###################################################
###############################################################################

have_stldb4=no
PKG_CHECK_MODULES(STLDB4, stldb4 >= 0.4.3, 
[
	have_stldb4=yes
], 
[
  echo ""
  echo "stldb4 library version not found or too old"
  echo ""
  AC_MSG_ERROR([Fatal Error: invalid or no libstldb4 found.])	
] )

AM_CONDITIONAL(HAVE_STLDB4, test x"$have_stldb4" = xyes)
AC_SUBST(STLDB4_CFLAGS)
AC_SUBST(STLDB4_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for edb
###############################################################################

AC_ARG_ENABLE(edb,
  [AS_HELP_STRING([--enable-edb],
                  [enable edb support (default=auto)])],[],[enable_edb=check])
if test x$enable_edb != xno; then

	if test x"$have_db4" = xyes; then
		have_edb="no";
		echo "Detected db4, edb module will not be created.";
	else
		AC_CHECK_PROG( have_edb, edb-config, yes, no )
	fi
fi

if test x"$have_edb" = xyes; then
	EDB_LIBS=" `edb-config --libs` "
	EDB_CFLAGS=" `edb-config --cflags` "
	AC_DEFINE(HAVE_EDB,1, [] )
fi
AM_CONDITIONAL(HAVE_EDB, test "$have_edb" = yes)
AC_SUBST(EDB_LIBS)
AC_SUBST(EDB_CFLAGS)


###############################################################################
###############################################################################
###############################################################################
# Test for gphoto2
###############################################################################

AC_ARG_ENABLE(gphoto2,
  [AS_HELP_STRING([--enable-gphoto2],
                  [enable gphoto2 support (default=auto)])],[],[enable_gphoto2=check])
if test x$enable_gphoto2 != xno; then

	AC_PATH_GENERIC( GPHOTO2, "2.1.4", [ have_gphoto2=yes ], [have_gphoto2=no] )
fi

AM_CONDITIONAL(HAVE_GPHOTO2, test "$have_gphoto2" = yes)
AC_SUBST(GPHOTO2_LIBS)
AC_SUBST(GPHOTO2_CFLAGS)

###############################################################################
###############################################################################
###############################################################################
# Test for libextractor
###############################################################################

HAVE_LIBEXTRACTOR=no;
AC_MSG_CHECKING(whether to build the extractor plugin...)
AC_ARG_ENABLE(plugin-extractor, 
AC_HELP_STRING([--enable-plugin-extractor],[build the file keywords plugin (default=auto)]),
        [ ac_use_plugin_extractor="$enableval"; plugin_extractor_explicit=1 ],
        [ ac_use_plugin_extractor="yes" ]
        )
if test X"$ac_use_plugin_extractor" = Xyes; then
       AC_MSG_RESULT(yes)
       AC_CHECK_LIB(extractor, EXTRACTOR_getKeywords,[
       HAVE_LIBEXTRACTOR=yes
       LIBEXTRACTOR_LIBS="-lextractor"
       LIBEXTRACTOR_CFLAGS=""
       AC_DEFINE(HAVE_EXTRACTOR, 1, libextractor keyword-library found)],[
       HAVE_LIBEXTRACTOR=no
       AC_MSG_WARN([libextractor not found, support disabled])])
else
       AC_MSG_RESULT(no)
       HAVE_LIBEXTRACTOR=no
	
	if test x"$plugin_extractor_explicit" = x1; then
		if test X"$ac_use_plugin_extractor" = Xyes; then
			AC_MSG_ERROR([Fatal Error: extractor plugin can not be built.])	
		fi
	fi
fi
AM_CONDITIONAL(HAVE_LIBEXTRACTOR, test "$HAVE_LIBEXTRACTOR" = yes)
AC_SUBST(LIBEXTRACTOR_LIBS)
AC_SUBST(LIBEXTRACTOR_CFLAGS)


###############################################################################
###############################################################################
###############################################################################
# update CFLAGS and LDFLAGS for either edb or db4 inclusion
###############################################################################

if test x"$have_db4" = xyes; then
	FERRIS_CF=" $FERRIS_CF $DB4_CFLAGS "
	FERRIS_LD=" $FERRIS_LD $DB4_LIBS "
else
	FERRIS_CF=" $FERRIS_CF $EDB_CFLAGS "
	FERRIS_LD=" $FERRIS_LD $EDB_LIBS "
fi

###############################################################################
###############################################################################
###############################################################################
# Test for eet
###############################################################################

AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(eet,
  [AS_HELP_STRING([--enable-eet],
                  [enable eet support (default=auto)])],[],[enable_eet=check])
if test x$enable_eet != xno; then

	AC_CHECK_PROG( have_eet, eet-config --cflags, yes, no )
	if test x"$have_eet" = xyes; then
		echo Building eet support functions
		AC_DEFINE(HAVE_EET,1, [] )
		EET_CFLAGS=" `eet-config --cflags` "
		EET_LIBS="   `eet-config --libs  ` "
	fi

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $EET_CFLAGS"
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS="$LDFLAGS $EET_LIBS"

	AC_TRY_LINK([
		#include <Eet.h>
	        ],
		[
		int fixme;
		],
	       	[have_eet=yes; FERRIS_CF=" $FERRIS_CF -DFERRIS_HAVE_EET "],
		[have_eet=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

if test x"$have_eet" = xyes; then
	AC_DEFINE(FERRIS_HAVE_EET,1, [] )
fi
AM_CONDITIONAL(HAVE_EET, test x"$have_eet" = xyes)
AC_SUBST(EET_CFLAGS)
AC_SUBST(EET_LIBS)
echo "have eet: $have_eet..."

###############################################################################
###############################################################################
###############################################################################
# Test for gdbm
###############################################################################

AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(gdbm,
  [AS_HELP_STRING([--enable-gdbm],
                  [enable gdbm support (default=auto)])],[],[enable_gdbm=check])
if test x$enable_gdbm != xno; then

	GDBM_CFLAGS=" "
	GDBM_LIBS=" -lgdbm "
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $GDBM_CFLAGS"
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS="$LDFLAGS $GDBM_LIBS"

	AC_TRY_LINK([
		#include <gdbm.h>
	        ],
		[
			GDBM_FILE f = gdbm_open ( "/tmp/junk.gdbm", 0, GDBM_WRCREAT, 644, 0 );
		],
       		[have_gdbm=yes; FERRIS_CF=" $FERRIS_CF -DFERRIS_HAVE_GDBM "],
		[have_gdbm=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

if test x"$have_gdbm" = xyes; then
	AC_DEFINE(HAVE_GDBM,1, [] )
	AC_DEFINE(FERRIS_HAVE_GDBM,1, [] )
dnl 	PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl 	LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/gdbm/libcreationgdbm_factory.la "
fi
AM_CONDITIONAL(HAVE_GDBM, test x"$have_gdbm" = xyes)
AC_SUBST(GDBM_CFLAGS)
AC_SUBST(GDBM_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for tdb
###############################################################################

AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(tdb,
  [AS_HELP_STRING([--enable-tdb],
                  [enable tdb support (default=auto)])],[],[enable_tdb=check])
if test x$enable_tdb != xno; then

	TDB_CFLAGS=" "
	TDB_LIBS=" -ltdb "
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $TDB_CFLAGS"
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS="$LDFLAGS $TDB_LIBS"

	AC_TRY_LINK([
		#include <sys/types.h>
		#include <tdb.h>
        	],
		[
			TDB_CONTEXT* f = tdb_open ( "/tmp/junk.tdb", 0, TDB_CLEAR_IF_FIRST, 644, 0 );
		],
       		[have_tdb=yes; FERRIS_CF=" $FERRIS_CF -DFERRIS_HAVE_TDB "],
		[have_tdb=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

if test x"$have_tdb" = xyes; then
	AC_DEFINE(HAVE_TDB,1, [] )
	AC_DEFINE(FERRIS_HAVE_TDB,1, [] )
fi
AM_CONDITIONAL(HAVE_TDB, test x"$have_tdb" = xyes)
AC_SUBST(TDB_CFLAGS)
AC_SUBST(TDB_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for ImageMagicK
###############################################################################

AC_ARG_ENABLE(imagemagick,
  [AS_HELP_STRING([--enable-imagemagick],
                  [enable imagemagick support (default=auto)])],[],[enable_imagemagick=check])
if test x$enable_imagemagick != xno; then

	echo Checking for ImageMagic C++
	MAGICK_LIBS=""
	MAGICK_CFLAGS=""
	#
	# Try to assume standard imagemagick is made with stlport
	#
	echo Checking for standard version of ImageMagic C++
	AC_CHECK_PROG( have_magick, "Magick++-config", yes, no )
	if test "$have_magick" = yes; then
		MAGICK_LIBS=" `Magick++-config --ldflags` "
		MAGICK_LIBS=" $MAGICK_LIBS `Magick++-config --libs` "
		MAGICK_CFLAGS=" `Magick++-config --cxxflags` "
		MAGICK_CFLAGS=" $MAGICK_CFLAGS `Magick++-config --cppflags` "
	fi
fi;

AM_CONDITIONAL(HAVE_MAGICK, test "$have_magick" = yes)
AC_SUBST(MAGICK_LIBS)
AC_SUBST(MAGICK_CFLAGS)
if test x"$have_magick" = xyes; then
	AC_DEFINE(HAVE_MAGICK,1, [] )
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for libpng
###############################################################################

AC_LANG_CPLUSPLUS
LIBPNG_CFLAGS="  "
LIBPNG_LIBS=" -lpng "

AC_ARG_WITH(libpng,
AC_HELP_STRING( [--with-libpng=DIR],
 [use libpng install rooted at <DIR> or --without-libpng to disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBPNG_CFLAGS=""
	LIBPNG_LIBS=""
  else
	LIBPNG_CFLAGS=" -I$withval/include "
	LIBPNG_LIBS=" -L$withval/lib -lpng "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBPNG_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBPNG_LIBS"

if test x$with_libpng != xno; then

	AC_TRY_LINK([#include <png.h>],
		[int FIXME;],
	       	[have_libpng=yes],[have_libpng=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBPNG, test "$have_libpng" = yes)
AC_SUBST(LIBPNG_CFLAGS)
AC_SUBST(LIBPNG_LIBS)
if test x"$have_libpng" = xyes; then
	AC_DEFINE(HAVE_LIBPNG,1, [] )
dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/png/libferrispng_factory.la "
	echo "building png module...";
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for djvulibre
###############################################################################


AC_ARG_ENABLE(djvu,
  [AS_HELP_STRING([--enable-djvu], [Enable DjVu support via cli tools and imlib2 (default=no)])],
[],[])
if test x$enable_djvu = xyes; then

	if test "$have_imlib2" = yes; then
		echo building DjVu support...;
		have_libdjvulibre=yes;
		LIBDJVULIBRE_CFLAGS=" $IMLIB_CFLAGS ";
		LIBDJVULIBRE_LIBS=" $IMLIB_LIBS ";
	else
		echo "DjVu support requires imlib2 support... ";
	fi
fi

AM_CONDITIONAL(HAVE_LIBDJVULIBRE, test "$have_libdjvulibre" = yes)
if test x"$have_libdjvulibre" = xyes; then
	AC_DEFINE(HAVE_LIBDJVULIBRE,1, [] )
	echo "building djvu module...";
else
	LIBDJVULIBRE_CFLAGS=""
	LIBDJVULIBRE_LIBS=""
fi;
AC_SUBST(LIBDJVULIBRE_CFLAGS)
AC_SUBST(LIBDJVULIBRE_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for pdf to text
###############################################################################

have_pdftotext="no"
dnl AC_CHECK_PROG( have_pdftotext, pdftotext, yes, no )
AC_ARG_WITH(pdftotext,
AC_HELP_STRING( [--with-pdftotext],
                [use pdftotext for as-text EA (default=no)]),
[
  have_pdftotext="$withval"
])

AM_CONDITIONAL(HAVE_PDFTOTEXT, test x"$have_pdftotext" = xyes)
if test x"$have_pdftotext" = xyes; then
dnl 	PFX="$LIBASTEXT_PLUGIN_FACTORY_PREFIX"
dnl 	LIBASTEXT_PLUGIN_FACTORIES_LA="$LIBASTEXT_PLUGIN_FACTORIES_LA $PFX/pdf/libastextpdf_factory.la "
	echo "building pdf as-text EA module...";
fi;


###############################################################################
###############################################################################
###############################################################################
# Test for lynx for html to text
###############################################################################

AC_CHECK_PROG( have_lynx, lynx --version, yes, no )
AM_CONDITIONAL(HAVE_LYNX, test x"$have_lynx" = xyes)
if test x"$have_lynx" = xyes; then
dnl 	PFX="$LIBASTEXT_PLUGIN_FACTORY_PREFIX"
dnl 	LIBASTEXT_PLUGIN_FACTORIES_LA="$LIBASTEXT_PLUGIN_FACTORIES_LA $PFX/html/libastexthtml_factory.la "
	echo "building html as-text EA module...";
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for wv for MS doc to text
###############################################################################

AC_CHECK_PROG( have_wv, wvText --help, yes, no )
AM_CONDITIONAL(HAVE_WV, test x"$have_wv" = xyes)
if test x"$have_wv" = xyes; then
dnl 	PFX="$LIBASTEXT_PLUGIN_FACTORY_PREFIX"
dnl 	LIBASTEXT_PLUGIN_FACTORIES_LA="$LIBASTEXT_PLUGIN_FACTORIES_LA $PFX/html/libastexthtml_factory.la "
	echo "building MS doc as-text EA module...";
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for some apps used by astext extern module
###############################################################################

# http://ftp.gnu.org/gnu/unrtf/
AC_CHECK_PROG( have_unrtf, unrtf --help, yes, no )
AM_CONDITIONAL(HAVE_UNRTF, test x"$have_unrtf" = xyes)
if test x"$have_unrtf" = xyes; then AC_DEFINE(HAVE_UNRTF, 1, []) fi;

AC_CHECK_PROG( have_hevea, hevea --help, yes, no )
AM_CONDITIONAL(HAVE_HEVEA, test x"$have_hevea" = xyes)
if test x"$have_hevea" = xyes; then AC_DEFINE(HAVE_HEVEA,1, []) fi;

AC_CHECK_PROG( have_ps2ascii, ps2ascii --help, yes, no )
AM_CONDITIONAL(HAVE_PS2ASCII, test x"$have_ps2ascii" = xyes)
if test x"$have_ps2ascii" = xyes; then AC_DEFINE(HAVE_PS2ASCII,1, []) fi;

###############################################################################
###############################################################################
###############################################################################
# Test for bogofilter for BinaryClassifierAgent subclass
###############################################################################

AC_CHECK_PROG( have_bogofilter, bogofilter -V, yes, no )
AM_CONDITIONAL(HAVE_BOGOFILTER, test x"$have_bogofilter" = xyes)
if test x"$have_bogofilter" = xyes; then
	AC_DEFINE( HAVE_BOGOFILTER, 1, [Bogofilter for machine learning] )
	echo "building bogofilter based binary classifier agent...";
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for libjasper
###############################################################################

AC_LANG_CPLUSPLUS

AC_ARG_WITH(libjasper,
AC_HELP_STRING( [--with-libjasper=DIR],
[use libjasper install rooted at <DIR> or --without-libjasper to explicitly disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBJASPER_CFLAGS=""
	LIBJASPER_LIBS=""
  else
	LIBJASPER_CFLAGS=" -I$withval/include "
       	LIBJASPER_LIBS=" -L$withval/lib -ljasper "
  fi
])

CFLAGS_cache=$CFLAGS
CFLAGS="$CFLAGS $LIBJASPER_CFLAGS $MAGICK_CFLAGS "
CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS=$CFLAGS
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBJASPER_LIBS $MAGICK_LIBS "

if test x$with_libjasper != xno; then

	AC_TRY_RUN(
	[
	#include <stdio.h>
	#include <string.h>
	#include <time.h>
	#include <sys/types.h>
	#include <magick/api.h>

	#include <string>

	using namespace std;

	int main(int argc,char **argv)
	{
	    ExceptionInfo exception;
	    Image* image;
	    ImageInfo* image_info;
	    const MagickInfo *info;
 
	    InitializeMagick(*argv);
	    GetExceptionInfo(&exception);

	    info = GetMagickInfo("*", &exception);
	    while (info)
	    {
	        string n = info->name;

	        if( n == "JP2" )
        	    return 0;
        
	        info = info->next ;
	    }
    
	    return 1;
	}
	],
	[magick_has_jasper=yes], [magick_has_jasper=no] )

	if test x"$magick_has_jasper" = xyes; then
		echo "Your ImageMagick supports jasper, thus explicit jasper module not made";
		LIBJASPER_CFLAGS=""
		LIBJASPER_LIBS=""
		have_libjasper="no"
	else
		echo "Your ImageMagick doesn't seem to include jasper support, checking if";
		echo "  jasper module should be built";

		AC_TRY_LINK([
			#include <jasper/jasper.h>
			],
			[ jas_init(); ],
		       	[have_libjasper=yes],[have_libjasper=no])

		echo "Building jasper module: $have_libjasper ";
		if test x$have_libjasper = xno; then
			LIBJASPER_CFLAGS=""
			LIBJASPER_LIBS=""
		fi
	fi;
fi;
LDFLAGS=$LDFLAGS_cache
CFLAGS=$CFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBJASPER, test "$have_libjasper" = yes)
AC_SUBST(LIBJASPER_CFLAGS)
AC_SUBST(LIBJASPER_LIBS)
if test x$have_libjasper = xyes; then
	AC_DEFINE(HAVE_LIBJASPER,1, [] )
dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/jasper/libferrisjasper_factory.la "
fi

AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# Test for libjpeg
###############################################################################

AC_LANG_CPLUSPLUS
LIBJPEG_CFLAGS="  "
LIBJPEG_LIBS=" -ljpeg "

AC_ARG_WITH(libjpeg,
AC_HELP_STRING( [--with-libjpeg=DIR],
 [use libjpeg install rooted at <DIR> or --without-libjpeg to explicitly disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBJPEG_CFLAGS=
        LIBJPEG_LIBS=
  else
	LIBJPEG_CFLAGS=" -I$withval/include "
        LIBJPEG_LIBS=" -L$withval/lib -ljpeg "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBJPEG_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBJPEG_LIBS"

if test x$with_libjpeg != xno; then

	AC_TRY_LINK([
		#include <stdio.h>
		#include <unistd.h>
		#include <jpeglib.h>],
		[int FIXME;],
       		[have_libjpeg=yes],[have_libjpeg=no])
fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBJPEG, test "$have_libjpeg" = yes)
AC_SUBST(LIBJPEG_CFLAGS)
AC_SUBST(LIBJPEG_LIBS)
if test x"$have_libjpeg" = xyes; then
	AC_DEFINE(HAVE_LIBJPEG,1, [] )
dnl  	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/jpeg/libferrisjpeg_factory.la "
fi;

###############################################################################
###############################################################################
###############################################################################
# Test for liba52
###############################################################################

AC_LANG_CPLUSPLUS
LIBA52_CFLAGS=" -I/usr/local/include  "
LIBA52_LIBS=" -la52 "

AC_ARG_WITH(liba52,
AC_HELP_STRING( [--with-liba52=DIR],
 [use liba52 install rooted at <DIR> or --without-liba52 to disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBA52_CFLAGS=
        LIBA52_LIBS=
  else
	LIBA52_CFLAGS=" -I$withval/include "
        LIBA52_LIBS=" -L$withval/lib -la52 "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBA52_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBA52_LIBS"

if test x$with_liba52 != xno; then

	AC_TRY_LINK([
		#include <stdio.h>
		#include <unistd.h>
		#include <inttypes.h>	
		#include <a52dec/a52.h>],
		[int FIXME;],
	       	[have_liba52=yes],[have_liba52=no])

fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBA52, test "$have_liba52" = yes)
AC_SUBST(LIBA52_CFLAGS)
AC_SUBST(LIBA52_LIBS)

dnl if test x$have_liba52 = xyes; then
dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/a52/libferrisa52_factory.la "
dnl fi;


###############################################################################
###############################################################################
###############################################################################
# Test for Platinum uPnP library
###############################################################################

AC_LANG_CPLUSPLUS
LIBPLATINUM_CFLAGS=" -I/usr/local/include/Platinum "
LIBPLATINUM_LIBS=" -L/usr/local/lib64 "
#PLATINUM_STATIC_LIBS_DIR="/usr/local/lib64/"
#PLATINUM_STATIC_LINK_LINE=" libPlatinum.a libPltFrameStreamer.a libPltMediaConnect.a libPltMediaRenderer.a libPltMediaServer.a libNeptune.a "
#PLATINUM_STATIC_LINK_LINE=" -lPlatinum -lPltFrameStreamer -lPltMediaConnect -lPltMediaRenderer -lPltMediaServer -lNeptune -lpthread "
PLATINUM_STATIC_LINK_LINE=" -lPltFrameStreamer -lPltMediaConnect -lPltMediaRenderer -lPltMediaServer -lPlatinum -lNeptune -lpthread "

AC_ARG_WITH(platinum-includedir,
AC_HELP_STRING( [--with-platinum-includedir=DIR],
 [find Platinum C++ header files in indicated directory. (default=auto)]),
[
	LIBPLATINUM_CFLAGS=" -I$withval/ "
])
AC_ARG_WITH(platinum-libdir,
AC_HELP_STRING( [--with-platinum-libdir=DIR],
 [find Platinum static libraries in indicated directory. (default=auto)]),
[
#	PLATINUM_STATIC_LIBS_DIR="$withval/"
	LIBPLATINUM_LIBS=" -L$withval "
])

#for if in $PLATINUM_STATIC_LINK_LINE
#do
#	LIBPLATINUM_LIBS="$LIBPLATINUM_LIBS $PLATINUM_STATIC_LIBS_DIR/$if"
#done
#LIBPLATINUM_LIBS="$LIBPLATINUM_LIBS -lpthread "
LIBPLATINUM_LIBS="$LIBPLATINUM_LIBS $PLATINUM_STATIC_LINK_LINE "

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBPLATINUM_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBPLATINUM_LIBS"

#
# have to use libtool for check
#
save_CXX=$CXX
CXX="${SHELL-/bin/sh} `pwd`/libtool link $CXX"
AC_TRY_LINK([
#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltMediaController.h"
#include "NptMap.h"
#include "NptStack.h"
#include "PltFileMediaServer.h"
#include "PltMediaRenderer.h"
#include "PltVersion.h"
		],
		[
    PLT_UPnP upnp;
    PLT_CtrlPointReference ctrlPoint(new PLT_CtrlPoint());
    upnp.AddCtrlPoint(ctrlPoint);
    upnp.Start();
    upnp.Stop();
],
      	[have_libplatinum=yes],[have_libplatinum=no])
CXX=$save_CXX


LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBPLATINUM, test "$have_libplatinum" = yes)
AC_SUBST(LIBPLATINUM_CFLAGS)
AC_SUBST(LIBPLATINUM_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for libmpeg2
###############################################################################

AC_LANG_CPLUSPLUS
LIBMPG2_CFLAGS=" -I/usr/local/include  "
LIBMPG2_LIBS=" -lmpeg2 "

AC_ARG_WITH(libmpg2,
AC_HELP_STRING( [--with-libmpeg2=DIR],
 [use libmpeg2 install rooted at <DIR> or --without-libmpeg2 to disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBMPG2_CFLAGS=
        LIBMPG2_LIBS=
  else
	LIBMPG2_CFLAGS=" -I$withval/include "
        LIBMPG2_LIBS=" -L$withval/lib -lmpeg2 "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBMPG2_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBMPG2_LIBS"

if test x$with_libmpg2 != xno; then

	AC_TRY_LINK([
		#include <stdio.h>
		#include <unistd.h>
		#include <inttypes.h>	
		#include <mpeg2dec/video_out.h>
		#include <mpeg2dec/mpeg2.h>
		],
		[int FIXME;],
	       	[have_libmpg2=yes],[have_libmpg2=no])

fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_LIBMPG2, test "$have_libmpg2" = yes)
AC_SUBST(LIBMPG2_CFLAGS)
AC_SUBST(LIBMPG2_LIBS)

dnl if test x$have_libmpg2 = xyes; then
dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/mpeg2/libferrismpeg2_factory.la "
dnl fi;


###############################################################################
###############################################################################
###############################################################################
# Test for libmpeg3
###############################################################################

AC_ARG_ENABLE(libmpeg3-support,
  [AS_HELP_STRING([--enable-libmpeg3-support],
 [If you know what you are doing then explicitly enable this plugin (default=no)])],
[],[])
if test x$enable_libmpeg3_support = xyes; then

	AC_LANG_CPLUSPLUS
	LIBMPEG3_CFLAGS=" "
	LIBMPEG3_LIBS=" -lmpeg3 -lpthread "

	AC_ARG_WITH(libmpeg3,
	AC_HELP_STRING( [--with-libmpeg3=DIR],
	 [use libmpeg3 install rooted at <DIR> or --without-libmpeg3 to disable (default=auto)]),
	[
	  if test x$withval = xno; then
		LIBMPEG3_CFLAGS=
		LIBMPEG3_LIBS=
	  else
		LIBMPEG3_CFLAGS=" -I$withval "
		LIBMPEG3_LIBS=" -L$withval -L$withval/i686 -lmpeg3 -lpthread "
	  fi
	])

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS $LIBMPEG3_CFLAGS"
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS="$LDFLAGS $LIBMPEG3_LIBS"

	if test x$with_libmpeg3 != xno; then
	
		AC_TRY_COMPILE([#include <libmpeg3.h>],
			[int FIXME;],
		       	[have_libmpeg3=yes],[have_libmpeg3=no])

	fi

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

AM_CONDITIONAL(HAVE_LIBMPEG3, test "$have_libmpeg3" = yes)
AC_SUBST(LIBMPEG3_CFLAGS)
AC_SUBST(LIBMPEG3_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for libattr native EA handling in (xfs)
###############################################################################

AC_DEFINE(XFS_SUPER_MAGIC,0x58465342,[])

AC_LANG_CPLUSPLUS
LIBATTR_CFLAGS=" "
LIBATTR_LIBS=" -L/lib  -lattr "

AC_ARG_WITH(libattr,
AC_HELP_STRING( [--with-libattr=DIR],
 [use libattr install rooted at <DIR> or --without-libattr to disable (default=auto)]),
[
  if test x$withval = xno; then
	LIBATTR_CFLAGS=
	LIBATTR_LIBS=
  else
	LIBATTR_CFLAGS=" -I$withval/include "
	LIBATTR_LIBS=" -L$withval/lib -L/lib -lattr "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $LIBATTR_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $LIBATTR_LIBS"

if test x$with_libattr != xno; then

	AC_TRY_LINK([
		#include <asm/types.h>
		#include <sys/types.h>
		#include <sys/param.h>
		#include <sys/stat.h>
		#include <stdio.h>
		#include <fcntl.h>
		#include <stdlib.h>
		#include <unistd.h>
		#include <errno.h>
		#include <string.h>
		#include <attr/attributes.h>],
		[int FIXME;],
	       	[have_libattr=yes; FERRIS_CF=" $FERRIS_CF "],[have_libattr=no])

fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

if test x$have_libattr = xyes; then
	AC_DEFINE(FERRIS_HAVE_ATTR,1, [] )

dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/xfsnative/libxfsnative_factory.la "
else
	LIBATTR_CFLAGS=""
	LIBATTR_LIBS=""
fi

AM_CONDITIONAL(HAVE_LIBATTR, test "$have_libattr" = yes)
AC_SUBST(LIBATTR_CFLAGS)
AC_SUBST(LIBATTR_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for XFS fs native headers (xfsprogs)
###############################################################################

AC_LANG_CPLUSPLUS
XFSPROGS_CFLAGS=" -I/usr/local/include "
XFSPROGS_LIBS=" -L/usr/local/lib -lhandle -lxfs "

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $XFSPROGS_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $XFSPROGS_LIBS"

AC_TRY_LINK([#include <xfs/libxfs.h> 
	],
	[int x;],
       	[have_xfsprogs=yes],[have_xfsprogs=no])
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_XFSPROGS, test "$have_xfsprogs" = yes)
if test x$have_xfsprogs = xyes; then
	AC_DEFINE(FERRIS_HAVE_XFSPROGS,1, [] )
	AC_DEFINE(FERRIS_HAVE_XFS,1, [] )
else
	XFSPROGS_CFLAGS=""
	XFSPROGS_LIBS=""
fi
AC_SUBST(XFSPROGS_CFLAGS)
AC_SUBST(XFSPROGS_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for libid3
###############################################################################

AC_LANG_CPLUSPLUS
ID3_CFLAGS=" -I/usr/local/include "
ID3_LIBS=" -L/usr/local/lib -lid3 -lz "

AC_ARG_WITH(libid3,
AC_HELP_STRING( [--with-libid3=DIR],
 [use libid3 install rooted at <DIR> or --without-libid3 to disable (default=auto)]),
[
  if test x$withval = xno; then
	ID3_CFLAGS=
        ID3_LIBS=
  else
	ID3_CFLAGS=" -I$withval/include "
        ID3_LIBS=" -L$withval/lib -lid3 -lz "
  fi
])

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $ID3_CFLAGS"
LDFLAGS_cache=$LDFLAGS
LDFLAGS="$LDFLAGS $ID3_LIBS"

if test x$with_libid3 != xno; then

	AC_TRY_LINK([#include <id3.h> 
		#include <id3/tag.h>],
		[ID3_Tag Tag;],
       		[have_id3=yes],[have_id3=no])

fi

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

AM_CONDITIONAL(HAVE_ID3, test "$have_id3" = yes)
AC_SUBST(ID3_CFLAGS)
AC_SUBST(ID3_LIBS)

dnl if test x$have_id3 = xyes; then
dnl 	PFX="$LIBEAGEN_PLUGIN_FACTORY_PREFIX"
dnl 	LIBEAGEN_PLUGIN_FACTORIES_LA=" $LIBEAGEN_PLUGIN_FACTORIES_LA $PFX/id3/libferrisid3_factory.la "
dnl fi;


###############################################################################
###############################################################################
###############################################################################
# Test for flac
###############################################################################

FLAC_CFLAGS=""
FLAC_LIBS=""

AC_ARG_ENABLE(flac,
  [AS_HELP_STRING([--enable-flac],
                  [enable flac support (default=auto)])],[],[enable_flac=check])
if test x$enable_flac != xno; then

PKG_CHECK_MODULES(FLAC, flac++ >= 1.2.1,
	[
		have_flac=yes
		AC_DEFINE(HAVE_FLAC,1, [] )
		echo "building flac support..."
	],
	[
		have_flac=no
		echo "flac not found... "
	])
fi
AM_CONDITIONAL(HAVE_FLAC, test x"$have_flac" = xyes)
AC_SUBST(FLAC_CFLAGS)
AC_SUBST(FLAC_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for oggz
###############################################################################

OGGZ_CFLAGS=""
OGGZ_LIBS=""

AC_ARG_ENABLE(oggz,
  [AS_HELP_STRING([--enable-oggz],
                  [enable oggz support (default=auto)])],[],[enable_oggz=check])
if test x$enable_oggz != xno; then

PKG_CHECK_MODULES(OGGZ, oggz >= 0.9.7,
	[
		have_oggz=yes
		AC_DEFINE(HAVE_OGGZ,1, [] )
		echo "building oggz support..."
	],
	[
		have_oggz=no
		echo "oggz not found... "
	])
fi
AM_CONDITIONAL(HAVE_OGGZ, test x"$have_oggz" = xyes)
AC_SUBST(OGGZ_CFLAGS)
AC_SUBST(OGGZ_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for taglib
###############################################################################

TAGLIB_CFLAGS=""
TAGLIB_LIBS=""

AC_ARG_ENABLE(taglib,
  [AS_HELP_STRING([--enable-taglib],
                  [enable taglib support (default=auto)])],[],[enable_taglib=check])
if test x$enable_taglib != xno; then

PKG_CHECK_MODULES(TAGLIB, taglib >= 1.5.0,
	[
		have_taglib=yes
		AC_DEFINE(HAVE_TAGLIB,1, [] )
		echo "building taglib support..."
	],
	[
		have_taglib=no
		echo "taglib not found... "
	])
fi
AM_CONDITIONAL(HAVE_TAGLIB, test x"$have_taglib" = xyes)
AC_SUBST(TAGLIB_CFLAGS)
AC_SUBST(TAGLIB_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Now that we know which Audio ID3/Comment/Tag libraries we have
# we need to work out which we prefer
###############################################################################

if test x$have_taglib = xyes; then
	AC_DEFINE(TAGLIB_FOR_OGG, 1, [] )
	AC_DEFINE(TAGLIB_FOR_MP3, 1, [] )
	AC_DEFINE(TAGLIB_FOR_FLAC, 1, [] )
else
	if test x$have_oggz = xyes; then
		AC_DEFINE(LIBOGGZ_FOR_OGG, 1, [] )
	fi
	if test x$have_flac = xyes; then
		AC_DEFINE(LIBFLAC_FOR_FLAC, 1, [] )
	fi
	if test x$have_id3 = xyes; then
		AC_DEFINE(ID3LIB_FOR_MP3, 1, [] )
		if test x$have_oggz != xyes; then
			AC_DEFINE(ID3LIB_FOR_OGG, 1, [] )
		fi
		if test x$have_flac != xyes; then
			AC_DEFINE(ID3LIB_FOR_FLAC, 1, [] )
		fi
	fi
fi

###############################################################################
###############################################################################
###############################################################################
# Test for mediainfo
###############################################################################

MEDIAINFO_CFLAGS=""
MEDIAINFO_LIBS=""

AC_ARG_ENABLE(mediainfo,
  [AS_HELP_STRING([--enable-mediainfo],
                  [enable mediainfo support (default=auto)])],[],[enable_mediainfo=check])
if test x$enable_mediainfo != xno; then

PKG_CHECK_MODULES(MEDIAINFO, libmediainfo >= 0.7,
	[
		have_mediainfo=yes
		AC_DEFINE(HAVE_MEDIAINFO,1, [] )
		echo "building mediainfo support..."
	],
	[
		have_mediainfo=no
		echo "mediainfo not found... "
	])
fi
AM_CONDITIONAL(HAVE_MEDIAINFO, test x"$have_mediainfo" = xyes)
AC_SUBST(MEDIAINFO_CFLAGS)
AC_SUBST(MEDIAINFO_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for xpath/xquery with xqilla
###############################################################################

AM_FERRIS_XQILLA( 1.0,,[echo "not building xqilla functionality..."] )

###############################################################################
###############################################################################
###############################################################################
# Test for pccts
###############################################################################

echo "checking for pccts..."

ANTLR_H=/usr/include/pccts
PCCTS_CFLAGS=" -I/usr/include/pccts "
ANTLR=antlr
DLG=dlg

AC_ARG_WITH(pccts,
        [  --with-pccts=DIR         use pccts install rooted at <DIR>],
        [

ANTLR_H="$withval/pccts"
PCCTS_CFLAGS=" -I$withval/pccts "

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS=" $CXXFLAGS $PCCTS_CFLAGS "

AC_TRY_LINK([
	#include <pccts/config.h>
	],
	[int FIXME;],
       	[have_pccts=yes],[have_pccts=no])
])


dnl if test x"$have_pccts" = xno; then
dnl 	AC_MSG_ERROR([Fatal Error: no pccts found.])	
dnl fi

CXXFLAGS=$CXXFLAGS_cache
AM_CONDITIONAL(HAVE_PCCTS, test x"$have_pccts" = xyes)
AC_SUBST(PCCTS_CFLAGS)
AC_SUBST(ANTLR_H)
AC_SUBST(ANTLR)
AC_SUBST(DLG)

###############################################################################
###############################################################################
###############################################################################
# Test for libdvdread
###############################################################################

AC_CHECK_LIB("dvdread", DVDOpen,
  [
	AM_CONDITIONAL(HAVE_DVDREAD, test yes = yes)
	AC_DEFINE(HAVE_DVDREAD,1, [] )
	DVDREAD_CFLAGS=" "
	DVDREAD_LIBS=" -ldvdread "
	have_dvdread="yes"
], [
	AM_CONDITIONAL(HAVE_DVDREAD, test yes = no)
	DVDREAD_CFLAGS=" "
	DVDREAD_LIBS=" "
	have_dvdread="no"
])
AC_SUBST(DVDREAD_CFLAGS)
AC_SUBST(DVDREAD_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Test for sysv IPC
###############################################################################

AC_LANG_CPLUSPLUS
IPC_CONTEXT_CFLAGS=" "
IPC_CONTEXT_LIBS=" "

#CXXFLAGS_cache=$CXXFLAGS
#CXXFLAGS="$CXXFLAGS $IPC_CONTEXT_CFLAGS"
#LDFLAGS_cache=$LDFLAGS
#LDFLAGS="$LDFLAGS $IPC_CONTEXT_LIBS"

#AC_TRY_LINK([
#	#include <sys/types.h>
#        #include <sys/shm.h> ],
#	[
#	char ch;
#	int shmid = shmget( IPC_PRIVATE, 2000, IPC_CREAT int shmflg);
#	if( shmid > 0 )
#	{
#	       shmctl( shmid, IPC_RMID, 0 );
#	}
#	],
#       	[have_ipc_context=yes; FERRIS_CF=" $FERRIS_CF -DFERRIS_HAVE_IPC_CONTEXT "],
#	[have_ipc_context=no])
#LDFLAGS=$LDFLAGS_cache
#CXXFLAGS=$CXXFLAGS_cache

have_ipc_context=yes;
if test "x${on_osx}" = x1; then
   have_ipc_context=no;
fi
AC_SUBST(IPC_CONTEXT_CFLAGS)
AC_SUBST(IPC_CONTEXT_LIBS)
AM_CONDITIONAL(HAVE_IPC_CONTEXT, test "x$have_ipc_context" = "xyes")
if test x"$have_ipc_context" = xyes; then
   AC_DEFINE(FERRIS_HAVE_IPC_CONTEXT,1, [] )
fi

###############################################################################
###############################################################################
###############################################################################
###############################################################################

###############################################################################
###############################################################################
###############################################################################
# Test for openldap
###############################################################################

AC_LANG_CPLUSPLUS
LDAP_CFLAGS=""
LDAP_LIBS=""

have_ldap=no
ldap_pkgcfg_modules="libldap-2.0 ldap-activation-2.0"

AC_ARG_WITH(ldap,
	AC_HELP_STRING( [--without-ldap],
			[dont use ldap]),
        [
	],
	[

	LDAP_CFLAGS=" ";
	LDAP_LIBS=" -llber -lldap ";

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $LDAP_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $LDAP_LIBS "

	AC_TRY_LINK([
		#include <ldap.h>
		#include <stdlib.h>
		],
		[
		    LDAP* l = ldap_init( "localhost", LDAP_PORT );
		],
	       	[have_ldap=yes],[have_ldap=no])

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	])

if test x"$have_ldap" = xno; then
	LDAP_CFLAGS=""
	LDAP_LIBS=""
fi

AM_CONDITIONAL(HAVE_LDAP, test "$have_ldap" = yes)
AC_SUBST(LDAP_CFLAGS)
AC_SUBST(LDAP_LIBS) 
if test x"$have_ldap" = xyes; then
	AC_DEFINE(HAVE_LDAP,1, [] )
fi;

echo "checking for ldap... $have_ldap"


###############################################################################
###############################################################################
###############################################################################
# Test for bonobo
###############################################################################

AC_LANG_CPLUSPLUS
BONOBO_CFLAGS=""
BONOBO_LIBS=""

bonobo_pkgcfg_modules="libbonobo-2.0 bonobo-activation-2.0"

AC_ARG_WITH(bonobo,
	AC_HELP_STRING( [--without-bonobo],
			[dont use bonobo]),
        [
	],
	[
	AC_CHECK_PROG( have_bonobo, pkg-config --exists ${bonobo_pkgcfg_modules}, yes, no )
	if test "$have_bonobo" = yes; then
		BONOBO_CFLAGS=" `pkg-config --cflags ${bonobo_pkgcfg_modules}` "
		BONOBO_LIBS="   `pkg-config --libs   ${bonobo_pkgcfg_modules}` "
	fi

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $BONOBO_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $BONOBO_LIBS "

	AC_TRY_LINK([
		#include <bonobo-activation/bonobo-activation.h>
		#include <libbonobo.h>
		#include <stdlib.h>
		],
		[
		    int argc = 1;
		    char** argv = (char**)calloc( 10, sizeof(char*) );
		    argv[0] = "test";
    
		    CORBA_ORB orb = bonobo_activation_init( argc, argv );
		    if (!bonobo_init_full ( &argc, argv, orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL))
		        g_error (_("I could not initialize Bonobo"));
		],
	       	[have_bonobo=yes],[have_bonobo=no])

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache

	])


AM_CONDITIONAL(HAVE_BONOBO, test "$have_bonobo" = yes)
AC_SUBST(BONOBO_CFLAGS)
AC_SUBST(BONOBO_LIBS) 
if test x"$have_bonobo" = xyes; then
	AC_DEFINE(HAVE_BONOBO,1, [] )
fi;

echo "checking for bonobo... $have_bonobo"

###############################################################################
###############################################################################
###############################################################################
# Test for dbus
###############################################################################

AC_LANG_CPLUSPLUS
DBUS_CFLAGS=""
DBUS_LIBS=""

dbus_pkgcfg_modules="dbus-1"

AC_ARG_WITH(dbus,
	AC_HELP_STRING( [--without-dbus],
			[dont use dbus]),
        [
	],
	[
	AC_CHECK_PROG( have_dbus, pkg-config --exists ${dbus_pkgcfg_modules}, yes, no )
	if test "$have_dbus" = yes; then
		DBUS_CFLAGS=" `pkg-config --cflags ${dbus_pkgcfg_modules}` "
		DBUS_LIBS="   `pkg-config --libs   ${dbus_pkgcfg_modules}` "
	fi

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $DBUS_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $DBUS_LIBS "

	AC_TRY_LINK([
        #define DBUS_API_SUBJECT_TO_CHANGE
	#include <dbus/dbus.h>
		],
		[
	        DBusError err;
        	DBusConnection *conn;
	        dbus_error_init (&err);
        	conn = dbus_bus_get (DBUS_BUS_SESSION, &err);
		],
	       	[have_dbus=yes],[have_dbus=no])

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache

	])


AM_CONDITIONAL(HAVE_DBUS, test "$have_dbus" = yes)
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS) 
if test x"$have_dbus" = xyes; then
	AC_DEFINE(HAVE_DBUS,1, [] )
fi;

echo "checking for dbus... $have_dbus"



###############################################################################
###############################################################################
###############################################################################
# Test for strigi
###############################################################################

STRIGI_CFLAGS=""
STRIGI_LIBS=""

AC_ARG_ENABLE(strigi,
  [AS_HELP_STRING([--enable-strigi],
                  [enable strigi support (default=auto)])],[],[enable_strigi=check])
if test x$enable_strigi != xno; then

PKG_CHECK_MODULES(STRIGI, 
libstreamanalyzer >= 0.5.1 
libstreams >= 0.5.1,
	[
		have_strigi=yes
		AC_DEFINE(HAVE_STRIGI, 1, [] )
		echo "building strigi support..."
	],
	[
		have_strigi=no
		echo "strigi not found... "
	])
fi
AM_CONDITIONAL(HAVE_STRIGI, test x"$have_strigi" = xyes)
AC_SUBST(STRIGI_CFLAGS)
AC_SUBST(STRIGI_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Test for swig for making perl, python and ocaml bindings.
###############################################################################

AC_ARG_WITH(swig-perl, 
 AC_HELP_STRING( [--with-swig-perl], [try to make perl wrapper (default=no)]),
[
 try_swig_perl=$withval
])

if test x"$try_swig_perl" = xyes; then

	AC_CHECK_PROG( have_swig_perl, swig -version, yes, no )
	
	if test x"$have_swig_perl" = xyes; then
		SWIG_PERL_CFLAGS=" -I`perl -MConfig -e 'print $Config{archlib}'`/CORE "
		AC_SUBST(SWIG_PERL_CFLAGS)
		SWIG_PERL_PMDIR="`perl -MConfig -e 'print $Config{vendorarch}' `"
		AC_SUBST(SWIG_PERL_PMDIR)
		echo "building perl wrappers with swig... $SWIG_PERL_CFLAGS";
		echo "   perl module will be installed at $SWIG_PERL_PMDIR";
	fi;

fi;
AM_CONDITIONAL(HAVE_SWIG_PERL, test x"$have_swig_perl" = xyes)


########################################
########################################
########################################

AC_ARG_WITH( swig-python-version, 
		AC_HELP_STRING( [--with-swig-python-version=python2.2],
				[use python version for python wrapper]),
		[ PYTHONVERSION="$withval" try_swig_python=yes; ] )

AC_ARG_WITH( swig-python-prefix, 
		AC_HELP_STRING( [--with-swig-python-prefix=/usr],
				[use python at prefix for python wrapper (default=no)]),
		[ PYTHONROOT="$withval" try_swig_python=yes; ] )

if test x"$PYTHONROOT" = xyes; then
	PYTHONROOT=/usr
fi;
if test x"$PYTHONVERSION" = xyes; then
	PYTHONVERSION=python2.2
fi;

if test x"$try_swig_python" = xyes; then

	AC_CHECK_PROG( have_swig_python, swig -version, yes, no )
	
	if test x"$have_swig_python" = xyes; then
		SWIG_PYTHON_CFLAGS=" -I$PYTHONROOT/include/$PYTHONVERSION "
		AC_SUBST(SWIG_PYTHON_CFLAGS)
		SWIG_PYTHON_MODULEDIR="$PYTHONROOT/lib/$PYTHONVERSION/site-packages"
		AC_SUBST(SWIG_PYTHON_MODULEDIR)
		SWIG_PYTHON_LIBDIR="$PYTHONROOT/lib/$PYTHONVERSION/lib-dynload"
		AC_SUBST(SWIG_PYTHON_LIBDIR)
		echo "building python wrappers with swig...     $SWIG_PYTHON_CFLAGS";
		echo "   python module will be installed at     $SWIG_PYTHON_MODULEDIR";
		echo "   python shared lib will be installed at $SWIG_PYTHON_LIBDIR";
	fi;

fi;
AM_CONDITIONAL(HAVE_SWIG_PYTHON, test x"$have_swig_python" = xyes)

########################################
########################################
########################################

AC_ARG_WITH( swig-ocaml, 
		AC_HELP_STRING( [--with-swig-ocaml],
				[try to make OCaml wrapper (default=no)]),
		try_swig_ocaml=$withval )

if test x"$try_swig_ocaml" = xyes; then

	AC_CHECK_PROG( have_swig_ocaml, swig -version, yes, no )
	
	if test x"$have_swig_ocaml" = xyes; then
		SWIG_OCAML_CFLAGS="  "
		AC_SUBST(SWIG_OCAML_CFLAGS)
		SWIG_OCAML_HEADERDIR="/usr/local/include/ocaml"
		AC_SUBST(SWIG_OCAML_HEADERDIR)
		echo "building ocaml wrappers with swig... $SWIG_OCAML_CFLAGS";
	fi;

fi;
AM_CONDITIONAL(HAVE_SWIG_OCAML, test x"$have_swig_ocaml" = xyes)


###############################################################################
###############################################################################
# Test for sleepycat dbxml 1.1
###############################################################################

have_dbxml=no

### g++ `stlport-config --cflags`  buildDB.cpp  
# -I /usr/local/dbxml/include -I /usr/local/dbxml/db4/include
#  /usr/local/dbxml/lib/libdbxml.a  /usr/local/dbxml/db4/lib/libdb-4.1.a
#  /usr/local/dbxml/db4/lib/libdb_cxx-4.1.a 
# -lxerces-c -lpathan -l stlport_gcc

AC_ARG_WITH( dbxml-prefix, 
		AC_HELP_STRING( [--with-dbxml-prefix=/usr/local/dbxml],
				[use sleepycat dbxml 1.1 at prefix (default=no)]),
		[ DBXML_PREFIX="$withval" build_dbxml_module=yes; ] )
if test x"$DBXML_PREFIX" = xyes; then
	DBXML_PREFIX=/usr/local/dbxml
fi;

AC_ARG_WITH( dbxml-db4-prefix, 
		AC_HELP_STRING( [--with-dbxml-db4-prefix=/usr/local/dbxml/db4],
				[db4.1 for sleepycat dbxml 1.1 is at prefix]),
		[ DBXMLDB4_PREFIX="$withval" build_dbxml_module=yes; ] )
if test x"$DBXMLDB4_PREFIX" = xyes; then
	DBXMLDB4_PREFIX=/usr/local/dbxml/db4
fi;

if test x"$build_dbxml_module" = xyes; then
	dbxml_error=no
	if test x"$have_xml4c" = xno; then
		dbxml_error=yes
	fi
	if test x"$have_xqilla" = xno; then
		dbxml_error=yes
	fi
	if test x"$dbxml_error" = xyes; then
		AC_MSG_ERROR([dbxml module relies on pathan 1.2+ and xerces-c = 2.2 detection...])
	fi
	echo "building dbxml mount support..."
	AC_DEFINE(HAVE_DBXML,1, [] )
	have_dbxml=yes
	DBXML_CFLAGS=" -I$DBXML_PREFIX/include -I$DBXMLDB4_PREFIX/include $XML4C_CFLAGS $XQILLA_CFLAGS "
	DBXML_LIBS=" -ldbxml -ldb -ldb_cxx  $XML4C_LIBS $XQILLA_LIBS "

dnl 	PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl 	LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/dbxml/libcreationdbxml_factory.la "
else
	DBXML_CFLAGS=""
	DBXML_LIBS=""
fi
AM_CONDITIONAL(HAVE_DBXML, test x"$build_dbxml_module" = xyes)
AC_SUBST(DBXML_CFLAGS)
AC_SUBST(DBXML_LIBS)



###############################################################################
###############################################################################
###############################################################################
# Test for gpgme
###############################################################################

echo "Checking if gpgme support should be built..."

package=gpgme
AC_PATH_GENERIC(GPGME, "0.4.3", 
[
	have_gpgme=yes
	AC_DEFINE( FERRIS_HAVE_GPGME, 1, [Using gpgme for some crypto stuff] )
	AC_DEFINE( HAVE_GPGME, 1, [Using gpgme for some crypto stuff] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
		echo "gpgme version 0.4.3+ not found..."
		have_gpgme=no
	], 
	[$3])     
	])


dnl AC_CHECK_PROG( have_gpgme, gpgme-config, yes, no )
dnl if test x`gpgme-config --version` != "x0.4.3"; then
dnl 	echo "Build currently detects fixed version 0.4.3. May work with latter gpgme aswell"
dnl 	echo "Disabling gpgme because of non 0.4.3 detection, hack the configure file if"
dnl 	echo "your gpgme is > 0.4.3"
dnl 	have_gpgme=no
dnl fi
dnl if test "$have_gpgme" = yes; then
dnl 	GPGME_LIBS=" `gpgme-config --libs` "
dnl 	GPGME_CFLAGS=" `gpgme-config --cflags` "
dnl fi
echo "GPGME_CFLAGS $GPGME_CFLAGS "
echo "GPGME_LIBS: $GPGME_LIBS "
AM_CONDITIONAL(FERRIS_HAVE_GPGME, test "$have_gpgme" = yes)
AC_SUBST(GPGME_LIBS)
AC_SUBST(GPGME_CFLAGS)
dnl if test x$have_gpgme = xyes; then
dnl 	echo "yes"
dnl 	AC_DEFINE( FERRIS_HAVE_GPGME,1, [] )
dnl else
dnl 	echo "...not building gpgme support"
dnl fi


###############################################################################
###############################################################################
###############################################################################
# Test for libgist-miq-custom-branch
###############################################################################

AC_LANG_CPLUSPLUS
echo -n "Checking for libgist-miq-custom-branch for some benchmark optimizations..."
LIBGISTMIQ_CFLAGS="-I/usr/local/libgist/include -DUNIX -DHAVE_BVECTOR -I/usr/include "
LIBGISTMIQ_LIBS="-L/usr/local/libgist/lib -lbtree -lrtree -lnptree -lgist -lm -lstdc++ /usr/local/libgist/lib/librtree.a /usr/local/libgist/lib/libbtree.a /usr/local/libgist/lib/libnptree.a /usr/local/libgist/lib/libgist.a"

LDFLAGS=$LDFLAGS_cache
CXXFLAGS_cache=$CXXFLAGS
CFLAGS_cache=$CFLAGS
CXXFLAGS=" $CXXFLAGS $LIBGISTMIQ_CFLAGS "
LDFLAGS=" $LDFLAGS $LIBGISTMIQ_LIBS "

AC_TRY_LINK(
	[ 
	#include "EasyRDTreeClient.hh" 
	#include <iostream>
	using namespace std;
	using namespace FerrisBitMagic;
	#include "gist.h"
	#include "gist_support.h"
	],
	[
	gist_init();
	typedef bvector<> key_type;
	typedef EasyRDTreeClientBitMagic tree_type;
	key_type template_bitset;
 	tree_type cl( "junk_data1" );
 	key_type bv = template_bitset;
	],
       	[
		AC_DEFINE(HAVE_LIBGISTMIQ,1, [] )
		have_libgistmiq=yes;
		echo "yes."
	],
	[
		have_libgistmiq=no;
		LIBGISTMIQ_CFLAGS=""
		LIBGISTMIQ_LIBS=""
		echo "no."
	])

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache
AM_CONDITIONAL(HAVE_LIBGISTMIQ, test x"$have_libgistmiq" = xyes)
AC_SUBST(LIBGISTMIQ_CFLAGS)
AC_SUBST(LIBGISTMIQ_LIBS)


###############################################################################
###############################################################################
###############################################################################
###############################################################################

MP3_CFLAGS=""
MP3_LIBS=""
AC_SUBST(MP3_CFLAGS)
AC_SUBST(MP3_LIBS)

OGG_CFLAGS=""
OGG_LIBS=""
AC_SUBST(OGG_CFLAGS)
AC_SUBST(OGG_LIBS)

WAV_CFLAGS=""
WAV_LIBS=""
AC_SUBST(WAV_CFLAGS)
AC_SUBST(WAV_LIBS)


###############################################################################
###############################################################################
###############################################################################
###############################################################################

AC_LANG_C

AM_PATH_GLIB_2_0( 2.2.0, [], [], [gmodule gobject] )
AM_PATH_GTK_2_0( 2.2.0, [have_gtk2=yes], [have_gtk2=no] )

# I had a problem that on some ARM targets qemu would segv in the PATH_GTK_2_0 test
# so this is to quietly force the issue.
AC_ARG_ENABLE(gtk2pkgconfighit,
  [AS_HELP_STRING([--enable-gtk2pkgconfighit],
                  [try to hit gtk2 via pkgconfig in cases where the normal test segfaults (default=auto)])],
		  [],[])

echo "enable_gtk2pkgconfighit: $enable_gtk2pkgconfighit"

if test x$enable_gtk2pkgconfighit = xyes; then

	PKG_CHECK_MODULES(GTK, gtk+-2.0, 
	[
		echo "Found GTK+2 through pkgconfig..."
		have_gtk2=yes
		AC_SUBST(GTK_CFLAGS)
		AC_SUBST(GTK_LIBS)
	], 
	[
		echo "Still couldn't find GTK+2 even with pkg-config !..."
	] )
fi

AM_CONDITIONAL(HAVE_GTK2, test "$have_gtk2" = yes)


#Test for GIMP, and define GIMP_CFLAGS and GIMP_LIBS
#AM_PATH_GIMP(1.2.1, [have_gimp=yes], [have_gimp=no])
have_gimp=no
AM_CONDITIONAL(HAVE_GIMP, test "$have_gimp" = yes)

dnl Checks for header files.
dnl AC_CHECK_HEADERS(fam.hh famstatic.h,,AC_MSG_ERROR([ERROR: fam++ must be installed.]); exit)

dnl 
dnl Work out which xerces-c version is available and use the correct 
dnl version of the local headers for it.
dnl

AC_LANG_CPLUSPLUS
CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS
CFLAGS_cache=$CFLAGS
CXXFLAGS=" $XERCESC_CFLAGS "
CFLAGS=" $XERCESC_CFLAGS "


dnl XERCES_VERSION_DOTTED="UNDEFINED"
dnl AC_EGREP_CPP(yes,
dnl [
dnl #include <xercesc/util/XercesVersion.hpp>
dnl #if defined(XERCES_VERSION_MINOR) && (XERCES_VERSION_MINOR==8)
dnl yes
dnl #endif
dnl ],  
dnl [ XERCES_VERSION_DOTTED="2.8.0" ], 
dnl [ XERCES_VERSION_DOTTED="2.7.0" ] )

XERCES_VERSION_DOTTED="3.0.0"

AC_EGREP_CPP(yes,
[
#include <xercesc/util/XercesVersion.hpp>
#if defined(XERCES_VERSION_MINOR) && (XERCES_VERSION_MINOR==1)
yes
#endif
],  
[ XERCES_VERSION_DOTTED="3.1.0" ] )



XERCES_INTERNAL_HEADERS_PATH="${absolute_srcdir}/ThirdParty/internal-xerces-c-headers/$XERCES_VERSION_DOTTED/"
XERCES_INTERNAL_HEADERS_CFLAGS=" -I$XERCES_INTERNAL_HEADERS_PATH "
AC_SUBST(XERCES_INTERNAL_HEADERS_CFLAGS)

LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache
CFLAGS=$CFLAGS_cache


GNULIB_LA=" \$(top_builddir)/lib/libgnu.la "
AC_SUBST(GNULIB_LA)

LIBIMAGEMAGICKNAMESPACEGLUE_LA=" \$(top_builddir)/FerrisGlue/libImageMagickNamespaceGlue.la "
LIBFERRIS_LA=" \$(top_builddir)/Ferris/libferris.la \$(top_builddir)/lib/libgnu.la "
LIBFERRISUI_LA=" \$(top_builddir)/FerrisUI/libferrisui.la "
LIBFERRISXSLT_LA=" \$(top_builddir)/xsltfunctions/libferrisxslt.la "
LIBFERRISMG_LA=" \$(top_builddir)/mg/libferrismg.la "
LIBFERRISINDEXING_LA=" \$(top_builddir)/Indexing/libferrisindexing.la "
LIBFERRISQTGUI_LA=" \$(top_builddir)/FerrisQtGui/libferrisqtgui.la "
dnl dnl LIBASTEXT_PLUGIN_FACTORIES_LA was built as each test passed.
dnl PFX="$LIBASTEXT_PLUGIN_FACTORY_PREFIX"
dnl LIBASTEXT_PLUGIN_FACTORIES_LA="$LIBASTEXT_PLUGIN_FACTORIES_LA $PFX/man/libastextman_factory.la "

dnl LIBCREATION_PLUGIN_FACTORIES_LA was built as each test passed.
dnl PFX="$LIBCREATION_PLUGIN_FACTORY_PREFIX"
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/devsource/libcreationdevsource_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/fulltextindex/libcreationfulltextindex_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/eaindex/libcreationeaindex_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/markup/libcreationmarkup_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/mng/libcreationmng_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/mp3/libcreationmp3_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/mpeg2/libcreationmpeg2_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/ogg/libcreationogg_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/shellscript/libcreationshellscript_factory.la "
dnl LIBCREATION_PLUGIN_FACTORIES_LA="$LIBCREATION_PLUGIN_FACTORIES_LA $PFX/wav/libcreationwav_factory.la "

AC_SUBST(LIBIMAGEMAGICKNAMESPACEGLUE_LA)
AC_SUBST(LIBFERRIS_LA)
AC_SUBST(LIBFERRISUI_LA)
AC_SUBST(LIBFERRISQTGUI_LA)
AC_SUBST(LIBFERRISXSLT_LA)
AC_SUBST(LIBFERRISMG_LA)
AC_SUBST(LIBFERRISINDEXING_LA)
AC_SUBST(LIBASTEXT_PLUGIN_FACTORIES_LA)
AC_SUBST(LIBCREATION_PLUGIN_FACTORIES_LA)
AC_SUBST(LIBEAGEN_PLUGIN_FACTORIES_LA)



SQLPLUSMODDIR="${absolute_srcdir}/plugins/context/sqlplus/"
AC_SUBST(SQLPLUSMODDIR)
if test "$have_sqlplus" = yes; then
SHAREDSQLPLUS_LA=" \$(top_builddir)/plugins/context/sqlplus/libferrissqlplusshared.la "
else
SHAREDSQLPLUS_LA=""
fi
AC_SUBST(SHAREDSQLPLUS_LA)


DTLMODDIR="${absolute_srcdir}/plugins/context/dtl/"
AC_SUBST(DTLMODDIR)
if test "$have_dtl" = yes; then
SHAREDDTL_LA=" \$(top_builddir)/plugins/context/dtl/libferrisdtlshared.la "
else
SHAREDDTL_LA=""
fi
AC_SUBST(SHAREDDTL_LA)

PGCTXMODDIR="${absolute_srcdir}/plugins/context/postgresql/"
AC_SUBST(PGCTXMODDIR)
if test x"$have_libpqxx" = xyes; then
SHAREDPGCTX_LA=" \$(top_builddir)/plugins/context/postgresql/libferrispostgresqlshared.la "
else
SHAREDPGCTX_LA=""
fi
AC_SUBST(SHAREDPGCTX_LA)


ZMCTXMODDIR="${absolute_srcdir}/plugins/context/zoneminder/"
AC_SUBST(ZMCTXMODDIR)
SHAREDZMCTX_LA=" \$(top_builddir)/plugins/context/zoneminder/libferriszoneminder_shared.la "
AC_SUBST(SHAREDZMCTX_LA)


FERRISRESTCTXMODDIR="${absolute_srcdir}/plugins/context/ferrisrest/"
AC_SUBST(FERRISRESTCTXMODDIR)
SHAREDFERRISRESTCTX_LA=" \$(top_builddir)/plugins/context/ferrisrest/libferrisferrisrest_shared.la "
AC_SUBST(SHAREDFERRISRESTCTX_LA)


KIOCTXMODDIR="${absolute_srcdir}/plugins/context/kio/"
AC_SUBST(KIOCTXMODDIR)
SHAREDKIOCTX_LA=" \$(top_builddir)/plugins/context/kio/libferriskio_shared.la "
AC_SUBST(SHAREDKIOCTX_LA)



SHAREDXAPIANMODDIR="${absolute_srcdir}/plugins/fulltextindexers/xapian/"
AC_SUBST(SHAREDXAPIANMODDIR)
if test x"$have_xapian" = xyes; then
SHAREDXAPIANEA_LA=" \$(top_builddir)/plugins/fulltextindexers/xapian/libferrisxapianeashared.la "
else
SHAREDXAPIANEA_LA=""
fi
AC_SUBST(SHAREDXAPIANEA_LA)

dnl XMLMODDIR="${absolute_srcdir}/plugins/context/xml/"
dnl AC_SUBST(XMLMODDIR)
dnl if test "$have_xml4c" = yes; then
dnl SHAREDXML_LA=" \$(top_builddir)/plugins/context/xml/libferrisxmlshared.la "
dnl else
dnl SHAREDXML_LA=""
dnl fi
dnl AC_SUBST(SHAREDXML_LA)


LDAPMODDIR="${absolute_srcdir}/plugins/context/ldap/"
AC_SUBST(LDAPMODDIR)
if test "$have_ldap" = yes; then
SHAREDLDAP_LA=" \$(top_builddir)/plugins/context/ldap/libferrisldapshared.la "
else
SHAREDLDAP_LA=""
fi
AC_SUBST(SHAREDLDAP_LA)


COMMONDBDIR="${absolute_srcdir}/plugins/context/commondbapi/"
AC_SUBST(COMMONDBDIR)
COMMONDB_LA=" \$(top_builddir)/plugins/context/commondbapi/libcommondbapi.la "
AC_SUBST(COMMONDB_LA)

COMMONSQLDBDIR="${absolute_srcdir}/plugins/context/commonsqldbapi/"
AC_SUBST(COMMONSQLDBDIR)
COMMONSQLDBDIR_LA=" \$(top_builddir)/plugins/context/commonsqldbapi/libcommonsqldbapi.la "
AC_SUBST(COMMONSQLDBDIR_LA)

SOPRANOEAMODDIR="${absolute_srcdir}/plugins/eagenerators/soprano/"
AC_SUBST(SOPRANOEAMODDIR)
if test x"$have_soprano" = xyes; then
SHAREDSOPRANOEA_LA=" \$(top_builddir)/plugins/eagenerators/soprano/libferrissopranoeashared.la "
else
SHAREDSOPRANOEA_LA=""
fi
AC_SUBST(SHAREDSOPRANOEA_LA)


EAIDXCUSTOMFERRISDIR="${absolute_srcdir}/eaindexers_custom_plugin/"
AC_SUBST(EAIDXCUSTOMFERRISDIR)
EAIDXCUSTOMFERRIS_LA=" \$(top_builddir)/eaindexers_custom_plugin/libeaidxcustomferris.la "
AC_SUBST(EAIDXCUSTOMFERRIS_LA)
EAIDXCUSTOMFERRISDB4TREE_LA=" \$(top_builddir)/eaindexers_custom_plugin/libeaidxcustomferrisdb4tree.la "
AC_SUBST(EAIDXCUSTOMFERRISDB4TREE_LA)

FTXIDXCUSTOMFERRISDIR="${absolute_srcdir}/fulltextindexers_custom_plugin/"
AC_SUBST(FTXIDXCUSTOMFERRISDIR)
FTXIDXCUSTOMFERRIS_LA=" \$(top_builddir)/fulltextindexers_custom_plugin/libftxcustomferris.la "
AC_SUBST(FTXIDXCUSTOMFERRIS_LA)


FTXIDXLUCENEANALYZERDIR="${absolute_srcdir}/plugins/fulltextindexers/lucene/"
AC_SUBST(FTXIDXLUCENEANALYZERDIR)
FTXIDXLUCENEANALYZER_LA=" \$(top_builddir)/plugins/fulltextindexers/lucene/libftxidxluceneanalyzer.la "
AC_SUBST(FTXIDXLUCENEANALYZER_LA)


CTXGOOGLEDIR="${absolute_srcdir}/plugins/context/google/"
AC_SUBST(CTXGOOGLEDIR)
CTXGOOGLE_LA=" \$(top_builddir)/plugins/context/google/libferrisgoogle_shared.la "
AC_SUBST(CTXGOOGLE_LA)

dnl CTXBOXCOMDIR="${absolute_srcdir}/plugins/context/boxcom/"
dnl AC_SUBST(CTXBOXCOMDIR)
dnl CTXBOXCOM_LA=" \$(top_builddir)/plugins/context/boxcom/libferrisboxcom_shared.la "
dnl AC_SUBST(CTXBOXCOM_LA)


CTXFACEBOOKDIR="${absolute_srcdir}/plugins/context/facebook/"
AC_SUBST(CTXFACEBOOKDIR)
CTXFACEBOOK_LA=" \$(top_builddir)/plugins/context/facebook/libferrisfacebook_shared.la "
AC_SUBST(CTXFACEBOOK_LA)


CTXVIMEODIR="${absolute_srcdir}/plugins/context/vimeo/"
AC_SUBST(CTXVIMEODIR)
CTXVIMEO_LA=" \$(top_builddir)/plugins/context/vimeo/libferrisvimeo_shared.la "
AC_SUBST(CTXVIMEO_LA)

CTXIDENTICADIR="${absolute_srcdir}/plugins/context/identica/"
AC_SUBST(CTXIDENTICADIR)
CTXIDENTICA_LA=" \$(top_builddir)/plugins/context/identica/libferrisidentica_shared.la "
AC_SUBST(CTXIDENTICA_LA)


CTXZONEMINDERDIR="${absolute_srcdir}/plugins/context/zoneminder/"
AC_SUBST(CTXZONEMINDERDIR)
CTXZONEMINDER_LA=" \$(top_builddir)/plugins/context/zoneminder/libferriszoneminder_shared.la "
AC_SUBST(CTXZONEMINDER_LA)


CTXFERRISRESTDIR="${absolute_srcdir}/plugins/context/ferrisrest/"
AC_SUBST(CTXFERRISRESTDIR)
CTXFERRISREST_LA=" \$(top_builddir)/plugins/context/ferrisrest/libferrisferrisrest_shared.la "
AC_SUBST(CTXFERRISREST_LA)


CTXKIODIR="${absolute_srcdir}/plugins/context/kio/"
AC_SUBST(CTXKIODIR)
CTXKIO_LA=" \$(top_builddir)/plugins/context/kio/libferriskio_shared.la "
AC_SUBST(CTXKIO_LA)


CTXGSTREAMERDIR="${absolute_srcdir}/plugins/context/gstreamer/"
AC_SUBST(CTXGSTREAMERDIR)
CTXGSTREAMER_LA=" \$(top_builddir)/plugins/context/gstreamer/libferrisgstreamer_shared.la "
AC_SUBST(CTXGSTREAMER_LA)


WEBPHOTOSDIR="${absolute_srcdir}/plugins/context/webphotos/"
AC_SUBST(WEBPHOTOSDIR)
SHAREDWEBPHOTOS_LA=" \$(top_builddir)/plugins/context/webphotos/libferriswebphotos_shared.la "
AC_SUBST(SHAREDWEBPHOTOS_LA)



WIKIDIR="${absolute_srcdir}/plugins/context/wiki/"
AC_SUBST(WIKIDIR)
SHAREDWIKI_LA=" \$(top_builddir)/plugins/context/wiki/libferriswikishared.la "
AC_SUBST(SHAREDWIKI_LA)


PASTEBINDIR="${absolute_srcdir}/plugins/context/pastebin/"
AC_SUBST(PASTEBINDIR)
SHAREDPASTEBIN_LA=" \$(top_builddir)/plugins/context/pastebin/libferrispastebinshared.la "
AC_SUBST(SHAREDPASTEBIN_LA)





LIBFERRIS_PLUGIN_LIBADD=${LIBFERRIS_LA}
AC_SUBST(LIBFERRIS_PLUGIN_LIBADD)
dnl LIBFERRIS_PLUGIN_FACTORY_LIBADD=${LIBFERRIS_LA}
LIBFERRIS_PLUGIN_FACTORY_LIBADD=
AC_SUBST(LIBFERRIS_PLUGIN_FACTORY_LIBADD)
LIBFERRIS_PLUGIN_FACTORY_SMALL_LIBADD=
AC_SUBST(LIBFERRIS_PLUGIN_FACTORY_SMALL_LIBADD)



###############################################################################
###############################################################################
###############################################################################
# Use of custom malloc from ftp://gee.cs.oswego.edu/pub/misc/
###############################################################################

CUSTOMALLOC_LA=""
AC_ARG_ENABLE(custom-malloc,
  [AS_HELP_STRING([--enable-custom-malloc], [use custom malloc from ftp://gee.cs.oswego.edu/pub/misc/ (default=no)])],
[],[])
if test x$enable_custom_malloc = xyes; then
	echo "Using custom malloc...";
	use_customalloc=yes
	CUSTOMALLOC_LA=" \$(top_builddir)/customalloc/libcustomalloc.la "
else
	echo "Using glibc malloc...";
	use_customalloc=no
fi


dnl use_customalloc=yes
dnl CUSTOMALLOC_LA=""
dnl CUSTOMALLOC_LA=" \$(top_builddir)/customalloc/libcustomalloc.la "

AM_CONDITIONAL(USE_CUSTOMALLOC, test "$use_customalloc" = yes)
AC_SUBST(CUSTOMALLOC_LA)

###############################################################################
###############################################################################
###############################################################################
# package management stuff
###############################################################################

LIBRPM_CFLAGS="-I/usr/include"
LIBRPM_LIBS="-lrpm"

CXXFLAGS_cache=$CXXFLAGS
CXXFLAGS=" $CXXFLAGS $LIBRPM_CFLAGS "
LDFLAGS_cache=$LDFLAGS
LDFLAGS=" $LDFLAGS $LIBRPM_LIBS "

AC_TRY_LINK([
	#include <rpm/rpmdb.h>
	#include <rpm/rpmfi.h>
	#include <rpm/rpmcli.h>
	#include <rpm/rpmerr.h>
	],
	[
		int fixme;
	],
       	[have_librpm=yes],[have_librpm=no])
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache


if test x$have_librpm = xyes; then
	echo building librpm support...
	AC_DEFINE(HAVE_LIBRPM,1, [] )
else
	LIBRPM_CFLAGS=
	LIBRPM_LIBS=
fi

AC_SUBST(LIBRPM_CFLAGS)
AC_SUBST(LIBRPM_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_LIBRPM, test x"$have_librpm" = xyes)


###############################################################################
###############################################################################
###############################################################################
# SELinux support
###############################################################################

AC_LANG_CPLUSPLUS
LIBSELINUX_INCLUDEDIR=""
AC_ARG_WITH( selinux-includedir,
 [  --with-selinux-includedir=DIR directory containing selinux dir which contains headers],
 [LIBSELINUX_INCLUDEDIR=" -I$withval "
 ])

LIBSELINUX_CFLAGS=
LIBSELINUX_LIBS=

AC_ARG_ENABLE(selinux,
  [AS_HELP_STRING([--enable-selinux],
                  [enable selinux support (default=auto)])],[],[])

if test x$enable_selinux != xno; then

	LIBSELINUX_CFLAGS=" $LIBSELINUX_INCLUDEDIR "
	LIBSELINUX_LIBS=" -lselinux "

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $LIBSELINUX_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $LIBSELINUX_LIBS "

	AC_TRY_LINK([
	extern "C" {
	#include <selinux/selinux.h>
	#include <selinux/context.h>
	};
		],
		[
		    context_t ctx = context_new( "this_will_fail" )
		],
	       	[have_libselinux=yes],[have_libselinux=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache

fi

if test x$have_libselinux = xyes; then
	echo building libselinux support...
	AC_DEFINE(HAVE_LIBSELINUX,1, [] )
else
	LIBSELINUX_CFLAGS=
	LIBSELINUX_LIBS=
fi

AC_SUBST(LIBSELINUX_CFLAGS)
AC_SUBST(LIBSELINUX_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_LIBSELINUX, test x"$have_libselinux" = xyes)

AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# libbtparse support
###############################################################################

AC_LANG_CPLUSPLUS
LIBBTPARSE_INCLUDEDIR=""
AC_ARG_WITH(btparse-includedir,
 [  --with-btparse-includedir=DIR directory containing btparse dir which contains headers],
 [LIBBTPARSE_INCLUDEDIR=" -I$withval "
 ])

LIBBTPARSE_CFLAGS=
LIBBTPARSE_LIBS=

AC_ARG_ENABLE(btparse,
  [AS_HELP_STRING([--enable-btparse],
                  [enable btparse support (default=auto)])],[],[])

if test x$enable_btparse != xno; then

	LIBBTPARSE_CFLAGS=" $LIBBTPARSE_INCLUDEDIR "
	LIBBTPARSE_LIBS=" -lbtparse "

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $LIBBTPARSE_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $LIBBTPARSE_LIBS "

	AC_TRY_LINK([
	extern "C" {
	#include <btparse.h>
	};
		],
		[
		  bt_initialize ();
		],
       		[have_libbtparse=yes],[have_libbtparse=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

have_btparse=no
if test x$have_libbtparse = xyes; then
	echo "building libbtparse support (mount bibtex)..."
	AC_DEFINE(HAVE_LIBBTPARSE,1, [] )
	have_btparse=yes
else
	echo "libbtparse not found... if you want to mount bibtex install http://www.gerg.ca/software/btOOL/"
	LIBBTPARSE_CFLAGS=
	LIBBTPARSE_LIBS=
fi

AC_SUBST(LIBBTPARSE_CFLAGS)
AC_SUBST(LIBBTPARSE_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_LIBBTPARSE, test x"$have_libbtparse" = xyes)

AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# cgicc support
###############################################################################

AC_LANG_CPLUSPLUS

CGICC_CFLAGS=
CGICC_LIBS=

AC_ARG_ENABLE(cgicc,
  [AS_HELP_STRING([--enable-cgicc],
                  [enable cgicc support (default=auto)])],[],[])

if test x$enable_cgicc != xno; then

	CGICC_CFLAGS=" "
	CGICC_LIBS=" -lcgicc "

	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $CGICC_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $CGICC_LIBS "

	AC_TRY_LINK([
                    #include <cgicc/Cgicc.h>
		],
		[
		    cgicc::Cgicc cgi;
		],
       		[have_cgicc=yes],[have_cgicc=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

#have_cgicc=no
if test x$have_cgicc = xyes; then
	echo "building cgicc support..."
	AC_DEFINE(HAVE_CGICC,1, [] )
	have_cgicc=yes
else
	echo "cgicc not found..."
	CGICC_CFLAGS=
	CGICC_LIBS=
fi

AC_SUBST(CGICC_CFLAGS)
AC_SUBST(CGICC_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_CGICC, test x"$have_cgicc" = xyes)

AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# clucene support
###############################################################################

AC_LANG_CPLUSPLUS

CLUCENE_CFLAGS=
CLUCENE_LIBS=


AC_ARG_ENABLE(clucene,
  [AS_HELP_STRING([--enable-clucene],
                  [enable clucene support (default=auto)])],[],[])

echo "enable_clucene:$enable_clucene"
if test x$enable_clucene != xno; then

	CLUCENE_CFLAGS=" -I/usr/local/include -I/usr/lib64 -I/usr/lib "
	CLUCENE_LIBS="  -L/usr/local/lib -lclucene-core "


	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $CLUCENE_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $CLUCENE_LIBS "

	AC_TRY_LINK([
	#include <CLucene.h>
	using namespace lucene;
	using namespace lucene::index;
	using namespace lucene::analysis;
		],
		[
		    Analyzer* a = 0;
		    IndexWriter* foo = new IndexWriter( "/tmp", a, true );
		    const TCHAR* k = _T("foo");
		    const TCHAR* v = _T("bar");
                    Term* t = new Term( k, v );
		],
       		[have_clucene=yes],[have_clucene=no])
	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
fi

echo "post check... have_clucene:$have_clucene"
if test x$have_clucene = xyes; then
	echo "building clucene support..."
	AC_DEFINE(HAVE_CLUCENE,1, [] )
	have_clucene=yes
else
	echo "clucene not found..."
	CLUCENE_CFLAGS=
	CLUCENE_LIBS=
fi

AC_SUBST(CLUCENE_CFLAGS)
AC_SUBST(CLUCENE_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_CLUCENE, test x"$have_clucene" = xyes)
AM_CONDITIONAL(HAVE_CLUCENE, test x"$have_clucene" = xyes)

AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# Optional support for fulltext index access/updates using Java Lucene
###############################################################################

package=lucene
AC_LANG_CPLUSPLUS
CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS

AC_ARG_ENABLE(gcj,
  [AS_HELP_STRING([--enable-gcj],
                  [enable gcj support (default=auto)])],[],[])

AC_ARG_ENABLE(lucene,
  [AS_HELP_STRING([--enable-lucene],
                  [enable lucene support (default=auto)])],
[
	if test x$enableval = xyes; then
		enable_gcj=yes;
	fi
],
[])

if test x$enable_gcj != xno; then

	CXXFLAGS=" $CXXFLAGS $STLPORT_CFLAGS $STLPORT_IO64_CFLAGS "
	LDFLAGS=" $LDFLAGS $STLPORT_LIBS -lgcj "

	AC_TRY_LINK([
		#include <gcj/cni.h>
		#include <java/lang/System.h>
		#include <java/io/PrintStream.h>
		#include <java/lang/Throwable.h>
		],[
	    using namespace java::lang;
     
	    try
	    {
        	JvCreateJavaVM(NULL);
	        JvAttachCurrentThread(NULL, NULL);
     
	        String *message = JvNewStringLatin1("Hello from C++");
        	JvInitClass(&System::class$);
	        System::out->println(message);
     
	        JvDetachCurrentThread();
	    }
	    catch (Throwable *t)
	    {
        	System::err->println(JvNewStringLatin1("Unhandled Java exception:"));
	        t->printStackTrace();
	    }
		],
	       	[have_gcj=yes; GCJ_CFLAGS=; GCJ_LIBS=" -lgcj "],
		[have_gcj=no])
fi

AC_SUBST(GCJ_CFLAGS)
AC_SUBST(GCJ_LIBS)
AM_CONDITIONAL(HAVE_GCJ, test x"$have_gcj" = xyes)



if test x$enable_lucene != xno && test "$have_gcj" = yes; then

	AC_DEFINE( FERRIS_HAVE_GCJ, 1, [using gcj for some java code] )

	GCJLUCENE_CFLAGS=" "
	GCJLUCENE_LIBS=" -llucene.jar "
	GCJLUCENE_GCJFLAGS=" --classpath=/usr/share/java/lucene.jar "
	CXXFLAGS=" $CXXFLAGS $GCJLUCENE_CFLAGS  "
	LDFLAGS=" $LDFLAGS $GCJLUCENE_LIBS "

AC_TRY_LINK([
	#include <gcj/cni.h>
	#include <java/lang/System.h>
	#include <java/io/PrintStream.h>
	#include <java/lang/Throwable.h>

#include <org/apache/lucene/document/Document.h>
#include <org/apache/lucene/document/Field.h>
#include <org/apache/lucene/document/DateField.h>
#include <org/apache/lucene/analysis/LowerCaseTokenizer.h>
#include <org/apache/lucene/analysis/LowerCaseFilter.h>
#include <org/apache/lucene/analysis/StopAnalyzer.h>

	],[
    using namespace java::lang;
     
    try
    {
        JvCreateJavaVM(NULL);
        JvAttachCurrentThread(NULL, NULL);
     
        String *message = JvNewStringLatin1("Hello from C++");
        JvInitClass(&System::class$);
        System::out->println(message);
     
        JvDetachCurrentThread();
    }
    catch (Throwable *t)
    {
        System::err->println(JvNewStringLatin1("Unhandled Java exception:"));
        t->printStackTrace();
    }
	],
	[
		have_gcjlucene=yes
		AC_DEFINE(HAVE_GCJLUCENE,1, [] )
		echo "Building optional support for Lucene handled fulltext indexing..."
	],
	[
		GCJLUCENE_CFLAGS=
		GCJLUCENE_LIBS=
		GCJLUCENE_GCJFLAGS=
		have_gcjlucene=no
		echo ""
		echo "version ($version) or later of $package not found, "
		echo " optional support for Lucene handled fulltext and EA indexing disabled"
		echo ""
	])

 	AC_SUBST(GCJLUCENE_CFLAGS)
 	AC_SUBST(GCJLUCENE_LIBS)
	AC_SUBST(GCJLUCENE_GCJFLAGS)

fi

AM_CONDITIONAL(HAVE_GCJLUCENE, test x"$have_gcjlucene" = xyes)


LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache
AC_LANG_C

if test "$have_gcj" = yes; then
FERRISGCJ_LA=" \$(top_builddir)/FerrisGCJ/libferrisgcj.la "
else
FERRISGCJ_LA=""
fi
AC_SUBST(FERRISGCJ_LA)


###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting evolution mail / searching evolution
###############################################################################

AC_LANG_CPLUSPLUS
CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS

AC_ARG_ENABLE(evolution,
  [AS_HELP_STRING([--enable-evolution],
                  [enable evolution support (default=auto)])],[],[enable_evolution=check])

if test x$enable_evolution != xno; then

EVOLUTION_CFLAGS=" `pkg-config libebook-1.2 camel-1.2 camel-provider-1.2 evolution-data-server-1.2 --cflags ` "
EVOLUTION_LIBS="   `pkg-config libebook-1.2 camel-1.2 camel-provider-1.2 evolution-data-server-1.2 --libs `  "

CXXFLAGS=" $CXXFLAGS $STLPORT_CFLAGS $STLPORT_IO64_CFLAGS $EVOLUTION_CFLAGS "
LDFLAGS=" $LDFLAGS $STLPORT_LIBS $EVOLUTION_LIBS "

EVOLUTION_BUG__EXTRA_HEADERS_TO_PREINCLUDE=
 
AC_TRY_COMPILE([
extern "C" {
#include <libebook/e-book.h>
#include <libbonobo.h>
#include <camel/camel.h>
#include <camel/camel-object.h>
};

#include <iostream>
#include <sstream>
#include <string>

	],[
using namespace std;

		    int argc = 1;
		    char** argv = (char**)calloc( 10, sizeof(char*) );
		    argv[0] = "test";

    if (bonobo_init (&argc, argv) == FALSE)
 		g_error ("Could not initialize Bonobo");


    const string camelPath = "/tmp/EvolutionSample";
	camel_init (camelPath.c_str(), FALSE);
    camel_type_init ();
     
	],
       	[have_evolution=yes;],
	[have_evolution=no;])

if test "x$have_evolution" = xno; then
	echo "Trying to build with extra junk to force compilation..."

EVOLUTION_BUG__EXTRA_HEADERS_TO_PREINCLUDE='#include "/usr/share/gettext/gettext.h"'
AC_DEFINE(EVOLUTION_BUG__EXTRA_HEADERS_TO_PREINCLUDE,1, [] )

 
AC_TRY_COMPILE([
extern "C" {
#include "/usr/share/gettext/gettext.h"

#include <libebook/e-book.h>
#include <libbonobo.h>
#include <camel/camel.h>
#include <camel/camel-object.h>
};

#include <iostream>
#include <sstream>
#include <string>

	],[
using namespace std;

		    int argc = 1;
		    char** argv = (char**)calloc( 10, sizeof(char*) );
		    argv[0] = "test";

    if (bonobo_init (&argc, argv) == FALSE)
 		g_error ("Could not initialize Bonobo");


    const string camelPath = "/tmp/EvolutionSample";
	camel_init (camelPath.c_str(), FALSE);
    camel_type_init ();
     
	],
       	[have_evolution=yes;],
	[have_evolution=no; EVOLUTION_CFLAGS=; EVOLUTION_LIBS=;])

fi
fi

if test x$enable_evolution = xyes && test x$have_evolution=xno; then
	AC_MSG_ERROR([Fatal Error: Evolution explicitly desired but unavailable.])	
fi

AC_SUBST(EVOLUTION_CFLAGS)
AC_SUBST(EVOLUTION_LIBS)
AC_SUBST(EVOLUTION_BUG__EXTRA_HEADERS_TO_PREINCLUDE)
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache
AC_LANG_C

AM_CONDITIONAL(HAVE_EVOLUTION, test x"$have_evolution" = xyes)

if test "$have_evolution" = yes; then
	echo "Building evolution data source support..."
else
	echo "Not building support for mounting evolution..."
fi

###############################################################################
###############################################################################
###############################################################################
# Optional support for using gtk+ extras for FCA and graphical clients
###############################################################################

have_gtkextra2=no

AC_ARG_ENABLE(gtkextra2,
  [AS_HELP_STRING([--enable-gtkextra2],
                  [enable gtkextra2 support (default=auto)])],[],[enable_gtkextra2=check])

if test x$enable_gtkextra2 != xno; then

	PKG_CHECK_MODULES(GTKEXTRA2, gtkextra-2.0 >= 2.1.0, 
	[
		echo "Building optional clients which rely on gtk+ extra 2.0..."
		have_gtkextra2=yes
	], 
	[
		echo "NOT Building optional clients which rely on gtk+ extra 2.0..."
	] )
fi

AM_CONDITIONAL(HAVE_GTKEXTRA2, test x"$have_gtkextra2" = xyes)
AC_SUBST(GTKEXTRA2_CFLAGS)
AC_SUBST(GTKEXTRA2_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting X window using ecore_X
###############################################################################

have_ecore=no

AC_ARG_ENABLE(ecore,
  [AS_HELP_STRING([--enable-ecore],
                  [enable ecore support (default=auto)])],[],[enable_ecore=check])

if test x$enable_ecore != xno; then

	PKG_CHECK_MODULES(ECORE, ecore-x >= 0.9.9.013, 
	[
		echo "Building optional support for mounting X Window..."
		have_ecore=yes
		AC_DEFINE(FERRIS_HAVE_ECORE, 1, [] )
	], 
	[
		echo "NOT Building optional support for mounting X Window..."
	] )
fi

AM_CONDITIONAL(FERRIS_HAVE_ECORE, test x"$have_ecore" = xyes)
AC_SUBST(ECORE_CFLAGS)
AC_SUBST(ECORE_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Optional support for:
#    mounting X clipboard via klipper
#    using KDE4 metainfo interface for generating EA
###############################################################################

if test x"$have_tested_for_kde" = x; then
	AM_FERRIS_KDE( 4.0.0 )
fi
echo "(verbose) have found KDE4...$have_kde"

dnl
dnl Here we work out if the top level xwin:// should be built. It must have
dnl either ecore for windows or dcop for klipper or both.
dnl
build_xwin_context=no;
if test x"$have_ecore" = xyes; then
	build_xwin_context=yes;
fi
if test x"$have_kde" = xyes; then
	build_xwin_context=yes;
fi
AM_CONDITIONAL(FERRIS_BUILD_XWIN_CONTEXT, test x"$build_xwin_context" = xyes)


#
###
#

build_kde_eagenerator=no;
AC_MSG_CHECKING(whether to compile KDE4 metadata EA Generator)
AC_ARG_ENABLE(kde-extractor,
  [AC_HELP_STRING([--enable-kde-extractor],[build the KDE4 EA extractor plugin. default=yes if kde3 is found, no otherwise])],
[],
[
if test x"$have_kde" = xyes; then
	enable_kde_extractor=yes
fi
])
if test x$enable_kde_extractor = xyes; then
	build_kde_eagenerator=yes;
	KDE_EAGENERATOR_CFLAGS="$KDE_CFLAGS -I/usr/include/kde/kio "
	KDE_EAGENERATOR_LIBS="$KDE_LIBS -lkio"
	echo "Building KDE4 metadata extraction module..."
else
	echo "Not building KDE4 metadata extraction module..."
fi

AM_CONDITIONAL(FERRIS_BUILD_KDE_EAGENERATOR, test x"$build_kde_eagenerator" = xyes)
AC_SUBST(KDE_EAGENERATOR_CFLAGS)
AC_SUBST(KDE_EAGENERATOR_LIBS)


############
# moc-qt4
############

AC_PATH_PROGS( MOC, moc-qt4 moc )
echo "moc program is at: $MOC"
AC_SUBST(MOC)

LIBFERRIS_PLATFORM_SPECIFICS

############
# QtSql
############

package=QtSql
version=4.4.3

AC_ARG_ENABLE(qtsql,
  [AS_HELP_STRING([--enable-qtsql],
                  [enable qtsql support (default=auto)])],[],[enable_qtsql=check])
if test x$enable_qtsql != xno; then

	PKG_CHECK_MODULES(QTSQL, [QtCore >= $version $package >= $version], 
	[
		have_qtsql=yes
		AC_DEFINE(HAVE_QTSQL, 1, [] )
		echo "building QtSql support..."
	],
	[
		have_qtsql=no
		echo "NOT building QtSql support..."
	])
fi
AM_CONDITIONAL(HAVE_QTSQL, test x"$have_qtsql" = xyes)
AC_SUBST(QTSQL_CFLAGS)
AC_SUBST(QTSQL_LIBS)


QTSQLCTXMODDIR="${absolute_srcdir}/plugins/context/qtsql/"
AC_SUBST(QTSQLCTXMODDIR)
if test x"$have_qtsql" = xyes; then
SHAREDQTSQLCTX_LA=" \$(top_builddir)/plugins/context/qtsql/libferrisqtsqlshared.la "
else
SHAREDQTSQLCTX_LA=""
fi
AC_SUBST(SHAREDQTSQLCTX_LA)


############
# QtDBus
############

package=QtDBus
version=4.4.3

AC_ARG_ENABLE(qtdbus,
  [AS_HELP_STRING([--enable-qtdbus],
                  [enable qtdbus support (default=auto)])],[],[enable_qtdbus=check])
if test x$enable_qtdbus != xno; then

	PKG_CHECK_MODULES(QTDBUS, [QtCore >= $version $package >= $version], 
	[
		have_qtdbus=yes
		AC_DEFINE(HAVE_QTDBUS, 1, [] )
		echo "building Qtdbus support..."
	],
	[
		have_qtdbus=no
		echo "NOT building Qtdbus support..."
	])
fi
AM_CONDITIONAL(HAVE_QTDBUS, test x"$have_qtdbus" = xyes)
AC_SUBST(QTDBUS_CFLAGS)
AC_SUBST(QTDBUS_LIBS)

############
# QtNetwork
############

package=QtNetwork
version=4.4.3

AC_ARG_ENABLE(qtnetwork,
  [AS_HELP_STRING([--enable-qtnetwork],
                  [enable qtnetwork support (default=auto)])],[],[enable_qtnetwork=check])
if test x$enable_qtnetwork != xno; then

	PKG_CHECK_MODULES(QTNETWORK, [QtCore >= $version $package >= $version], 
	[
		have_qtnetwork=yes
		AC_DEFINE(HAVE_QTNETWORK, 1, [] )
		echo "building Qtnetwork support..."
	],
	[
		have_qtnetwork=no
		echo "NOT building Qtnetwork support..."
	])
fi
AM_CONDITIONAL(HAVE_QTNETWORK, test x"$have_qtnetwork" = xyes)
AC_SUBST(QTNETWORK_CFLAGS)
AC_SUBST(QTNETWORK_LIBS)

############
# QtGui
############

package=QtGui
version=4.6.0

AC_ARG_ENABLE(qtgui,
  [AS_HELP_STRING([--enable-qtgui],
                  [enable qtgui support (default=auto)])],[],[enable_qtgui=check])
if test x$enable_qtgui != xno; then

	PKG_CHECK_MODULES(QTGUI, [QtCore >= $version $package >= $version], 
	[
		have_qtgui=yes
		AC_DEFINE(HAVE_QTGUI, 1, [] )
		echo "building Qtgui support..."
	],
	[
		have_qtgui=no
		echo "NOT building Qtgui support..."
	])
fi
AM_CONDITIONAL(HAVE_QTGUI, test x"$have_qtgui" = xyes)
AC_SUBST(QTGUI_CFLAGS)
AC_SUBST(QTGUI_LIBS)


############
# qoauth
############

package=qoauth
version=1.0.0

AC_ARG_ENABLE(qoauth,
  [AS_HELP_STRING([--enable-qoauth],
                  [enable qoauth support (default=auto)])],[],[enable_qoauth=check])
if test x$enable_qoauth != xno; then

	PKG_CHECK_MODULES(QOAUTH, [$package >= $version], 
	[
		have_qoauth=yes
		AC_DEFINE(HAVE_QOAUTH,1, [] )
		echo "building Qoauth support..."
	],
	[
		have_qoauth=no
		echo "NOT building Qoauth support..."
	])
fi
AM_CONDITIONAL(HAVE_QOAUTH, test x"$have_qoauth" = xyes)
AC_SUBST(QOAUTH_CFLAGS)
AC_SUBST(QOAUTH_LIBS)



############
# QJson
############

package=QJson
version=0.7.0

AC_ARG_ENABLE(qjson,
  [AS_HELP_STRING([--enable-qjson],
                  [enable qjson support (default=auto)])],[],[enable_qjson=check])
if test x$enable_qjson != xno; then

	PKG_CHECK_MODULES(QJSON, [$package >= $version], 
	[
		have_qjson=yes
		AC_DEFINE(HAVE_QJSON,1, [] )
		echo "building Qjson support..."
	],
	[
		have_qjson=no
		echo "NOT building Qjson support..."
	])
fi
AM_CONDITIONAL(HAVE_QJSON, test x"$have_qjson" = xyes)
AC_SUBST(QJSON_CFLAGS)
AC_SUBST(QJSON_LIBS)



############
# amarok
############

ferris_have_amarok=no;
AC_MSG_CHECKING(whether be able to mount amarok)
AC_ARG_ENABLE(amarok,
  [AC_HELP_STRING([--enable-amarok],[build support for mounting amarok. default=yes if kde3 is found, no otherwise])],
[
if test x"$enable_amarok" = xyes; then
	if test x"$have_kde" = xyes; then
		enable_amarok_support=yes
	else
	  	AC_MSG_ERROR([Fatal Error: amarok support needs KDE4])	
	fi
fi
],
[
if test x"$have_kde" = xyes; then
	enable_amarok_support=yes
fi
])
if test x$enable_amarok_support = xyes; then
	ferris_have_amarok=yes;
	LIBAMAROK_CFLAGS=" $KDE_CFLAGS "
	LIBAMAROK_LIBS=" $KDE_LIBS "
	echo "Building support for mounting amarok..."
else
	echo "Not building support for mounting amarok..."
fi

AM_CONDITIONAL(FERRIS_HAVE_AMAROK, test x"$ferris_have_amarok" = xyes)
AC_SUBST(LIBAMAROK_CFLAGS)
AC_SUBST(LIBAMAROK_LIBS)


############
# mounting dbus
############


AC_ARG_ENABLE(dbus-mounting,
  [AS_HELP_STRING([--enable-dbus-mounting],
                  [enable mounting dbus support (default=auto)])],[],[enable_dbusmounting=check])
if test x$enable_dbusmounting != xno; then
	if test x$have_qtdbus = xyes; then
		have_dbusmounting=yes
		AC_DEFINE(BUILD_DBUS_MOUNTING,1, [] )
		echo "building dbus mounting support..."
        else
		echo "you need qtdbus for mounting the bus..."
	fi
fi
AM_CONDITIONAL(BUILD_DBUS_MOUNTING, test x"$have_dbusmounting" = xyes)


###############################################################################
###############################################################################
###############################################################################
# Test for soprano RDF library
###############################################################################

package=soprano
	PKG_CHECK_MODULES(SOPRANO, soprano >= 2.3.0, 
	[
	have_soprano=yes
	echo "found Soprano RDF library..."
	],
	[
	  AC_MSG_ERROR([Fatal Error: invalid or no soprano RDF library found.])	
	])

SOPRANO_LIBS="$SOPRANO_LIBS -lsopranoclient"
AM_CONDITIONAL(HAVE_SOPRANO, test x"$have_soprano" = xyes)
AC_SUBST(SOPRANO_LIBS)
AC_SUBST(SOPRANO_CFLAGS)


AC_LANG_CPLUSPLUS
NEPOMUK_CFLAGS=
NEPOMUK_LIBS=
if test x"$have_kde" = xyes; then

   	NEPOMUK_CFLAGS=" $KDE_CFLAGS $SOPRANO_CFLAGS "
   	NEPOMUK_LIBS=" $KDE_LIBS $SOPRANO_LIBS -lnepomuk "

	CFLAGS_cache=$CFLAGS
	CXXFLAGS_cache=$CXXFLAGS
	LDFLAGS_cache=$LDFLAGS
	CXXFLAGS=" $NEPOMUK_CFLAGS "
	LDFLAGS=" $NEPOMUK_LIBS "
   
AC_TRY_LINK([ #include <KDE/Nepomuk/ResourceManager> ],
	[
 Nepomuk::ResourceManager::instance()->init();
 Nepomuk::ResourceManager::instance()->mainModel();
	],
       	[
		AC_DEFINE(HAVE_NEPOMUK,1, [] )
		have_nepomuk=yes;
	],
	[
		have_nepomuk=no;
	])

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	CFLAGS=$CFLAGS_cache

fi
AM_CONDITIONAL(HAVE_NEPOMUK, test x"$have_nepomuk" = xyes)
AC_SUBST(NEPOMUK_CFLAGS)
AC_SUBST(NEPOMUK_LIBS)
AC_LANG_C

###############################################################################
###############################################################################
###############################################################################
# Optional support for metadata through libxine
###############################################################################

have_xine=no

AC_ARG_ENABLE(xine,
  [AS_HELP_STRING([--enable-xine],
                  [enable xine support (default=auto)])],[],[enable_xine=check])

if test x$enable_xine != xno; then
	AM_PATH_XINE(1.1.1, [ have_xine=yes ])
fi

AM_CONDITIONAL(FERRIS_BUILD_XINE_EAGENERATOR, test x"$have_xine" = xyes)
AC_SUBST(XINE_CFLAGS)
AC_SUBST(XINE_LIBS)
XINE_EAGENERATOR_CFLAGS=$XINE_CFLAGS
XINE_EAGENERATOR_LIBS=$XINE_LIBS
AC_SUBST(XINE_EAGENERATOR_CFLAGS)
AC_SUBST(XINE_EAGENERATOR_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting annodex streaming media files
###############################################################################

have_cmml=no
have_annodex=no

AC_ARG_ENABLE(annodex,
  [AS_HELP_STRING([--enable-annodex],
                  [enable annodex support (default=auto)])],[],[enable_annodex=check])

if test x$enable_annodex != xno; then

	PKG_CHECK_MODULES(CMML, cmml >= 0.9.0, 
	[
		echo "Building optional support for mounting Cmml media..."
		have_cmml=yes
		AC_DEFINE(FERRIS_HAVE_CMML,1, [] )
	], 
	[
		echo "NOT Building optional support for mounting Cmml media..."
	] )


	if test "x${have_cmml}" = 'xyes'; then

		PKG_CHECK_MODULES(ANNODEX, annodex >= 0.7.2, 
		[
			echo "Building optional support for mounting Annodex media..."
			have_annodex=yes
			AC_DEFINE(FERRIS_HAVE_ANNODEX,1, [] )
		], 
		[
			echo "NOT Building optional support for mounting Annodex media..."
		] )
	fi;
fi

AM_CONDITIONAL(FERRIS_HAVE_CMML, test x"$have_cmml" = xyes)
AC_SUBST(CMML_CFLAGS)
AC_SUBST(CMML_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_ANNODEX, test x"$have_annodex" = xyes)
AC_SUBST(ANNODEX_CFLAGS)
AC_SUBST(ANNODEX_LIBS)


###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting xemacs
###############################################################################

have_xemacs=yes
AM_CONDITIONAL(FERRIS_HAVE_EMACS_MOUNT_ABILITY, test x"$have_xemacs" = xyes)
EMACS_MOUNT_CFLAGS=
EMACS_MOUNT_LIBS=
EMACS_GNUCLIENT_PATH=gnuclient
AC_SUBST(EMACS_MOUNT_CFLAGS)
AC_SUBST(EMACS_MOUNT_LIBS)
AC_SUBST(EMACS_GNUCLIENT_PATH)


###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting firefox
###############################################################################

have_firefox=yes
AM_CONDITIONAL(FERRIS_HAVE_FIREFOX_MOUNT_ABILITY, test x"$have_firefox" = xyes)
FIREFOX_MOUNT_CFLAGS=
FIREFOX_MOUNT_LIBS=
AC_SUBST(FIREFOX_MOUNT_CFLAGS)
AC_SUBST(FIREFOX_MOUNT_LIBS)

###############################################################################
###############################################################################
###############################################################################
# Optional support for mounting libobby (shared editing).
###############################################################################

have_obby=yes
package=obby-0.4-disabled
if test x"$ac_use_sigcxx_2" = xyes; then
	package=obby-0.4
else
	echo "To use libobby you must compile ferris and its dependancies"
	echo " using libsigc++-2.0...  USE configure --with-sigcxx-2x=yes" 
fi
version=0.4.0

AC_ARG_ENABLE(obby,
  [AS_HELP_STRING([--enable-obby],
                  [enable obby support (default=auto)])],[],[enable_obby=check])

if test x$enable_obby != xno; then

	PKG_CHECK_MODULES(OBBY, $package >= $version, 
	[
		have_obby=yes
		AC_DEFINE(HAVE_OBBY,1, [] )
		echo "building support for mounting libobby collaborative editing..."
	],
	[
		have_obby=no
		echo "not building support for mounting libobby collaborative editing..."
		echo "get libobby and libnet6 from here if you like http://gobby.0x539.de/"
	])
fi

AM_CONDITIONAL(HAVE_OBBY, test x"$have_obby" = xyes)
AC_SUBST(OBBY_CFLAGS)
AC_SUBST(OBBY_LIBS)
OBBY_MOUNT_CFLAGS=" $OBBY_CFLAGS "
OBBY_MOUNT_LIBS=" $OBBY_LIBS "
AC_SUBST(OBBY_MOUNT_CFLAGS)
AC_SUBST(OBBY_MOUNT_LIBS)
AM_CONDITIONAL(FERRIS_HAVE_OBBY_MOUNT_ABILITY, test x"$have_obby" = xyes)

OBBYMODDIR="${absolute_srcdir}/plugins/context/obby/"
AC_SUBST(OBBYMODDIR)
if test "$have_obby" = yes; then
SHAREDOBBY_LA=" \$(top_builddir)/plugins/context/obby/libferrisobbyshared.la "
else
SHAREDOBBY_LA=""
fi
AC_SUBST(SHAREDOBBY_LA)


###############################################################################
###############################################################################
###############################################################################
# Optional support for beagle.
###############################################################################

have_beagle=no
package=libbeagle-0.0
version=0.2.6

AC_ARG_ENABLE(beagle,
  [AS_HELP_STRING([--enable-beagle],
                  [enable beagle support (default=auto)])],[],[enable_beagle=check])

if test x$enable_beagle != xno; then

	PKG_CHECK_MODULES(BEAGLE, $package >= $version, 
	[
		have_beagle=yes
		AC_DEFINE(HAVE_BEAGLE,1, [] )
		echo "building support for searching with beagle..."
	],
	[
		have_beagle=no
		echo "not building support for searching with beagle..."
	])
fi

AM_CONDITIONAL(HAVE_BEAGLE, test x"$have_beagle" = xyes)
AC_SUBST(BEAGLE_CFLAGS)
AC_SUBST(BEAGLE_LIBS)
AM_CONDITIONAL(BUILD_BEAGLE_FTXIDX, test x"$have_beagle" = xyes)

###############################################################################
###############################################################################
###############################################################################
# Optional support for pulseaudio
###############################################################################

have_pulseaudio=no
package=libpulse
version=0.9.21

AC_ARG_ENABLE(pulseaudio,
  [AS_HELP_STRING([--enable-pulseaudio],
                  [enable pulseaudio support (default=auto)])],[],[enable_pulseaudio=check])

if test x$enable_pulseaudio != xno; then

	PKG_CHECK_MODULES(PULSEAUDIO, libpulse >= 0.9.21 libpulse-simple, 
	[
		have_pulseaudio=yes
		AC_DEFINE(HAVE_PULSEAUDIO,1, [] )
		echo "building support for pulseaudio..."
	],
	[
		have_pulseaudio=no
		echo "not building support for pulseaudio..."
	])
fi

AM_CONDITIONAL(HAVE_PULSEAUDIO, test x"$have_pulseaudio" = xyes)
AC_SUBST(PULSEAUDIO_CFLAGS)
AC_SUBST(PULSEAUDIO_LIBS)

###############################################################################
###############################################################################
###############################################################################

AM_FERRIS_XMMS( 1.2.10 )

AM_FERRIS_SQLITE3( 3.3 )

AC_ARG_ENABLE(fspot,
  [AS_HELP_STRING([--enable-fspot],
                  [enable fspot support (default=auto)])],[],[enable_fspot=check])

have_fspot=no
if test x$enable_fspot != xno; then

	if test x"$have_sqlite3" = xyes; then
		have_fspot=yes
		AC_DEFINE( HAVE_FSPOT, 1, [] )
	fi
fi
AM_CONDITIONAL(HAVE_FSPOT, test x"$have_fspot" = xyes)
FSPOT_CFLAGS=$SQLITE3_CFLAGS
AC_SUBST(FSPOT_CFLAGS)
FSPOT_LIBS="$FSPOT_LIBS $SQLITE3_LIBS"
AC_SUBST(FSPOT_LIBS)

###############################################################################
###############################################################################
###############################################################################
# For custom schema support we need to have some values defined in an enum in
# source code and also as an attribute in an xml file.
# for descriptions of these values see the schema.xml file or SchemaSupport.hh
#
# NOTE: values should never be changed or reused
###############################################################################

XSD_UNKNOWN=290
XSD_SCHEMA=291
XSD_BASIC_BOOL=300
XSD_BASIC_FLOAT=301
XSD_BASIC_DOUBLE=302
XSD_BASIC_STRING=303
XSD_BASIC_INT=304
FXD_MODE_T=305
FXD_UNIXEPOCH_T=306
FXD_INODE_T=307
FXD_MIMETYPE=308
FXD_GID_T=309
FXD_UID_T=310
FXD_USERNAME=311
FXD_GROUPNAME=312
FXD_PID=313
FXD_FILESIZE=314
FXD_URL=315
FXD_URL_IMPLICIT_RESOLVE=316
FXD_MACHINE_NAME=317
FXD_FSID_T=318
FXD_INT32=319
FXD_UINT32=320
FXD_LONG=321
FXD_BINARY=322
FXD_BINARY_RGBA32=323
FXD_BINARY_NATIVE_EA=324
FXD_BINARY_PGMPIPE=325
FXD_BINARY_A52PIPE=326
FXD_EANAMES=327
FXD_DIGEST=328
FXD_XMLDOCSTRING=329
FXD_PIXELCOUNT=330
FXD_WIDTH_PIXELS=331
FXD_HEIGHT_PIXELS=332
FXD_PRIMARY_KEY=333
FXD_PRIMARY_KEY_REAL=334
FXD_PRIMARY_KEY_VIRTUAL=335
FXD_FTX_RANK=336
FXD_FFILTER=337
FXD_STR_IP4ADDR=338
FXD_IP4PORT=339
FXD_DISTINGUISHED_PERSON=340
FXD_CIPHER_NAME=341
FXD_CIPHER_VERSION=342
FXD_CIPHER_NAME_LIST=343
FXD_CIPHER_BITS=344
FXD_STRINGLIST=345
XSD_BASIC_INTEGER=346
FXD_UNIXEPOCH_STRING=347
FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM=350
FXD_URLLIST=351
FXD_INTLIST=352
FXD_EXIF_VERSION=360
FXD_EXIF_USER_COMMENT=361
FXD_EXIF_COPYRIGHT=362
FXD_EXIF_FLASH=363
FXD_EXIF_EXPOSURE_PROGRAM=364
FXD_UNIT_NAME_LENGTH=365
FXD_EXIF_DATETIME_STRING=366
FXD_ENUMERATED_STRING=367
FXD_EXIF_SENSING_METHOD=368
FXD_EXIF_ORIENTATION=369
FXD_EXIF_METERING_MODE=370
FXD_EXIF_YCBCR_POSITIONING=371
FXD_EXIF_COMPRESSION=372
FXD_EXIF_LIGHT_SOURCE=373
FXD_MODE_STRING_T=374
FXD_LATLONG=375
FXD_LATITUDE=376
FXD_LONGITUDE=377
FXD_XLIST=400
FXD_UINT64=401
FXD_INT64=402

AC_SUBST(XSD_UNKNOWN)
AC_SUBST(XSD_SCHEMA)
AC_SUBST(XSD_BASIC_BOOL)
AC_SUBST(XSD_BASIC_FLOAT)
AC_SUBST(XSD_BASIC_DOUBLE)
AC_SUBST(XSD_BASIC_STRING)
AC_SUBST(XSD_BASIC_INT)
AC_SUBST(FXD_MODE_T)
AC_SUBST(FXD_UNIXEPOCH_T)
AC_SUBST(FXD_INODE_T)
AC_SUBST(FXD_MIMETYPE)
AC_SUBST(FXD_GID_T)
AC_SUBST(FXD_UID_T)
AC_SUBST(FXD_USERNAME)
AC_SUBST(FXD_GROUPNAME)
AC_SUBST(FXD_PID)
AC_SUBST(FXD_FILESIZE)
AC_SUBST(FXD_URL)
AC_SUBST(FXD_URL_IMPLICIT_RESOLVE)
AC_SUBST(FXD_MACHINE_NAME)
AC_SUBST(FXD_FSID_T)
AC_SUBST(FXD_INT32)
AC_SUBST(FXD_UINT32)
AC_SUBST(FXD_LONG)
AC_SUBST(FXD_BINARY)
AC_SUBST(FXD_BINARY_RGBA32)
AC_SUBST(FXD_BINARY_NATIVE_EA)
AC_SUBST(FXD_BINARY_PGMPIPE)
AC_SUBST(FXD_BINARY_A52PIPE)
AC_SUBST(FXD_EANAMES)
AC_SUBST(FXD_DIGEST)
AC_SUBST(FXD_XMLDOCSTRING)
AC_SUBST(FXD_PIXELCOUNT)
AC_SUBST(FXD_WIDTH_PIXELS)
AC_SUBST(FXD_HEIGHT_PIXELS)
AC_SUBST(FXD_PRIMARY_KEY)
AC_SUBST(FXD_PRIMARY_KEY_REAL)
AC_SUBST(FXD_PRIMARY_KEY_VIRTUAL)
AC_SUBST(FXD_FTX_RANK)
AC_SUBST(FXD_FFILTER)
AC_SUBST(FXD_STR_IP4ADDR)
AC_SUBST(FXD_IP4PORT)
AC_SUBST(FXD_DISTINGUISHED_PERSON)
AC_SUBST(FXD_CIPHER_NAME)
AC_SUBST(FXD_CIPHER_VERSION)
AC_SUBST(FXD_CIPHER_NAME_LIST)
AC_SUBST(FXD_CIPHER_BITS)
AC_SUBST(FXD_STRINGLIST)
AC_SUBST(XSD_BASIC_INTEGER)
AC_SUBST(FXD_UNIXEPOCH_STRING)
AC_SUBST(FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM)
AC_SUBST(FXD_URLLIST)
AC_SUBST(FXD_INTLIST)
AC_SUBST(FXD_EXIF_VERSION)
AC_SUBST(FXD_EXIF_USER_COMMENT)
AC_SUBST(FXD_EXIF_COPYRIGHT)
AC_SUBST(FXD_EXIF_FLASH)
AC_SUBST(FXD_EXIF_EXPOSURE_PROGRAM)
AC_SUBST(FXD_UNIT_NAME_LENGTH)
AC_SUBST(FXD_EXIF_DATETIME_STRING)
AC_SUBST(FXD_ENUMERATED_STRING)
AC_SUBST(FXD_EXIF_SENSING_METHOD)
AC_SUBST(FXD_EXIF_ORIENTATION)
AC_SUBST(FXD_EXIF_METERING_MODE)
AC_SUBST(FXD_EXIF_YCBCR_POSITIONING)
AC_SUBST(FXD_EXIF_COMPRESSION)
AC_SUBST(FXD_EXIF_LIGHT_SOURCE)
AC_SUBST(FXD_MODE_STRING_T)
AC_SUBST(FXD_LATLONG)
AC_SUBST(FXD_LATITUDE)
AC_SUBST(FXD_LONGITUDE)
AC_SUBST(FXD_XLIST)
AC_SUBST(FXD_UINT64)
AC_SUBST(FXD_INT64)

###############################################################################

AC_SUBST(GIMP_THREAD_FLAGS)
AC_SUBST(GIMP_THREAD_LIBS)
AC_SUBST(GIMP_MP_FLAGS)
AC_SUBST(GIMP_MP_LIBS)


if test "x${prefix}" = 'xNONE'; then
        prefix=${ac_default_prefix};
fi;
if test "x${exec_prefix}" = 'xNONE'; then
        exec_prefix=${ac_default_prefix};
fi;

is64=$(echo `uname -a` | grep x86_64 | wc -l );
if test "x${libdir}" = 'xNONE'; then
        libdir=${prefix}/lib;
	if test "x${is64}" = x1; then
        	libdir=${prefix}/lib64;
	fi
fi;

PREFIX=${prefix};  
SYSCONFDIR=`eval echo ${sysconfdir}`
EXECPREFIX=`eval echo ${exec_prefix}`
DOTFILE_ETC=`eval echo /etc`  # many places, but keep this one.
BINDIR=`eval echo ${bindir}`
LIBDIR=`eval echo ${libdir}`  # ./Ferris/Makefile.am
INCLUDEDIR=`eval echo ${includedir}`
PREFIXSTR=`eval echo ${prefix}` # used in code like ./Ferris/Ferris.cpp
AC_DEFINE_UNQUOTED(PREFIX, std::string("$PREFIXSTR"), [] )
AC_DEFINE_UNQUOTED(AUTOTOOLS_CONFIG_PREFIX, std::string("$PREFIXSTR"), [] )
AC_SUBST(PREFIX)
AC_SUBST(PREFIXSTR)
AC_SUBST(SYSCONFDIR)
AC_SUBST(DOTFILE_ETC)
dnl AC_SUBST(EXECPREFIX)
dnl AC_SUBST(BINDIR)
AC_SUBST(LIBDIR)
AC_SUBST(INCLUDEDIR)
DATADIR=`eval echo ${datarootdir}`
DATADIRSTR=`eval echo ${datarootdir}` # used in code like ./Ferris/Ferris.cpp
AC_DEFINE_UNQUOTED(AUTOTOOLS_CONFIG_DATADIR, std::string("$DATADIRSTR"), [] )
AC_DEFINE_UNQUOTED(AUTOTOOLS_CONFIG_DATADIRTEST, std::string("$DATADIR"), [] )
LIBDIRSTR=`eval echo ${libdir}`
AC_DEFINE_UNQUOTED(AUTOTOOLS_CONFIG_LIBDIR, std::string("$LIBDIRSTR"), [] )


AC_DEFINE( FERRIS_USER_NAME_NOBODY, "nobody", [] )


DESKTOP_FILE_DIR=`eval echo ${PREFIXSTR}/ferris/dot-desktop-files`
AC_ARG_WITH(dot-desktop-directory,
        [  --with-dot-desktop-directory=$prefix/ferris/dot-desktop-files],
        [DESKTOP_FILE_DIR="$withval"])
AC_SUBST(DESKTOP_FILE_DIR)

FERRIS_AS_TEXT_PLUGIN_DIR="$LIBDIR/ferris/plugins/astext"
AC_SUBST(FERRIS_AS_TEXT_PLUGIN_DIR)
AC_DEFINE_UNQUOTED(FERRIS_AS_TEXT_PLUGIN_DIR, std::string("$FERRIS_AS_TEXT_PLUGIN_DIR"), [] )

FERRIS_CREATION_PLUGIN_DIR="$LIBDIR/ferris/plugins/creation"
AC_SUBST(FERRIS_CREATION_PLUGIN_DIR)
AC_DEFINE_UNQUOTED(FERRIS_CREATION_PLUGIN_DIR, std::string("$FERRIS_CREATION_PLUGIN_DIR"), [] )



dnl SMALLOBJCXXFLAGS="-DMAX_SMALL_OBJECT_SIZE=64 -DDEFAULT_CHUNK_SIZE=4096"
# this fails to compile! -DLOKI_MAX_SMALL_OBJECT_SIZE=0
SMALLOBJCXXFLAGS="-DMAX_SMALL_OBJECT_SIZE=0 -DDEFAULT_CHUNK_SIZE=4096"
FERRIS_CF=" $FERRIS_CF -I${absolute_srcdir} -I${absolute_srcdir}/Ferris $XML4C_CFLAGS $XALAN_CFLAGS "
CFLAGS=" $KDE4_LINK_CFLAGS_PREFIX $STLPORT_CFLAGS $SIGC_CFLAGS $LOKI_CFLAGS $CFLAGS -I/usr/include/xfs $FERRIS_CF $STLDB4_CFLAGS $SMALLOBJCXXFLAGS "
CXXFLAGS=" $KDE4_LINK_CFLAGS_PREFIX $STLPORT_CFLAGS $SIGC_CFLAGS $SMALLOBJCXXFLAGS $LOKI_CFLAGS $CXXFLAGS -I/usr/include/xfs $FERRIS_CF $GLIB_CFLAGS $OPENSSL_CFLAGS $STLDB4_CFLAGS  $QT_CFLAGS $QTNETWORK_CFLAGS "
dnl LDFLAGS=" $LDFLAGS $STLPORT_LDFLAGS $SIGC_LDFLAGS $XML4C_LIBS $XALAN_LIBS $QT_LIBS $QTNETWORK_LIBS " 
LDFLAGS_APP=" $LDFLAGS $STLPORT_LDFLAGS $SIGC_LDFLAGS $XML4C_LIBS $XALAN_LIBS $QT_LIBS $QTNETWORK_LIBS " 

AC_ARG_ENABLE(quickerlinking,
  [AS_HELP_STRING([--enable-quickerlinking], [developer option: do not optimize linked objects, faster link, slower startup (default=no)])],
[],[])
if test x$enable_quickerlinking = xyes; then
	echo "faster linking enabled"
else
	LDFLAGS=" $LDFLAGS -Wl,-O1 -Wl,--hash-style=both"
fi


dnl CFLAGS=" $CFLAGS  -fPIC -DPIC -D__PIC__ "
dnl CXXFLAGS=" $CXXFLAGS  -fPIC -DPIC -D__PIC__ "

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CXXCPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LDFLAGS_APP)


if test "$have_curl" = yes; then
	if test "$have_openssl" = yes; then
		AC_DEFINE( HAVE_WEBPHOTOS_API, "1", [] )
	fi
fi


if test "$have_sqlplus" = yes; then
	modlist="$modlist sqlplus"
fi

if test "$have_obby" = yes; then
	modlist="$modlist obby"
fi

if test "$have_dtl" = yes; then
	modlist="$modlist odbc"
fi

if test "$have_btparse" = yes; then
	modlist="$modlist bibtex"
fi

if test x"$have_ecore" = xyes; then
	modlist="$modlist xwin"
fi

if test x"$have_annodex" = xyes; then
	modlist="$modlist annodex"
fi

if test "$have_libpqxx" = yes; then
	modlist="$modlist postgresql"
fi

if test "$have_curl" = yes; then
	modlist="$modlist curl"
fi

if test "$have_soprano" = yes; then
	modlist="$modlist soprano"
fi

if test x"$have_xmp" = xyes; then
	modlist="$modlist xmp"
fi

if test x"$HAVE_LIBEXTRACTOR" = xyes; then
	modlist="$modlist libextractor"
fi

if test x"$have_gphoto2" = xyes; then
	modlist="$modlist gphoto2"
fi

if test "$have_libexif" = yes; then
	modlist="$modlist exif"
fi

if test "$have_imlib2" = yes; then
	modlist="$modlist imlib2"
fi

if test "$have_edb" = yes; then
	modlist="$modlist edb"
fi

if test "$have_dvdread" = yes; then
	modlist="$modlist dvdread"
fi

if test "$have_ipc_context" = yes; then
	modlist="$modlist ipc"
fi

if test "$have_db4" = yes; then
	modlist="$modlist db4"
fi

if test "$have_eet" = yes; then
	modlist="$modlist eet"
fi

if test "$have_gdbm" = yes; then
	modlist="$modlist gdbm"
fi

if test "$have_tdb" = yes; then
	modlist="$modlist tdb"
fi

if test "$have_id3" = yes; then
	modlist="$modlist id3"
fi

if test "$have_xml4c" = yes; then
	modlist="$modlist xml"
fi

if test "$have_xqilla" = yes; then
	modlist="$modlist xpath"
fi

if test x"$have_dbxml" = xyes; then
	modlist="$modlist dbxml"
fi

if test "$have_pccts" = yes; then
	modlist="$modlist filter"
fi

if test "$have_gimp" = yes; then
	modlist="$modlist gimp"
fi

if test "$have_libjpeg" = yes; then
	modlist="$modlist libjpeg"
fi

if test "$have_libpng" = yes; then
	modlist="$modlist libpng"
fi

if test "$have_libdjvulibre" = yes; then
	modlist="$modlist libdjvulibre"
fi

if test "$have_magick" = yes; then
	modlist="$modlist ImageMagick++"
fi

if test "$have_libmpeg3" = yes; then
	modlist="$modlist mpeg3(video)"
fi

if test "$have_libattr" = yes; then
	modlist="$modlist xfs-attributes"
fi

if test "$have_liba52" = yes; then
	modlist="$modlist a52"
fi

if test "$have_libmpg2" = yes; then
	modlist="$modlist mpeg2"
fi

if test "$have_ldap" = yes; then
	modlist="$modlist ldap"
fi

if test "$have_evolution" = yes; then
	modlist="$modlist evolution"
fi

if test "$have_libplatinum" = yes; then
	modlist="$modlist upnp"
fi

if test "$have_sane" = yes; then
	modlist="$modlist sane"
fi


FERRIS_MODULE_COMPILE_LIST="\"$modlist\""
AC_SUBST(FERRIS_MODULE_COMPILE_LIST)

FERRIS_IOSIZE="\"$IOSIZE\""
AC_SUBST(FERRIS_IOSIZE)

FERRIS_IOSIZE_WARNING="\"\""
if test x"$IOSIZE" = x32; then
FERRIS_IOSIZE_WARNING="\"  WARNING, 32bit IO. Ferris will only work for files and data <= 2Gb in size\""
fi
AC_SUBST(FERRIS_IOSIZE_WARNING)


CLIENT_APP_CFLAGS="$STLPORT_CFLAGS $BOOST_CFLAGS $GLIB_CFLAGS $LOKI_CFLAGS $FSTREAM_CFLAGS $FAMPP2_CFLAGS $XML4C_CFLAGS $XALAN_CFLAGS $DB4_CFLAGS $GNOMEVFS_CFLAGS $LIBFILE_CFLAGS $EFSD_CFLAGS $QT_CFLAGS $QTNETWORK_CFLAGS $KDE_CFLAGS $OPENSSL_CFLAGS $LIBZ_CFLAGS $BZIP2_CFLAGS $STLDB4_CFLAGS $SIGC_CFLAGS -I/usr/local/include/Ferris $SOPRANO_CFLAGS $GPGME_CFLAGS  "

CLIENT_APP_LIBS="-lfam $FAMPP2_LIBS -lcrypto -lssl $BOOST_LIBS $LOKI_LIBS $FSTREAM_LIBS $XML4C_LIBS $XALAN_LIBS $STLPORT_LIBS $DB4_LIBS $GNOMEVFS_LIBS $LIBFILE_LIBS $EFSD_LIBS $QT_LIBS $QTNETWORK_LIBS $KDE_LIBS $LIBCAP_LIBS $OPENSSL_LIBS $LIBZ_LIBS $BZIP2_LIBS $STLDB4_LIBS $SIGC_LIBS $SOPRANO_LIBS -lpopt -lferris $GPGME_LIBS "
AC_SUBST(CLIENT_APP_CFLAGS)
AC_SUBST(CLIENT_APP_LIBS)
REQUIREMENTS=stldb4
AC_SUBST(REQUIREMENTS)


CLIENTUI_APP_CFLAGS="$CLIENT_APP_CFLAGS $GTK_CFLAGS  "
CLIENTUI_APP_LIBS="$CLIENT_APP_LIBS $GTK_LIBS -lferrisui "
AC_SUBST(CLIENTUI_APP_CFLAGS)
AC_SUBST(CLIENTUI_APP_LIBS)
REQUIREMENTSUI=
AC_SUBST(REQUIREMENTSUI)

CLIENTXSLT_APP_CFLAGS="$CLIENT_APP_CFLAGS   "
CLIENTXSLT_APP_LIBS="$CLIENT_APP_LIBS -lferrisxslt "
AC_SUBST(CLIENTXSLT_APP_CFLAGS)
AC_SUBST(CLIENTXSLT_APP_LIBS)
REQUIREMENTSXSLT=
AC_SUBST(REQUIREMENTSXSLT)

#
# All done.
#
AC_OUTPUT([
Makefile
lib/Makefile
ThirdParty/Makefile
ThirdParty/internal-xerces-c-headers/Makefile
ThirdParty/internal-xerces-c-headers/2.7.0/Makefile
ThirdParty/internal-xerces-c-headers/2.7.0/xerces/Makefile
ThirdParty/internal-xerces-c-headers/2.8.0/Makefile
ThirdParty/internal-xerces-c-headers/2.8.0/xerces/Makefile
ThirdParty/internal-xerces-c-headers/3.0.0/Makefile
ThirdParty/internal-xerces-c-headers/3.0.0/xerces/Makefile
ThirdParty/internal-xerces-c-headers/3.1.0/Makefile
ThirdParty/internal-xerces-c-headers/3.1.0/xerces/Makefile
ThirdParty/boost/Makefile
customalloc/Makefile
DBusGlue/Makefile
DBusGlue/com.Libferris.Volume.Manager.service
DBusGlue/com.libferris.Metadata.Broker.service
DBusGlue/com.libferris.Metadata.Worker.xine.service
DBusGlue/com.libferris.Metadata.Worker.magick.service
DBusGlue/com.libferris.Metadata.Worker.imlib2.service
DBusGlue/com.libferris.Metadata.Worker.png.service
DBusGlue/com.libferris.Metadata.Worker.jpg.service
FerrisGCJ/Makefile
mg/Makefile
Indexing/Makefile
FerrisGlue/Makefile
Ferris/Makefile
Ferris/BuildDependentMethods.hh
Ferris/FerrisSmartPointerChecker.hh
Ferris/SchemaSupport.hh
dot-ferris/schema.xml
FerrisQtGui/Makefile
FerrisUI/Makefile
fulltextindexers_custom_plugin/Makefile
eaindexers_custom_plugin/Makefile
apps/Makefile
apps/fulltextindex/Makefile
apps/importdesktop/Makefile
apps/importdesktop/dot-desktop-files/Makefile
apps/importdesktop/dot-desktop-files/Graphics/Makefile
apps/importdesktop/dot-desktop-files/Multimedia/Makefile
apps/fileclip/Makefile
apps/fileclip/fclipcopy/Makefile
apps/fileclip/fclipcut/Makefile
apps/fileclip/fcliplink/Makefile
apps/fileclip/fclippaste/Makefile
apps/fileclip/fclipredo/Makefile
apps/fileclip/fclipundo/Makefile
apps/cat/Makefile
apps/fcompress/Makefile
apps/ai/Makefile
apps/namespaces/Makefile
apps/cursors/Makefile
apps/fmkdir/Makefile
apps/fmodestr2octal/Makefile
apps/oprocdeamon/Makefile
apps/ftouch/Makefile
apps/fschema/Makefile
apps/fmedallion/Makefile
apps/ferriscd/Makefile
apps/fca/Makefile
apps/ls/Makefile
apps/copy/Makefile
apps/rm/Makefile
apps/mv/Makefile
apps/xml/Makefile
apps/xml/XSLTransform/Makefile
apps/xml/CGITransform/Makefile
apps/head/Makefile
apps/tail/Makefile
apps/fileactions/Makefile
apps/ferrisredirect/Makefile
apps/rdf/Makefile
apps/xmmsdaemon/Makefile
apps/metadataserver/Makefile
apps/phpsearchinterface/Makefile
apps/phpsearchinterface/dhtmlxSuite/Makefile
apps/frunner/Makefile
apps/volumemanager/Makefile
apps/du/Makefile
apps/scripts/Makefile
apps/upnpserver/Makefile
apps/tools/Makefile
apps/rest/Makefile
apps/yuibrowser/Makefile
apps/yuibrowser/images/Makefile
appsui/Makefile
appsui/gfcopy/Makefile
appsui/gfrm/Makefile
appsui/gfmv/Makefile
appsui/gfdl/Makefile
appsui/gfproperties/Makefile
appsui/gfindex/Makefile
appsui/volumemanager/Makefile
tests/Makefile
tests/contextiostream/Makefile
tests/runner/Makefile
tests/trivial/Makefile
tests/quoting/Makefile
tests/sigtoy/Makefile
tests/extoy/Makefile
tests/comparetoy/Makefile
tests/logging/Makefile
tests/regression/Makefile
tests/regression/relativePaths/Makefile
tests/regression/timber/Makefile
tests/stringtrimming/Makefile
tests/sockets/Makefile
tests/sockets/daytime/Makefile
tests/sockets/echo/Makefile
tests/sockets/echoclienttls/Makefile
tests/sockets/echoserver/Makefile
tests/sockets/echoservertls/Makefile
tests/sockets/socketops/Makefile
tests/sqlplus/Makefile
tests/sqlplus/simplequery/Makefile
tests/streams/Makefile
tests/streams/ostream_modifiers/Makefile
tests/streams/performance/Makefile
tests/medallions/Makefile
tests/medallions/simpleroot/Makefile
tests/memorymanage/Makefile
tests/memorymanage/cleanuptest/Makefile
tests/xml/Makefile
tests/xml/testMakeDOM/Makefile
tests/ipc/Makefile
tests/ipc/shm/Makefile
tests/ipc/semaphores/Makefile
tests/contextiterator/Makefile
tests/rdf/Makefile
tests/links/Makefile
tests/lucene/Makefile
tests/timeparsing/Makefile
tests/touch/Makefile
tests/hiddensymbols/Makefile
factories/Makefile
factories/eagenerators/Makefile
factories/astext/Makefile
factories/creation/Makefile
plugins/Makefile
plugins/context/Makefile
plugins/context/commondbapi/Makefile
plugins/context/commonsqldbapi/Makefile
plugins/context/libmpeg3/Makefile
plugins/context/xml/Makefile
plugins/context/dbxml/Makefile
plugins/context/edb/Makefile
plugins/context/dvdread/Makefile
plugins/context/nativembox/Makefile
plugins/context/sockets/Makefile
plugins/context/sqlplus/Makefile
plugins/context/ipc/Makefile
plugins/context/db4/Makefile
plugins/context/eet/Makefile
plugins/context/gdbm/Makefile
plugins/context/tdb/Makefile
plugins/context/spirit/Makefile
plugins/context/spirit/ffilter/Makefile
plugins/context/spirit/fulltextboolean/Makefile
plugins/context/external/Makefile
plugins/context/external/mapping.cpp
plugins/context/external/extfs_modules/Makefile
plugins/context/external/extfs_modules/ssh
plugins/context/ldap/Makefile
plugins/context/soprano/Makefile
plugins/context/dtl/Makefile
plugins/context/postgresql/Makefile
plugins/context/gphoto2/Makefile
plugins/context/evolution/Makefile
plugins/context/bibtex/Makefile
plugins/context/xwin/Makefile
plugins/context/annodex/Makefile
plugins/context/emacs/Makefile
plugins/context/emacs/libferrisemacs.hh
plugins/context/firefox/Makefile
plugins/context/firefox/firefox-extension/Makefile
plugins/context/obby/Makefile
plugins/context/xsltfs/Makefile
plugins/context/xmms/Makefile
plugins/context/amarok/Makefile
plugins/context/dbus/Makefile
plugins/context/hal/Makefile
plugins/context/qtsql/Makefile
plugins/context/recordfile/Makefile
plugins/context/google/Makefile
plugins/context/facebook/Makefile
plugins/context/vimeo/Makefile
plugins/context/identica/Makefile
plugins/context/gstreamer/Makefile
plugins/context/upnp/Makefile
plugins/context/webphotos/Makefile
plugins/context/qtftp/Makefile
plugins/context/qthttp/Makefile
plugins/context/plasma/Makefile
plugins/context/sane/Makefile
plugins/context/printer/Makefile
plugins/context/pulseaudio/Makefile
plugins/context/wiki/Makefile
plugins/context/pastebin/Makefile
plugins/context/zoneminder/Makefile
plugins/context/ferrisrest/Makefile
plugins/context/kio/Makefile
plugins/eagenerators/Makefile
plugins/eagenerators/jasper/Makefile
plugins/eagenerators/id3/Makefile
plugins/eagenerators/imlib2/Makefile
plugins/eagenerators/magick/Makefile
plugins/eagenerators/png/Makefile
plugins/eagenerators/djvu/Makefile
plugins/eagenerators/jpeg/Makefile
plugins/eagenerators/libmpeg3/Makefile
plugins/eagenerators/gimp/Makefile
plugins/eagenerators/xfsnative/Makefile
plugins/eagenerators/a52/Makefile
plugins/eagenerators/mpeg2/Makefile
plugins/eagenerators/soprano/Makefile
plugins/eagenerators/exif/Makefile
plugins/eagenerators/xmp/Makefile
plugins/eagenerators/libextractor/Makefile
plugins/eagenerators/kde3metadata/Makefile
plugins/eagenerators/xine/Makefile
plugins/eagenerators/fspot/Makefile
plugins/eagenerators/strigi/Makefile
plugins/eagenerators/flac/Makefile
plugins/eagenerators/oggz/Makefile
plugins/eagenerators/taglib/Makefile
plugins/eagenerators/mediainfo/Makefile
plugins/eagenerators/subtitles/Makefile
plugins/astext/Makefile
plugins/astext/pdf/Makefile
plugins/astext/html/Makefile
plugins/astext/man/Makefile
plugins/astext/djvu/Makefile
plugins/astext/msdoc/Makefile
plugins/astext/extern/Makefile
plugins/creation/Makefile
plugins/creation/db4/Makefile
plugins/creation/dbxml/Makefile
plugins/creation/devsource/Makefile
plugins/creation/fulltextindex/Makefile
plugins/creation/fulltextindexgeneric/Makefile
plugins/creation/eaindex/Makefile
plugins/creation/eaindexgeneric/Makefile
plugins/creation/gdbm/Makefile
plugins/creation/magickimage/Makefile
plugins/creation/markup/Makefile
plugins/creation/mng/Makefile
plugins/creation/mp3/Makefile
plugins/creation/mpeg2/Makefile
plugins/creation/ogg/Makefile
plugins/creation/shellscript/Makefile
plugins/creation/tdb/Makefile
plugins/creation/wav/Makefile
plugins/creation/soprano/Makefile
plugins/eaindexers/Makefile
plugins/eaindexers/ldap/Makefile
plugins/eaindexers/lucene/Makefile
plugins/eaindexers/clucene/Makefile
plugins/eaindexers/odbc/Makefile
plugins/eaindexers/qtsql/Makefile
plugins/eaindexers/postgresql/Makefile
plugins/eaindexers/soprano/Makefile
plugins/eaindexers/xapian/Makefile
plugins/eaindexers/null/Makefile
plugins/eaindexers/federation/Makefile
plugins/eaindexers/boost/Makefile
plugins/fulltextindexers/Makefile
plugins/fulltextindexers/lucene/Makefile
plugins/fulltextindexers/clucene/Makefile
plugins/fulltextindexers/odbc/Makefile
plugins/fulltextindexers/qtsql/Makefile
plugins/fulltextindexers/xapian/Makefile
plugins/fulltextindexers/tsearch2/Makefile
plugins/fulltextindexers/yahoo/Makefile
plugins/fulltextindexers/external/Makefile
plugins/fulltextindexers/beagle/Makefile
plugins/fulltextindexers/null/Makefile
plugins/fulltextindexers/uniqsorted/Makefile
plugins/fulltextindexers/strigi/Makefile
dot-ferris/Makefile
dot-ferris/licenses/Makefile
dot-ferris/full-text-index/Makefile
dot-ferris/ea-index/Makefile
dot-ferris/tmp/Makefile
dot-ferris/etagere/Makefile
dot-ferris/rdfdb/Makefile
dot-ferris/sane/Makefile
dot-ferris/sane/dot-discovered/Makefile
dot-ferris/sane/dot-discovered/color-100/Makefile
dot-ferris/sane/dot-discovered/color-300/Makefile
dot-ferris/sane/dot-discovered/color-600/Makefile
dot-ferris/sane/dot-discovered/color-base/Makefile
dot-ferris/sane/dot-discovered/gray-100/Makefile
dot-ferris/sane/dot-discovered/gray-300/Makefile
dot-ferris/sane/dot-discovered/gray-600/Makefile
dot-ferris/sane/dot-discovered/gray-base/Makefile
dot-ferris/sane/dot-discovered/color-100/options/Makefile
dot-ferris/sane/dot-discovered/color-300/options/Makefile
dot-ferris/sane/dot-discovered/color-600/options/Makefile
dot-ferris/sane/dot-discovered/color-base/options/Makefile
dot-ferris/sane/dot-discovered/gray-100/options/Makefile
dot-ferris/sane/dot-discovered/gray-300/options/Makefile
dot-ferris/sane/dot-discovered/gray-600/options/Makefile
dot-ferris/sane/dot-discovered/gray-base/options/Makefile
media/Makefile
media/icons/Makefile
media/xslt/Makefile
cc/Makefile
cc/Ferris/Makefile
cc/capplets/Makefile
cc/capplets/curl-ftp/Makefile
cc/capplets/logging/Makefile
cc/capplets/locale/Makefile
cc/capplets/auth/Makefile
cc/capplets/version/Makefile
cc/capplets/version/ferris-capplet-version.hh
cc/capplets/general/Makefile
cc/capplets/index/Makefile
cc/capplets/medallion/Makefile
cc/capplets/agents/Makefile
cc/capplets/thumbnails/Makefile
cc/capplets/rdf/Makefile
cc/Ferris/curl-ftp.desktop
cc/Ferris/locale.desktop
cc/Ferris/logging.desktop
cc/Ferris/auth.desktop
cc/Ferris/version.desktop
cc/Ferris/medallion.desktop
cc/Ferris/indexing.desktop
cc/Ferris/general.desktop
cc/Ferris/agents.desktop
cc/Ferris/thumbnails.desktop
firsttime/Makefile
firsttime/console/Makefile
firsttime/ui/Makefile
xsltfunctions/Makefile
noarch/Makefile
migration/Makefile
perl/Makefile
exposures/Makefile
exposures/sqlitevtab/Makefile
exposures/plasma/Makefile
exposures/perl/Makefile
exposures/kio/Makefile
patches/Makefile
ferris-config
ferris.pc
ferrisui.pc
ferrisxslt.pc],
[
chmod +x ferris-config
])

dnl plugins/context/pccts/Makefile
dnl plugins/context/pccts/ffilter/Makefile
dnl plugins/context/pccts/fulltextboolean/Makefile

dnl plugins/eagenerators/Makefile


dnl A really bad hack for ffilter make
echo '# dummy' >|plugins/context/pccts/ffilter/.deps/DLGLexer.Po
echo '# dummy' >|plugins/context/pccts/ffilter/.deps/DLGLexer.Plo

echo "

libferris.so: $FERRIS_VERSION

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC} 
	CFLAGS:			${CFLAGS} 
	CXXFLAGS:		${CXXFLAGS} 
	LDFLAGS:                ${LDFLAGS}
	IOSIZE:                 ${IOSIZE}
"

if test "$have_sqlplus" = yes; then
echo "	sqlplus libs:    	$SQLPLUS_LIBS
	sqlplus cflags:  	$SQLPLUS_CFLAGS"
fi

if test "$have_dtl" = yes; then
echo "	DTL libs:    	$DTL_LIBS
	DTL cflags:  	$DTL_CFLAGS"
fi

if test "$have_libpqxx" = yes; then
echo "	PQXX libs:    	$LIBPQXX_LIBS
	PQXX cflags:  	$LIBPQXX_CFLAGS"
fi

if test "$have_curl" = yes; then
echo "	curl libs:    	  	$CURL_LIBS
	curl cflags:      	$CURL_CFLAGS"
fi

if test "$have_imlib2" = yes; then
echo "	imlib2 libs:    	$IMLIB2_LIBS
	imlib2 cflags:  	$IMLIB2_CFLAGS"
fi

if test "$have_edb" = yes; then
echo "	edb    libs:    	$EDB_LIBS
	edb    cflags:  	$EDB_CFLAGS"
fi

if test "$have_dvdread" = yes; then
echo "	dvdread libs:    	$DVDREAD_LIBS
	dvdread cflags:  	$DVDREAD_CFLAGS"
fi

if test "$have_ipc_context" = yes; then
echo "	ipc context libs:    	$IPC_CONTEXT_LIBS
	ipc context cflags:  	$IPC_CONTEXT_CFLAGS"
fi

if test "$have_db4" = yes; then
echo "	db4 context libs:    	$DB4_LIBS
	db4 context cflags:  	$DB4_CFLAGS"
fi

if test "$have_eet" = yes; then
echo "	eet context libs:    	$EET_LIBS
	eet context cflags:  	$EET_CFLAGS"
fi

if test "$have_gdbm" = yes; then
echo "	gdbm context libs:    	$GDBM_LIBS
	gdbm context cflags:  	$GDBM_CFLAGS"
fi

if test "$have_tdb" = yes; then
echo "	tdb context libs:    	$TDB_LIBS
	tdb context cflags:  	$TDB_CFLAGS"
fi

if test "$have_id3" = yes; then
echo "	id3    libs:    	$ID3_LIBS
	id3    cflags:  	$ID3_CFLAGS"
fi

if test "$have_xml4c" = yes; then
echo "	xml4c  libs:    	$XML4C_LIBS
	xml4c  cflags:  	$XML4C_CFLAGS"
fi

if test "$have_xalan" = yes; then
echo "	xalan  libs:    	$XALAN_LIBS
	xalan  cflags:  	$XALAN_CFLAGS"
fi

if test "$have_pccts" = yes; then
echo "	pccts  cflags:  	$PCCTS_CFLAGS"
fi

if test "$have_gimp" = yes; then
echo "	gimp   libs:    	$GIMP_LIBS
	gimp   cflags:  	$GIMP_CFLAGS"
fi

if test "$have_libjpeg" = yes; then
echo "	libjpeg libs:    	$LIBJPEG_LIBS
	libjpeg cflags:  	$LIBJPEG_CFLAGS"
fi

if test "$have_libpng" = yes; then
echo "	libpng  libs:    	$LIBPNG_LIBS
	libpng  cflags:  	$LIBPNG_CFLAGS"
fi

if test "$have_libdjvulibre" = yes; then
echo "	libdjvulibre  libs:    	$LIBDJVULIBRE_LIBS
	libdjvulibre  cflags:  	$LIBDJVULIBRE_CFLAGS"
fi

if test "$have_magick" = yes; then
echo "	ImageMagick++ libs:    	$MAGICK_LIBS
	ImageMagick++ cflags:  	$MAGICK_CFLAGS"
fi

if test "$have_libmpeg3" = yes; then
echo "	libmpeg3      libs:    	$LIBMPEG3_LIBS
	libmpeg3      cflags:  	$LIBMPEG3_CFLAGS"
fi

if test "$have_libattr" = yes; then
echo "	libattr       libs:    	$LIBATTR_LIBS
	libattr       cflags:  	$LIBATTR_CFLAGS"
fi

if test "$have_liba52" = yes; then
echo "	liba52        libs:    	$LIBA52_LIBS
	liba52        cflags:  	$LIBA52_CFLAGS"
fi

if test "$have_libmpg2" = yes; then
echo "	libmpg2       libs:    	$LIBMPG2_LIBS
	libmpg2       cflags:  	$LIBMPG2_CFLAGS"
fi

if test "$have_ldap" = yes; then
echo "	ldap       libs:    	$LDAP_LIBS
	ldap       cflags:  	$LDAP_CFLAGS"
fi

if test "$have_evolution" = yes; then
echo "	evolution       libs:    	$EVOLUTION_LIBS
	evolution       cflags:  	$EVOLUTION_CFLAGS"
fi

if test "$have_soprano" = yes; then
echo "	soprano       libs:    	$SOPRANO_LIBS
	soprano       cflags:  	$SOPRANO_CFLAGS"
fi

if test "$have_libplatinum" = yes; then
echo "	Platinum uPnP libs:    	$LIBPLATINUM_LIBS
	Platinum uPnP cflags:  	$LIBPLATINUM_CFLAGS"
fi

echo " ";
echo "MIME Engine: $MIMETYPE_ENGINE_DESC";
echo " ";

echo " ";
echo "QT_CFLAGS: $QT_CFLAGS"
echo "QT_LIBS: $QT_LIBS"
echo " ";

echo " ";
echo "linking in 'as-text' EA generators at $LIBASTEXT_PLUGIN_FACTORIES_LA";
echo " ";
echo " ";
echo "linking in 'creation' factories at $LIBCREATION_PLUGIN_FACTORIES_LA";
echo " ";
echo " ";
echo "linking in 'ea generation' factories at $LIBEAGEN_PLUGIN_FACTORIES_LA";
echo " ";

echo $FERRIS_IOSIZE_WARNING

echo "***************"
echo "*** WARNING ***"
echo "The code for indexing is still not 100% disk format frozen."
echo "you should remake index files when upgrading libferris, "
echo "default index is at ~/.ferris/full-text-index"
echo "you can use ./apps/fulltextindex/findexadd in a script to automate this process"
echo ""
echo "Attribute indexes are much more volatile and should be recreated"
echo "each upgrade for now. A similar tool to findexadd should exist to help in this"
echo "

optional modules that will be compiled: 
$modlist

	Copyright (C) 2001-2015 Ben Martin
"
