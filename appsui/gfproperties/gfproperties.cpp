/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris properties GTK2 client
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

    $Id: gfproperties.cpp,v 1.4 2011/06/18 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <Ferris/Ferris.hh>
#include <FerrisUI/All.hh>

#include <popt.h>

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

const string PROGRAM_NAME = "gfproperties";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int window_count = 0;

static void close_the_window( GtkButton *button, gpointer user_data )
{
    GtkWindow* w = GTK_WINDOW( user_data );
    gtk_widget_destroy( GTK_WIDGET(w) );

    if( --window_count == 0 )
    {
        exit(0);
    }
}

gboolean close_a_window (GtkObject *object, gpointer udata)
{
    if( --window_count == 0 )
    {
        exit(0);
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, char** argv )
{
    const char* DstNameCSTR              = 0;
    struct poptOption optionsTable[] =
        {
//             { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
//               "Specify destination explicity, all remaining URLs are assumed to be source files",
//               "DIR" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2");

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//                 switch (c) {
//                 }
    }

    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }

    gtk_init( &argc, &argv );
    
    try
    {
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string filename = RootNameCSTR;

            fh_context c;
            
            try
            {
                c = Resolve( filename );
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "Can not resolve given URL:" << filename
                   << "reason:" << e.what();
                RunErrorDialog( tostr(ss) );
                continue;
            }
            

            GtkWidget* win;
            GtkWidget* vbx;
            GtkWidget* hbx;
            GtkWidget* b;
            ContextPropertiesEditor* cpe = new ContextPropertiesEditor();
            cpe->setShowURL( true );

            win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_signal_connect(GTK_OBJECT (win), "destroy",
                               GTK_SIGNAL_FUNC(close_a_window), win );
            
            gtk_window_set_title( GTK_WINDOW(win), "Edit context" );

            vbx = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());

            ////////////////////////////////
            gtk_notebook_append_page( notebook,
                                      cpe->getWidget(),
                                      gtk_label_new( "properties" ));
            cpe->setContext( c );

            ////////////////////////////////

            static fh_medallionEditor me = new MedallionEditor();
            me->setShowDescription( true );
            me->setShowFuzzyHints( true );
            me->setContext( c );
            gtk_notebook_append_page( notebook,
                                      me->getWidget(),
                                      gtk_label_new( "medallion" ));
            
            ////////////////////////////////
            gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(notebook),  1, 1, 0 );
            hbx = GTK_WIDGET(gtk_hbox_new(0,0));
            b = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
            gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
            gtk_signal_connect(GTK_OBJECT (b), "clicked",
                               GTK_SIGNAL_FUNC(close_the_window), win );
            
            gtk_box_pack_start(GTK_BOX(vbx), hbx,  0, 0, 0 );
            
            
            gtk_container_add(GTK_CONTAINER(win), vbx);
            gtk_widget_show_all(win);
            ++window_count;
        }
        gtk_main();
    }
    catch( NoSuchContextClass& e )
    {
        fh_stringstream ss;
        ss << "Invalid context class given e:" << e.what() << endl;
        cerr << tostr(ss);
        RunErrorDialog( tostr(ss) );
        exit(1);
    }
    catch( exception& e )
    {
        fh_stringstream ss;
        ss << "Error: " << e.what() << endl;
        cerr << tostr(ss);
        RunErrorDialog( tostr(ss) );
        exit(1);
    }
    poptFreeContext(optCon);
    return 0;
}
 


