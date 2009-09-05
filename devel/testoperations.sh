#!/bin/sh
# $Id; testoperations.sh $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2008 Tomi Ollila
#	    All rights reserved
#
# Created: Wed Jan 02 22:43:11 EET 2008 too
# Last modified: Sat 05 Sep 2009 08:04:32 EEST too

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

ln -s /this/is/symlink $tdir/symlink
readlink $tdir/symlink
mknod $tdir/node c 100 200
: ^^^ error if no rights ^^^
mkdir $tdir/dir

unlink $tdir/symlink
ls -l $tdir/symlink
: ^^^ error ok ^^^

mv $tdir/dir $tdir/dir2
ls -ld $tdir/dir
: ^^^ error ok ^^^

ls -ld $tdir/dir2
rmdir $tdir/dir2
ls -ld $tdir/dir2
: ^^^ error ok ^^^

# now we need a file.

echo foobar > $tdir/file
cat $tdir/file
ln $tdir/file $tdir/file-link
ls -l $tdir/file-link
chmod 755 $tdir/file-link
ls -l $tdir/file-link
perl -e 'truncate $ARGV[0], 3' $tdir/file-link
stat $tdir/file
perl -e '$atime = $mtime = time-99; utime $atime, $mtime, $ARGV[0]' $tdir/file
stat $tdir/file-link

chown root $tdir/file-link || true
: ^^^ error if no rights ^^^

xxx
