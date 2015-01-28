#!/bin/sh

inputfile=${1:?-};

if [ x"$inputfile" == "x-" ];
then
    inputfile=/dev/stdin
fi;

hevea $inputfile -text -s -o /dev/stdout | sed -e '$d' | sed -e '$d'
