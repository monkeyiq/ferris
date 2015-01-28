#!/bin/bash
#find . -name "*.hh" -or -name "*.cpp" -print | etags -
#find . -name "*.[chCH]" -print | etags -
find . -regextype posix-extended \
    -name "libferris*wrap*" -prune \
    -o -regex ".*/\.#.*" -prune  \
    -o -regex ".*\.(cpp|hh)$" -print | etags -
