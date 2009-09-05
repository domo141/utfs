#!/usr/bin/perl

use strict;
use warnings;

use Socket;

mkdir 'test-dot' unless -d 'test-dot';

socketpair(C, P, AF_UNIX, SOCK_STREAM, PF_UNSPEC) or die "socketpair: $!";

#exec qw(echo foo);

my $pid;
if ($pid = fork) {
    close C;
    open STDIN, "<&P" or die;
    open STDOUT, "<&P" or die;
    close P;
    #exec qw(ssh remote utfs-server -v -l foo -r bar -u 0:1 utfs-test);
    exec qw(./utfs-server -v -l foo -r bar -u 0:1 .);
}
else {
    close P;
    open STDIN, "<&C" or die;
    open STDOUT, "<&C" or die;
    close C;
    # dup2(1, 3) (using perl syntax) if removing '-d' below is desired.
    exec qw(./utfs-client -v -l bar -r foo -u 0:1 test-dot -d);
}
