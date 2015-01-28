declare variable $docurl     := "customers.db";
declare variable $customerid := "131";
<resultdata>
 {
  for $c in ferris-doc( $docurl )/customers/*[@id=$customerid]
  return
    <person cid="{ $c/@id }" surname="{ $c/@familyname }" />
 }
</resultdata>
