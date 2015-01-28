<html>
  <head>

<meta http-equiv="content-type" content="text/html; charset=utf-8">
     <link rel="stylesheet" type="text/css" href="yui/assets/yui.css" >
     <link rel="stylesheet" type="text/css" href="yui/assets/dpSyntaxHighlighter.css">
     <link type="text/css" rel="stylesheet" href="yui/build/cssfonts/fonts-min.css" />
     <link rel="stylesheet" href="yui/build/cssreset/cssreset.css" type="text/css">
     <link rel="stylesheet" href="yui/build/cssfonts/cssfonts.css" type="text/css">
     <link rel="stylesheet" href="yui/build/cssgrids/cssgrids.css" type="text/css">
     <script src="yui/build/yui/yui-min.js" charset="utf-8"></script>

    <style type="text/css">

     .mtime { font-family: "Andale Mono", monospace; white-space: pre; }
     .protectionls { font-family: "Andale Mono", monospace; }
     .upper { text-transform: uppercase; }

.yui3-g .content {
    border: 2px solid #000;
    margin-right:10px; /* "column" gutters */
    padding: 1em;
}
     div.yui3-g {
    text-align:center;
    vertical-align:middle;
}
.tabcontent {
    position: relative;
    text-align: center;
    border:solid #ccc;
    border-width:1px;
    color:#000;
    margin: 15px 10px 15px 0; /*10px 10px 0 0; "column" gutters */
    padding:5px 2px;
    z-index: 1;
}

a:link, a:visited
{
  color: #0338AF;
}

#annotationeditor {
    height: 500px;
}

#shella {
background-color: #cccccc;
height: 100%;
}

#shell {
background-color: #cccccc;
height: mainarea.height;
}


#log .yui3-console .yui3-console-title {
    border: 0 none;
    color: #000;
    font-size: 15px;
    font-weight: bold;
    margin: 0;
    text-transform: none;
}


#layout {
    padding-left:300px; /* "left col" width */ 
    padding-right:150px; /* "right col" width */
height: 200px;

}

#nav {
    margin-left:-300px; /* "left col" width */
    width:300px;
    border: 2px solid #000;
}

#extra {
    width:150px;
    margin-right:-150px; /* "right col" width */
    border: 2px solid #F00;
}

#main {
    width: 100%;
    border: 2px solid #0F0;
}
     
    </style>

          
    <script>
<?php
    $performingQuery = 0;
     $dirname = "/";
     if( isset($_GET["dirname"] ))
         $dirname = $_GET["dirname"];
     if( $dirname == "" )
         $dirname = "/";

     $searchEAPredPrefix = "(url=~";
     if( isset($_GET["searchEAPredPrefix"] ))
         $searchEAPredPrefix = $_GET["searchEAPredPrefix"];
     $searcheaquery = "";
     if( isset($_GET["searcheaquery"] ))
         $searcheaquery = $_GET["searcheaquery"];
     if( $searcheaquery != "" && $searchEAPredPrefix != "" )
     {
         $dirname = "eaquery://filter-500/" . $searchEAPredPrefix . $searcheaquery . ")";
         $performingQuery = 1;
     }

     if( isset( $_GET["searcheaquery"] ))
         $searcheaquery = $_GET["searcheaquery"];

?>
     var dirname = '<?php echo $dirname ?>';
     var performingQuery = '<?php echo $performingQuery ?>';

     function setCookie(c_name,value,exdays)
     {
         var exdate=new Date();
         exdate.setDate(exdate.getDate() + exdays);
         var c_value=escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
         document.cookie=c_name + "=" + c_value;
     }
     function getCookie(c_name)
     {
         var i,x,y,ARRcookies=document.cookie.split(";");
         for (i=0;i<ARRcookies.length;i++)
         {
             x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
             y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
             x=x.replace(/^\s+|\s+$/g,"");
             if (x==c_name)
             {
                 return unescape(y);
             }
         }
     }
     function endsWith(str, suffix)
     {
         return str.indexOf(suffix, str.length - suffix.length) !== -1;
     }

     </script>
    
  </head>
  <body class="yui3-skin-sam  yui-skin-sam">
  <div id="shella">
  <div id="shell">

<!--
         Welcome <?php echo $_GET["dirname"]; ?>.<br /> 
         Welcome <?php echo $dirname; ?>.<br />
-->

<?php
    function selflink( $extra )
    {
        return "index.php?one=two&dirname=/&a=b&$extra";
    }
    function parentdir( $earl )
    {
        if( $earl == "~" )
            return "/home";
        
        return preg_replace("/\\/[^\\/]*$/i","",$earl);
    }

    function mi( $link, $label, $idval = "" ) 
    {
        $ret = "<li ";
        $ret = $ret . " id=\"" . $idval . "\" ";
        $ret = $ret . " class=\"yui3-menuitem\"><a class=\"yui3-menuitem-content\" href=\"" . $link . "\">"
            . $label . "</a></li>";
        return $ret;
    }

    $parentearl = parentdir($dirname);
    if( $parentearl == "" )
        $parentearl = "/";


?>




    <div id="menubar" class="yui3-menu yui3-menu-horizontal yui3-menubuttonnav keepopen ">
        <div class="yui3-menu-content">
            <ul>

              <li>
                <?php echo mi(selflink("dirname=$parentearl"),'<img src="images/up.svg"/>'); ?>
              </li>
              <li>
                <?php echo mi("javascript:void(0);",'<img id="goto" src="images/folder.svg"/>'); ?>
              </li>
              <li>
                <?php echo mi(selflink("dirname=~"),'<img src="images/home.svg"/>'); ?>
              </li>
              <li>
                <?php echo mi(selflink("dirname=bookmarks://"),'<img src="images/bookmark.svg"/>'); ?>
              </li>
              
<!--     
              <li>
              <a class="yui3-menu-label" href="#new-finance">News &#38; Finance</a>
                        <div id="new-finance" class="yui3-menu">
                            <div class="yui3-menu-content">
                                <ul>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://buzz.yahoo.com/">Buzz</a></li>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://finance.yahoo.com">Finance</a></li>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://news.yahoo.com">News</a></li>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://publisher.yahoo.com">Publisher Network</a></li>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://sports.yahoo.com/">Sports</a></li>
                                    <li class="yui3-menuitem"><a class="yui3-menuitem-content" href="http://weather.yahoo.com/">Weather</a></li>
                                </ul>
                            </div>
                        </div>
              </li>
-->
        
            </ul>
         </div>
    </div>
        
<div class="yui3-g" id="mainarea">
<div class="yui3-u-1-5">
<div class="tabcontent">    
<div id="tabs">
    <ul>
        <li><a href="#tabannotation">Annotation</a></li>
        <li><a href="#tabsearchea">Search Metadata</a></li>
        <li><a href="#tabtag">Tag</a></li>
        <li><a href="#tabedit">Edit</a></li>
    </ul>
    <div>
        <div id="tabannotation">
            <div id="annotationeditor"></div>
        </div>
        <div id="tabsearchea">
           <div id="searchea"></div>
        
            <div id="searcheatype" class="yui3-menu yui3-menu-horizontal">
              <div id="fooAA" class="yui3-menu-content">
                <ul>
                  <li>
                    <a id="searcheabymenuA" class="yui3-menu-label" href="javascript:setupsearchbyopen()"><em>Search By</em></a>
                    <div id="submenu-1" class="yui3-menu">
                      <div id="searcheabymenu" class="yui3-menu-content">
                        <ul>
                           <?php echo mi("javascript:setupsearchby('(url=~','');",'Regex on URL'); ?>
                           <?php echo mi("javascript:setupsearchby('(ferris-ftx=~','');",'Query on Text Content'); ?>
                           <?php echo mi("javascript:setupsearchby('(mtime>=','begin this ')",  'Modified >= than this...'); ?>
                           <?php echo mi("javascript:setupsearchby('(mtime>=','begin last ')",  'Modified >= than last...'); ?>
                           <?php echo mi("javascript:setupsearchby('(mtime>=','   months ago')",'Modified >= X months ago'); ?>
                        </ul>
                      </div>
                    </div>
                  </li>
                </ul>
              </div>
                        
              <form action="/ferris/index.php?foo=bar" id="searcheaform">
                 <fieldset>
                  <p>
                    <label for="searcheaquery">Query</label>
                    <input type="text"   name="searcheaquery" id="searcheaquery" placeholder="" value="<?php echo $searcheaquery; ?>">
                    <input id="searchEAPredPrefixForm" type="hidden" name="searchEAPredPrefix" id="searchEAPredPrefix" value="<?php echo $searchEAPredPrefix; ?>">
<!--
                    <input type="hidden" name="dirname" id="dirname" value="<?php echo $dirname; ?>">
                    <input type="submit">
-->
                  </p>
                 </fieldset>
              </form>
<br/>                        
<br/>                        
            </div>
        </div>
        <div id="tabtag">
           <div id="tag">
              <form action="javascript:addTag()" id="tagform">
                 <fieldset>
                  <p>
                    <label for="tagac">Add Tag:</label><br>
                    <input id="tagac" type="text" name="tagac">
                    <input type="hidden" name="dirname" id="dirname" value="<?php echo $dirname; ?>">
                    <input type="submit" value="add">
                  </p>
                 </fieldset>
              </form>
        <br/>Show and Remove Tags...
            <div id="filetags"></div>
           </div>
        </div>
        <div id="tabedit">
          <div id="editpanel">
            <div id="textedit">
            </div>
          </div>
        </div>
        
    </div>
</div>
           <div id="log"></div>
</div>
</div>
        
    <div class="yui3-u-4-5">
      <div class="tabcontent">    
         <div id="tree"></div>
      </div>
    </div>

</div>
        
    <div id="gotopanel">
      <div class="yui3-widget-bd">
          <form>
             <fieldset>
                <p>
                    <label for="dirname">URL</label><br/>
                    <input type="text" name="dirname" id="dirname" placeholder="">
                </p>
            </fieldset>
        </form>
      </div>
    </div>

        
<!--    end... <br/> -->

  </div> <!-- shell -->
  </div> <!-- shella -->
  </body>

    <script>
      var restBase = "/~libferrissearch/cgi-bin/rest";
      var dirname = '<?php echo $dirname; ?>';
      var filetags;
      var tabview;

      function sortStringOrEmpty(col, a, b, desc)
      {
          var aa = a.get(col);
          var bb = b.get(col);
          var order = 0;
          if( aa == undefined )
              order = 1;
          else if( aa == undefined && bb == undefined )
              order = 0;
          else if( bb == undefined )
              order = -1;
          else
              order = (aa > bb) ? 1 : -(aa < bb);
          return desc ? -order : order;
      }

      function sortNumeric(col, a, b, desc)
      {
          var aa = a.get(col);
          var bb = b.get(col);
          var order = (+aa > +bb) ? 1 : -(+aa < +bb);
          return desc ? -order : order;
      }

      function namelink( o )
      {
        var path = dirname + "/" + o.data['name'];
        if( o.data['is-dir'] == '1' )
        {
           o.value = '<a href="index.php?dirname=' + path + '">' + o.value + '</a>';
        }
        else 
        {
            o.value = '<a href="' + restBase + '?method=read&ea=content&path=' + path + '">' + o.value + '</a>';
        }
      }

      function tagdellink( o )
      {
        var tag = o.data['name'];
        o.value = '<a href="javascript:removeTag(\'' + tag + '\');">' + tag + '</a>';
      }

      function valueFormat( o ) {
          return o.value;
          o.value = '<a href="xxx">' + o.value + '</a>';
      }

      function stripHTML( s ) 
      {
          var stripHTML = /<\S[^><]*>/g;
          var ret = s.replace(/<br>/gi, '\n').replace(stripHTML, '');
          return ret;
      }

      function editableEAFormatter( eaname, o )
      {
          o.value = "";
          if( o.data[eaname] )
          {
              o.value = '<a href="javascript:edit(\'' + o.data.url + '\',\'' + o.data.id + '\',\'' + eaname + '\')">' + 'edit' + '</a>';
              o.value = o.value + ' ' + o.data[eaname];
          }
      }

      var oldselectedearl = "";
      var oldselectedid = "";
      var oldannotation = "";
      var annotationeditor;
      var tree;
      var YInstance;
      function getCurrentAnnotationSidepanelTextNoHTML()
      {
          var Y = YInstance;
          var editor = annotationeditor;
          var DATA = editor.getContent();
          return( stripHTML( DATA ));
      }

      function writeAnnotation( oldselectedearl ) 
      {
          var Y = YInstance;
          var editor = annotationeditor;
          if( oldselectedearl != "" )
          {
              var DATA = getCurrentAnnotationSidepanelTextNoHTML();
              if( DATA != oldannotation )
              {
                  var putearl = restBase + '?method=write&ea=annotation&path='
                      + oldselectedearl
                      + '&zz=top';
                  Y.log("NEWDATA:" + DATA, 'info');
                  Y.log("OLDDATA:" + oldannotation, 'info');
                  Y.log('put: ' + putearl + '','info');

                  Y.io(putearl, {
                        method: 'POST',
                              data: DATA,
                              on: {
                            complete: function(id, e) {
                                  Y.log("put complete...", 'info');
                                  oldannotation = DATA;
//                                  var table = Y.one('#tree');
                                  tree.modifyRow( oldselectedid, { annotation: DATA } );
                              }
                          }
                      });
              }
          }
      }
      function writeAnnotationTimer() 
      {
          var Y = YInstance;
//          Y.log("writeAnnotationTimer() earl:" + oldselectedearl, 'info');
          writeAnnotation( oldselectedearl );
          setTimeout("writeAnnotationTimer()", 2000);
      }

      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////

      var editea;
      var oldtext;
      var texteditor;
      var texteditorearl;
      var texteditorid;

      function writeTextEditor( oldselectedearl ) 
      {
          var Y = YInstance;
          var editor = texteditor;
          if( oldselectedearl != "" )
          {
              var DATA = stripHTML(editor.getContent());
              if( DATA != oldtext )
              {
                  var putearl = restBase + '?method=write&ea=' + editea + '&path='
                      + oldselectedearl
                      + '&zz=top';
                  Y.log("NEWDATA:" + DATA, 'info');
                  Y.log("OLDDATA:" + oldtext, 'info');
                  Y.log('put: ' + putearl + '','info');

                  Y.io(putearl, {
                        method: 'POST',
                              data: DATA,
                              on: {
                            complete: function(id, e) {
                                  Y.log("put complete...", 'info');
                                  oldtext = DATA;
                                  updateObj=new Object();
                                  updateObj[editea] = DATA;
                                  tree.modifyRow( texteditorid, updateObj );
                              }
                          }
                      });
              }
          }
      }
      function writeTextEditorTimer() 
      {
          var Y = YInstance;
          writeTextEditor( texteditorearl );
          setTimeout("writeTextEditorTimer()", 2000);
      }
      function edit( earl, id, ea )
      {
          editea = ea;
          texteditorearl = earl;
          texteditorid   = id;
          var Y = YInstance;
          tabview.selectChild(3);
          var getearl = restBase + '?method=read&ea=' + editea + '&path=' + earl + '&zz=top';
          Y.log('select(' + getearl + ')','info');
          Y.io(getearl, {
                on: {
                    complete: function(id, e) {
                          var t = e.responseText;
                          var thtml = t.replace(/\n/g,'<br/>');
                          
                          texteditor.set("content", thtml );
                          oldtext = stripHTML(texteditor.getContent());
                          Y.log("read attribute:" + editea + " value:" + t, 'info');
                          setTimeout("writeTextEditorTimer()", 2000);
                    }
                }
          });
      }

      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////

      function select( earl, id )
      {
          var Y = YInstance;
          var editor = annotationeditor;
          Y.log("earl:" + earl, 'info');

          /////////////////////
          // update annotations
          //
          writeAnnotation( oldselectedearl );
          var getearl = restBase + '?method=read&ea=annotation&path=' + earl + '&zz=top';
          Y.log('select(' + getearl + ')','info');
          Y.io(getearl, {
                on: {
                    complete: function(id, e) {
                          var t = e.responseText;
                          var thtml = t.replace(/\n/g,'<br/>');
//              DATA = DATA.replace(/<br>/gi, '\n').replace(stripHTML, ''); 
                          editor.set("content", thtml );
                          oldannotation = getCurrentAnnotationSidepanelTextNoHTML();
                          Y.log("read annotation:" + t, 'info');
                    }
                }
          });

          /////////////////////
          // update emblems
          //
//          var filetags = Y.one('#filetags');
          filetags.datasource.load({
                request: "&path=" + earl,
          });
          
          /////////////////////
          // remember current state
          //
          oldselectedearl = earl;
          oldselectedid = id;
      }
      function selectFormat( o ) {
          o.value = "";
          
          if( o.data['is-dir']=="0" && o.data['is-dir-try-automounting']=="1" )
              o.value = o.value + '<a href="index.php?dirname=' + o.data.url + '">' + 'read' + '</a>';
          
          o.value = o.value + ' <a href="javascript:select(\'' + o.data.url + '\',\'' + o.data.id + '\')">' + 'row' + '</a>';
//          if( endsWith( o.data['name'], ".txt" ))
          o.value = o.value + ' <a href="javascript:edit(\'' + o.data.url + '\',\'' + o.data.id + '\',\'content\')">' + 'edit' + '</a>';
      }

      ///////////////////////////////////////
      ///////////////////////////////////////
      ///////////////////////////////////////

      function removeTag( tag )
      {
          var Y = YInstance;
          Y.log("removeTag():" + tag, 'info' );

          var emblemName = tag;
          var putearl = restBase + '?method=file-emblems-remove&path='
              + oldselectedearl
              + '&emblem=' + emblemName
              + '&zz=top';
          Y.log('put: ' + putearl + '','info');
          
          Y.io(putearl, {
                method: 'PUT',
                      on: {
                          complete: function(id, e) {
                              Y.log("put complete...", 'info');
                              filetags.datasource.load({
                                    request: "&path=" + oldselectedearl,
                              });
                          }
                      }
              });
          
      }
      function addTag() 
      {
          var Y = YInstance;
          Y.log("addTag()", 'info' );
          var tag = document.forms["tagform"].elements["tagac"].value;
          Y.log("tag:" + tag, 'info' );

          var emblemName = tag;
          var putearl = restBase + '?method=file-emblems-add&path='
              + oldselectedearl
              + '&emblem=' + emblemName
              + '&create=' + '1'
              + '&zz=top';
          Y.log('put: ' + putearl + '','info');
          
          Y.io(putearl, {
                method: 'PUT',
                      on: {
                          complete: function(id, e) {
                              Y.log("put complete...", 'info');
                              filetags.datasource.load({
                                    request: "&path=" + oldselectedearl,
                              });
                          }
                      }
              });

      }

      ///////////////////////////////////////
      ///////////////////////////////////////
      ///////////////////////////////////////

      var searchEAPredPrefix = '(url=~';
      function setupsearchby( searchPredPrefix, query )
      {
          var Y = YInstance;
          searchEAPredPrefix = searchPredPrefix;
          document.forms['searcheaform'].elements['searchEAPredPrefixForm'].value = searchPredPrefix;

////          Y.one("#submenu-1").hide();
//          setTimeout('YInstance.one("#submenu-1").show()', 2000);
//          Y.one("#submenu-1").show();
          Y.one("#searcheaquery").focus();
      }
      function getSearchEAPredPrefix() 
      {
          return searchEAPredPrefix;
      }
      function setupsearchbyopen()
      {
          var Y = YInstance;
          /* Y.one("#submenu-1").hide(); */
          /* Y.one("#submenu-1").show(); */
          /* Y.log("showing",'info'); */
      }



      ///////////////////////////////////////
      ///////////////////////////////////////
      ///////////////////////////////////////

      YUI().use('datatable', 'datasource-get', 'datasource-jsonschema', 
                'datasource-io', 'datasource-xmlschema', 
                'datatable-sort',
                'node-menunav',
                'panel', 'dd-plugin',
                'console', 'console-filters',
                'tabview',
                'editor',
                'yui2-editor',
                'autocomplete', 'autocomplete-highlighters',
                'datatable-datasource','node', 'event', function (Y) {
      YInstance = Y;
                    
      /* function selectFormat( o ) { */
      /*     f = select.bind(Y,Y,o.data.url); */
      /*     o.value = '<a href="javascript:f()">' + 'yyy' + '</a>'; */
      /* } */
                    
      var log = new Y.Console({
            style: 'block'
      }).render( "#log" );

      tabview = new Y.TabView({srcNode:'#tabs'});
      tabview.render();
      
      var earl = restBase + "?foo=bar";
      var eanames = "name,url,annotation,is-dir,is-dir-try-automounting,size,size-human-readable,mtime,mtime-display,user-owner-name,group-owner-name,message,protection-ls";
      if( performingQuery > 0 ) 
      {
          eanames = "name,url,size,size-human-readable,mtime,mtime-display,user-owner-name,group-owner-name,protection-ls";
      }
      var query = "&method=ls&path=" + dirname + "&format=xml&eanames=" + eanames + "&zz=top";

      
      function test1( o ) 
      {
          alert("response");
      }

      var dataSource = new Y.DataSource.IO({
            source: earl,
      });

      dataSource.plug(Y.Plugin.DataSourceXMLSchema, {
         schema: {
            resultListLocator: "context/context",
            resultFields: [
                {key:"id",                  locator:"keyval[@key ='name']"},
                {key:"name",                locator:"keyval[@key ='name']"},
                {key:"url",                 locator:"keyval[@key ='url']"},
                {key:"is-dir",              locator:"keyval[@key ='is-dir']"},
                {key:"is-dir-try-automounting",              locator:"keyval[@key ='is-dir-try-automounting']"},
                {key:"size",                locator:"keyval[@key ='size']"},
                {key:"size-human-readable", locator:"keyval[@key ='size-human-readable']"},
                {key:"mtime",               locator:"keyval[@key ='mtime']"},
                {key:"mtime-display",       locator:"keyval[@key ='mtime-display']"},
                {key:"group-owner-name",    locator:"keyval[@key ='group-owner-name']"},
                {key:"user-owner-name",     locator:"keyval[@key ='user-owner-name']"},
                {key:"protection-ls",       locator:"keyval[@key ='protection-ls']"},
                {key:"annotation",          locator:"keyval[@key ='annotation']"},
                {key:"message",             locator:"keyval[@key ='message']"},
                {key:"select",              },
//                {key:"Rating",  locator:"*[local-name()='Rating']/*[local-name()='AverageRating']"},
            ]
         }
      });
      var table = new Y.DataTable({
         columns: [
            {key:"name",                label:"Name", formatter: namelink, allowHTML: true }, 
            {key:"select",              label:"select", formatter: selectFormat, allowHTML: true,},
//            {key:"is-dir",              label:"is-dir", },
//            {key:"is-dir-try-automounting",              label:"is-dir-try-automounting", },
            {key:"size-human-readable", label:"Size", sortable:true, sortFn: sortNumeric.bind(undefined,'size') }, 
		    {key:"mtime-display",       label:"Modified",
                  className: "mtime",
                  sortFn: sortNumeric.bind(undefined,'mtime'),
                  formatter: valueFormat, allowHTML: true,
            },
		    {key:"user-owner-name",     label:"Owner"},
		    {key:"group-owner-name",    label:"Group"},
            {key:"protection-ls",       label:"Protection", className: "protectionls"},
            {key:"annotation",          label:"Annotation", sortFn:   sortStringOrEmpty.bind(undefined,'annotation') },
            {key:"message",             label:"message", formatter: editableEAFormatter.bind(undefined,'message'), allowHTML: true },
//		    {key:"mtime",               label:"mtime", sortable:true },
//		    {key:"size",                label:"size",  sortable:true, sortFn: sortNumeric.bind(undefined,'size') },
                  ],
         summary: "Viewing directory: " + dirname,
         sortable: true,
         caption: "Viewing directory: " + dirname,
      });
      table.plug(Y.Plugin.DataTableDataSource, {
                  datasource:     dataSource,
                  initialRequest: query,
      });
      dataSource.after("response", function( e ) {
              table.render("#tree");
              tree = table;
              
               var xmldoc = e.data.responseXML;

               var path = xmldoc.getElementsByTagName("path")[0].childNodes[0].nodeValue;
               
               Y.log("read dir:" + path, 'info');
//               Y.log( e.data.responseXML, 'warn');
//               Y.log( xmldoc.getElementsByTagName("desc")[0].childNodes[0].nodeValue, 'warn' );

      });


      // annotations
      var editor = new Y.EditorBase({
            content: '<p><b>This is <i class="foo">a test</i></b></p><p><b style="color: red; font-family: Comic Sans MS">This is <span class="foo">a test</span></b></p>',
                  extracss: '.body {background-color:#f00 } .foo { font-weight: normal; color: black; background-color: yellow; }'
                  });

      //      var editor = Y.one('#annotationeditor');
      Y.log("html1:" + editor.getContent(),'info');
      Y.log("html2:" + editor.get('content'),'info');
          
      ///////////////////////////////////////
      //Rendering the Editor
      //
      editor.render('#annotationeditor');

      editor.set("content", "");
      annotationeditor = editor;
      
      
      
// later
//      table.datasource.load({ request: query });

      Y.on("contentready", function () {
              this.plug(Y.Plugin.NodeMenuNav);
          }, "#menubar" );

      var gotoBtn   = Y.one('#goto');
      var earlField = Y.one('#dirname');
      var gotopanel = new Y.Panel({
                  srcNode      : '#gotopanel',
                  headerContent: 'Goto the given URL...',
                  width        : 250,
                  zIndex       : 5,
                  centered     : true,
                  modal        : true,
                  visible      : false,
                  render       : true,
                  plugins      : [Y.Plugin.Drag]
                  });
      gotopanel.addButton({
                  value  : 'open URL',
                  section: Y.WidgetStdMod.FOOTER,
                  action : function (e) {
                    e.preventDefault();
                    earl = earlField.get('value');
                    window.location = "index.php?one=two&dirname=" + earl;
              }
          });
      gotoBtn.on('click', function (e) {
          gotopanel.show();
      });


      ///////////////////////////////////////
      // write annotations away if the user stops typing.
      //
      setTimeout("writeAnnotationTimer()", 2000);

      ///////////////////////////////////////
      // search ea panel
      //
      var searcheapage = Y.one("#searcheatype");
      searcheapage.plug(Y.Plugin.NodeMenuNav);

      ///////////////////////////////////////
      // tag panel
      //
      var tagds = new Y.DataSource.IO({
            source: restBase + "?method=list-all-emblems&format=xml&zz=top"
      });
      tagds.plug(Y.Plugin.DataSourceXMLSchema, {
         schema: {
            resultListLocator: "result/emblem[contains(@name,'')]",
            resultFields: [
                {key:"id",                  locator:"@id"},
                {key:"name",                locator:"@name"},
            ]
         }
      });
      Y.one('#tagac').plug(Y.Plugin.AutoComplete, {
            maxResults: 15,
            resultHighlighter: 'phraseMatch',  
            resultTextLocator: 'name',
            resultFilters    : function( q, results )
              {
                  tagds.resultListLocator = "result/emblem";

                  var ret = new Array();
                  var retidx = 0;
                  Y.log( q, 'info' );
                  Y.log( results.length, 'info' );
                  for( var i = 0; i < results.length; ++i )
                  {
                      Y.log( results[i].text, 'info' );
                      if( results[i].text.indexOf(q) < 0 ) 
                      {
                          continue;
                      }
                      ret[ retidx++ ] = results[i];
                  }
                  return ret;
              },
            source: tagds,
      });

      

      var filetagsds = new Y.DataSource.IO({
            source: restBase + "?method=file-emblems-list&format=xml&zz=top"
      });
      filetagsds.plug(Y.Plugin.DataSourceXMLSchema, {
         schema: {
            resultListLocator: "result/emblem[contains(@name,'')]",
            resultFields: [
                {key:"id",                  locator:"@id"},
                {key:"name",                locator:"@name"},
            ]
         }
      });
      filetags = new Y.DataTable({
         columns: [
            {key:"name",                label:"name", formatter: tagdellink, allowHTML: true }, 
             ],
                  summary: "xxx",
                  sortable: true,
                  caption: "xxx",
      });
      filetags.plug(Y.Plugin.DataTableDataSource, {
                  datasource:     filetagsds,
                  initialRequest: "&path=/",
      });
      filetagsds.after("response", function( e ) {
              filetags.render("#filetags");
      });
      
      
      ///////////////////////////////////////
      // monitor which tab is active
      //
//      var tabs = Y.one("#tabs");

      tabview.after( "selectionChange", function(e) {
//             Y.log("xxxxx",'info');
             Y.log(e.newVal.getClassName(),'info');
             Y.log(e.newVal.get("id"),'info');
             Y.log(e.newVal.get("label"),'info');
              
             var sel = e.newVal.get("label");
             setCookie( "activetab", sel, 365 );
             Y.log("setCookie:" + sel,'info');
          });
      
      tabview.after( "activeDescendantChange", function(e) {

             /* Y.log(e.newVal.getClassName(),'info'); */
             /* Y.log(e.newVal.get("id"),'info'); */
             /* Y.log(e.newVal.get("label"),'info'); */
              
             /*  var sel = e.newVal.get("label"); */
             /* setCookie( "activetab", sel, 365 ); */
             /* Y.log("setCookie:" + sel,'info'); */
          });
      
      var atab = getCookie( "activetab" );
      Y.log('activate tab:' + atab,'info');
      Y.log('len:' + tabview.length,'info');
//      tabview.selectChild(1);
      tabview.each( function( item ) {
              var t = item.get("label");
              if( t == atab ) 
              {
                  item.set('selected', 1);
              }
              Y.log( t,'info');
          });


      // Text editor component.
      texteditor = new Y.EditorBase({});
      texteditor.render('#textedit');
      
      /* var YAHOO  = Y.YUI2; */
      /* texteditor = new YAHOO.widget.Editor('textedit', { */
      /*             height:  '300px', */
      /*             width:   '600px', */
      /*             dompath: true, */
      /*             animate: true, */
      /* }); */
      /* texteditor.render(); */

      Y.log("performingQuery:" + performingQuery, 'info');
      console.log("end...");
      });
    </script>

</html>

