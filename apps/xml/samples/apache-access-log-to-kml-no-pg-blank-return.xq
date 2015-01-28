<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
 {
  for $res in ferris-doc( "/home/ben/example-data/apache/fuuko.libferris.com/access.log" )/*
	let $ip := data($res/@ip)
       return
    <Placemark> 
    </Placemark>
 }
</Document>
</kml> 
