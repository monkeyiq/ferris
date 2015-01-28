<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
    <title>Ferris Query</title>

    <script language="JavaScript">

     function search( eaname, opcode, val )
     {
         q='(' + eaname + opcode + val + ')';
         if( onlyfiles.v.checked )
         {
             q = '(&(is-file==1)' + q + ')';
         }
         q = q.replace(/&/g, "%26");
         earl = "runquery-simple.php?q=" + q;
//         alert(earl);
         results.src = earl;
     }

     function OnLoadPage()
     {
         // set focus onto search box
         document.searchurlr.query.focus();
     }

    </script>
  </head>

  <body onLoad="javascript:OnLoadPage()" bgcolor="lightgrey">

  <form name="onlyfiles" action="">
     Only search for files:<input type="checkbox" name="v" value="1" checked="1" >
  </form>

  <table border="0" colums="2">
  <tr>
     <td>Search by URL (regex)</td>
     <td>
     <form name="searchurlr" action="javascript: search( 'url', '=~', document.searchurlr.query.value)">
         <input type='text' name='query'></form>
     </td>
  </tr>
  <tr>
     <td>Search full text</td>
     <td>
     <form name="searchurlftx" action="javascript: search( 'ferris-fulltext-search', '==', document.searchurlftx.query.value)">
         <input type='text' name='query'></form>
     </td>
  </tr>
  </table>

     <hr/>
     <iframe id="results" src ="runquery-simple.php" width="100%" height="100%" />

</body>     
</html>
     
     
