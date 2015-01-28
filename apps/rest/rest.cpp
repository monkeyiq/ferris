/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2012 Ben Martin

    This file is part of libferris.

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: rest.cpp,v 1.26 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <cgicc/Cgicc.h>

#include <FerrisCopy.hh>
#include <FerrisCopy_private.hh>
#include <Ferris.hh>
#include <FerrisLoki/loki/Factory.h>
#include <Ferris/EAIndexerMetaInterface.hh>
#include <Ferris/EAQuery.hh>
#include <FerrisQt_private.hh>
#include <FerrisDOM.hh>
#include <FerrisFileActions.hh>
#include <Medallion.hh>
#include <Personalities.hh>

#include <iostream>

using namespace std;
using namespace Ferris;

#define DEBUG cerr

cgicc::Cgicc cgi;

#define X(str) XStr(str).unicodeForm()

struct krustyGatewayMethod;
typedef Loki::SingletonHolder<
    Loki::Factory< krustyGatewayMethod, std::string >,
    Loki::CreateUsingNew, Loki::NoDestroy >
krustyGatewayFactory;

struct krustyGatewayMethod
{
    krustyGatewayMethod( const char* name )
    {
    }
    virtual void operator()()
    {
    }
    void displayContentType()
    {
        string format = cgi("format");
        if( format == "xml" )
        {
            cout << "Content-Type: text/xml\n" << endl;
            return;
        }
        cout << "Content-Type: text/json\n" << endl;
    }
    
    void display( fh_context c,
                  stringlist_t eanames,
                  int recurse = 0,
                  bool includeContent = true ) const
    {
        string format = cgi("format");
        if( format == "xml" )
        {
//            cerr << "display in XML!" << endl;
            std::string ret = XML::contextToXML( c, eanames, recurse );
            cout << ret << endl;
            return;
        }
        std::string ret = contextToJSON( c, eanames, recurse );
        cout << ret << endl;
    }

    void prepOutputStringMap( fh_context c, stringmap_t& sm,
                              const std::string& attr, const std::string& defval = "" ) const
    {
        sm[ attr ] = getStrAttr( c, attr, defval );
    }
    void display( stringmap_t& sm ) const
    {
        string format = cgi("format");
        if( format == "xml" )
        {
            std::string ret = XML::stringmapToXML( sm );
            cout << ret << endl;
            return;
        }

        std::string ret = stringmapToJSON( sm );
        cout << ret << endl;
    }
    void display( fh_domdoc doc )
    {
        string format = cgi("format");
        if( format == "xml" )
        {
            fh_stringstream retss = tostream( doc );
            cout << tostr( retss ) << endl;
            return;
        }
        std::string ret = XMLToJSON( doc );
        cout << ret << endl;
    }
    
    void showEmblemList( emblems_t& el )
    {
        DOMImplementation *impl = Factory::getDefaultDOMImpl();
        fh_domdoc    doc = impl->createDocument( 0, X("result"), 0 );
        DOMElement* root = doc->getDocumentElement();
    
        for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
        {
            fh_emblem em = *ei;
            DOMElement* e = doc->createElement( X("emblem") );
            root->appendChild( e );
            setAttribute( e, "id",   tostr(em->getID()) );
            setAttribute( e, "name", em->getName() );
        }
        display( doc );
    }
    
    
};
template < class ChildClass >
struct krustyGatewayMethodBase : public krustyGatewayMethod
{
    static krustyGatewayMethod* create()
    {
        return new ChildClass();
    }
    krustyGatewayMethodBase( const char* name )
        : krustyGatewayMethod( name )
    {
        krustyGatewayFactory::Instance().Register( name, &create );
    }
};

#define METHODDECL( cppMethodName, methodName )                              \
    struct cppMethodName : krustyGatewayMethodBase< cppMethodName >          \
    {                                                                        \
    cppMethodName() : krustyGatewayMethodBase< cppMethodName >( methodName ) \
    {}                                                                       \
    void operator()();                                                       \
    };                                                                       \
cppMethodName cppMethodName ## obj;

#define METHODIMPL( cppMethodName, methodName )                         \
    void cppMethodName::operator()() 

#define METHODDEF( cppMethodName, methodName )                         \
    METHODDECL( cppMethodName, methodName );                           \
    METHODIMPL( cppMethodName, methodName )

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static bool isTrue( const char* cgivar, std::string defval )
{
    string s = cgi( cgivar );
    if( !s.empty() )
        return isTrue( s );
    return isTrue( defval );
}

METHODDEF( eaqueryMethod, "eaquery" )
{
    displayContentType();
    
    DEBUG << "eaquery!" << endl;
    string eaindexpath = cgi("eaindexpath");
    string q = cgi("q");
    stringlist_t eanames = Util::parseCommaSeperatedList( cgi("eanames"));
    long limit = toint(cgi("limit"));
        
    if( !eaindexpath.empty() )
    {
        ::Ferris::EAIndex::Factory::setDefaultEAIndexPath( eaindexpath.c_str() );
    }

    DEBUG << "eaindexpath:" << eaindexpath << endl;
    DEBUG << "q:" << q << endl;
    DEBUG << "limit:" << limit << endl;
    DEBUG << "eanames.sz:" << eanames.size() << endl;
        
    fh_context c = EAIndex::ExecuteQuery( q, limit );
    DEBUG << "result count:" << c->getSubContextCount() << endl;
    display( c, eanames, 1 );
}


METHODDEF( getDocByEAIndexDocIDMethod, "get-doc-by-ea-index-docid" )
{
    DEBUG << "get by index docid..." << endl;
    string eaindexpath = cgi("eaindexpath");
    string docid       = cgi("docid");

    if( !eaindexpath.empty() )
    {
        ::Ferris::EAIndex::Factory::setDefaultEAIndexPath( eaindexpath.c_str() );
    }
        
    fh_context c = Resolve((string)"docidea:" + docid);
    DEBUG << "c.url:" << c->getURL() << endl;
    std::string mimetype = "text/plain";
    mimetype = getStrAttr( c, "mimetype-from-file-command", mimetype );
    std::string rdn = c->getDirName();
    std::string nameonly = getStrAttr( c, "name-only", "" );
    if( !nameonly.empty() )
    {
        rdn = nameonly;
        int pos = rdn.rfind('/');
        if( pos != string::npos )
            rdn = rdn.substr( pos + 1 );
    }
    cout << "Content-Type: " << mimetype << endl;
    cout << "Content-Disposition: inline; filename=" << rdn << endl;
    cout << endl;
    cout << getStrAttr( c, "content", "", true, true );
}

METHODDEF( ls, "ls" )
{
    displayContentType();
//    DEBUG << "ls..." << endl;

    string          path = cgi("path");
    stringlist_t eanames = Util::parseCommaSeperatedList( cgi("eanames"));

    try
    {
        fh_context c = Resolve( path );
        display( c, eanames, 1 );
    }
    catch( FerrisNotReadableAsContext& e )
    {
        cout << "<error type=\"call\" exception=\"NotReadableAsContext\" >" << endl
             << "   <path>" << path << "</path>" << endl
             << "   <desc id=\"emsg\">Can no read because:" << e.what() << "</desc>" << endl
             << "</error>" << endl;
    }
}


METHODDEF( readmethod, "read" )
{
//    DEBUG << "read..." << endl;
    string          path   = cgi("path");
    string          eaname = cgi("ea");
    if( eaname.empty() )
        eaname = "content";

    fh_context c = Resolve( path );
    string mt = getStrAttr( c, "mimetype-from-file-command", "text/plain" );
    if( ends_with( path, "txt" ) && mt == "application/octet-stream" )
        mt = "text/plain";
    
    cout << "Content-Type: " << mt << "\n" << endl;
    cout << getStrAttr( c, eaname, "", true );
}


METHODDEF( writemethod, "write" )
{
    displayContentType();

    string          path   = cgi("path");
    string          eaname = cgi("ea");
    string          data   = cgi.getEnvironment().getPostData();

    ofstream tss;
    tss.open("/tmp/data");
    tss << data;
    tss.close();
    
    
    try
    {
        if( eaname.empty() )
            eaname = "content";
        if( data.empty() )
            data = cgi("data");
        
        fh_context c = Resolve( path );
        setStrAttr( c, eaname, data, true, true );
        cout << "<result code=\"ok\"/>" << endl;
    }
    catch( CanNotGetStream& e )
    {
        cout << "<error type=\"fail\" exception=\"CanNotGetStream\" >" << endl
             << "   <path>" << path << "</path>" << endl
             << "   <desc>" << e.what() << "</desc>" << endl
             << "</error>" << endl;
    }
}



METHODDEF( statmethod, "stat" )
{
    displayContentType();
    string             path   = cgi("path");
    unsigned long Dereference = isTrue( "derefrence", "0" );
    stringlist_t      eanames = Util::parseCommaSeperatedList( cgi("eanames"));
    string         LinkPrefix = "dontfollow-";
    if( Dereference )
    {
       LinkPrefix = "";
    }

    try
    {
        stringmap_t sm;
        fh_context c = Resolve( path );
        prepOutputStringMap( c, sm, "path" );
        prepOutputStringMap( c, sm, "size" );
        prepOutputStringMap( c, sm, "block-count" );
        prepOutputStringMap( c, sm, "block-size" );
        prepOutputStringMap( c, sm, "mode" );
        prepOutputStringMap( c, sm, "device-major-hex" );
        prepOutputStringMap( c, sm, "device-minor-hex" );
        prepOutputStringMap( c, sm, "device-hex" );
        prepOutputStringMap( c, sm, "inode" );
        prepOutputStringMap( c, sm, "hard-link-count" );
        prepOutputStringMap( c, sm, "protection-raw" );
        prepOutputStringMap( c, sm, "protection-ls" );
        prepOutputStringMap( c, sm, "user-owner-number" );
        prepOutputStringMap( c, sm, "user-owner-name" );
        prepOutputStringMap( c, sm, "filesystem-filetype" );
        prepOutputStringMap( c, sm, "group-owner-number" );
        prepOutputStringMap( c, sm, "group-owner-name" );
        prepOutputStringMap( c, sm, "atime-display" );
        prepOutputStringMap( c, sm, "mtime-display" );
        prepOutputStringMap( c, sm, "ctime-display" );
        prepOutputStringMap( c, sm, "atime" );
        prepOutputStringMap( c, sm, "mtime" );
        prepOutputStringMap( c, sm, "ctime" );
        for( stringlist_t::iterator si = eanames.begin(); si != eanames.end(); ++si )
        {
            prepOutputStringMap( c, sm, *si );
        }
        display( sm );
    }
    catch( NoSuchSubContext& e )
    {
        cout << "<error type=\"call\" exception=\"NoSuchSubContext\" >" << endl
             << "   <path>" << path << "</path>" << endl
             << "   <desc>" << e.what() << "</desc>" << endl
             << "</error>" << endl;
    }
}
METHODDEF( readlinkmethod, "readlink" )
{
    displayContentType();
    DEBUG << "readlink..." << endl;
    string path = cgi("path");

    stringmap_t sm;
    fh_context c = Resolve( path );
    prepOutputStringMap( c, sm, "url" );
    prepOutputStringMap( c, sm, "link-target" );
    prepOutputStringMap( c, sm, "is-link" );
    string s = getStrAttr( c, "link-target", "" );
    if( s.empty() )
        s = getStrAttr( c, "url", "" );
    sm[ "target" ] = s;
    display( sm );
}


METHODDEF( fileactionmethod, "file-action" )
{
    displayContentType();
    DEBUG << "file-action..." << endl;
    string path   = cgi("path");
    string opname = cgi("opname");
    if( opname.empty() )
        opname = "view";
    ctxlist_t cl;
    cl.push_back( Resolve( path ) );
    FileActions::OpenWith( opname, cl );
}

METHODDEF( touchmethod, "touch" )
{
    displayContentType();
    string path     = cgi("path");
    bool isDir      = !cgi("isdir").empty();
    bool create     = true;
    
    fh_context c = Shell::touch( path, create, isDir );
}

METHODDEF( mkdirmethod, "mkdir" )
{
    displayContentType();
    string path     = cgi("path");
    
    fh_context c = Shell::CreateDir( path );
}

METHODDEF( cpmethod, "cp" )
{
    displayContentType();
    string src = cgi("source");
    string dst = cgi("destination");

    fh_cp_tty obj = new FerrisCopy_TTY();
    obj->setShowMeter( true );
    obj->setSrcURL( src );
    obj->setDstURL( dst );
    obj->copy();
}


METHODDEF( listallemblemsmethod, "list-all-emblems" )
{
    displayContentType();

    fh_etagere et = Factory::getEtagere();
    emblems_t el = et->getAllEmblems();
    showEmblemList( el );
}

METHODDEF( fileemblemslistmethod, "file-emblems-list" )
{
    displayContentType();
    string path     = cgi("path");

    fh_context   c = Resolve( path );
    fh_medallion m = c->getMedallion();
    emblems_t   el = m->getMostSpecificEmblems();
    cerr << "ListEmblem count:" << el.size() << endl;
    showEmblemList( el );
}

METHODDEF( fileemblemsaddmethod, "file-emblems-add" )
{
    displayContentType();
    string path       = cgi("path");
    string EmblemName = cgi("emblem");
    string EmblemID   = cgi("emblemid");
    bool   CreateEmblem = isTrue(cgi("create"));

    fh_etagere et = Factory::getEtagere();
    fh_emblem em = 0;
    try 
    {
        if( !EmblemName.empty() )
            em = et->getEmblemByName( EmblemName );
        if( !EmblemID.empty() )
            em = et->getEmblemByID( toType<emblemID_t>( EmblemID ));
    }
    catch( EmblemNotFoundException& e )
    {
        if( CreateEmblem )
        {
            fh_emblem root = et->obtainEmblemByName( "libferris-created-emblems" );
            em = root->obtainChild( EmblemName );
            et->sync();
        }
    }
    
    fh_context   c = Resolve( path );
    fh_medallion m = c->getMedallion();
    m->addEmblem( em );
    m->sync();
}

METHODDEF( fileemblemsremovemethod, "file-emblems-remove" )
{
    displayContentType();
    string path       = cgi("path");
    string EmblemName = cgi("emblem");
    string EmblemID   = cgi("emblemid");

    fh_etagere et = Factory::getEtagere();
    fh_emblem em = 0;
    if( !EmblemName.empty() )
        em = et->getEmblemByName( EmblemName );
    if( !EmblemID.empty() )
        em = et->getEmblemByID( toType<emblemID_t>( EmblemID ));

    fh_context   c = Resolve( path );
    fh_medallion m = c->getMedallion();
    m->removeEmblem( em );
    m->sync();
}


METHODDEF( versionmethod, "version" )
{
    displayContentType();
    stringmap_t sm;
    sm["version"] = VERSION;
    display( sm );
}


int
main ()
{
//    cout << "name:" << cgi("name") << " foo:" << cgi("foo") << endl;

    std::string methodname = cgi("method");
    try
    {
        krustyGatewayMethod* m = krustyGatewayFactory::Instance().CreateObject( methodname );
        (*m)();
        return 0;
    }
    catch( Loki::Factory< krustyGatewayMethod, std::string >::Exception& e )
    {
        cerr << "<error type=\"fatal\">" << endl
             << "   <desc>method " << methodname << " does not exist!</desc>" << endl
             << "</error>" << endl;
        return 1;
    }
    catch( exception& e )
    {
        cerr << e.what() << endl;
        return 1;
    }
}

