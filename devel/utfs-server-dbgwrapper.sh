#!/bin/sh
# $Id; utfs-server-dbgwrapper.sh $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2007 Tomi Ollila
#	    All rights reserved
#
# Created: Sun Dec 30 17:35:55 EET 2007 too
# Last modified: Sat 05 Sep 2009 08:03:33 EEST too

# symlink this to utfs-server at somewher in your path...
# example: ln -s `pwd`/utfs-server-dbgwrapper.sh $HOME/bin/utfs-server

LC_ALL=C LANG=C; export LC_ALL LANG

case $1 in link)
	case $2 in '') echo link needs target dir; exit 1 ;; esac
	test -d $2 || { echo $2: not a directory; exit 1; }
	case $0 in	/*) s=$0 ;;
			*)  s=`cd \`dirname $0\`; pwd`/`basename $0` ;; esac
	ln -s $s $2/utfs-server
	exit 0
esac

lf=`/bin/ls -l $0 | awk '{print $11}'`
ld=`dirname "$lf"`

exec 2>> $HOME/utfs-server.log
date +' -- start %c --' >&2

set -x
"$ld"/../utfs-server "$@"
