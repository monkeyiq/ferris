<data>
 {
  for $log in ferris-doc( "/home/ben/example-data/apache/fuuko.libferris.com/head20.access.log" )
    for $res in $log/*
	let $ip := $res/@ip
	let $ipinfo := ferris-doc( concat( "pg://localhost/ipinfodb/ipgeo('", $ip, "')" ) )/*
       return
    <match 
	name="{ $res/@name }" ip="{ $ip }" 
        city="{ $ipinfo/@city }"
        long="{ $ipinfo/@long }" lat="{ $ipinfo/@lat }"
 	>
    </match>
 }
</data> 
