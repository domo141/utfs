#!/bin/sh
# $Id; testreadwrite.sh $
#
# Author: Tomi Ollila -- too ät iki piste fi
#	    All rights reserved
#
# Created: Sat Jan 12 20:48:54 EET 2008 too
# Last modified: Sat 05 Sep 2009 08:04:25 EEST too

# Separate script for some (initial) read/write testing.
# More thorough tests are done with ...

case $1 in
    *' '*) echo "'"$1"'" has spaces >&2; exit 1 ;;
    '')    echo "Usage $0 dir" >&2; exit 1 ;;
esac

test -d $1 || { echo "$1: not a directory"; exit 1; }

#tdir=$1/$USER-utfs-testdir-`date +%Y%m%d-%H%M%S`
tdir=$1/utfs-testdir-`date +%Y%m%d-%H%M%S`

echo ::: | tr : \\012
set -x

test -d $tdir && exit 1

trap "rm -rf $tdir" 0
mkdir $tdir

xxx () {
	set +x
	echo
	echo enter "'"exit"'" to exit this '(deletes $tdir)'
	while :; do
		read line
		case $line in exit) exit 0 ;; esac
	done
}

# travis2utfs uses 1024 byte buffer. playing around that...

thisdir=`cd \`dirname $0\`; pwd`

seq="1020 1021 1022 1023 1024 1025 1026 1027 1028"

set +x

for i in $seq
do
  	echo head -c $i $thisdir/travis2utfs.c '>' $tdir/$i
	head -c $i $thisdir/travis2utfs.c > $tdir/$i &
done

for i in $seq
do
  	md5sum $tdir/$i
done

echo
for i in $seq
do
	#echo head -c $i $thisdir/travis2utfs.c '|' md5sum
	head -c $i $thisdir/travis2utfs.c | md5sum
done

# test many files at the same time...

exec 3> $tdir/file1
exec 4> $tdir/file2
exec 5> $tdir/file3
exec 6> $tdir/file4
exec 7> $tdir/file5

echo file1 > $tdir/file1
echo file2 > $tdir/file2
echo file3 > $tdir/file3
echo file4 > $tdir/file4
echo file5 > $tdir/file5

# these are one at at time
cat $tdir/file1 $tdir/file2 $tdir/file3 $tdir/file4 $tdir/file5

exec 3<&-
exec 4<&-
exec 5<&-
exec 6<&-
exec 7<&-

xxx
