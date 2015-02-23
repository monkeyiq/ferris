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

    $Id: TreeStoreDriver.cpp,v 1.2 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <GtkFerris_private.hh>
// #include <TreeStoreDriver.hh>
// #include <gtk/gtk.h>

// namespace FerrisUI
// {

    

// //     TreeStoreDriver::TreeStoreDriver( GtkFerrisTreeStore* _t )
// //         :
// //         t(_t),
// //         m_contextsDone(0),
// //         m_subContextCountGuess(0),
// //         ls( new Ferrisls() )
// //     {
// //         setTree( _t );
        
// //         ls->setDisplay( this );
    
// //         ls->getDiskReadStarted_Sig().connect(  slot( this, &_Self::DiskReadStarted));
// //         ls->getDiskReadProgress_Sig().connect( slot( this, &_Self::DiskReadProgress));
// //         ls->getDiskReadDone_Sig().connect(     slot( this, &_Self::DiskReadDone));
// //         ls->getFilterStarted_Sig().connect(    slot( this, &_Self::FilterStarted));
// //         ls->getSortStarted_Sig().connect(      slot( this, &_Self::SortStarted));
// //         ls->getContextPropergationStarted_Sig().connect(
// //             slot( this, &_Self::ContextPropergationStarted));

// //         setMonitorCreate( true );
// //         setMonitorDelete( true );
// //         setMonitorChanged( true );
// //     }

// //     void
// //     TreeStoreDriver::setTree( GtkFerrisTreeStore* _t )
// //     {
// //         t = _t;
// //         if( t )
// //             t->d = this;
// //     }
    
// //     Ferrisls&
// //     TreeStoreDriver::getFerrisls()
// //     {
// //         Ferrisls* p = GetImpl(ls);
// //         return *p;
// //     }
    
//     void
//     TreeStoreDriver::EnteringContext( fh_context ctx )
//     {
//         m_subContextCountGuess = ctx->guessSize();
//         m_contextsDone = 0;
//         if( !m_subContextCountGuess )
//             m_subContextCountGuess = 1;

//         getEnteringContextSig().emit( ctx );
//     }

//     void
//     TreeStoreDriver::workComplete()
//     {
//         getWorkCompleteSig().emit();
//     }

//     void
//     TreeStoreDriver::ShowAttributes( fh_context ctx )
//     {
// //        cerr << "TreeStoreDriver::ShowAttributes url:" << ctx->getURL() << endl;
//         add( t, ctx );
//         getShowAttributesSig().emit( ctx, m_contextsDone, m_subContextCountGuess );

//         ++m_contextsDone;
//         if( m_contextsDone % 20 == 0 )
//         {
//             getShowAttributesUpdateGUISig().emit( ctx, m_contextsDone, m_subContextCountGuess );
//         }
//     }

//     void
//     TreeStoreDriver::DiskReadStarted( Ferrisls& ls, fh_context )
//     {
// //        cerr << "gtk ferris tree diskReadStarted() t:" << (void*)t << endl;
//     }


//     void
//     TreeStoreDriver::ContextPropergationStarted(  Ferrisls& ls, fh_context c )
//     {
// //        cerr << "ContextPropergationStarted() c:" << c->getDirPath() <<  endl;
    
// //         AddAllNewEANames( t, c );
    
// //         if( !isBound( t->rootCtx ) )
// //         {
// //             gtk_ferris_tree_store_set_root_context( t, c );
// //             gint NumberOfColumns = t->columnNames.size();
// //             cerr << "NumberOfColumns:" << NumberOfColumns << endl;
// //         }
//     }

//     void
//     TreeStoreDriver::DiskReadProgress( Ferrisls& ls, fh_context, long fnum )
//     {
    
//     }

//     void
//     TreeStoreDriver::DiskReadDone( Ferrisls& ls, fh_context )
//     {
//     }

//     void
//     TreeStoreDriver::FilterStarted( Ferrisls& ls )
//     {
//     }

//     void
//     TreeStoreDriver::SortStarted( Ferrisls& ls )
//     {
//     }

// //     void
// //     TreeStoreDriver::OnExists ( NamingEvent_Exists* ev,  std::string olddn, std::string newdn )
// //     {
// //         fh_context c    = ev->getSource();
// //         fh_context newc = c->getSubContext(newdn);

// // //         if( !isBound( t->rootCtx ) )
// // //         {
// // //             gtk_ferris_tree_store_set_root_context( t, c );
// // //         }
    
    
// // //         cerr << "TreeStoreDriver::OnExists    c:" << c->getURL() << endl;
// // //         cerr << "TreeStoreDriver::OnExists newc:" << newc->getURL() << endl;

// //         ShowAttributes( newc );
// // //        AttachSignals(  newc );
    
// //     }

// //     void
// //     TreeStoreDriver::OnSubContextCreated( NamingEvent_Created* ev,
// //                                           std::string olddn, std::string newdn )
// //     {
// //         fh_context c    = ev->getSource();
// //         fh_context newc = c->getSubContext(newdn);
// //         add( t, newc );
// //     }
    
// //     void
// //     TreeStoreDriver::OnCreated( NamingEvent_Created* ev, std::string olddn, std::string newdn )
// //     {
// //         fh_context c    = ev->getSource();
// //         fh_context newc = c->getSubContext(newdn);
    
// //         cout << "TreeStoreDriver::OnCreated newc" << newc->getURL() << endl;
    
// //         ShowAttributes( newc );
// // //        AttachSignals(  newc );
// //     }

// //     void
// //     TreeStoreDriver::OnChanged( NamingEvent_Changed* ev, std::string olddn, std::string newdn )
// //     {
// //         fh_context c = ev->getSource();
// //         cout << "+++++ Changed c:" << c->getURL()
// //              << " olddn:" << olddn
// //              << endl;

// //         GtkTreeIter  iter = getIterForContext( t, c );
// //         cerr << "TreeStoreDriver::OnChanged 2." << endl;
// //         GtkTreePath* path = gtk_tree_model_get_path( GTK_TREE_MODEL (t), &iter );
// //         cerr << "TreeStoreDriver::OnChanged 3." << endl;
// //         g_return_if_fail( path );

// //         cerr << "TreeStoreDriver::OnChanged has_child:"
// //              << gtk_tree_model_iter_has_child( GTK_TREE_MODEL(t), &iter )
// //              << " strattr:" << toint( getStrAttr( c, "has-subcontexts-guess", "x" ))
// //              << endl;
// //         cerr << "TreeStoreDriver::OnChanged 4." << endl;

// // //         GtkTreeIter tmp;
// // //         gboolean rc = gtk_tree_model_iter_children( GTK_TREE_MODEL(t),
// // //                                                     &tmp,
// // //                                                     &iter );
// // //         cerr << "TreeStoreDriver::OnChanged rc:" << rc << endl;
// // //         if( rc )
// // //             cerr << "TreeStoreDriver::OnChanged child:"
// // //                  << toContext( t, &tmp )->getURL() << endl;
        
        
// // //         gboolean gtk_tree_store_iter_has_child( GTK_TREE_MODEL (t), &iter );
 
// // //                                       GtkTreeIter  *iter )
        
// //         gtk_tree_model_row_has_child_toggled( GTK_TREE_MODEL (t), path, &iter );

        
// //         gtk_tree_model_row_changed( GTK_TREE_MODEL (t), path, &iter );
// //         gtk_tree_path_free( path );
// //     }

// //     void
// //     TreeStoreDriver::OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn )
// //     {
// //         fh_context rc = ev->getSource();
// //         fh_context c  = rc->getSubContext(olddn);
// //         cout << "TreeStoreDriver::OnDeleted() c:" << c->getURL() << endl;

// //         GtkTreeIter  iter = getIterForContext( t, c );
// //         GtkTreePath* path = gtk_tree_model_get_path( GTK_TREE_MODEL (t), &iter );
// //         g_return_if_fail( path );

// //         cerr << "gtk_path being removed:" << tostr(path) << endl;
// //         gtk_tree_model_row_deleted( GTK_TREE_MODEL (t), path );
// //         gtk_tree_path_free( path );
// //     }

    

// };
