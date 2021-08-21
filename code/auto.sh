#!/bin/sh
SUFFIX=$1
ELEMENTS=$2

git pull
mpicc $FILE.c /usr/local/lib/libpapi.a -lm -o mergesortparallelo$SUFFIX
mpirun $FILE $ELEMENTS
