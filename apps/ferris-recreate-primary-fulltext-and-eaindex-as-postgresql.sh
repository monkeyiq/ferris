#!/bin/bash

dbname=${1:-findexpriv};

echo "-------------------------------------------------------------"
echo "This remakes the default EA and Fulltext indexes to use PostgreSQL"
echo "The indexes are linked to allow combined metadata and text search"
echo "-------------------------------------------------------------"
echo "-------------------------------------------------------------"
echo "WARNING This script will drop the postgresql database:$dbname"
echo " in order to create it again... press return to proceed";
read

echo "drop database $dbname" | psql template1
sleep 2;
fcreate ~/.ferris/full-text-index --create-type=fulltextindextsearch2 dbname="$dbname"
fcreate ~/.ferris/ea-index --create-type=eaindexpostgresql dbname="$dbname" db-exists=1 
feaindex-attach-fulltext-index -P ~/.ferris/ea-index -F ~/.ferris/full-text-index
