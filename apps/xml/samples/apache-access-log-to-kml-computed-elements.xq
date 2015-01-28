element kml
 {
  for $log in ferris-doc( "/home/ben/example-data/apache/fuuko.libferris.com/access.log" )
    for $res in $log/*
	let $ip := $res/@ip
	let $ipinfo := ferris-doc( concat( "pg://localhost/ipinfodb/ipgeo('", $ip, "')" ) )/*
	where compare($ipinfo/@city,"") != 0
    return
	element Placemark 
	{
		element name { data($res/@name) },
		element description 
		{
			"A hit from",data($ipinfo/@city),
			"<br/>",data($ip) 
		},
		element Point 
		{ 
			element coordinates { 
				concat(data($ipinfo/@long),",",data($ipinfo/@lat)) 
			} 
		}
	}
 }
