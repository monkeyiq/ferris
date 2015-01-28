#!/usr/bin/perl

use Time::localtime;
use libferris;
$trunc = 0;
my @options = ();
push(@options,"trunc") if $trunc;
# tie( *FERRIS, 'libferris', '>/tmp/df', @options );
# print FERRIS "hello1\n";
# print FERRIS "hello2\n";
# print FERRIS ctime() , "\n";

# print "tell1:", tell(FERRIS), "\n";
# seek( FERRIS, -3, SEEK_END );
# print "tell2:", tell(FERRIS), "\n";
# print FERRIS "KK";
# # $l = <FERRIS>;
# # print $l , "\n";

# #@l = <FERRIS>;
# #print @l;

# untie( *FERRIS );
# print "end of program 1...\n";


# # use IO::WrapTie;
# # $WT = wraptie('libferris', ">/tmp/df2", @options );
# # $WT->print("Hello, world\n");  
# # $WT->fflush();
# # print "end of program 2...\n";


# tie( *FERRIS, 'libferris', '/tmp/test.xml/test/foo', @options );
# while( <FERRIS> ) {
#     print;
# }
# untie( *FERRIS );
# print "end of program 3...\n";

{
    tie( *FERRIS, 'libferris', '>printer://Cups-PDF/foo.pdf', @options );
    print FERRIS ctime() , "\n";
    untie( *FERRIS );
}
print "end of program 4...\n";

