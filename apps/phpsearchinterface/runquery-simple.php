<?php
$STYLESHEET="xml-results-to-xhtml.xsl";
$restriction="filter";
$restriction="filter-100";
$showea=escapeshellarg("url,name,size,size-human-readable,mtime,mtime-display,parent-url");
$q="(url=~8)";
$q=$_REQUEST["q"];

header('Content-type: text/xml');
print "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
print "<?xml-stylesheet href=\"$STYLESHEET\" type=\"text/xsl\"?>\n";

$cmd="/usr/local/bin/ferrisls --xml --hide-xml-declaration ";
$cmd.=" --show-ea=$showea ";
$cmd.=escapeshellarg("eaquery://$restriction/$q");

system( $cmd );
?>
