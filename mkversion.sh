#!/bin/sh

STAMP_H=${STAMP_H-${1-version.h}}
N_BUILD=${N_BUILD-1}

echo "#define VERSION_BUILD $N_BUILD" >> $STAMP_H

N_BUILD=$( awk '/VERSION_BUILD/ { print 1 + $3 ; exit }' $STAMP_H )
S_STAMP=$(date +%Y.%m.%d)
X_STAMP=$(date +%Y%m%d)
S_BRANCH=$( git branch --show-current )

cat > $STAMP_H <<XXXX
#define VERSION_BUILD $N_BUILD
#define VERSION_STAMP "$S_STAMP"
#define VERSION_BRANCH "$S_BRANCH"
XXXX
