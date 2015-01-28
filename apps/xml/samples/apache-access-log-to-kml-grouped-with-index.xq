

declare variable $logfilename as xs:string external;

<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
{
let $map := for $hit in ferris-doc( $logfilename )/*
            let $nodename := concat("n",data($hit/@ip))
            return element {$nodename} {
                <hit foo="bar"
		ip="{data($hit/@ip)}"  >
		<date>{data($hit/@date)}</date>
		<referer>{data($hit/@referer)}</referer>
		</hit>}
for $ip in distinct-values(ferris-doc( $logfilename )/*/@ip)
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
		for $res in $map//.[@ip = $ip]
		return <p>At: { data($res/date) }
			{
				let $r := data($res/referer)
				where $r != "-"
				return <a href="{ $r }">{ $r }</a>
			}
			</p>
		}
	</description>
    </Placemark> 
}
</Document>
</kml> 
