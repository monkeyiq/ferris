#!/bin/sh
#
# scale an image from std to sz and write it to stdout
#
sz=${1:?supply size as arg1}
format=${2:-unknown};

# graphics magick
# gm convert -resize $sz - -

# Nokia n900 with newer ImageMagick which can respect exif orientation tags.
# setup by extracting tarball to /opt and then:
# cd /usr/lib
# ln -s /opt/ImageMagick-6.6.0/lib/ImageMagick-6.6.0 .
#
# -resize takes a handful of seconds but is a much nicer result.
# -sample takes almost no time but is blocky
# 
if [ -e /opt/ImageMagick-6.6.0 ]; 
then 
    export LD_LIBRARY_PATH=/opt/ImageMagick-6.6.0/lib
    /opt/ImageMagick-6.6.0/bin/convert -auto-orient -resize $sz - -
    exit
fi

# try to do a better job for CR2 RAW files than using
# the embedded JPEG preview.
if [ "xCR2" = "x$format" ]; then
    fn=$(mktemp)
    cat - >| $fn;
    dcraw -w -c $fn | convert -resize $sz -auto-orient - - | ppmtojpeg 
else
    # Raw ImageMagick
    convert -resize $sz -auto-orient - -
fi



