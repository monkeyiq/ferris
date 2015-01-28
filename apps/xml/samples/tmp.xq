

declare variable $logfilename as xs:string external;

<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
{
for $ip in distinct-values( ferris-doc( $logfilename )/*/@ip )
  return
    <Placemark> 
	<name>{ $ip }</name>
	<description>
		<br/>
			{ $ip }
		<br/>
		{
		for $res in ferris-doc( $logfilename )/*
		let $zz := $res/@ip
		where data($zz) = data($ip)
		return <p></p>
		}
	</description>
    </Placemark> 
}
</Document>
</kml> 
