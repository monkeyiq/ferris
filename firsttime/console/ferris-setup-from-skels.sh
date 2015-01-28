#!/bin/bash

cp -av /etc/skel/.ferris ~;
find ~/.ferris -type d -exec chmod -c 700 {} \;
find ~/.ferris -type f -exec chmod -c 600 {} \;
