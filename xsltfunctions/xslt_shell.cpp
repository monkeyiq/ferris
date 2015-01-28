/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: xslt_shell.cpp,v 1.3 2008/05/19 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "xslt_base.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/Shell.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisXalan_private.hh>

using namespace Ferris;
using namespace std;

namespace FerrisXSLT 
{
    
    /**
     */
    class ShellQuoteFunction
        :
        public FerrisXSLTFunctionBase< ShellQuoteFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                // Use the XObjectFactory createNumber() method to create an XObject 
                // corresponding to the XSLT number data type.
                XObjectFactory& xfac = executionContext.getXObjectFactory();

                try
                {
                    
                    if (args.size() != 1)
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );

                    std::string s = Ferris::tostr( args[0]->str() );
                
                    return xfac.createString( domstr( Shell::quote( s ) ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:

        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("shell-quote requires one string argument");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new ShellQuoteFunction();
            }
    };

    // Register the function. 
    namespace { static bool v1 = registerXSLTFunction( "shell-quote", ShellQuoteFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class GetCWDFunction
        :
        public FerrisXSLTFunctionBase< GetCWDFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    return xfac.createString( domstr( Shell::getCWDString() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("get-cwd requires 0, and ignores any args");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new GetCWDFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v2 = registerXSLTFunction( "get-cwd", GetCWDFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class SetCWDFunction
        :
        public FerrisXSLTFunctionBase< SetCWDFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    
                    if (args.size() != 1)
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );

                    std::string s = Ferris::tostr( args[0]->str() );
                    fh_context c = Resolve( s );
                
                    Ferris::Shell::setCWD( c );
                
                    return xfac.createString( domstr( Shell::getCWDString() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("set-cwd requires 1 string arg, the new path");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SetCWDFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v3 = registerXSLTFunction( "set-cwd", SetCWDFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class CreateFileFunction
        :
        public FerrisXSLTFunctionBase< CreateFileFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();

                try
                {
                    if (args.size() != 2)
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );
                    fh_context c = Resolve( path );
                
                    fh_context newc = Shell::CreateFile( c, rdn );
                
                    return xfac.createString( domstr( newc->getURL() ));
                }
                catch( std::exception& e )
                {
                    return xfac.createString( domstr( e.what() ));
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("create-file requires two args. First the existing path and second the rdn of the new file");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new CreateFileFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v4 = registerXSLTFunction( "create-file", CreateFileFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class CreateDirFunction
        :
        public FerrisXSLTFunctionBase< CreateDirFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    
                    cerr << "create-dir args.sz:" << args.size() << endl;
                
                    if (args.size() < 2 || args.size() > 3 )
                    {
                        cerr << "create-dir wrong args size" << endl;
                    
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    cerr << "create-dir2 args.sz:" << args.size() << endl;
                
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                
                    cerr << "create-dir3 args.sz:" << args.size() << endl;
                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );
                    bool WithParents = false;
                    if (args.size() >= 3 )
                        WithParents = args[0]->num() != 0;

                    cerr << "create-dir4 -p:" << WithParents << endl;
                
                    fh_context c = Resolve( path );
                    fh_context newc = Shell::CreateDir( c, rdn, WithParents );
                
                    return xfac.createString( domstr( newc->getURL() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
                
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("create-dir requires two args. First the existing path and second the rdn of the new dir. Optionally the third arg is equal to the -p for mkdir and is boolean (false by default)");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new CreateDirFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v5 = registerXSLTFunction( "create-dir", CreateDirFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class AcquireSubContextFunction
        :
        public FerrisXSLTFunctionBase< AcquireSubContextFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    
                    if (args.size() != 2 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                
                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );

                    fh_context c    = Resolve( path );
                    fh_context retc = Shell::acquireSubContext( c, rdn );

                    return xfac.createString( domstr( retc->getURL() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("acquire-subcontext requires two args. First the existing path and second the rdn of the new or existing dir. ");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new AcquireSubContextFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v6 = registerXSLTFunction( "acquire-subcontext", AcquireSubContextFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class AcquireContextFunction
        :
        public FerrisXSLTFunctionBase< AcquireContextFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    if (args.size() != 1 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    fh_context retc  = Shell::acquireContext( path );
                    return xfac.createString( domstr( retc->getURL() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("acquire-context requires one arg, the path to create or resolve");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new AcquireContextFunction();
            }
        
        
    };

    // Register the function. 
    namespace { static bool v7 = registerXSLTFunction( "acquire-context", AcquireContextFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class getHomeDirPathFunction
        :
        public FerrisXSLTFunctionBase< getHomeDirPathFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    return xfac.createString( domstr( Shell::getHomeDirPath() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("get-homedir requires 0 args");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new getHomeDirPathFunction();
            }
    };

    // Register the function. 
    namespace { static bool v8 = registerXSLTFunction( "get-homedir", getHomeDirPathFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class getTmpDirPathFunction
        :
        public FerrisXSLTFunctionBase< getTmpDirPathFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    return xfac.createString( domstr( Shell::getTmpDirPath() ));
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("get-tmpdir requires 0 args");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new getTmpDirPathFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v9 = registerXSLTFunction( "get-tmpdir", getTmpDirPathFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class contextExistsFunction
        :
        public FerrisXSLTFunctionBase< contextExistsFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    
                    if (args.size() != 1 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    bool v = Shell::contextExists( path );
                    return xfac.createBoolean( v );
                }
                catch( std::exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
                
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("context-exists requires the path to test as its only arg");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new contextExistsFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v10 = registerXSLTFunction( "context-exists", contextExistsFunction() ); };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Check if there is a context in a given dir with ea matching a given value
     *
     */
    class ContextExistsWithEAFunction
        :
        public FerrisXSLTFunctionBase< ContextExistsWithEAFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    if (args.size() != 3 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    string path   = Ferris::tostr( args[0]->str() );
                    string eaname = Ferris::tostr( args[1]->str() );
                    string val    = Ferris::tostr( args[2]->str() );
                    string notval = "";

                    if( val == "" )
                        notval = "x";
                    
                    fh_context c = Resolve( path );
                    for( Context::iterator iter = c->begin(); iter != c->end(); ++iter )
                    {
                        if( getStrAttr( *iter, eaname, notval ) == val )
                        {
                            return xfac.createString( domstr( iter->getURL() ));
                        }
                    }
                    return xfac.createString( domstr( "" ));
                }
                catch( exception& e )
                {
                        executionContext.error( domstr(e.what()), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("context-exists-with-ea requires: string the path for parent of sought context, string ea name to search, string value to match. returns the URL of matching context or empty string");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new ContextExistsWithEAFunction();
            }
    };

    // Register the function. 
    namespace { static bool v11 = registerXSLTFunction( "context-exists-with-ea", ContextExistsWithEAFunction() ); };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Check if there is a context in a given dir with ea matching a given value
     *
     */
    class LookupNewsByGUIDFunction
        :
        public FerrisXSLTFunctionBase< LookupNewsByGUIDFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    if (args.size() != 3 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    string path   = Ferris::tostr( args[0]->str() );
                    string guid   = Ferris::tostr( args[1]->str() );
                    string digest = Ferris::tostr( args[2]->str() );
                    
                    fh_context c = Resolve( path );

                    cerr << "lookup-news-by-digest md5:" << digest
                         << " guid:" << guid
                         << " path:" << path
                         << " bound:" << c->isSubContextBound( digest )
                         << endl;
                    
                    if( c->isSubContextBound( digest ) )
                    {
                        fh_context childc = c->getSubContext( digest );
                        if( getStrAttr( childc, "guid", "" ) == guid )
                        {
                            return xfac.createString( domstr( childc->getURL() ));
                        }
                    }
                    
                    for( int i=0; ; ++i )
                    {
                        fh_stringstream ss;
                        
                        ss << digest << "--" << i;
                        if( c->isSubContextBound( tostr(ss) ) )
                        {
                            fh_context childc = c->getSubContext( tostr(ss) );
                            if( getStrAttr( childc, "guid", "" ) == guid )
                            {
                                return xfac.createString( domstr( childc->getURL() ));
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    
                    return xfac.createString( domstr( "" ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr(e.what()), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("lookup-news-by-digest requires: string the path for parent of sought context, string guid to find, string digest of guid to find. returns the URL of matching context or empty string");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new LookupNewsByGUIDFunction();
            }
    };

    // Register the function. 
    namespace { static bool v12 = registerXSLTFunction( "lookup-news-by-digest", LookupNewsByGUIDFunction() ); };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Check if there is a context in a given dir with ea matching a given value
     *
     */
    class MonsterNameFunction
        :
        public FerrisXSLTFunctionBase< MonsterNameFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    if (args.size() != 2 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                
                    assert( !args[0].null() );
                    assert( !args[1].null() );

                    string path  = Ferris::tostr( args[0]->str() );
                    string rdn   = Ferris::tostr( args[1]->str() );

                    fh_context c = Resolve( path );
                    string ret = monsterName( c, rdn );
                    
                    return xfac.createString( domstr( ret ));
                }
                catch( exception& e )
                {
                        executionContext.error( domstr(e.what()), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = XalanDOMString("monster-rdn requires: string the path for parent of sought context, string rdn desired rdn which might be monstered. return is rdn or rdn--nn ");
                return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new MonsterNameFunction();
            }
    };

    // Register the function. 
    namespace { static bool v13 = registerXSLTFunction( "monster-rdn", MonsterNameFunction() ); };
    
    
    
};

