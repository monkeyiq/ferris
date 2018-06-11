/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisxsltfs.cpp,v 1.8 2010/09/24 21:31:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisXalan_private.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/SyncDelayer.hh>

#include <libferrisxmlshared.hh>

#include <algorithm>
#include <numeric>

#include <xercesc/util/PlatformUtils.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>
#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>

#include <xalanc/XercesParserLiaison/FormatterToXercesDOM.hpp>
#include <xalanc/XercesParserLiaison/XercesDOMFormatterWalker.hpp>

#include <config.h>

using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    bool ShouldPerformUpdates()
    {
        static bool ret = true;
        
        if( const gchar* p = g_getenv ("LIBFERRIS_XSLTFS_DONT_UPDATE") )
        {
            ret = false;
        }
        return ret;
    }
    
    
    static std::string xgetURLScheme( const std::string fsArgs )
    {
        fh_stringstream ss;
        ss << "xsltfs";
        if( !fsArgs.empty() )
            ss << "?" << fsArgs;
                
        return tostr(ss);
    }
    
    string Shared_getFSArgs( Context* ctx );
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class XSLTFS_DOMWrapper;
    class Lazy_XSLTFS_DOMWrapper;
    class xsltfsRootContext;
    FERRIS_SMARTPTR( XSLTFS_DOMWrapper, fh_XSLTFS_DOMWrapper );
    
    class FERRISEXP_DLLLOCAL XSLTFS_DOMWrapper
        :
        public ChainedViewContext
    {
        typedef XSLTFS_DOMWrapper _Self;
        typedef ChainedViewContext _Base;
        typedef fh_XSLTFS_DOMWrapper fh_Self;

        fh_domdoc m_dom;
        fh_context m_inputXMLContext;
        
        fh_domdoc getDOM();

        fh_context getInputXMLContext()
            {
                return getBaseContext<_Self>()->m_inputXMLContext;
            }

        fh_Self child( const std::string& rdn )
            {
                fh_context c = getSubContext( rdn );
                fh_Self ret = dynamic_cast<XSLTFS_DOMWrapper*>( GetImpl(c) );
                return ret;
            }
        
        
        string getReverseStyleSheetURL();

        typedef std::list< DOMElement* > del_t;

        void processReverseXSLTUpdates_Relative( fh_domdoc theDOM,
                                                 DOMNode* dn, fh_context dc,
                                                 bool autoCreate )
            {
                LG_XSLTFS_D << "processReverseXSLTUpdates_Relative() url:" << dc->getURL()
                            << " dn.name:" << tostr(dn->getNodeName())
                            << endl;
                for( Context::iterator ci = dc->begin(); ci != dc->end(); ++ci )
                {
                    LG_XSLTFS_D << " ctx.child:" << (*ci)->getDirName() << endl;
                }

                DOMNodeList* nl = dn->getChildNodes();
                LG_XSLTFS_D << " nl.sz:" << nl->getLength() << endl;
                
                for( int i=0; i < nl->getLength(); ++i )
                {
                    DOMNode*   cn = nl->item( i );
                    LG_XSLTFS_D << " cn.name:" << tostr(cn->getNodeName()) << endl;
                    if( cn->getNodeType() != DOMNode::ELEMENT_NODE )
                        continue;
                    DOMElement* e = (DOMElement*)cn;

                    
                    bool ignoreNameAttribute = false;
                    string    rdn = tostr(cn->getNodeName());
                    {
                        string t = Ferris::getAttribute( e, "name" );
                        if( !t.empty() )
                        {
                            ignoreNameAttribute = true;
                            rdn = t;
                        }
                    }
                    
                    LG_XSLTFS_D << "processReverseXSLTUpdates_Relative() rdn:" << rdn
                                << " dc:" << dc->getURL() << endl << endl;

                    if( rdn == "context" && !dc->isSubContextBound(rdn))
                    {
                        continue;
                    }
                    
                    fh_context cc = 0;
                    try
                    {
                        cc = dc->getSubContext( rdn );
                    }
                    catch( exception& ex )
                    {
                        if( autoCreate )
                        {
                            bool isDir = false;
                            
                            {
                                DOMNodeList* nl = e->getChildNodes();
                                for( int i=0; i < nl->getLength(); ++i )
                                {
                                    DOMNode*   cn = nl->item( i );
                                    LG_XSLTFS_D << " cn.name:" << tostr(cn->getNodeName()) << endl;
                                    if( cn->getNodeType() == DOMNode::ELEMENT_NODE )
                                    {
                                        isDir = true;
                                        break;
                                    }
                                }
                            }
                            
                            LG_XSLTFS_D << "using autoCreate option to make child:" << rdn
                                        << " isDir:" << isDir
                                        << " for url:" << dc->getURL()
                                        << endl;
                            
                            if( isDir )
                                cc = Shell::CreateDir( dc, rdn );
                            else
                                cc = Shell::CreateFile( dc, rdn );
                        }
                        else
                        {
                            throw;
                        }
                    }
                    
                    LG_XSLTFS_D << " cc:" << cc->getURL() << endl;

                    string contents = XML::getChildText( e );
                    if( !contents.empty() )
                    {
                        LG_XSLTFS_D << "UPDATE-CONTENTS c:" << cc->getURL()
                             << " contents:" << contents << endl;

                        bool create = true;
                        bool throw_for_errors = true;
                        bool dontDelegateToOvermountContext = false;
                        if( ShouldPerformUpdates() )
                            setStrAttr( cc, "content", contents, create, throw_for_errors,
                                        dontDelegateToOvermountContext );
                    }
                    
                    DOMNamedNodeMap* attributes = cn->getAttributes();
                    int attrCount = attributes->getLength();
                    for (int i = 0; i < attrCount; i++)
                    {
                        DOMNode* attribute = attributes->item(i);
                        string attrName = tostr( attribute->getNodeName() );
                        string        v = tostr( attribute->getNodeValue() );
                        LG_XSLTFS_D << "A attrName:" << attrName << " v:" << v << endl;
                        if( attribute->getNamespaceURI() )
                        {
                            attrName = tostr(attribute->getNamespaceURI())
                                + tostr(attribute->getLocalName());
                            LG_XSLTFS_D << "BBB attrName:" << attrName << " v:" << v << endl;
                        }

                        // PURE DEBUG
                        {
//                         if( attribute->getNamespaceURI() )
//                             LG_XSLTFS_D << "AAAAA attrName:" << attrName
//                                         << " nsURI:" << tostr(attribute->getNamespaceURI()) << endl;
//                         if( attribute->getLocalName() )
//                             LG_XSLTFS_D << "AAAAA attrName:" << attrName
//                                         << " ln:" << tostr(attribute->getLocalName()) << endl;
                        }
                        
                        
                        if( attrName == "name" && ignoreNameAttribute )
                            continue;
                        
                        LG_XSLTFS_D << "UPDATE-ATTR c:" << cc->getURL()
                                    << " attr:" << attrName << " value:" << v << endl;
                        bool create = true;
                        bool throw_for_errors = true;
                        bool dontDelegateToOvermountContext = false;
                        if( ShouldPerformUpdates() )
                            setStrAttr( cc, attrName, v, create, throw_for_errors,
                                        dontDelegateToOvermountContext );
                    }
                    
                    processReverseXSLTUpdates_Relative( theDOM, cn, cc, autoCreate );
                }
            }
        
        void processReverseXSLTUpdates_Relative( fh_domdoc theDOM )
            {
//                fh_Self dc = dynamic_cast<XSLTFS_DOMWrapper*>( GetImpl(*this->begin()) );
//                fh_context dc = *(getInputXMLContext()->begin());
                fh_context dc = getInputXMLContext();
                DOMNode* dn = theDOM->getDocumentElement();

                bool autoCreate = isTrue( ::Ferris::getAttribute( (DOMElement*)dn, "autocreate" ) );

                int strip = 0;
                {
                    XMLCh* kx = XMLString::transcode( "strip" );
                    if( dn->getNodeType() == DOMNode::ELEMENT_NODE 
                        && ((DOMElement*)dn)->hasAttribute( kx ) )
                    {
                        strip = toint(::Ferris::getAttribute( (DOMElement*)dn, "strip" ));
                    }
                    XMLString::release( &kx );
                }

                LG_XSLTFS_D << "processReverseXSLTUpdates_Relative(checking strip)"
                            << " autoCreate:" << autoCreate
                            << " strip:" << strip
                            << " this:" << getURL()
                            << " rdn:" << getDirName()
                            << " parent:" << getParent()->getURL()
                            << " dc:" << dc->getURL()
                            << endl;
                
                for( ; strip > 0 ; --strip )
                {
                    DOMNodeList* nl = dn->getChildNodes();
                    LG_XSLTFS_D << "strip:" << strip << " nl.sz:" << nl->getLength() << endl;
                
                    for( int i=0; i < nl->getLength(); ++i )
                    {
                        DOMNode*   cn = nl->item( i );
                        LG_XSLTFS_D << " cn.name:" << tostr(cn->getNodeName()) << endl;
                        if( cn->getNodeType() != DOMNode::ELEMENT_NODE )
                            continue;
                        DOMElement* e = (DOMElement*)cn;
                        dn = e;
                        break;
                    }
                }
                
                LG_XSLTFS_D << "processReverseXSLTUpdates_Relative(TOP)"
                            << " autoCreate:" << autoCreate
                            << " strip:" << strip
                            << " this:" << getURL()
                            << " rdn:" << getDirName()
                            << " parent:" << getParent()->getURL()
                            << " dc:" << dc->getURL()
                            << endl;
                
                processReverseXSLTUpdates_Relative( theDOM, dn, dc, autoCreate );
            }
        
        void processReverseXSLTUpdates_Explicit( fh_domdoc theDOM )
            {
                del_t del;
                    
                del = XML::getAllChildrenElements(
                    theDOM->getDocumentElement(),
                    "context", true );
                LG_XSLTFS_D << "del.sz:" << del.size() << endl;

                for( del_t::iterator di = del.begin(); di!=del.end(); ++di )
                {
                    DOMElement* de = *di;
                    string earl = ::Ferris::getAttribute( de, (string)"url" );
                    string content = XML::getChildText( de );
                        
                    LG_XSLTFS_D << "de.url:" << earl << endl;
                    LG_XSLTFS_D << "de.content:" << content << endl;

                    fh_context c = Resolve( earl );
                    if( ShouldPerformUpdates() )
                        setStrAttr( c, "content", content );
                }

                del = XML::getAllChildrenElements(
                    theDOM->getDocumentElement(),
                    "attribute", true );
                LG_XSLTFS_D << "del.sz:" << del.size() << endl;

                for( del_t::iterator di = del.begin(); di!=del.end(); ++di )
                {
                    DOMElement* de = *di;
                    string earl = ::Ferris::getAttribute( de, (string)"url" );
                    string rdn  = ::Ferris::getAttribute( de, (string)"name" );
                    string content = XML::getChildText( de );
                        
                    LG_XSLTFS_D << "de.url:" << earl << endl;
                    LG_XSLTFS_D << "de.content:" << content << endl;

                    fh_context c = Resolve( earl );
                    if( ShouldPerformUpdates() )
                        setStrAttr( c, rdn, content, true, true );
                }
            }
        
        void processReverseXSLT()
            {
                LG_XSLTFS_D << "processReverseXSLT() url:" << getURL() << endl;
                
                string stylesheet_earl = getReverseStyleSheetURL();
                fh_domdoc inputDOM = getDOM();

                LG_XSLTFS_D << "stylesheet_earl:" << stylesheet_earl << endl;
                if( stylesheet_earl.empty() )
                {
                    fh_stringstream ss;
                    ss << "No reverse stylesheet given!";
                    Throw_getIOStreamCloseUpdateFailed( tostr(ss), this );
                }
                
                if( LG_XSLTFS_D_ACTIVE )
                {
                    fh_stringstream ss = tostream( inputDOM );
                    LG_XSLTFS_D << "revxslt. input:" << tostr(ss) << endl;
                }

                
                XercesParserLiaison theParserLiaison;
                XercesDOMSupport theDOMSupport( theParserLiaison );

                const XercesDOMWrapperParsedSource parsedSource(
                    GetImpl(inputDOM), theParserLiaison, theDOMSupport );

                fh_domdoc outputDOM = Factory::makeDOM("");
                LG_XSLTFS_D << "Have output dom" << endl;
                LG_XSLTFS_D << "applying stylesheet at:" << stylesheet_earl << endl;
                FormatterToXercesDOM theFormatter( GetImpl(outputDOM), 0);
                XalanTransformer theXalanTransformer;
                int theResult = theXalanTransformer.transform( parsedSource,
                                                               stylesheet_earl.c_str(),
                                                               theFormatter );
                if(theResult != 0)
                {
                    string e = theXalanTransformer.getLastError();
                    LG_XSLTFS_W << "XalanError: \n" << e << endl;
                    cerr << "XalanError: \n" << e << endl;
                    Throw_getIOStreamCloseUpdateFailed( e, this );
                }
                LG_XSLTFS_D << "transform() performed" << endl;
                if( LG_XSLTFS_D_ACTIVE )
                {
                    fh_stringstream ss = tostream( outputDOM );
                    LG_XSLTFS_D << "output:" << tostr(ss) << endl;
                }

                LG_XSLTFS_D << "creating sync delayer object." << endl;
                SyncDelayer syncObj1;
                LG_XSLTFS_D << "sync delayer exists:" << SyncDelayer::exists()  << endl;
                DOMNode* dn = outputDOM->getDocumentElement();
                if( tostr(dn->getNodeName()) == "explicit-updates" )
                    processReverseXSLTUpdates_Explicit( outputDOM );
                else
                    getBaseContext<_Self>()->processReverseXSLTUpdates_Relative( outputDOM );

                if( starts_with( getInputXMLContext()->getURL(), "etagere:" ))
                {
                    Factory::getEtagere()->sync();
                }
            }
        
        
    protected:

        virtual stringset_t& getForceLocalAttributeNames()
            {
                static stringset_t ret;
                ret.insert("name");
                ret.insert("path");
                ret.insert("url");
                return ret;
            }
        
        virtual void OnDocumentRootStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);

                LG_XSLTFS_D << "OnStreamClosed() s:" << s << endl;
                LG_XSLTFS_D << "OnStreamClosed() this:" << this
                            << " base:" << getBaseContext<_Self>() << endl;

                if( this == getBaseContext<_Self>() )
                {
                    //
                    // have to update underlying filesystem to the new XML file
                    //
                    
//                     fh_context bc = Factory::mountDOM( outputDOM );
//                     fh_context wc = new XSLTFS_DOMWrapper( this, bc, rdn, outputDOM );
//                     Insert( GetImpl(wc) );
                    
                    processReverseXSLT();
                }
            }

        virtual void OnSubStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);

                LG_XSLTFS_D << "OnSubStreamClosed() this:" << getURL() << endl;
                LG_XSLTFS_D << "OnSubStreamClosed() s:" << s << endl;
                processReverseXSLT();
            }
        
        ferris_ios::openmode getSupportedOpenModes()
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        
        fh_stringstream priv_getRealStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;

                fh_stringstream ss;
                if( !( m & ios_base::trunc ) )
                {
                    fh_stringstream ss = tostream( getDOM() );
                    return ss;
                }
                return ss;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;

                return priv_getRealStream( m );
            }
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);

    public:
        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;

                if( this == getBaseContext<_Self>() )
                    return Context::getIStream( m );
                return _Base::getIStream( m );
            }
        
            
    
        virtual fh_iostream getIOStream( ferris_ios::openmode m = std::ios::in|std::ios::out )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;

                if( this == getBaseContext<_Self>() )
                {
                    LG_XSLTFS_D << "getIOStream() on document root element" << endl;
                    return Context::getIOStream( m );
                }
                

                LG_XSLTFS_D << "getIOStream() on non base element" << endl;
                fh_iostream ret = _Base::getIOStream( m );
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun( *this, &_Self::OnSubStreamClosed ), m ));
                return ret;
            }
    protected:
        
        bool
        shouldInsertContext( const fh_context& c, bool created )
            {
                string rdn = c->getDirName();

                // dont bind it twice for ourself.
                if( priv_discoveredSubContext( rdn, created ) )
                    return false;

                return true;
            }
        

        void
        cascadedInsert( Context* c, bool created )
            {
                fh_context cc = new _Self( this, c );
                Insert( GetImpl(cc), created );
            }
        
        void
        OnExists( NamingEvent_Exists* ev,
                  const fh_context& subc,
                  string olddn, string newdn )
            {
                LG_XSLTFS_D << "xslt...OnExists() subc:" << subc->getURL() << endl;
                if( shouldInsertContext( subc, false ) )
                {
                    cascadedInsert( GetImpl(subc), false );
                }
                else
                {
                    LG_CTX_D << "OnExists() not inserting context:" << subc->getDirPath() << endl;
                }
            }

        void
        OnCreated( NamingEvent_Created* ev,
                   const fh_context& subc,
                   std::string olddn, std::string newdn )
            {
                LG_XSLTFS_D << "xslt...OnCreated() subc:" << subc->getURL() << endl;
                if( shouldInsertContext( subc, true ) )
                {
                    cascadedInsert( GetImpl(subc), true );
                }
                else
                {
                    LG_CTX_D << "OnCreated() not inserting context:" << subc->getDirPath() << endl;
                }
            }
        
        const std::string&
        getDirName() const
            {
                return Context::getDirName();
            }

        string
        getDirPath() throw (FerrisParentNotSetError)
            {
                return Context::getDirPath();
            }


        virtual std::string getURL()
            {
                return Context::getURL();
            }
        
        virtual std::string getURLScheme()
            {
                return xgetURLScheme( Shared_getFSArgs( this ) );
            }

        virtual void priv_read()
            {
                LG_XSLTFS_D << "xslt.dom.priv_read... url:" << getURL() << endl;
                
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    SetupEventConnections();
                    Delegate->read( true );
                }
            }
        
        
        void read( bool force )
            {
                LG_XSLTFS_D << "xslt.dom.read... url:" << getURL() << endl;
                Context::read( force );
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:
        XSLTFS_DOMWrapper( fh_context parent, fh_context ctx )
            :
            _Base( ctx, false )
            {
                setContext( GetImpl( parent ), ctx->getDirName() );

                createStateLessAttributes( true );
//                SetupEventConnections();                
            }
        XSLTFS_DOMWrapper( fh_context parent, fh_context ctx, const std::string& n,
                           fh_domdoc dom = 0, fh_context m_inputXMLContext = 0 )
            :
            _Base( ctx, false ),
            m_dom( dom ),
            m_inputXMLContext( m_inputXMLContext )
            {
                setContext( GetImpl( parent ), n );
                createStateLessAttributes( true );
//                SetupEventConnections();                
            }
    };


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    template< class ParentClass >
    class XSLTProcessorBaseClass
        :
        public ParentClass
    {
        typedef ParentClass _Base;

        string m_reverseStyleSheetURL;
        string m_implicitArgsVirtualDirExtension;
        
    protected:

        string find_Xsltfs_Stylesheet_Path( const std::string s )
            {
                if( s.empty() )
                    return s;
                
                string d = getConfigString( FDB_GENERAL,
                                            CFG_XSLTFS_STYLESHEET_PATH_K,
                                            CFG_XSLTFS_STYLESHEET_PATH_DEFAULT );
                stringlist_t sl;
                Util::parseNullSeperatedList( d, sl );
                sl.push_back( (string)CFG_XSLTFS_STYLESHEET_PATH_DEFAULT );

                if( const gchar* p = g_getenv ("LIBFERRIS_XSLTFS_SHEETS_URL") )
                {
                    sl.push_front( (string)p );
                }
                
                for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                {
                    try
                    {
                        string earl = *si + "/" + s;
                        fh_context c = Resolve( earl );
                        return c->getURL();
                    }
                    catch( ... )
                    {}
                }
                return "";
            }

        fh_domdoc applyForwardSheet( fh_context xmlContext, StringMap_t& args )
            {
                LG_XSLTFS_D << "applyForwardSheet(called) xmlContext:" << xmlContext->getURL() << endl;
                
                string stylesheet_earl = "";
                string reverse_stylesheet_earl = "";

                if( args.empty() )
                {
                    fh_stringstream ss;
                    ss << "Can't work out what the stylesheets should be for "
                       << "url:" << this->getURL();
                    Throw_NoSuchSubContext( tostr(ss), this );
                }
                
                if( !args["implicit-sheets"].empty() )
                {
                    LG_XSLTFS_D << "args[implicit-sheets]:" << args["implicit-sheets"] << endl;
                    string dirPath = "~/.ferris/filesystem-to-xsltfs-sheets/" + args["implicit-sheets"];
                    fh_context d = Resolve( dirPath );
                    LG_XSLTFS_D << "1.d:" << d->getURL() << endl;
                    args["stylesheet"] = d->getURL() + "/forward-sheet.xsl";
                    args["reverse-stylesheet"] = d->getURL() + "/reverse-sheet.xsl";
//                     string s = getStrSubCtx( d, "append-extension", "" );
//                     args["append-extension"] = s;
                }
                else
                {
                    args["stylesheet"] = find_Xsltfs_Stylesheet_Path( args["stylesheet"] );
                    args["reverse-stylesheet"] = find_Xsltfs_Stylesheet_Path( args["reverse-stylesheet"] );
                }
                    

                stylesheet_earl = args["stylesheet"];
                reverse_stylesheet_earl = args["reverse-stylesheet"];
                m_reverseStyleSheetURL = reverse_stylesheet_earl;
                LG_XSLTFS_D << "1. stylesheet:" << stylesheet_earl << endl;
                LG_XSLTFS_D << "2. reverse_stylesheet_earl:" << reverse_stylesheet_earl << endl;

                if( stylesheet_earl.empty() )
                {
                    fh_stringstream ss;
                    ss << "Can't work out what the stylesheets should be for "
                       << "url:" << this->getURL();
                    Throw_NoSuchSubContext( tostr(ss), this );
                }
                
                

                LG_XSLTFS_D << "applyForwardSheet(making dom) xmlContext:" << xmlContext->getURL() << endl;
//                fh_context xmlContext = Delegate->getSubContext( delegate_rdn );
                fh_domdoc theDOM = Factory::makeDOM( xmlContext );

//                 if( LG_XSLTFS_D_ACTIVE )
//                 {
//                     fh_stringstream ss = tostream( theDOM );
//                     LG_XSLTFS_D << "input document...\n" << tostr(ss) << endl;
//                 }
                    
                LG_XSLTFS_D << "xmlContext:" << xmlContext->getURL() << endl;
                    
                XercesParserLiaison theParserLiaison;
                XercesDOMSupport theDOMSupport( theParserLiaison );

                const XercesDOMWrapperParsedSource parsedSource(
                    GetImpl(theDOM), theParserLiaison, theDOMSupport );
                LG_XSLTFS_D << "have parsed source..." << endl;

                fh_domdoc outputDOM = Factory::makeDOM("");
                LG_XSLTFS_D << "Have output dom" << endl;
                LG_XSLTFS_D << "applying stylesheet at:" << stylesheet_earl << endl;
                FormatterToXercesDOM theFormatter( GetImpl(outputDOM), 0);
                XalanTransformer theXalanTransformer;
                int theResult = theXalanTransformer.transform( parsedSource,
                                                               stylesheet_earl.c_str(),
                                                               theFormatter );
                LG_XSLTFS_D << "transform() performed" << endl;
                if(theResult != 0)
                {
                    string e = theXalanTransformer.getLastError();
                    LG_XSLTFS_W << "XalanError: \n" << e << endl;
                    cerr << "XalanError: \n" << e << endl;
                    Throw_NoSuchSubContext( e, this );
                }

//                 if( LG_XSLTFS_D_ACTIVE )
//                 {
//                     fh_stringstream ss = tostream( outputDOM );
//                     LG_XSLTFS_D << "transformed document...\n" << tostr(ss) << endl;
//                 }

                return outputDOM;
            }
        
        
        fh_context applyForwardSheetAndInsert( string& rdn, fh_context xmlContext, StringMap_t& args )
            {
                Context::Items_t::iterator isSubContextBoundCache;
                if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                {
                    LG_XSLTFS_D << "priv_getSubContext(bound already) p:" << this->getDirPath()
                                << " rdn:" << rdn
                                << endl;
                    return *isSubContextBoundCache;
                }
                
                fh_domdoc outputDOM = applyForwardSheet( xmlContext, args );

                fh_context bc = Factory::mountDOM( outputDOM );
                fh_context wc = new XSLTFS_DOMWrapper( this, bc, rdn, outputDOM, xmlContext );
                LG_XSLTFS_D << " bc:" << bc->getDirName()
                     << " bc.url:" << bc->getURL()
                     << " wc:" << wc->getDirName()
                     << " wc.url:" << wc->getURL()                    
                     << "---- rdn:" << rdn
                     << endl;
                this->Insert( GetImpl(wc) );
                return wc;
            }
        
        
        
    public:
        XSLTProcessorBaseClass( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn ),
            m_implicitArgsVirtualDirExtension("")
            {
            }
        XSLTProcessorBaseClass( fh_context ctx, bool v )
            :
            _Base( ctx, v ),
            m_implicitArgsVirtualDirExtension("")
            {
            }
        string getReverseStyleSheetURL()
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << ((Context*)this)->getURL()
                            << " m_reverseStyleSheetURL:" << m_reverseStyleSheetURL
                            << endl;
                return m_reverseStyleSheetURL;
            }

        string
        getImplicitArgsVirtualDirExtension()
            {
                if( m_implicitArgsVirtualDirExtension.empty() )
                {
                    string fsArgs = getFSArgs();
                    StringMap_t args = Util::ParseKeyValueString( fsArgs );
                    
                    if( !args["append-extension"].empty() )
                        m_implicitArgsVirtualDirExtension = args["append-extension"];
                    else
                    {
                        if( !args["implicit-sheets"].empty() )
                        {
                            string dirPath = "~/.ferris/filesystem-to-xsltfs-sheets/" + args["implicit-sheets"];
                            fh_context d = Resolve( dirPath );
                            m_implicitArgsVirtualDirExtension = getStrSubCtx( d, "append-extension", "" );
                        }

                        if( m_implicitArgsVirtualDirExtension.empty() )
                        {
                            m_implicitArgsVirtualDirExtension = "-xsltfs";
                        }
                    }
                }
                return m_implicitArgsVirtualDirExtension;
            }
        string
        getFSArgs()
            {
                return Shared_getFSArgs( this );
//                 string ret = "";

//                 xsltfsRootContext* rc = 0;
//                 rc = this->getFirstParentOfContextClass( rc );

//                 if( rc )
//                 {
//                     ret = getStrAttr( (Context*)rc, "filesystem-args", "" );
//                 }

//                 return ret;
            }

        
    };
    
    
    class FERRISEXP_DLLLOCAL Lazy_XSLTFS_DOMWrapper
        :
        public StateLessEAHolder< Lazy_XSLTFS_DOMWrapper, XSLTProcessorBaseClass< FakeInternalContext > >
    {
        typedef Lazy_XSLTFS_DOMWrapper _Self;
        typedef StateLessEAHolder< Lazy_XSLTFS_DOMWrapper, XSLTProcessorBaseClass< FakeInternalContext >  > _Base;

        fh_domdoc m_doc;
        fh_context m_docfs;
        fh_XSLTFS_DOMWrapper m_real_XSLTFS_DOMWrapper;
        
        void ensureDocFS()
            {
                if( !m_docfs )
                    priv_read();
            }
        
    protected:

        bool
        disableOverMountingForContext()
            {
                return true;
            }
    
        
        std::string getURLScheme()
            {
                return xgetURLScheme( getFSArgs() );
            }
        
        ferris_ios::openmode getSupportedOpenModes()
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                ensureDocFS();
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl
                            << " m_docfs:" << m_docfs->getURL()
                            << endl;
                if( LG_XSLTFS_D_ACTIVE )
                {
                    fh_stringstream ss = tostream( m_doc );
                    LG_XSLTFS_D << " m_doc:" << toVoid(m_doc)
                                << " m_docfs:" << toVoid(m_docfs)
                                << " doc.sz:" << tostr(ss).size()
                                << " m_doc...\n" << tostr(ss) << "---END-OF-DOC---" << endl;
                }
                
                return m_docfs->getIStream( m );
            }
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                ensureDocFS();
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl
                            << " m_docfs:" << m_docfs->getURL()
                            << endl;
                if( !(m & ios_base::out ))
                {
                    LG_XSLTFS_D << __PRETTY_FUNCTION__ << " using m_docfs" << endl;
                    return m_docfs->getIOStream( m );
                }
                
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " using m_real_XSLTFS_DOMWrapper" << endl;
                return m_real_XSLTFS_DOMWrapper->getIOStream( m );
            }

        
        virtual void priv_read()
            {
                LG_XSLTFS_D << "lazy.xslt.dom.priv_read(begin)... url:" << getURL() << endl;
                
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
//                     // FIXME:
//                     if( getDirName() != "msgs-xsltfs2" )
//                         return;
                    
                    fh_context p = getParent();
                    string rdn = getDirName();

                    string fsArgs = getFSArgs();
                    StringMap_t args = Util::ParseKeyValueString( fsArgs );
                    
                    fh_context xmlContext = p->getSubContext(
                        rdn.substr( 0,
                                    rdn.length() - getImplicitArgsVirtualDirExtension().length() ));

                    if( starts_with( xmlContext->getURL(), "xsltfs" ) )
                    {
                        string rawPath = xmlContext->getDirPath();
                        PrefixTrimmer trimmer;
                        trimmer.push_back( "/" );
                        trimmer.push_back( "context" );
                        rawPath = trimmer( rawPath );

                        int firstSlash = rawPath.find( '/' );
                        stringstream earlss;
                        earlss << rawPath.substr( 0, firstSlash );
                        earlss << "://";
                        earlss << rawPath.substr( firstSlash+1 );

                        LG_XSLTFS_D << " non xstlfs:// prefixed URL:" << tostr(earlss) << endl;
                        
                        xmlContext = Resolve( tostr(earlss) );
                    }
                    xmlContext->read();
                    
                    LG_XSLTFS_D << "rdn:" << rdn
                                << " xmlC:" << xmlContext->getURL()
                                << " applying XSLT" << endl;
//                    BackTrace();

//                     fh_context wc = applyForwardSheetAndInsert( rdn, xmlContext, args );
//                     LG_XSLTFS_D << "lazy.xslt.dom.priv_read... wc.name:" << wc->getDirName()
//                          << " wc.strn:" << getStrAttr( wc, "name", "<none>" )
//                          << " wc.url:" << wc->getURL() << endl;
//                     LG_XSLTFS_D << " subc#:" << getSubContextCount() << endl;
// // //                     dumpOutItems();


                    
                    
                    m_doc = applyForwardSheet( xmlContext, args );
                    m_docfs = Factory::mountDOM( m_doc );
                    if( m_docfs->getSubContextCount() )
                    {
                        fh_context c = *(m_docfs->begin());
                        string rdn = c->getDirName();

                        m_real_XSLTFS_DOMWrapper = new XSLTFS_DOMWrapper( this, c, rdn, m_doc, xmlContext );
                        fh_context wc = GetImpl(m_real_XSLTFS_DOMWrapper);
                        LG_XSLTFS_D << " wc:" << wc->getDirName()
                             << " wc.url:" << wc->getURL()                    
                             << "---- rdn:" << rdn
                             << endl;
                        this->Insert( GetImpl(wc) );
                    }
                    
                    
//                    SetupEventConnections();
//                    Delegate->read( true );
                }

                LG_XSLTFS_D << "lazy.xslt.dom.priv_read(end)... url:" << getURL() << endl;
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:
        Lazy_XSLTFS_DOMWrapper( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
//            m_doc( 0 )
            {
                LG_XSLTFS_D << "Lazy_XSLTFS_DOMWrapper() rdn:" << rdn << endl;
                createStateLessAttributes( true );
            }
        fh_domdoc getDOM()
            {
                return m_doc;
            }
    };
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    class FERRISEXP_DLLLOCAL NormalContextXSLTFSWrapper
        :
        public XSLTProcessorBaseClass< ChainedViewContext >
    {
        typedef XSLTProcessorBaseClass< ChainedViewContext > _Base;
        typedef NormalContextXSLTFSWrapper _Self;

//         string m_reverseStyleSheetURL;
        
    public:
        std::string
        getURL()
            {
                return Context::getURL();
            }
        std::string getURLScheme()
            {
                return xgetURLScheme( getFSArgs() );
            }
    protected:
        
        virtual stringset_t& getForceLocalAttributeNames()
            {
                static stringset_t ret;
                ret.insert("name");
                ret.insert("path");
                ret.insert("url");
                return ret;
            }

        const std::string&
        getDirName() const
            {
                return Context::getDirName();
            }

        string
        getDirPath() throw (FerrisParentNotSetError)
            {
                return Context::getDirPath();
            }
        
        bool
        shouldInsertContext( const fh_context& c, bool created );

        void
        cascadedInsert( Context* c, bool created )
            {
                fh_context cc = new _Self( this, c );
                Insert( GetImpl(cc), created );
            }
        
        void
        OnExists( NamingEvent_Exists* ev,
                  const fh_context& subc,
                  string olddn, string newdn )
            {
                LG_XSLTFS_D << "xslt...OnExists() subc:" << subc->getURL() << endl;
                if( shouldInsertContext( subc, false ) )
                {
                    cascadedInsert( GetImpl(subc), false );
                }
                else
                {
                    LG_CTX_D << "OnExists() not inserting context:" << subc->getDirPath() << endl;
                }
            }

        void
        OnCreated( NamingEvent_Created* ev,
                   const fh_context& subc,
                   std::string olddn, std::string newdn )
            {
                LG_XSLTFS_D << "xslt...OnCreated() subc:" << subc->getURL() << endl;
                if( shouldInsertContext( subc, true ) )
                {
                    cascadedInsert( GetImpl(subc), true );
                }
                else
                {
                    LG_CTX_D << "OnCreated() not inserting context:" << subc->getDirPath() << endl;
                }
            }

        
        virtual void priv_read()
            {
                LG_XSLTFS_D << "xslt.priv_read... url:" << getURL() << endl;
                LG_XSLTFS_D << "xslt.priv_read... this.sz:" << getSubContextCount() << endl;
                LG_XSLTFS_D << "xslt.priv_read... D.url:" << Delegate->getURL() << endl;
                
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    SetupEventConnections();
                    Delegate->read( true );

                    LG_XSLTFS_D << "NormalContextXSLTFSWrapper::priv_read() empty, fsargs:" << getFSArgs() << endl;
                    
                    bool haveFSArgs = false;
                    string fsArgs = getFSArgs();
                    haveFSArgs = !fsArgs.empty();
                    StringMap_t args = Util::ParseKeyValueString( fsArgs );

                    LG_XSLTFS_D << "NormalContextXSLTFSWrapper::priv_read(2)" << endl;
                    
                    if( haveFSArgs )
                    {
                        typedef list< fh_context > clist_t;
                        clist_t clist;
                        Context::Items_t::iterator e = getItems().end();
                        for( Context::Items_t::iterator ci = getItems().begin(); ci != e; ++ci )
                        {
                            clist.push_back( *ci );
                        }
                        LG_XSLTFS_D << "NormalContextXSLTFSWrapper::priv_read(3)" << endl;

                        for( clist_t::iterator ci = clist.begin(); ci != clist.end(); ++ci )
                        {
                            string delegate_rdn = (*ci)->getDirName();
                            string rdn = delegate_rdn + getImplicitArgsVirtualDirExtension();

                            LG_XSLTFS_D << "NormalContextXSLTFSWrapper::priv_read(4L) rdn:" << rdn << endl;
                            
                            fh_context wc = new Lazy_XSLTFS_DOMWrapper( this, rdn );
                            Insert( GetImpl(wc) );
//                            fh_context wc = applyForwardSheetAndInsert( rdn, delegate_rdn, args );
                        }
                    }
                }
            }
        
        
        void read( bool force )
            {
                LG_XSLTFS_D << "xslt.read... url:" << getURL() << endl;
                Context::read( force );
            }

        string find_Xsltfs_Stylesheet_Path( const std::string s )
            {
                if( s.empty() )
                    return s;
                
                string d = getConfigString( FDB_GENERAL,
                                            CFG_XSLTFS_STYLESHEET_PATH_K,
                                            CFG_XSLTFS_STYLESHEET_PATH_DEFAULT );
                stringlist_t sl;
                Util::parseNullSeperatedList( d, sl );
                sl.push_back( (string)CFG_XSLTFS_STYLESHEET_PATH_DEFAULT );
                
                for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                {
                    try
                    {
                        string earl = *si + "/" + s;
                        fh_context c = Resolve( earl );
                        return c->getURL();
                    }
                    catch( ... )
                    {}
                }
                return "";
            }

//        string getFSArgs();

//         fh_context applyForwardSheet( string& rdn, string& delegate_rdn, StringMap_t& args )
//             {
//                 LG_XSLTFS_D << "applyForwardSheet(called) rdn:" << rdn << endl;
                
//                 string stylesheet_earl = "";
//                 string reverse_stylesheet_earl = "";

//                 if( args.empty() )
//                 {
//                     fh_stringstream ss;
//                     ss << "Can't work out what the stylesheets should be for "
//                        << "url:" << getURL();
//                     Throw_NoSuchSubContext( tostr(ss), this );
//                 }
                
//                 if( !args["implicit-sheets"].empty() )
//                 {
//                     LG_XSLTFS_D << "args[implicit-sheets]:" << args["implicit-sheets"] << endl;
//                     string dirPath = "~/.ferris/filesystem-to-xsltfs-sheets/" + args["implicit-sheets"];
//                     fh_context d = Resolve( dirPath );
//                     args["stylesheet"] = d->getURL() + "/forward-sheet.xsl";
//                     args["reverse-stylesheet"] = d->getURL() + "/reverse-sheet.xsl";
//                 }
//                 else
//                 {
//                     args["stylesheet"] = find_Xsltfs_Stylesheet_Path( args["stylesheet"] );
//                     args["reverse-stylesheet"] = find_Xsltfs_Stylesheet_Path( args["reverse-stylesheet"] );
//                 }
                    

//                 stylesheet_earl = args["stylesheet"];
//                 reverse_stylesheet_earl = args["reverse-stylesheet"];
//                 m_reverseStyleSheetURL = reverse_stylesheet_earl;
//                 LG_XSLTFS_D << "stylesheet:" << stylesheet_earl << endl;
//                 LG_XSLTFS_D << "reverse_stylesheet_earl:" << reverse_stylesheet_earl << endl;

//                 if( stylesheet_earl.empty() )
//                 {
//                     fh_stringstream ss;
//                     ss << "Can't work out what the stylesheets should be for "
//                        << "url:" << getURL();
//                     Throw_NoSuchSubContext( tostr(ss), this );
//                 }
                
                
//                 Items_t::iterator isSubContextBoundCache;
//                 if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
//                 {
//                     LG_XSLTFS_D << "priv_getSubContext(bound already) p:" << getDirPath()
//                                 << " rdn:" << rdn
//                                 << endl;
//                     return *isSubContextBoundCache;
//                 }

//                 fh_context xmlContext = Delegate->getSubContext( delegate_rdn );
//                 fh_domdoc theDOM = Factory::makeDOM( xmlContext );

//                 if( LG_XSLTFS_D_ACTIVE )
//                 {
//                     fh_stringstream ss = tostream( theDOM );
//                     LG_XSLTFS_D << "input document...\n" << tostr(ss) << endl;
//                 }
                    
//                 LG_XSLTFS_D << "xmlContext:" << xmlContext->getURL() << endl;
                    
//                 XercesDOMSupport theDOMSupport;
//                 XercesParserLiaison theParserLiaison(theDOMSupport);

//                 const XercesDOMWrapperParsedSource parsedSource(
//                     GetImpl(theDOM), theParserLiaison, theDOMSupport );
//                 LG_XSLTFS_D << "have parsed source..." << endl;

//                 fh_domdoc outputDOM = Factory::makeDOM("");
//                 LG_XSLTFS_D << "Have output dom" << endl;
//                 LG_XSLTFS_D << "applying stylesheet at:" << stylesheet_earl << endl;
//                 FormatterToXercesDOM theFormatter( GetImpl(outputDOM), 0);
//                 XalanTransformer theXalanTransformer;
//                 int theResult = theXalanTransformer.transform( parsedSource,
//                                                                stylesheet_earl.c_str(),
//                                                                theFormatter );
//                 LG_XSLTFS_D << "transform() performed" << endl;
//                 if(theResult != 0)
//                 {
//                     string e = theXalanTransformer.getLastError();
//                     LG_XSLTFS_W << "XalanError: \n" << e << endl;
//                     cerr << "XalanError: \n" << e << endl;
//                     Throw_NoSuchSubContext( e, this );
//                 }

//                 if( LG_XSLTFS_D_ACTIVE )
//                 {
//                     fh_stringstream ss = tostream( outputDOM );
//                     LG_XSLTFS_D << "transformed document...\n" << tostr(ss) << endl;
//                 }
                    
//                 fh_context bc = Factory::mountDOM( outputDOM );
//                 fh_context wc = new XSLTFS_DOMWrapper( this, bc, rdn, outputDOM, xmlContext );
//                 Insert( GetImpl(wc) );
//                 return wc;
//             }
        
        
        fh_context
        priv_getSubContext( const string& rdn_const )
            throw( NoSuchSubContext )
            {
                string rdn = rdn_const;
                string delegate_rdn = rdn;
                
                LG_XSLTFS_D << "priv_getSubContext() rdn:" << rdn << endl;

                string args_string = "";
                int qpos = rdn.find( "?" );
                if( qpos != string::npos )
                {
                    args_string = rdn.substr( qpos + 1 );
                    delegate_rdn = delegate_rdn.substr( 0, qpos );
                }
                
                LG_XSLTFS_D << "FOUND AUGMENTED RDN:" << rdn_const << endl;
                LG_XSLTFS_D << "args_string:" << args_string << endl;
                StringMap_t args = Util::ParseKeyValueString( args_string );

                Items_t::iterator isSubContextBoundCache;
                if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
                {
                    LG_XSLTFS_D << "priv_getSubContext(bound already) p:" << getDirPath()
                                << " rdn:" << rdn
                                << endl;
                    return *isSubContextBoundCache;
                }
                
                if( !args.empty() )
                {
                    fh_context wc = applyForwardSheetAndInsert(
                        rdn, Delegate->getSubContext( delegate_rdn ), args );

                    return wc;
                }

                fh_context bc = Delegate->getSubContext( rdn );
                fh_context wc = new NormalContextXSLTFSWrapper( this, bc );
                LG_XSLTFS_D << "priv_getSubContext() ncwrap. bc:" << bc->getDirName() << " rdn:" << rdn << endl;
                Insert( GetImpl(wc) );
                return wc;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:

        NormalContextXSLTFSWrapper( fh_context parent, fh_context ctx )
            :
            _Base( ctx, false )
            {
                LG_XSLTFS_D << "NormalContextXSLTFSWrapper() ctx:" << ctx->getDirName()
                     << " ctx.url:" << ctx->getURL()
                     << endl;
//                BackTrace();
                
                setContext( GetImpl( parent ), ctx->getDirName() );

                createStateLessAttributes( true );
//                SetupEventConnections();                
            }
        NormalContextXSLTFSWrapper( fh_context parent, fh_context ctx, const std::string& n )
            :
            _Base( ctx, false )
            {
                LG_XSLTFS_D << "NormalContextXSLTFSWrapper() ctx:" << ctx->getDirName() << " n:" << n << endl;
                setContext( GetImpl( parent ), n );

                createStateLessAttributes( true );
//                SetupEventConnections();                
            }
        
//         string getReverseStyleSheetURL()
//             {
//                 return m_reverseStyleSheetURL;
//             }
    };
    
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN xsltfsDynamicRootContext
        :
        public StateLessEAHolder< xsltfsDynamicRootContext, FakeInternalContext >
    {
        typedef xsltfsDynamicRootContext _Self;
        typedef StateLessEAHolder< _Self, FakeInternalContext > _Base;

    protected:

        std::string getURLScheme()
            {
                string fsArgs = getStrAttr( getParent(), "filesystem-args", "" );
                return xgetURLScheme( fsArgs );
            }
        
        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context tc = Resolve( "context://" );

                    LG_XSLTFS_D << "xsltfsDynamicRootContext::priv_read() sz:" << tc->getSubContextCount() << endl;

                    fh_context bc = Resolve( "file://" );
                    fh_context wc = new NormalContextXSLTFSWrapper( this, bc, "file" );
                    Insert( GetImpl(wc) );
                    
                    {
                        fh_context bc = Resolve( "postgresql://" );
                        fh_context wc = new NormalContextXSLTFSWrapper( this, bc, "postgresql" );
                        Insert( GetImpl(wc) );
                    }
                    {
                        fh_context bc = Resolve( "pg://" );
                        fh_context wc = new NormalContextXSLTFSWrapper( this, bc, "pg" );
                        Insert( GetImpl(wc) );
                    }


                    {
                        fh_context bc = Resolve( "etagere://" );
                        fh_context wc = new NormalContextXSLTFSWrapper( this, bc, "etagere" );
                        Insert( GetImpl(wc) );
                    }
                    
//                     for( Context::iterator ci = tc->begin(); ci != tc->end(); ++ci )
//                     {
//                         try
//                         {
//                             string earl = (*ci)->getDirName() + "://";
//                             LG_XSLTFS_D << "earl:" << earl << endl;
                            
// //                             fh_context bc = Resolve( earl );
// //                             fh_context wc = new NormalContextXSLTFSWrapper( this, bc );
// //                             Insert( GetImpl(wc) );
//                         }
//                         catch( exception& e )
//                         {
//                         }
//                     }
                }
            }
        
    public:

        xsltfsDynamicRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xsltfsDynamicRootContext()
            {
            }
        
    };
    
    class FERRISEXP_CTXPLUGIN xsltfsRootContext
        :
        public StateLessEAHolder< xsltfsRootContext, FakeInternalContext >
    {
        typedef xsltfsRootContext                           _Self;
        typedef StateLessEAHolder< xsltfsRootContext, FakeInternalContext > _Base;

        string m_fsArgs;
        
    protected:

        std::string getURLScheme()
            {
                return xgetURLScheme( m_fsArgs );
            }
        
        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context dc = new xsltfsDynamicRootContext( this, "context" );
                    Insert( GetImpl(dc) );
                }
            }

        
        static fh_stringstream
        SL_getFSArgs( xsltfsRootContext* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ret;
                ret << c->m_fsArgs;
                return ret;
            }

        static void
        SL_updateFSArgs( xsltfsRootContext* c, const std::string& rdn,
                         EA_Atom* atom, fh_istream ss )
            {
                c->m_fsArgs = StreamToString( ss );
                LG_XSLTFS_D << "Setting m_fsArgs:" << c->m_fsArgs << endl;
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    tryAddStateLessAttribute( "filesystem-args",
                                              SL_getFSArgs,
                                              SL_getFSArgs,
                                              SL_updateFSArgs,
                                              XSD_BASIC_STRING );
                    
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:

        xsltfsRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        virtual ~xsltfsRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }
        

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        xsltfsRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                xsltfsRootContext* ret = new xsltfsRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
        
    };




        string
        XSLTFS_DOMWrapper::getReverseStyleSheetURL()
            {
                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl;                
                NormalContextXSLTFSWrapper* d = 0;
                d = getFirstParentOfContextClass( d );

                LG_XSLTFS_D << __PRETTY_FUNCTION__ << " d:" << d->getURL() << endl;                
                if( Lazy_XSLTFS_DOMWrapper* lp = dynamic_cast<Lazy_XSLTFS_DOMWrapper*>( getParent() ) )
                {
                    LG_XSLTFS_D << __PRETTY_FUNCTION__ << " lp:" << lp->getURL() << endl; 
                    LG_XSLTFS_D << __PRETTY_FUNCTION__ << " lp->rss:" << lp->getReverseStyleSheetURL() << endl;
                    return lp->getReverseStyleSheetURL();
                }
                
                return d->getReverseStyleSheetURL();
            }

    bool
    NormalContextXSLTFSWrapper::shouldInsertContext( const fh_context& c, bool created )
    {
        string rdn = c->getDirName();

        // dont bind it twice for ourself.
        if( priv_discoveredSubContext( rdn, created ) )
            return false;

        LG_XSLTFS_D << "shouldInsertContext() c:" << c->getURL() << endl;
        
        // dont create a facade ontop of Lazy_XSLTFS_DOMWrapper objects
        if( dynamic_cast<Lazy_XSLTFS_DOMWrapper*>(GetImpl(c)) )
            return false;
        if( dynamic_cast<XSLTFS_DOMWrapper*>(GetImpl(c)) )
            return false;

        return true;
    }

    fh_domdoc
    XSLTFS_DOMWrapper::getDOM()
            {
//                return getBaseContext<_Self>()->m_dom;

//                fh_domdoc ret = (dynamic_cast<XMLBaseContext*>(GetImpl(getBaseContext<_Self>()->Delegate)))->getDocument();
                
                _Self* basec = getBaseContext<_Self>();
                fh_domdoc ret = (dynamic_cast<XMLBaseContext*>(GetImpl(basec->Delegate)))->getDocument();
                if( Lazy_XSLTFS_DOMWrapper* lp = dynamic_cast<Lazy_XSLTFS_DOMWrapper*>( basec->getParent() ) )
                {
                    if( getBaseContext<_Self>()->m_dom )
                    {
                        fh_stringstream ss = tostream( getBaseContext<_Self>()->m_dom );
                        LG_XSLTFS_D << "DUMPING DOMS1... self.dom:" << tostr(ss) << endl;
                    }
                    if( ret )
                    {
                        fh_stringstream ss = tostream( ret );
                        LG_XSLTFS_D << "DUMPING DOMS2... self.dom:" << tostr(ss) << endl;
                    }
//                     if( lp->getDOM() )
//                     {
//                         fh_stringstream ss = tostream( lp->getDOM() );
//                         LG_XSLTFS_D << "DUMPING DOMS3... self.dom:" << tostr(ss) << endl;
//                     }


//                     ret = lp->getDOM();
                }

                LG_XSLTFS_D << "XSLTFS_DOMWrapper::getDOM() m_dom:" << GetImpl(m_dom)
                            << " ret:"
                            << GetImpl(ret) << endl;
                return ret;
            }

    fh_iostream
    XSLTFS_DOMWrapper::priv_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        LG_XSLTFS_D << __PRETTY_FUNCTION__ << " this:" << getURL() << endl
                    << " delegate:" << Delegate->getURL()
                    << endl;

        Parent_t p = getParent();
        if( Lazy_XSLTFS_DOMWrapper* lp = dynamic_cast<Lazy_XSLTFS_DOMWrapper*>( p ) )
        {
            fh_iostream ret = Delegate->getParent()->priv_getIOStream( m );
            ret->getCloseSig().connect( sigc::bind( sigc::mem_fun( *this, &_Self::OnDocumentRootStreamClosed ), m ));
            return ret;
        }
                
        fh_iostream ret = Delegate->priv_getIOStream( m );
//                fh_stringstream ret = priv_getRealStream( m );
        ret->getCloseSig().connect( sigc::bind( sigc::mem_fun( *this, &_Self::OnDocumentRootStreamClosed ), m ));
        return ret;
    }
    
//     string
//     NormalContextXSLTFSWrapper::getFSArgs()
//             {
//                 string ret = "";

//                 xsltfsRootContext* rc = 0;
//                 rc = getFirstParentOfContextClass( rc );

//                 if( rc )
//                 {
//                     ret = getStrAttr( rc, "filesystem-args", "" );
//                 }

//                 return ret;
//             }

//     string
//     X::getFSArgs()
//             {
//                 string ret = "";

//                 xsltfsRootContext* rc = 0;
//                 rc = getFirstParentOfContextClass( rc );

//                 if( rc )
//                 {
//                     ret = getStrAttr( rc, "filesystem-args", "" );
//                 }

//                 return ret;
//             }

    string Shared_getFSArgs( Context* ctx )
    {
        string ret = "";
        
        xsltfsRootContext* rc = 0;
        rc = ctx->getFirstParentOfContextClass( rc );

        if( rc )
        {
            ret = getStrAttr( (Context*)rc, "filesystem-args", "" );
        }
        
        return ret;
    }

    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static xsltfsRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_XSLTFS_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
