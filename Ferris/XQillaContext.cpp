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

    $Id: XQillaContext.cpp,v 1.3 2010/09/24 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <XQillaContext_private.hh>

#include <Resolver_private.hh>
#include <ValueRestorer.hh>
#include <FerrisDOM.hh>
#include <FerrisDOM_private.hh>
#include <xercesc/util/XMLString.hpp>

// #include <pathan/PathanPlatformUtils.hpp>
// #include <pathan/PathanNSResolver.hpp>
// #include <pathan/PathanException.hpp>
// #include <pathan/PathanExpression.hpp>
// #include <pathan/PathanImplementation.hpp>
// #include <pathan/XPath2Result.hpp> 
// #include <pathan/StaticContext.hpp>
// #include <pathan/DynamicContext.hpp>

// #include <include/pathan/internal/dom-extensions/PathanExpressionImpl.hpp>

#define X(str) XStr(str).unicodeForm()

using namespace std;
namespace Ferris
{
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
//     XPath2MemoryManager* init_pathan()
//     {
//         static XPath2MemoryManager* mm = 0;
//         if( !mm )
//         {
//             PathanPlatformUtils::initialize();
//             DOMImplementation* pathanImplementation = 
//                 DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

//             mm = PathanEngine::createMemoryManager();
//         }
//         return mm;
//     }
    
    
    fh_context
    XPathRootContext::performQuery( const std::string& qs_const )
    {
        string qs = qs_const;
//         string qs = "/fakeroot/child::*";
        qs = string("/*");
        qs += qs_const;
        fh_context ret = 0;
//        qs = "/*/*";
        
        LG_XML_D << "qsc:" << qs_const << endl;
        LG_XML_D << "qs :" << qs << endl;
        
        // Initialise Xerces-C and XQilla using XQillaPlatformUtils
        static Util::SingleShot xqilla_virgin;
        if( xqilla_virgin() )
        {
            XQillaPlatformUtils::initialize();
        }
        Factory::ensureXMLPlatformInitialized();

        static DOMImplementation *xqillaImplementation = 0;
        static DOMDocument* tdoc = 0;

        if( !xqillaImplementation )
            xqillaImplementation = DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
        if( !tdoc )
            tdoc =  xqillaImplementation->createDocument();

        
//        XMLCh* qsUnicode = XMLString::transcode( qs.c_str() );
//        XPath2MemoryManager* mm = init_pathan();

        fh_context root = Resolve("root://");
        fh_domdoc   doc = Factory::makeDOM( root );

        LG_XML_D << "XPathRootContext::performQuery(1) making ns resolver" << endl;
        const DOMXPathNSResolver* nsResolver = tdoc->createNSResolver(tdoc->getDocumentElement());
        
        LG_XML_D << "XPathRootContext::performQuery(2) made ns resolver" << endl;
        XQillaNSResolver* xqillaResolver = (XQillaNSResolver*)nsResolver;

        const DOMXPathExpression* parsedExpression = NULL;
		XPath2Result *result = NULL;

        LG_XML_D << "XPathRootContext::performQuery() root:" << root->getURL()
                 << " qs:" << qs
                 << " qs_const:" << qs_const
                 << endl;

        try
        {
            LG_XML_D << "XPathRootContext::performQuery(3) getting pathan expr impl" << endl;

            parsedExpression = tdoc->createExpression(X( qs.c_str() ), nsResolver );

            LG_XML_D << "XPathRootContext::performQuery(4) evaluating expression" << endl;

            result = (XPath2Result*)parsedExpression->evaluate( doc->getDocumentElement(),
                                                                DOMXPathResult::SNAPSHOT_RESULT_TYPE, 0);

            LG_XML_D << "XPathRootContext::performQuery(5) evaluating success!" << endl;

            
//             LG_XML_D << "XPathRootContext::performQuery(3) getting pathan expr impl" << endl;
// //            parsedExpression = doc->createExpression( qsUnicode, nsResolver );
//             XMLGrammarPool *_xmlGrammarPool = 0;
// //            parsedExpression = doc->createExpression(X(expression), nsResolver );
            
//             parsedExpression = new (mm) PathanExpressionImpl( qsUnicode, GetImpl(doc),
//                                                               mm, nsResolver, _xmlGrammarPool);

//             LG_XML_D << "XPathRootContext::performQuery(4) evaluating expression" << endl;
//             result = (XPath2Result*)parsedExpression->evaluate( doc->getDocumentElement(),
// //                                                                XPath2Result::FIRST_RESULT, 0 );
//                                                                 XPath2Result::SNAPSHOT_RESULT, 0 );
// //                                                                XPath2Result::ITERATOR_RESULT, 0 );
//             LG_XML_D << "XPathRootContext::performQuery(5) evaluating success!" << endl;
        }
		catch( const XQException &e ) {
//            delete nsResolver;
            delete parsedExpression;
            delete result;
//            XMLString::release( &qsUnicode );
            fh_stringstream ss;
            ss << "NoSuchSubContext XPath error for query:" << qs
               << " error:" << tostr(e.getError()) << endl;
            LG_XML_D << tostr(ss) << endl;
            Throw_NoSuchSubContext( tostr(ss), this );
		}
        catch (const XMLException& e) {
//            delete nsResolver;
            delete parsedExpression;
            delete result;
//            XMLString::release( &qsUnicode );
            fh_stringstream ss;
            ss << "NoSuchSubContext XML error for query:" << qs
               << " error:" << e.getMessage();
            LG_XML_D << tostr(ss) << endl;
            Throw_NoSuchSubContext( tostr(ss), this );
		}
        catch(...)
        {
//            delete nsResolver;
            delete parsedExpression;
//            XMLString::release( &qsUnicode );
            fh_stringstream ss;
            ss << "NoSuchSubContext XPath error for query:" << qs;
            LG_XML_D << tostr(ss) << endl;
            Throw_NoSuchSubContext( tostr(ss), this );
        }

        LG_XML_D << "XPathRootContext::performQuery(6) getting context list" << endl;
        //
        // We should add a new proxy Context class for each node that is in the
        // result set. Since we know that the XML Node will just be a wrapper
        // over the underlying filesystem object we can get its path and
        // then resolve() the path to get the underlying Context object.
        //
        fh_contextlist clist = Factory::MakeContextList();
        LG_XML_D << "XPathRootContext::performQuery(6) result:" << result << endl;
        LG_XML_D << "result->getSnapshotLength():" << result->getSnapshotLength() << endl;
        
        for(unsigned int i=0; i<result->getSnapshotLength(); i++)
        {
            result->snapshotItem( i );
//            DOMNode *node = result->snapshotItem( i );

            LG_XML_D << "XPathRootContext::performQuery(loop) index:" << i << endl;
            if( result->isNode() )
            {
                const DOMNode* node = result->getNodeValue();
                LG_XML_D << "XPathRootContext::performQuery(loop) node:" << node << endl;
                
                const XMLCh* path = node->getNodeName();
                LG_XML_D << "XPathRootContext::performQuery(loop) path:" << tostr(path) << endl;
//                delete[] (XMLCh *)path;

                if( const Ferris_DOMElement* e = dynamic_cast<const Ferris_DOMElement*>( node ))
                {
                    fh_context c = e->getContext();
                    LG_XML_D << "dom element... c:" << c->getURL() << endl;
//                    cerr << "dom element... c:" << c->getURL() << endl;
                    clist->insert( c );
                }
            }

//            result->iterateNext();
        }
        ret = clist;
        LG_XML_D << "XPathRootContext::performQuery(loop) done."
                 << " ret.size:" << ret->getSubContextCount() << endl;
        
//        delete nsResolver;
//        delete parsedExpression;
//        XMLString::release( &qsUnicode );
        LG_XML_D << "XPathRootContext::performQuery() returning" << endl;
        return ret;
    }
    

    //
    // Treat the attempt to view a context as the initiation of a query
    // if the dir isn't already there then call performQuery() to make it
    // Short cut loading each dir unless absolutely needed.
    //
    fh_context
    XPathRootContext::getSubContext( const std::string& rdn )
        throw( NoSuchSubContext )
    {
        try
        {
            if( priv_isSubContextBound( rdn ) )
            {
                return _Base::priv_getSubContext( rdn );
            }

            if( rdn.empty() )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext no rdn given";
                Throw_NoSuchSubContext( tostr(ss), this );
            }

            if( rdn == "//" )
                return this;

            if( m_querying )
            {
                if( rdn == "//" )
                    return this;
                
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
            
            Util::ValueRestorer< bool > _obj( m_querying, true );
            fh_context qc = performQuery( rdn );
            if( !isBound( qc ) )
            {
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
            
            return qc;
        }
        catch( NoSuchSubContext& e )
        {
            LG_XML_D << "XXX NoSuchSubContext e"  << endl;
            throw e;
        }
		catch( const XQException &e )
        {
            LG_XML_D << "XXX NoSuchSubContext pe:" << tostr(e.getError()) << endl;
        }
        catch (const DOMException& e)
        {
            LG_XML_D << "XXX NoSuchSubContext de:" << tostr(e.getMessage())  << endl;
        }
        catch( exception& e )
        {
            string s = e.what();
            LG_XML_D << "XXX NoSuchSubContext s:" << s << endl;
            Throw_NoSuchSubContext( s, this );
        }
        catch(...)
        {
            LG_XML_D << "XXX NoSuchSubContext e ..."  << endl;
        }
        
        fh_stringstream ss;
        ss << "NoSuchSubContext:" << rdn;
        Throw_NoSuchSubContext( tostr(ss), this );
    }
    
    void
    XPathRootContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }
    
    XPathRootContext::XPathRootContext()
        :
        _Base( 0, "/" ),
        m_querying( 0 )
    {
        createStateLessAttributes();
    }
    
    XPathRootContext::~XPathRootContext()
    {
    }
    
    void
    XPathRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }


    class FERRISEXP_DLLLOCAL XPathRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        XPathRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("xpath");
                RootContextFactory::Register( "xpath", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new XPathRootContext();
                }
                return c;
            }
    };
    static XPathRootContext_RootContextDropper ___XPathRootContext_static_init;

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
