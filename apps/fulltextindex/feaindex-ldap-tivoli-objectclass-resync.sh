#!/bin/bash

echo "This script resyncs the EA index attribute list for the ferris-context"
echo "objectClass in the schema from the raw schema of the Tivoli server"

IDX=${1:?"Supply the Tivoli LDAP index path as arg1."};

ATTRLIST=$(sudo su -c 'cat /etc/ldapschema/V3.modifiedschema' \
  | grep ferris-context - \
  | cut -d '(' -f 4   \
  | sed 's/ \$ /,/g'  \
  | sed 's/)//g'      \
  | sed 's/ //g');

PATH="/ferris/tests/regression:$PATH"

#echo "ATTRLIST=$ATTRLIST"

writecontext "$IDX/ea-index-config.db/ldapidx-ferriscontext-attrlist" "$ATTRLIST"
