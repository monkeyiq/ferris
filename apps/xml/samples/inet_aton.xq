
declare namespace libferris = "http://www.libferris.com"; 
declare function libferris:inet_aton 
  ( $ip as xs:string? )  as xs:string {
       
	let $toks := tokenize($ip,'\.')
	return
	(
	(($toks[1] cast as xs:integer)*256*256*256) +
	(($toks[2] cast as xs:integer)*256*256) +
	(($toks[3] cast as xs:integer)*256) +
	(($toks[4] cast as xs:integer)) 
	) cast as xs:string 
 } ;

let $ip2 := "64.50.6.0"
let $ip := ""
return libferris:inet_aton( $ip )
