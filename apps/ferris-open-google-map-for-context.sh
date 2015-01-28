#!/bin/bash

#
# A little glue to open a context's lat/long point in a new firefox tab
#

if=$1;
openInNewTab=${2:-1}

loc=$(ferrisls -ld --show-ea=google-maps-location $if);

if test "x$(echo $loc | sed 's/ //g')" = "x"; then
    echo "No maps.google.com location for this context"
else
    echo $openInNewTab
    newPane="new-tab"
    if test -z $openInNewTab; then
	newPane="new-window"
    fi

    cmd="firefox -remote 'openURL(http://maps.google.com/?q=$loc,$newPane)' "
    echo "command: $cmd"
    eval $cmd
fi
