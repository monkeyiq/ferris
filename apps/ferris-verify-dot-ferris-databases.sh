#!/bin/bash

for if in $(find ~/.ferris -name "*.db")
do
    echo "Verifying database $if..."
    db_verify $if
done
