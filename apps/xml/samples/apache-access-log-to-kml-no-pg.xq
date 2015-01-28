declare variable $logfilename as xs:string external;

<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
 {
  for $res in ferris-doc( $logfilename )/*
	let $ip := data($res/@ip)
       return
    <Placemark> 
	<name>{ $ip }</name>
	<description>
		<br/>
			{ $ip }
		<br/>
			At: { data($res/@date) }
		<br/>
			user-agent: { data($res/@user-agent) }
		<br/>
		{
			let $r := data($res/@referer)
			where $r != "-"
			return <a href="{ $r }">{ $r }</a>
		}
	</description>
	<Point>
		<coordinates>
			X,Y
		</coordinates>
	</Point>
    </Placemark>
 }
</Document>
</kml> 
