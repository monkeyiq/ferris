declare variable $docurl     := "file:///ferris/apps/xml/samples/customers.xml";
declare variable $customerid := "131";
<resultdata>
 {
  for $c in doc( $docurl )/customers/*[@id=$customerid]
  return
    <person cid="{ $c/@id }" surname="{ $c/@familyname }" />
 }
</resultdata>
