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

    $Id: FerrisPopt.cpp,v 1.2 2010/09/24 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisPopt.hh>

#include <iostream>

using namespace std;

namespace Ferris
{
    

    namespace CallBacks
    {

        poptCallable::poptCallable()
        {
        }
        
        poptCallable::~poptCallable()
        {
        }
    
        
        void poptCallback(poptContext con,
                          enum poptCallbackReason reason,
                          const struct poptOption * opt,
                          const char * arg,
                          const void * data)
        {
            poptCallable* c = (poptCallable*)data;
            c->poptCallback( con, reason, opt, arg, data );
        }
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    basic_PopTableCollector::basic_PopTableCollector()
        :
        table(0)
    {
    }

    basic_PopTableCollector::~basic_PopTableCollector()
    {
        if( table )
            delete[] table;
    }


    void
    basic_PopTableCollector::setEntry( struct ::poptOption* p,
                                       const string& _longname,
                                       char shortname,
                                       int arginfo,
                                       void* arg,
                                       const char* _descrip,
                                       const char* _argDescrip )
    {
        static list<string> cache;

        cache.push_back( _longname );   const string&  ln  = cache.back();
    
        p->longName  = ln.length() ? ln.c_str() : 0;
        p->shortName = shortname;
        p->argInfo   = arginfo;
        p->arg       = arg;
        p->val       = 0;

        p->descrip = 0;
        if( _descrip )
        {
            cache.push_back( _descrip );
            const string&  des = cache.back();
            p->descrip         = des.c_str();
        }
        
        p->argDescrip = 0;
        if( _argDescrip )
        {
            cache.push_back( _argDescrip );
            const string& ades = cache.back();
            p->argDescrip= ades.c_str();
        }
    }


    void
    basic_PopTableCollector::setEntry( struct ::poptOption* p,
                                       const string& _longname,
                                       char shortname,
                                       int arginfo,
                                       void* arg,
                                       const string& _descrip,
                                       const string& _argDescrip)
    {
        setEntry( p, _longname, shortname, arginfo, arg,
                  _descrip.c_str(), _argDescrip.c_str() );
    }

    void
    basic_PopTableCollector::setToCallbackEntry( struct ::poptOption* p )
    {
        setEntry( p, "", 0,
                  POPT_ARG_CALLBACK, (void*)CallBacks::poptCallback,
                  "" );
        p->longName= 0;
        p->descrip = (const char*)this;
        p->argDescrip="";
    }
        

    void
    basic_PopTableCollector::clearEntry( struct ::poptOption* p )
    {
        setEntry( p, "", 0, 0, 0 );
        p->longName = 0;
        p->descrip  = 0;
        p->argDescrip=0;
    }


    void
    basic_PopTableCollector::allocTable( int sz )
    {
        int extraTableLines = 5;

        if( table )
            delete[] table;
        table = new struct ::poptOption[ sz + extraTableLines ];
    }

 
};
