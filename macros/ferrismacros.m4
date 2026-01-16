dnl @synopsis AC_PATH_GENERIC(LIBRARY [, MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl
dnl Runs a LIBRARY-config script and defines LIBRARY_CFLAGS and LIBRARY_LIBS
dnl
dnl The script must support `--cflags' and `--libs' args.
dnl If MINIMUM-VERSION is specified, the script must also support the
dnl `--version' arg.
dnl If the `--with-library-[exec-]prefix' arguments to ./configure are given,
dnl it must also support `--prefix' and `--exec-prefix'.
dnl (In other words, it must be like gtk-config.)
dnl
dnl For example:
dnl
dnl    AC_PATH_GENERIC(Foo, 1.0.0)
dnl
dnl would run `foo-config --version' and check that it is at least 1.0.0
dnl
dnl If so, the following would then be defined:
dnl
dnl    FOO_CFLAGS to `foo-config --cflags`
dnl    FOO_LIBS   to `foo-config --libs`
dnl
dnl At present there is no support for additional "MODULES" (see AM_PATH_GTK)
dnl (shamelessly stolen from gtk.m4 and then hacked around a fair amount)
dnl
dnl @author Angus Lees <gusl@cse.unsw.edu.au>
dnl @version $Id: ferrismacros.m4,v 1.36 2011/06/17 21:38:48 ben Exp $

AC_DEFUN(AC_PATH_GENERIC,
[dnl
dnl we're going to need uppercase, lowercase and user-friendly versions of the
dnl string `LIBRARY'
pushdef([UP], translit([$1], [a-z], [A-Z]))dnl
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl

dnl
dnl Get the cflags and libraries from the LIBRARY-config script
dnl
AC_ARG_WITH(DOWN-prefix,[  --with-]DOWN[-prefix=PFX       Prefix where $1 is installed (optional)],
        DOWN[]_config_prefix="$withval", DOWN[]_config_prefix="")
AC_ARG_WITH(DOWN-exec-prefix,[  --with-]DOWN[-exec-prefix=PFX Exec prefix where $1 is installed (optional)],
        DOWN[]_config_exec_prefix="$withval", DOWN[]_config_exec_prefix="")

  if test x$DOWN[]_config_exec_prefix != x ; then
     DOWN[]_config_args="$DOWN[]_config_args --exec-prefix=$DOWN[]_config_exec_prefix"
     if test x${UP[]_CONFIG+set} != xset ; then
       UP[]_CONFIG=$DOWN[]_config_exec_prefix/bin/DOWN-config
     fi
  fi
  if test x$DOWN[]_config_prefix != x ; then
     DOWN[]_config_args="$DOWN[]_config_args --prefix=$DOWN[]_config_prefix"
     if test x${UP[]_CONFIG+set} != xset ; then
       UP[]_CONFIG=$DOWN[]_config_prefix/bin/DOWN-config
     fi
  fi

  AC_PATH_PROG(UP[]_CONFIG, DOWN-config, no)
  ifelse([$2], ,
     AC_MSG_CHECKING(for $1),
     AC_MSG_CHECKING(for $1 - version >= $2)
  )
  no_[]DOWN=""
  if test "$UP[]_CONFIG" = "no" ; then
     no_[]DOWN=yes
  else
     UP[]_CFLAGS="`$UP[]_CONFIG $DOWN[]_config_args --cflags`"
     UP[]_LIBS="`$UP[]_CONFIG $DOWN[]_config_args --libs`"
     ifelse([$2], , ,[
        DOWN[]_config_major_version=`$UP[]_CONFIG $DOWN[]_config_args \
         --version | sed 's/[[^0-9]]*\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
        DOWN[]_config_minor_version=`$UP[]_CONFIG $DOWN[]_config_args \
         --version | sed 's/[[^0-9]]*\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
        DOWN[]_config_micro_version=`$UP[]_CONFIG $DOWN[]_config_args \
         --version | sed 's/[[^0-9]]*\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
        DOWN[]_wanted_major_version="regexp($2, [\<\([0-9]*\)], [\1])"
        DOWN[]_wanted_minor_version="regexp($2, [\<\([0-9]*\)\.\([0-9]*\)], [\2])"
        DOWN[]_wanted_micro_version="regexp($2, [\<\([0-9]*\).\([0-9]*\).\([0-9]*\)], [\3])"

        # Compare wanted version to what config script returned.
        # If I knew what library was being run, i'd probably also compile
        # a test program at this point (which also extracted and tested
        # the version in some library-specific way)
        if test "$DOWN[]_config_major_version" -lt \
                        "$DOWN[]_wanted_major_version" \
          -o \( "$DOWN[]_config_major_version" -eq \
                        "$DOWN[]_wanted_major_version" \
            -a "$DOWN[]_config_minor_version" -lt \
                        "$DOWN[]_wanted_minor_version" \) \
          -o \( "$DOWN[]_config_major_version" -eq \
                        "$DOWN[]_wanted_major_version" \
            -a "$DOWN[]_config_minor_version" -eq \
                        "$DOWN[]_wanted_minor_version" \
            -a "$DOWN[]_config_micro_version" -lt \
                        "$DOWN[]_wanted_micro_version" \) ; then
          # older version found
          no_[]DOWN=yes
          echo -n "*** An old version of $1 "
          echo -n "($DOWN[]_config_major_version"
          echo -n ".$DOWN[]_config_minor_version"
          echo    ".$DOWN[]_config_micro_version) was found."
          echo -n "*** You need a version of $1 newer than "
          echo -n "$DOWN[]_wanted_major_version"
          echo -n ".$DOWN[]_wanted_minor_version"
          echo    ".$DOWN[]_wanted_micro_version."
          echo "***"
          echo "*** If you have already installed a sufficiently new version, this error"
          echo "*** probably means that the wrong copy of the DOWN-config shell script is"
          echo "*** being found. The easiest way to fix this is to remove the old version"
          echo "*** of $1, but you can also set the UP[]_CONFIG environment to point to the"
          echo "*** correct copy of DOWN-config. (In this case, you will have to"
          echo "*** modify your LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf"
          echo "*** so that the correct libraries are found at run-time)"
        fi
     ])
  fi
  if test "x$no_[]DOWN" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$3], , :, [$3])
  else
     AC_MSG_RESULT(no)
     if test "$UP[]_CONFIG" = "no" ; then
       echo "*** The DOWN-config script installed by $1 could not be found"
       echo "*** If $1 was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the UP[]_CONFIG environment variable to the"
       echo "*** full path to DOWN-config."
     fi
     UP[]_CFLAGS=""
     UP[]_LIBS=""
     ifelse([$4], , :, [$4])
  fi
  AC_SUBST(UP[]_CFLAGS)
  AC_SUBST(UP[]_LIBS)

  popdef([UP])
  popdef([DOWN])
])

################################################################################
################################################################################
################################################################################

#
# Common macros used by many configure.in scripts in the ferris suite.
#

dnl Force the use of libtool and AC_TRY_LINK!
dnl http://www.mail-archive.com/libtool@gnu.org/msg01271.html
AC_DEFUN(AM_FERRIS_LIBTOOL_TRYLINK,
[dnl
dnl
	am_ferris_libtool_trylink_pass=no
	save_CXX=$CXX
	CXX="${SHELL-/bin/sh} ./libtool  --tag=CXX --mode=link $CXX"
	AC_TRY_LINK( [$1], [$2], 
		[ am_ferris_libtool_trylink_pass=yes; ], [ am_ferris_libtool_trylink_pass=no; ] )
	CXX=$save_CXX

	if test x"$am_ferris_libtool_trylink_pass" = xyes; then
		# success
		ifelse([$3], , :, [$3])
	else
		ifelse([$4], , 	[], [$4])     
	fi
])

###############################################################################
###############################################################################
###############################################################################
# Test for stlport 4.5
###############################################################################

dnl
dnl
dnl See AM_FERRIS_STLPORT() for the macro you want to call externally.
dnl
dnl

STLPORT_IO64_CFLAGS=" -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 "

dnl
dnl
dnl AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]])
dnl Used internally to try to link an STLPort using C++ application
dnl using the STLPORT_CFLAGS and STLPORT_LIBS. 
dnl STLPORT_IOSIZE will be set to either 32 or 64 depending on what 
dnl width of IO the found STLPort supports and STLPORT_CFLAGS may be adjusted
dnl for 64 bit building.
dnl 
dnl the STLPORT_CFLAGS etc are all cleared after a failed test.
dnl
AC_DEFUN(AM_FERRIS_STLPORT_INTERNAL_TRYLINK,
[dnl
dnl
	AC_LANG_CPLUSPLUS
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $STLPORT_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $STLPORT_LIBS "

	AC_TRY_LINK([
		#include <hash_map>
		],
		[
		std::hash_map<int,int> hm;
		hm[5] = 6;
		],
	       	[ferris_stlport_internal_trylink=yes; STLPORT_IOSIZE=32 ],
	       	[ferris_stlport_internal_trylink=no] )

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	AC_LANG_C

	if test x"$ferris_stlport_internal_trylink" = xno; then
		AC_LANG_CPLUSPLUS
		CXXFLAGS_cache=$CXXFLAGS
		CXXFLAGS=" $CXXFLAGS $STLPORT_IO64_CFLAGS $STLPORT_CFLAGS "
		LDFLAGS_cache=$LDFLAGS
		LDFLAGS=" $LDFLAGS $STLPORT_LIBS "

		AC_TRY_LINK([
			#include <hash_map>
			],
			[
			std::hash_map<int,int> hm;
			hm[5] = 6;
			],
		       	[ferris_stlport_internal_trylink=yes; STLPORT_IOSIZE=64 ],
	       		[ferris_stlport_internal_trylink=no] )

		LDFLAGS=$LDFLAGS_cache
		CXXFLAGS=$CXXFLAGS_cache
		AC_LANG_C
	fi

	if test x"$ferris_stlport_internal_trylink" = xyes; then
	     ifelse([$1], , :, [$1])     
	else
	     ifelse([$2], , :, [$2])     
		if test x"$have_stlport" = xno; then
			STLPORT_CFLAGS=""
			STLPORT_LDFLAGS=""
	 		STLPORT_LIB=""
			STLPORT_LIBS=""
		fi
	fi
])

dnl AM_FERRIS_STLPORT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate STLPort for installation. 
dnl ie. default is to REQUIRE STLPort MINIMUM-VERSION or stop running.
dnl
dnl MINIMUM-VERSION must be a three part value, like 4.5.0
dnl
dnl Test for STLPort, and define STLPORT_CFLAGS, STLPORT_LIBS and STLPORT_IOSIZE
dnl other side effects include
dnl AM_CONDITIONAL( HAVE_STLPORT, 1 or 0 )
dnl AC_SUBST( STLPORT_CFLAGS )
dnl AC_SUBST( STLPORT_LIBS )
dnl 
dnl if( success ) 
dnl    AC_DEFINE( HAVE_STLPORT )
dnl    AC_DEFINE( STLPORT_IOSIZE )
dnl
AC_DEFUN(AM_FERRIS_STLPORT,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl

stlport_required_version=$1
have_stlport=no

AC_ARG_WITH(stlport,
        [  --with-stlport=DIR          use stlport 4.5+ install rooted at <DIR>],
        [STLPORT_CFLAGS=" -I$withval/stlport "
	 STLPORT_LDFLAGS=" -L$withval/lib "
 	 STLPORT_LIB=" -lstlport -lpthread "
	 STLPORT_LIBS=" -L$withval/lib ${STLPORT_LIB} "
	 stlport_try_trivial_compile=yes
        ])
if test x"$stlport_try_trivial_compile" = xyes; then
	AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [have_stlport=yes], [have_stlport=no]  )
fi

if test x"$have_stlport" = xno; then

	package=stlport
	version=4.5.3
	PKG_CHECK_MODULES(STLPORT, $package >= $version, [ have_stlport=yes ], [foo=1] )
fi

if test x"$have_stlport" = xno; then

	package=stlport
	version=5.0
	PKG_CHECK_MODULES(STLPORT, $package >= $version, [ have_stlport=yes ], [foo=1] )
fi

if test x"$have_stlport" = xno; then

	AC_LANG_CPLUSPLUS
	STLPORT_IO64_CFLAGS=" -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 "

	AC_CHECK_PROG( have_stlportcfg, stlport-config, yes, no )
	
	if test "$have_stlportcfg" = yes; then

#		AC_PATH_GENERIC( STLPORT, 4.5, [ have_stlport=yes ], [foo=2] )
		stlport_installed_version=`stlport-config --version`

		# Calculate the available version number
		[f_tmp=( `echo $stlport_installed_version | sed 's/[^0-9]\+/ /g'` )]
		[f_tmp=$(( 1000000 * ${f_tmp[0]:-0} + 1000 * ${f_tmp[1]:-0} + ${f_tmp[2]:-0} ))]

		[freq_version=( `echo $stlport_required_version | sed 's/[^0-9]\+/ /g'` )]
		[freq_version=$(( 1000000 * ${freq_version[0]:-0} + 1000 * ${freq_version[1]:-0} + ${freq_version[2]:-0} ))]

		if test $freq_version -gt $f_tmp ; then
			AC_MSG_WARN([STLPort version $1 is required, you have $stlport_installed_version])
		else
			have_stlport=yes
			STLPORT_LIBS=" `stlport-config --libs` -lpthread "
			STLPORT_CFLAGS=" `stlport-config --cflags` "
			AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [have_stlport=yes], [have_stlport=no]  )
		fi
	fi
fi

if test x"$have_stlport" = xno; then
	STLPORT_CFLAGS=" -I/usr/local/STLport-4.5/stlport "
	STLPORT_LIB=" -lstlport -lpthread "
	STLPORT_LIBS=" -L/usr/local/STLport-4.5/lib ${STLPORT_LIB} "

	AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [have_stlport=yes], [have_stlport=no]  )
fi

if test x"$have_stlport" = xno; then
	STLPORT_CFLAGS=" -I/usr/local/include/stlport "
	STLPORT_LIB=" -lstlport -lpthread "
	STLPORT_LIBS=" -L/usr/local/lib ${STLPORT_LIB} "

	AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [have_stlport=yes], [have_stlport=no]  )
fi

if test x"$have_stlport" = xno; then
	STLPORT_CFLAGS=" -I/usr/include/stlport "
	STLPORT_LIB=" -lstlport -lpthread "
	STLPORT_LIBS="  ${STLPORT_LIB} "

	AM_FERRIS_STLPORT_INTERNAL_TRYLINK( [have_stlport=yes], [have_stlport=no]  )
fi

dnl
dnl just make sure of the assertion that we have a valid STLPort
dnl
if test x"$have_stlport" = xyes; then

	AM_FERRIS_STLPORT_INTERNAL_TRYLINK( 
	[
		AC_DEFINE( HAVE_STLPORT, 1, [Is STLPort 4.5+ installed] )
		AC_DEFINE( STLPORT_IOSIZE, 1, [Width of seekable units in iostreams] )

		echo "Found an STLport that meets required needs..."
		echo "  STLPORT_CFLAGS: $STLPORT_CFLAGS "
		echo "  STLPORT_LIBS:   $STLPORT_LIBS "

		# success
		ifelse([$2], , :, [$2])
	], 
	[
		# fail
		ifelse([$3], , 
		[
			echo ""
			echo "STLPort $version can not be detected on your system. "
			echo ""
			echo "Please make sure that STLPort with IOStreams"
			echo "support is available on your machine before "
			echo "trying again. "
			echo ""
			echo "get it from the URLs below"
			echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
			echo "  http://www.stlport.com/download.html"
			AC_MSG_ERROR([Fatal Error: no STLPort $version or later found.])	
		], 
	[$3])     
	] )
else
	ifelse([$3], , 
	[
echo "cflags:$STLPORT_CFLAGS"
echo "ldflags:$STLPORT_LDFLAGS"
echo "libs:$STLPORT_LIBS"
		echo ""
		echo "STLPort $version can not be detected on your system. "
		echo ""
		echo "Please make sure that STLPort with IOStreams"
		echo "support is available on your machine before "
		echo "trying again. "
		echo ""
		echo "get it from the URLs below"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		echo "  http://www.stlport.com/download.html"
		AC_MSG_ERROR([Fatal Error: no STLPort $version or later found.])	
	], 
	[$3])     
fi

AC_SUBST(STLPORT_CFLAGS)
AC_SUBST(STLPORT_LDFLAGS)
AC_SUBST(STLPORT_LIBS)
AC_SUBST(STLPORT_LIB)
AM_CONDITIONAL(HAVE_STLPORT, test x"$have_stlport" = xyes)

AC_LANG_C
])


dnl AM_FERRIS_STLPORT_OPTIONAL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to just inform the user that gcc's std/stl are being used.
dnl
dnl MINIMUM-VERSION must be a three part value, like 4.5.0
dnl
dnl Test for STLPort, and define STLPORT_CFLAGS, STLPORT_LIBS and STLPORT_IOSIZE
dnl other side effects include
dnl AM_CONDITIONAL( HAVE_STLPORT, 1 or 0 )
dnl AC_SUBST( STLPORT_CFLAGS )
dnl AC_SUBST( STLPORT_LIBS )
dnl 
dnl if( success ) 
dnl    AC_DEFINE( HAVE_STLPORT )
dnl    AC_DEFINE( STLPORT_IOSIZE )
dnl
AC_DEFUN(AM_FERRIS_STLPORT_OPTIONAL,
[dnl 
dnl

stlport_required_version=$1
have_stlport=no

attempt_to_use_stlport=yes
AC_ARG_ENABLE(stlport,
[--disable-stlport            Don't use STLport even if it is detected],
[
  if test x$enableval = xyes; then
	attempt_to_use_stlport=yes
  else
	attempt_to_use_stlport=no
  fi
])

echo "attempt_to_use_stlport:${attempt_to_use_stlport}"

if test x"$attempt_to_use_stlport" = xyes; then
	version=${stlport_required_version}
	AM_FERRIS_STLPORT( $version, 
	[
		IOSIZE=STLPORT_IOSIZE
		AC_SUBST(IOSIZE)
	],
	[
		echo "No STLport found, attempting to use your compilers std and STL."
	] )
fi

HAVE_STLPORT=y
AM_CONDITIONAL(HAVE_STLPORT, test x"$have_stlport" = xyes)
])


dnl ################################################################################
dnl ################################################################################
dnl ################################################################################
dnl ################################################################################



dnl AM_FERRIS_FERRIS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libferris for installation. 
dnl ie. default is to REQUIRE libferris MINIMUM-VERSION or stop running.
dnl
dnl FERRIS_CFLAGS and FERRIS_LIBS are set and AC_SUBST()ed when library is found.
dnl LIBFERRIS_CFLAGS and LIBFERRIS_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_FERRIS,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
required_version=$1

package=ferris
version=$required_version
PKG_CHECK_MODULES(FERRIS, $package >= $version,
[
	AC_DEFINE( HAVE_FERRIS, 1, [Have libferris installed] )
	have_package=yes

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
  		echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "See  http://witme.sourceforge.net/libferris.web/"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])
AM_CONDITIONAL(HAVE_FERRIS, test x"$have_package" = xyes)
AC_SUBST(FERRIS_CFLAGS)
AC_SUBST(FERRIS_LIBS)
LIBFERRIS_CFLAGS="$FERRIS_CFLAGS"
LIBFERRIS_LIBS="$FERRIS_LIBS"
AC_SUBST(LIBFERRIS_CFLAGS)
AC_SUBST(LIBFERRIS_LIBS)
])


dnl AM_FERRIS_FERRISUI([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libferris for installation. 
dnl ie. default is to REQUIRE libferris MINIMUM-VERSION or stop running.
dnl
dnl FERRISUI_CFLAGS and FERRISUI_LIBS are set and AC_SUBST()ed when library is found.
dnl LIBFERRISUI_CFLAGS and LIBFERRISUI_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_FERRISUI,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
required_version=$1

package=ferrisui
version=$required_version
PKG_CHECK_MODULES(FERRISUI, $package >= $version,
[
	AC_DEFINE( HAVE_FERRISUI, 1, [have libferrisui installed] )
	have_package=yes

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
  		echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "See  http://witme.sourceforge.net/libferris.web/"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])
AM_CONDITIONAL(HAVE_FERRISUI, test x"$have_package" = xyes)
AC_SUBST(FERRISUI_CFLAGS)
AC_SUBST(FERRISUI_LIBS)
LIBFERRISUI_CFLAGS="$FERRISUI_CFLAGS"
LIBFERRISUI_LIBS="$FERRISUI_LIBS"
AC_SUBST(LIBFERRISUI_CFLAGS)
AC_SUBST(LIBFERRISUI_LIBS)
])


dnl AM_FERRIS_SIGC([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate sigc++ for installation. 
dnl ie. default is to REQUIRE sigc++ MINIMUM-VERSION or stop running.
dnl
dnl SIGC_CFLAGS and SIGC_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_SIGC,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
sigc_required_version=$1

AC_ARG_WITH(sigcxx-2x,
AC_HELP_STRING([--with-sigcxx-2x=yes],[use sigc++ 2.x, --with-sigcxx-2x=yes default]),
[  ac_use_sigcxx_2=$withval
], ac_use_sigcxx_2="no"
)

package=sigc++-2.0

version=$sigc_required_version
PKG_CHECK_MODULES(SIGC, $package >= $version,
[
	AC_DEFINE( HAVE_SIGC, 1, [Is sigc++ installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
  		echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "this should be on the freshrpms.net website"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])

AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)
])


dnl AM_FERRIS_SIGC([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate sigc++ for installation. 
dnl ie. default is to REQUIRE sigc++ MINIMUM-VERSION or stop running.
dnl
dnl SIGC_CFLAGS and SIGC_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_SIGC2,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
sigc_required_version=$1

package=sigc++-2.0

version=$sigc_required_version
PKG_CHECK_MODULES(SIGC, $package >= $version,
[
	AC_DEFINE( HAVE_SIGC, 1, [Is sigc++ installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
  		echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "this should be on the freshrpms.net website"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])

dnl This is not really good. But it seems that gcc wants it to work with sigc++ 2.x on Fedora.
SIGC_CFLAGS="$SIGC_CFLAGS -std=c++11"
AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)
])


dnl AM_FERRIS_SIGC([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate sigc++ for installation. 
dnl ie. default is to REQUIRE sigc++ MINIMUM-VERSION or stop running.
dnl
dnl SIGC_CFLAGS and SIGC_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_SIGC3,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
sigc_required_version=$1

package=sigc++-3.0

version=$sigc_required_version
PKG_CHECK_MODULES(SIGC, $package >= $version,
[
	AC_DEFINE( HAVE_SIGC, 1, [Is sigc++ installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
  		echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "this should be on the freshrpms.net website"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])

dnl This is not really good. But it seems that gcc wants it to work with sigc++ 3.x on Fedora.
SIGC_CFLAGS="$SIGC_CFLAGS"
AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)
])


dnl AM_FERRIS_LOKI([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate ferrisloki for installation. 
dnl ie. default is to REQUIRE ferrisloki MINIMUM-VERSION or stop running.
dnl
dnl LOKI_CFLAGS and LOKI_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_LOKI,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
required_version=$1

package=ferrisloki
version=$required_version
PKG_CHECK_MODULES(LOKI, $package >= $version, 
[
	AC_DEFINE( HAVE_LOKI, 1, [is the libferrisloki library installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])
AC_SUBST(LOKI_CFLAGS)
AC_SUBST(LOKI_LIBS)
])


dnl AM_FERRIS_STREAMS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate ferrisstreams for installation. 
dnl ie. default is to REQUIRE ferrisstreams MINIMUM-VERSION or stop running.
dnl
dnl STREAMS_CFLAGS and STREAMS_LIBS are set and AC_SUBST()ed when library is found.
dnl FSTREAM_CFLAGS and FSTREAM_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_STREAMS,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
required_version=$1

package=ferrisstreams
version=$required_version
PKG_CHECK_MODULES(STREAMS, $package >= $version, 
[
	AC_DEFINE( HAVE_STREAMS, 1, [Is libferrisstreams installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])
AC_SUBST(STREAMS_CFLAGS)
AC_SUBST(STREAMS_LIBS)

FSTREAM_CFLAGS=$STREAMS_CFLAGS
FSTREAM_LIBS=$STREAMS_LIBS
AC_SUBST(FSTREAM_CFLAGS)
AC_SUBST(FSTREAM_LIBS)
])


dnl AM_FERRIS_STLDB4([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate stldb4 for installation. 
dnl ie. default is to REQUIRE stldb4 MINIMUM-VERSION or stop running.
dnl
dnl STLDB4_CFLAGS and STLDB4_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_STLDB4,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
required_version=$1

package=stldb4
version=$required_version
PKG_CHECK_MODULES(STLDB4, $package >= $version, 
[
	AC_DEFINE( HAVE_STLDB4, 1, [have libstldb4] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])
AC_SUBST(STLDB4_CFLAGS)
AC_SUBST(STLDB4_LIBS)
])


dnl AM_FERRIS_FAMPP2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate fampp2 for installation. 
dnl ie. default is to REQUIRE fampp2 MINIMUM-VERSION or stop running.
dnl
dnl FAMPP2_CFLAGS and FAMPP2_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_FAMPP2,
[dnl 
dnl Get the cflags and libraries from pkg-config or x-config
dnl
required_version=$1

package=fampp2
version=$required_version
PKG_CHECK_MODULES(FAMPP2, $package >= $version,
[
	AC_DEFINE( HAVE_FAMPP2, 1, [Is fampp2 installed] )

	# success
	ifelse([$2], , :, [$2])
],
[
	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		echo "--- pkg-config output follows for diagnostics"
		pkg-config fampp2 --cflags
		export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/lib/pkgconfig"
		echo "--- pkg-config 32+64 output follows for diagnostics"
		pkg-config fampp2 --cflags
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	])


dnl AC_PATH_GENERIC(FAMPP2, $version, 
dnl [
dnl 	AC_DEFINE( HAVE_FAMPP2, 1, [Is fampp2 installed] )

dnl 	# success
dnl 	ifelse([$2], , :, [$2])
dnl ],
dnl [
dnl 	ifelse([$3], , 
dnl 	[
dnl 	  	echo ""
dnl 		echo "latest version of $package required. ($version or better) "
dnl 		echo ""
dnl 		echo "get it from the URL"
dnl 		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
dnl 		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
dnl 	], 
dnl 	[$3])     
dnl 	])
AC_SUBST(FAMPP2_CFLAGS)
AC_SUBST(FAMPP2_LIBS)
])


dnl
dnl
dnl AM_FERRIS_INTERNAL_TRYLINK( CFLAGS, LIBS, HEADERS, BODY, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]])
dnl Used internally to try to link to a library using C++ application
dnl using the CFLAGS and LIBS. 
dnl
AC_DEFUN(AM_FERRIS_INTERNAL_TRYLINK,
[dnl
dnl
	AC_LANG_CPLUSPLUS
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $1 "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $2 "

	AC_TRY_LINK([
		$3
		],
		[
		$4
		],
	       	[trylink_passed=yes ],
	       	[trylink_passed=no] )

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	AC_LANG_C

	if test x"$trylink_passed" = xyes; then
	     ifelse([$5], , :, [$5])     
	else
	     ifelse([$6], , :, [$6])     
	fi
])

dnl
dnl
dnl AM_FERRIS_INTERNAL_TRYRUN( CFLAGS, LIBS, HEADERS, BODY, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]])
dnl Used internally to try to link to a library using C++ application
dnl using the CFLAGS and LIBS. 
dnl
AC_DEFUN(AM_FERRIS_INTERNAL_TRYRUN,
[dnl
dnl
	AC_LANG_CPLUSPLUS
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $1 "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $2 "

	AC_TRY_RUN([
		$3
		
		int main( int argc, char** argv ) {
			$4
			return 0; }
		],
	       	[trylink_passed=yes ],
	       	[trylink_passed=no] )

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	AC_LANG_C

	if test x"$trylink_passed" = xyes; then
	     ifelse([$5], , :, [$5])     
	else
	     ifelse([$6], , :, [$6])     
	fi
])


dnl AM_FERRIS_XERCESC3([EXACT-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate xerces-c for installation. 
dnl ie. default is to REQUIRE xerces-c EXACT-VERSION or stop running.
dnl
dnl XERCESC_CFLAGS and XERCESC_LIBS are set and AC_SUBST()ed when library is found.
dnl XML4C_CFLAGS   and XML4C_LIBS   are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_XML4C) and AC_DEFINE(HAVE_XERCESC)
dnl
AC_DEFUN(AM_FERRIS_XERCESC3,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
required_version=$1
have_xml4c=no

package=xerces-c
version=$required_version
PKG_CHECK_MODULES(XERCESC, $package >= $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(XERCES-C, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

INCLUDES="$(cat <<-HEREDOC
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <iostream> 
XERCES_CPP_NAMESPACE_USE
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
    // Initialize the XML4C2 system.
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException& toCatch)
    {
        char *pMsg = XMLString::transcode(toCatch.getMessage());
        std::cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << pMsg;
        XMLString::release(&pMsg);
        return 1;
    }

    
    if( XERCES_VERSION_MAJOR != 2 && XERCES_VERSION_MINOR != 2 )
    {
        return 1;
    }
HEREDOC
)"

if test x"$have_package" = xno; then
AC_ARG_WITH(xercesc,
        [  --with-xercesc=DIR          use xercesc $version install rooted at <DIR>],
        [XERCESC_CFLAGS=" -I$withval/xercesc "
	 XERCESC_LIBS=" -L$withval/lib -lxerces-c " 
	 AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	XERCESC_CFLAGS=" -I/usr/include/xercesc "
	XERCESC_LIBS="  -lxerces-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	XERCESC_CFLAGS=" -I/usr/local/include/xercesc "
	XERCESC_LIBS=" -L/usr/local/lib -lxerces-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


if test x"$have_package" = xyes; then
	have_xml4c=yes
	AC_DEFINE( HAVE_XERCESC, 1, [Is Xerces-C installed])
	AC_DEFINE( HAVE_XML4C, 1, [Is Xerces-C installed])

	echo "Found an xerces-c that meets required needs..."
	echo "  XERCESC_CFLAGS: $XERCESC_CFLAGS "
	echo "  XERCESC_LIBS:   $XERCESC_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , 
	[
		have_xml4c=no
		echo ""
		echo "explicit version ($version) of $package required. "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(XERCESC_CFLAGS)
AC_SUBST(XERCESC_LIBS)

AM_CONDITIONAL(HAVE_XML4C, test x"$have_xml4c" = xyes)
XML4C_CFLAGS=$XERCESC_CFLAGS
XML4C_LIBS=$XERCESC_LIBS
AC_SUBST(XML4C_CFLAGS)
AC_SUBST(XML4C_LIBS)

])


dnl AM_FERRIS_XERCESC([EXACT-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate xerces-c for installation. 
dnl ie. default is to REQUIRE xerces-c EXACT-VERSION or stop running.
dnl
dnl XERCESC_CFLAGS and XERCESC_LIBS are set and AC_SUBST()ed when library is found.
dnl XML4C_CFLAGS   and XML4C_LIBS   are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_XML4C) and AC_DEFINE(HAVE_XERCESC)
dnl
AC_DEFUN(AM_FERRIS_XERCESC,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
have_package=no
required_version=$1
have_xml4c=no

package=xerces-c
version=$required_version
PKG_CHECK_MODULES(XERCESC, $package = $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(XERCES-C, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

INCLUDES="$(cat <<-HEREDOC
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <iostream> 
XERCES_CPP_NAMESPACE_USE
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
    // Initialize the XML4C2 system.
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException& toCatch)
    {
        char *pMsg = XMLString::transcode(toCatch.getMessage());
        std::cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << pMsg;
        XMLString::release(&pMsg);
        return 1;
    }

    
    if( XERCES_VERSION_MAJOR != 2 && XERCES_VERSION_MINOR != 2 )
    {
        return 1;
    }
HEREDOC
)"

if test x"$have_package" = xno; then
AC_ARG_WITH(xercesc,
        [  --with-xercesc=DIR          use xercesc $version install rooted at <DIR>],
        [XERCESC_CFLAGS=" -I$withval/xercesc "
	 XERCESC_LIBS=" -L$withval/lib -lxerces-c " 
	 AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	XERCESC_CFLAGS=" -I/usr/include/xercesc-2.7.0/ -I/usr/include/xercesc-2.7.0/xercesc/ "
	XERCESC_LIBS=" -L/usr/lib64/xerces-c-2.7.0 -L/usr/lib/xerces-c-2.7.0   -lxerces-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


# try to hit it directly.
if test x"$have_package" = xno; then
	XERCESC_CFLAGS=" -I/usr/include/xercesc "
	XERCESC_LIBS="  -lxerces-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	XERCESC_CFLAGS=" -I/usr/local/include/xercesc "
	XERCESC_LIBS=" -L/usr/local/lib -lxerces-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XERCESC_CFLAGS], [$XERCESC_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


if test x"$have_package" = xyes; then
	have_xml4c=yes
	AC_DEFINE( HAVE_XERCESC, 1, [Is Xerces-C installed])
	AC_DEFINE( HAVE_XML4C, 1, [Is Xerces-C installed])

	echo "Found an xerces-c that meets required needs..."
	echo "  XERCESC_CFLAGS: $XERCESC_CFLAGS "
	echo "  XERCESC_LIBS:   $XERCESC_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , 
	[
		have_xml4c=no
		echo ""
		echo "explicit version ($version) of $package required. "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(XERCESC_CFLAGS)
AC_SUBST(XERCESC_LIBS)

AM_CONDITIONAL(HAVE_XML4C, test x"$have_xml4c" = xyes)
XML4C_CFLAGS=$XERCESC_CFLAGS
XML4C_LIBS=$XERCESC_LIBS
AC_SUBST(XML4C_CFLAGS)
AC_SUBST(XML4C_LIBS)

])


dnl AM_FERRIS_XALAN([EXACT-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate xerces-c for installation. 
dnl ie. default is to REQUIRE xerces-c EXACT-VERSION or stop running.
dnl
dnl XALAN_CFLAGS and XALAN_LIBS are set and AC_SUBST()ed when library is found.
dnl XML4C_CFLAGS   and XML4C_LIBS   are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_XML4C) and AC_DEFINE(HAVE_XALAN)
dnl
AC_DEFUN(AM_FERRIS_XALAN,
[dnl 
dnl Get the cflags and libraries from pkg-config, stlport-config or attempt to
dnl detect the STLPort on the users system.
dnl
required_version=$1
have_xalan=no

package=xalan-c
version=$required_version

AC_ARG_ENABLE(xalanc,
  [AS_HELP_STRING([--enable-xalanc],
                  [enable xalanc support (default=auto)])],[],[enable_xalanc=check])

if test x$enable_xalanc != xno; then


PKG_CHECK_MODULES(XALAN, $package >= $version, [ have_xalan=yes ],  [ have_xalan=no ] )

INCLUDES="$(cat <<-HEREDOC
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <Include/XalanVersion.hpp>
#include <iostream> 
XERCES_CPP_NAMESPACE_USE
using namespace std;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
    // Initialize the XML4C2 system. v3
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException& toCatch)
    {
        char *pMsg = XMLString::transcode(toCatch.getMessage());
        cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << pMsg;
        XMLString::release(&pMsg);
        return 1;
    }

    
    if( XALAN_VERSION_MAJOR != 1 && XALAN_VERSION_MINOR != 5 )
    {
        return 1;
    }
HEREDOC
)"


if test x"$have_xalan" = xno; then
AC_ARG_WITH(xalan,
        [  --with-xalan=DIR          use xalan $version install rooted at <DIR>],
        [XALAN_CFLAGS=" $XERCESC_CFLAGS -I$withval/xalanc -I/usr/include/xercesc -DFedora13Xerces "
	 XALAN_LIBS=" $XERCESC_LIBS -L$withval/lib -lxalan-c " 
	 AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_xalan=yes], [have_xalan=no] )
	])
fi


if test x"$have_xalan" = xno; then
AC_ARG_WITH(xalan,
        [  --with-xalan=DIR          use xalan $version install rooted at <DIR>],
        [XALAN_CFLAGS=" $XERCESC_CFLAGS -I$withval/xalan-c "
	 XALAN_LIBS=" $XERCESC_LIBS -L$withval/lib -lxalan-c " 
	 AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_xalan=yes], [have_xalan=no] )
	])
fi

if test x"$have_xalan" = xno; then
AC_ARG_WITH(xalan,
        [  --with-xalan=DIR          use xalan $version install rooted at <DIR>],
        [XALAN_CFLAGS=" $XERCESC_CFLAGS -I$withval/xalanc "
	 XALAN_LIBS=" $XERCESC_LIBS -L$withval/lib -lxalan-c " 
	 AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_xalan=yes], [have_xalan=no] )
	])
fi


# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/include/xalan-c1.8 "
	XALAN_LIBS=" $XERCESC_LIBS  -lxalan-c1_8_0 "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi
# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/local/include/xalan-c1.8 "
	XALAN_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lxalan-c1_8_0 "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi
# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/include/xalan-c "
	XALAN_LIBS=" $XERCESC_LIBS  -lxalan-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi
# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/local/include/xalan-c "
	XALAN_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lxalan-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi
# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/include/xalanc "
	XALAN_LIBS=" $XERCESC_LIBS  -lxalan-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi
# try to hit it directly.
if test x"$have_xalan" = xno; then
	XALAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/local/include/xalanc "
	XALAN_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lxalan-c "
	AM_FERRIS_INTERNAL_TRYRUN( [$XALAN_CFLAGS], [$XALAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_xalan=yes], [have_xalan=no] )
fi

fi

if test x"$have_xalan" = xyes; then

	have_xalan=yes
	AC_DEFINE( HAVE_XALAN, 1, [Is Xalan-C installed] )

	echo "Found an xalan-c that meets required needs..."
	echo "  XALAN_CFLAGS: $XALAN_CFLAGS "
	echo "  XALAN_LIBS:   $XALAN_LIBS "

	# success
	ifelse([$2], , :, [$2])

else
	XALAN_CFLAGS=
	XALAN_LIBS=
	ifelse([$3], , 
	[
		echo ""
		echo "version ($version) or later of $package required. "
		echo ""
		echo "config.log"
		cat config.log
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi


AM_CONDITIONAL(HAVE_XALAN, test x"$have_xalan" = xyes)
AC_SUBST(XALAN_CFLAGS)
AC_SUBST(XALAN_LIBS)

])


dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################


dnl AM_FERRIS_DTL([EXACT-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to put a message for the user to see that
dnl this module was not found and thus code written for it is not being compiled in.
dnl
dnl DTL_CFLAGS and DTL_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_DTL) and AM_CONDITIONAL(HAVE_DTL)
dnl
AC_DEFUN([AM_FERRIS_DTL],
[{
dnl

required_version=$1
have_dtl=no
package=dtl
version=$required_version

AC_ARG_ENABLE(dtl,
  [AS_HELP_STRING([--enable-dtl],
                  [enable dtl support (default=auto)])],[],[enable_dtl=check])
if test x$enable_dtl != xno; then

	PKG_CHECK_MODULES(DTL, $package >= $version, [ have_dtl=yes ],  [ have_dtl=no ] )

fi


if test x"$have_dtl" = xyes; then

	have_dtl=yes
	AC_DEFINE( HAVE_DTL, 1, [Is DTL installed] )

	echo "Found a DTL ODBC library that meets required needs..."
	echo "  DTL_CFLAGS: $DTL_CFLAGS "
	echo "  DTL_LIBS:   $DTL_LIBS "

	# success
	ifelse([$2], , :, [$2])

else
	ifelse([$3], , 
	[
	echo "Support for DTL version ($version) not being built... "
	], 
	[$3])     
fi

AM_CONDITIONAL(HAVE_DTL, test x"$have_dtl" = xyes)
AC_SUBST(DTL_CFLAGS)
AC_SUBST(DTL_LIBS)
}])

dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################


AC_DEFUN(AM_FERRIS_BOOST_INTERNAL_TRYLINK,
[dnl 
dnl
	AC_LANG_CPLUSPLUS
	CXXFLAGS_cache=$CXXFLAGS
	CXXFLAGS=" $CXXFLAGS $STLPORT_CFLAGS $BOOST_CFLAGS "
	LDFLAGS_cache=$LDFLAGS
	LDFLAGS=" $LDFLAGS $STLPORT_LIBS $BOOST_LIBS "

	AM_FERRIS_LIBTOOL_TRYLINK([

#define BOOST_SPIRIT_USE_OLD_NAMESPACE

		#include <fstream>

		#include <boost/spirit/include/classic.hpp>
		using namespace boost::spirit;
		using namespace boost;

		#include <boost/archive/text_oarchive.hpp>
		#include <boost/archive/text_iarchive.hpp>
		#include <boost/archive/binary_oarchive.hpp>
		#include <boost/archive/binary_iarchive.hpp>
		#include <boost/serialization/list.hpp>
		#include <boost/serialization/set.hpp>
		#include <boost/serialization/map.hpp>
		],
		[
		rule<>  r = real_p >> *(ch_p(',') >> real_p);

	            std::ifstream ifs( "/tmp/doesnt-matter-no-exist" );
        	    boost::archive::binary_iarchive archive( ifs );

		],
	       	[ have_boost=yes; ],
	       	[ have_boost=no; ] )

	LDFLAGS=$LDFLAGS_cache
	CXXFLAGS=$CXXFLAGS_cache
	AC_LANG_C
])

dnl AM_FERRIS_BOOST([MIN-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to put a message for the user to see that
dnl this module was not found and thus code written for it is not being compiled in.
dnl
dnl BOOST_CFLAGS and BOOST_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_BOOST) and AM_CONDITIONAL(HAVE_BOOST)
dnl
AC_DEFUN(AM_FERRIS_BOOST,
[dnl 
dnl
required_version=$1
have_boost=no

package=boost
version=$required_version

AC_LANG_CPLUSPLUS

BOOST_CFLAGS=""
BOOST_LIBS=" -lboost_wserialization -lboost_serialization -lboost_regex "
AM_FERRIS_BOOST_INTERNAL_TRYLINK

if test x"$have_boost" = xno; then
	BOOST_CFLAGS=" -I/usr/local/include "
	BOOST_LIBS=" -L/usr/local/lib -lboost_system -lboost_wserialization -lboost_serialization -lboost_regex "
	AM_FERRIS_BOOST_INTERNAL_TRYLINK
fi

if test x"$have_boost" = xno; then
	if test "x$HAVE_STLPORT"="xy"; then
		BOOST_CFLAGS=" $STLPORT_CFLAGS "
		BOOST_LIBS=" $STLPORT_LIBS -lboost_system-gcc-p -lboost_wserialization-gcc-p  -lboost_serialization-gcc-p "
		AM_FERRIS_BOOST_INTERNAL_TRYLINK
	fi
fi

if test x"$have_boost" = xno; then
	if test "x$HAVE_STLPORT"="xy"; then
		BOOST_CFLAGS=" $STLPORT_CFLAGS "
		BOOST_LIBS=" $STLPORT_LIBS -lboost_system-mt -lboost_wserialization-mt  -lboost_serialization-mt -lboost_regex-mt "
		AM_FERRIS_BOOST_INTERNAL_TRYLINK
	fi
fi

if test x"$have_boost" = xyes; then

	have_boost=yes
	AC_DEFINE( HAVE_BOOST, 1,[is the boost library installed] )

	echo "Found a BOOST library that meets required needs..."
	echo "  BOOST_CFLAGS : $BOOST_CFLAGS "
	echo "  BOOST_LIBS   : $BOOST_LIBS "

	# success
	ifelse([$2], , :, [$2])

else
	ifelse([$3], , 
	[
	echo "Support for BOOST version ($version) not being built... "
	], 
	[$3])     
fi

AC_LANG_C
AM_CONDITIONAL(HAVE_BOOST, test x"$have_boost" = xyes)
AC_SUBST(BOOST_CFLAGS)
AC_SUBST(BOOST_LIBS)
])

AC_DEFUN(AM_FERRIS_BOOST_NEEDED,
[
  TESTING_FEATURE="Boost C++ library";
  AM_FERRIS_BOOST( 1.33.1,
          [ echo "Found boost library ..."; ],
          [ AC_MSG_ERROR([ERROR: boost 1.33.1 (or 1.34.1 or maybe later) is required]); exit; ] )
])			 


dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################

dnl 1.2.x test
dnl AM_FERRIS_PATHAN([VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libpathan for installation. 
dnl ie. default is to REQUIRE atleast libpathan VERSION or stop running.
dnl
dnl PATHAN_CFLAGS and PATHAN_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_PATHAN)
dnl
AC_DEFUN(AM_FERRIS_PATHAN,
[dnl 
dnl
have_package=no
required_version=$1
have_pathan=no

package=pathan
version=$required_version
dnl PKG_CHECK_MODULES(PATHAN, $package = $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(PATHAN, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

INCLUDES="$(cat <<-HEREDOC
#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <pathan/Pathan.hpp>
#include <pathan/XPathEvaluator.hpp>
XERCES_CPP_NAMESPACE_USE
using namespace std;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
  // Standard Xerces-C initalisation code

  try {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) {
    cerr << "Error during Xerces-c Initialization.\n"
	 << "Exception message:"
	 << XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage()) << endl;
    return 1;
  }

  XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser *xmlParser = new XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser();

  //Parse data.xml into a DOM tree

  xmlParser->setDoNamespaces(true);
  xmlParser->parse("data.xml");

  // Retreive the parsed document

  const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *document = xmlParser->getDocument();
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *documentElement = document->getDocumentElement();

  // Create an XPathEvaluator class

  // This class is used as a factory for creating XPathExpression and
  // XPathNSResolver (it is rarely used for evaluation in the latest
  // spec [31/10/2001], however, and is somewhat misnamed)

  XPathEvaluator *evaluator = XPathEvaluator::createEvaluator();
HEREDOC
)"


if test x"$have_package" = xno; then
AC_ARG_WITH(pathan,
        [  --with-pathan=DIR          use pathan $version install rooted at <DIR>],
        [PATHAN_CFLAGS=" $XERCESC_CFLAGS -I$withval/pathan "
	 PATHAN_LIBS=" $XERCESC_LIBS -L$withval/lib -lpathan " 
	 AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN_CFLAGS], [$PATHAN_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	PATHAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/include/pathan "
	PATHAN_LIBS=" $XERCESC_LIBS  -lpathan "
	AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN_CFLAGS], [$PATHAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	PATHAN_CFLAGS=" $XERCESC_CFLAGS -I/usr/local/include/pathan "
	PATHAN_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lpathan "
	AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN_CFLAGS], [$PATHAN_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


if test x"$have_package" = xyes; then
	have_pathan=yes
	AC_DEFINE( HAVE_PATHAN,1,[is libpathan installed] )

	echo "Found an pathan that meets required needs..."
	echo "  PATHAN_CFLAGS: $PATHAN_CFLAGS "
	echo "  PATHAN_LIBS:   $PATHAN_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	PATHAN_CFLAGS=
	PATHAN_LIBS=
	ifelse([$3], , 
	[
		have_pathan=no
		echo ""
		echo "version ($version) or later of $package required. "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(PATHAN_CFLAGS)
AC_SUBST(PATHAN_LIBS)

AM_CONDITIONAL(HAVE_PATHAN, test x"$have_pathan" = xyes)

])

dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################

dnl 2.0.x test
dnl AM_FERRIS_PATHAN2([VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libpathan for installation. 
dnl ie. default is to REQUIRE atleast libpathan VERSION or stop running.
dnl
dnl PATHAN2_CFLAGS and PATHAN2_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_PATHAN2)
dnl
AC_DEFUN(AM_FERRIS_PATHAN2,
[dnl 
dnl
have_package=no
required_version=$1
have_pathan2=no

package=pathan2
version=$required_version
dnl PKG_CHECK_MODULES(PATHAN2, $package = $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(PATHAN2, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

INCLUDES="$(cat <<-HEREDOC
#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <pathan/Pathan.hpp>
#include <pathan/PathanEngine.hpp>
#include <include/pathan/internal/dom-extensions/PathanExpressionImpl.hpp>
XERCES_CPP_NAMESPACE_USE
using namespace std;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
  // Standard Xerces-C initalisation code

  try {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) {
    cerr << "Error during Xerces-c Initialization.\n"
	 << "Exception message:"
	 << XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage()) << endl;
    return 1;
  }

  XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser *xmlParser = new XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser();

  //Parse data.xml into a DOM tree

  xmlParser->setDoNamespaces(true);
  xmlParser->parse("data.xml");

  // Retreive the parsed document

  const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *document = xmlParser->getDocument();
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *documentElement = document->getDocumentElement();

  // Create an XPathEvaluator class

  // This class is used as a factory for creating XPathExpression and
  // XPathNSResolver (it is rarely used for evaluation in the latest
  // spec [31/10/2001], however, and is somewhat misnamed)

  XPath2MemoryManager* mm = PathanEngine::createMemoryManager();
  PathanNSResolver* res = PathanEngine::createNSResolver( documentElement, mm );

HEREDOC
)"

AC_ARG_WITH(pathan2-source,
        [  --with-pathan2-source=DIR          use pathan2 source code tree $version rooted at <DIR>],
        [PATHAN2_SOURCEDIR_CFLAGS=" -I$withval "
	 PATHAN2_SOURCEDIR="$withval" 
	])


if test x"$have_package" = xno; then
AC_ARG_WITH(pathan2,
        [  --with-pathan2=DIR          use pathan2 $version install rooted at <DIR>],
        [PATHAN2_CFLAGS=" $XERCESC_CFLAGS $PATHAN2_SOURCEDIR_CFLAGS -I$withval/pathan "
	 PATHAN2_LIBS=" $XERCESC_LIBS -L$withval/lib -lpathan " 
	 AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN2_CFLAGS], [$PATHAN2_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	PATHAN2_CFLAGS=" $XERCESC_CFLAGS $PATHAN2_SOURCEDIR_CFLAGS -I/usr/include/pathan "
	PATHAN2_LIBS=" $XERCESC_LIBS  -lpathan "
	AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN2_CFLAGS], [$PATHAN2_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	PATHAN2_CFLAGS=" $XERCESC_CFLAGS $PATHAN2_SOURCEDIR_CFLAGS -I/usr/local/include/pathan "
	PATHAN2_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lpathan "
	AM_FERRIS_INTERNAL_TRYLINK( [$PATHAN2_CFLAGS], [$PATHAN2_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


if test x"$have_package" = xyes; then
	have_pathan2=yes
	AC_DEFINE( HAVE_PATHAN2,1,[is libpathan installed] )

	echo "Found a pathan2 that meets required needs..."
	echo "  PATHAN2_CFLAGS: $PATHAN2_CFLAGS "
	echo "  PATHAN2_LIBS:   $PATHAN2_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	PATHAN2_CFLAGS=
	PATHAN2_LIBS=
	ifelse([$3], , 
	[
		have_pathan2=no
		echo ""
		echo "version ($version) or later of $package required. "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(PATHAN2_CFLAGS)
AC_SUBST(PATHAN2_LIBS)

AM_CONDITIONAL(HAVE_PATHAN2, test x"$have_pathan2" = xyes)

])

dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################

dnl AM_FERRIS_XQILLA([VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libxqilla for installation. 
dnl ie. default is to REQUIRE atleast libxqilla VERSION or stop running.
dnl
dnl XQILLA_CFLAGS and XQILLA_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_XQILLA)
dnl
AC_DEFUN(AM_FERRIS_XQILLA,
[dnl 
dnl
have_package=no
required_version=$1
have_xqilla=no

package=xqilla
version=$required_version
dnl PKG_CHECK_MODULES(XQILLA, $package = $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(XQILLA, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

AC_ARG_ENABLE(xqilla,
  [AS_HELP_STRING([--enable-xqilla],
                  [enable xqilla support (default=auto)])],[],[enable_xqilla=check])

if test x$enable_xqilla != xno; then


INCLUDES="$(cat <<-HEREDOC
#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xqilla/xqilla-dom3.hpp>

XERCES_CPP_NAMESPACE_USE;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC

    // Initialise Xerces-C and XQilla using XQillaPlatformUtils
    XQillaPlatformUtils::initialize();

HEREDOC
)"

AC_ARG_WITH(xqilla-source,
        [  --with-xqilla-source=DIR          use xqilla source code tree $version rooted at <DIR>],
        [XQILLA_SOURCEDIR_CFLAGS=" -I$withval "
	 XQILLA_SOURCEDIR="$withval" 
	])


if test x"$have_package" = xno; then
AC_ARG_WITH(xqilla,
        [  --with-xqilla=DIR          use xqilla $version install rooted at <DIR>],
        [XQILLA_CFLAGS=" $XERCESC_CFLAGS $XQILLA_SOURCEDIR_CFLAGS -I$withval/xqilla "
	 XQILLA_LIBS=" $XERCESC_LIBS -L$withval/lib -lxqilla " 
	 AM_FERRIS_INTERNAL_TRYLINK( [$XQILLA_CFLAGS], [$XQILLA_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	XQILLA_CFLAGS=" $XERCESC_CFLAGS $XQILLA_SOURCEDIR_CFLAGS -I/usr/include/xqilla "
	XQILLA_LIBS=" $XERCESC_LIBS  -lxqilla "
	AM_FERRIS_INTERNAL_TRYLINK( [$XQILLA_CFLAGS], [$XQILLA_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	XQILLA_CFLAGS=" $XERCESC_CFLAGS $XQILLA_SOURCEDIR_CFLAGS -I/usr/local/include/xqilla "
	XQILLA_LIBS=" $XERCESC_LIBS -L/usr/local/lib -lxqilla "
	AM_FERRIS_INTERNAL_TRYLINK( [$XQILLA_CFLAGS], [$XQILLA_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

fi

if test x"$have_package" = xyes; then
	have_xqilla=yes
	AC_DEFINE( HAVE_XQILLA,1,[is libxqilla installed] )

	echo "Found a xqilla that meets required needs..."
	echo "  XQILLA_CFLAGS: $XQILLA_CFLAGS "
	echo "  XQILLA_LIBS:   $XQILLA_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	XQILLA_CFLAGS=
	XQILLA_LIBS=
	ifelse([$3], , 
	[
		have_xqilla=no
		echo ""
		echo "version ($version) or later of $package required. "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(XQILLA_CFLAGS)
AC_SUBST(XQILLA_LIBS)

AM_CONDITIONAL(HAVE_XQILLA, test x"$have_xqilla" = xyes)

])


dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################

dnl AM_FERRIS_SOCKETPP([VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to just print a didn't find optional pkg msg.
dnl
dnl SOCKETPP_CFLAGS and SOCKETPP_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_SOCKETPP)
dnl AM_CONDITIONAL(HAVE_SOCKETPP
dnl
AC_DEFUN(AM_FERRIS_SOCKETPP,
[dnl 
dnl
have_package=no
required_version=$1
have_socketpp=no

package=socketpp
version=$required_version
dnl PKG_CHECK_MODULES(SOCKETPP, $package = $version, [ have_package=yes ], [ have_package=no ] )
dnl if test x"$have_package" = xno; then
dnl 	AC_PATH_GENERIC(SOCKETPP, $version, [ have_package=yes ], [ have_package=no ] )
dnl fi

INCLUDES="$(cat <<-HEREDOC
#include <socket++/sockinet.h>
#include <socket++/sockstream.h>
#include <socket++/sockunix.h>
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
    iosockinet* sock_ss = 0;
    sock_ss = new iosockinet( sockbuf::sock_stream );
    (*sock_ss)->connect( "localhost", 80 );
HEREDOC
)"


if test x"$have_package" = xno; then
AC_ARG_WITH(socketpp,
        [  --with-socketpp=DIR          use socketpp $version install rooted at <DIR>],
        [SOCKETPP_CFLAGS="  -I$withval/include "
	 SOCKETPP_LIBS="  -L$withval/lib -lsocket++ " 
	 AM_FERRIS_INTERNAL_TRYLINK( [$SOCKETPP_CFLAGS], [$SOCKETPP_LIBS], 
					[ $INCLUDES ], [$PROGRAM],
					[have_package=yes], [have_package=no] )
	])
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	SOCKETPP_CFLAGS="  -I/usr/include "
	SOCKETPP_LIBS="   -lsocket++ "
	AM_FERRIS_INTERNAL_TRYLINK( [$SOCKETPP_CFLAGS], [$SOCKETPP_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly.
if test x"$have_package" = xno; then
	SOCKETPP_CFLAGS="  -I/usr/local/include "
	SOCKETPP_LIBS="  -L/usr/local/lib -lsocket++ "
	AM_FERRIS_INTERNAL_TRYLINK( [$SOCKETPP_CFLAGS], [$SOCKETPP_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi


if test x"$have_package" = xyes; then
	have_socketpp=yes
	AC_DEFINE( HAVE_SOCKETPP, 1, [Is socket++ installed] )

	echo "Found a socketpp that meets required needs..."
	echo "  SOCKETPP_CFLAGS: $SOCKETPP_CFLAGS "
	echo "  SOCKETPP_LIBS:   $SOCKETPP_LIBS "

	# success
	ifelse([$2], , :, [$2])
else
	ifelse([$3], , 
	[
		have_socketpp=no
		echo "[optional] Didn't find a socketpp that meets required needs..."
dnl 		echo ""
dnl 		echo "version ($version) or later of $package required. "
dnl 		echo ""
dnl 		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
fi

AC_SUBST(SOCKETPP_CFLAGS)
AC_SUBST(SOCKETPP_LIBS)

AM_CONDITIONAL(HAVE_SOCKETPP, test x"$have_socketpp" = xyes)

])





dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_PQXX([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libpqxx for installation. 
dnl ie. default is to REQUIRE pqxx MINIMUM-VERSION or stop running.
dnl
dnl LIBPQXX_CFLAGS and LIBPQXX_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_PQXX,
[dnl 
dnl
required_version=$1

have_libpqxx=no
package=libpqxx
version=$required_version

AC_ARG_ENABLE(pqxx,
  [AS_HELP_STRING([--enable-pqxx],
                  [enable pqxx support (default=auto)])],[],[enable_pqxx=check])

if test x$enable_pqxx != xno; then

	PKG_CHECK_MODULES(LIBPQXX, $package >= $version, [ 
	   have_libpqxx_pkgconfig=yes
	   have_libpqxx=yes 
	],[
	   have_libpqxx_pkgconfig=no
	   have_libpqxx=maybe
	])


INCLUDES="$(cat <<-HEREDOC
	#include <stdlib.h>

	#include <pqxx/connection>
	#include <pqxx/tablewriter>
	#include <pqxx/transaction>
	#include <pqxx/nontransaction>
	#include <pqxx/tablereader>
	#include <pqxx/tablewriter>

	using namespace PGSTD;
	using namespace pqxx;

	#include <string>
	using namespace std;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
	    string constring;
	    connection c( constring );
HEREDOC
)"

CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS
AC_LANG_CPLUSPLUS
have_package=no

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBPQXX_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBPQXX_CFLAGS $LIBPQXX_CXXFLAGS "
	LIBPQXX_LIBS=" $STLPORT_LIBS $LDFLAGS $LIBPQXX_LIBS "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBPQXX_CFLAGS], [$LIBPQXX_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBPQXX_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBPQXX_CFLAGS $LIBPQXX_CXXFLAGS -I/usr/include/pqxx "
	LIBPQXX_LIBS=" $STLPORT_LIBS $LDFLAGS $LIBPQXX_LIBS "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBPQXX_CFLAGS], [$LIBPQXX_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBPQXX_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBPQXX_CFLAGS $LIBPQXX_CXXFLAGS -I/usr/local/include/pqxx "
	LIBPQXX_LIBS=" $STLPORT_LIBS $LDFLAGS $LIBPQXX_LIBS -L/usr/local/lib "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBPQXX_CFLAGS], [$LIBPQXX_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

AC_LANG_C
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

fi

#####################################################


have_libpqxx=no;

if test x"$have_package" = xyes; then
	have_libpqxx=yes;
	AC_DEFINE( HAVE_LIBPQXX, 1, [Is libpqxx installed] )

	# success
	ifelse([$2], , :, [$2])

else
	if test x$have_libpqxx_pkgconfig = xyes; then
		echo "pkg-config could find your libpqxx but can't compile and link against it..." 
	fi

	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://pqxx.tk/"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	LIBPQXX_CFLAGS=" "
	LIBPQXX_LIBS=" "
fi

AM_CONDITIONAL(HAVE_LIBPQXX, test x"$have_libpqxx" = xyes)
AC_SUBST(LIBPQXX_CFLAGS)
AC_SUBST(LIBPQXX_LIBS)
])



dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_KDE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl currently seeks KDE4 instead of KDE3, changed at March 2009.
dnl
dnl a CONDITIONAL FERRIS_HAVE_KDE is defined and the shell var have_kde is either yes/no on exit
dnl KDE_CFLAGS and KDE_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_KDE,
[dnl 
dnl
required_version=$1
have_kde3=no


AC_ARG_ENABLE(kde-detection,
[--disable-kde-detection            Don't try to find KDE or QT],
[
  if test x$enableval = xyes; then
	attempt_to_find_kde=yes
  else
	attempt_to_find_kde=no
  fi
])

if test x"$attempt_to_find_kde" = xno; then
	FERRIS_HAVE_KDE=no
	KDE_CFLAGS=""
	KDE_LIBS=""
    	AC_MSG_RESULT([compilation of KDE/Qt functions disabled])
else
  if test x"$have_tested_for_kde" = x; then
	AC_LANG_CPLUSPLUS
dnl	AC_ARG_WITH(qt, [  --with-qt               build with Qt utils. [autodetected]],,with_qt=yes)
dnl	if test x$with_qt = xyes ; then

		PKG_CHECK_MODULES(QTCORE, QtCore >= 4.4.3, [ have_qtcore=yes ],  [ have_qtcore=no ] )

		QT_CFLAGS=" $QTCORE_CFLAGS -I/usr/include/Qt "
		QT_LIBS="   $QTCORE_LIBS "
		QT_CXXFLAGS="$QT_CFLAGS"
		QT_LDADD="$QT_LIBS"

		echo "QT_CFLAGS: $QT_CFLAGS"
		echo "QT_LIBS: $QT_LIBS"

		KDE_INCLUDEDIR="`kde4-config --path include` "
		KDE_LIBDIR="`kde4-config --prefix`/lib`kde4-config --libsuffix`/kde4/devel"
		AC_ARG_WITH(kde-includedir,
	        [  --with-kde-includedir=DIR          root directory containing KDE include files],
	        	[KDE_INCLUDEDIR=" -I$withval "
		])
		AC_ARG_WITH(kde-libdir,
	        [  --with-kde-libdir=DIR          directory continaing KDE libs],
	        	[KDE_LIBDIR=" -I$withval "
		])

		KDE4_LINK_CFLAGS_PREFIX=" -L/usr/lib64/kde4/devel "
		echo "Have KDE_LIBDIR:$KDE_LIBDIR"
		KDELIB_MINUS_L_OPTION=" -L$KDE_LIBDIR "
		if test x"$KDE_LIBDIR" = "x/usr/lib"; then
			echo "Standard KDE Libdir..."
			KDELIB_MINUS_L_OPTION=" "
			KDE4_LINK_CFLAGS_PREFIX=" -L/usr/lib/kde4/devel "
		fi
		if test x"$KDE_LIBDIR" = "x/usr/lib64"; then
			echo "Standard KDE Libdir..."
			KDELIB_MINUS_L_OPTION=" "
		fi

		KDE_CFLAGS=" $KDE_CFLAGS -I$KDE_INCLUDEDIR $QT_CFLAGS "
		KDE_LIBS=" $KDE_LIBS  $KDELIB_MINUS_L_OPTION -lkdeui -lkdecore $QT_LIBS "

		CXXFLAGS_cache=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $KDE_CFLAGS"
		LDFLAGS_cache=$LDFLAGS
		LDFLAGS="$LDFLAGS $KDE_LIBS"

		echo "trying to link a KDE client..."

		AC_TRY_LINK([
		#include <iostream>
		#include <qapplication.h>
		#include <kmimetype.h>
		#include <kdebug.h>
		#include <kapplication.h>

		using namespace std;
        	],
		[
		KApplication a( false );
    
                KUrl u;
                u.setPath( "/tmp" );
                KMimeType::Ptr type = KMimeType::findByUrl( u );
		cerr << type->name().toUtf8().data() << endl;
		return 0;
		],
       		[have_kde=yes], [have_kde=no])

		LDFLAGS=$LDFLAGS_cache
		CXXFLAGS=$CXXFLAGS_cache

		if test x"$have_kde" = xyes; then
			echo "Building kde support funtions"
			MIMETYPE_ENGINE_DESC="KDE"
			MIMETYPE_ENGINE_CHOSEN=yes
			FERRIS_HAVE_KDE=yes
			AC_DEFINE(HAVE_KDE,1,[])
			AC_DEFINE(FERRIS_HAVE_KDE,1,[])
		else
			echo "Couldn't link sample KDE4 application, disabling KDE support"
			FERRIS_HAVE_KDE=no
			KDE_CFLAGS=""
			KDE_LIBS=""
		fi
	else
		echo "with_qt was not set...with_qt:$with_qt"
		FERRIS_HAVE_KDE=no
		KDE_CFLAGS=""
		KDE_LIBS=""
	    	AC_MSG_RESULT([compilation of Qt functions disabled])
	fi

	AC_LANG_C
	AC_SUBST(KDE_CFLAGS)
	AC_SUBST(KDE_LIBS)
	AC_SUBST(QT_CFLAGS)
	AC_SUBST(QT_LIBS)
	AC_SUBST(QT_CXXFLAGS)
	AC_SUBST(QT_LDADD)
dnl  fi
fi

have_tested_for_kde=yes

AM_CONDITIONAL(FERRIS_HAVE_KDE, test x"$have_kde" = xyes)
])

	
# --libsuffix == 64
dnl  -I/usr/include/kde4 -L/usr/lib64/kde4/devel  -lkdecore  -lkdeui
dnl AC_SUBST(QT_CXXFLAGS)
dnl AC_SUBST(QT_LDADD)
dnl AC_SUBST(QT_GUILINK)


dnl if test x"$attempt_to_find_kde" = xno; then
dnl 	FERRIS_HAVE_KDE3=no
dnl 	KDE3_CFLAGS=""
dnl 	KDE3_LIBS=""
dnl     	AC_MSG_RESULT([compilation of Qt functions disabled])
dnl else
dnl   if test x"$have_tested_for_kde3" = x; then

dnl 	AC_LANG_CPLUSPLUS
dnl 	AC_ARG_WITH(qt, [  --with-qt               build with Qt utils. [autodetected]],,with_qt=yes)
dnl 	if test x$with_qt = xyes ; then

dnl 		gw_CHECK_QT
dnl 		QT_CFLAGS=" $QT_CXXFLAGS "
dnl 		QT_LIBS="   $QT_LDADD "
dnl dnl 		QT_CFLAGS=" `pkg-config --cflags  qt3 ` "
dnl dnl 		QT_LIBS="   `pkg-config --libs    qt3 ` "

dnl 		AC_DEFINE(QT_THREAD_SUPPORT)

dnl dnl 		AC_PATH_KDE
dnl dnl 		KDE3_CFLAGS=" $KDE_INCLUDES $QT_CFLAGS "
dnl dnl 		KDE3_LIBS="   $KDE_LDFLAGS $QT_LIBS "

dnl dnl 		AC_PATH_KDE
dnl dnl 		KDE3_CFLAGS=" $KDE_INCLUDES $QT_CFLAGS "
dnl dnl 		KDE3_LIBS=" $KDE_LDFLAGS $QT_LIBS "

dnl 		KDE3_INCLUDEDIR="`kde-config --prefix`/include/kde "
dnl 		KDE3_LIBDIR="`kde-config --prefix`/lib "
dnl 		AC_ARG_WITH(kde-includedir,
dnl 	        [  --with-kde-includedir=DIR          root directory containing KDE include files],
dnl 	        	[KDE3_INCLUDEDIR=" -I$withval "
dnl 		])
dnl 		AC_ARG_WITH(kde-libdir,
dnl 	        [  --with-kde-libdir=DIR          directory continaing KDE libs],
dnl 	        	[KDE3_LIBDIR=" -I$withval "
dnl 		])

dnl 		echo "Have KDE3_LIBDIR:$KDE3_LIBDIR"
dnl 		KDE3LIB_MINUS_L_OPTION=" -L$KDE3_LIBDIR "
dnl 		if test x"$KDE3_LIBDIR" = "x/usr/lib"; then
dnl 			echo "Standard KDE3 Libdir..."
dnl 			KDE3LIB_MINUS_L_OPTION=" "
dnl 		fi
dnl 		if test x"$KDE3_LIBDIR" = "x/usr/lib "; then
dnl 			echo "Standard KDE3 Libdir..."
dnl 			KDE3LIB_MINUS_L_OPTION=" "
dnl 		fi
dnl 		KDE3_CFLAGS=" $KDE3_CFLAGS -I$KDE3_INCLUDEDIR $QT_CFLAGS "
dnl 		KDE3_LIBS=" $KDE3_LIBS  $KDE3LIB_MINUS_L_OPTION   -lkio -lkdefx -lkdeui -lkdecore -ldl $QT_LIBS "

dnl 		CXXFLAGS_cache=$CXXFLAGS
dnl 		CXXFLAGS="$CXXFLAGS $KDE3_CFLAGS"
dnl 		LDFLAGS_cache=$LDFLAGS
dnl 		LDFLAGS="$LDFLAGS $KDE3_LIBS"

dnl 		echo "trying to link a KDE3 client..."

dnl 		AC_TRY_LINK([
dnl 		#include <iostream>
dnl 		#include <qapplication.h>
dnl 		#include <kmimetype.h>
dnl 		#include <kdebug.h>
dnl 		#include <kapplication.h>

dnl 		using namespace std;
dnl         	],
dnl 		[
dnl 		KApplication a( false, false );
    
dnl 	 	KMimeType::Ptr type = KMimeType::findByURL("/tmp/a.out");
dnl 		if (type->name() == KMimeType::defaultMimeType())
dnl 	        	cerr << "Could not find out type" << endl;
dnl 		else
dnl         		cerr << "Type: " << type->name() << endl;
dnl 		a.unlock();
dnl 		return 0;
dnl 		],
dnl        		[have_kde3=yes], [have_kde3=no])

dnl 		LDFLAGS=$LDFLAGS_cache
dnl 		CXXFLAGS=$CXXFLAGS_cache

dnl 		if test x"$have_kde3" = xyes; then
dnl 			echo "Building kde support funtions"
dnl 			MIMETYPE_ENGINE_DESC="KDE 3"
dnl 			MIMETYPE_ENGINE_CHOSEN=yes
dnl 			FERRIS_HAVE_KDE3=yes
dnl 			AC_DEFINE(HAVE_KDE3)
dnl 			AC_DEFINE(FERRIS_HAVE_KDE3)
dnl 		else
dnl 			echo "Couldn't link sample KDE3 application, disabling KDE3 support"
dnl 			FERRIS_HAVE_KDE3=no
dnl 			KDE3_CFLAGS=""
dnl 			KDE3_LIBS=""
dnl 		fi
dnl 	else
dnl 		echo "with_qt was not set...with_qt:$with_qt"
dnl 		FERRIS_HAVE_KDE3=no
dnl 		KDE3_CFLAGS=""
dnl 		KDE3_LIBS=""
dnl 	    	AC_MSG_RESULT([compilation of Qt functions disabled])
dnl 	fi

dnl 	AC_LANG_C
dnl 	AC_SUBST(KDE3_CFLAGS)
dnl 	AC_SUBST(KDE3_LIBS)
dnl   fi
dnl fi

dnl have_tested_for_kde3=yes

dnl AM_CONDITIONAL(FERRIS_HAVE_KDE3, test x"$have_kde3" = xyes)
dnl ])


dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_PLASMA([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl a CONDITIONAL FERRIS_HAVE_PLASMA is defined and the shell var have_plasma is either yes/no on exit
dnl PLASMA_CFLAGS and PLASMA_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_PLASMA,
[dnl 
dnl
required_version=$1
have_plasma=no


AC_ARG_ENABLE(plasma-detection,
[--disable-plasma-detection            Don't try to find PLASMA or QT],
[
  if test x$enableval = xyes; then
	attempt_to_find_plasma=yes
  else
	attempt_to_find_plasma=no
  fi
])

if test x"$attempt_to_find_plasma" = xno; then
	FERRIS_HAVE_PLASMA=no
	PLASMA_CFLAGS=""
	PLASMA_LIBS=""
    	AC_MSG_RESULT([compilation of PLASMA/Qt functions disabled])
else
  if test x"$have_tested_for_plasma" = x; then
	AC_LANG_CPLUSPLUS

		PLASMA_INCLUDEDIR=" -I`kde4-config --prefix`/include/kde4/plasma -I`kde4-config --prefix`/include/kde4/KDE -I`kde4-config --prefix`/include/KDE "
		PLASMA_LIBDIR="`kde4-config --prefix`/lib`plasma4-config --libsuffix` "
		AC_ARG_WITH(plasma-includedir,
	        [  --with-plasma-includedir=DIR          root directory containing PLASMA include files],
	        	[PLASMA_INCLUDEDIR=" -I$withval "
		])
		AC_ARG_WITH(plasma-libdir,
	        [  --with-plasma-libdir=DIR          directory continaing PLASMA libs],
	        	[PLASMA_LIBDIR=" -I$withval "
		])

		PLASMA4_LINK_CFLAGS_PREFIX=" -L/usr/lib64/plasma4/devel "
		echo "Have PLASMA_LIBDIR:$PLASMA_LIBDIR"
		PLASMALIB_MINUS_L_OPTION=" -L$PLASMA_LIBDIR "
		if test x"$PLASMA_LIBDIR" = "x/usr/lib"; then
			echo "Standard PLASMA Libdir..."
			PLASMALIB_MINUS_L_OPTION=" "
			PLASMA4_LINK_CFLAGS_PREFIX=" -L/usr/lib/plasma4/devel "
		fi
		if test x"$PLASMA_LIBDIR" = "x/usr/lib64"; then
			echo "Standard PLASMA Libdir..."
			PLASMALIB_MINUS_L_OPTION=" "
		fi
		PLASMA_CFLAGS=" $PLASMA_CFLAGS $PLASMA_INCLUDEDIR $QT_CFLAGS $KDE_CFLAGS "
		PLASMA_LIBS=" $PLASMA_LIBS  $PLASMALIB_MINUS_L_OPTION -lplasma $QT_LIBS $KDE_LIBS "

		CXXFLAGS_cache=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $PLASMA_CFLAGS"
		LDFLAGS_cache=$LDFLAGS
		LDFLAGS="$LDFLAGS $PLASMA_LIBS"

		echo "trying to link a PLASMA client..."

		AC_TRY_LINK([
		#include <iostream>
		#include <qapplication.h>
		#include <kmimetype.h>
		#include <kapplication.h>
                #include <Plasma/Applet>

		using namespace std;
        	],
		[
		KApplication a( false );
    
                KUrl u;
                u.setPath( "/tmp" );
                KMimeType::Ptr type = KMimeType::findByUrl( u );
		cerr << type->name().toUtf8().data() << endl;
		return 0;
		],
       		[have_plasma=yes], [have_plasma=no])

		LDFLAGS=$LDFLAGS_cache
		CXXFLAGS=$CXXFLAGS_cache

		if test x"$have_plasma" = xyes; then
			echo "Building plasma support funtions"
			FERRIS_HAVE_PLASMA=yes
			AC_DEFINE(HAVE_PLASMA,1,[])
			AC_DEFINE(FERRIS_HAVE_PLASMA,1,[])
		else
			echo "Couldn't link sample PLASMA4 application, disabling PLASMA support"
			FERRIS_HAVE_PLASMA=no
			PLASMA_CFLAGS=""
			PLASMA_LIBS=""
		fi

	AC_LANG_C
	AC_SUBST(PLASMA_CFLAGS)
	AC_SUBST(PLASMA_LIBS)
  fi
fi

have_tested_for_plasma=yes
AM_CONDITIONAL(FERRIS_HAVE_PLASMA, test x"$have_plasma" = xyes)
])

dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_SANE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl a CONDITIONAL FERRIS_HAVE_SANE is defined and the shell var have_sane is either yes/no on exit
dnl SANE_CFLAGS and SANE_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_KSANE,
[dnl 
dnl
required_version=$1
have_sane=no


AC_ARG_ENABLE(sane-detection,
[--disable-sane-detection            Don't try to find SANE or libksane],
[
  if test x$enableval = xyes; then
	attempt_to_find_sane=yes
  else
	attempt_to_find_sane=no
  fi
])

if test x"$attempt_to_find_sane" = xno; then
	FERRIS_HAVE_SANE=no
	FERRIS_HAVE_KSANE=no
	SANE_CFLAGS=""
	SANE_LIBS=""
	KSANE_CFLAGS=""
	KSANE_LIBS=""
    	AC_MSG_RESULT([compilation of SANE and ksane functions disabled])
else
  if test x"$have_tested_for_sane" = x; then
	AC_LANG_CPLUSPLUS

		PKG_CHECK_MODULES(QTGUI, QtGui >= 4.4.3, [ have_qtgui=yes ],  [ have_qtgui=no ] )
		QTGUI_CFLAGS=" $QTGUI_CFLAGS -I/usr/include/Qt " 
		
		SANE_INCLUDEDIR=" -I`kde4-config --prefix`/include/kde4 "
		SANE_LIBDIR=""
		AC_ARG_WITH(sane-includedir,
	        [  --with-sane-includedir=DIR          root directory containing SANE include files],
	        	[SANE_INCLUDEDIR=" -I$withval "
		])
		AC_ARG_WITH(sane-libdir,
	        [  --with-sane-libdir=DIR          directory continaing SANE libs],
	        	[SANE_LIBDIR=" -I$withval "
		])

		echo "Have SANE_LIBDIR:$SANE_LIBDIR"
		SANELIB_MINUS_L_OPTION=" -L$SANE_LIBDIR "
		if test x"$SANE_LIBDIR" = "x/usr/lib"; then
			echo "Standard SANE Libdir..."
			SANELIB_MINUS_L_OPTION=" "
		fi
		if test x"$SANE_LIBDIR" = "x/usr/lib64"; then
			echo "Standard SANE Libdir..."
			SANELIB_MINUS_L_OPTION=" "
		fi
		SANE_CFLAGS=" $QT_CFLAGS $QTGUI_CFLAGS $KDE_CFLAGS "
		SANE_LIBS=" $QT_LIBS $QTGUI_LIBS -lksane $KDE_LIBS "

		CXXFLAGS_cache=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $SANE_CFLAGS"
		LDFLAGS_cache=$LDFLAGS
		LDFLAGS="$LDFLAGS $SANE_LIBS"

		echo "trying to link a SANE client..."

		AC_TRY_LINK([
		#include <libksane/ksane.h>
		#include <QApplication>
		#include <QMap>

		#include <iostream>
		#include <sstream>
		using namespace std;
        	],
		[
                int argc = 0;
                char** argv = 0;
                QApplication app( argc, argv );

 		KSaneIface::KSaneWidget *m_ksanew;
                m_ksanew = new KSaneIface::KSaneWidget( 0 );
 		if ( !m_ksanew->openDevice("test"))
		{
                   m_ksanew->initGetDeviceList();
                }
		return 0;
		],
       		[have_sane=yes], [have_sane=no])

		LDFLAGS=$LDFLAGS_cache
		CXXFLAGS=$CXXFLAGS_cache

		if test x"$have_sane" = xyes; then
			echo "Building sane support funtions"
			FERRIS_HAVE_SANE=yes
			AC_DEFINE(HAVE_SANE,1,[])
			AC_DEFINE(FERRIS_HAVE_SANE,1,[])
			KSANE_CFLAGS=" $SANE_CFLAGS "
			KSANE_LIBS=" $SANE_LIBS "
		else
			echo "Couldn't link sample SANE4 application, disabling SANE support"
			FERRIS_HAVE_SANE=no
			SANE_CFLAGS=""
			SANE_LIBS=""
		fi

	AC_LANG_C
	AC_SUBST(SANE_CFLAGS)
	AC_SUBST(SANE_LIBS)
	AC_SUBST(KSANE_CFLAGS)
	AC_SUBST(KSANE_LIBS)
  fi
fi

have_tested_for_sane=yes
AM_CONDITIONAL(FERRIS_HAVE_SANE, test x"$have_sane" = xyes)
AM_CONDITIONAL(FERRIS_HAVE_KSANE, test x"$have_sane" = xyes)
])


dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_QPRINTER([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl a CONDITIONAL FERRIS_HAVE_QPRINTER is defined and the shell var have_qprinter is either yes/no on exit
dnl QPRINTER_CFLAGS and QPRINTER_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_QPRINTER,
[dnl 
dnl
required_version=$1
have_qprinter=no


AC_ARG_ENABLE(qprinter-detection,
[--disable-qprinter-detection            Don't try to build support for printing to a QPrinter],
[
  if test x$enableval = xyes; then
	attempt_to_find_qprinter=yes
  else
	attempt_to_find_qprinter=no
  fi
])

if test x"$attempt_to_find_qprinter" = xno; then
	FERRIS_HAVE_QPRINTER=no
	QPRINTER_CFLAGS=""
	QPRINTER_LIBS=""
    	AC_MSG_RESULT([compilation of QPRINTER functions disabled])
else
  if test x"$have_tested_for_qprinter" = x; then
	AC_LANG_CPLUSPLUS

		PKG_CHECK_MODULES(QTGUI, QtGui >= 4.4.3, [ have_qtgui=yes ],  [ have_qtgui=no ] )
		QTGUI_CFLAGS=" $QTGUI_CFLAGS " 
		
		QPRINTER_CFLAGS=" $QTGUI_CFLAGS "
		QPRINTER_LIBS=" $QTGUI_LIBS "

		CXXFLAGS_cache=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $QPRINTER_CFLAGS"
		LDFLAGS_cache=$LDFLAGS
		LDFLAGS="$LDFLAGS $QPRINTER_LIBS"

		echo "trying to link a QPrinter client..."

		AC_TRY_LINK([
#include <QApplication>
#include <QTextDocument>
#include <QPrinterInfo>
#include <iostream>
		using namespace std;
        	],
		[
                int argc = 0;
                char** argv = 0;
                QApplication app( argc, argv );

                QList<QPrinterInfo> pi = QPrinterInfo:: availablePrinters();
                QTextDocument *document = new QTextDocument( 0 );
		return 0;
		],
       		[have_qprinter=yes], [have_qprinter=no])

		LDFLAGS=$LDFLAGS_cache
		CXXFLAGS=$CXXFLAGS_cache

		if test x"$have_qprinter" = xyes; then
			echo "Building qprinter support funtions"
			FERRIS_HAVE_QPRINTER=yes
			AC_DEFINE(HAVE_QPRINTER,1,[])
			AC_DEFINE(FERRIS_HAVE_QPRINTER,1,[])
			QPRINTER_CFLAGS=" $QPRINTER_CFLAGS "
			QPRINTER_LIBS=" $QPRINTER_LIBS "
		else
			echo "Couldn't link sample QPRINTER4 application, disabling QPRINTER support"
			FERRIS_HAVE_QPRINTER=no
			QPRINTER_CFLAGS=""
			QPRINTER_LIBS=""
		fi

	AC_LANG_C
	AC_SUBST(QPRINTER_CFLAGS)
	AC_SUBST(QPRINTER_LIBS)
  fi
fi

have_tested_for_qprinter=yes
AM_CONDITIONAL(FERRIS_HAVE_QPRINTER, test x"$have_qprinter" = xyes)
])



dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl ###############################################################################
dnl ###############################################################################
dnl ###############################################################################
dnl # Test for xmms remote API
dnl ###############################################################################
dnl
dnl AM_FERRIS_XMMS([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to say "xmms not detected..."
dnl LIBXMMS_CFLAGS and LIBXMMS_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_XMMS,
[dnl 
dnl
required_version=$1

have_libxmms=no
package=libxmms
version=$required_version

AC_ARG_ENABLE(xmms,
  [AS_HELP_STRING([--enable-xmms],
                  [enable xmms support (default=auto)])],[],[enable_xmms=check])

if test x$enable_xmms != xno; then

	AC_CHECK_PROG( have_xmms, xmms-config, yes, no )
fi


if test "$have_xmms" = yes; then
	have_libxmms=yes;
	LIBXMMS_LIBS="   `xmms-config --libs` "
	LIBXMMS_CFLAGS=" `xmms-config --cflags` "
	AC_DEFINE(HAVE_XMMS, 1, [have xmms installed])

	# success
	ifelse([$2], , :, [$2])

else
	ifelse([$3], , 
	[
	  	echo "xmms not found..."
	], 
	[$3])     
	LIBXMMS_CFLAGS=" "
	LIBXMMS_LIBS=" "
fi

AM_CONDITIONAL(HAVE_XMMS, test "$have_xmms" = yes)
AC_SUBST(LIBXMMS_LIBS)
AC_SUBST(LIBXMMS_CFLAGS)
])




dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_FUSELAGE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libfuselage for installation. 
dnl ie. default is to REQUIRE fuselage MINIMUM-VERSION or stop running.
dnl
dnl LIBFUSELAGE_CFLAGS and LIBFUSELAGE_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_FUSELAGE,
[dnl 
dnl
required_version=$1

have_libfuselage=no
package=libfuselage
version=$required_version

dnl AC_ARG_ENABLE(fuselage,
dnl   [AS_HELP_STRING([--enable-fuselage],
dnl                   [enable fuselage support (default=auto)])],[],[enable_fuselage=check])

if test x$enable_fuselage != xno; then

dnl 	PKG_CHECK_MODULES(LIBFUSELAGE, $package >= $version, [ 
dnl 	   have_libfuselage_pkgconfig=yes
dnl 	   have_libfuselage=yes 
dnl 	])


INCLUDES="$(cat <<-HEREDOC
	#include <string>
	using namespace std;
	#include <fuselagefs/fuselagefs.hh>
	using namespace Fuselage;
	using namespace Fuselage::Helpers;
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
	Delegatefs myfuse;
	struct poptOption* fuselage_optionsTable = myfuse.getPopTable();
HEREDOC
)"

CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS
AC_LANG_CPLUSPLUS
have_package=no

LIBFUSELAGE_CXXFLAGS=" -D_FILE_OFFSET_BITS=64  "
# try to hit it directly.
if test x"$have_package" = xno; then
	LIBFUSELAGE_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBFUSELAGE_CFLAGS $LIBFUSELAGE_CXXFLAGS "
	LIBFUSELAGE_LIBS=" $STLPORT_LIBS $LDFLAGS -lfuselagefs "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBFUSELAGE_CFLAGS], [$LIBFUSELAGE_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi
# try to hit it directly. 64.
if test x"$have_package" = xno; then
	LIBFUSELAGE_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBFUSELAGE_CFLAGS $LIBFUSELAGE_CXXFLAGS "
	LIBFUSELAGE_LIBS=" $STLPORT_LIBS $LDFLAGS -L/usr/lib64 -lfuselagefs "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBFUSELAGE_CFLAGS], [$LIBFUSELAGE_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBFUSELAGE_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBFUSELAGE_CFLAGS $LIBFUSELAGE_CXXFLAGS -I/usr/local/include "
	LIBFUSELAGE_LIBS=" $STLPORT_LIBS $LDFLAGS -L/usr/local/lib -lfuselagefs "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBFUSELAGE_CFLAGS], [$LIBFUSELAGE_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

AC_LANG_C
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

fi

#####################################################


have_libfuselage=no;

if test x"$have_package" = xyes; then
	have_libfuselage=yes;
	AC_DEFINE( HAVE_LIBFUSELAGE, 1, [Is libfuselage installed] )

	# success
	ifelse([$2], , :, [$2])

else
	if test x$have_libfuselage_pkgconfig = xyes; then
		echo "pkg-config could find your libfuselage but can't compile and link against it..." 
	fi

	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		echo "get it from the URL"
		echo "http://sourceforge.net/project/showfiles.php?group_id=16036"
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	LIBFUSELAGE_CFLAGS=" "
	LIBFUSELAGE_LIBS=" "
fi

AM_CONDITIONAL(HAVE_LIBFUSELAGE, test x"$have_libfuselage" = xyes)
AC_SUBST(LIBFUSELAGE_CFLAGS)
AC_SUBST(LIBFUSELAGE_LIBS)
])



dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################

dnl AM_FERRIS_POPT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to AC_MSG_ERROR() with a description of where
dnl to locate libpopt for installation. 
dnl ie. default is to REQUIRE popt MINIMUM-VERSION or stop running.
dnl
dnl LIBPOPT_CFLAGS and LIBPOPT_LIBS are set and AC_SUBST()ed when library is found.
dnl
AC_DEFUN(AM_FERRIS_POPT,
[dnl 
dnl
required_version=$1

have_libpopt=no
package=libpopt
version=$required_version

dnl AC_ARG_ENABLE(popt,
dnl   [AS_HELP_STRING([--enable-popt],
dnl                   [enable popt support (default=auto)])],[],[enable_popt=check])

if test x$enable_popt != xno; then

dnl 	PKG_CHECK_MODULES(LIBPOPT, $package >= $version, [ 
dnl 	   have_libpopt_pkgconfig=yes
dnl 	   have_libpopt=yes 
dnl 	])


INCLUDES="$(cat <<-HEREDOC
	#include <popt.h>
HEREDOC
)"
PROGRAM="$(cat <<-HEREDOC
HEREDOC
)"

CXXFLAGS_cache=$CXXFLAGS
LDFLAGS_cache=$LDFLAGS
AC_LANG_CPLUSPLUS
have_package=no

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBPOPT_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBPOPT_CFLAGS $LIBPOPT_CXXFLAGS "
	LIBPOPT_LIBS=" $STLPORT_LIBS $LDFLAGS -lpopt "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBPOPT_CFLAGS], [$LIBPOPT_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

# try to hit it directly.
if test x"$have_package" = xno; then
	LIBPOPT_CFLAGS=" $STLPORT_CFLAGS $CXXFLAGS $LIBPOPT_CFLAGS $LIBPOPT_CXXFLAGS -I/usr/local/include "
	LIBPOPT_LIBS=" $STLPORT_LIBS $LDFLAGS -L/usr/local/lib -lpopt "
	AM_FERRIS_INTERNAL_TRYLINK( [$LIBPOPT_CFLAGS], [$LIBPOPT_LIBS], 
				[ $INCLUDES ], [$PROGRAM],
				[have_package=yes], [have_package=no] )
fi

AC_LANG_C
LDFLAGS=$LDFLAGS_cache
CXXFLAGS=$CXXFLAGS_cache

fi

#####################################################


have_libpopt=no;

if test x"$have_package" = xyes; then
	have_libpopt=yes;
	AC_DEFINE( HAVE_LIBPOPT, 1, [Is libpopt installed] )

	# success
	ifelse([$2], , :, [$2])

else
	if test x$have_libpopt_pkgconfig = xyes; then
		echo "pkg-config could find your libpopt but can't compile and link against it..." 
	fi

	ifelse([$3], , 
	[
	  	echo ""
		echo "latest version of $package required. ($version or better) "
		echo ""
		AC_MSG_ERROR([Fatal Error: no correct $package found.])	
	], 
	[$3])     
	LIBPOPT_CFLAGS=" "
	LIBPOPT_LIBS=" "
fi

AM_CONDITIONAL(HAVE_LIBPOPT, test x"$have_libpopt" = xyes)
AC_SUBST(LIBPOPT_CFLAGS)
AC_SUBST(LIBPOPT_LIBS)
])




dnl #####################################################################
dnl #####################################################################
dnl #####################################################################
dnl #####################################################################





dnl AM_FERRIS_SQLITE3([MIN-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl
dnl The default ACTION-IF-NOT-FOUND is to put a message for the user to see that
dnl this module was not found and thus code written for it is not being compiled in.
dnl
dnl SQLITE3_CFLAGS and SQLITE3_LIBS are set and AC_SUBST()ed when library is found.
dnl AC_DEFINE(HAVE_SQLITE3) and AM_CONDITIONAL(HAVE_SQLITE3)
dnl
AC_DEFUN([AM_FERRIS_SQLITE3],
[{
dnl

required_version=$1
have_sqlite3=no
package=sqlite3
version=$required_version

AC_ARG_ENABLE(sqlite3,
  [AS_HELP_STRING([--enable-sqlite3],
                  [enable sqlite3 support (default=auto)])],[],[enable_sqlite3=check])
if test x$enable_sqlite3 != xno; then

	PKG_CHECK_MODULES(sqlite3, $package >= $version, [ have_sqlite3=yes ],  [ have_sqlite3=no ] )

fi


if test x"$have_sqlite3" = xyes; then

	have_sqlite3=yes
	AC_DEFINE( HAVE_SQLITE3, 1, [Is SQLITE3 installed] )

	SQLITE3_CFLAGS=" $sqlite3_CFLAGS "
	SQLITE3_LIBS=" $sqlite3_LIBS "
	echo "Found a Sqlite3 library that meets required needs..."
	echo "  SQLITE3_CFLAGS: $SQLITE3_CFLAGS "
	echo "  SQLITE3_LIBS:   $SQLITE3_LIBS "

	# success
	ifelse([$2], , :, [$2])

else
	ifelse([$3], , 
	[
	echo "Support for SQLITE3 version ($version) not being built... "
	], 
	[$3])     
fi

AM_CONDITIONAL(HAVE_SQLITE3, test x"$have_sqlite3" = xyes)
AC_SUBST(SQLITE3_CFLAGS)
AC_SUBST(SQLITE3_LIBS)
}])



dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################


dnl Configure paths for XINE
dnl
dnl Copyright (C) 2001 Daniel Caujolle-Bert <segfault@club-internet.fr>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl  
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl  
dnl  
dnl As a special exception to the GNU General Public License, if you
dnl distribute this file as part of a program that contains a configuration
dnl script generated by Autoconf, you may include it under the same
dnl distribution terms that you use for the rest of that program.
dnl  

dnl AM_PATH_XINE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for XINE, and define XINE_CFLAGS and XINE_LIBS
dnl
AC_DEFUN([AM_PATH_XINE],
[dnl 
dnl Get the cflags and libraries from the xine-config script
dnl
AC_ARG_WITH(xine-prefix,
    AS_HELP_STRING([--with-xine-prefix=DIR], [prefix where XINE is installed (optional)]),
            xine_config_prefix="$withval", xine_config_prefix="")
AC_ARG_WITH(xine-exec-prefix,
    AS_HELP_STRING([--with-xine-exec-prefix=DIR], [exec prefix where XINE is installed (optional)]),
            xine_config_exec_prefix="$withval", xine_config_exec_prefix="")
AC_ARG_ENABLE(xinetest, 
    AS_HELP_STRING([--disable-xinetest], [do not try to compile and run a test XINE program]),
            enable_xinetest=$enableval, enable_xinetest=yes)

  AC_LANG_PUSH([C])

  if test x$xine_config_exec_prefix != x ; then
     xine_config_args="$xine_config_args --exec-prefix=$xine_config_exec_prefix"
     if test x${XINE_CONFIG+set} != xset ; then
        XINE_CONFIG=$xine_config_exec_prefix/bin/xine-config
     fi
  fi
  if test x$xine_config_prefix != x ; then
     xine_config_args="$xine_config_args --prefix=$xine_config_prefix"
     if test x${XINE_CONFIG+set} != xset ; then
        XINE_CONFIG=$xine_config_prefix/bin/xine-config
     fi
  fi

  min_xine_version=ifelse([$1], ,0.5.0,$1)
  if test "x$enable_xinetest" != "xyes" ; then
    AC_MSG_CHECKING([for XINE-LIB version >= $min_xine_version])
  else
    AC_PATH_TOOL(XINE_CONFIG, xine-config, no)
    AC_MSG_CHECKING([for XINE-LIB version >= $min_xine_version])
    no_xine=""
    if test "$XINE_CONFIG" = "no" ; then
      no_xine=yes
    else
      XINE_CFLAGS=`$XINE_CONFIG $xine_config_args --cflags`
      XINE_LIBS=`$XINE_CONFIG $xine_config_args --libs`
      XINE_ACFLAGS=`$XINE_CONFIG $xine_config_args --acflags`
      xine_config_major_version=`$XINE_CONFIG $xine_config_args --version | \
             sed -n 's/^\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*$/\1/p'`
      xine_config_minor_version=`$XINE_CONFIG $xine_config_args --version | \
             sed -n 's/^\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*$/\2/p'`
      xine_config_sub_version=`$XINE_CONFIG $xine_config_args --version | \
             sed -n 's/^\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*$/\3/p'`
      xine_data_dir=`$XINE_CONFIG $xine_config_args --datadir`
      xine_script_dir=`$XINE_CONFIG $xine_config_args --scriptdir`
      xine_plugin_dir=`$XINE_CONFIG $xine_config_args --plugindir`
      xine_locale_dir=`$XINE_CONFIG $xine_config_args --localedir`
      dnl    if test "x$enable_xinetest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $XINE_CFLAGS"
      LIBS="$XINE_LIBS $LIBS"
dnl
dnl Now check if the installed XINE is sufficiently new. (Also sanity
dnl checks the results of xine-config to some extent
dnl
      rm -f conf.xinetest
      AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <xine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main ()
{
  int major, minor, sub;
   char *tmp_version;

  system ("touch conf.xinetest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = (char *) strdup("$min_xine_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &sub) != 3) {
     printf("%s, bad version string\n", "$min_xine_version");
     exit(1);
   }

  if ((XINE_MAJOR_VERSION != $xine_config_major_version) ||
      (XINE_MINOR_VERSION != $xine_config_minor_version) ||
      (XINE_SUB_VERSION != $xine_config_sub_version))
    {
      printf("\n*** 'xine-config --version' returned %d.%d.%d, but XINE (%d.%d.%d)\n", 
             $xine_config_major_version, $xine_config_minor_version, $xine_config_sub_version,
             XINE_MAJOR_VERSION, XINE_MINOR_VERSION, XINE_SUB_VERSION);
      printf ("*** was found! If xine-config was correct, then it is best\n");
      printf ("*** to remove the old version of XINE. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If xine-config was wrong, set the environment variable XINE_CONFIG\n");
      printf("*** to point to the correct copy of xine-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else
    {
      if ((XINE_MAJOR_VERSION > major) ||
        ((XINE_MAJOR_VERSION == major) && (XINE_MINOR_VERSION > minor)) ||
        ((XINE_MAJOR_VERSION == major) && (XINE_MINOR_VERSION == minor) && (XINE_SUB_VERSION >= sub)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of XINE (%d.%d.%d) was found.\n",
               XINE_MAJOR_VERSION, XINE_MINOR_VERSION, XINE_SUB_VERSION);
        printf("*** You need a version of XINE newer than %d.%d.%d. The latest version of\n",
	       major, minor, sub);
        printf("*** XINE is always available from:\n");
        printf("***        http://xine.sourceforge.net\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the xine-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of XINE, but you can also set the XINE_CONFIG environment to point to the\n");
        printf("*** correct copy of xine-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
]])],[],[no_xine=yes],[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
    fi
    if test "x$no_xine" = x ; then
       AC_MSG_RESULT(yes)
       ifelse([$2], , :, [$2])     
    else
      AC_MSG_RESULT(no)
      if test "$XINE_CONFIG" = "no" ; then
        echo "*** The xine-config script installed by XINE could not be found"
        echo "*** If XINE was installed in PREFIX, make sure PREFIX/bin is in"
        echo "*** your path, or set the XINE_CONFIG environment variable to the"
        echo "*** full path to xine-config."
      else
        if test -f conf.xinetest ; then
          :
        else
          echo "*** Could not run XINE test program, checking why..."
          CFLAGS="$CFLAGS $XINE_CFLAGS"
          LIBS="$LIBS $XINE_LIBS"
          AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <xine.h>
#include <stdio.h>
]], [[ return ((XINE_MAJOR_VERSION) || (XINE_MINOR_VERSION) || (XINE_SUB_VERSION)); ]])],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding XINE or finding the wrong"
          echo "*** version of XINE. If it is not finding XINE, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means XINE was incorrectly installed"
          echo "*** or that you have moved XINE since it was installed. In the latter case, you"
          echo "*** may want to edit the xine-config script: $XINE_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
        fi
      fi
    XINE_CFLAGS=""
    XINE_LIBS=""
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(XINE_CFLAGS)
  AC_SUBST(XINE_LIBS)
  AC_SUBST(XINE_ACFLAGS)
  AC_LANG_POP([C])
  rm -f conf.xinetest

dnl Make sure HAVE_STRSEP, HAVE_SETENV and HAVE_STRPBRK are defined as
dnl necessary.
  AC_CHECK_FUNCS([strsep strpbrk setenv])
dnl alloca (in public macro) and MinGW
  AC_CHECK_HEADERS([malloc.h])
])


dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl # inspired by FONTFORGE_PLATFORM_SPECIFICS
dnl ############
AC_DEFUN_ONCE([LIBFERRIS_PLATFORM_SPECIFICS],
[
cd ${srcdir}; absolute_srcdir=`pwd`; cd -;



m4_define([default_SDK],[/])
m4_define([default_CARBON],[System/Library/Frameworks/Carbon.framework/Carbon])
m4_define([default_COCOA],[/System/Library/Frameworks/Cocoa.framework/Cocoa])
m4_define([default_CORESERVICES],[System/Library/Frameworks/CoreServices.framework/CoreServices])

echo "plaform specifics for host: $host "
AS_CASE([$host],
   [*-apple-darwin*],[
      libferris_ismac="yes"
      AC_DEFINE([_Keyboard],[1],[Platform specific stuff])
      AC_DEFINE([__Mac],[1],[Platform specific stuff])
      AC_DEFINE([PLATFORM_OSX],[1],[Platform specific stuff])
   ],

   [:]  dnl DEFAULT AS_CASE

) dnl END AS_CASE

AM_CONDITIONAL([PLATFORM_OSX],[test x"${libferris_ismac}" = xyes])


AC_ARG_ENABLE(debug,
[--enable-debug            compile with -g and -O0 debug information],
[
  if test x$enableval = xyes; then
	echo setting debug mode to on...;
        CFLAGS="    $CFLAGS   -O0 -g -pipe "; #-Wall ";
	CXXFLAGS="  $CXXFLAGS -O0 -g -pipe "; #-Wall ";
  else
	echo setting debug mode to off...
  fi
])

AC_ARG_ENABLE(hiddensymbols,
[--enable-hiddensymbols            use hidden symbols for private APIs],
[
  if test x$enableval = xyes; then
	echo setting hidden symbol support...;
        CFLAGS="    $CFLAGS   -DGCC_HASCLASSVISIBILITY -fvisibility=default -fvisibility-inlines-hidden ";
	CXXFLAGS="  $CXXFLAGS -DGCC_HASCLASSVISIBILITY -fvisibility=default -fvisibility-inlines-hidden ";
	AC_DEFINE(GCC_HASCLASSVISIBILITY)
  fi
])


AC_ARG_ENABLE(profile,
[--enable-profile            compile with profile debug information],
[
  if test x$enableval = xyes; then
     echo setting profile mode to on...
     CFLAGS="   $CFLAGS   -O0 -g -pg -fprofile-arcs -ftest-coverage "; 
     CXXFLAGS=" $CXXFLAGS -O0 -g -pg -fprofile-arcs -ftest-coverage "; 
  else
     echo setting profile mode to off...
  fi
])

AC_ARG_ENABLE(wrapdebug,
[--enable-wrapdebug            compile with -g and -O0 debug information],
[
  if test x$enableval = xyes; then
	echo setting debug mode to on...;
	CFLAGS="   $CFLAGS   -O0 -g -pipe -Wall "; 
	CXXFLAGS=" $CXXFLAGS -O0 -g -pipe -Wall "; 
  else
	echo setting debug mode to off...
  fi
])


])

AC_DEFUN([LIBFERRIS_PLATFORM_SUBST_COMPILER_FLAGS],
[
   AC_SUBST(CFLAGS)
   AC_SUBST(CPPFLAGS)
   AC_SUBST(LDFLAGS)
   AC_SUBST(CXXFLAGS)
   AC_SUBST(CXXCPPFLAGS)
])


dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
dnl ######################################################################
