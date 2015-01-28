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

    $Id: EditSQLColumns.hh,v 1.2 2010/09/24 21:31:04 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRISUI_EDIT_SQL_COLUMNS_H_
#define _ALREADY_INCLUDED_FERRISUI_EDIT_SQL_COLUMNS_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <gtk/gtk.h>

#include <Ferris/TypeDecl.hh>

#include <string>

namespace FerrisUI
{
    class FERRISEXP_API EditSQLColumns
        :
        public Ferris::Handlable
    {
        friend void EditSQLColumns_add_cb( GtkButton *button, gpointer user_data );
        friend void EditSQLColumns_del_cb( GtkButton *button, gpointer user_data );
        friend void EditSQLColumns_clear_cb( GtkButton *button, gpointer user_data );
        friend void EditSQLColumns_edited_cb( GtkCellRendererText *cell,
                                              gchar *path_string,
                                              gchar *new_text,
                                              gpointer user_data );
        

    protected:

        enum {
            C_COLUMN_NAME=0,
            C_COLUMN_TYPE=1,
            C_COLUMN_COUNT=2
        };
        GtkTreeStore*      w_treemodel;
        GtkWidget*         w_treeview;
        GtkTreeViewColumn* w_cols[ C_COLUMN_COUNT ];
        GtkWidget*         w_baseWidget;
        
        std::string m_columnNameLabel;
        std::string m_columnTypeLabel;
        std::string m_descriptionLabel;

    public:

        EditSQLColumns();
        virtual ~EditSQLColumns();

        /**
         * Set the description string for this string list
         * call before getWidget()
         */
        void setDescriptionLabel( const std::string& s );

        /**
         * Get the current contents of the string list
         * call AFTER getWidget()
         */
        Ferris::stringmap_t  getStringMap();

        /**
         * set the content of the string list
         * call AFTER getWidget()
         */
        void setStringMap( const Ferris::stringmap_t& sm );

        /**
         * Get the GTK+ view for this string editing component.
         */
        GtkWidget* getWidget();

        /**
         * clear the stringlist
         * call AFTER getWidget()
         */
        void clear();

        /**
         * add a new blank item
         * call AFTER getWidget()
         */
        GtkTreeIter appendNewBlankItem();

    };

    FERRIS_SMARTPTR( EditSQLColumns, fh_editsqlcolumns );
    
};
#endif
