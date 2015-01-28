#!/bin/bash

mkdir -p ~/.ferris/full-text-index
fcreate ~/.ferris/full-text-index --create-type=fulltextindexclucene 
fcreate ~/.ferris/ea-index --create-type=eaindexclucene db-exists=1 
feaindex-attach-fulltext-index -P ~/.ferris/ea-index -F ~/.ferris/full-text-index
