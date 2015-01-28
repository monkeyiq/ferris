declare namespace libferris = "http://www.libferris.com"; 
declare function libferris:inet_aton 
  ( $ip as xs:string? )  as xs:string {
       
	let $toks := tokenize($ip,'\.')
	return
	(
	(($toks[1] cast as xs:integer)*256*256*256) +
	(($toks[2] cast as xs:integer)*256*256) +
	(($toks[3] cast as xs:integer)*256) +
	0 *	(($toks[4] cast as xs:integer)) 
	) cast as xs:string 
 } ;


declare variable $logfilename as xs:string external;



<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
 {
  for $res in ferris-doc( $logfilename )/*
	let $ip2 := "64.57.229.0"
	let $ip3 := "58.165.100.100"
	let $ip  := data($res/@ip)
	let $ipstart  := libferris:inet_aton( $ip )
	let $ipstartf := 983917568
	let $dbname := concat("/tmp/import/ipinfo.db/", $ipstart )
	let $ipinfo := ferris-doc-quiet( $dbname )
	where compare($ipinfo/@city,"") != 0
       return
    <Placemark> 
	<name>
	{ $ip } { $ipstart} dbname:{$dbname} 
	</name>
	<foo> { data($ipinfo/@city) } </foo>
    </Placemark>
 }
</Document>
</kml> 
