#!/usr/bin/perl -w

#
# A little glue to open a context's lat/long point in a new firefox tab
#

$uid=`id -u`;
chomp $uid;
$tmpfile="/tmp/libferris-google-earth-glue-$uid.kml";
print "tmpfile:$tmpfile\n";

open(OSS,">$tmpfile");
print OSS "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
print OSS "<kml xmlns=\"http://earth.google.com/kml/2.1\">\n";
print OSS "<Folder>\n";

sub makeLocation {
    my $line = shift;
    print "line:$line\n";

    ($long,$lat,$zoom,$emblemid,$stamp,$fname,$earl) = split /,/, $line;
    print "name:$fname stamp:$stamp lat:$lat long:$long zoom:$zoom id:$uid emblemid:$emblemid\n";

    if( $lat == 0 || $long == 0 ) 
    {
	print "No map location for this context:$fname\n"
    }
    else
    {
	print "ok\n";

	$name=`basename $fname`;
	print OSS "<Placemark>\n";
	print OSS "	<name>$name</name>\n";
	if( $stamp =~ /T/ ) {
	    print OSS "<TimeStamp>\n";
	    print OSS "  <when>$stamp</when>\n";
	    print OSS "</TimeStamp> \n";
	}
	print OSS "      <description>\n";
	print OSS "<![CDATA[\n";
	if( $fname =~ /.jpg/ ) {
	    print OSS "      <img src=\"$earl\" width=\"320\" height=\"200\" /><br/>\n";
	}
	print OSS "Search nearby (5km) for:\n";
	print OSS "<a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS5 $emblemid\">files</a>,\n";
	print OSS "<a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS5-IMAGE-ONLY $emblemid\">image files</a>,\n";
	print OSS "<a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS5-VIDEO-ONLY $emblemid\">video files</a>.\n";
	print OSS "<br><hr>Search for\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID $emblemid\">files with emblem</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-IMAGE-ONLY $emblemid\">image files with emblem</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-VIDEO-ONLY $emblemid\">video files with emblem</a>\n";
	print OSS "<br><hr>Search near (15km) for\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS15 $emblemid\">files</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS15-IMAGE-ONLY $emblemid\">image files</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS15-VIDEO-ONLY $emblemid\">video files</a>\n";
	print OSS "<br><hr>Search around (50km) for\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS50 $emblemid\">files</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS50-IMAGE-ONLY $emblemid\">image files</a>\n";
	print OSS "<br><a href=\"LIBFERRIS-SEARCH-EMBLEM-BY-ID-RADIUS50-VIDEO-ONLY $emblemid\">video files</a>\n";
	print OSS "]]></description>\n";
	print OSS "	<LookAt>\n";
	print OSS "         <longitude>$long</longitude>\n";
	print OSS "         <latitude>$lat</latitude>" ;
	print OSS "         <range>$zoom</range>" ;
	print OSS "	</LookAt>\n";
	print OSS "      <Point>\n";
	print OSS "         <coordinates>$long,$lat,0</coordinates>\n";
	print OSS "      </Point>\n";
	print OSS "</Placemark>\n";
    }
}

$cmd = "ferrisls -ld --show-ea=longitude,latitude,geospatial-range,geospatial-emblem-id,exif:date-time-original-xml,name,url --field-seperator=\",\" " ;
while( $v = pop ) {
    $cmd .= " \"$v\" ";
}
$cmd .= " | ";
print "ferrisls cmd:$cmd\n";
   
open FERRISLS, $cmd;
while( <FERRISLS> )
{
    makeLocation $_;
}

print OSS "</Folder>\n";
print OSS "</kml>\n";
    
$cmd="libferris-googleearth $tmpfile";
print "command: $cmd\n";
exec $cmd
