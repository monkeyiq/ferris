#!/bin/bash

mimetype=${1:?usage: setupopen mimetype app};
app=${2};

ferris-setupaction.sh ${mimetype} open ${app};
