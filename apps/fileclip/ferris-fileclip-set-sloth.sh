#!/bin/bash

val=${1:-1};
clipname=${2:-~/.ferris/file-clipboard.db/primary};

echo -n $val | ferris-redirect -T --ea=commands-use-sloth $clipname
