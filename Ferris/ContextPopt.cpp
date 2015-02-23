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

    $Id: ContextPopt.cpp,v 1.2 2010/09/24 21:30:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <ContextPopt.hh>
#include <Context.hh>

using namespace std;

namespace Ferris
{

    namespace Private
    {

        static string trimdots( const string& s, int maxl=10, const string& dotss = "..." )
        {
            if( s.length() > maxl )
            {
                string ret = s.substr(0,maxl);
                ret += dotss;
                return ret;
            }
            return s;
        }
        

        class FERRISEXP_DLLLOCAL ContextPopTableCollector : public basic_PopTableCollector
        {
            fh_mdcontext c;
    
        public:


            string makeCmdLine( const string& s )
                {
                    string ret = "override-";
                    ret += s;
                    return ret;
                }

            string unmakeCmdLine( const string& s )
                {
                    string prefix = "override-";
                    return s.substr( prefix.length() );
                }
            
    
    
            virtual void poptCallback(poptContext con,
                                      enum poptCallbackReason reason,
                                      const struct poptOption * opt,
                                      const char * arg,
                                      const void * data)
                {
                    const string key = opt->longName;
                    typedef Context::SubContextNames_t cnt;
                    cnt cn = c->getSubContextNames();
//                    fh_context ctx = GetImpl(c);
                    fh_context ctx;
                    Upcast( ctx, c );
                    if( cn.size() < 2 )
                    {
                        ctx = c->getSubContext( cn.front() );
                        cn = ctx->getSubContextNames();
                    }
                    
                    cnt::iterator iter = find( cn.begin(), cn.end(),
                                               unmakeCmdLine(key) );

//                     cerr << "poptCallback() key1:" << key << endl;
//                     cerr << "poptCallback() key2:" << unmakeCmdLine(key) << endl;
                    
                    if( iter != cn.end() )
                    {

                        string argstr(arg);
//                         cerr << "poptCallback()   iter:" << *iter << endl;
//                         cerr << "poptCallback() argstr:" << argstr << endl;
                        Context* childc = GetImpl(ctx->getSubContext( *iter ));
                        if( CreateMetaDataContext* child
                            = dynamic_cast<CreateMetaDataContext*>(childc))
                        {
                            child->setBody( argstr );
                        }
                    }
                }


        
            struct ::poptOption* getTable( fh_mdcontext _c )
                {
                    c = _c;
                    int extraTableLines = 5;


                    typedef Context::SubContextNames_t cnt;
                    cnt cn = c->getSubContextNames();
//                    fh_context ctx = GetImpl(c);
                    fh_context ctx;
                    Upcast( ctx, c );
                    if( cn.size() < 2 )
                    {
                        ctx = c->getSubContext( cn.front() );
                        cn = ctx->getSubContextNames();
                    }

                    allocTable( cn.size() + extraTableLines );
                    int i=0;
                    setToCallbackEntry( &table[i] );
                    ++i;

                    for( cnt::iterator iter = cn.begin(); iter != cn.end(); ++iter )
                    {
                        string s = getStrSubCtx( ctx, *iter, "" );
                        setEntry( &table[i], makeCmdLine( *iter ),
                                  0, POPT_ARG_STRING, 0,
                                  0,
                                  trimdots(s, 45).c_str() );
                        ++i;
//                        cerr << "adding param(c) iter:" << *iter << endl;
                    }
                    clearEntry( &table[i] );
                    return table;
                }
    
        };
        
    };
    
        

    struct ::poptOption* getContextPopTableCollector( fh_mdcontext c,
                                                      fh_context& setc )
    {
//        setc = GetImpl(c);
        Upcast( setc, c );
        typedef Loki::SingletonHolder<
            Private::ContextPopTableCollector
            > ContextPopTableCollector_t;
        
        return ContextPopTableCollector_t::Instance().getTable( c );
    }

    
        
};

    
