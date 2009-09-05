#!/bin/sh
# $Id; dbgsetup.sh $
#
#
# Created: Wed Jan 02 20:29:41 EET 2008 too
# Last modified: Sat 05 Sep 2009 08:05:07 EEST too

die () { echo "$@" >&2; exit 1; }

case `env which utfs-server` in
    /*) ;;
    *) die ' ' utfs-server can not be found in your PATH ;;
esac

travis2utfs=`dirname $0`/travis2utfs

test -x $travis2utfs || sh $travis2utfs.c
test -x $travis2utfs || die '"'$travis2utfs'"' does not exist.

r () { echo + "$@"; "$@" & }

echo

r xterm -g 80x10-0-10 -e "$travis2utfs" $1
echo --- >> $HOME/utfs-server.log
r xterm -g 80x10-0-210 -e tail -f $HOME/utfs-server.log

echo
echo Now enter
echo
echo ' ' ./utfs -c 2001 -p 127.0.0.1:2000 -d 127.0.0.1:/tmp mnt\; fusermount -u mnt
echo
echo on command line "(remember also '-f')"
echo
