#!/bin/sh

inputfile=${1:?-};

if [ x"$inputfile" == "x-" ];
then
    inputfile=/dev/stdin
fi;

ps2ascii $inputfile 
