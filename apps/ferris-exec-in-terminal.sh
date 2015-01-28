#!/bin/bash

fname=`mktemp`
arg=${1:?Supply bash script as arg1};

echo "fname:$fname  arg: $arg"
if ftest -e "$arg"; then
    fcat $arg 2>/dev/null >| $fname
else
    echo "#!/bin/bash" >| $fname
    echo "$@" >> $fname
fi
echo "echo " >> $fname
echo "echo ---Execution Complete at `date`---" >> $fname
echo "read" >> $fname
chmod +x $fname
konsole -e $fname


