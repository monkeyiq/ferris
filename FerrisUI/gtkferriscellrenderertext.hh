/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cell renderer
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

    $Id: gtkferriscellrenderertext.hh,v 1.2 2010/09/24 21:31:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __GTK_FERRIS_CELL_RENDERER_TEXT_H__
#define __GTK_FERRIS_CELL_RENDERER_TEXT_H__

#include <Ferris/HiddenSymbolSupport.hh>
#include <pango/pango.h>
#include <gtk/gtkcellrenderertext.h>

extern "C" {


#define GTK_TYPE_FERRIS_CELL_RENDERER_TEXT		(gtk_ferris_cell_renderer_text_get_type ())
#define GTK_FERRIS_CELL_RENDERER_TEXT(obj)		(GTK_CHECK_CAST ((obj), GTK_TYPE_FERRIS_CELL_RENDERER_TEXT, GtkFerrisCellRendererText))
#define GTK_FERRIS_CELL_RENDERER_TEXT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FERRIS_CELL_RENDERER_TEXT, GtkFerrisCellRendererTextClass))
#define GTK_IS_FERRIS_CELL_RENDERER_TEXT(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_FERRIS_CELL_RENDERER_TEXT))
#define GTK_IS_FERRIS_CELL_RENDERER_TEXT_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), GTK_TYPE_FERRIS_CELL_RENDERER_TEXT))
#define GTK_FERRIS_CELL_RENDERER_TEXT_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_FERRIS_CELL_RENDERER_TEXT, GtkFerrisCellRendererTextClass))

typedef struct _GtkFerrisCellRendererText      GtkFerrisCellRendererText;
typedef struct _GtkFerrisCellRendererTextClass GtkFerrisCellRendererTextClass;

struct FERRISEXP_API _GtkFerrisCellRendererText
{
    GtkCellRendererText parent;

    gboolean calc_fixed_size;
    gchar* calc_fixed_template;
};

struct FERRISEXP_API _GtkFerrisCellRendererTextClass
{
  GtkCellRendererTextClass parent_class;

  void (* onedit) (GtkFerrisCellRendererText *cell_renderer_text,
                   gchar               *path,
                   gpointer            shouldveto );

    void (* old_get_size)(GtkCellRenderer *cell,
                          GtkWidget       *widget,
                          GdkRectangle    *cell_area,
                          gint            *x_offset,
                          gint            *y_offset,
                          gint            *width,
                          gint            *height);
    
    
};

    FERRISEXP_API GtkType          gtk_ferris_cell_renderer_text_get_type (void);
    FERRISEXP_API GtkCellRenderer *gtk_ferris_cell_renderer_text_new      (void);

    FERRISEXP_API void
    gtk_cell_renderer_text_set_fixed_size_string( GtkFerrisCellRendererText *renderer,
                                                  const gchar* template_string );

}
#endif
