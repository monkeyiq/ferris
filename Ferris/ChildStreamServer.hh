/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

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

    $Id: ChildStreamServer.hh,v 1.4 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CHILD_STREAM_SERV_H_
#define _ALREADY_INCLUDED_FERRIS_CHILD_STREAM_SERV_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <list>
#include <string>

#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>

#include <Functor.h>

namespace Ferris
{
    class ChildStreamServer;
    FERRIS_SMARTPTR( ChildStreamServer, fh_childserv );

    class FERRISEXP_API ChildStreamServer
        :
        public Handlable
    {
        typedef std::list< fh_runner > children_t;
        children_t m_children;

        static guint   s_childWatcherID;
        static guint32 s_childWatcherInterval;

    public:

        ChildStreamServer();
        ~ChildStreamServer();
        
        void addChild( fh_runner r );

        typedef sigc::signal< void ( ChildStreamServer*, fh_runner, int, int ) > ChildCompleteSig_t;
        ChildCompleteSig_t& getChildCompleteSig();
        
        typedef Loki::Functor< void,
                               LOKI_TYPELIST_3( fh_childserv,
                                                fh_runner,
                                                int /* exit status */ ) > ChildDiedFunctor_t;
        typedef std::list< ChildDiedFunctor_t > ChildDiedFunctors_t;
        ChildDiedFunctors_t m_ChildDiedFunctors;
        void addDiedFunctor( ChildDiedFunctor_t f );
        void removeDiedFunctor( ChildDiedFunctor_t f );
        void clearDiedFunctors();

        
        // private
        gint childHasDied( pid_t pid, int status );

    private:
        ChildCompleteSig_t ChildCompleteSig;
    };

    /****************************************/
    /****************************************/
    /****************************************/

//     class ChildWatcher;
//     FERRIS_SMARTPTR( ChildWatcher, fh_childwatcher );
    
//     class ChildWatcher
//         :
//         public Handlable
//     {
//     public:

//         ChildWatcher();
//         void attach( fh_childserv serv );

//         typedef sigc::signal< void ( ChildWatcher*, fh_runner, int, int,  bool ) > ChildCompleteSig_t;
//         ChildCompleteSig_t& getChildCompleteSig();

//     private:

//         bool m_childComplete;
//         bool m_childWasCanceled;
//         int  m_childExitStatus;
        
//         void child_complete( fh_childserv serv, fh_runner r, int status );
        
//         ChildCompleteSig_t ChildCompleteSig;
//     };
    
};

#endif
