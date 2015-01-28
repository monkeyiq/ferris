#!/bin/sh

inputfile=${1:?-};

if [ x"$inputfile" == "x-" ];
then
    inputfile=/dev/stdin
fi;

    fcat $inputfile/maindoc.xml | xmllint --format - | sed -e 's/<[^>]*>//g' -e '/^[ 	]*$/d'
