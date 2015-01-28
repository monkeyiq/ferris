/**
 * Changed a little from the code in gaim 0.68 to handle natively
 * the -100 to +100 range I throw at it.
 */

/* gtkcellrendererprogress.c
 * Copyright (C) 2002, Sean Egan <bj91704@binghamton.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This is taken largely from GtkCellRenderer[Text|Pixbuf|Toggle] by 
 * Jonathon Blandford <jrb@redhat.com> for RedHat, Inc.
 */

#include "gtkcellrendererprogress.h"

static void gtk_cell_renderer_progress_get_property  (GObject                    *object,
						      guint                       param_id,
						      GValue                     *value,
						      GParamSpec                 *pspec);
static void gtk_cell_renderer_progress_set_property  (GObject                    *object,
						      guint                       param_id,
						      const GValue               *value,
						      GParamSpec                 *pspec);
static void gtk_cell_renderer_progress_init       (GtkCellRendererProgress      *cellprogress);
static void gtk_cell_renderer_progress_class_init (GtkCellRendererProgressClass *class);
static void gtk_cell_renderer_progress_get_size   (GtkCellRenderer            *cell,
						   GtkWidget                  *widget,
						   GdkRectangle               *cell_area,
						   gint                       *x_offset,
						   gint                       *y_offset,
						   gint                       *width,
						   gint                       *height);
static void gtk_cell_renderer_progress_render     (GtkCellRenderer            *cell,
						   GdkWindow                  *window,
						   GtkWidget                  *widget,
						   GdkRectangle               *background_area,
						   GdkRectangle               *cell_area,
						   GdkRectangle               *expose_area,
						   guint                       flags);
#if 0
static gboolean gtk_cell_renderer_progress_activate  (GtkCellRenderer            *cell,
						      GdkEvent                   *event,
						      GtkWidget                  *widget,
						      const gchar                *path,
						      GdkRectangle               *background_area,
						      GdkRectangle               *cell_area,
						      guint                       flags);
#endif
static void  gtk_cell_renderer_progress_finalize (GObject *gobject);

enum {
	LAST_SIGNAL
};

enum {
	PROP_0,

	PROP_PERCENTAGE,
	PROP_TEXT,
	PROP_SHOW_TEXT
};
     
static gpointer parent_class;
/* static guint progress_cell_renderer_signals [LAST_SIGNAL]; */

GType  gtk_cell_renderer_progress_get_type (void)
{
	static GType cell_progress_type = 0;
	
	if (!cell_progress_type)
		{
			static const GTypeInfo cell_progress_info =
				{
					sizeof (GtkCellRendererProgressClass),
					NULL,           /* base_init */
					NULL,           /* base_finalize */
					(GClassInitFunc) gtk_cell_renderer_progress_class_init,
					NULL,           /* class_finalize */
					NULL,           /* class_data */
					sizeof (GtkCellRendererProgress),
					0,              /* n_preallocs */
					(GInstanceInitFunc) gtk_cell_renderer_progress_init,
				};
			
			cell_progress_type =
				g_type_register_static (GTK_TYPE_CELL_RENDERER, "GtkCellRendererProgress",
							&cell_progress_info, 0);
		}
	
	return cell_progress_type;
}


static void gtk_cell_renderer_progress_init (GtkCellRendererProgress *cellprogress)
{
	GTK_CELL_RENDERER(cellprogress)->mode = GTK_CELL_RENDERER_MODE_INERT;
	GTK_CELL_RENDERER(cellprogress)->xpad = 2;
	GTK_CELL_RENDERER(cellprogress)->ypad = 2;
}

static void gtk_cell_renderer_progress_class_init (GtkCellRendererProgressClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(class);
	
	parent_class = g_type_class_peek_parent (class);
	object_class->finalize = gtk_cell_renderer_progress_finalize;

	object_class->get_property = gtk_cell_renderer_progress_get_property;
	object_class->set_property = gtk_cell_renderer_progress_set_property;
	
	cell_class->get_size = gtk_cell_renderer_progress_get_size;
	cell_class->render   = gtk_cell_renderer_progress_render;
	
	g_object_class_install_property (object_class,
					 PROP_PERCENTAGE,
					 g_param_spec_double ("percentage",
							      "Percentage",
							      "The fractional progress to display",
							      -100, 100, 0,
							      G_PARAM_READWRITE));
							      
	g_object_class_install_property (object_class,
					 PROP_TEXT,
					 g_param_spec_string ("text",
							      "Text",
							      "Text to overlay over progress bar",
							      NULL,
							      G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_SHOW_TEXT,
					g_param_spec_string("text_set",
							    "Text set",
							    "Whether to overlay text on the progress bar",
							    FALSE,
							    G_PARAM_READABLE | G_PARAM_WRITABLE));
}



static void gtk_cell_renderer_progress_finalize (GObject *object)
{
/*
	GtkCellRendererProgress *cellprogress = GTK_CELL_RENDERER_PROGRESS(object);
*/

	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void gtk_cell_renderer_progress_get_property (GObject    *object,
						     guint      param_id,
						     GValue     *value,
						     GParamSpec *psec)
{
	GtkCellRendererProgress *cellprogress = GTK_CELL_RENDERER_PROGRESS(object);

	switch (param_id)
		{
		case PROP_PERCENTAGE:
			g_value_set_double(value, cellprogress->progress);
			break;
		case PROP_TEXT:
			g_value_set_string(value, cellprogress->text);
			break;
		case PROP_SHOW_TEXT:
			g_value_set_boolean(value, cellprogress->text_set);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, psec);
			break;
		}
}

static void gtk_cell_renderer_progress_set_property (GObject      *object,
						     guint        param_id,
						     const GValue *value,
						     GParamSpec   *pspec)
{
	GtkCellRendererProgress *cellprogress = GTK_CELL_RENDERER_PROGRESS (object);

	switch (param_id)
		{
		case PROP_PERCENTAGE:
			cellprogress->progress = g_value_get_double(value);
			break;
		case PROP_TEXT:
			if (cellprogress->text)
				g_free(cellprogress->text);
			cellprogress->text = g_strdup(g_value_get_string(value));
			g_object_notify(object, "text");
			break;
		case PROP_SHOW_TEXT:
			cellprogress->text_set = g_value_get_boolean(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
			break;
		}
}

GtkCellRenderer *gtk_cell_renderer_progress_new(void)
{
	return g_object_new(GTK_TYPE_CELL_RENDERER_PROGRESS, NULL);
}

static void gtk_cell_renderer_progress_get_size (GtkCellRenderer *cell,
						 GtkWidget       *widget,
						 GdkRectangle    *cell_area,
						 gint            *x_offset,
						 gint            *y_offset,
						 gint            *width,
						 gint            *height)
{
	gint calc_width;
	gint calc_height;
	
	calc_width = (gint) cell->xpad * 2 + 50;
	calc_height = (gint) cell->ypad * 2 + 10;
	
	if (width)
		*width = calc_width;
	
	if (height)
		*height = calc_height;
	
	if (cell_area)
		{
			if (x_offset)
				{
					*x_offset = cell->xalign * (cell_area->width - calc_width);
					*x_offset = MAX (*x_offset, 0);
				}
			if (y_offset)
				{
					*y_offset = cell->yalign * (cell_area->height - calc_height);
					*y_offset = MAX (*y_offset, 0);
				}
		}
}


static void gtk_cell_renderer_progress_render (GtkCellRenderer *cell,
					       GdkWindow       *window,
					       GtkWidget       *widget,
					       GdkRectangle    *background_area,
					       GdkRectangle    *cell_area,
					       GdkRectangle    *expose_area,
					       guint            flags)
{
	GtkCellRendererProgress *cellprogress = (GtkCellRendererProgress *) cell;
	
	gint width, height;
	gint x_offset, y_offset;
	GtkStateType state;

	gtk_cell_renderer_progress_get_size (cell, widget, cell_area,
                                         &x_offset, &y_offset,
                                         &width, &height);
	
	
	if (GTK_WIDGET_HAS_FOCUS (widget))
		state = GTK_STATE_ACTIVE;
	else
		state = GTK_STATE_NORMAL;

    width -= cell->xpad*2;
	height -= cell->ypad*2;

	gtk_paint_box (widget->style,
                   window,
                   GTK_STATE_NORMAL, GTK_SHADOW_IN, 
                   NULL, widget, "trough",
                   cell_area->x + x_offset + cell->xpad,
                   cell_area->y + y_offset + cell->ypad,
                   width - 1, height - 1);

    int    leftToRight = 0;
    double zeroToOne = cellprogress->progress / 100.0;

    int maxBrightness = 40000;
    
    GdkColor c;
    c.pixel = 0;
    c.red   = 0;
    c.green = maxBrightness *    fabs(zeroToOne);
    c.blue  = 0;
    
    if( zeroToOne < 0 )
    {
        c.blue  = c.red;
        c.red   = c.green;
        c.green = c.blue;
        c.blue  = 0;
        zeroToOne *= -1;
        leftToRight = width - (width * zeroToOne);
    }
    if( zeroToOne > 1 )
        zeroToOne = 1;
    
//    gtk_widget_modify_bg( widget, state, &c );
//    printf("zeroToOne:%f\n", zeroToOne );

    if( zeroToOne > 0.05 )
    {
        GdkGC *gc = gdk_gc_new (window);
        gdk_gc_set_rgb_fg_color (gc, &c);
      
        gdk_draw_rectangle (window,
                            gc,
                            TRUE,
                            cell_area->x + x_offset + cell->xpad + leftToRight,
                            cell_area->y + y_offset + cell->ypad,
                            width * zeroToOne,
                            height - 1);

        g_object_unref (gc);
        
/*         gtk_paint_box (widget->style, */
/*                        window, */
/*                        state, GTK_SHADOW_OUT, */
/*                        NULL, widget, "bar", */
/*                        cell_area->x + x_offset + cell->xpad + leftToRight, */
/*                        cell_area->y + y_offset + cell->ypad, */
/*                        width * zeroToOne, */
/*                        height - 1); */
    }
    
}
