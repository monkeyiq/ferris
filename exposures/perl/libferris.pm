
package libferris;

use lib '/usr/local/lib';
use libferrisperl;

*Resolve = *libferrisperl::Resolve;
*getStrAttr = *libferrisperl::getStrAttr;
*StreamToString = *libferrisperl::StreamToString;
*pread = *libferrisperl::pread;
*pwrite = *libferrisperl::pwrite;
*tellgi = *libferrisperl::tellgi;
*eofi = *libferrisperl::eofi;
*fsync = *libferrisperl::fsync;
*ferriswrite = *libferrisperl::ferriswrite;
*goodio = *libferrisperl::goodio;
*tellgio = *libferrisperl::tellgio;
*tellp = *libferrisperl::tellp;
*seekgio = *libferrisperl::seekgio;
*seekp = *libferrisperl::seekp;

$ios::trunc = libferrisperl::get_ios_trunc();
$ios::in    = libferrisperl::get_ios_in();
$ios::out   = libferrisperl::get_ios_out();
$ios::ate   = libferrisperl::get_ios_ate();
$ios::app   = libferrisperl::get_ios_app();
$ios::beg   = libferrisperl::get_ios_beg();
$ios::cur   = libferrisperl::get_ios_cur();
$ios::end   = libferrisperl::get_ios_end();

#field test1 => 'aa';

sub TIEHANDLE { 
    print "<libferris>\n"; 
    my %i = ();
    bless \%i, 
    shift;

    $filename = shift;
    print "tie.fn1: $filename\n";
    @options = @_;

    $streamopts = $ios::out | $ios::in;
    print @options;
    $trunc = 0;
    if ( grep( /^trunc$/, @options ) ) {
	print "YES trunc:$trunc\n";
	$trunc = 1;
	$streamopts = $ios::trunc;
    }
    if( $filename =~ s/^>>//g ) {
	$streamopts = $ios::out | $ios::app;
	print "tie.fn3: $filename\n";
    }
    if( $filename =~ s/^>\|//g || $filename =~ s/^>//g ) {
	print "YES2 trunc\n";
	$trunc = 1;
	$streamopts |= $ios::out | $ios::trunc;
	print "tie.fn2.1: $filename\n";
	$fh = new IO::File "> $filename";
	print "tie.fn3: $filename\n";
	$c = libferrisperl::acquireContext( $filename, 0, 0 );
	undef($c);
	print "tie.fn4: $filename\n";
    }
    $c = Resolve( $filename );
    $oss = $c->getIOStream( $streamopts );

#    {
#	libferrisperl::setStrAttr( $c, "content", "fdsfsdxxxxxx2" );
	# $streamopts = 0;
	# $oss = $c->getIOStream();
	# libferrisperl::ferriswrite( $oss, "hjey" );
	# libferrisperl::fireCloseSig( $oss );
	# $oss = 0;
	
 #   }
    print "--------------------\n";
    $i{'fn'}   = $filename;
    $i{'c'}    = $c;
    $i{'oss'}  = $oss;
    $i{'lastop'}  = 'read';

    return \%i;
}

sub UNTIE {
    $i = shift; 
    $count = shift;
    $fn = $i->{'fn'};
    $lop = $i->{'lastop'};
    print "UNTIE count:$count lop:$lop fn:$fn\n";
    fsync( $i->{'oss'} );
    libferrisperl::fireCloseSig( $i->{'oss'} );
    undef($i->{'oss'});
    $c = Resolve( "/tmp/df" );
    $oss = $c->getIOStream();
    $i->{'oss'} = $oss;
    1;
}

sub DESTROY {
    # $i = shift; 
    # $fn = $i->{'fn'};
    print "</ferris-object> $fn\n" 
}
sub fflush {
    $i = shift;
    print "SYNC()\n";
    fsync( $i->{'oss'} );
}

sub EOF {
    $i = shift; 
    print "EOF()\n";
    eofio( $i->{'oss'} );
}

sub TELL {
    $i = shift; 
    if( $i->{'lastop'} eq 'read' ) {
	print "was read\n";
	tellgio( $i->{'oss'} );
    } else {
	tellp( $i->{'oss'} );
    }
}

sub SEEK {
    $i = shift; 
    $offset = shift;
    $whence = shift;
    $cppwhence = $ios::beg;
    if( $whence == SEEK_CUR ) {
	$cppwhence = $ios::cur;
    }
    if( $whence == SEEK_END ) {
	$cppwhence = $ios::end;
    }
    print "seek() offset:$offset cppwhence:$cppwhence\n";
    if( $i->{'lastop'} eq 'read' ) {
	seekgio( $i->{'oss'}, $offset, $cppwhence );
    } else {
	seekp( $i->{'oss'}, $offset, $cppwhence );
    }
}
sub PRINT { 
    $i = shift; 
    $i->{'lastop'}  = 'write';

    $data = join($,,@_);
    ferriswrite( $i->{'oss'}, $data );
}

sub PRINTF {
    $i = shift; 
    my $fmt = shift;
    $i->{'lastop'}  = 'write';

    $data = sprintf($fmt, @_);
    ferriswrite( $i->{'oss'}, $data );
}


sub READLINE {
    $i = shift; 
    $fn = $i->{'fn'};
    $i->{'lastop'}  = 'read';
#    print "readline.fn top fn:$fn\n";

    if (wantarray) 
    {
	print "want array...\n";
	# my @ret = ("all remaining\n",
	# 	   "lines up\n",
	# 	   "to eof\n");
	my @ret = ();
	my $ss = $i->{'oss'};
	while( goodio( $ss ) ) {
	    $str = libferrisperl::readline( $ss );
	    if( goodio( $ss ) ) {
		push(@ret, "$str\n" );
	    }
	}
	return @ret;
    }
    else
    {
	if( libferrisperl::eofio( $i->{'oss'} ) ) {
	    return undef;
	}
	return libferrisperl::readline( $i->{'oss'} ) . "\n";
    }
}

1;
