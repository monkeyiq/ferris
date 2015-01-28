/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

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

    $Id: MedallionEditor.hh,v 1.5 2010/09/24 21:31:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRISUI_MEDALLION_EDITOR_H_
#define _ALREADY_INCLUDED_FERRISUI_MEDALLION_EDITOR_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <gtk/gtk.h>
#include <Ferris/Ferris.hh>
#include <Ferris/Medallion.hh>

namespace FerrisUI
{
    using namespace Ferris;

    
    class MedallionEditor;
    FERRIS_SMARTPTR( MedallionEditor, fh_medallionEditor );
    
    class FERRISEXP_API MedallionEditor
        :
        public Handlable
    {
        typedef MedallionEditor _Self;
        
        fh_context m_ctx;
        fh_etagere m_et;
        bool       m_showDesc;
        bool       m_showFuzzyHints;
        bool       m_userMedallionUpdateHandled;
        
        enum {
            C_ASSERT_COLUMN=0,
            C_RETRACT_COLUMN,
            C_EMBLEM_ID_COLUMN,
            C_EMBLEM_NAME_COLUMN,
            C_EMBLEM_ICON_COLUMN,
            C_EMBLEM_DESC_COLUMN,
            C_EMBLEM_FUZZY_COLUMN,
            C_COLUMN_COUNT
        };
        GtkTreeStore*      w_treemodel;
        GtkWidget*         w_treeview;
        GtkTreeViewColumn* w_cols[C_COLUMN_COUNT];
        GtkWidget*         w_baseWidget;

        sigc::connection   m_ctxConnection;

        void update_model( fh_medallion& med, GtkTreeIter* piter, fh_emblem parent_em );
        void update_all_emblems_in_partial_order_rec( GtkTreeModel* model, GtkTreeIter* iter,
                                                      fh_medallion med, fh_emblem em, emblemID_t eid );
        void update_all_emblems_in_partial_order( GtkTreeModel* model, fh_medallion med, fh_emblem em );

        
    public:

        MedallionEditor();
        virtual ~MedallionEditor();
        

        /**
         * Show the description of the emblem too. Default false
         * set before calling getWidget()
         */
        void setShowDescription( bool v );

        /**
         * Show the result of getFuzzyBelief for each emblem too.
         * Default true. Set before calling getWidget()
         */
        void setShowFuzzyHints( bool v );
        
        /**
         * set the context we are editing
         */
        void setContext( fh_context c );
        fh_context getContext();
        
        GtkWidget* getWidget();

        /*********************************************************/
        /*** the following should be considered private **********/
        /*********************************************************/
        emblemID_t getCurrentSelectedEmblemID();
        emblemID_t getCurrentSelectedEmblemID( GtkTreeIter *iter );
        
        void clear();
        void update_model();

        void OnMedallionUpdated( fh_context c );
        void OnEmblemCreated( fh_etagere, fh_emblem em );

        /*********************************************************/
        /*** signal handlers *************************************/
        /*********************************************************/
        void emblem_assert_toggled( GtkCellRendererToggle *cell, gchar *path_string );
        void emblem_retract_toggled( GtkCellRendererToggle *cell, gchar *path_string );

        void on_gtk_row_expanded ( GtkTreeView *treeview,
                                   GtkTreeIter *arg1,
                                   GtkTreePath *arg2 );
        
    };
};
#endif
