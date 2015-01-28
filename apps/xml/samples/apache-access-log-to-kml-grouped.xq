declare variable $logfilename as xs:string external;

<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
 {
  for $ip in distinct-values( ferris-doc( $logfilename )/*/@ip )
  let $ipinfo := ferris-doc( concat( "pg://localhost/ipinfodb/ipgeo('", $ip, "')" ) )/*
  where compare($ipinfo/@city,"") != 0
  return
    <Placemark> 
	<name>{ $ip }</name>
	<description>
		A hit from {data($ipinfo/@city)}
		<br/>
			{ $ip }
		<br/>
		{
		for $res in ferris-doc( $logfilename )/*[@ip = $ip]
		return <p>At: { data($res/@date) }
			{
				let $r := data($res/@referer)
				where $r != "-"
				return <a href="{ $r }">{ $r }</a>
			}
			</p>
		}
	</description>
	<Point>
		<coordinates>
			{concat(data($ipinfo/@long),",",data($ipinfo/@lat))}
		</coordinates>
	</Point>
    </Placemark>
 }
</Document>
</kml> 
