<?
$STYLESHEET="ferrisls-xml-to-dhtmlxSuite.xsl";
$restriction="filter";
$restriction="filter-100";
$showea=escapeshellarg("url,name,name-only,size,size-human-readable,mtime,mtime-display,parent-url");
$q="(url=~8)";
$q=$_REQUEST["q"];


//phpinfo();

header('Content-type: text/xml');

$cmd="/usr/local/bin/ferrisls --xml --hide-xml-declaration ";
$cmd.=" --show-ea=$showea ";
$cmd.=escapeshellarg("eaquery://$restriction/$q");

//print "q:$q\n";
//print "cmd:$cmd\n";

$xmldata = shell_exec( $cmd );

$xslt = new xsltProcessor;
$xslt->importStyleSheet(DomDocument::load($STYLESHEET));
print $xslt->transformToXML(DomDocument::loadXML($xmldata));

?>



      
