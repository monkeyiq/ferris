#!/bin/bash

SQLFILE=${1:=test.sql};

>|/tmp/ben/debug/ferris-log.txt; 
rm -f /tmp/empty/test.sqlite
touch /tmp/empty/test.sqlite
/ferris/exposures/sqlitevtab/sqlite3 -init  $SQLFILE  /tmp/empty/test.sqlite  </dev/null; 
cat /tmp/ben/debug/ferris-log.txt
