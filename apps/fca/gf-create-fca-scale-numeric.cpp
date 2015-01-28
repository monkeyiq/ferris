/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
    Copyright (C) 2005 Ben Martin

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

    $Id: gf-create-fca-scale-numeric.cpp,v 1.4 2010/09/24 21:31:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <glib.h>
#include <gtk/gtk.h>

#include "libferrisfcascaling.hh"
#include <Ferris/FactoriesCreationCommon_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/General.hh>

#include <gtkextra/gtkplot.h>
#include <gtkextra/gtkplotdata.h>
#include <gtkextra/gtkplotbar.h>
#include <gtkextra/gtkplotcanvas.h>
#include <gtkextra/gtkplotcanvastext.h>
#include <gtkextra/gtkplotcanvasline.h>
#include <gtkextra/gtkplotcanvasellipse.h>
#include <gtkextra/gtkplotcanvasrectangle.h>
#include <gtkextra/gtkplotcanvasplot.h>
#include <gtkextra/gtkplotcanvaspixmap.h>
#include <gtkextra/gtkplotps.h>
#include <gtkextra/gtkplotprint.h>

const string PROGRAM_NAME = "ferris-create-fca-scale-numeric";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

unsigned long Verbose             = 0;
unsigned long OutputDouble        = 0;
unsigned long OutputInterval      = 0;
string attrname;

GtkWidget* gtk_window;
gfloat scale = 1.0;
gint page_width  = (int)(GTK_PLOT_LETTER_H * scale);
gint page_height = (int)(GTK_PLOT_LETTER_W * scale/2);
gint nlayers = 0;
GtkPlotCanvas *canvas = 0;
GtkWidget *active_plot = 0;

typedef map< string, string, versionltstr > SelectedValues_t;
SelectedValues_t SelectedValues;

void
quit ()
{
    gtk_main_quit();
}

GtkWidget *
new_layer(GtkWidget *canvas)
{
    GtkWidget* plot = 0;

    plot = gtk_plot_new_with_size(NULL, 1.0, 1.0 );
    gtk_widget_show( plot );

    return plot;
}

GtkPlotData *dataset[5];
void
build_example1(GtkWidget *plot)
{
 GdkColor color;
 GdkColor color2;
 GtkPlotAxis *axis;

 static gdouble px1[]={0., 0.2, 0.4, 0.6, 0.8, 1.0};
 static gdouble py1[]={.2, .4, .5, .35, .30, .40};
 static gdouble dx1[]={.2, .2, .2, .2, .2, .2};
 static gdouble dy1[]={.1, .1, .1, .1, .1, .1};

 static gdouble px2[]={0., -0.2, -0.4, -0.6, -0.8, -1.0};
 static gdouble py2[]={.2, .4, .5, .35, .30, .40};
 static gdouble dx2[]={.2, .2, .2, .2, .2, .2};
 static gdouble dy2[]={.1, .1, .1, .1, .1, .1};

 /* CUSTOM TICK LABELS */

// gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), TRUE);
// axis = gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT);
//  gtk_signal_connect(GTK_OBJECT(axis), "tick_label", 
//                     GTK_SIGNAL_FUNC(my_tick_label), NULL);

 dataset[0] = GTK_PLOT_DATA(gtk_plot_data_new());
 gtk_plot_add_data(GTK_PLOT(plot), dataset[0]);
 gtk_widget_show(GTK_WIDGET(dataset[0]));
 gtk_plot_data_set_points(dataset[0], px1, py1, 0,0, 6);
// gtk_plot_data_set_points(dataset[0], px1, py1, dx1, dy1, 6);

// gtk_plot_data_add_marker(dataset[0], 3);
// gtk_plot_data_add_marker(dataset[0], 4);
// gtk_plot_data_gradient_set_visible(dataset[0], TRUE);

 gdk_color_parse("red", &color);
 gdk_color_parse("blue", &color2);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 
 gdk_color_alloc(gdk_colormap_get_system(), &color2); 

 gtk_plot_data_set_symbol(dataset[0],
                          GTK_PLOT_SYMBOL_NONE,
                          GTK_PLOT_SYMBOL_OPAQUE,
                          10, 2, &color, &color);
 gtk_plot_data_set_line_attributes(dataset[0],
                                   GTK_PLOT_LINE_SOLID,
                                   (GdkCapStyle)0, (GdkJoinStyle)0, 1, &color2);
 
 gtk_plot_data_set_connector(dataset[0], GTK_PLOT_CONNECT_SPLINE);

 gtk_plot_data_show_yerrbars(dataset[0]);
 gtk_plot_data_set_legend(dataset[0], "Spline + EY");

//  dataset[3] = GTK_PLOT_DATA(gtk_plot_data_new());
//  gtk_plot_add_data(GTK_PLOT(plot), dataset[3]);
//  gtk_widget_show(GTK_WIDGET(dataset[3]));
//  gtk_plot_data_set_points(dataset[3], px2, py2, dx2, dy2, 6);
//  gtk_plot_data_set_symbol(dataset[3],
//                              GTK_PLOT_SYMBOL_SQUARE,
// 			     GTK_PLOT_SYMBOL_OPAQUE,
//                              8, 2, 
//                              &plot->style->black,
//                              &plot->style->black);
//  gtk_plot_data_set_line_attributes(dataset[3],
//                                    GTK_PLOT_LINE_SOLID,
//                                    (GdkCapStyle)0, (GdkJoinStyle)0, 4, &color);
//  gtk_plot_data_set_connector(dataset[3], GTK_PLOT_CONNECT_STRAIGHT);

//  gtk_plot_data_set_x_attributes(dataset[3], 
//                                 GTK_PLOT_LINE_SOLID,
//                                 (GdkCapStyle)0, (GdkJoinStyle)0, 0, &plot->style->black);
//  gtk_plot_data_set_y_attributes(dataset[3], 
//                                 GTK_PLOT_LINE_SOLID,
//                                 (GdkCapStyle)0, (GdkJoinStyle)0, 0, &plot->style->black);

//  gtk_plot_data_set_legend(dataset[3], "Line + Symbol");

}


// gint
// move_item(GtkWidget *widget, GtkPlotCanvasChild *child, double nx, double ny, gpointer data)
// {
//   GtkWidget **widget_list = NULL;
//   GtkWidget *active_widget = NULL;
//   gint n = 0;
//   gdouble *x = NULL, *y = NULL;

//   if(GTK_IS_PLOT_CANVAS_PLOT(child))
//   {
//     switch(GTK_PLOT_CANVAS_PLOT(child)->pos)
//     {
//       case GTK_PLOT_CANVAS_PLOT_IN_DATA:
//         x = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
//         y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
//         n = GTK_PLOT_CANVAS_PLOT(child)->datapoint;
//         printf("mx Item selected: DATA\n");
//         printf("mx Active point: %d   %f %f -> %f %f \n", 
//                GTK_PLOT_CANVAS_PLOT(child)->datapoint, x[n], y[n], nx, ny);
//         break;
//     }
//   }
//   return 0;
// }


void updateSelectedValues( const std::string& an, const std::string& v, const std::string& prev_value )
{
    stringstream ffilterss;
    if( prev_value.empty() )
    {
        ffilterss << "(" << attrname << "<=" << fixed << v << ")";
    }
    else
    {
        ffilterss << "(&"
                  <<    "(" << attrname << "<=" << fixed << v << ")"
                  <<    "(" << attrname << ">"  << fixed << prev_value << ")"
                  << ")";
    }
    
    SelectedValues[ an ] = ffilterss.str();
}



gint
select_item(GtkWidget *widget, GdkEvent *event, GtkPlotCanvasChild *child, 
            gpointer data)
{
  GtkWidget **widget_list = NULL;
  GtkWidget *active_widget = NULL;
  gint n = 0;
  gdouble *x = NULL, *y = NULL;

//   if(GTK_IS_PLOT_CANVAS_TEXT(child))
//         printf("Item selected: TEXT\n");
//   if(GTK_IS_PLOT_CANVAS_PIXMAP(child))
//         printf("Item selected: PIXMAP\n");
//   if(GTK_IS_PLOT_CANVAS_RECTANGLE(child))
//         printf("Item selected: RECTANGLE\n");
//   if(GTK_IS_PLOT_CANVAS_ELLIPSE(child))
//         printf("Item selected: ELLIPSE\n");
//   if(GTK_IS_PLOT_CANVAS_LINE(child))
//         printf("Item selected: LINE\n");
  if(GTK_IS_PLOT_CANVAS_PLOT(child)){
    switch(GTK_PLOT_CANVAS_PLOT(child)->pos){
//       case GTK_PLOT_CANVAS_PLOT_IN_TITLE:
//         printf("Item selected: TITLE\n");
//         break;
//       case GTK_PLOT_CANVAS_PLOT_IN_LEGENDS:
//         printf("Item selected: LEGENDS\n");
//         break;
//       case GTK_PLOT_CANVAS_PLOT_IN_PLOT:
//         printf("Item selected: PLOT\n");
//         break;
//       case GTK_PLOT_CANVAS_PLOT_IN_AXIS:
//         printf("Item selected: AXIS\n");
//         break;
//       case GTK_PLOT_CANVAS_PLOT_IN_MARKER:
//         printf("Item selected: MARKER\n");
//         break;
//       case GTK_PLOT_CANVAS_PLOT_IN_GRADIENT:
//         printf("Item selected: GRADIENT\n");
//         break;
      case GTK_PLOT_CANVAS_PLOT_IN_DATA:
        x = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        n = GTK_PLOT_CANVAS_PLOT(child)->datapoint;
        cerr << "Item selected: DATA" << endl;
        cerr << "Active point: " << GTK_PLOT_CANVAS_PLOT(child)->datapoint
             << " x:" << x[n] << " y:" <<  y[n] << endl;
        {
            stringstream valuess;
            if( OutputDouble )
                valuess << fixed << x[n];
            else
                valuess << (int)(ceil(x[n]));
            string v = valuess.str();
            stringstream anss;
            anss << attrname << "_" << v;

            if( OutputInterval )
            {
                cerr << "LB anss.str():" << anss.str() << endl;
                SelectedValues_t::iterator iter = SelectedValues.upper_bound( anss.str() );
                if( iter == SelectedValues.end() && !SelectedValues.empty() )
                {
                    cerr << "size && !LB anss.str():" << anss.str() << endl;
                    iter = SelectedValues.end();
                }
                if( !SelectedValues.empty() )
                    --iter;
                
                if( iter != SelectedValues.end() )
                {
                    string prev_an = iter->first;
                    string prev_value = prev_an.substr( prev_an.find("_")+1 );
                    cerr << "prev_an:" << prev_an << endl;

                    if( toint( prev_value ) > toint( v ) )
                    {
                        updateSelectedValues( anss.str(), v, "" );
                        updateSelectedValues( prev_an, prev_value, v );
                    }
                    else
                    {
                        updateSelectedValues( anss.str(), v, prev_value );
                        SelectedValues_t::iterator si = SelectedValues.find( anss.str() );
                        ++si;
                        if( si != SelectedValues.end() )
                        {
                            string next_an = si->first;
                            string next_value = next_an.substr( next_an.find("_")+1 );
                            cerr << "next_an:" << next_an << endl;
                            updateSelectedValues( next_an, next_value, v );
                        }
                    }
                }
                else
                {
                    cerr << "NO LB anss.str():" << anss.str() << endl;
                    updateSelectedValues( anss.str(), v, "" );
                    SelectedValues_t::iterator si = SelectedValues.find( anss.str() );
                    ++si;
                    if( si != SelectedValues.end() )
                    {
                        string next_an = si->first;
                        string next_value = next_an.substr( next_an.find("_")+1 );
                        cerr << "next_an:" << next_an << endl;
                        updateSelectedValues( next_an, next_value, v );
                    }
                }
            }
            else
            {
                updateSelectedValues( anss.str(), v, "" );
            }
        }
        
        return TRUE;
        break;
      default:
        break;
    }

//     widget_list = plots;
//     active_widget = GTK_WIDGET(GTK_PLOT_CANVAS_PLOT(child)->plot);
  }

  return FALSE;
}

void 
size_alloc_event(GtkWidget *area, GtkAllocation* a, gpointer data)
{
//    cerr << "size_alloc_event() w:" << a->width << " height:" << a->height << endl;
//    gtk_plot_canvas_set_magnification( GTK_PLOT_CANVAS( canvas ), 1.4 );
//    gtk_plot_canvas_set_size( GTK_PLOT_CANVAS( canvas ), 1000, 800 );
//    gtk_plot_resize( GTK_PLOT( active_plot ), a->width, a->height );
//    gtk_plot_paint( GTK_PLOT( active_plot ) );
    gtk_plot_canvas_set_size( GTK_PLOT_CANVAS( canvas ), a->width, a->height );
    gtk_plot_canvas_paint( GTK_PLOT_CANVAS( canvas ) );
}

void make_window()
{
    GtkWidget *vbox1 = 0;
    GtkWidget *scrollw1 = 0;

    int win_w = 1000;
    int win_h = 800;
    
    gtk_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gtk_window), "gf-create-fca-scale-numeric");
    gtk_widget_set_usize(gtk_window,win_w,win_h);

    gtk_signal_connect (GTK_OBJECT (gtk_window), "destroy",
                        GTK_SIGNAL_FUNC (quit), NULL);
    gtk_signal_connect(GTK_OBJECT(gtk_window), "size_allocate",
                       GTK_SIGNAL_FUNC(size_alloc_event), 0 );
    
    vbox1=gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(gtk_window),vbox1);
    gtk_widget_show(vbox1);

//     scrollw1=gtk_scrolled_window_new(NULL, NULL);
//     gtk_container_border_width(GTK_CONTAINER(scrollw1),0);
//     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw1),
//                                    GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
//     gtk_box_pack_start(GTK_BOX(vbox1),scrollw1, TRUE, TRUE,0);
//     gtk_widget_show(scrollw1);

    
    canvas = GTK_PLOT_CANVAS(gtk_plot_canvas_new(win_w,win_h, 1.0));
//    canvas = GTK_PLOT_CANVAS(gtk_plot_canvas_new(page_width, page_height, 1.0));
    
    GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);

    gtk_box_pack_start(GTK_BOX(vbox1),GTK_WIDGET(canvas), TRUE, TRUE,0);
//    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), GTK_WIDGET(canvas));
    gtk_widget_show(GTK_WIDGET(canvas));
    active_plot = GTK_WIDGET(new_layer(GTK_WIDGET(canvas)));


    gtk_plot_set_range(GTK_PLOT(active_plot), -1., 1., -1., 1.4);
    gtk_plot_legends_move(GTK_PLOT(active_plot), .500, .05);
// gtk_plot_set_legends_border(GTK_PLOT(active_plot), 0, 0);
    gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
    gtk_plot_axis_show_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), 15, 3);
    gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_X, 1., 1);
    gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_Y, 1., 1);
    gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
//    gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
    gtk_plot_x0_set_visible(GTK_PLOT(active_plot), TRUE);
    gtk_plot_y0_set_visible(GTK_PLOT(active_plot), TRUE);
    gtk_plot_axis_set_labels_suffix(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_LEFT), "");

 GtkPlotCanvasChild *child;
 
 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .1, .85, .85);
 gtk_widget_show(active_plot);
 GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;
// GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_DND_POINT;

// build_example1(active_plot);

 GtkPlotAxis *axis;
// gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
// axis = gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT);
//  gtk_signal_connect(GTK_OBJECT(axis), "tick_label", 
//                     GTK_SIGNAL_FUNC(my_tick_label), NULL);

 gtk_signal_connect(GTK_OBJECT(canvas), "select_item",
                    (GtkSignalFunc) select_item, NULL);

//  gtk_signal_connect(GTK_OBJECT(canvas), "move_item",
//                     (GtkSignalFunc) move_item, NULL);
 
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long ExcludeZero = 0;
        unsigned long TargetNumberOfAttrs = 10;
        unsigned long ReverseOrder = 0;
        unsigned long isTimeEA = 0;
        const char*   findexPath_CSTR    = 0;
        const char*   forceMinTime_CSTR  = 0;
        const char*   forceMinTimeRel_CSTR  = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "exclude-zero", 'z', POPT_ARG_NONE, &ExcludeZero, 0,
                  "explicitly exclude zero from output ffilters", "" },
                
                { "double", 'd', POPT_ARG_NONE, &OutputDouble, 0,
                  "output ffilters using double precision", "" },

                { "interval", 'I', POPT_ARG_NONE, &OutputInterval, 0,
                  "generate interval attributes", "" },

                { "is-time-attribute", 'T', POPT_ARG_NONE, &isTimeEA, 0,
                  "handle attribute as a time_t", "" },

//                 { "target-number-of-attributes", 'g', POPT_ARG_INT, &TargetNumberOfAttrs, 10,
//                   "desired number of formal attributes to create", "" },
                
//                 { "reverse", 'r', POPT_ARG_NONE, &ReverseOrder, 0,
//                   "reverse the sorting order", "" },

                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },

                { "force-min-time", 0, POPT_ARG_STRING, &forceMinTime_CSTR, 0,
                  "force min time to be given date string index", "" },

                { "force-min-time-relative", 0, POPT_ARG_STRING, &forceMinTimeRel_CSTR, 0,
                  "force min time to be given date string index", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        gtk_init( &argc, &argv );
        
        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
        
        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
        string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
        string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );

        if( forceMinTime_CSTR || forceMinTimeRel_CSTR )
            isTimeEA = true;
        
        string ffilterComparitor = "<=";
        if( ReverseOrder )
            ffilterComparitor = ">=";

        stringstream conSS;
        conSS << " host=" << host;
        conSS << " dbname=" << dbname;
        connection con( conSS.str() );
        work trans( con, "getting the schema..." );

        const char* CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;

        make_window();
        
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            attrname = RootNameCSTR;

            string ValueTableName = guessLookupTableName( trans, attrname );
            if( OutputDouble )
                ValueTableName = "doublelookup";
            
            fh_stringstream selectss;
            string selectAttrName = Util::replace_all( attrname, "-", "_" );
            string fromClause = " from docmap ";
            string whereClauseBegining = " where ";

            stringmap_t m_ExtraColumnsToInlineInDocmap;
            m_ExtraColumnsToInlineInDocmap =
                Util::ParseKeyValueString(
                    idx->getConfig( CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                                    CFG_POSTGRESQLIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                    );
            cerr << "m_ExtraColumnsToInlineInDocmap.size:" << m_ExtraColumnsToInlineInDocmap.size() << endl;
            
            bool isNormalized
                = m_ExtraColumnsToInlineInDocmap.find( attrname ) == m_ExtraColumnsToInlineInDocmap.end();
            
            if( isNormalized )
            {
                selectAttrName = "attrvalue";
                stringstream ss;
//                 ss << " from docattrs da, "
//                    << ValueTableName << " l, attrmap an " << endl
//                    << " where an.attrname = '" << attrname
//                    << "' and an.attrid = da.attrid and da.vid = l.vid ";
                ss << " from docattrs da, "
                   << ValueTableName << " l " << endl
                   << " where da.attrid = "
                   << "    (select max(attrid) from attrmap where attrname = '" << attrname << "') "
                   << "  and da.vid = l.vid ";
                fromClause = ss.str();
                whereClauseBegining = " and ";
            }

            string selectAttrNameNumeric = selectAttrName;
            if( isTimeEA )
            {
                stringstream ss;
                ss << "EXTRACT(EPOCH FROM " << selectAttrName << " )";
                selectAttrNameNumeric = ss.str();
            }
            
            selectss << "select max(" << selectAttrNameNumeric << ") as v,"
                     << "count(" << selectAttrName << ") as count "
                     << fromClause
                     << whereClauseBegining << selectAttrNameNumeric << " <> 0 "
                     << " group by " << selectAttrName << " order by  " << selectAttrName << " ";
            cerr << "SQL:" << selectss.str() << endl;
            result res = trans.exec( tostr( selectss ) );
            int res_sz = res.size();
            int point_count = res_sz;
            
            gdouble* px=new gdouble[res_sz+1];
            gdouble* py=new gdouble[res_sz+1];
            gdouble* dx=new gdouble[res_sz+1];
            gdouble* dy=new gdouble[res_sz+1];

            gdouble x_min = 0, x_max = 0, y_min = 0, y_max = 0;
            int idx = 0;
            for (result::const_iterator c = res.begin(); c != res.end(); ++c, ++idx)
            {
                gdouble v = 0, count = 0;
                c[0].to(v);
                c[1].to(count);
                
                px[ idx ] = v;
                py[ idx ] = count;
                dx[ idx ] = 0;
                dy[ idx ] = 0;

                if( !idx )
                {
                    x_min = x_max = v;
                    y_min = y_max = count;
                }
                else
                {
                    x_min = MIN( v, x_min );
                    x_max = MAX( v, x_max );
                    y_min = MIN( count, y_min );
                    y_max = MAX( count, y_max );
                }
            }

//            x_max = 4096;
            cerr << " x_min:" << x_min
                 << " x_max:" << x_max
                 << " y_min:" << y_min
                 << " y_max:" << y_max
                 << endl;
            GtkWidget *plot = active_plot;

            gtk_plot_set_ticks(GTK_PLOT(plot), GTK_PLOT_AXIS_X, (x_max-x_min)/5.0, 4);
            gtk_plot_set_ticks(GTK_PLOT(plot), GTK_PLOT_AXIS_Y, (y_max-y_min)/10.0, 4);
            gtk_plot_set_range(GTK_PLOT(plot), x_min, x_max, y_min, y_max );
//             gtk_plot_set_xscale( GTK_PLOT(plot), GTK_PLOT_SCALE_LINEAR );
//             gtk_plot_set_yscale( GTK_PLOT(plot), GTK_PLOT_SCALE_LOG10 );
            gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), "");
            gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_LEFT), "count");
            gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), attrname.c_str() );

            if( isTimeEA )
            {
                time_t min_time = (time_t)x_min;
                time_t max_time = (time_t)x_max;

                if( forceMinTime_CSTR )
                {
                    struct tm tm = Time::ParseTimeString( forceMinTime_CSTR );
                    min_time = mktime( &tm );
                }
                if( forceMinTimeRel_CSTR )
                {
                    min_time = Time::ParseRelativeTimeString( forceMinTimeRel_CSTR );
                }
                
                if( min_time < 0 )
                    min_time = 0;
                
                cerr << " min_time:" << (long)( min_time )
                     << " max_time:" << (long)( max_time )
                     << endl;
                cerr << " min_time:" << Time::toTimeString( min_time )
                     << " max_time:" << Time::toTimeString( max_time )
                     << endl;

//                static gchar* custom_labels[] = {"FIXME Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
                GtkPlotArray *array;
                static gchar* custom_labels[13];
                int custom_labels_sz = 12;
                string format = "%b_%y";

                time_t diff_time = max_time - min_time;
                if( diff_time < (3600*24*365) )
                {
                    format = "%d_%b";
                }
                if( diff_time < (3600*24*31) )
                {
                    format = "%d_%a";
                }
                if( diff_time < (3600*24) )
                {
                    format = "%R";
                }
                
                for( long i = 0; i < custom_labels_sz; ++i )
                {
                    long long offset = diff_time/(custom_labels_sz-1);
                    offset *= i;
                    time_t tt = min_time + offset;
                    string tv = Time::toTimeString( tt, format );
                    custom_labels[i] = strdup( tv.c_str() );
                    cerr << " min_time:" << min_time << " dtdiv:" << (diff_time/custom_labels_sz)
                         << " offset:" << offset
                         << " (i*diff_time/custom_labels_sz):" << (i*diff_time/custom_labels_sz)
                         << " i:" << i << " tt:" << tt << " tv:" << tv << endl;
                }
                
                array = GTK_PLOT_ARRAY(gtk_plot_array_new(NULL, custom_labels, custom_labels_sz, GTK_TYPE_STRING, FALSE)); 
                gtk_plot_axis_set_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), array); 
                gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), TRUE);
                gtk_plot_axis_set_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_TOP), array); 
                gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_TOP), TRUE);
                gtk_plot_set_ticks(GTK_PLOT(plot), GTK_PLOT_AXIS_X, (x_max-x_min)/12.0, 4);
                
                {
                    stringstream ss;
                    ss << attrname
                       << " - \\Bmin\\N:" << Time::toTimeString( min_time )
                       << " - \\Bmax\\N:" << Time::toTimeString( max_time );
                    gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_BOTTOM), ss.str().c_str() );
                }
            }
            
            
            GdkColor color;
            GdkColor color2;
            GtkPlotAxis *axis;

            dataset[0] = GTK_PLOT_DATA(gtk_plot_data_new());
            gtk_plot_add_data(GTK_PLOT(plot), dataset[0]);
            gtk_widget_show(GTK_WIDGET(dataset[0]));
            gtk_plot_data_set_points(dataset[0], px, py, dx, dy, point_count );

//          gtk_plot_data_add_marker(dataset[0], 4);

            gdk_color_parse("red", &color);
            gdk_color_parse("blue", &color2);
            gdk_color_alloc(gdk_colormap_get_system(), &color); 
            gdk_color_alloc(gdk_colormap_get_system(), &color2); 

            gtk_plot_data_set_symbol(dataset[0],
                                     GTK_PLOT_SYMBOL_CIRCLE,
                                     GTK_PLOT_SYMBOL_OPAQUE,
                                     10, 2, &color, &color);
            gtk_plot_data_set_line_attributes(dataset[0],
                                              GTK_PLOT_LINE_SOLID,
                                              (GdkCapStyle)0, (GdkJoinStyle)0, 1, &color2);
 
//            gtk_plot_data_set_connector(dataset[0], GTK_PLOT_CONNECT_SPLINE);
            gtk_plot_data_set_connector(dataset[0], GTK_PLOT_CONNECT_NONE);

            gtk_plot_data_show_yerrbars(dataset[0]);
            {
                stringstream ss;
                ss << attrname;
                gtk_plot_data_set_legend(dataset[0], ss.str().c_str() );
            }
        }

        gtk_widget_show_all( gtk_window );
        gtk_main();

        if( !SelectedValues.empty() )
        {
            SelectedValues_t::iterator si = SelectedValues.begin();
            string an = si->first;
            string v = an.substr( an.find("_")+1 );
            updateSelectedValues( si->first, v, ExcludeZero ? "0" : "" );
        }
        
        for( SelectedValues_t::const_iterator si = SelectedValues.begin();
             si != SelectedValues.end(); ++si )
        {
            string an = si->first;
            string ffilter = si->second;
            
            cout << an << " '" << ffilter << "' " << endl;
        }
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


        
