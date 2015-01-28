#!/bin/bash

#
#
# Import a whole hive of .desktop files
#
#

hiveroot=${1:-/etc/X11/applnk};

for if in `find ${hiveroot} -type f -name "*.desktop" `
do
    echo Importing file $if;
    ferris-import-desktop-file $if;
done
