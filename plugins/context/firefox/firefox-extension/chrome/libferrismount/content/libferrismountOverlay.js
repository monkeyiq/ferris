// /******************************************************************************
// *******************************************************************************
// *******************************************************************************

//     libferris
//     Copyright (C) 2005 Ben Martin

//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.

//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with this program; if not, write to the Free Software
//     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//     For more details see the COPYING file in the root directory of this
//     distribution.

//     $Id: libferrismountOverlay.js,v 1.8 2007/01/26 14:04:53 ben Exp $

// *******************************************************************************
// *******************************************************************************
// ******************************************************************************/

function LOG(msg) {
  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                 .getService(Components.interfaces.nsIConsoleService);
  consoleService.logStringMessage(msg);
}

//
// Inspiration for this file came from reading through many Firefox extensions.
//

function starts_with( n, be ) {
    var bel = be.length;
    var l = bel;
    if( n.length < l )
        l = n.length;
    return n.substr( 0, l ) == be.substr( 0, l );
}

function getNodePath( n ) {
    if( !n || n.nodeType == n.DOCUMENT_NODE )
        return "";
    var ret = getNodePath( n.parentNode ) + "/" + n.nodeName;
    return ret;
}

function getChildText( n )
{
    var ret = "";
    var nl = n.childNodes;
    for (var i = 0; i < nl.length; i++)
    {
        var child = nl[i];
        if( child.nodeType == child.TEXT_NODE )
        {
            ret += child.nodeValue;
        }
    }
    return ret;
}

function trimTrailingGarbo( s ) 
{
    while( s.length )
    {
        if( s[ s.length-1 ] == '\n' || s[ s.length-1 ] == '\r' || s[ s.length-1 ] == '\0' )
            s = s.substring( 0, s.length - 1 );
        else
            return s;
    }
    return s;
}



function replace(string,text,by) {
// Replaces text with by in string
    var strLength = string.length, txtLength = text.length;
    if ((strLength == 0) || (txtLength == 0)) return string;

    var i = string.indexOf(text);
    if ((!i) && (text != string.substring(0,txtLength))) return string;
    if (i == -1) return string;

    var newstr = string.substring(0,i) + by;

    if (i+txtLength < strLength)
        newstr += replace(string.substring(i+txtLength,strLength),text,by);

    return newstr;
}

// function myGetNode( document, tagname, eaname, eavalue ) {
    
//     var nl = document.getElementsByTagName ( tagname );
//     for( var nliter = 0; nliter != nl.length; nliter++ ) {
//         var n = nl.item( nliter );
//         var attrs = n.attributes;
//         var srcnode = attrs.getNamedItem( eaname );
//         if( srcnode && srcNode.nodeValue==eavalue ) {
//             return n;
//         }
//     }
//     return 0;
// }

include (jslib_file);
include (jslib_fileutils);
include (jslib_dirutils);
include (jslib_dir);

function cleanupURL( fn ) 
{
    if( starts_with( fn, "~/" ) )
    {
        var du = new DirUtils;
        var homedir = du.getHomeDir();
        fn = homedir + "/" + fn.substring(2);
    }
    return fn;
}

function ensureDotFilesSetup()
{
    var d=new Dir( cleanupURL("~/.ferris/tmp/mount-firefox/" ) );
    if( !d.exists() )
    {
        d.create();
    }
}

function writeDom( fn, dom ) 
{
    var serializer = new XMLSerializer();
    var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"]
        .createInstance(Components.interfaces.nsIFileOutputStream);
    var f = new File( cleanupURL( fn ) );
    var nsifile = f.nsIFile;
    foStream.init(nsifile, 0x02 | 0x08 | 0x20, 0664, 0);   // write, create, truncate
    serializer.serializeToStream(dom, foStream, "UTF-8");
    foStream.close();
}


function wf( fn, c ) {

    fn = cleanupURL( fn );
    
    var f = new File( fn );
    f.create();
    f.open('w');
    c = replace(c,"&","&amp;");
    f.write(c);
    f.close();
}

function findDocument( title ) 
{
    var DOMWindow = Components.interfaces.nsIDOMWindow;
    var WindowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService(Components.interfaces.nsIWindowMediator);
    var windows = WindowManager.getEnumerator('navigator:browser'), currentWindow;

    while (windows.hasMoreElements())
    {
        var currentWindow = windows.getNext().QueryInterface(DOMWindow);
        var Tabbrowser = currentWindow.getBrowser();
        var Tabs = Tabbrowser.mTabContainer.childNodes;

        for (var i = 0; i < Tabs.length; i++) {
            var curTab = Tabs[i];
            var curBrowser = Tabbrowser.getBrowserForTab(curTab);
            var curTitle = curBrowser.contentTitle;

//            if( starts_with( title, curTitle ) )
            if( title == curTitle )
            {
                return curBrowser.contentDocument;
            }
        }
    }

    return null;
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

	// create the prototype for download

function PersistProgressListener() {}
PersistProgressListener.prototype = {
    QueryInterface : function(aIID) {
		if(aIID.equals(Components.interfaces.nsIWebProgressListener))
            return this;
        throw Components.results.NS_OK;
    },
	
    init : function() {var flag=0; var identity=0;},
    destroy : function() {},
    
	onStateChange:(function (aWebProgress, aRequest, aStateFlags, aStatus)
        {}),
    
    onProgressChange:(function (aWebProgress, aRequest, aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress)
        {
        }),

	onLocationChange: (function (aWebProgress, aRequest, aLocation) {}), 
	onStatusChange:(function (aWebProgress, aRequest, aStatus, aMessage) {}), 
	onSecurityChange:(function (aWebProgress, aRequest, aState) {}),
    
}
    

    function mountWithFerrisWriteImages( basePath ) {

    var DOMWindow = Components.interfaces.nsIDOMWindow;
    var WindowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService(Components.interfaces.nsIWindowMediator);
    var windows = WindowManager.getEnumerator('navigator:browser'), currentWindow;
    while (windows.hasMoreElements()) {

        var currentWindow = windows.getNext().QueryInterface(DOMWindow);
        var Tabbrowser = currentWindow.getBrowser();
        var Tabs = Tabbrowser.mTabContainer.childNodes;

        for (var i = 0; i < Tabs.length; i++) {
            var curTab = Tabs[i];
            var curBrowser = Tabbrowser.getBrowserForTab(curTab);
            var curTitle = curBrowser.contentTitle;
            var curURL = "" + curBrowser.currentURI.prePath + curBrowser.currentURI.path;
            var doc = curBrowser.contentDocument;

            var nl = doc.getElementsByTagName("img");
            for( var nliter = 0; nliter != nl.length; nliter++ ) {
                var n = nl.item( nliter );
                var attrs = n.attributes;
                var srcnode = attrs.getNamedItem( "src" );
                if( srcnode )
                {
                    var srcEarl = srcnode.nodeValue;
                    
                    const FileFactory = new Components.Constructor("@mozilla.org/file/local;1", "nsILocalFile", "initWithPath");
                    const nsIWBP = Components.interfaces.nsIWebBrowserPersist;
                    
                    var filename = basePath + "/" + srcnode.nodeValue;

                    if ((filename.indexOf("..") > 0))
                    {
                        wf("~/.ferris/tmp/mount-firefox/bad-filename",filename);
                        continue;
                    }
                    filename = replace( filename, "//" ,"/" );
                    filename = replace( filename, "//" ,"/" );
                    
                    {
                        var f = new File( cleanupURL( filename ) );
                        f.create();
                    }
                            
                    var dest = new FileFactory( filename );
                    wf("~/.ferris/tmp/mount-firefox/progress6.a","1");
                    var refPage = Components.classes['@mozilla.org/network/standard-url;1'].createInstance(Components.interfaces.nsIURI);
                    wf("~/.ferris/tmp/mount-firefox/progress6.b","1");

                    var uri = Components.classes['@mozilla.org/network/standard-url;1'].createInstance(Components.interfaces.nsIURI);
                    uri.spec = srcnode.nodeValue;
                    if( !starts_with( uri.spec, "http" ) )
                    {
                        uri.spec = curBrowser.currentURI.prePath + "/" + srcnode.nodeValue;
                    }
                    
                    wf("~/.ferris/tmp/mount-firefox/data-uri.spec", uri.spec );

                    wf("~/.ferris/tmp/mount-firefox/progress7","1");
                    var persist = Components.classes['@mozilla.org/embedding/browser/nsWebBrowserPersist;1'].createInstance(Components.interfaces.nsIWebBrowserPersist);
                    wf("~/.ferris/tmp/mount-firefox/progress8","1");
                    var flags = nsIWBP.PERSIST_FLAGS_NO_CONVERSION | nsIWBP.PERSIST_FLAGS_REPLACE_EXISTING_FILES | nsIWBP.PERSIST_FLAGS_BYPASS_CACHE;
                    persist.persistFlags = flags;
                    wf("~/.ferris/tmp/mount-firefox/progress9",filename);
                    persist.progressListener = new PersistProgressListener();
                    persist.saveURI(uri, null, refPage, null, null, dest);
                    wf("~/.ferris/tmp/mount-firefox/progress10","1");

                    




//                 wf("~/.ferris/tmp/mount-firefox/starting","1");
//                 dump("Starting...");
//                 var ImageLoadingContent = Components.interfaces.nsIImageLoadingContent;
//                 var loader = n.QueryInterface(ImageLoadingContent);
//                 if( srcnode && loader )
//                 {
//                     var request = loader.getRequest(
//                         Components.interfaces.nsIImageLoadingContent.CURRENT_REQUEST );
//                     wf("~/.ferris/tmp/mount-firefox/progress1","1");
//                     dump("progress...1");
//                     if (request && (request.imageStatus & request.STATUS_SIZE_AVAILABLE))
//                     {
//                         wf("~/.ferris/tmp/mount-firefox/progress2","1");
//                         dump("progress...2");
//                         var image = request.image;

//                         wf("~/.ferris/tmp/mount-firefox/progress3","1");
//                         var currentFrame = image.currentFrame;
//                         wf("~/.ferris/tmp/mount-firefox/progress4","1");
//                         if( currentFrame )
//                         {
//                             wf("~/.ferris/tmp/mount-firefox/progress5","1");


//                             var uri = Components.classes['@mozilla.org/network/standard-url;1'].createInstance(Components.interfaces.nsIURI);
//                             wf("~/.ferris/tmp/mount-firefox/progress6","1");

//                             const FileFactory = new Components.Constructor("@mozilla.org/file/local;1", "nsILocalFile", "initWithPath");
//                             const nsIWBP = Components.interfaces.nsIWebBrowserPersist;
                            
// //                            var dest = new FileFactory("/tmp/firefox-dump/");
//                             var filename = "/tmp/firefox-dump/" + srcnode.nodeValue;

//                             if ((filename.indexOf("..") > 0))
//                             {
//                                 wf("~/.ferris/tmp/mount-firefox/bad-filename",filename);
//                                 continue;
//                             }
//                             {
//                                 var f = new File( cleanupURL( filename ) );
//                                 f.create();
//                             }
                            
//                             var dest = new FileFactory( filename );
// //                            dest.initWithPath("/tmp/firefox-dump/" + "/" + "ggg/hh");
                            
// //                            dest.append("ggg");
//                             wf("~/.ferris/tmp/mount-firefox/progress6.a","1");
// 	var refPage = Components.classes['@mozilla.org/network/standard-url;1'].createInstance(Components.interfaces.nsIURI);
//                             wf("~/.ferris/tmp/mount-firefox/progress6.b","1");

//                             uri.spec = "http://localhost/";
//                             if( srcnode ) {
//                                 uri.spec = srcnode.nodeValue;
//                             }
                            
//                             wf("~/.ferris/tmp/mount-firefox/progress7","1");
//                             var persist = Components.classes['@mozilla.org/embedding/browser/nsWebBrowserPersist;1'].createInstance(Components.interfaces.nsIWebBrowserPersist);
//                             wf("~/.ferris/tmp/mount-firefox/progress8","1");
//                             var flags = nsIWBP.PERSIST_FLAGS_NO_CONVERSION | nsIWBP.PERSIST_FLAGS_REPLACE_EXISTING_FILES | nsIWBP.PERSIST_FLAGS_BYPASS_CACHE;
//                             persist.persistFlags = flags;
//                             wf("~/.ferris/tmp/mount-firefox/progress9","1");
//                             persist.progressListener = new PersistProgressListener();
//                             persist.saveURI(uri, null, refPage, null, null, dest);
//                             wf("~/.ferris/tmp/mount-firefox/progress10","1");

                            
//                         }
//                     }
                    
                }
            }
        }
    }
    LOG("Harvest complete!");
    }

    


function mountWithFerrisWriteData() {

    var dom = document.implementation.createDocument("", "", null);
    var roote = dom.createElement( "firefox" );
    dom.appendChild(roote);
    
    
    var DOMWindow = Components.interfaces.nsIDOMWindow;
    var WindowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService(Components.interfaces.nsIWindowMediator);
    var windows = WindowManager.getEnumerator('navigator:browser'), currentWindow;
    while (windows.hasMoreElements()) {

        var browserWindowNode = dom.createElement( "browserwindow" );
        roote.appendChild( browserWindowNode );
        
        var currentWindow = windows.getNext().QueryInterface(DOMWindow);
        var Tabbrowser = currentWindow.getBrowser();
        var Tabs = Tabbrowser.mTabContainer.childNodes;

        for (var i = 0; i < Tabs.length; i++) {
            var curTab = Tabs[i];
            var curBrowser = Tabbrowser.getBrowserForTab(curTab);
            var curTitle = curBrowser.contentTitle;
            var curURL = "" + curBrowser.currentURI.prePath + curBrowser.currentURI.path;
            var doc = curBrowser.contentDocument;

            var tabNode = dom.createElement( "tab" );
            tabNode.setAttribute("href",curURL);
            browserWindowNode.appendChild( tabNode );
            var titleNode = dom.createElement( "title" );
            tabNode.appendChild( titleNode );
            titleNode.appendChild( dom.createTextNode( curTitle ) );
            

            var imagesNode = dom.createElement( "images" );
            tabNode.appendChild( imagesNode );

            var nl = doc.getElementsByTagName("img");
            for( var nliter = 0; nliter != nl.length; nliter++ ) {
                var n = nl.item( nliter );
                var attrs = n.attributes;
                var srcnode = attrs.getNamedItem( "src" );
                if( srcnode ) {
                    var nn = dom.createElement( "image" );
                    nn.setAttribute("src",srcnode.nodeValue);
                    imagesNode.appendChild( nn );
                }
            }

            var linksNode = dom.createElement( "links" );
            tabNode.appendChild( linksNode );
            
            var nl = doc.getElementsByTagName("a");
            for( var nliter = 0; nliter != nl.length; nliter++ ) {
                var n = nl.item( nliter );
                var attrs = n.attributes;
                var href = attrs.getNamedItem( "href" );
                if( href )
                {
                    var title = getChildText( n );
                    var linktext = href.nodeValue;
                
                    var nn = dom.createElement( "a" );
                    nn.appendChild( dom.createTextNode( title ) );
                    nn.setAttribute("href", linktext );
                    linksNode.appendChild( nn );
                }
            }
        }
    }

    ensureDotFilesSetup();
    writeDom( "~/.ferris/tmp/mount-firefox/libferris-firefox-metadata", dom );
}


function mountWithFerrisPageLoad(event)
{
  var page = event.originalTarget;
  mountWithFerrisWriteData();
}



function mountWithFerrisInit()
{
    // Add listener for page loads
    if (document.getElementById("appcontent"))
        document.getElementById("appcontent").addEventListener("load", 
                                                               mountWithFerrisPageLoad, 
                                                               true);
}

// Create event listener.
window.addEventListener('load', mountWithFerrisInit, false);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

var handleIncomingRequestsStream_setup = 0;
function handleIncomingRequestsStream() {

    if( handleIncomingRequestsStream_setup ) {
        return;
    }
    handleIncomingRequestsStream_setup = 1;
    
    var listener =
        {
            onSocketAccepted : function(serverSocket, transport)
            {
                var oss = transport.openOutputStream(0,0,0);

                var stream = transport.openInputStream(0,0,0);
                var instream =    Components.classes["@mozilla.org/scriptableinputstream;1"]
                .createInstance(Components.interfaces.nsIScriptableInputStream);
                instream.init(stream);
                // pump takes in data in chunks asynchronously
                var pump = Components.classes["@mozilla.org/network/input-stream-pump;1"].
                createInstance(Components.interfaces.nsIInputStreamPump);
                pump.init(stream, -1, -1, 0, 0, false);

                var dataListener  = {

                    onStartRequest: function(request, context) {
                        ensureDotFilesSetup();
//                        wf("~/.ferris/tmp/mount-firefox/start-request","7");
                    },
                    onStopRequest: function(request, context, status) {
//                        wf("~/.ferris/tmp/mount-firefox/stop-request","1");
                    },
                    onDataAvailable: function(request, context, inputStream, offset, count) {
                        var cmd = instream.read(count);
                        
                        if( starts_with( cmd, "write-dom" ) )
                        {
                            var title = trimTrailingGarbo(cmd.substr( "write-dom".length + 1 ));
//                            wf("~/.ferris/tmp/mount-firefox/libferris-moz-reply-export-dom-debug", title);

                            var outputPath = "~/.ferris/tmp/mount-firefox/libferris-moz-reply-export-dom";
                            outputPath = cleanupURL( outputPath );

                            var fu = new FileUtils;
                            fu.remove(outputPath);

                            var theDoc = findDocument( title );
                            if( theDoc )
                            {
                                writeDom( outputPath, theDoc );
                            }

                            oss.write("OK",2);
                        }
                        else if( starts_with( cmd, "update-metadata" ) )
                        {
                            mountWithFerrisWriteData();

                            oss.write("OK",2);
                        }
                        else if( starts_with( cmd, "load-virtual-page" ) )
                        {
                            var p = cmd.substr( "load-virtual-page".length + 1 );
                            p = trimTrailingGarbo(p);

                            var newtab = gBrowser.addTab( p );
//                            var newbr  = gBrowser.getBrowserForTab( newtab );
                            gBrowser.selectedTab = newtab;
//                            gURLBar.value = "fooey!" + p;

                            oss.write("OK",2);
                        }
                        else if( starts_with( cmd, "quit" ) )
                        {
                            instream.close();
                            stream.close();
                            oss.close();
                        }
                        else
                        {
                            var s = "offset:";
                            s += offset + " count:" + count;
                            s += " cmd:" + cmd + ":\n";
                            wf("~/.ferris/tmp/mount-firefox/libferris-moz-reply-unmatched-command-given",s);
                        }
                    },
                };
                
                pump.asyncRead(dataListener,null);
            },

            onStopListening : function(serverSocket, status){}
        };
    
    var serverSocket = Components.classes["@mozilla.org/network/server-socket;1"]
        .createInstance(Components.interfaces.nsIServerSocket);
    serverSocket.init(7111,true,-1);
    serverSocket.asyncListen(listener);
}

handleIncomingRequestsStream();


