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

    $Id: xslt_string.cpp,v 1.3 2008/05/19 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "xslt_base.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/Shell.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FerrisXalan_private.hh>

#include <algorithm>

using namespace Ferris;
using namespace std;

namespace FerrisXSLT 
{
    
    /**
     * Like substring-after but we return the substring after the *last* occurance of
     * the pattern.
     */
    class SubStringAfterLastFunction
        :
        public FerrisXSLTFunctionBase< SubStringAfterLastFunction >
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

                    string str = Ferris::tostr( args[0]->str() );
                    string pat = Ferris::tostr( args[1]->str() );

                    int pos = str.rfind( pat );
                    if( pos != string::npos )
                        ++pos;
                    
                    cerr << "substring-after-last str:" << str << " pat:" << pat
                         << " res:" << str.substr( pos ) << endl;
                    
                    return xfac.createString( domstr( str.substr( pos ) ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr( e.what() ), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "substring-after-last requires: string str to operate on, string pattern" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SubStringAfterLastFunction();
            }
        
    };

    // Register the function. 
    namespace { static bool v1 = registerXSLTFunction( "substring-after-last", SubStringAfterLastFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Like substring-before but we return the substring before the *last* occurance of
     * the pattern.
     */
    class SubStringBeforeLastFunction
        :
        public FerrisXSLTFunctionBase< SubStringBeforeLastFunction >
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

                    string str = Ferris::tostr( args[0]->str() );
                    string pat = Ferris::tostr( args[1]->str() );

                    int pos = str.rfind( pat );
                    
                    cerr << "substring-before-last str:" << str << " pat:" << pat
                         << " res:" << str.substr( 0, pos ) << endl;
                    
                    return xfac.createString( domstr( str.substr( 0, pos ) ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr( e.what() ), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "substring-before-last requires: string str to operate on, string pattern" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new SubStringBeforeLastFunction();
            }
    };

    // Register the function. 
    namespace { static bool v2 = registerXSLTFunction( "substring-before-last", SubStringBeforeLastFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /**
     * Like substring-before but we return the substring before the *last* occurance of
     * the pattern.
     */
    class StringReplaceCharFunction
        :
        public FerrisXSLTFunctionBase< StringReplaceCharFunction >
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
                    if (args.size() != 3)
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    assert( !args[0].null() );
                    assert( !args[1].null() );
                    assert( !args[2].null() );

                    string str  = Ferris::tostr( args[0]->str() );
                    string oldv = Ferris::tostr( args[1]->str() );
                    string newv = Ferris::tostr( args[2]->str() );

                    if( oldv.empty() || newv.empty() )
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }
                    
                    replace_if( str.begin(), str.end(),
                                bind2nd( equal_to<char>(), oldv[0] ),
                                newv[0] );
                    cerr << "string-replace-char str:" << str
                         << " oldv:" << oldv
                         << " newv:" << newv
                         << endl;
                    return xfac.createString( domstr( str ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr( e.what() ), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "string-replace-char requires: string str to operate on, string to use first char as char to remove, string to use first char is char to replace old char with, returns string with all arg 2 occurances replaced with arg3" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new StringReplaceCharFunction();
            }
    };

    // Register the function. 
    namespace { static bool v3 = registerXSLTFunction( "string-replace-char", StringReplaceCharFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    /**
     * convert a string like hourly to 3600
     */
    class StringTimeIntervalToIntFunction
        :
        public FerrisXSLTFunctionBase< StringTimeIntervalToIntFunction >
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

                    string str = Ferris::tostr( args[0]->str() );
                    string ret = str;

                    if( str == "hourly" )
                        ret = "3600";

                    if( str == "daily" )
                        ret = tostr(3600*24);
                    
                    return xfac.createString( domstr( ret ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr( e.what() ), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "string-time-interval-to-int requires: string time string in the form of syndication time to convert to a string containing the number of seconds in that interval" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new StringTimeIntervalToIntFunction();
            }
    };

    // Register the function. 
    namespace { static bool v4 = registerXSLTFunction( "string-time-interval-to-int",
                                                       StringTimeIntervalToIntFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Yes, I know that having a write/read/incr counter is not good design
     * but for quick and dirty XSLT sheets that break a file into parts this
     * can be helpful instead of forcing recursive template matching in the XSLT.
     */
    static int Counter = 0;
    
    
    /**
     * Read value in Counter
     */
    class GetNextCounterFunction
        :
        public FerrisXSLTFunctionBase< GetNextCounterFunction >
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
                    if (args.size() != 0)
                    {
                        XalanDOMString buf;
                        executionContext.error( getError( buf ), context);
                    }

                    ++Counter;
                    string ret = Ferris::tostr( Counter );
                    cerr << "get-next-counter ret:" << ret << " c:" << Counter << endl;
                    return xfac.createString( domstr( ret ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr( e.what() ), context );
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "get-next-counter requires nothing. returns the string that was stored as the counter or 0 if no write/incr has been performed" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new GetNextCounterFunction();
            }
    };

    // Register the function. 
    namespace { static bool v6 = registerXSLTFunction( "get-next-counter",
                                                       GetNextCounterFunction() ); };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
};

