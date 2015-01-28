#!/bin/bash

mimetype=${1:?usage: setupaction mimetype applabel app};
applabel=${2};
app=${3:-${applabel}};

major=`dirname ${mimetype}`
minor=`basename ${mimetype}`

createdir  mime:// ${major}
createdir  mime://${major} ${minor}
createdir  mime://${mimetype} actions
createfile mime://${mimetype}/actions ${applabel}
createea   mime://${mimetype}/actions/${applabel} ferris-appname ${app}
