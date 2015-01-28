#!/bin/bash

# zip -q9r /tmp/DevelopBuild/ferris/plugins/context/firefox/firefox-extension/chrome/libferrismount.jar content skin
# zip -q9 /tmp/DevelopBuild/ferris/plugins/context/firefox/firefox-extension/libferrismount.xpi chrome.manifest install.rdf  chrome/libferrismount.jar

rm -rf /tmp/stage
mkdir -p /tmp/stage
cp -a skin chrome install.rdf  chrome.manifest     /tmp/stage
cd /tmp/stage
rm -f /tmp/DevelopBuild/ferris/plugins/context/firefox/firefox-extension/libferrismount.xpi
zip -q9r  /tmp/DevelopBuild/ferris/plugins/context/firefox/firefox-extension/libferrismount.xpi .
