#!/bin/sh

inputfile=${1:?-};

if [ x"$inputfile" == "x-" ];
then
    inputfile=/dev/stdin
fi;

unrtf --text $inputfile 2>/dev/null | sed 's/^###.*//g'
