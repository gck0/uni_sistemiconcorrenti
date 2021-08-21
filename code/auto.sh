#!/bin/sh
SUF=$1
ELEM=$2

PRE=mergesortparallelo
FILE=${PRE}${SUF}

git pull
mpicc ${FILE}.c /usr/local/lib/libpapi.a -lm -o $FILE
mpirun $FILE $ELEM
