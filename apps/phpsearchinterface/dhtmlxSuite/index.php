<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
    <title>Ferris Query</title>

 <SCRIPT language="JavaScript">
//
     
     function loadComplete()
{
    var ld=document.getElementById("loading").style;
    ld.visibility="hidden";
//    alert('done');
}
     
     function search( eaname, opcode, val )
{
    q='(' + eaname + opcode + val + ')';

//    alert(onlyfiles.v.checked);
    if( onlyfiles.v.checked )
    {
        q = '(&(is-file==1)' + q + ')';
    }
    
    q = q.replace(/&/g, "%26");
//    alert( q );

    var ld=document.getElementById("loading").style;
    ld.visibility="visible";
    mygrid.loadXML( "query.php?q=" + q, loadComplete );
}

function OnLoadPage()
{
    // set focus onto search box
    document.searchurlr.query.focus();
}

</SCRIPT>

  </head>

  <body onLoad="javascript:OnLoadPage()">

    <link rel="STYLESHEET" type="text/css" href="dhtmlxgrid.css">
    <script  src="/dhtmlxSuite/dhtmlxGrid/codebase/dhtmlxcommon.js"></script>
    <script  src="/dhtmlxSuite/dhtmlxGrid/codebase/dhtmlxgrid.js"></script>
    <script  src="/dhtmlxSuite/dhtmlxGrid/codebase/dhtmlxgridcell.js"></script>
    <script  src="/dhtmlxSuite/dhtmlxGrid/codebase/excells/dhtmlxgrid_excell_link.js"></script>

<form name="onlyfiles" action="">
     Only search for files:<input type="checkbox" name="v" value="1" checked="1" >
</form>

<table border="0" colums="2">
<!--
     <tr>
<td>Search by URL (simple)</td>
<td>
<form name="searchurl" action="javascript: search( 'url', '=~', '.*' + document.searchurl.query.value + '.*')">
     <input type='text' name='query'></form>
</td>
</tr>
     -->
     <tr>
<td>Search by URL (regex)</td>
<td>
<form name="searchurlr" action="javascript: search( 'url', '=~', document.searchurlr.query.value)">
     <input type='text' name='query'></form>
</td>
</tr>
     <tr>
<td>Modified within 30 days</td>
<td>
<form name="searchurl30d" action="javascript: search( 'mtime', '>=', '30 days ago')">
     <input type='submit' name='query'></form>
</td>
</tr>
     <?
     if( $_REQUEST["advanced"] )
     {
         print "     <tr>\n";
         print "<td>Direct query string</td>\n";
         print "<td>\n";
         print "<form name=\"searchdirect\" action=\"javascript: search( '', '', searchdirect.query.value)\">\n";
         print "     <input type='text' name='query'></form>\n";
         print "</td>\n";
         print "</tr>\n";
     }
?>
     
</table>


     
     <hr>
<div id="loading" style="position:absolute; width:100%; text-align:center; top:150px; visibility:hidden;">
<img src="loading.png" border=0></div>


<div id="gridbox" width="100%" height="800px" style="background-color:lightgrey;overflow:hidden"></div>
<script>
function serializeGrid(){
     mygrid.setSerializationLevel(true,true);
     var myXmlStr = mygrid.serialize()
     document.getElementById("mytextarea").value = myXmlStr;
 }

mygrid = new dhtmlXGridObject('gridbox');
mygrid.imgURL = "/dhtmlxSuite/dhtmlxGrid/codebase/imgs/";
mygrid.init();
//mygrid.loadXML( "query.php?q=(url=~aa)" );


</script>


  </body>
</html>
