#!/bin/sh
FILE=$1
ELEMENTS=$2

git pull
mpicc $FILE.c /usr/local/lib/libpapi.a -lm -o $FILE
mpirun $FILE $ELEMENTS
