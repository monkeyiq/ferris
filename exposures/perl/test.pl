#!/usr/bin/perl

use lib '/usr/local/lib';
use libferrisperl;

#libferrisperl->import(qw(Resolve));

*Resolve = *libferrisperl::Resolve;
*getStrAttr = *libferrisperl::getStrAttr;
*StreamToString = *libferrisperl::StreamToString;
*pread = *libferrisperl::pread;
*tellgi = *libferrisperl::tellgi;
*eofi = *libferrisperl::eofi;

my $c = Resolve("/tmp/df");
my $earl = $c->getURL();
print "is dir:$earl xx\n";
print getStrAttr( $c, "content", "", 1 );
# libferrisperl::setStrAttr( $c, "content", "foo\n" );

$nn = "content";
my $a = $c->getAttribute( "content" );
my $iss = $a->getIStream();

my $str = "";

#$str = StreamToString($iss);
$str = pread( $iss, 100, 0 );
print chomp( $str ) . "\n";
print "have iss/str: $str:\n";
print "tellg:" . tellgi( $iss ) . "\n";
print "eof:"   . eofi( $iss ) . "\n";

# $str = "undef";
# libferrisperl::getline( $iss, $str, "\n" );
# print "have getline/str: $str\n";

print "end of program...\n";
