#!/bin/bash

mountpoint=${1:-~/ferrisfuse/etagere-as-kml};

export LIBFERRIS_XSLTFS_SHEETS_URL="/usr/local/ferris/stylesheets"
URL='single-file-filesystem:xsltfs?append-extension=.kml&stylesheet=ferris-geospatial-emblems-to-kml.xsl&reverse-stylesheet=ferris-kml-to-geospatial-emblems.xsl://context/etagere/libferris/libferris-geospatial.kml'

echo "mountpoint:$mountpoint"
mkdir -p "$mountpoint"
ferrisfs -u "$URL"  -F '.*.kml' --force-append-flag-clear  "$mountpoint"

