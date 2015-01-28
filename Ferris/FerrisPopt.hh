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

    $Id: FerrisPopt.hh,v 1.2 2010/09/24 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#ifndef _ALREADY_INCLUDED_FERRIS_POPT_H_
#define _ALREADY_INCLUDED_FERRIS_POPT_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <string>
#include <list>

#include <popt.h>

namespace Ferris
{

    namespace CallBacks
    {
        class FERRISEXP_DLLLOCAL poptCallable
        {
            poptCallable( const poptCallable& );
            poptCallable& operator=( poptCallable& );
        
        public:
            
            poptCallable();
            virtual ~poptCallable();
            
            virtual void poptCallback(poptContext con,
                                      enum poptCallbackReason reason,
                                      const struct poptOption * opt,
                                      const char * arg,
                                      const void * data) = 0;
        };
        
    };
    
    class FERRISEXP_DLLLOCAL basic_PopTableCollector : public CallBacks::poptCallable
    {
    protected:
        
        struct ::poptOption* table;
        
    public:

        basic_PopTableCollector();
        virtual ~basic_PopTableCollector();
        
        virtual void poptCallback(poptContext con,
                          enum poptCallbackReason reason,
                          const struct poptOption * opt,
                          const char * arg,
                          const void * data) = 0;

        /**
         * The joy of using this method is that it caches all strings in
         * a location that is not going to change for the lifetime of
         * usage. Otherwise the caller would need to make sure such a
         * cache exists.
         */
        void setEntry( struct ::poptOption* p,
                       const std::string& _longname,
                       char shortname = 0,
                       int arginfo = POPT_ARG_NONE,
                       void* arg = 0,
                       const char* _descrip = "",
                       const char* _argDescrip = "");

        void setEntry( struct ::poptOption* p,
                       const std::string& _longname,
                       char shortname,
                       int arginfo,
                       void* arg,
                       const std::string& _descrip,
                       const std::string& _argDescrip);
        
        void setToCallbackEntry( struct ::poptOption* p );
        void clearEntry( struct ::poptOption* p );
        

        void allocTable( int sz );
        
//         virtual struct ::poptOption* getTable(
//             const std::string& desc,
//             const std::list<std::string>& names );
    };
};



#endif
