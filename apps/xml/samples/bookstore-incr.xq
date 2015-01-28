declare revalidation skip;

let $n := doc("/tmp/bookstore.xml")/bookstore/book[1]/year/a
return 
   replace value of node $n with $n + 1


