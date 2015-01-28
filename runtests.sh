#!/bin/bash
cd /ferris;
runtest -v -v --tool ferris EXBASE=`pwd`/ SDATA=`pwd`/testsuite/sampledata --srcdir `pwd`/testsuite
