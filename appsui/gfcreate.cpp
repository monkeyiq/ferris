/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of FerrisCreate.

    FerrisCreate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FerrisCreate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FerrisCreate.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: gfcreate.cpp,v 1.7 2011/06/18 21:38:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define HAVE_GTK2 1


#include <gmodule.h>

// create table newtable (age int, name char(5))
#include "config.h"
#include "ferriscreatecommon.hh"
#include "gfcreate.hh"

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <popt.h>

#include <sys/time.h>


#include <Ferris/Runner.hh>
#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisUI/GtkFerris.hh>
#include <FerrisUI/EditStringList.hh>
#include <FerrisUI/EditSQLColumns.hh>

#include <string>
using namespace std;
using namespace Ferris;
using namespace FerrisUI;
using namespace Ferris::Factory;


const string PROGRAM_NAME = "gfcreate";

unsigned long Sloth              = 0;

GtkWindow* win;
GtkWidget* okb;
GtkWidget* canb;

FerrisCreate fc;
GladeXML *xml;

static fh_context theContext;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void setPage_cb( GtkWidget *w, gpointer udata )
{
    string* n = (string*)udata;
    const string& k = *n;

    if( GTK_IS_NOTEBOOK( w ))
    {
        GList* l    = gtk_container_get_children( GTK_CONTAINER(w) );
        guint lsize = g_list_length( l );
        
        for( int i=0; i<lsize; ++i )
        {
            gpointer child = g_list_nth_data( l, i );
            string wname = gtk_widget_get_name( GTK_WIDGET( child ) );

            if( wname == k )
            {
                gtk_notebook_set_current_page( GTK_NOTEBOOK(w), i );
                
                for( GtkWidget* p = w ; p ; )
                {
                    GtkWidget* nextp = gtk_widget_get_parent( p );
                    if( GTK_IS_NOTEBOOK( nextp ) )
                    {
                        gint pn = gtk_notebook_page_num( GTK_NOTEBOOK(nextp), p );
                        gtk_notebook_set_current_page(   GTK_NOTEBOOK(nextp), pn );
                    }
                    p = nextp;
                }
                return;
            }
        }
    }
    if( GTK_IS_CONTAINER( w ))
    {
        gtk_container_foreach( GTK_CONTAINER(w), GtkCallback(setPage_cb), n );
    }
}

void setPage( GtkWidget* w, string n )
{
    gtk_container_foreach( GTK_CONTAINER(w), GtkCallback(setPage_cb), &n );
}


void setEntryProperty_cb( GtkWidget *w, gpointer udata )
{
    pair< string, string >* p = (pair< string, string >*) udata;

    if( GTK_IS_ENTRY( w ) && p->first == gtk_widget_get_name(w) )
    {
        gtk_entry_set_text( GTK_ENTRY(w), p->second.c_str() );
    }
    else if( GTK_IS_CONTAINER( w ))
    {
        gtk_container_foreach( GTK_CONTAINER(w), GtkCallback(setEntryProperty_cb), p );
    }
}

void setEntryProperty( GtkWidget* w, string k, string v )
{
    pair< string, string > p = make_pair( k, v );
    gtk_container_foreach( GTK_CONTAINER(w), GtkCallback(setEntryProperty_cb), &p );
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void game_over()
{
    theContext = 0;
    gtk_main_quit();
}


void can_clicked_cb( GtkWidget* w, gpointer data )
{
    game_over();
}


typedef list< fh_editstringlist > stringlist_cache_t;
static stringlist_cache_t stringlist_cache;

typedef list< fh_editsqlcolumns > editsqlcolumns_cache_t;
static editsqlcolumns_cache_t editsqlcolumns_cache;

void stringlist_toentry_fe( GtkWidget *w, gpointer udata )
{
    if( !w )
        return;
    
    for( stringlist_cache_t::iterator ci = stringlist_cache.begin();
         ci != stringlist_cache.end(); ++ci )
    {
        if( (*ci)->getWidget() == GTK_WIDGET( w ) )
        {
            stringlist_t sl = (*ci)->getStringList();

            GtkWidget* e = gtk_entry_new();
            string sltext = Util::createCommaSeperatedList( sl );
            gtk_entry_set_text( GTK_ENTRY( e ), sltext.c_str() );

            gtk_widget_set_name( e, gtk_widget_get_name( GTK_WIDGET( w ) ));
            
            GtkWidget* parent = gtk_widget_get_parent(w);
            gtk_container_remove(GTK_CONTAINER(parent), w );
            gtk_container_add(GTK_CONTAINER(parent), e );
            return;
        }
    }

    for( editsqlcolumns_cache_t::iterator ci = editsqlcolumns_cache.begin();
         ci != editsqlcolumns_cache.end(); ++ci )
    {
        if( (*ci)->getWidget() == GTK_WIDGET( w ) )
        {
            stringmap_t sm = (*ci)->getStringMap();
            string sltext = Util::CreateKeyValueString( sm, "=", ":" );
                
            GtkWidget* e = gtk_entry_new();
            gtk_entry_set_text( GTK_ENTRY( e ), sltext.c_str() );

            gtk_widget_set_name( e, gtk_widget_get_name( GTK_WIDGET( w ) ));
            
            GtkWidget* parent = gtk_widget_get_parent(w);
            gtk_container_remove(GTK_CONTAINER(parent), w );
            gtk_container_add(GTK_CONTAINER(parent), e );
            return;
        }
    }
    
    if( GTK_IS_CONTAINER(w) )
        gtk_container_foreach ( GTK_CONTAINER(w), stringlist_toentry_fe, 0 );
}

void ok_clicked_cb( GtkWidget* w, gpointer data )
{
    try
    {
        gtk_container_foreach ( GTK_CONTAINER(win), stringlist_toentry_fe, 0 );
        stringlist_cache.clear();
        
        LG_FERRISCREATE_D << "ok_clicked_cb() " << endl;
        fh_istream ss = fc.createDocument( xml, win );

        LG_FERRISCREATE_D << "Creation url:" << theContext->getURL() << endl;

        string fakeMountName = "/tmp/ferris_mount_this.xml";
        {
            fh_ofstream oss( fakeMountName );
            std::copy( std::istreambuf_iterator<char>(ss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(oss));
        }
        
        fh_context md   = Resolve( fakeMountName );
        cerr << "Creating new object in context:" << theContext->getURL() << endl;
        fh_context newc = theContext->createSubContext( "", md );
        cerr << "New context is:" << newc->getURL() << endl;
        LG_FERRISCREATE_D << "newc:" << newc->getURL() << endl;
    }
    catch( exception& e )
    {
        LG_FERRISCREATE_D << "Error:" << e.what() << endl;
        cerr << "e:" << e.what() << endl;

        if( Sloth )
        {
            gtk_widget_show_all(GTK_WIDGET( win ));
        }

        fh_stringstream ss;
        ss << "Error creating object:" << e.what() << endl;
        RunErrorDialog( tostr(ss) );
        return;
    }
    
    game_over();
}


void fixopm_fe ( GtkWidget *w, gpointer udata )
{
    if( !w )
        return;
    
    if( GTK_IS_ENTRY(w) )
    {
        fh_stringstream ss;
        ss << gtk_entry_get_text ( GTK_ENTRY(w) );
        string s;

        getline(ss,s,',');
        if( starts_with( s, "MAKE_INTO_OPMENU" ))
        {
            GtkWidget* opm   = gtk_option_menu_new();
            GtkWidget* m     = gtk_menu_new ();
            GtkWidget* child = 0;

            gtk_widget_set_name( opm, gtk_widget_get_name( GTK_WIDGET( w ) ));
            
            while( getline(ss,s,','))
            {
                child = gtk_menu_item_new_with_label( s.c_str() );
                gtk_menu_shell_append( GTK_MENU_SHELL(m), child );
            }

            gtk_option_menu_set_menu ( GTK_OPTION_MENU(opm), m);
            GtkWidget* parent = gtk_widget_get_parent(w);
            gtk_container_remove(GTK_CONTAINER(parent), w );
            gtk_container_add(GTK_CONTAINER(parent), opm );
        }
        return;
    }

    if( GTK_IS_CONTAINER(w) )
        gtk_container_foreach ( GTK_CONTAINER(w), fixopm_fe, 0 );
}

void fixstringlist_fe ( GtkWidget *w, gpointer udata )
{
    if( GTK_IS_ENTRY(w) )
    {
        fh_stringstream ss;
        ss << gtk_entry_get_text ( GTK_ENTRY(w) );
        string s;

        getline(ss,s,',');
        if( starts_with( s, "MAKE_INTO_STRINGLIST" ))
        {
            fh_editstringlist esl = new EditStringList();
            stringlist_cache.push_back( esl );
            
            esl->setDescriptionLabel("");
            esl->setColumnLabel("string");

            stringlist_t sl;
            while( getline(ss,s,','))
            {
                sl.push_back( s );
            }

            GtkWidget* eslw = esl->getWidget();
            gtk_widget_set_name( eslw, gtk_widget_get_name( GTK_WIDGET( w ) ));
            GtkWidget* parent = gtk_widget_get_parent(w);
            gtk_container_remove(GTK_CONTAINER(parent), w );
            gtk_container_add(GTK_CONTAINER(parent), eslw );
//            gtk_widget_show_all(GTK_WIDGET( eslw ));

            esl->setStringList( sl );
//             esl->appendNewBlankItem();
            
        }
        if( starts_with( s, "MAKE_INTO_EDITSQLCOLUMNS" ))
        {
            fh_editsqlcolumns sqledit = new EditSQLColumns();
            editsqlcolumns_cache.push_back( sqledit );
            
            sqledit->setDescriptionLabel("");
            
            cerr << "make_into_editsqlcols. ss:" << tostr(ss) << endl;
            stringmap_t sm = Util::ParseKeyValueString( ss );

            GtkWidget* sqleditw = sqledit->getWidget();
            gtk_widget_set_name( sqleditw, gtk_widget_get_name( GTK_WIDGET( w ) ));
            GtkWidget* parent = gtk_widget_get_parent(w);
            gtk_container_remove(GTK_CONTAINER(parent), w );
            gtk_container_add(GTK_CONTAINER(parent), sqleditw );
//            gtk_widget_show_all(GTK_WIDGET( sqleditw ));

            sqledit->setStringMap( sm );
//             sqledit->appendNewBlankItem();
            
        }
        return;
    }
    if( GTK_IS_CONTAINER(w) )
        gtk_container_foreach ( GTK_CONTAINER(w), fixstringlist_fe, 0 );
}


void fixnb_fe ( GtkWidget *w, gpointer udata )
{
    try
    {
        
    if( GTK_IS_NOTEBOOK(w) )
    {
        GList* l    = gtk_container_get_children( GTK_CONTAINER(w) );
        guint lsize = g_list_length( l );
        
        for( int i=0; i<lsize; ++i )
        {
            gpointer child = g_list_nth_data( l, i );
            const gchar* name = gtk_widget_get_name( GTK_WIDGET( child ) );
            string iconname = "unknown";

            try
            {
            
                fh_stringstream ss;

                string majort = "";
                string minort = "";

                if( !strcmp(name,"dir") || !strcmp(name,"file") || !strcmp(name,"ea") )
                {
                    minort = name;
                }
                if( !strcmp(name,"png")
                    || !strcmp(name,"gif") || !strcmp(name,"jpeg")
                    || !strcmp(name,"bmp") || !strcmp(name,"fax")
                    || !strcmp(name,"fits") || !strcmp(name,"pcx")
                    || !strcmp(name,"pnm") || !strcmp(name,"sgi")
                    || !strcmp(name,"sun") || !strcmp(name,"tga")
                    || !strcmp(name,"tiff")
                    )
                {
                    majort = "image";
                    minort = name;
                }
                if( !strcmp(name,"cpp") || !strcmp(name,"hh")
                    || !strcmp(name,"c") || !strcmp(name,"h")
                    || !strcmp(name,"html") || !strcmp(name,"sgml")
                    || !strcmp(name,"xml") || !strcmp(name,"txt")
                    || !strcmp(name,"sh") || !strcmp(name,"csh")
                    )
                {
                    majort = "text";
                    minort = name;
                }
                else if( !strcmp(name,"mp3") || !strcmp(name,"wav") || !strcmp(name,"ogg") )
                {
                    majort = "audio";
                    minort = name;
                }
                else if( !strcmp(name,"mpeg2") || !strcmp(name,"mng") )
                {
                    majort = "video";
                    minort = name;
                }
            
                if( minort.length() )
                {
                    fh_stringstream ss;
                    ss << "mime://" << majort << "/" << minort;
                    fh_context c = Resolve( tostr(ss) );
                    iconname = getStrAttr( c, "ferris-iconname", "" );
                    LG_FERRISCREATE_D << "icon:" << iconname << endl;
                }
                if( !strcmp(name,"table") || !strcmp(name,"queryview") || !strcmp(name,"tuple") )
                {
                    ss << "icons://" << name << ".png";
                    fh_context c = Resolve( tostr(ss) );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"fulltextindexnull")
                         || !strcmp(name,"eaindexnull")
                         || !strcmp(name,"fulltextindexuniqsorted"))
                {
                    ss << "icons://" << "debug" << ".png";
                    fh_context c = Resolve( tostr(ss) );
                    iconname = c->getDirPath();
                }
                else if( starts_with( name, "eaindex" )
                         || starts_with( name, "fulltextindex" ) )
                {
                    try
                    {
                        ss << "icons://" << name << ".png";
                        fh_context c = Resolve( tostr(ss) );
                        iconname = c->getDirPath();
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "icons://eaindex.png";
                        fh_context c = Resolve( tostr(ss) );
                        iconname = c->getDirPath();
                    }
                }
                else if( !strcmp(name,"socket")
                         || !strcmp(name,"serversocket")
                         || !strcmp(name,"clientsocket")
                         || !strcmp(name,"hardlink")
                         || !strcmp(name,"softlink")
                         || !strcmp(name,"special") )
                {
                    ss << "icons://" << name << ".png";
                    fh_context c = Resolve( tostr(ss) );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"_image") )
                {
                    ++name;
                    fh_context c = Resolve( "icons://image.png" );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"_text") )
                {
                    ++name;
                    fh_context c = Resolve( "icons://text.png" );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"_misc") )
                {
                    ++name;
                    fh_context c = Resolve( "icons://misc.png" );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"_audio") )
                {
                    ++name;
                    fh_context c = Resolve( "icons://audio.png" );
                    iconname = c->getDirPath();
                }
                else if( !strcmp(name,"_video") )
                {
                    ++name;
                    fh_context c = Resolve( "icons://video.png" );
                    iconname = c->getDirPath();
                }
            

                if( iconname.length() )
                {
                    fh_context tmp = Resolve( iconname );
                    string realiconname = getStrAttr( tmp, "path", "" );
                    if( !realiconname.length() )
                    {
                        realiconname = iconname;
                    }
                    
                
                    LG_FERRISCREATE_D << "setting name:" << name << " to icon:" << realiconname << endl;
                    cerr << "setting name:" << name << " to icon:" << realiconname << endl;
                    GtkContainer* vbx = GTK_CONTAINER(gtk_hbox_new(0,5));
                    gtk_container_add( vbx, gtk_image_new_from_file ( realiconname.c_str()));
//                 if( !strcmp(name,"dir") || !strcmp(name,"file") 
//                     || !strcmp(name,"c") || !strcmp(name,"h") 
//                     || !strcmp(name,"cpp") || !strcmp(name,"hh")
//                     )
                    {
                        gtk_container_add( vbx, GTK_WIDGET( gtk_label_new( name ) ));
                    }
                
//                GtkWidget* l = gtk_label_new("xx");
//                GtkWidget* l = gtk_button_new_with_label( "xx" );
//                GtkWidget* l = gtk_image_new_from_file ( realiconname.c_str());

                    gtk_notebook_set_tab_label( GTK_NOTEBOOK(w),
                                                GTK_WIDGET( child ),
                                                GTK_WIDGET( vbx ));
                    gtk_widget_show_all(GTK_WIDGET( vbx ));
                }
                else
                {
                    gtk_notebook_set_tab_label( GTK_NOTEBOOK(w),
                                                GTK_WIDGET( child ),
                                                gtk_label_new( name ));
                }
            }
        
            catch( exception& e )
            {
                cerr << "failed to load icon:" << iconname << endl;
                gtk_notebook_set_tab_label( GTK_NOTEBOOK(w),
                                            GTK_WIDGET( child ),
                                            gtk_label_new( name ));
            }
        }

        gtk_notebook_popup_enable( GTK_NOTEBOOK(w) );
    }

    if( GTK_IS_CONTAINER(w) )
        gtk_container_foreach ( GTK_CONTAINER(w), fixnb_fe, 0 );
    
    }
    catch( exception& e )
    {
        LG_FERRISCREATE_D << "fixnb_fe e:" << e.what() << endl;
        cerr << "fixnb_fe e:" << e.what() << endl;
    }
    
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

static gint click_button( gpointer user_data )
{
    GtkButton* b = GTK_BUTTON( user_data );
    gtk_button_clicked( GTK_BUTTON(b) );
    return 0; // dont call again
}


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

int main( int argc, const char** argv )
{
    const char* SchemaToGladeXSLTFileName    = getSchemaToGladeXSLTFileNameDefault();
    const char* GladeToDocumentXSLTFileName  = getGladeToDocumentXSLTFileNameDefault();
    const char* CreateTypeName              = 0;
    const char* CreateRdn                   = 0;
    const char* TargetPath                  = 0;
    unsigned long cix = 1;
    unsigned long AutoRun            = 0;
    unsigned long ShowVersion        = 0;
    unsigned long DumpFerrisXSD      = 0;
    unsigned long DumpToGladeXML     = 0;
    unsigned long DumpFromGladeXML   = 0;
    unsigned long DumpToFerrisXML    = 0;
    unsigned long DontReadRootContext= 0;

    struct poptOption optionsTable[] = {

        { "auto-run", 'f', POPT_ARG_NONE, &AutoRun, 0,
          "Automatically try to create the object without the user hitting ok", 0 },

        { "sloth", 0, POPT_ARG_NONE, &Sloth, 0,
          "Only open GUI if needed", 0 },
        
        { "schema-to-glade-xsl", 0, POPT_ARG_STRING, &SchemaToGladeXSLTFileName, 0,
          "xslt file to parse DTD schema to a glade 2.0 xml file", 0 },

        { "glade-to-document-xsl", 0, POPT_ARG_STRING, &GladeToDocumentXSLTFileName, 0,
          "xslt file to parse glade 2.0 xml into a new xml document", 0 },

        { "create-type", 0, POPT_ARG_STRING, &CreateTypeName, 0,
          "name of object type to create", 0 },

        { "target-path", 0, POPT_ARG_STRING, &TargetPath, 0,
          "specify the target for the new object seperately from k=v pairs", 0 },
        
        { "rdn", 0, POPT_ARG_STRING, &CreateRdn, 0,
          "rdn of object to create", 0 },
        
        { "dump-ferris-xsd", '1', POPT_ARG_NONE, &DumpFerrisXSD, 0,
          "Dump the incomming ferris xsd file " DumpFerrisXSDFileName, 0 },

        { "dump-to-libglade-xml", '2', POPT_ARG_NONE, &DumpToGladeXML, 0,
          "Dump the file that is used to make the UI to " DumpToGladeXMLFileName, 0 },

        { "dump-from-libglade-xml", '3', POPT_ARG_NONE, &DumpFromGladeXML, 0,
          "Dump the file that made from the UI to " DumpFromGladeXMLFileName, 0 },

        { "dump-to-ferris-xml", '4', POPT_ARG_NONE, &DumpToFerrisXML, 0,
          "Dump the file that is passed back to ferris to " DumpToFerrisXMLFileName, 0 },

        { "dont-read-root-context", 'x', POPT_ARG_NONE, &DontReadRootContext, 0,
          "Normally if the initial root context is seen to have children it is read to allow proper"
          " ea creation by the overmounting context. This option disables reading the root context",
          0 },
        
        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        FERRIS_POPT_OPTIONS
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    /* Now do options processing */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << "fcreate version: $Id: gfcreate.cpp,v 1.7 2011/06/18 21:38:27 ben Exp $\n"
             << "release      version: " << VERSION << endl
             << "Written by Ben Martin, aka monkeyiq" << endl
             << endl
             << "Copyright (C) 2001 Ben Martin" << endl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    
    /*
     * Make sure that this user is setup.
     */
    ensureDotFilesCreated( SchemaToGladeXSLTFileName, GladeToDocumentXSLTFileName );
    
    ensureXMLPlatformInitialized();
    //    XALAN_CPP_NAMESPACE::XalanTransformer::initialize();
    gtk_init( &argc, (char***)&argv );
    glade_init();

    try
    {
        stringlist_t earls;
        for( const char* t = ""; t ; )
        {
            t = poptGetArg(optCon);
            if( !t )
            {
                break;
            }

            earls.push_back(t);
        }

        if( TargetPath )
            earls.push_back( TargetPath );


        for( stringlist_t::iterator sliter = earls.begin(); sliter!=earls.end(); ++sliter )
        {
            string RootName = *sliter;
            fc.setSchemaToGladeXSLTFileName( SchemaToGladeXSLTFileName );
            fc.setGladeToDocumentXSLTFileName( GladeToDocumentXSLTFileName );
            
            fc.setDumpFerrisXSD( DumpFerrisXSD );
            fc.setDumpToGladeXML( DumpToGladeXML );
            fc.setDumpFromGladeXML( DumpFromGladeXML );
            fc.setDumpToFerrisXML( DumpToFerrisXML );

            LG_FERRISCREATE_D << "Should create RootName:" << RootName << endl;

            fh_context ctx = theContext = Resolve( RootName );

            if( !DontReadRootContext && ctx->getHasSubContextsGuess() )
            {
                cerr << "Reading url:" << ctx->getURL() << endl;
                ctx->read();
            }
            
            LG_FERRISCREATE_D << "Setting schema" << endl;
            fc.setSchema( ctx->getCreateSubContextSchema() );

            LG_FERRISCREATE_D << "Getting glade2 document" << endl;
//             struct timeval pre_glade2xsl_tv;
//             gettimeofday( &pre_glade2xsl_tv, 0 );

            fh_istream xdoc = fc.getGlade2Document();

            LG_FERRISCREATE_D << "Processing..." << endl;

            string xdocstr = StreamToString(xdoc);
        
            if(!(xml = glade_xml_new_from_buffer( xdocstr.c_str(),
                                                  xdocstr.length(), "win", 0))) 
            {
                LG_FERRISCREATE_D << "Could not load the interface!" << endl;
                return 1;
            }

            if( CreateTypeName )
            {
                GtkWidget* w = glade_xml_get_widget( xml, "win" );
                setPage( w, CreateTypeName );
            }
            if( CreateRdn )
            {
                GtkWidget* w = glade_xml_get_widget( xml, "win" );
                setEntryProperty( w, "name", CreateRdn );
            }
            

            
            win = GTK_WINDOW( glade_xml_get_widget(xml, "win") );
            gtk_container_foreach ( GTK_CONTAINER(win), fixnb_fe, 0 );
            gtk_container_foreach ( GTK_CONTAINER(win), fixopm_fe, 0 );
            gtk_container_foreach ( GTK_CONTAINER(win), fixstringlist_fe, 0 );
        
            okb  = glade_xml_get_widget(xml, "ok");
            canb = glade_xml_get_widget(xml, "cancel");

            if( okb )
            {
                gtk_signal_connect(GTK_OBJECT(okb),"clicked",
                                   GTK_SIGNAL_FUNC(ok_clicked_cb),
                                   0 );
            }

            if( canb )
            {
                gtk_signal_connect(GTK_OBJECT(canb),"clicked",
                                   GTK_SIGNAL_FUNC(can_clicked_cb),
                                   0 );
            }

//            gtk_signal_connect(GTK_OBJECT (win), "destroy", gtk_main_quit, NULL);
            gtk_signal_connect(GTK_OBJECT (win), "destroy", game_over, NULL);
            if( !Sloth )
            {
                gtk_widget_show_all(GTK_WIDGET( win ));
            }

            if( AutoRun && okb )
            {
                g_timeout_add( 1, GSourceFunc(click_button), okb );
            }
            
            LG_FERRISCREATE_D << "going to gtk_main() " << endl;
            gtk_main();
        }
    }
    catch( exception& e )
    {
        cerr << "cought e:" << e.what() << endl;
        exit(1);
    }
    

    poptFreeContext(optCon);
    return 0;
}
