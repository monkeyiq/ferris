#!/bin/bash

echo "This is the monkeyiq config invoker. please don't use it."
export CXX="ccache g++"
export CC="ccache gcc"

nice /ferris/configure                      \
--x-libraries=/usr/X11R6/lib          \
--enable-debug                        \
--enable-djvu                         \
--with-pdftotext                      \
--enable-stlport-nocustomio           \
--enable-postgresql-tsearch2-indexing \
--disable-stlport                     \
--enable-fca                          \
--with-sigcxx-2x=yes                  \
--disable-fastinstall                 \
--disable-strigi                      \
--enable-quickerlinking               \
--with-swig-perl 

# --enable-hiddensymbols                
# --disable-kde-extractor               \




# --enable-gold

## --disable-xqilla --disable-xalanc

## --with-dbxml-prefix=/usr              \


# --with-pathan2-source=/usr/local/src/libpathan-2.0beta 

# --with-dbxml-prefix=/usr

#--with-swig-ocaml                     
#--enable-kde-extractor

#--enable-hiddensymbols                \
# --with-dbxml-prefix=/usr              \
# --with-dbxml-db4-prefix=/usr          \

#--with-gnome-vfs-mime                 \

# --with-dbxml-prefix=/usr/local/dbxml  \
# --with-dbxml-db4-prefix=/usr/local/dbxml/db4 

# --with-swig-perl \
# --with-swig-python-version=python2.2  \
# --with-swig-python-prefix=/usr        \


#--with-libmpeg3=/usr/local/src/libmpeg3-1.3 \
#--disable-fast-install
# --with-xml4c=/store/Lindev/xerces-c-src1_5_0 
