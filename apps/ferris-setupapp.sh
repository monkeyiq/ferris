#!/bin/bash

appname=${1:?Usage: setupapp applabel appexe openmany handleurl};
exename=${2:-${appname}};
openmany=${3:-0};
handleurl=0;

cd ~/.ferris
createfile applications:// ${appname}
createea   applications://${appname} ferris-exe ${exename}
createea   applications://${appname} ferris-handles-urls ${handleurl}
createea   applications://${appname} ferris-opens-many   ${openmany}
