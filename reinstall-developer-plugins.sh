#!/bin/bash

rm -rf /tmp/alternate-ferris-plugins
cd /ferris/plugins
make DESTDIR=/tmp/alternate-ferris-plugins install
