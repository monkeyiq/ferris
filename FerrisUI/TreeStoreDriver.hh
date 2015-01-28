/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris context tree store driver for reading and putting into a
    gtkferristreestore
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

    $Id: TreeStoreDriver.hh,v 1.2 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __GTK_FERRIS_TREE_STORE_DRIVER_H__
#define __GTK_FERRIS_TREE_STORE_DRIVER_H__

#include <Ferris/HiddenSymbolSupport.hh>

#include <string>
#include <vector>
#include <list>

#include <Ferris/Ferris.hh>
// #include <Ferris/Ferrisls.hh>

// #include <sigc++/signal_system.h>

// #include <FerrisUI/gtkferristreestore.hh>

// #include <gtk/gtk.h>

// namespace FerrisUI
// {
//     struct SubContextMonitoringConnections
//     {
//     };
    

//     class TreeStoreDriver
//         :
//         public Ferrisls_long_display
//     {
//         typedef Ferrisls_long_display _Base;
//         typedef TreeStoreDriver       _Self;
        
//     protected:
    
//         fh_ls ls;
//         GtkFerrisTreeStore* t;
    
//         virtual void EnteringContext(fh_context ctx);
//         virtual void workComplete();
//         virtual void ShowAttributes( fh_context ctx );
//         long m_contextsDone;
//         long m_subContextCountGuess;

//     public:
    
//         TreeStoreDriver( GtkFerrisTreeStore* _t = 0 );
//         void setTree( GtkFerrisTreeStore* _t );

//         Ferrisls& getFerrisls();

//         typedef SigC::Signal3< void, fh_context, long, long > ShowAttributesUpdateGUISig_t;
//         typedef SigC::Signal0< void > WorkCompleteSig_t;
//         typedef SigC::Signal1< void, fh_context > EnteringContextSig_t;
//         typedef SigC::Signal3< void, fh_context, long, long > ShowAttributesSig_t;
    
//         ShowAttributesUpdateGUISig_t ShowAttributesUpdateGUISig;
//         ShowAttributesUpdateGUISig_t& getShowAttributesUpdateGUISig();

//         WorkCompleteSig_t WorkCompleteSig;
//         WorkCompleteSig_t& getWorkCompleteSig();

//         EnteringContextSig_t EnteringContextSig;
//         EnteringContextSig_t& getEnteringContextSig();

//         ShowAttributesSig_t ShowAttributesSig;
//         ShowAttributesSig_t& getShowAttributesSig();
    
//         void DiskReadStarted( Ferrisls& ls, fh_context );
//         void ContextPropergationStarted(  Ferrisls& ls, fh_context c );
//         void DiskReadProgress( Ferrisls& ls, fh_context, long fnum );
//         void DiskReadDone( Ferrisls& ls, fh_context );
//         void FilterStarted( Ferrisls& ls );
//         void SortStarted( Ferrisls& ls );

//         virtual void OnExists ( NamingEvent_Exists* ev,  std::string olddn, std::string newdn );
//         virtual void OnCreated( NamingEvent_Created* ev, std::string olddn, std::string newdn );
//         virtual void OnChanged( NamingEvent_Changed* ev, std::string olddn, std::string newdn );
//         virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );

//         void OnSubContextCreated( NamingEvent_Created* ev, std::string olddn, std::string newdn );

//         void StartMonitoringSubContext( fh_context c );
//         void StopMonitoringSubContext( fh_context c );

//         struct SignalCollection
//         {
//             SigC::Connection ExistsConnection;
//             SigC::Connection CreatedConnection;
//             SigC::Connection ChangedConnection;
//             SigC::Connection DeletedConnection;
//         };
//         typedef std::map< fh_context, SignalCollection > SigCol_t;
//         SigCol_t SigCol;
//     };

 
// };


#endif
