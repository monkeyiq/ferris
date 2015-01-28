#!/bin/bash

PGCONTRIB=/usr/share/pgsql/contrib

echo create database ferrisftxtemplate | psql 
cat ${PGCONTRIB}/tsearch2.sql   | psql ferrisftxtemplate
cat ${PGCONTRIB}/dbsize.sql     | psql ferrisftxtemplate
cat ${PGCONTRIB}/btree_gist.sql | psql ferrisftxtemplate
cat ${PGCONTRIB}/_int.sql       | psql ferrisftxtemplate
createlang -d ferrisftxtemplate plpgsql
echo "update pg_database set datistemplate='t' where datname='ferrisftxtemplate';" | psql ferrisftxtemplate
echo "grant SELECT on table pg_ts_cfg to PUBLIC;" | psql ferrisftxtemplate
echo "grant SELECT on table pg_ts_cfgmap to PUBLIC;" | psql ferrisftxtemplate
echo "grant SELECT on table pg_ts_parser to PUBLIC;" | psql ferrisftxtemplate
echo "grant SELECT on table pg_ts_dict to PUBLIC;" | psql ferrisftxtemplate

# echo "update pg_database set datistemplate='f' where datname='ferrisftxtemplate';" | psql

echo create database ferriseatemplate | psql 
cat ${PGCONTRIB}/dbsize.sql     | psql ferriseatemplate
cat ${PGCONTRIB}/btree_gist.sql | psql ferriseatemplate
cat ${PGCONTRIB}/_int.sql       | psql ferriseatemplate
createlang -d ferriseatemplate plpgsql
echo "update pg_database set datistemplate='t' where datname='ferriseatemplate';" | psql ferriseatemplate
