#!/bin/bash

alias ldd='otool -L'
. /Users/ben/macports-personal/setup.sh

ORIGDIR=`pwd`
TEMPDIR=/tmp/ferris-bundle
rm -rf  /tmp/ferris-bundle
mkdir $TEMPDIR

scriptdir=$TEMPDIR/Ferris.app/Contents/MacOS
frameworkdir=$TEMPDIR/Ferris.app/Contents/Frameworks
bundle_res=$TEMPDIR/Ferris.app/Contents/Resources
bundle_bin="$bundle_res/usr/local/bin"
bundle_lib="$bundle_res/usr/local/lib"
bundle_libexec="$bundle_res/usr/local/libexec"
bundle_etc="$bundle_res/usr/local/etc"
bundle_share="$bundle_res/usr/local/share"
export PATH="$PATH:$scriptdir"
srcdir=$(pwd)


echo "...doing the make install..."
DESTDIR=$bundle_res make install

#echo "...setup Ferris.app bundle..."
#rsync -av $bundle_res/opt/local/share/fontforge/osx/FontForge.app $TEMPDIR/

#
# Now to fix the binaries to have shared library paths that are all inside
# the distrubtion instead of off into /opt/local or where macports puts things.
#
cd $bundle_bin
for if in ferrisls fcat
do
  dylibbundler --overwrite-dir --bundle-deps --fix-file \
    ./$if \
    --install-path @executable_path/../lib \
    --dest-dir ../lib
done
    
mkdir -p $frameworkdir
rsync -av /Users/ben/macports-personal/Library/Frameworks/Qt*  $frameworkdir
# /Applications/FontForge.app/Contents/Resources/opt/local/share/fontforge/osx/FontForge.app/Contents/Frameworks

cd $bundle_bin
for if in ferrisls fcat ferris-redirect 
do
    for libname in QtDBus QtXml QtCore QtSql
    do
       install_name_tool -change \
          /Users/ben/macports-personal/Library/Frameworks/$libname.framework/Versions/4/$libname \
          /Applications/Ferris.app/Contents/Frameworks/$libname.framework/Versions/4/$libname \
          $if
    done
done

#
# setup plugins
#
rsync -av /usr/local/lib/ferris $bundle_lib
cd $bundle_lib/ferris/plugins/context
for libname in *dylib
do
    library-paths-macports-personal-to-absolute.sh $libname /Applications/Ferris.app/Contents/Resources/usr/local/lib
done
for libname in *factory.dylib
do
    library-paths-macports-personal-to-absolute.sh $libname /Applications/Ferris.app/Contents/Resources/usr/local/lib
done

#
# Qt paths in shared libraries
#
cd $bundle_lib
for if in libferris.15.dylib *.dylib
do
    for libname in QtDBus QtXml QtCore QtSql QtNetwork
    do
       install_name_tool -change \
          /Users/ben/macports-personal/Library/Frameworks/$libname.framework/Versions/4/$libname \
          /Applications/Ferris.app/Contents/Frameworks/$libname.framework/Versions/4/$libname \
          $if
    done
done


#       @executable_path/../../../../Frameworks/$libname.framework/Versions/4/$libname \

rsync -av /Users/ben/macports-personal/share/qt4 /Applications/Ferris.app/Contents/Resources/usr/share
ln -s /Applications/Ferris.app/Contents/Resources/usr/share/qt4/plugins /Applications/Ferris.app/Contents/qt4--plugins
LANG=C sed -i -e 's|/Users/ben/macports-personal/share/qt4/plugins|/Applications/Ferris.app/Contents/qt4--plugins|g' /Applications/Ferris.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore

rsync -av /Users/ben/macports-personal/lib/libsqlite* $bundle_lib/
rsync -av /Users/ben/macports-personal/lib/libintl*   $bundle_lib/
cd $bundle_lib/
library-paths-macports-personal-to-absolute.sh libsqlite3.dylib  /Applications/Ferris.app/Contents/Resources/usr/local/lib
library-paths-macports-personal-to-absolute.sh libintl.dylib  /Applications/Ferris.app/Contents/Resources/usr/local/lib
ln -s /Applications/Ferris.app/Contents/Frameworks/QtSql.framework/Versions/4/QtSql .
ln -s /Applications/Ferris.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore .
ln -s /Applications/Ferris.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml .


##install_name_tool -change /Users/ben/macports-personal/Library/Frameworks/QtCore.framework/Versions/4/QtCore /Applications/Ferris.app/Contents/Frameworks/QtSql.framework/Versions/4/QtSql  QtSql 

for libname in QtNetwork QtXml QtDBus
do
    ls -l /Applications/Ferris.app/Contents/Frameworks/$libname.framework/Versions/4/$libname
library-paths-macports-personal-to-absolute.sh /Applications/Ferris.app/Contents/Frameworks/$libname.framework/Versions/4/$libname /Applications/Ferris.app/Contents/Resources/usr/local/lib
done

###library-paths-macports-personal-to-absolute.sh /Applications/Ferris.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml  /Applications/Ferris.app/Contents/Resources/usr/local/lib
