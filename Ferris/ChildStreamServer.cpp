/******************************************************************************
*******************************************************************************
*******************************************************************************

    fcat
    Copyright (C) 2002 Ben Martin

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

    $Id: ChildStreamServer.cpp,v 1.5 2010/09/24 21:30:25 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "ChildStreamServer.hh"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

using namespace std;

/*
 * Maintain one single timer and demux to each ChildStreamServer from there
 * maintain the list of ChildStreamServers in the ctor and dtor of ChildStreamServer
 *
 */
namespace Ferris
{
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    typedef list< ChildStreamServer* > childServers_t;
    childServers_t childServers;
    
    
    static gint childWatcher_cb( gpointer )
    {
//        cerr << "libferris:childWatcher_cb" << endl;
        
        int status  = 0;
        int options = WNOHANG;

//         struct rusage rusage;
//         pid_t pid = wait3( &status, options, &rusage );

        pid_t pid = waitpid( -1, &status, options );
        
        if( pid == 0 )
        {
            // WNOHANG and no children ready
        }
        if( pid < 0 )
        {
            // error
        }
        else
        {
            // valid child has died
            for( childServers_t::iterator iter = childServers.begin();
                 iter != childServers.end(); ++iter )
            {
                (*iter)->childHasDied( pid, status );
            }
        }

        return 1;
    }
    

    guint   ChildStreamServer::s_childWatcherID       = 0; 
    guint32 ChildStreamServer::s_childWatcherInterval = 100;
    
    ChildStreamServer::ChildStreamServer()
    {
        if( !s_childWatcherID )
        {
//             s_childWatcherID = g_timeout_add( s_childWatcherInterval,
//                                               GSourceFunc(::Ferris::childWatcher_cb),
//                                               0 );

            GSource* src = g_timeout_source_new( s_childWatcherInterval );
            g_source_set_callback( src,
                                   GSourceFunc(::Ferris::childWatcher_cb),
                                   0,
                                   0 );
            s_childWatcherID = g_source_attach( src, 0 );
            g_source_set_can_recurse( src, false );
            
        }
        childServers.push_back( this );
    }

    ChildStreamServer::~ChildStreamServer()
    {
        LG_EAIDX_D << "~ChildStreamServer()" << endl;

        childServers.remove( this );
        if( childServers.empty() )
        {
            g_source_remove( s_childWatcherID );
            s_childWatcherID = 0;
        }
    }
    
    
    void
    ChildStreamServer::addChild( fh_runner r )
    {
        r->setSpawnFlags(
            GSpawnFlags(
                r->getSpawnFlags() | G_SPAWN_DO_NOT_REAP_CHILD ));
        m_children.push_back( r );
    }
    
    gint
    ChildStreamServer::childHasDied( pid_t pid, int status )
    {
//         int status  = 0;
//         int options = WNOHANG;

//         pid_t pid = waitpid( -1, &status, options );

// //         struct rusage rusage;
// //         pid_t pid = wait3( &status, options, &rusage );

//         if( pid == 0 )
//         {
//             // WNOHANG and no children ready
//         }
//         if( pid < 0 )
//         {
//             // error
//         }
//         else
//         {
            // valid child has died
            for( children_t::iterator iter = m_children.begin();
                 iter != m_children.end(); ++iter )
            {
                fh_runner r = *iter;
                
                if( r->getChildProcessID() == pid )
                {
                    fh_childserv serv = this;

//                    cerr << "m_ChildDiedFunctors.sz:" << m_ChildDiedFunctors.size() << endl;

                    // signal the user that a child has died
                    // copy the functor list in case the user wants to change it
                    // from a callback
                    typedef ChildDiedFunctors_t::iterator I;
                    ChildDiedFunctors_t lcp = m_ChildDiedFunctors;
                    for( I fi = lcp.begin(); fi != lcp.end(); ++fi )
                    {
                        (*fi)( serv, r, status );
                    }

                    r->setExitStatus( status );

                    int estatus = WEXITSTATUS( status );
                    getChildCompleteSig().emit( this, r, status, estatus );

                    //
                    // Reap the child.
                    //
                    int status  = 0;
                    int options = 0;
                    pid_t rc = waitpid( pid, &status, options );
                }
            }
//        }
        
        return 1; // call again
    }

    ChildStreamServer::ChildCompleteSig_t&
    ChildStreamServer::getChildCompleteSig()
    {
        return ChildCompleteSig;
    }
    
    

    void
    ChildStreamServer::addDiedFunctor( ChildDiedFunctor_t f )
    {
        m_ChildDiedFunctors.push_back( f );
    }

    void
    ChildStreamServer::removeDiedFunctor( ChildDiedFunctor_t f )
    {
    }
    
    void
    ChildStreamServer::clearDiedFunctors()
    {
        m_ChildDiedFunctors.clear();
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


//     ChildWatcher::ChildWatcher()
//         :
//         m_childComplete( false ),
//         m_childWasCanceled( false ),
//         m_childExitStatus( -1 )
//     {
//     }

//     void
//     ChildWatcher::child_complete( fh_childserv serv, fh_runner r, int status )
//     {
//         if( WIFEXITED( status ) )
//         {
//             int s = WEXITSTATUS( status );
//             if( s != 0 )
//                 m_childWasCanceled = true;
//             m_childExitStatus = s;
//         }
        
//         cerr << "ChildWatcher::child_complete() " << endl;
//         m_childComplete = true;

// //         /*
// //          * Make sure that all async IO calls have been accepted.
// //          */
// //         Main::processAllPendingEvents();
//         serv->removeDiedFunctor(
//             ChildStreamServer::ChildDiedFunctor_t(
//                 this, &ChildWatcher::child_complete ));

//         getChildCompleteSig().emit( this, r, status, m_childExitStatus, m_childWasCanceled );
//     }

//     void
//     ChildWatcher::attach( fh_childserv serv )
//     {
//         serv->addDiedFunctor(
//             ChildStreamServer::ChildDiedFunctor_t(
//                 this, &ChildWatcher::child_complete ));
//     }

//     ChildWatcher::ChildCompleteSig_t&
//     ChildWatcher::getChildCompleteSig()
//     {
//         return ChildCompleteSig;
//     }
    
    

};
