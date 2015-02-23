/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libxml.cpp,v 1.10 2010/09/24 21:31:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

/**
 * See XMLContext comment for details
 */

#include <libferrisxmlshared.hh>

#include <Resolver_private.hh>

#include <fstream>
#include <list>
#include <iostream>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <config.h>

using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    

//     static XalanDOMString sstr( const std::string& s )
//     {
//         return XalanDOMString( s.c_str() );
//     }
//     static DOMString xstr( const std::string& s )
//     {
//         return DOMString( s.c_str() );
//     }

    
    

    class DOMTreeErrorReporter;

    /**
     *
     * This class has been reduced to the following tasks:
     * 1) turn file into xerces-c DOM
     * 2) turn xerces-c DOM into file
     *
     * This guts of the DOM/ferris handling is now in XMLBaseContext which
     * is in libferrisxmlshared.cpp in this directory.
     *
     ***************************************************
     *
     * Note that we need to have full metadata and data cross process journaling so that we
     * can avoid reparsing the whole XML file on every change that another process has made.
     */
    class FERRISEXP_CTXPLUGIN XMLContext
        :
        public XMLBaseContext,
        public IHandleOutOfProcContextCreationNotification,
        public IHandleOutOfProcContextDeletionNotification,
        public IHandleOutOfProcEANotification,
        public IHandleOutOfProcContextChangedNotification
    {
        typedef XMLContext      _Self;
        typedef XMLBaseContext  _Base;

        virtual bool supportsMonitoring()
            {
                return shouldPerformFullJournaling();
            }
        
        virtual bool shouldPerformFullJournaling()
            {
#ifdef DEBUGGING_XML_MODULE
                return false;
#endif
                static bool alwaysJournal = false;
                static bool alwaysJournalInitialized = false;

                if( !alwaysJournalInitialized )
                {
                    alwaysJournalInitialized = true;
                    alwaysJournal =
                        isTrue(
                            getEDBString( FDB_GENERAL, "always-journal-xml", "0" ));
                    LG_XML_D << "always-journal-xml config:" << alwaysJournal << endl;

                    if( !alwaysJournal )
                        if( const gchar* p = g_getenv ("LIBFERRIS-ALWAYS-JOURNAL-XML") )
                            alwaysJournal = true;
                }
                if( alwaysJournalInitialized && alwaysJournal )
                    return true;
                
                XMLContext* bc = getBaseContext();
                if( bc != this )
                    return bc->shouldPerformFullJournaling();

                typedef map< XMLContext*, bool > cache_t;
                static cache_t cache;

                cache_t::iterator ci = cache.find( this );
                if( ci != cache.end() )
                {
                    LG_XML_D << "shouldPerformFullJournaling() this:" << getURL()
                             << " ret:" << ci->second << endl;
                    return ci->second;
                }
                bool r = isTrue( getStrAttr( bc, "libferris-journal-changes", "0" ) );
                cache[ this ] = r;

                LG_XML_D << "shouldPerformFullJournaling(adding to cache) this:" << getURL()
                         << " ret:" << r << endl;
                return r;
            }
        

        virtual void OnOutOfProcContextChangedNotification( const std::string& rdn,
                                                            bool isDataValid,
                                                            const std::string& data )
            {
                LG_JOURNAL_D << "OnContextChanged() this:" << this->getURL()
                             << " rdn:" << rdn
                             << " is-bound:" << priv_isSubContextBound( rdn )
                             << " data:" << data
                             << endl;

                DOMElement* node = getElement();
                if( isDataValid )
                {
                    XML::removeAllTextChildren( node );
                    DOMText* t = getDocument()->createTextNode( X(data.c_str()) );
                    node->appendChild( t );
                }
                Emit_Changed( 0, getDirPath(), getDirPath(), 0 );
            }
        
        virtual void OnOutOfProcContextCreationNotification( const std::string& rdn,
                                                             bool isDataValid,
                                                             const std::string& data )
            {
                LG_JOURNAL_D << "OnContextCreated() this:" << this->getURL()
                             << " rdn:" << rdn
                             << " is-bound:" << priv_isSubContextBound( rdn )
                             << endl;

                if( !priv_isSubContextBound( rdn ) )
                {
                    fh_domdoc doc = getDocument();
                    DOMElement* node = XML::createElement( doc, getElement(), rdn );
                    if( isDataValid )
                    {
                        DOMText* t = doc->createTextNode( X( data.c_str() ) );
                        node->appendChild( t );

                         LG_JOURNAL_D << "OnContextCreated() this:" << this->getURL()
                                      << " rdn:" << rdn
                                      << " isDataValid:" << isDataValid
                                      << " data:" << data
                                      << endl;
                    }
                    fh_xmlbc newchild = this->ensureCreated( rdn, node );
                }
            }
        virtual void OnOutOfProcContextDeletionNotification( const std::string& rdn )
            {
                LG_JOURNAL_D << "OnOutOfProcContextDeletionNotification() this:" << getURL()
                             << " rdn:" << rdn << endl;
                if( this->isSubContextBound( rdn ))
                {
                    fh_context sub = this->getSubContext( rdn );
                    this->Remove( sub, true );
                }
            }
        virtual void OnOutOfProcEACreationNotification( const std::string& eaname,
                                                        bool isDataValid,
                                                        const std::string& data )
            {
                LG_JOURNAL_D << "OnOutOfProcEACreationNotification() ea:" << eaname << endl;
                LG_JOURNAL_D << "OnOutOfProcEACreationNotification(FIXME code it!) ea:" << eaname << endl;

                if( !isAttributeBound( eaname ) )
                {
                    string namespaceURI = "";
                    
                    DOMElement* e = getElement();
                    ::Ferris::setAttributeNS( e, namespaceURI, eaname, data );
                    addNewAttribute( namespaceURI, eaname, XSD_BASIC_STRING );
                }
            }
        virtual void OnOutOfProcEADeletionNotification( const std::string& eaname )
            {
                DOMElement* e = getElement();
                e->removeAttribute( X( eaname.c_str() ));
                unsetAttribute( eaname );
            }

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        Context* priv_CreateContext( Context* parent, string rdn );

        typedef Loki::SmartPtr< XercesDOMParser,
                                Loki::DestructiveCopy,
                                Loki::DisallowConversion,
                                Loki::AssertCheck,
                                Loki::DefaultSPStorage > parser_t;
        parser_t parser;
    
        typedef Loki::SmartPtr< DOMTreeErrorReporter,
                                Loki::DestructiveCopy,
                                Loki::DisallowConversion,
                                Loki::AssertCheck,
                                Loki::DefaultSPStorage > ErrorHandler_t;
        ErrorHandler_t ErrorHandler;
    

        typedef Loki::SmartPtr< XMLFormatter,
                                Loki::DestructiveCopy,
                                Loki::DisallowConversion,
                                Loki::AssertCheck,
                                Loki::DefaultSPStorage > gFormatter_t;
        gFormatter_t gFormatter;
    
    
        XMLContext* getBaseContext()
            {
                return dynamic_cast<_Self*>( _Base::getBaseContext() );
            }
        
        /*******************************************************************************/
        /*******************************************************************************/
        std::string StaticStringData;
        void setStaticString( const std::string& s )
            {
                StaticStringData = s;
            }
        fh_domdoc m_staticDOM;
        void setStaticDOM( fh_domdoc d )
            {
                m_staticDOM = d;
            }
        

        //
        // FIXME: We really only need this string in the base context of each XML document
        //
        std::string m_ForcedURLPrefix;
        void setForcedURLPrefix( const std::string& s )
            {
                m_ForcedURLPrefix = s;
            }
        virtual std::string getURL()
            {
                XMLContext* bc = getBaseContext();
                
                if( bc->m_ForcedURLPrefix.empty() )
                    return _Base::getURL();
                fh_stringstream ss;
                ss << bc->m_ForcedURLPrefix << getDirPath();
                return tostr(ss);
            }
        
        /*******************************************************************************/
        /*******************************************************************************/

        /**
         * Read in the XML file, create a DOM for it and hand that DOM
         * to WrapDOMTree()
         */
        void readXML( RootContextFactory* rf ) throw( RootContextCreationFailed );

    protected:

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Methods that subclasses must override for the module to work ***************/
        /********************************************************************************/

        virtual void priv_syncTree( bool force = false )
            {
                fh_stringstream ss = tostream( getDocument() );
                LG_XML_D << "syncContext() fn:" << xmlFileName() << " data:" << tostr(ss) << endl;

                string fn = xmlFileName();
                string nn = fn + "~";
                
                {
                    XMLContext* bc = getBaseContext();
                    if( bc && bc->isParentBound() )
                    {
                        Context* p = bc->getParent();
                        if( p->getIsNativeContext() )
                        {
                            ::rename( fn.c_str(), nn.c_str() );

                            LG_XML_D << "syncContext(native) fn:" << xmlFileName() << " data:" << tostr(ss) << endl;
                            fh_ofstream oss( fn );
                            copy( istreambuf_iterator<char>(ss),
                                  istreambuf_iterator<char>(),
                                  ostreambuf_iterator<char>(oss));
                            oss << flush;
                            return;
                        }
                    }
                    
                }

                static bool priv_syncTreeCounter = false;

                if( priv_syncTreeCounter )
                {
                    fh_context c = getBaseContext();
                    LG_XML_D << "syncContext(already-syncing) url:" << c->getURL() << " data:" << tostr(ss) << endl;
                    return;
                }
                Util::ValueRestorer< bool > _obj( priv_syncTreeCounter, true );

                fh_context c = getBaseContext();

                LG_XML_D << "syncContext(c):" << GetImpl(c) << endl;
                {
                    fh_stringstream ss;
                    c->dumpRefDebugData( ss );
                    LG_XML_D << tostr(ss) << endl;
                }
                
                string data = StreamToString( ss );
                c = getBaseContext()->getCoveredContext();
                LG_XML_D << "syncContext(non-native1) url:" << c->getURL() << " data:" << tostr(ss) << endl;
                {
                    fh_iostream oss = c->getIOStream( ios::out | ios::trunc );
                    oss << data << flush;
                }
                // This is needed for xsltfs:// but crashes normal xml overmounting.
                if( starts_with( getURL(), "xsltfs" ) )
                {
                    if( getBaseContext()->getCoveredContext() != getBaseContext() )
                    {
                        c = getBaseContext();
                        LG_XML_D << "syncContext(non-native2) url:" << c->getURL() << " data:" << tostr(ss) << endl;
                        fh_iostream oss = c->getIOStream( ios::out | ios::trunc );
                        oss << data << flush;
                    }
                }
            }
        
        virtual fh_xmlbc ensureCreated( const std::string& rdn, DOMElement* e );

    
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        virtual void createStateLessAttributes( bool force = false );

    public:

        XMLContext( Context* parent, const std::string& rdn );
        ~XMLContext();

    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//     ostream& operator<< (ostream& target, const DOMString& s)
//     {
//         char *p = s.transcode();
//         target << p;
//         delete [] p;
//         return target;
//     }


//     string tostr( const DOMString& ds )
//     {
//         ostringstream ss;
//         ss << ds;
//         return tostr(ss);
//     }




class FERRISEXP_DLLLOCAL DOMTreeErrorReporter : public ErrorHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DOMTreeErrorReporter() :
       fSawErrors(false)
    {
    }

    ~DOMTreeErrorReporter()
    {
    }

    // -----------------------------------------------------------------------
    //  Implementation of the error handler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawErrors() const;

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSawErrors
    //      This is set if we get any errors, and is queryable via a getter
    //      method. Its used by the main code to suppress output if there are
    //      errors.
    // -----------------------------------------------------------------------
    bool    fSawErrors;
};

inline bool DOMTreeErrorReporter::getSawErrors() const
{
    return fSawErrors;
}


void DOMTreeErrorReporter::warning(const SAXParseException&)
{
    //
    // Ignore all warnings.
    //
}

void DOMTreeErrorReporter::error(const SAXParseException& toCatch)
{
    fSawErrors = true;
    LG_XML_ER << "Error at file \"" << tostr(toCatch.getSystemId())
              << "\", line " << toCatch.getLineNumber()
              << ", column " << toCatch.getColumnNumber()
              << "\n   Message: " << tostr(toCatch.getMessage()) << endl;
}

void DOMTreeErrorReporter::fatalError(const SAXParseException& toCatch)
{
    fSawErrors = true;
    LG_XML_ER << "Fatal Error at file \"" << tostr(toCatch.getSystemId())
              << "\", line " << toCatch.getLineNumber()
              << ", column " << toCatch.getColumnNumber()
              << "\n   Message: " << tostr(toCatch.getMessage()) << endl;
}

void DOMTreeErrorReporter::resetErrors()
{
    // No-op in this case
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



class FERRISEXP_DLLLOCAL DOMPrintFormatTarget : public XMLFormatTarget
{
public:
    DOMPrintFormatTarget()  {};
    ~DOMPrintFormatTarget() {};

    // -----------------------------------------------------------------------
    //  Implementations of the format target interface
    // -----------------------------------------------------------------------

    void writeChars( const   XMLByte* const  toWrite,
                     XMLSize_t count,
                     XMLFormatter * const formatter)
    {
        // Surprisingly, Solaris was the only platform on which
        // required the char* cast to print out the string correctly.
        // Without the cast, it was printing the pointer value in hex.
        // Quite annoying, considering every other platform printed
        // the string with the explicit cast to char* below.
        cout.write((char *) toWrite, (int) count);
    };

private:
    // -----------------------------------------------------------------------
    //  Unimplemented methods.
    // -----------------------------------------------------------------------
    DOMPrintFormatTarget(const DOMPrintFormatTarget& other);
    void operator=(const DOMPrintFormatTarget& rhs);
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


Context*
XMLContext::priv_CreateContext( Context* parent, string rdn )
{
    XMLContext* ret = new XMLContext( parent, rdn );
//    ret->setContext( parent, rdn );
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


XMLContext::XMLContext( Context* parent, const std::string& rdn )
    :
    _Base( parent, rdn )
{
    createStateLessAttributes();    
}

XMLContext::~XMLContext()
{
}

void
XMLContext::createStateLessAttributes( bool force )
{
    static Util::SingleShot virgin;
    if( virgin() )
    {
        _Base::createStateLessAttributes( true );
        supplementStateLessAttributes( true );
    }
}


void
XMLContext::readXML( RootContextFactory* rf )
    throw( RootContextCreationFailed )
{
    fh_domdoc doc = 0;
    XMLFormatter::UnRepFlags gUnRepFlags = XMLFormatter::UnRep_CharRef;
    
    if( m_staticDOM )
    {
        LG_XML_D << "XMLContext::readXML(static)" << endl;
        doc = m_staticDOM;
        setDocument( doc );
    }
    else
    {
        LG_XML_D << "XMLContext::readXML(dynamic)" << endl;
    
        parser = Sink( parser, new XercesDOMParser );
        parser->setValidationScheme( XercesDOMParser::Val_Never );
//        parser->setDoNamespaces( 0 );
        parser->setDoNamespaces( 1 );
        parser->setDoSchema( 0 );

        ErrorHandler = Sink( ErrorHandler, new DOMTreeErrorReporter() );
        parser->setErrorHandler( GetImpl(ErrorHandler) );
        parser->setCreateEntityReferenceNodes( 0 );
//    parser->setToCreateXMLDeclTypeNode( true );

        try
        {
            fh_stringstream datass;

        
            if( StaticStringData.length() )
            {
                LG_XML_D << "Loading memory string a URL src for url:" << getURL() << endl;
                LG_XML_D << "StaticStringData:" << StaticStringData << endl;
            
                int xl = StaticStringData.length();
                // static_cast<>
                const XMLByte* xb = (const XMLByte *)(StaticStringData.data());
            
                MemBufInputSource xmlsrc( xb, xl, getDirPath().c_str());
                parser->parse( xmlsrc );
            }
            else
            {
                LG_XML_D << "Loading XML from baseovermount context for url:" << getURL() << endl;
                auto_ptr<InputSource> isrc( Factory::makeInputSource( rf->getBaseOverMountContext() ));
                parser->parse( *isrc );
            
//             string srcURL = rf->getBaseOverMountContext()->getURL();
//             LG_XML_D << " srcURL:" << srcURL << endl;
//             FerrisURL fu = FerrisURL::fromString( srcURL );
//             string sc = fu.getScheme();

//             if( sc == "file" || sc == "http" || sc == "ftp" )
//             {
//                 LG_XML_D << "Loading from a URL src for "
//                          << " path:" << getDirPath()
//                          << " url:" << getURL() << endl;
//                 URLInputSource xmlsrc( srcURL.c_str() );//getDirPath().c_str() );
//                 parser->parse( xmlsrc );
//             }
//             else
//             {
//                 LG_XML_D << "Loading from a ferris stream for strange "
//                          << " URL:" << getURL() << endl;

//                 fh_istream iss = rf->getBaseOverMountContext()->getIStream();
//                 string s = StreamToString( iss );
                
//                 int xl = s.length();
//                 // static_cast<>
//                 const XMLByte* xb = (const XMLByte *)(s.data());

//                 MemBufInputSource xmlsrc( xb, xl, getDirPath().c_str());
//                 parser->parse( xmlsrc );
//             }
            }
        
            if( parser->getErrorCount() || ErrorHandler->getSawErrors())
            {
                ostringstream ss;
                ss << "An error occured during parsing";
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
        }
        catch (const XMLException& e)
        {
            ostringstream ss;
            ss << "An error occured during parsing\n   Message: "
               << tostr(e.getMessage()) << endl;
            Throw_RootContextCreationFailed( tostr(ss), 0 );
        }
        catch (const DOMException& e)
        {
            ostringstream ss;
            ss << "A DOM error occured during parsing\n   DOMException code: "
               << e.code << endl;
            Throw_RootContextCreationFailed( tostr(ss), 0 );
        }
        catch (...)
        {
            ostringstream ss;
            ss << "An error occured during parsing\n " << endl;
            Throw_RootContextCreationFailed( tostr(ss), 0 );
        }

        setDocument( parser->adoptDocument() );
        doc = getDocument();
        
//         DOMDocument* parserOwnedDoc = parser->getDocument();
//         fh_domdoc theDoc = (DOMDocument *)parserOwnedDoc->cloneNode(parserOwnedDoc);
//         setDocument( theDoc );
//         doc = getDocument();
    }
    
//    FerrisHandle<DOMPrintFormatTarget> formatTarget = new DOMPrintFormatTarget();
    typedef Loki::SmartPtr< DOMPrintFormatTarget,
              Loki::DestructiveCopy,
              Loki::DisallowConversion,
              Loki::AssertCheck,
              Loki::DefaultSPStorage > DOMPrintFormatTarget_t;
    DOMPrintFormatTarget_t formatTarget(new DOMPrintFormatTarget());
    
    
    string encNameStr("UTF-8");
//     DOMNode* aNode = doc.getFirstChild();
//     if (aNode.getNodeType() == DOMNode::XML_DECL_NODE)
//     {
//         DOMString aStr = ((DOM_XMLDecl &)aNode).getEncoding();
//         if (aStr != "")
//         {
//             encNameStr = aStr;
//         }
//     }

    string Encoding = encNameStr;
    unsigned int lent = Encoding.length();

    XMLCh* gEncodingName = new XMLCh[lent + 1];
    XMLString::transcode( Encoding.c_str(), gEncodingName, lent );
    gEncodingName[ lent ] = 0;

    try
    {
        gFormatter = Sink( gFormatter, new XMLFormatter(gEncodingName, GetImpl(formatTarget),
                                                        XMLFormatter::NoEscapes, gUnRepFlags));

        // Using watchtree is a mistake here, when a new file is created the whole tree is
        // written to disk and a bunch of events are fired. It is much cleaner to just signal
        // for only the new element/attribute that was created
//        Factory::getPluginOutOfProcNotificationEngine().watchTree( this );
        if( shouldPerformFullJournaling() )
            Factory::getPluginOutOfProcNotificationEngine().ensureServerRunning();
        
        //LG_XML_D << doc << endl;
        WrapDOMTree( GetImpl(doc), this, "", rf );
    }
    catch (XMLException& e)
    {
        delete [] gEncodingName;

        ostringstream ss;
        ss << "An error occurred during creation of output transcoder. Msg is:"
             << endl
             << tostr(e.getMessage()) << endl;
        Throw_RootContextCreationFailed( tostr(ss), 0 );
    }

    delete [] gEncodingName;
}



fh_xmlbc
XMLContext::ensureCreated( const string& rdn, DOMElement* e )
{
    LG_XML_D << "============ XMLContext::ensureCreated() =========== rdn:" << rdn << endl;

    Util::ValueBumpDrop<ref_count_t> dummy( ref_count );

    if( !isSubContextBound( rdn ) )
    {
        XMLContext* ret = (XMLContext*)CreateContext( this, monsterName( rdn ));
        ret->setElement( (DOMElement*)e );
        
        LG_XML_D << "XMLContext::ensureCreated() emit for rdn:" << rdn << endl;
        Insert( ret, true );
        return ret;
    }
    return (XMLContext*)GetImpl(getSubContext( rdn ));
}







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static void StaticInit()
{
    Factory::ensureXMLPlatformInitialized();
    static bool xml4c_has_been_statically_inited = false;
    
    if( !xml4c_has_been_statically_inited )
    {
        xml4c_has_been_statically_inited = true;
                
    }
}


    fh_domdoc stringToDomPointer( const std::string& s )
    {
//        fh_domdoc ret = 0;
//        cerr << "stringToDomPointer() s:" << s << endl;
//         void* ptr = toType< void* >( s );
//         ret = static_cast<DOMDocument*>( ptr );


        fh_domdoc ret = 0;
        void* ptr = toType< void* >( s );
        fh_domdoc* dptr = static_cast<fh_domdoc*>( ptr );
        ret = *dptr;
        

        
//        cerr << "getting dom.ptr:" << (void*)GetImpl(ret) << " dom.str:" << s << endl;
        
        return ret;
    }
    
    extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        StaticInit();
        static XMLContext raw_obj(0,"/");
#ifdef FERRIS_DEBUG_VM
        cerr << "XMLContext::raw_obj:" << toVoid( &raw_obj ) << endl;
#endif
//        cerr << "XMLContext::raw_obj:" << toVoid( &raw_obj ) << endl;

        LG_XML_D << "libxml.brew() root:" << rf->getInfo( "Root" ) << endl;
        
        XMLContext* rctx = (XMLContext*)raw_obj.CreateContext( 0, rf->getInfo( "Root" ));
        if( !rf->getInfo( "StaticString" ).empty() || !rf->getInfo( "StaticDOM" ).empty() )
        {
            LG_XML_D << "have static string:" << rf->getInfo( "StaticString" ) << endl;
            LG_XML_D << "have static dom ptr string:" << rf->getInfo( "StaticDOM" ) << endl;
            LG_XML_D << "rctx:" << toVoid( rctx ) << endl;
            rctx->setStaticString( rf->getInfo( "StaticString" ) );
            rctx->setStaticDOM( stringToDomPointer(rf->getInfo( "StaticDOM" )) );
            rctx->setForcedURLPrefix( rf->getInfo( "forcedURLPrefix" ) );
//            cerr << "Forced URL:" << rf->getInfo( "forcedURLPrefix" ) << endl;
            rctx->readXML( rf );
        }
        else
        {
            rctx->readXML( rf );
        }
//        cerr << "XMLContext::brew(returning)" << endl;
        return rctx;
    }
}
 
};
