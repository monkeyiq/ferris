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

    $Id: gtkferriscellrenderertext.cpp,v 1.3 2010/09/24 21:31:07 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <gtkferriscellrenderertext.hh>
#include <gtk/gtkeditable.h>
#include <gtk/gtkentry.h>
#include <gtk/gtksignal.h>
//#include <gtk/gtkintl.h>

#include <Ferris/Ferris.hh>
using namespace Ferris;
using namespace std;


static void gtk_ferris_cell_renderer_text_init       (GtkFerrisCellRendererText      *celltext);
static void gtk_ferris_cell_renderer_text_class_init (GtkFerrisCellRendererTextClass *c);
static void gtk_ferris_cell_renderer_text_finalize   (GObject                  *object);


static GtkCellEditable *gtk_ferris_cell_renderer_text_start_editing (
    GtkCellRenderer      *cell,
    GdkEvent             *event,
    GtkWidget            *widget,
    const gchar                *path,
    GdkRectangle         *background_area,
    GdkRectangle         *cell_area,
    GtkCellRendererState  flags);

enum {
    ONEDIT,
    LAST_SIGNAL
};

GtkCellEditable* (*parent_start_editing)
    (GtkCellRenderer      *cell,
     GdkEvent             *event,
     GtkWidget            *widget,
     const gchar                *path,
     GdkRectangle         *background_area,
     GdkRectangle         *cell_area,
     GtkCellRendererState  flags);
    
static gpointer parent_class;
static guint ferris_cell_renderer_signals [LAST_SIGNAL];


void
gtkf_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                      GValue       *return_value,
                                      guint         n_param_values,
                                      const GValue *param_values,
                                      gpointer      invocation_hint,
                                      gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__STRING_POINTER) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
  register GMarshalFunc_VOID__STRING_POINTER callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__STRING_POINTER) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            (char*)   g_value_get_string (param_values + 1),
            (gpointer)g_value_get_pointer(param_values + 2),
            data2);
}


void
monkeyiq_gtk_cell_renderer_set_fixed_size (GtkCellRenderer *cell,
                                           gint             width,
                                           gint             height)
{
  g_return_if_fail (GTK_IS_CELL_RENDERER (cell));

  if ((width != cell->width) || (height != cell->height))
    {
      g_object_freeze_notify (G_OBJECT (cell));

      if (width != cell->width)
        {
          cell->width = width;
          g_object_notify (G_OBJECT (cell), "width");
        }

      if (height != cell->height)
        {
          cell->height = height;
          g_object_notify (G_OBJECT (cell), "height");
        }

      g_object_thaw_notify (G_OBJECT (cell));
    }
}

static void
gtk_ferris_cell_renderer_text_get_size (GtkCellRenderer *cell,
				 GtkWidget       *widget,
				 GdkRectangle    *cell_area,
				 gint            *x_offset,
				 gint            *y_offset,
				 gint            *width,
				 gint            *height)
{
    GtkFerrisCellRendererText* fcell = (GtkFerrisCellRendererText*)cell;
    GtkCellRendererText *celltext    = GTK_CELL_RENDERER_TEXT(cell);

//     cerr << "gtk_ferris_cell_renderer_text_get_size(top)"
//          << " calc_fixed_size:" << fcell->calc_fixed_size
//          << " width:" << (width!=0) << " height:" << (height!=0)
//          << endl;
    
    PangoRectangle rect;
    PangoLayout *layout;

    if( fcell->calc_fixed_size )
    {
        PangoContext*          context;
        PangoFontMetrics*      metrics;
        PangoFontDescription*  font_desc;
        gint                   row_width;
        gint                   row_height;

        font_desc = pango_font_description_copy (widget->style->font_desc);
        pango_font_description_merge (font_desc, celltext->font, TRUE);

        if (celltext->scale_set)
        {
            pango_font_description_set_size(
                font_desc,
                (gint)(celltext->font_scale * pango_font_description_get_size (font_desc)));
        }
        
        context = gtk_widget_get_pango_context (widget);
        
        metrics = pango_context_get_metrics (context,
                                             font_desc,
                                             pango_context_get_language (context));
    
        row_height =
            pango_font_metrics_get_ascent (metrics)
            + pango_font_metrics_get_descent (metrics);

        row_width  =
            pango_font_metrics_get_approximate_char_width(metrics)
            * strlen( fcell->calc_fixed_template );
    
        pango_font_metrics_unref (metrics);

//         cerr << "gtk_ferris_cell_renderer_text_get_size() XXXOURSZ "
//              << " rw:" << PANGO_PIXELS(row_width)
//              << " rh:" << PANGO_PIXELS(row_height) << endl;
        
        monkeyiq_gtk_cell_renderer_set_fixed_size(
            cell,
            2*cell->xpad + PANGO_PIXELS (row_width),
            2*cell->ypad + celltext->fixed_height_rows * PANGO_PIXELS (row_height));

        if (height)
        {
            *height = cell->height;
            height = 0;
        }
        if( width )
        {
            *width = cell->width;
            width = 0;
        }

        return;
    }

//     cerr << "gtk_ferris_cell_renderer_text_get_size(fall)"
//          << " FALL THROUGH GET_SIZE "
//          << " calc_fixed_size:" << fcell->calc_fixed_size
//          << " width:" << (width!=0) << " height:" << (height!=0)
//          << endl;
    
    GtkFerrisCellRendererTextClass* cl = GTK_FERRIS_CELL_RENDERER_TEXT_GET_CLASS(cell);
    cl->old_get_size( cell, widget, cell_area, x_offset, y_offset, width, height );

//     char* tmp = "";
//     if( height ) tmp = " XXXCELLHEIGHT ";
//     cerr << "gtk_ferris_cell_renderer_text_get_size(end)"
//          << " FALL THROUGH GET_SIZE "
//          << tmp
//          << " calc_fixed_size:" << fcell->calc_fixed_size
//          << " width:" << (width ? *width : -1)
//          << " height:" << (height ? *height : -1)
//          << endl;
    
}



#define GTK_FERRIS_CELL_RENDERER_TEXT_PATH "gtk-ferris-cell-renderer-text-path"

GtkType
gtk_ferris_cell_renderer_text_get_type (void)
{
    static GtkType cell_text_type = 0;

    if (!cell_text_type)
    {
        static const GTypeInfo cell_text_info =
            {
                sizeof (GtkFerrisCellRendererTextClass),
                NULL,		/* base_init */
                NULL,		/* base_finalize */
                (GClassInitFunc) gtk_ferris_cell_renderer_text_class_init,
                NULL,		/* class_finalize */
                NULL,		/* class_data */
                sizeof (GtkFerrisCellRendererText),
                0,              /* n_preallocs */
                (GInstanceInitFunc) gtk_ferris_cell_renderer_text_init,
            };

        cell_text_type = g_type_register_static(
            GTK_TYPE_CELL_RENDERER_TEXT,
            "GtkFerrisCellRendererText",
            &cell_text_info, GTypeFlags(0));
    }

    return cell_text_type;
}

static void
gtk_ferris_cell_renderer_text_init (GtkFerrisCellRendererText *renderer)
{
    renderer->calc_fixed_template = 0;
}

static void
gtk_ferris_cell_renderer_text_class_init (GtkFerrisCellRendererTextClass *c)
{
    GObjectClass *object_class = G_OBJECT_CLASS (c);
    GtkFerrisCellRendererTextClass *fcell_class = GTK_FERRIS_CELL_RENDERER_TEXT_CLASS(c);
    parent_class = g_type_class_peek_parent (c);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (c);
  
    object_class->finalize    = gtk_ferris_cell_renderer_text_finalize;
    
    parent_start_editing      = cell_class->start_editing;
    cell_class->start_editing = gtk_ferris_cell_renderer_text_start_editing;
    
    fcell_class->old_get_size = cell_class->get_size;
    cell_class->get_size      = gtk_ferris_cell_renderer_text_get_size;
  
    ferris_cell_renderer_signals [ONEDIT] =
        gtk_signal_new ("onedit",
                        GTK_RUN_LAST,
                        GTK_CLASS_TYPE (object_class),
                        GTK_SIGNAL_OFFSET (GtkFerrisCellRendererTextClass, onedit),
                        gtkf_marshal_VOID__STRING_POINTER,
                        GTK_TYPE_NONE, 2,
                        G_TYPE_STRING, G_TYPE_POINTER );
}

static void
gtk_ferris_cell_renderer_text_finalize (GObject *object)
{
    GtkFerrisCellRendererText *celltext = GTK_FERRIS_CELL_RENDERER_TEXT (object);
    GtkFerrisCellRendererText *renderer = GTK_FERRIS_CELL_RENDERER_TEXT( object );
    
    if( renderer->calc_fixed_template )
        g_free( renderer->calc_fixed_template );
    
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}



GtkCellRenderer *
gtk_ferris_cell_renderer_text_new (void)
{
//    cerr << "gtk_ferris_cell_renderer_text_new (void)" << endl;
    return GTK_CELL_RENDERER(g_object_new (gtk_ferris_cell_renderer_text_get_type (), NULL));
}

static GtkCellEditable *
gtk_ferris_cell_renderer_text_start_editing (GtkCellRenderer      *cell,
                                             GdkEvent             *event,
                                             GtkWidget            *widget,
                                             const gchar                *path,
                                             GdkRectangle         *background_area,
                                             GdkRectangle         *cell_area,
                                             GtkCellRendererState  flags)
{
    GtkFerrisCellRendererText *celltext;
    GtkWidget *entry;
    guint32 ve = 0;

    gtk_signal_emit (GTK_OBJECT (cell),
                     ferris_cell_renderer_signals[ONEDIT],
                     path, &ve );

    LG_GTKFERRIS_D  << "gtk_ferris_cell_renderer_text_start_editing()"
                    << " veto:" << ve
                    << endl;
    
    if( ve )
    {
        return 0;
    }

    return parent_start_editing(
        cell, event, widget, path, background_area, cell_area, flags );
}



/**
 * gtk_cell_renderer_text_set_fixed_size_string:
 * @renderer: A #GtkFerrisCellRendererText
 * @template_string: template to use for finding the absolute cell size or 0
 *
 * Sets the size of the renderer to explicitly be determined by the "font", "x_pad"
 * and "y_pad" property set on it.  Further changes in these properties do not
 * affect the cell size, so they must be accompanied by a subsequent call to this
 * function.  Using this function is unflexible, and should really only be used
 * if calculating the size of a cell is too slow (ie, a massive number of cells
 * displayed) or if the cells of a column follow a rigid template and there are many
 * cells.  If @template_string is 0 then the fixed size is unset, and
 * the cell size is determined by the properties again.
 **/
void
gtk_cell_renderer_text_set_fixed_size_string( GtkFerrisCellRendererText *renderer,
                                              const gchar* template_string )
{
  g_return_if_fail (GTK_IS_FERRIS_CELL_RENDERER_TEXT (renderer));

  if( !template_string )
  {
        if( renderer->calc_fixed_template )
            g_free( renderer->calc_fixed_template );
        renderer->calc_fixed_template = 0;
        renderer->calc_fixed_size = false;
        
        gtk_cell_renderer_set_fixed_size (GTK_CELL_RENDERER (renderer), -1, -1);
  }
  else
  {
        renderer->calc_fixed_size = true;
        renderer->calc_fixed_template = g_strdup( template_string );
  }
}
