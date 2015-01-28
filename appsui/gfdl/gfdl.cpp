/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris download GTK2 client
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

    $Id: gfdl.cpp,v 1.5 2011/06/18 21:30:23 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>

#include <algorithm>
#include <iomanip>

#include <Ferris/Ferris.hh>
#include <Ferris/ChildStreamServer.hh>
#include <FerrisUI/GtkFerris.hh>

#include <glib.h>
#include <gtk/gtk.h>

#include <unistd.h>

#include "config.h"

using namespace std;
using namespace Ferris;
using namespace FerrisUI;

const string PROGRAM_NAME = "gfdl";

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


GtkWidget*    gtk_window = 0;
GtkLabel*       w_src = 0;
GtkLabel*       w_dst = 0;
GtkProgressBar* w_progress = 0;
unsigned long ArtificialDelay = 0;

fh_childserv serv;
streamsize totalsz = 0;
streamsize donesz = 0;
fh_context src;

void make_window()
{
    GtkWidget* page;
    GtkWidget* lab;
    GtkWidget* slide;
    GtkWidget* w;
    GtkWidget* b;
    int r=-1;
    int c=0;
    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title ( GTK_WINDOW( gtk_window ), "gfdl async io test" );
    gtk_window_set_default_size (GTK_WINDOW (gtk_window), 450, 250);
    gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);

    page = gtk_table_new ( 1, 2, false );

    ++r; c=0;
    w = GTK_WIDGET( gtk_label_new("src:") );
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
    ++c;
    w = GTK_WIDGET( gtk_label_new("x") );
    w_src = GTK_LABEL(w);
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );

    ++r; c=0;
    w = GTK_WIDGET( gtk_label_new("dst:") );
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
    ++c;
    w = GTK_WIDGET( gtk_label_new("x") );
    w_dst = GTK_LABEL(w);
    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );

    ++r; c=0;
    w_progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
    gtk_progress_bar_set_orientation( w_progress, GTK_PROGRESS_LEFT_TO_RIGHT );
    gtk_progress_bar_set_text( w_progress, "." );
    gtk_progress_set_percentage( GTK_PROGRESS(w_progress), 0 );
    gtk_table_attach_defaults(GTK_TABLE(page), GTK_WIDGET(w_progress), c, c+2, r, r+1 );
    
    gtk_container_add(GTK_CONTAINER(gtk_window), page );
}

fh_istream async_io_cb( fh_runner r, fh_istream iss )
{
    if( !totalsz )
    {
        totalsz = toType<streamsize>(getStrAttr( src, "size", "-1" ));
    }

    cerr << "totalsz:" << totalsz << " donesz:" << donesz << endl;

    fh_stringstream ss;
    copy( istreambuf_iterator<char>(iss),
          istreambuf_iterator<char>(),
          ostreambuf_iterator<char>(ss));

    donesz += tostr(ss).length();
    cerr << tostr(ss) << endl;
    
    if( totalsz > 0 )
    {
        double d = donesz;
        d /= totalsz;
        gtk_progress_set_percentage( GTK_PROGRESS(w_progress), d );
        fh_stringstream ss;
        ss << donesz << " / " << totalsz;
        gtk_progress_bar_set_text(   GTK_PROGRESS_BAR(w_progress), tostr(ss).c_str() );
    }

    if( ArtificialDelay )
    {
        sleep( 1 );
        while (gtk_events_pending ())
            gtk_main_iteration ();
    }
    
    return iss;
}



int main( int argc, char** argv )
{
    serv = new ChildStreamServer();
    
    const char* cdummy            = 0;
    const char* DstNameCSTR       = 0;
    unsigned long ShowVersion = 0;

    struct poptOption optionsTable[] = {

        { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
          "Specify destination explicity, all remaining URLs are assumed to be source files", 0 },

        { "artificial-delay", 'd', POPT_ARG_NONE, &ArtificialDelay, 0,
          "Slow down the process so that the progress meter can be seen on local tests", 0 },

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },

        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;

    gtk_init( &argc, &argv );
    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");
    if (argc < 1) {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {}

    if( ShowVersion )
    {
        cout << "gfdl version: $Id: gfdl.cpp,v 1.5 2011/06/18 21:30:23 ben Exp $\n"
             << "ferris   version: " << VERSION << nl
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    try
    {
        make_window();

        typedef vector<string> srcs_t;
        srcs_t srcs;

        cerr << "gfdl getting file names" << endl;
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            cerr << "Adding potential source:" << RootName << endl;
            srcs.push_back( RootName );
        }

        if( srcs.empty() )
        {
            cerr << "No objects need to be downloaded\n" << endl;
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
            
        /*
         * Setup the destination from either explicit command line or last arg
         */
        string DstName;
        if( DstNameCSTR )
        {
            DstName = DstNameCSTR;
        }
        else
        {
            DstName = srcs.back();
            srcs.pop_back();
        }
                
        {
            fh_stringstream ss;
            ss << "dst: " << DstName;
            gtk_label_set_text( GTK_LABEL(w_dst), g_strdup( tostr(ss).c_str() ) );
        }

            
        for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
        {
            string SrcName = *iter;
            cerr << "src:" << SrcName << endl;

            gtk_label_set_text( GTK_LABEL(w_src), g_strdup( SrcName.c_str() ) );
            
            fh_runner r = new Runner();
            src = Resolve( SrcName );
            string command = "fcat " + SrcName;
            r->setCommandLine( command );
            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                           | G_SPAWN_STDERR_TO_DEV_NULL
                                           | G_SPAWN_SEARCH_PATH) );
            cerr << "Using command:" << command << endl;
            r->setAsyncStdOutFunctor( async_io_cb );
            serv->addChild( r );
            r->Run();            
            break;
        }

        gtk_widget_show_all( gtk_window );

        gtk_main();
    }
    catch( exception& e )
    {
        cerr << "gfdl cought:" << e.what() << endl;
        exit(1);
    }
    poptFreeContext(optCon);
    return 0;
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
