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

    $Id: xslt_ferris.cpp,v 1.3 2008/05/19 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "xslt_base.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/Shell.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisXalan_private.hh>
#include <Ferris/FerrisOpenSSL.hh>

using namespace Ferris;
using namespace std;

namespace FerrisXSLT 
{
    /**
     */
    class SetStrAttrFunction
        :
        public FerrisXSLTFunctionBase< SetStrAttrFunction >
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
                    if (args.size() < 3 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );
                    std::string v    = Ferris::tostr( args[2]->str() );
                    bool create = false;
                    bool throw_for_errors = true;
                
                    if (args.size() >= 4 )
                        create = args[3]->num() != 0;
                    if (args.size() >= 5 )
                        throw_for_errors = args[4]->num() != 0;

                    fh_context c = Resolve( path );

                    std::string ret = setStrAttr( c,
                                                  rdn,
                                                  v,
                                                  create,
                                                  throw_for_errors );
                
                    return xfac.createString( domstr( ret ) );
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
                theBuffer = domstr("set-attr requires: string path to context, string ea name, string value for ea to be set to, and optionally bool create=false, bool throw_errs=false"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SetStrAttrFunction();
            }
    };

    // Register the function. 
    namespace { static bool v1 = registerXSLTFunction( "set-attr", SetStrAttrFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class GetStrAttrFunction
        :
        public FerrisXSLTFunctionBase< GetStrAttrFunction >
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
                    if (args.size() < 3 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );
                    std::string d    = Ferris::tostr( args[2]->str() );
                    bool getall = false;
                    bool throw_for_errors = true;
                
                    if (args.size() >= 4 )
                        getall = args[3]->num() != 0;
                    if (args.size() >= 5 )
                        throw_for_errors = args[4]->num() != 0;

                    cerr << "get-attr path:" << path << " rdn:" << rdn << " d:" << d << endl;
                
                
                    fh_context c = Resolve( path );

                    std::string ret = getStrAttr( c,
                                                  rdn,
                                                  d,
                                                  getall,
                                                  throw_for_errors );
                
                    return xfac.createString( domstr( ret ) );
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
                theBuffer = domstr("get-attr requires: string path to context, string ea name, string value for default if ea is not present, and optionally bool getall=false to get lines after the first line aswell, bool throw_errs=false"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new GetStrAttrFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v2 = registerXSLTFunction( "get-attr", GetStrAttrFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class SaveFileFunction
        :
        public FerrisXSLTFunctionBase< SaveFileFunction >
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
                    
                    if (args.size() < 3 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context );
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    std::string path = Ferris::tostr( args[0]->str() );
                    std::string rdn  = Ferris::tostr( args[1]->str() );
                    std::string bc   = Ferris::tostr( args[2]->str() );
                    bool shouldMonsterName = false;
                    bool overwrite = true;
                
                    if (args.size() >= 4 )
                        shouldMonsterName = args[3]->num() != 0;
                    if (args.size() >= 5 )
                        overwrite = args[4]->num() != 0;

                    fh_context retc = saveFile( path,
                                                rdn,
                                                bc,
                                                shouldMonsterName,
                                                overwrite );
                
                    return xfac.createString( domstr( retc->getURL() ) );
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
                theBuffer = domstr("save-file requires: string path to parent, string desired rdn name, string value for byte content for file itself and optionally bool shouldMonsterName = true to change rdn if there exists a child with that name already, bool overwrite = false to override any existing file without prompting"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SaveFileFunction();
            }
    };

    // Register the function. 
    namespace { static bool v3 = registerXSLTFunction( "save-file", SaveFileFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     */
    class DigestFunction
        :
        public FerrisXSLTFunctionBase< DigestFunction >
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
                    if (args.size() < 1 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context );
                    }
                    assert( !args[0].null() );

                    string s = Ferris::tostr( args[0]->str() );
                    string DigestName = "md5";
                    if (args.size() >= 2 )
                    DigestName = Ferris::tostr( args[1]->str() );

                    string ret = digest( s, DigestName );
                    
                    return xfac.createString( domstr( ret ) );
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
                theBuffer = domstr("digest requires: string data to digest, optionally the digest name to use. returns the string digest of the first arg."); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new DigestFunction();
            }
    };

    // Register the function. 
    namespace { static bool v4 = registerXSLTFunction( "digest", DigestFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    

    /**
     * Ability to get time(2) from XSLT
     */
    class TimeFunction
        :
        public FerrisXSLTFunctionBase< TimeFunction >
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
                    time_t t = time(0);
                    return xfac.createString( domstr( tostr(t) ) );
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
                theBuffer = domstr("time returns time(2) as a string"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new TimeFunction();
            }
    };

    // Register the function. 
    namespace { static bool v5 = registerXSLTFunction( "time", TimeFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static const int DEFAULT_MAX_RECENT = 2;
    
    /**
     * update the link farm for recent news items in this channel
     */
    class UpdateRecentNewsFunction
        :
        public FerrisXSLTFunctionBase< UpdateRecentNewsFunction >
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
                    if (args.size() < 2 )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context );
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );

                    string path = Ferris::tostr( args[0]->str() );
                    string mrdn = Ferris::tostr( args[1]->str() );

                    cerr << "update-recent-news path:" << path
                         << " mrdn:" << mrdn
                         << endl;
                    
                    fh_context itemc    = Resolve( path + "/" + mrdn );
                    fh_context channelc = itemc->getParent();
                    string recentChannelRdn = channelc->getDirName() + "-recent";

                    cerr << "update-recent-news path:" << path
                         << " mrdn:" << mrdn
                         << " recentChannelRdn:" << recentChannelRdn
                         << endl;
                    
                    fh_context cr = Shell::acquireSubContext( Resolve("news://"),
                                                              recentChannelRdn,
                                                              true );

                    fh_context lc = Shell::CreateLink( itemc,
                                                       cr,
                                                       itemc->getDirName(),
                                                       false );

                    // remove the oldest link
                    string removeRdn = "";
                    {
                        int maxRecent = toint(getStrAttr( cr, "max-recent", tostr( DEFAULT_MAX_RECENT ) ));
                        cr->read();
                        fh_context sortedc = Factory::MakeSortedContext( cr, ":#!:mtime" );
                        cerr << "update-recent-news path:" << path
                             << " maxrec:" << maxRecent
                             << " sub#:" << sortedc->SubContextCount()
                             << endl;
                    
                        if( maxRecent < sortedc->SubContextCount() )
                        {
                            string target = (*sortedc->begin())->getDirName();
                            cerr << "Would be removing c:" << (*sortedc->begin())->getURL() << endl;
//                            cr->remove( target  );
                            cerr << "Have removed c:" << target << endl;
                            removeRdn = target;
                        }
                    }
                    if( !removeRdn.empty() )
                        cr->remove( removeRdn );
                    
                    
                    return xfac.createString( domstr( lc->getURL() ) );
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
                theBuffer = domstr("update-recent-news requires: string path of channel, string rdn of new item. returns the link path"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new UpdateRecentNewsFunction();
            }
    };

    // Register the function. 
    namespace { static bool v6 = registerXSLTFunction( "update-recent-news",
                                                       UpdateRecentNewsFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static int new_news_counter = 0;

    void setNewNewsCounter( int x )
    {
        new_news_counter = x;
    }

    int getNewNewsCounter()
    {
        return new_news_counter;
    }

    /**
     */
    class IncrementNewNewsCounterFunction
        :
        public FerrisXSLTFunctionBase< IncrementNewNewsCounterFunction >
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
                    ++new_news_counter;
                    
                    return xfac.createString( domstr( "" ) );
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
                theBuffer = domstr("increment-new-news-count: increments the news counter for new items and returns an empty string"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new IncrementNewNewsCounterFunction();
            }
    };

    // Register the function. 
    namespace { static bool v7 = registerXSLTFunction( "increment-new-news-count",
                                                       IncrementNewNewsCounterFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    static int existing_news_counter = 0;

    void setExistingNewsCounter( int x )
    {
        existing_news_counter = x;
    }

    int getExistingNewsCounter()
    {
        return existing_news_counter;
    }
    
    /**
     */
    class IncrementExistingNewsCounterFunction
        :
        public FerrisXSLTFunctionBase< IncrementExistingNewsCounterFunction >
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
                    ++existing_news_counter;
                    
                    return xfac.createString( domstr( "" ) );
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
                theBuffer = domstr("increment-existing-news-count: increments the existing news counter for items that were already known and returns an empty string"); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new IncrementExistingNewsCounterFunction();
            }
    };

    // Register the function. 
    namespace { static bool v8 = registerXSLTFunction( "increment-existing-news-count",
                                                       IncrementExistingNewsCounterFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    static string channel_name = "";

    void setChannelName( string s )
    {
        channel_name = s;
    }

    string getChannelName()
    {
        return channel_name;
    }
    
    /**
     */
    class SetChannelNameFunction
        :
        public FerrisXSLTFunctionBase< SetChannelNameFunction >
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
                        executionContext.error( getError( buf ), context );
                    }
                    assert( !args[0].null() );

                    string cn = Ferris::tostr( args[0]->str() );
                    channel_name = cn;
                    return xfac.createString( domstr( cn ) );
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
                theBuffer = domstr("set-channel-name requires: string the channel name. returns the channel name."); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SetChannelNameFunction();
            }
    };

    // Register the function. 
    namespace { static bool v9 = registerXSLTFunction( "set-channel-name",
                                                       SetChannelNameFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    

    
    
};

