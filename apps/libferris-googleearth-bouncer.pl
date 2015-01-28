#!/usr/bin/perl -w

print "libferris-googleearth-bouncer.pl called!\n";
$cmd=$ARGV[0];
print "cmd:$cmd\n";
if( $cmd =~ s|^file:///LIBFERRIS-SEARCH-EMBLEM-BY-ID(.*)|$1|) 
  {
    $sidePanelToShow = "";
    $radius = 0;
    $majorMime = "";
    @extraColumnsToView = ("size-human-readable","mtime-display");

    $cmd = lc( $cmd );
    print "TOP cmd:$cmd\n";

    if( $cmd =~ s@^-radius([0-9]+)([%-].*)$@$1,$2@) {
      print "OO cmd:$cmd\n";
      ($radius,$cmd) = split /,/, $cmd;
      print "R radius:$radius cmd:$cmd\n";
    }

    if( $cmd =~ s@^-(image|video|text)-only%20([0-9]+)$@$1,$2@) {
      ($majorMime,$emblemid) = split /,/, $cmd;
    }
    if( $cmd =~ s|^%20([0-9]+)$|$1|) {
      $emblemid = $cmd;
    }

    if( $emblemid =~ m/^$/ ) {
      print "No emblem ID found!\n";
      exit;
    }

    if( $radius > 0 ) {
      @emblemlist = split /,/, `ferris-get-emblems-within-range -i $emblemid -A $radius -O $radius -k`;
    }

    
    print "ok emblem:$emblemid majorMime:$majorMime\n";
    $cmd = "(ego-read-dir \"eaq://(emblem:id-$emblemid==1)\")";
    
    $q = "";
    if( length $majorMime ) {
      $t = "image";
      if( $majorMime eq "video" ) {
	$t = "animation";
	$sidePanelToShow = "HotActions";
	push @extraColumnsToView, ("width","height");
      }
      if( $majorMime eq "image" ) {
	$t = "image";
	$sidePanelToShow = "HotActions";
	push @extraColumnsToView, ("width","height");
      }
      if( $majorMime eq "text" ) {
	$q = $q . "(&(!(is-animation-object==1)(is-image-object==1)(is-audio-object==1))";
      }
      else {
	$q = $q . "(&(is-$t-object==1)";
      }
    }
    if( scalar(@emblemlist) > 0 ) {
      $q = $q . "(|";
      foreach $emblemid (@emblemlist) {
	chomp ($emblemid);
	$q = $q . "(emblem:id-$emblemid==1)";
      }
      $q = $q . ")";
    }
    else {
      $q = $q . "(emblem:id-$emblemid==1)";
    }
    if( length $majorMime ) {
      $q = $q . ")";
    }

    $cmd =        "(define gec ( new-cview \"GTreeView\" ))\n";
    $cmd = $cmd . "(cview-set-root-url gec \"eaq://$q\")\n";
    foreach $col ( @extraColumnsToView ) {
      $cmd = $cmd . "(cview-addColumn gec \"$col\")\n";
    }
    if( length $sidePanelToShow > 0 ) {
      $cmd = $cmd . "(cview-show-sidepanel-page gec \"$sidePanelToShow\")\n";
    }
    $cmd = $cmd . "(cview-set-visible gec #t )\n";

    print "extraColumnsToView:@extraColumnsToView\n";
    print $cmd;
    open( EGO, "| Ego-send-command" );
    print EGO "$cmd\n";
  }
else
  {
    $brow = $ENV{LIBFERRISBROWSER};
    if( $brow =~ m/^$/ ) {
      print "no browser set!\n";
      $brow="firefox";
    }
    print "browser:$brow\n";
    $flatarg = join " ", @ARGV;
    print "flat:$flatarg\n";
    system "$brow $flatarg";
  }
print "CALLED WITH @ARGV" #>>/tmp/outtt
