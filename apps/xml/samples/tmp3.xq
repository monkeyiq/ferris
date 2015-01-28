<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
{
let $a := "<blah>"
let $b := "aa ]]> bb"
return
   <base>
   <description>
	{$a} &lt;foo&gt; {$a} {$a} &lt;/foo&gt;
   </description>
   <breakme>
	{$b}
   </breakme>
   </base>
}
</Document>
</kml> 
