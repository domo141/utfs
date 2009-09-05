#!/usr/bin/perl
# -*- cperl -*-

use strict;
use warnings;

my $default;

while (<>) {
    $default = $1 if /edefault=(\d+)/;
    last if /const unsigned char utfs_client_errmap.*=/;
}

die "Default not defined (have -DEDEFAULT_FOR_PP)\n" unless defined $default;

my @table;
$table[$_] = $default foreach (0..255);

my $c = 0;
while (<>)
{
    last if /};/;
    $table[$1] = $c, $c++ if /^\s*(\d+)/;
}

die "Table too large -- too big errno codes in input\n" if @table != 256;

my $last = pop @table;

print "/*\n * generated from utfs-client-errmap.c. do not edit\n */\n";
print qq(\n#include "../utfs-errmaps.h"\n\n);

$c = 0;
print "const unsigned char utfs_server_errmap[] = {";
foreach (@table) { print "\n" unless $c++ % 16; printf " %3d,", $_; }
printf " %3d\n};\n\n", $last;

print 'const unsigned int utfs_server_errmap_size =
    sizeof utfs_server_errmap / sizeof utfs_server_errmap[0];', "\n";
