#!/bin/bash

arg=${1:?Supply virtual bash script as arg1};
fcat $arg 2>/dev/null | bash

