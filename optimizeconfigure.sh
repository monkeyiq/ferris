echo "This is the monkeyiq config invoker. please don't use it."
export CXX="ccache g++"
export CC="ccache gcc"

nice ./configure                      \
--with-gnome-vfs-mime                 \
--enable-djvu                         \
--with-pdftotext                      \
--with-dbxml-prefix=/usr              \
--with-dbxml-db4-prefix=/usr          \
--enable-stlport-nocustomio           \
--enable-postgresql-tsearch2-indexing \
--with-swig-ocaml 


# --with-dbxml-prefix=/usr/local/dbxml  \
# --with-dbxml-db4-prefix=/usr/local/dbxml/db4 

# --with-swig-perl \
# --with-swig-python-version=python2.2  \
# --with-swig-python-prefix=/usr        \


#--with-libmpeg3=/usr/local/src/libmpeg3-1.3 \
#--disable-fast-install
# --with-xml4c=/store/Lindev/xerces-c-src1_5_0 
