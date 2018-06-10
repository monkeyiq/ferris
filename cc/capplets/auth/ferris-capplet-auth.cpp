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

    $Id: ferris-capplet-auth.cpp,v 1.19 2011/10/22 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define HAVE_VIMEO_API     1
#define HAVE_FACEBOOK_API  1
#define HAVE_GOOGLE_API    1
#define HAVE_IDENTICA_API  1
//#define HAVE_BOXCOM_API    0

#include <typeinfo>

#include "config.h"

#ifdef HAVE_DTL
#include <DTL.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <Ferris.hh>
#include <FerrisUI/GtkFerris.hh>

#ifdef HAVE_WEBPHOTOS_API
#include "plugins/context/webphotos/libferriswebphotos_shared.hh"
#endif
#ifdef HAVE_FACEBOOK_API
#include "plugins/context/facebook/libferrisfacebook_shared.hh"
using Ferris::Facebook::fh_facebook;
#endif

#ifdef HAVE_VIMEO_API
#include "plugins/context/vimeo/libferrisvimeo_shared.hh"
using Ferris::Vimeo::fh_vimeo;
#endif

#ifdef HAVE_IDENTICA_API
#include "plugins/context/identica/libferrisidentica_shared.hh"
using Ferris::Identica::fh_identica;
#endif

#ifdef HAVE_GOOGLE_API
#include "plugins/context/google/libferrisgoogle_shared.hh"
// namespace Ferris
// {
//     FERRISEXP_EXPORT userpass_t getGoogleUserPass( const std::string& server = "" );
//     FERRISEXP_EXPORT void setGoogleUserPass( const std::string& server,
//                                              const std::string& user, const std::string& pass );
// };
#endif

#ifdef HAVE_BOXCOM_API
#include "plugins/context/boxcom/libferrisboxcom_shared.hh"
#endif

#include "plugins/context/wiki/libferriswikishared.hh"


#include <FerrisUI/GtkFerris.hh>
using namespace FerrisUI;

#include <Ferris_private.hh>
#include <Enamel_priv.hh>

#include <string>
#include <iostream>
#include <iomanip>

#include <libferrisldapshared.hh>

#ifdef HAVE_SQLPLUS
#include <mysql++.h>
#include <libferrissqlplusshared.hh>
using namespace mysqlpp;
#endif

#ifdef HAVE_LIBPQXX
#include <pqxx/connection>
#include <pqxx/tablewriter>
#include <pqxx/transaction>
#include <pqxx/nontransaction>
#include <pqxx/tablereader>
#include <pqxx/tablewriter>
#include <pqxx/result>
#include <pqxx/cursor>
#include <libferrispostgresqlshared.hh>
using namespace PGSTD;
using namespace pqxx;
#endif

#include <libferriszoneminder_shared.hh>
#include <libferrisferrisrest_shared.hh>

#ifdef HAVE_DTL
#include <libferrisdtlshared.hh>
using namespace dtl;
#endif

#ifdef HAVE_OBBY
#include <libferrisobbyshared.hh>
#endif

using namespace std;

namespace Ferris
{
    const string PROGRAM_NAME = "ferris-capplet-auth";
    const string DBNAME = FDB_SECURE;

    GtkWidget*     gtk_window;
    GtkWidget*     gtk_scrolledwindow;
    GtkWidget*     gtk_table;
    GtkNotebook*   gtk_notebook;
    GtkTreeStore*  w_treemodel;
    GtkWidget*     w_treeview;
    enum {
        C_SERV=0,
//         C_USER,
//         C_PASS,
        C_COUNT
    };

    GtkTreeViewColumn* w_cols[C_COUNT];

    GtkWidget* sql_username;
    GtkWidget* sql_password;

    GtkWidget* psql_username;
    GtkWidget* psql_password;

    GtkWidget* zm_username;
    GtkWidget* zm_password;

    GtkWidget* ferrisrest_username;
    GtkWidget* ferrisrest_password;
    GtkWidget* ferrisrest_baseurl;
    
    GtkWidget* odbc_username;
    GtkWidget* odbc_password;
    
    GtkWidget* ldap_username;
    GtkWidget* ldap_password;
    GtkWidget* ldap_basedn;
    GtkWidget* ldap_lookup_basedn;

    GtkWidget* obby_username;
    GtkWidget* obby_password;

    GtkWidget* google_username;
    GtkWidget* google_password;
    GtkWidget* w_gdrive_status_label = 0;

    string treestr( GtkTreeIter *iter, int col )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        GValue value;
        memset( &value, 0, sizeof(value));
        gtk_tree_model_get_value( tm, iter, col, &value );
        string ret = g_value_get_string(&value);
        g_value_unset ( &value );

        return ret;
    }

/************************************************************/
/************************************************************/
/************************************************************/

    GtkWidget* w_facebook_status_label = 0;
    GtkWidget* w_facebook_default_resize_combo = 0;

    GtkWidget* w_vimeo_status_label = 0;

    GtkWidget* w_identica_status_label = 0;
    
    GtkWidget* w_flickr_status_label = 0;
    GtkWidget* w_flickr_default_resize_combo = 0;
    GtkWidget* w_flickr_include_ea_is_present = 0;
    GtkWidget* w_flickr_include_ea_and_value = 0;
    GtkWidget* w_flickr_default_public;
    GtkWidget* w_flickr_default_friend;
    GtkWidget* w_flickr_default_family;


    GtkWidget* w_twothreehq_status_label = 0;
    GtkWidget* w_twothreehq_default_resize_combo = 0;
    GtkWidget* w_twothreehq_include_ea_is_present = 0;
    GtkWidget* w_twothreehq_include_ea_and_value = 0;
    GtkWidget* w_twothreehq_default_public;
    GtkWidget* w_twothreehq_default_friend;
    GtkWidget* w_twothreehq_default_family;

    GtkWidget* w_pixelpipe_status_label = 0;
    GtkWidget* w_pixelpipe_default_resize_combo = 0;
    GtkWidget* w_pixelpipe_include_ea_is_present = 0;
    GtkWidget* w_pixelpipe_include_ea_and_value = 0;
    GtkWidget* w_pixelpipe_default_public;
    GtkWidget* w_pixelpipe_default_friend;
    GtkWidget* w_pixelpipe_default_family;

    
    
#ifdef HAVE_WEBPHOTOS_API

    fh_webPhotos flickrWebPhotos = 0;
    fh_webPhotos twothreehqWebPhotos = 0;
    fh_webPhotos pixelpipeWebPhotos = 0;

    void update_webphotos_status( fh_webPhotos wf, GtkWidget* lab )
    {
        LG_WEBPHOTO_D << "haveToken:" << wf->haveToken()
                      << " token:" << wf->getToken() << endl;
        
        if( wf->haveToken() )
        {
            stringstream ss;
            ss << "authentication OK";

            
            string fn = wf->getUserName();
            if( !fn.empty() )
            {
                ss << endl << "username:" << fn;
            }

            gtk_label_set_text( GTK_LABEL(lab), ss.str().c_str() );
        }
        else
        {
            gtk_label_set_text( GTK_LABEL(lab), "authentication needed" );
        }
    }

    
    
    string
    flickr_request_frob( fh_webPhotos wf )
    {
        stringmap_t args;
        args.insert( make_pair( "method", "flickr.auth.getFrob" ) );
        args.insert( make_pair( "api_key", wf->getAPIKey() ) );

        fh_context res = wf->signedCall( "flickr.auth.getFrob", args, true );

        LG_WEBPHOTO_W << "rsp:" << res->isSubContextBound( "rsp" ) << endl;
        LG_WEBPHOTO_W << "have-frob:" << res->isSubContextBound( "frob" ) << endl;

        
        if( res->isSubContextBound( "frob" ) )
        {
            fh_context e = res->getSubContext( "frob" );
            string frob = getStrAttr( e, "content", "" );
            if( !frob.empty() )
                return frob;

            stringstream ss;
            ss << "Error. no frob string!" << endl;
            LG_WEBPHOTO_W << tostr(ss);
            Throw_WebPhotoException( tostr(ss), 0 );
        }
        
        stringstream ss;
        ss << "Error. no frob element!" << endl;
        LG_WEBPHOTO_W << tostr(ss);
        Throw_WebPhotoException( tostr(ss), 0 );
    }

    void authenticate_with_flickr_run_ffox_cb( GtkButton *button, gpointer user_data )
    {
        const char* earl = (const char*)user_data;
        stringstream ss;
        
        fh_runner r = new Runner();
        r->pushCommandLineArg( "firefox" );
        r->pushCommandLineArg( earl );
        r->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                r->getSpawnFlags()));
        r->Run();
    }
    
    string createFrobLoginURL( fh_webPhotos wf, const std::string& frob )
    {
        stringmap_t args;
        args["perms"] = "write";
        args["frob"] = frob;
        args["api_key"] = wf->getAPIKey();
                
        string md5string = wf->makeSignedString( args );
        args["api_sig"] = md5string;
        string url = wf->getAuthURL();
        stringstream callss;
        callss << url << "?";
        for( stringmap_t::iterator iter = args.begin(); iter != args.end(); ++iter )
        {
            callss << iter->first << "=" << iter->second << "&";
        }
        string earl = callss.str();
        return earl;
    }

    
    void convertFrobToToken( fh_webPhotos wf, const std::string& frob, bool GUI = true )
    {
        stringmap_t args;
        args["frob"] = frob;
        fh_context res = wf->signedCall( "flickr.auth.getToken", args, true );

        if( res->isSubContextBound( "auth" ) )
        {
            // <token>45-76598454353455</token>
            // <perms>read</perms>
            // <user nsid="12037949754@N01" username="Bees" fullname="Cal H" />
            fh_context e = res->getSubContext( "auth" );

            string token = getStrSubCtx( e, "token", "" );
            string perms = getStrSubCtx( e, "perms", "" );
            string nsid = "";
            string username = "";
            string fullname = "";
                    
            if( e->isSubContextBound( "user" ) )
            {
                fh_context u = e->getSubContext( "user" );
                nsid     = getStrAttr( u, "nsid", "" );
                username = getStrAttr( u, "username", "" );
                fullname = getStrAttr( u, "fullname", "" );
            }

            cerr << "impl:" << wf->getImplementationShortName() << endl;
            cerr << "token:" << token << endl;
            if( !token.empty() )
            {
                wf->setDefaultUsername( username );
                wf->setUserName( username );
                wf->setFullName( fullname );
                wf->setToken( token );
                LG_WEBPHOTO_D << "Success!" << endl;
            }
            else
            {
                stringstream ss;
                ss << "No token could be retrieved from photo service";
                if( GUI )
                    RunErrorDialog(tostr(ss));
                else
                    cerr << ss.str() << endl;
            }
        }
    }
        
    
    void
    authenticate_with_flickr_cb( GtkButton *button, gpointer user_data )
    {
        fh_webPhotos wf = (WebPhotos*)user_data;
        
        try
        {
            
            LG_WEBPHOTO_D << "authenticate_with_flickr_cb() called" << endl;
            string frob = flickr_request_frob( wf );
//            string frob = "dummy";
            
            LG_WEBPHOTO_D << "authenticate_with_flickr_cb() have_frob:" << frob << endl;

            // 4. Create a login link
            {
                string earl = createFrobLoginURL( wf, frob );
                
                // stringmap_t args;
                // args["perms"] = "write";
                // args["frob"] = frob;
                // args["api_key"] = wf->getAPIKey();
                
                // string md5string = wf->makeSignedString( args );
                // args["api_sig"] = md5string;
                // string url = wf->getAuthURL();
                // stringstream callss;
                // callss << url << "?";
                // for( stringmap_t::iterator iter = args.begin(); iter != args.end(); ++iter )
                // {
                //     callss << iter->first << "=" << iter->second << "&";
                // }
                // string earl = callss.str();

                LG_WEBPHOTO_D << "link for user to validate frob:" << earl << endl;

                GtkWidget *dialog, *label;
   
                dialog = gtk_dialog_new_with_buttons ("Authenticate using browser",
                                                      0,
                                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_OK,
                                                      GTK_RESPONSE_NONE,
                                                      NULL);
                GtkWidget* linkb = 0;
#ifdef MAEMO_BUILD
                label = gtk_label_new ("You must now verify the frob using your web browser" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
#else
                label = gtk_label_new ("You must now verify the frob using your web browser\n"
                                       "once you are done return and confirm this dialog\n"
                                       "Either click the link to run firefox on the URL\n"
                                       "or copy and paste the URL shown below to your browser\n" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                linkb = gtk_link_button_new_with_label( earl.c_str(), "click here to run firefox on the URL" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), linkb);
                gtk_signal_connect(GTK_OBJECT (linkb), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_flickr_run_ffox_cb), (void*)earl.c_str() );
#endif

                GtkWidget* e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), earl.c_str() );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);
                
                gtk_widget_show_all (dialog);
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy( dialog );
            }

            LG_WEBPHOTO_D << "Dialog closed... attempt to call getToken() starting" << endl;
            // 5. Convert frob to a token
            bool GUI = true;
            convertFrobToToken( wf, frob, GUI );
            // {
            //     stringmap_t args;
            //     args["frob"] = frob;
            //     fh_context res = wf->signedCall( "flickr.auth.getToken", args, true );

            //     if( res->isSubContextBound( "auth" ) )
            //     {
            //         // <token>45-76598454353455</token>
            //         // <perms>read</perms>
            //         // <user nsid="12037949754@N01" username="Bees" fullname="Cal H" />
            //         fh_context e = res->getSubContext( "auth" );

            //         string token = getStrSubCtx( e, "token", "" );
            //         string perms = getStrSubCtx( e, "perms", "" );
            //         string nsid = "";
            //         string username = "";
            //         string fullname = "";
                    
            //         if( e->isSubContextBound( "user" ) )
            //         {
            //             fh_context u = e->getSubContext( "user" );
            //             nsid     = getStrAttr( u, "nsid", "" );
            //             username = getStrAttr( u, "username", "" );
            //             fullname = getStrAttr( u, "fullname", "" );
            //         }

            //         if( !token.empty() )
            //         {
            //             wf->setDefaultUsername( username );
            //             wf->setUserName( username );
            //             wf->setFullName( fullname );
            //             wf->setToken( token );
            //             LG_WEBPHOTO_D << "Success!" << endl;
            //         }
            //         else
            //         {
            //             stringstream ss;
            //             ss << "No token could be retrieved from photo service";
            //             RunErrorDialog(tostr(ss));
            //         }
            //     }
            // }
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error authenticaing with flickr!\n"
               << " e:" << e.what()
               << endl;
            RunErrorDialog(tostr(ss));
        }
        
        if( flickrWebPhotos )
            update_webphotos_status( flickrWebPhotos, w_flickr_status_label );
        if( twothreehqWebPhotos )
            update_webphotos_status( twothreehqWebPhotos, w_twothreehq_status_label );
        if( pixelpipeWebPhotos )
            update_webphotos_status( pixelpipeWebPhotos, w_pixelpipe_status_label );
    }

#endif


#ifdef HAVE_FACEBOOK_API

    void update_facebook_status( fh_facebook fb, GtkWidget* lab )
    {
        LG_FACEBOOK_D << "have session_key:" << fb->haveSessionKey()
                      << " session_key value:" << fb->getSessionKey() << endl;
        
        if( fb->haveSessionKey() )
        {
            stringstream ss;
            ss << "authentication OK";

            string fn = fb->getUserName();
            if( !fn.empty() )
            {
                ss << endl << "username:" << fn;
            }

            gtk_label_set_text( GTK_LABEL(lab), ss.str().c_str() );
        }
        else
        {
            gtk_label_set_text( GTK_LABEL(lab), "authentication needed" );
        }
    }

    
    void
    authenticate_with_facebook_cb( GtkButton *button, gpointer user_data )
    {
        fh_facebook fb = (Facebook::Facebook*)user_data;

        try
        {
            cerr << "authenticate_with_facebook_cb" << endl;
            LG_FACEBOOK_D << "authenticate_with_facebook_cb() called" << endl;

            string authToken = fb->createAuthToken();
            cerr << "authToken:" << authToken << endl;

            // 4. Create a login link
            {
                stringstream earlss;
                earlss << "http://www.facebook.com/login.php?v=1.0&auth_token=" << authToken
                       << "&api_key=" << fb->APIKey();
                string earl = earlss.str();
                LG_FACEBOOK_D << "earl:" << earl << endl;

                GtkWidget *dialog, *label;
   
                dialog = gtk_dialog_new_with_buttons ("Authenticate using browser",
                                                      0,
                                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_OK,
                                                      GTK_RESPONSE_NONE,
                                                      NULL);
                GtkWidget* linkb = 0;
#ifdef MAEMO_BUILD
                label = gtk_label_new ("You must now verify the authToken using your web browser" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
#else
                label = gtk_label_new ("You must now verify the authToken using your web browser\n"
                                       "once you are done return and confirm this dialog\n"
                                       "Either click the link to run firefox on the URL\n"
                                       "or copy and paste the URL shown below to your browser\n" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                linkb = gtk_link_button_new_with_label( earl.c_str(), "click here to run firefox on the URL" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), linkb);
                gtk_signal_connect(GTK_OBJECT (linkb), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_flickr_run_ffox_cb), (void*)earl.c_str() );
#endif

                GtkWidget* e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), earl.c_str() );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);


                //////////////////
                
                stringstream authss;
                authss << "http://www.facebook.com/authorize.php?api_key=" << fb->APIKey()
                       << "&v=1.0&ext_perm=publish_stream";
                string authearl = authss.str();

                linkb = gtk_link_button_new_with_label( earl.c_str(), "Then add permissions if you wish..." );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), linkb);
                gtk_signal_connect(GTK_OBJECT (linkb), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_flickr_run_ffox_cb), (void*)authearl.c_str() );

                e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), authearl.c_str() );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);
                

                //////////////////
                
                gtk_widget_show_all (dialog);
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy( dialog );
            }

            // 5. get a session key
            {
                string sessionKey = fb->getAuthSession( authToken );
            }
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error authenticaing with flickr!\n"
               << " e:" << e.what()
               << endl;
            RunErrorDialog(tostr(ss));
        }
        
        update_facebook_status( fb, w_facebook_status_label );
        
    }
#endif
    
/************************************************************/
/************************************************************/
/************************************************************/

#ifdef HAVE_VIMEO_API

    void update_vimeo_status( fh_vimeo v, GtkWidget* lab )
    {
        LG_VIMEO_D << "have auth_token:" << v->isAuthenticated()
                   << " token:" << v->getAuthToken() << endl;
        
        if( v->isAuthenticated() )
        {
            stringstream ss;
            ss << "authentication OK";

            string fn = v->getUserName();
            if( !fn.empty() )
            {
                ss << endl << "username:" << fn;
            }

            gtk_label_set_text( GTK_LABEL(lab), ss.str().c_str() );
        }
        else
        {
            gtk_label_set_text( GTK_LABEL(lab), "authentication needed" );
        }
    }

    
    void
    authenticate_with_vimeo_cb( GtkButton *button, gpointer user_data )
    {
        fh_vimeo v = (Vimeo::Vimeo*)user_data;

        try
        {
            cerr << "authenticate_with_vimeo_cb" << endl;
            LG_VIMEO_D << "authenticate_with_vimeo_cb() called" << endl;
            std::string verifier;

            // 4. Create a login link
            {
                std::string earl = v->requestToken();
                LG_VIMEO_D << "earl:" << earl << endl;

                GtkWidget *dialog, *label;
   
                dialog = gtk_dialog_new_with_buttons ("Authenticate using browser",
                                                      0,
                                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_OK,
                                                      GTK_RESPONSE_NONE,
                                                      NULL);
                GtkWidget* linkb = 0;
                label = gtk_label_new ("You must now verify the authToken using your web browser\n"
                                       "once you are done return, paste the link you were redirected to'\n"
                                       "and confirm this dialog...\n"
                                       "Either click the link below to run firefox on the URL\n"
                                       "or copy and paste the URL shown below to your browser\n" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                linkb = gtk_link_button_new_with_label( earl.c_str(), "click here to run firefox on the URL" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), linkb);
                gtk_signal_connect(GTK_OBJECT (linkb), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_flickr_run_ffox_cb), (void*)earl.c_str() );

                GtkWidget* e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), earl.c_str() );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);

                label = gtk_label_new ("Paste the link you were sent to below");
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                
                e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), "" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);
                GtkEntry* reply = GTK_ENTRY(e);
                
                //////////////////
                
                gtk_widget_show_all (dialog);
                gtk_dialog_run (GTK_DIALOG (dialog));

                verifier = FerrisUI::tostr(reply);
                cerr << "Getting token... v:" << verifier << endl;
                
                gtk_widget_destroy( dialog );
            }

            // 5. get a session key
            if( !verifier.empty() )
            {
                v->accessToken( verifier );
            }
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error authenticaing with flickr!\n"
               << " e:" << e.what()
               << endl;
            RunErrorDialog(tostr(ss));
        }
        
        update_vimeo_status( v, w_vimeo_status_label );
        
    }
#endif

/************************************************************/
/************************************************************/
/************************************************************/

#ifdef HAVE_GOOGLE_API

    void update_gdrive_status( fh_GDriveClient v, GtkWidget* lab )
    {
        LG_GDRIVE_D << "have auth_token:" << v->isAuthenticated() << endl;
        
        if( v->isAuthenticated() )
        {
            stringstream ss;
            ss << "authentication OK";
            gtk_label_set_text( GTK_LABEL(lab), ss.str().c_str() );
        }
        else
        {
            gtk_label_set_text( GTK_LABEL(lab), "authentication needed" );
        }
    }

    
    void
    authenticate_with_gdrive_cb( GtkButton *button, gpointer user_data )
    {
        fh_GDriveClient v = GDriveClient::getGDriveClient();

        try
        {
            LG_GDRIVE_D << "authenticate_with_gdrive_cb() called" << endl;
            std::string verifier;

            // 4. Create a login link
            {
                std::string earl = v->requestToken();
                LG_GDRIVE_D << "earl:" << earl << endl;

                GtkWidget *dialog, *label;
   
                dialog = gtk_dialog_new_with_buttons ("Authenticate using browser",
                                                      0,
                                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      GTK_STOCK_OK,
                                                      GTK_RESPONSE_NONE,
                                                      NULL);
                GtkWidget* linkb = 0;
                label = gtk_label_new("");
                gtk_label_set_markup ( GTK_LABEL(label),
                                       "FIRST: open up  <a href=\"https://code.google.com/apis/console\">code.google.com/apis/console</a>\n"
                                       "and ensure you have the Drive API enabled\n"
                                       "\n"
                                       "Then must now verify the authToken using your web browser\n"
                                       "once you are done return, paste the code you were redirected to'\n"
                                       "and confirm this dialog...\n"
                                       "Either click the link below to run firefox on the URL\n"
                                       "or copy and paste the URL shown below to your browser\n" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                linkb = gtk_link_button_new_with_label( earl.c_str(), "click here to run firefox on the URL" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), linkb);
                gtk_signal_connect(GTK_OBJECT (linkb), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_flickr_run_ffox_cb), (void*)earl.c_str() );

                GtkWidget* e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), earl.c_str() );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);

                label = gtk_label_new ("Paste the code you were sent to below");
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
                
                e = gtk_entry_new();
                gtk_entry_set_text( GTK_ENTRY( e ), "" );
                gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), e);
                GtkEntry* reply = GTK_ENTRY(e);
                
                //////////////////
                
                gtk_widget_show_all (dialog);
                gtk_dialog_run (GTK_DIALOG (dialog));

                verifier = FerrisUI::tostr(reply);
                cerr << "Getting token... v:" << verifier << endl;
                
                gtk_widget_destroy( dialog );
            }

            // 5. get a session key
            if( !verifier.empty() )
            {
                v->accessToken( verifier );
            }
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error authenticaing with flickr!\n"
               << " e:" << e.what()
               << endl;
            RunErrorDialog(tostr(ss));
        }
        
        update_gdrive_status( v, w_gdrive_status_label );
        
    }
#endif
    
/************************************************************/
/************************************************************/
/************************************************************/


    
static gboolean
my_search_equal_func(GtkTreeModel *model,
                     gint column,
                     const gchar *key,
                     GtkTreeIter *iter,
                     gpointer search_data)
{
    cerr << "key:" << key  << endl;
    string serv = treestr( iter, C_SERV );
    return serv == key;
}

/************************************************************/
/************************************************************/
/************************************************************/
    

    gboolean SaveData_fe( GtkTreeModel *model, GtkTreePath *path,
                          GtkTreeIter *iter, gpointer udata )
    {
        fh_stringstream ss = *((fh_stringstream*)udata);
        GtkTreeView*  tv = GTK_TREE_VIEW( w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        string serv = treestr( iter, C_SERV );
//         string user = treestr( iter, C_USER );
//         string pass = treestr( iter, C_PASS );

        ss << serv << " ";
        {
            fh_stringstream ss;
            ss << "mysql-server-" << serv;
            setConfigString( DBNAME, tostr(ss), "1" );
        }

//         setUserPass( serv, user, pass );
        return 0;
    }

    void SaveData(void)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW( w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        fh_stringstream ss;
        gtk_tree_model_foreach( tm, GtkTreeModelForeachFunc(SaveData_fe), &ss );
        setConfigString( DBNAME, "mysql-server-keys", tostr(ss) );

        GtkTreeSelection* selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_unselect_all( selection );

#ifdef HAVE_FACEBOOK_API
        if( w_facebook_default_resize_combo )
        {
            string s = gtk_combo_box_get_active_text(
                GTK_COMBO_BOX( w_facebook_default_resize_combo ));
            if( starts_with( s, "no" ))
            {
                Facebook::Upload::setMaxDesiredWidthOrHeight( 0 );
            }
            else
            {
                Facebook::Upload::setMaxDesiredWidthOrHeight( toint(s) );
            }
        }
#endif
            
        
#ifdef HAVE_WEBPHOTOS_API
        {
            if( w_flickr_default_resize_combo )
            {
                string s = gtk_combo_box_get_active_text(
                    GTK_COMBO_BOX( w_flickr_default_resize_combo ));
                if( starts_with( s, "no" ))
                {
                    flickrWebPhotos->setDefaultLargestDimension( 0 );
                }
                else
                {
                    flickrWebPhotos->setDefaultLargestDimension( toint( s ) );
                }
            }

            if( w_flickr_include_ea_is_present )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_flickr_include_ea_is_present));
                flickrWebPhotos->setDefaultIncludeEAIsPresentRegex( s );
            }
            if( w_flickr_include_ea_and_value )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_flickr_include_ea_and_value));
                flickrWebPhotos->setDefaultIncludeEAandValueRegex( s );
            }

            
            flickrWebPhotos->setDefaultImageProtectionPublic(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_flickr_default_public)));
            flickrWebPhotos->setDefaultImageProtectionFriend(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_flickr_default_friend)));
            flickrWebPhotos->setDefaultImageProtectionFamily(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_flickr_default_family)));
        }


        {
            if( w_twothreehq_default_resize_combo )
            {
                string s = gtk_combo_box_get_active_text(
                    GTK_COMBO_BOX( w_twothreehq_default_resize_combo ));
                if( starts_with( s, "no" ))
                {
                    twothreehqWebPhotos->setDefaultLargestDimension( 0 );
                }
                else
                {
                    twothreehqWebPhotos->setDefaultLargestDimension( toint( s ) );
                }
            }

            if( w_twothreehq_include_ea_is_present )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_twothreehq_include_ea_is_present));
                twothreehqWebPhotos->setDefaultIncludeEAIsPresentRegex( s );
            }
            if( w_twothreehq_include_ea_and_value )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_twothreehq_include_ea_and_value));
                twothreehqWebPhotos->setDefaultIncludeEAandValueRegex( s );
            }

            
            twothreehqWebPhotos->setDefaultImageProtectionPublic(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_twothreehq_default_public)));
            twothreehqWebPhotos->setDefaultImageProtectionFriend(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_twothreehq_default_friend)));
            twothreehqWebPhotos->setDefaultImageProtectionFamily(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_twothreehq_default_family)));
        }


        {
            if( w_pixelpipe_default_resize_combo )
            {
                string s = gtk_combo_box_get_active_text(
                    GTK_COMBO_BOX( w_pixelpipe_default_resize_combo ));
                if( starts_with( s, "no" ))
                {
                    pixelpipeWebPhotos->setDefaultLargestDimension( 0 );
                }
                else
                {
                    pixelpipeWebPhotos->setDefaultLargestDimension( toint( s ) );
                }
            }

            if( w_pixelpipe_include_ea_is_present )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_pixelpipe_include_ea_is_present));
                pixelpipeWebPhotos->setDefaultIncludeEAIsPresentRegex( s );
            }
            if( w_pixelpipe_include_ea_and_value )
            {
                string s = gtk_entry_get_text(GTK_ENTRY(w_pixelpipe_include_ea_and_value));
                pixelpipeWebPhotos->setDefaultIncludeEAandValueRegex( s );
            }

            
            pixelpipeWebPhotos->setDefaultImageProtectionPublic(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_pixelpipe_default_public)));
            pixelpipeWebPhotos->setDefaultImageProtectionFriend(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_pixelpipe_default_friend)));
            pixelpipeWebPhotos->setDefaultImageProtectionFamily(
                gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(w_pixelpipe_default_family)));
        }
#endif

#ifdef HAVE_GOOGLE_API
        {
            if( google_username && google_password )
            {
                string u = gtk_entry_get_text(GTK_ENTRY(google_username));
                string p = gtk_entry_get_text(GTK_ENTRY(google_password));

                setGoogleUserPass( "", u, p );
            }
        }
#endif
        
    }

    GtkTreeIter appendNewBlankItem()
    {
        GtkTreeIter iter;
        const char* d = "x";
        gtk_tree_store_append( w_treemodel, &iter, NULL );
        gtk_tree_store_set( w_treemodel, &iter,
                            C_SERV, d,
//                             C_USER, d,
//                             C_PASS, d,
                            -1 );
        return iter;
    }


    void LoadData()
    {
        bool addedany = false;
        GtkTreeIter treeiter;
        int i=0;
        char** p;
        string servdb;
    
        fh_stringstream kss;
        kss << getConfigString( DBNAME, "mysql-server-keys", "" );

        while( getline( kss, servdb, ' ' ))
        { 
            cerr << "loading server-db:" << servdb << endl;
        
            addedany = true;
            gtk_tree_store_append( w_treemodel, &treeiter, NULL );

            string db;
            string serv;
//             userpass_t up = getUserPass( servdb );
//             string user = up.first;
//             string pass = up.second;

            {
                fh_stringstream ss;
                ss << servdb;
                getline( ss, serv, '-' );
                getline( ss, db );
            }
        
        
            gtk_tree_store_set( w_treemodel, &treeiter,
                                C_SERV, serv.c_str(),
//                                 C_USER, user.c_str(),
//                                 C_PASS, pass.c_str(),
                                -1 );
        
        }

        /* bug in gtk2 */
        if( !addedany )
        {
            appendNewBlankItem();
        }
    }

/**********************/

    void add_cb( GtkButton *button, gpointer user_data )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
    
        GtkTreeIter iter = appendNewBlankItem();

        GtkTreePath* path      = gtk_tree_model_get_path( tm, &iter );
        GtkTreeViewColumn* col = gtk_tree_view_get_column( tv, C_SERV );
        // FIXME: gtk2 doesn't like this. it doesn't close off the old cell edit
        gtk_tree_view_set_cursor( tv, path, col, true );
        gtk_tree_path_free( path );
    }


    typedef list< GtkTreeIter* > deliters_t;

    void del_cb_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        deliters_t* x = (deliters_t*)udata;
        x->push_back( gtk_tree_iter_copy( iter ));
    }


    void del_cb( GtkButton *button, gpointer user_data )
    {
        deliters_t deliters;
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_selected_foreach(
            selection, GtkTreeSelectionForeachFunc(del_cb_fe), &deliters );
        gtk_tree_selection_unselect_all( selection );

        for( deliters_t::iterator iter = deliters.begin(); iter != deliters.end(); ++iter )
        {
            gtk_tree_store_remove( ts, *iter );
        }
    }


#ifdef HAVE_SQLPLUS
    void sql_test_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        string db   = "";
        string serv = treestr( iter, C_SERV );
        string user = gtk_entry_get_text(GTK_ENTRY(sql_username));
        string pass = gtk_entry_get_text(GTK_ENTRY(sql_password));

        try
        {
            Connection con(use_exceptions);
            con.connect( db.c_str(), serv.c_str(), user.c_str(), pass.c_str() );

            fh_stringstream ss;
            ss << "Success connecting to db:" << db
               << " serv:" << serv
               << " user:" << user
               << " pass:" << pass
               << endl;
            RunInfoDialog(tostr(ss));
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error connecting to db:" << db
               << " serv:" << serv
               << " user:" << user
               << " pass:" << pass
               << endl;
            RunErrorDialog(tostr(ss));
        }
    }
#endif

#ifdef HAVE_LIBPQXX
    void psql_test_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        string db   = "";
        string serv = treestr( iter, C_SERV );
        string user = gtk_entry_get_text(GTK_ENTRY(psql_username));
        string pass = gtk_entry_get_text(GTK_ENTRY(psql_password));

        try
        {
            fh_stringstream ss;
            ss << "host=" << serv << " ";
            if( !user.empty() )
                ss << "user=" << user << " ";
            if( !pass.empty() )
                ss << "password=" << pass << " ";
            if( !db.empty() )
                ss << " dbname=" << db;
            ss << endl;

            connection con( ss.str() );
            fh_stringstream infoss;
            infoss << "Success connecting to db:" << db
                   << " serv:" << serv
                   << " user:" << user
                   << " pass:" << pass
                   << endl;
            RunInfoDialog(tostr(infoss));
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error connecting to db:" << db
               << " serv:" << serv
               << " user:" << user
               << " pass:" << pass
               << " error:" << e.what()
               << endl;
            RunErrorDialog(tostr(ss));
        }
    }
#endif

    void zm_test_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
    }


    void ferrisrest_test_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        string db   = "";
        string serv = treestr( iter, C_SERV );
        string user = gtk_entry_get_text(GTK_ENTRY(ferrisrest_username));
        string pass = gtk_entry_get_text(GTK_ENTRY(ferrisrest_password));
        string base = gtk_entry_get_text(GTK_ENTRY(ferrisrest_baseurl));

        Ferrisrest::setUserPass( serv, user, pass );
        Ferrisrest::setBaseURI(  serv, base );
        
        stringmap_t args;
        args["method"] = "version";
        args["format"] = "xml";
        args["zz"]     = "top";

        Ferrisrest::fh_ferrisrest zm = Factory::getFerrisrest( serv );
        QNetworkReply* reply = zm->post( args );
        QByteArray ba = reply->readAll();
        string v = "";
        try
        {
            fh_domdoc dom = zm->toDOM( ba );
            DOMElement* di = dom->getDocumentElement();
            if( di && tostr(di->getNodeName()) == "result" )
            {
                std::list< DOMElement* > keyvals = XML::getAllChildrenElements( (DOMNode*)di, "keyval" );
                for( std::list< DOMElement* >::iterator kviter = keyvals.begin();
                     kviter != keyvals.end(); ++kviter )
                {                        
                    string key = ::Ferris::getAttribute( (DOMElement*)*kviter, "key" );
                    string val = XML::getChildText( (DOMElement*)*kviter );
                    if( key == "version" )
                        v = val;
                }
                if( !v.empty() )
                {
                    fh_stringstream infoss;
                    infoss << "Success connecting to libferris over rest." << endl
                           << " remote version:" << v << endl;
                    RunInfoDialog(tostr(infoss));
                }
            }
        }
        catch( exception& e )
        {
        }
        if( v.empty() )
        {
            fh_stringstream infoss;
            infoss << "Failed connecting to libferris over rest." << endl
                   << " response:" << tostr(ba) << endl;
            RunErrorDialog(tostr(infoss));
        }
        
    }
    
    
#ifdef HAVE_DTL
    void odbc_test_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);

        string serv = treestr( iter, C_SERV );
        string user = gtk_entry_get_text(GTK_ENTRY(odbc_username));
        string pass = gtk_entry_get_text(GTK_ENTRY(odbc_password));

        try
        {
            fh_stringstream args;
            args << "UID=" << user  << ";"
                 << "PWD=" << pass  << ";"
                 << "DSN=" << serv  << ";"
                 << endl;
            DBConnection con;
            con.Connect( tostr( args ) );

            fh_stringstream ss;
            ss << "Success connecting to"
               << " serv:" << serv
               << " user:" << user
               << " pass:" << pass
               << endl;
            RunInfoDialog(tostr(ss));
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Error connecting to"
               << " serv:" << serv
               << " user:" << user
               << " pass:" << pass
               << endl;
            RunErrorDialog(tostr(ss));
        }
    }
#endif
    
    void test_cb( GtkButton *button, gpointer user_data )
    {
        GtkTreeView*  tv = GTK_TREE_VIEW(w_treeview);
        GtkTreeModel* tm = GTK_TREE_MODEL(w_treemodel);
        GtkTreeStore* ts = GTK_TREE_STORE(w_treemodel);
        GtkTreeSelection *selection;

        selection = gtk_tree_view_get_selection ( tv );
        gtk_tree_selection_selected_foreach(
            selection, GtkTreeSelectionForeachFunc(user_data), 0 );
    }

    static void
    cell_edited_cb (GtkCellRendererText *cell, gchar *path_string,
                    gchar *new_text, gpointer udata)
    {
        gint cidx = GPOINTER_TO_INT(udata);
        GtkTreeModel *model = GTK_TREE_MODEL (w_treemodel);
        GtkTreeIter iter;
        GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter, cidx, new_text, -1);
        gtk_tree_path_free (path);
    }

    void tv_changed_fe( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer udata)
    {
        string* sp = (string*)udata;
        *sp = treestr( iter, C_SERV );
    }
    
    void tv_changed_cb( GtkTreeSelection *sel, gpointer )
    {
        static string old;
        string current;
        
        gtk_tree_selection_selected_foreach(
            sel, GtkTreeSelectionForeachFunc(tv_changed_fe), &current  );

        cerr << "tv_changed_cb() old:" << old << " new:" << current << endl;
        
        if( old != current )
        {
            if( old.length() )
            {
                // save the old data
                
#ifdef HAVE_SQLPLUS
                {
                    string username = gtk_entry_get_text(GTK_ENTRY(sql_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(sql_password));
                    setUserPass( old, username, password );
                }
#endif

#ifdef HAVE_LIBPQXX
                {
                    string username = gtk_entry_get_text(GTK_ENTRY(psql_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(psql_password));
                    setPostgreSQLUserPass( old, username, password );
                }
#endif

                {
                    string username = gtk_entry_get_text(GTK_ENTRY(zm_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(zm_password));
                    Zoneminder::setUserPass( old, username, password );
                }

                {
                    string username = gtk_entry_get_text(GTK_ENTRY(ferrisrest_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(ferrisrest_password));
                    string baseurl  = gtk_entry_get_text(GTK_ENTRY(ferrisrest_baseurl));
                    Ferrisrest::setUserPass( old, username, password );
                    Ferrisrest::setBaseURI(  old, baseurl );
                }
                
#ifdef HAVE_DTL
                {
                    string username = gtk_entry_get_text(GTK_ENTRY(odbc_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(odbc_password));
                    setODBCUserPass( old, username, password );
                }
#endif

#ifdef HAVE_LDAP
                {
                    LDAPAuth::LDAPAuthInfo d;
                    d.username      = gtk_entry_get_text(GTK_ENTRY(ldap_username));
                    d.password      = gtk_entry_get_text(GTK_ENTRY(ldap_password));
                    d.basedn        = gtk_entry_get_text(GTK_ENTRY(ldap_basedn));
                    d.lookup_basedn = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(ldap_lookup_basedn));
                    LDAPAuth::setUserPass( old, d );
                }
#endif
                
#ifdef HAVE_OBBY
                {
                    string username = gtk_entry_get_text(GTK_ENTRY(obby_username));
                    string password = gtk_entry_get_text(GTK_ENTRY(obby_password));
                    setOBBYUserPass( old, username, password );
                }
#endif
            }

            // load the new selection's data

#ifdef HAVE_SQLPLUS
            {
                userpass_t up = getUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(sql_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(sql_password), password.c_str() );
            }
#endif

#ifdef HAVE_LIBPQXX
            {
                userpass_t up = getPostgreSQLUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(psql_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(psql_password), password.c_str() );
            }
#endif

            {
                userpass_t up = Zoneminder::getUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(zm_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(zm_password), password.c_str() );
            }


            {
                userpass_t up = Ferrisrest::getUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(ferrisrest_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(ferrisrest_password), password.c_str() );
                gtk_entry_set_text( GTK_ENTRY(ferrisrest_baseurl),
                                    Ferrisrest::getBaseURI( current ).c_str() );
            }
            
            
#ifdef HAVE_DTL
            {
                userpass_t up = getODBCUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(odbc_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(odbc_password), password.c_str() );
            }
#endif
            
#ifdef HAVE_LDAP
            {
                LDAPAuth::LDAPAuthInfo d = LDAPAuth::getUserPass( current );

                gtk_entry_set_text( GTK_ENTRY(ldap_username), d.username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(ldap_password), d.password.c_str() );
                gtk_entry_set_text( GTK_ENTRY(ldap_basedn), d.basedn.c_str() );
                gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(ldap_lookup_basedn),
                                              d.lookup_basedn );
            }
#endif
            
#ifdef HAVE_OBBY
            {
                userpass_t up = getOBBYUserPass( current );
                string username = up.first;
                string password = up.second;

                gtk_entry_set_text( GTK_ENTRY(obby_username), username.c_str() );
                gtk_entry_set_text( GTK_ENTRY(obby_password), password.c_str() );
            }
#endif
            
            old = current;
        }
    }
    
/**********************/

    void save_and_quit_cb( GtkButton *button, gpointer user_data )
    {
        SaveData();
        gtk_main_quit();
    }

    void quit_cb( GtkButton *button, gpointer user_data )
    {
        gtk_main_quit();
    }


    void make_window()
    {
        GtkWidget* list_scrollwin;
        GtkWidget* sw;
        GtkWidget* b;
        GtkWidget* listarea;

        gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title ( GTK_WINDOW( gtk_window ), "Ferris authentication" );
        gtk_window_set_default_size (GTK_WINDOW (gtk_window), 600, 450);
        gtk_signal_connect(GTK_OBJECT (gtk_window), "destroy", gtk_main_quit, NULL);
        GtkNotebook* main_widget = GTK_NOTEBOOK(gtk_notebook_new());

        GtkPaned* paned = GTK_PANED(gtk_hpaned_new());
        GtkWidget* tree_frame    = gtk_frame_new (NULL);
        GtkWidget* details_frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (tree_frame),    GTK_SHADOW_IN);
        gtk_frame_set_shadow_type (GTK_FRAME (details_frame), GTK_SHADOW_IN);
        gtk_paned_pack1 (GTK_PANED (paned), tree_frame,    true, false );
        gtk_paned_pack2 (GTK_PANED (paned), details_frame, true, false );
        gtk_frame_set_label( GTK_FRAME (details_frame), "Authentication Information" );
        gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "Servers" ));

        
        int r=0;
    
        {
            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            
            GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
            GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

            b = gtk_toolbar_append_item( tb, "New",
                                         "create a new server/database connection", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_NEW,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(add_cb),
                                         0 );

            b = gtk_toolbar_append_item( tb, "Delete",
                                         "delete selected server/database connection(s)", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_DELETE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(del_cb),
                                         0 );

//            gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
            gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( tb ));
            gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
            
            
//             b = gtk_button_new_from_stock( GTK_STOCK_NEW );
//             gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
//             gtk_signal_connect(GTK_OBJECT (b), "clicked",
//                                GTK_SIGNAL_FUNC(add_cb), NULL);
        
//             b = gtk_button_new_from_stock( GTK_STOCK_DELETE );
//             gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
//             gtk_signal_connect(GTK_OBJECT (b), "clicked",
//                                GTK_SIGNAL_FUNC(del_cb), NULL);

//             b = gtk_button_new_from_stock( GTK_STOCK_EXECUTE );
//             gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
//             gtk_signal_connect(GTK_OBJECT (b), "clicked",
//                                GTK_SIGNAL_FUNC(test_cb), NULL);


        
//            gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );
//            gtk_table_attach_defaults(GTK_TABLE(page), vbx, 0, 1, r, r+1 );

    

            w_treemodel = gtk_tree_store_new( C_COUNT,
                                              G_TYPE_STRING, G_TYPE_STRING,
                                              G_TYPE_STRING, G_TYPE_STRING );
            w_treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL( w_treemodel )); 
//    gtk_table_attach_defaults(GTK_TABLE(page), w_treeview, 0, 1, r, r+1 );
//            gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(w_treeview), 1, 1, 0 );
   
            gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (w_treeview), TRUE);
            GObject *selection;
            selection = G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (w_treeview)));
//            gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_MULTIPLE);
            gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);
            gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW (w_treeview), true );
            gtk_tree_view_set_enable_search(GTK_TREE_VIEW (w_treeview), true );
            gtk_tree_view_set_search_column(GTK_TREE_VIEW (w_treeview), C_SERV );
            gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW (w_treeview), 
                                                my_search_equal_func, 0, 0 );
            g_signal_connect_data( G_OBJECT( selection ), "changed",
                                   G_CALLBACK (tv_changed_cb), 0, 0, GConnectFlags(0));

            
            /* Setup view */
            GtkCellRenderer* ren;

            ren = gtk_cell_renderer_text_new ();
            g_object_set(ren, "editable", 1, 0 );
            g_signal_connect_data( G_OBJECT( ren ), "edited",
                                   G_CALLBACK (cell_edited_cb),
                                   GINT_TO_POINTER(C_SERV), 0, GConnectFlags(0));
    
            w_cols[C_SERV] = gtk_tree_view_column_new_with_attributes( "Server", ren,
                                                                       "text", C_SERV,
                                                                       NULL);
            gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[C_SERV] );
//            gtk_tree_view_column_set_sizing ( w_cols[C_SERV], GTK_TREE_VIEW_COLUMN_RESIZABLE );
            gtk_tree_view_column_set_sort_column_id( w_cols[C_SERV], C_SERV );

//             ren = gtk_cell_renderer_text_new ();
//             g_object_set(ren, "editable", 1, 0 );
//             g_signal_connect_data( G_OBJECT( ren ), "edited",
//                                    G_CALLBACK (cell_edited_cb),
//                                    (gpointer)C_USER, 0, GConnectFlags(0));

//             w_cols[C_USER] = gtk_tree_view_column_new_with_attributes( "Username", ren,
//                                                                        "text", C_USER,
//                                                                        NULL);
//             gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[C_USER] );
// //            gtk_tree_view_column_set_sizing ( w_cols[C_USER], GTK_TREE_VIEW_COLUMN_RESIZABLE );
//             gtk_tree_view_column_set_sort_column_id( w_cols[C_USER], C_USER );

//             ren = gtk_cell_renderer_text_new ();
//             g_object_set(ren, "editable", 1, 0 );
//             g_signal_connect_data( G_OBJECT( ren ), "edited",
//                                    G_CALLBACK (cell_edited_cb),
//                                    (gpointer)C_PASS, 0, GConnectFlags(0));
    
//             w_cols[C_PASS] = gtk_tree_view_column_new_with_attributes( "Password", ren,
//                                                                        "text", C_PASS,
//                                                                        NULL);
//             gtk_tree_view_append_column( GTK_TREE_VIEW( w_treeview ), w_cols[C_PASS] );
// //            gtk_tree_view_column_set_sizing ( w_cols[C_PASS], GTK_TREE_VIEW_COLUMN_RESIZABLE );
//             gtk_tree_view_column_set_sort_column_id( w_cols[C_PASS], C_PASS );
    
    
    
    
    

//            list_scrollwin = sw = gtk_scrolled_window_new(NULL, NULL);
//            gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET( page ));

            sw = gtk_scrolled_window_new(NULL, NULL);
            gtk_container_add( GTK_CONTAINER( sw ), GTK_WIDGET( w_treeview ));
            
            gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            
            gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(sw), 1, 1, 0 );
            gtk_container_add( GTK_CONTAINER( tree_frame ), vbx );
            
        }


#ifdef HAVE_WEBPHOTOS_API
        {
            GtkPaned* paned = GTK_PANED(gtk_hpaned_new());
            gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "Photo servers" ));

            GtkNotebook* photoservers = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_paned_pack1 (GTK_PANED (paned), GTK_WIDGET(photoservers), true, false );

            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = 0;

            r=0; c=0;
//            page = gtk_table_new ( 2, 2, false );
            GtkNotebook* flickrpage = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_notebook_append_page( photoservers,
                                      GTK_WIDGET(flickrpage),
                                      gtk_label_new( "flickr" ));

            fh_webPhotos wf = flickrWebPhotos = Factory::getDefaultFlickrWebPhotos();
            if( wf->haveAPIKey() )
            {
                {
                    page = gtk_table_new ( 2, 2, false );
                    gtk_notebook_append_page( GTK_NOTEBOOK(flickrpage), page,
                                              gtk_label_new( "auth" ));
                    
                    b = gtk_button_new_with_label("Authenticate with flickr");
                    gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                    ++c;
                    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                       GTK_SIGNAL_FUNC(authenticate_with_flickr_cb), GetImpl(wf) );
                    ++r; c=0;
                    
                    l = gtk_label_new ("Status" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    l = gtk_label_new ("" );
                    w_flickr_status_label = l;
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                }
                
                {
                    string s;
                    
                    page = gtk_table_new ( 2, 2, false );
                    gtk_notebook_append_page( GTK_NOTEBOOK(flickrpage), page,
                                              gtk_label_new( "settings" ));
                    r=0; c=0;

                    l = gtk_label_new("Default include EA is Present Regex");
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    w_flickr_include_ea_is_present = e = gtk_entry_new();
                    gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                    s = flickrWebPhotos->getDefaultIncludeEAIsPresentRegexString();
                    gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                    c=0; ++r; 

                    l = gtk_label_new("Default include EA and Value Regex");
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    w_flickr_include_ea_and_value = e = gtk_entry_new();
                    gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                    s = flickrWebPhotos->getDefaultIncludeEAandValueRegexString();
                    gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                    c=0; ++r; 


                    e = gtk_check_button_new_with_label("default permissions public");
                    w_flickr_default_public = e;
                    gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                    gtk_toggle_button_set_active(
                        GTK_TOGGLE_BUTTON(e), flickrWebPhotos->isDefaultImageProtectionPublic() );
                    c=0; ++r; 

                    e = gtk_check_button_new_with_label("default permissions friend");
                    w_flickr_default_friend = e;
                    gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                    gtk_toggle_button_set_active(
                        GTK_TOGGLE_BUTTON(e), flickrWebPhotos->isDefaultImageProtectionFriend() );
                    c=0; ++r; 

                    e = gtk_check_button_new_with_label("default permissions family");
                    w_flickr_default_family = e;
                    gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                    gtk_toggle_button_set_active(
                        GTK_TOGGLE_BUTTON(e), flickrWebPhotos->isDefaultImageProtectionFamily() );
                    c=0; ++r; 
                    
                    

                    l = gtk_label_new ("Default upload resize" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    GtkWidget* w = gtk_combo_box_new_text();
                    w_flickr_default_resize_combo = w;
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "no resize" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "320" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "640" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "800" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1024" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1280" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1600" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1920" );
                    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
                
                    {
                        gint index = -1;
                        long v = wf->getDefaultLargestDimension();
                        switch( wf->getDefaultLargestDimension() )
                        {
                        case 0:    index = 0; break;
                        case 320:  index = 1; break;
                        case 640:  index = 2; break;
                        case 800:  index = 3; break;
                        case 1024: index = 4; break;
                        case 1280: index = 5; break;
                        case 1600: index = 6; break;
                        case 1920: index = 7; break;
                        }
                        gtk_combo_box_set_active( GTK_COMBO_BOX(w), index );
                    }
                    ++r; c=0;
                }
                
                update_webphotos_status( wf, w_flickr_status_label );
            }
            else
            {
                l = gtk_label_new("No flickr API and secret defined.\n"
                                  "Please obtain an API key and shared secret from:\n"
                                  "http://www.flickr.com/services/api/auth.howto.desktop.html\n"
                                  "and place them in ~/.ferris/flickr-api-key.txt and flickr-api-secret.txt"
                    );
                gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+2, r, r+1 );
                ++c;
            }

            /********************************************************************************/
            /********************************************************************************/
            /********************************************************************************/
            
            {
                r=0; c=0;
                GtkNotebook* twothreehqpage = GTK_NOTEBOOK(gtk_notebook_new());
                gtk_notebook_append_page( photoservers,
                                          GTK_WIDGET(twothreehqpage),
                                          gtk_label_new( "23hq" ));

                fh_webPhotos wf = twothreehqWebPhotos = Factory::getDefault23hqWebPhotos();
                if( wf->haveAPIKey() )
                {
                    {
                        page = gtk_table_new ( 2, 2, false );
                        gtk_notebook_append_page( GTK_NOTEBOOK(twothreehqpage), page,
                                                  gtk_label_new( "auth" ));
                    
                        b = gtk_button_new_with_label("Authenticate with 23hq");
                        gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                        ++c;
                        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                           GTK_SIGNAL_FUNC(authenticate_with_flickr_cb), GetImpl(wf) );
                        ++r; c=0;
                    
                        l = gtk_label_new ("Status" );
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        l = gtk_label_new ("" );
                        w_twothreehq_status_label = l;
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    }
                
                    {
                        string s;
                    
                        page = gtk_table_new ( 2, 2, false );
                        gtk_notebook_append_page( GTK_NOTEBOOK(twothreehqpage), page,
                                                  gtk_label_new( "settings" ));
                        r=0; c=0;

                        l = gtk_label_new("Default include EA is Present Regex");
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        w_twothreehq_include_ea_is_present = e = gtk_entry_new();
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                        s = twothreehqWebPhotos->getDefaultIncludeEAIsPresentRegexString();
                        gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                        c=0; ++r; 

                        l = gtk_label_new("Default include EA and Value Regex");
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        w_twothreehq_include_ea_and_value = e = gtk_entry_new();
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                        s = twothreehqWebPhotos->getDefaultIncludeEAandValueRegexString();
                        gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                        c=0; ++r; 


                        e = gtk_check_button_new_with_label("default permissions public");
                        w_twothreehq_default_public = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), twothreehqWebPhotos->isDefaultImageProtectionPublic() );
                        c=0; ++r; 

                        e = gtk_check_button_new_with_label("default permissions friend");
                        w_twothreehq_default_friend = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), twothreehqWebPhotos->isDefaultImageProtectionFriend() );
                        c=0; ++r; 

                        e = gtk_check_button_new_with_label("default permissions family");
                        w_twothreehq_default_family = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), twothreehqWebPhotos->isDefaultImageProtectionFamily() );
                        c=0; ++r; 
                    
                    

                        l = gtk_label_new ("Default upload resize" );
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        GtkWidget* w = gtk_combo_box_new_text();
                        w_twothreehq_default_resize_combo = w;
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "no resize" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "320" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "640" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "800" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1024" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1280" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1600" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1920" );
                        gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
                
                        {
                            gint index = -1;
                            long v = wf->getDefaultLargestDimension();
                            switch( wf->getDefaultLargestDimension() )
                            {
                            case 0:    index = 0; break;
                            case 320:  index = 1; break;
                            case 640:  index = 2; break;
                            case 800:  index = 3; break;
                            case 1024: index = 4; break;
                            case 1280: index = 5; break;
                            case 1600: index = 6; break;
                            case 1920: index = 7; break;
                            }
                            gtk_combo_box_set_active( GTK_COMBO_BOX(w), index );
                        }
                        ++r; c=0;
                    }
                
                    update_webphotos_status( wf, w_twothreehq_status_label );
                }
                else
                {
                    l = gtk_label_new("No twothreehq API and secret defined.\n"
                                      "Please obtain an API key and shared secret from:\n"
                                      "http://www.23hq.com/\n"
                                      "and place them in ~/.ferris/23hq-api-key.txt and 23hq-api-secret.txt"
                        );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+2, r, r+1 );
                    ++c;
                }
            }


            {
                r=0; c=0;
                GtkNotebook* pixelpipepage = GTK_NOTEBOOK(gtk_notebook_new());
                gtk_notebook_append_page( photoservers,
                                          GTK_WIDGET(pixelpipepage),
                                          gtk_label_new( "pixelpipe" ));

                fh_webPhotos wf = pixelpipeWebPhotos = Factory::getDefaultPixelPipeWebPhotos();
                if( wf->haveAPIKey() )
                {
                    {
                        page = gtk_table_new ( 2, 2, false );
                        gtk_notebook_append_page( GTK_NOTEBOOK(pixelpipepage), page,
                                                  gtk_label_new( "auth" ));
                    
                        b = gtk_button_new_with_label("Authenticate with pixelpipe");
                        gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                        ++c;
                        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                           GTK_SIGNAL_FUNC(authenticate_with_flickr_cb), GetImpl(wf) );
                        ++r; c=0;
                    
                        l = gtk_label_new ("Status" );
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        l = gtk_label_new ("" );
                        w_pixelpipe_status_label = l;
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    }
                
                    {
                        string s;
                    
                        page = gtk_table_new ( 2, 2, false );
                        gtk_notebook_append_page( GTK_NOTEBOOK(pixelpipepage), page,
                                                  gtk_label_new( "settings" ));
                        r=0; c=0;

                        l = gtk_label_new("Default include EA is Present Regex");
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        w_pixelpipe_include_ea_is_present = e = gtk_entry_new();
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                        s = pixelpipeWebPhotos->getDefaultIncludeEAIsPresentRegexString();
                        gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                        c=0; ++r; 

                        l = gtk_label_new("Default include EA and Value Regex");
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        w_pixelpipe_include_ea_and_value = e = gtk_entry_new();
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
                        s = pixelpipeWebPhotos->getDefaultIncludeEAandValueRegexString();
                        gtk_entry_set_text( GTK_ENTRY(e), s.c_str() );
                        c=0; ++r; 


                        e = gtk_check_button_new_with_label("default permissions public");
                        w_pixelpipe_default_public = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), pixelpipeWebPhotos->isDefaultImageProtectionPublic() );
                        c=0; ++r; 

                        e = gtk_check_button_new_with_label("default permissions friend");
                        w_pixelpipe_default_friend = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), pixelpipeWebPhotos->isDefaultImageProtectionFriend() );
                        c=0; ++r; 

                        e = gtk_check_button_new_with_label("default permissions family");
                        w_pixelpipe_default_family = e;
                        gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
                        gtk_toggle_button_set_active(
                            GTK_TOGGLE_BUTTON(e), pixelpipeWebPhotos->isDefaultImageProtectionFamily() );
                        c=0; ++r; 
                    
                    

                        l = gtk_label_new ("Default upload resize" );
                        gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                        ++c;
                        GtkWidget* w = gtk_combo_box_new_text();
                        w_pixelpipe_default_resize_combo = w;
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "no resize" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "320" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "640" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "800" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1024" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1280" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1600" );
                        gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1920" );
                        gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
                
                        {
                            gint index = -1;
                            long v = wf->getDefaultLargestDimension();
                            switch( wf->getDefaultLargestDimension() )
                            {
                            case 0:    index = 0; break;
                            case 320:  index = 1; break;
                            case 640:  index = 2; break;
                            case 800:  index = 3; break;
                            case 1024: index = 4; break;
                            case 1280: index = 5; break;
                            case 1600: index = 6; break;
                            case 1920: index = 7; break;
                            }
                            gtk_combo_box_set_active( GTK_COMBO_BOX(w), index );
                        }
                        ++r; c=0;
                    }
                
                    update_webphotos_status( wf, w_pixelpipe_status_label );
                }
                else
                {
                    l = gtk_label_new("No pixelpipe API and secret defined.\n"
                                      "Please obtain an API key and shared secret from:\n"
                                      "http://pixelpipe.com/api\n"
                                      "and place them in ~/.ferris/pixelpipe-api-key.txt and pixelpipe-api-secret.txt"
                        );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+2, r, r+1 );
                    ++c;
                }
            }
            

            ////
        }
#endif

#ifdef HAVE_FACEBOOK_API
        {
            GtkNotebook* paned = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "Facebook" ));

            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = 0;
            
            fh_facebook fb = Factory::getFacebook();
            if( fb->haveAPIKey() )
            {
                // auth page...
                {
                    page = gtk_table_new ( 2, 2, false );
                    gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));
                    
                    b = gtk_button_new_with_label("Authenticate with Facebook");
                    gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                    ++c;
                    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                       GTK_SIGNAL_FUNC(authenticate_with_facebook_cb), GetImpl(fb) );
                    ++r; c=0;
                    
                    l = gtk_label_new ("Status" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    l = gtk_label_new ("" );
                    w_facebook_status_label = l;
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                }

                // settings page ...
                {
                    page = gtk_table_new ( 2, 2, false );
                    gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "settings" ));

                    l = gtk_label_new ("Default upload resize" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    GtkWidget* w = gtk_combo_box_new_text();
                    w_facebook_default_resize_combo = w;
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "no resize" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "320" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "640" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "800" );
                    gtk_combo_box_append_text( GTK_COMBO_BOX(w), "1024" );
                    gtk_table_attach_defaults(GTK_TABLE(page), w, c, c+1, r, r+1 );
                
                    {
                        gint index = -1;
                        long v = Facebook::Upload::getMaxDesiredWidthOrHeight();
                        switch( v )
                        {
                        case 0:    index = 0; break;
                        case 320:  index = 1; break;
                        case 640:  index = 2; break;
                        case 800:  index = 3; break;
                        case 1024: index = 4; break;
                        case 1280: index = 5; break;
                        case 1600: index = 6; break;
                        case 1920: index = 7; break;
                        }
                        gtk_combo_box_set_active( GTK_COMBO_BOX(w), index );
                    }
                    ++r; c=0;                    
                    l = gtk_label_new ("" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                }

                update_facebook_status( fb, w_facebook_status_label );
            }
            else
            {
                page = gtk_table_new ( 2, 2, false );
                gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));
                
                l = gtk_label_new("No facebook API and secret defined.\n"
                                  "Please obtain an API key and shared secret from:\n"
                                  "http://wiki.developers.facebook.com/index.php/Connect/Setting_Up_Your_Site\n"
                                  "and place them in ~/.ferris/facebook-api-key.txt and facebook-api-secret.txt"
                    );
                gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+2, r, r+1 );
                ++c;
            }
        }
#endif        


#ifdef HAVE_VIMEO_API
        {
            GtkNotebook* paned = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "Vimeo" ));

            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = 0;

            fh_vimeo v = Factory::getVimeo();
            if( v->haveAPIKey() )
            {
                // auth page...
                {
                    page = gtk_table_new ( 2, 2, false );
                    gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));
                    
                    b = gtk_button_new_with_label("Authenticate with Vimeo");
                    gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                    ++c;
                    gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                       GTK_SIGNAL_FUNC(authenticate_with_vimeo_cb), GetImpl(v) );
                    ++r; c=0;
                    
                    l = gtk_label_new ("Status" );
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                    ++c;
                    l = gtk_label_new ("" );
                    w_vimeo_status_label = l;
                    gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                }

                // settings page ...
                {
                }

                update_vimeo_status( v, w_vimeo_status_label );
            }
            else
            {
                page = gtk_table_new ( 2, 2, false );
                gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));
                
                l = gtk_label_new("No vimeo API and secret defined.\n"
                                  "Please obtain an API key and shared secret from:\n"
                                  "http://www.vimeo.com/api/docs/desktop-auth\n"
                                  "and place them in ~/.ferris/vimeo-api-key.txt and vimeo-api-secret.txt"
                    );
                gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+2, r, r+1 );
                ++c;
            }
        }
#endif        
        

#ifdef HAVE_GOOGLE_API

        // gdrive oauth 2.0
        {
            GtkNotebook* paned = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "GDrive" ));

            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = 0;

            fh_GDriveClient v = GDriveClient::getGDriveClient();
            if( v->haveAPIKey() )
            {
                page = gtk_table_new ( 2, 2, false );
                gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));
                    
                b = gtk_button_new_with_label("Authenticate with GDrive");
                gtk_table_attach_defaults(GTK_TABLE(page), b, c, c+2, r, r+1 );
                ++c;
                gtk_signal_connect(GTK_OBJECT (b), "clicked",
                                   GTK_SIGNAL_FUNC(authenticate_with_gdrive_cb), GetImpl(v) );
                ++r; c=0;
                    
                l = gtk_label_new ("Status" );
                gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
                ++c;
                l = gtk_label_new ("" );
                w_gdrive_status_label = l;
                gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );

                update_gdrive_status( v, w_gdrive_status_label );
            }
        }
        
        // old google        
        {
            GtkNotebook* paned = GTK_NOTEBOOK(gtk_notebook_new());
            gtk_notebook_append_page( main_widget, GTK_WIDGET(paned), gtk_label_new( "Google" ));

            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = 0;

            userpass_t up = getGoogleUserPass( "" );
            string username = up.first;
            string password = up.second;

            page = gtk_table_new ( 2, 2, false );
            gtk_notebook_append_page( GTK_NOTEBOOK(paned), page, gtk_label_new( "auth" ));

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            google_username = e = gtk_entry_new();
            gtk_entry_set_text( GTK_ENTRY(e), username.c_str() );
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            google_password = e = gtk_entry_new();
            gtk_entry_set_text( GTK_ENTRY(e), password.c_str() );
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 
        }
#endif        
        
    
        /* ok / cancel part */

        GtkWidget* vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        GtkWidget* hbx = GTK_WIDGET(gtk_hbox_new(0,0));

        b = gtk_button_new_from_stock( GTK_STOCK_OK );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC(save_and_quit_cb), NULL);

        b = gtk_button_new_from_stock( GTK_STOCK_CANCEL );
        gtk_container_add(GTK_CONTAINER(hbx), GTK_WIDGET( b ));
        gtk_signal_connect(GTK_OBJECT (b), "clicked",
                           GTK_SIGNAL_FUNC(quit_cb), NULL);

        
        // Many pages to handle the different auth info for each server type
        gtk_notebook = GTK_NOTEBOOK(gtk_notebook_new());

#ifdef HAVE_SQLPLUS
        // sqlplus page
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            sql_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            sql_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            b = gtk_toolbar_append_item( tb, "Test",
                                         "try to connect to selected server/databases", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_EXECUTE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(test_cb),
                                         (gpointer)sql_test_fe );
            
            gtk_box_pack_start(GTK_BOX(v), GTK_WIDGET(tb), 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "sqlplus" ));
        }
#endif

#ifdef HAVE_LIBPQXX
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            psql_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            psql_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            b = gtk_toolbar_append_item( tb, "Test",
                                         "try to connect to selected server/databases", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_EXECUTE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(test_cb),
                                         (gpointer)psql_test_fe );
            
            gtk_box_pack_start(GTK_BOX(v), GTK_WIDGET(tb), 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "postgresql" ));
        }
#endif

        // zoneminder
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            zm_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            zm_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            b = gtk_toolbar_append_item( tb, "Test",
                                         "try to connect to selected server/databases", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_EXECUTE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(test_cb),
                                         (gpointer)zm_test_fe );
            
            gtk_box_pack_start(GTK_BOX(v), GTK_WIDGET(tb), 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "zoneminder" ));
        }


        // ferrisrest
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ferrisrest_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ferrisrest_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("REST path");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ferrisrest_baseurl = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 
            
            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            b = gtk_toolbar_append_item( tb, "Test",
                                         "try to connect to selected server", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_EXECUTE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(test_cb),
                                         (gpointer)ferrisrest_test_fe );
            
            gtk_box_pack_start(GTK_BOX(v), GTK_WIDGET(tb), 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "ferrisrest" ));
        }
        
#ifdef HAVE_DTL
        // odbc page
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;
            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            odbc_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            odbc_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));

            GtkToolbar* tb = GTK_TOOLBAR(gtk_toolbar_new());
            b = gtk_toolbar_append_item( tb, "Test",
                                         "try to connect to selected server/databases", "",
                                         gtk_image_new_from_stock(
                                             GTK_STOCK_EXECUTE,
                                             GTK_ICON_SIZE_LARGE_TOOLBAR ),
                                         GTK_SIGNAL_FUNC(test_cb),
                                         (gpointer)odbc_test_fe );
            
            gtk_box_pack_start(GTK_BOX(v), GTK_WIDGET(tb), 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "odbc" ));
        }
#endif
        
#ifdef HAVE_LDAP
        // ldap page
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;

            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ldap_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ldap_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 
            
            l = gtk_label_new("basedn");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            ldap_basedn = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            e = ldap_lookup_basedn = gtk_check_button_new_with_label("lookup basedn from server");
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+2, r, r+1 );
            c=0; ++r; 

            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "ldap" ));
        }
#endif

#ifdef HAVE_OBBY
        // obby page
        {
            int r=0;
            int c=0;
            GtkWidget* l;
            GtkWidget* e;

            GtkWidget* page = gtk_table_new ( 2, 2, false );

            l = gtk_label_new("username");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            obby_username = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 

            l = gtk_label_new("password");
            gtk_table_attach_defaults(GTK_TABLE(page), l, c, c+1, r, r+1 );
            ++c;
            obby_password = e = gtk_entry_new();
            gtk_table_attach_defaults(GTK_TABLE(page), e, c, c+1, r, r+1 );
            c=0; ++r; 
            
            GtkWidget* v = GTK_WIDGET(gtk_vbox_new(0,0));
            gtk_box_pack_start(GTK_BOX(v), page, 0, 0, 0 );
            gtk_box_pack_start(GTK_BOX(v), gtk_label_new(""), 1, 1, 0 );
            gtk_notebook_append_page( gtk_notebook, v, gtk_label_new( "obby" ));
        }
#endif        
        gtk_container_add( GTK_CONTAINER( details_frame ), GTK_WIDGET(gtk_notebook) );
        
//         GtkWidget* topbox = GTK_WIDGET(gtk_hbox_new(0,0));
//         gtk_box_pack_start(GTK_BOX(topbox), list_scrollwin, 1, 1, 0 );
//         gtk_box_pack_start(GTK_BOX(topbox), GTK_WIDGET(gtk_notebook), 0, 0, 0 );
        
//         gtk_box_pack_start(GTK_BOX(vbx), topbox, 1, 1, 0 );
//         gtk_box_pack_start(GTK_BOX(vbx), hbx, 0, 0, 0 );

//         gtk_container_add(GTK_CONTAINER(gtk_window), vbx);


        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(main_widget), 1, 1, 0 );
        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(hbx), 0, 0, 0 );
        gtk_container_add(GTK_CONTAINER(gtk_window), vbx);
        
    }

    int kickit( int argc, char** argv )
    {
        poptContext optCon;

        const char* copt   = "copt";
        const char* AuthWithSite = 0;
        unsigned long iopt = 0;
        unsigned long ListAuthSites = 0;
        const char* AuthService_CSTR = 0;
        const char* AuthUser_CSTR = 0;
        const char* AuthPass_CSTR = 0;
        
        try
        {
            struct poptOption optionsTable[] = {
//             { "root-context-class", 0, POPT_ARG_STRING, &RootContextClass, 0,
//               "Name of the class that handles reading the root context", "Native" },

                { "list-auth-sites", 0, POPT_ARG_NONE, &ListAuthSites, 0,
                  "List the sites which you can authenticate against", "" },

                { "auth-with-site", 0, POPT_ARG_STRING, &AuthWithSite, 0,
                  "Perform web based authentication with the nominated site", "" },


                { "auth-service", 0, POPT_ARG_STRING, &AuthService_CSTR, 0,
                  "set username and password for given service at the supplied Rootname", "" },
                { "auth-user", 0, POPT_ARG_STRING, &AuthUser_CSTR, 0,
                  "set username and password for given service at the supplied Rootname", "" },
                { "auth-pass", 0, POPT_ARG_STRING, &AuthPass_CSTR, 0,
                  "set username and password for given service at the supplied Rootname", "" },
                
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };

            optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
            poptSetOtherOptionHelp(optCon, "[OPTIONS]* RootName1 ...");

            if (argc < 1) {
//          poptPrintHelp(optCon, stderr, 0);
                poptPrintUsage(optCon, stderr, 0);
                exit(1);
            }


            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {
            }
            
            if( AuthService_CSTR && AuthUser_CSTR && AuthPass_CSTR )
            {
                cerr << "have authservice..." << endl;
                
                string AuthService = AuthService_CSTR;
                stringlist_t srcs;
                while( const char* RootNameCSTR = poptGetArg(optCon) )
                {
                    string RootName = RootNameCSTR;
                    srcs.push_back( RootName );
                }

                for( stringlist_t::iterator si = srcs.begin(); si != srcs.end(); ++si )
                {
                    if( AuthService == "wiki" )
                    {
                        setWikiUserPass( *si, AuthUser_CSTR, AuthPass_CSTR );
                    }
                }
                
                exit(0);
            }
            
            


            gtk_init( &argc, &argv );
            
            if( ListAuthSites )
            {
#ifdef HAVE_FACEBOOK_API
                fh_facebook fb = Factory::getFacebook();
                if( fb->haveAPIKey() )
                {
                    cout << "facebook" << endl;
                }
#endif
#ifdef HAVE_WEBPHOTOS_API
                {
                    fh_webPhotos wf = flickrWebPhotos = Factory::getDefaultFlickrWebPhotos();
                    if( wf->haveAPIKey() )
                    {
                        cout << wf->getImplementationShortName() << endl;
                    }
                }
                {
                    fh_webPhotos wf = flickrWebPhotos = Factory::getDefault23hqWebPhotos();
                    if( wf->haveAPIKey() )
                    {
                        cout << wf->getImplementationShortName() << endl;
                    }
                }
                {
                    fh_webPhotos wf = flickrWebPhotos = Factory::getDefaultPixelPipeWebPhotos();
                    if( wf->haveAPIKey() )
                    {
                        cout << wf->getImplementationShortName() << endl;
                    }
                }
#endif         
#ifdef HAVE_VIMEO_API
            fh_vimeo v = Factory::getVimeo();
            if( v->haveAPIKey() )
            {
                cout << "vimeo" << endl;
            }
#endif
#ifdef HAVE_GOOGLE_API
            cout << "google" << endl;
#endif
#ifdef HAVE_BOXCOM_API
            cout << "boxcom" << endl;
#endif
            exit(0);
            }

            if( AuthWithSite )
            {
                string site = AuthWithSite;
#ifdef HAVE_FACEBOOK_API
                if( site == "facebook" )
                {
                    fh_facebook fb = Factory::getFacebook();
                    string authToken = fb->createAuthToken();
//                    string authToken = "af5e832ba06c831cc564ab1aa5254ea6";
                    cerr << "authToken:" << authToken << "___" << endl;

                    stringstream earlss;
                    earlss << "http://www.facebook.com/login.php?v=1.0&auth_token=" << authToken
                           << "&api_key=" << fb->APIKey();
                    string earl = earlss.str();

                    cerr << "Grant Auth following URL..." << endl
                         << endl
                         << earl
                         << endl
                         << endl;
                    cerr << "Then press return to continue..." << endl << flush;
                    string dummy;
                    getline( cin, dummy );
                    cerr << "Getting token..." << endl;

                    string sessionKey = "";
                    
                    if( dummy.length() == 5 )
                        authToken = dummy;
                    
                    sessionKey = fb->getAuthSession( authToken );
                    cerr << "Done..." << endl;
                }
#endif
#ifdef HAVE_WEBPHOTOS_API
                if( site == "flickr" || site == "pixelpipe" || site == "23hq" )
                {
                    bool GUI = false;

                    if( fh_webPhotos wf = Factory::getDefaultWebPhotosForShortName( site ) )
                    {
                        string frob = flickr_request_frob( wf );
                        string earl = createFrobLoginURL( wf, frob );
                        cerr << "Grant Auth following URL..." << endl
                             << endl
                             << earl
                             << endl
                             << endl;
                        cerr << "Then press return to continue..." << endl << flush;
                        string dummy;
                        getline( cin, dummy );
                        cerr << "Getting token..." << endl;
                        // 5. Convert frob to a token
                        convertFrobToToken( wf, frob, GUI );
                        cerr << "Done..." << endl;
                    }
                    else
                    {
                        cerr << "Can not find service:" << site << " check --list-auth-sites" << endl;
                    }
                    
                }
#endif         
#ifdef HAVE_VIMEO_API
                if( site == "vimeo" )
                {
                    fh_vimeo v = Factory::getVimeo();
                    if( v->haveAPIKey() )
                    {
                        std::string earl = v->requestToken();
                        cerr << "Grant Auth following URL..." << endl
                             << endl
                             << earl
                             << endl
                             << endl;
                        cerr << "Then return here and enter the URL you were redirected to..."
                             << endl << flush;
                        std::string verifier;
                        getline( cin, verifier );
                        cerr << "Getting token... v:" << verifier << endl;
                        // Convert frob to a token
                        v->accessToken( verifier );
                        cerr << "isAuthenticated():" << v->isAuthenticated() << endl;
                    }
                }
#endif

#ifdef HAVE_BOXCOM_API
                if( site == "boxcom" )
                {
                    fh_BoxComClient v = BoxComClient::getBoxComClient();
                    if( v->haveAPIKey() )
                    {
                        std::string earl = v->requestToken();
                        cerr << "Grant Auth following URL..." << endl
                             << endl
                             << earl
                             << endl
                             << endl;
                        cerr << "Then return here and enter the \"code\" you obtained from Google..."
                             << endl << flush;
                        std::string code;
                        getline( cin, code );
                        cerr << "Getting auth token... code:" << code << endl;
                        v->accessToken( code );
                        cerr << "isAuthenticated():" << v->isAuthenticated() << endl;

                        cerr << "***********" << endl;
//                        v->ensureAccessTokenFresh( 1 );
                        cerr << "isAuthenticated(2):" << v->isAuthenticated() << endl;
                    }
                }
#endif                
                
#ifdef HAVE_GOOGLE_API
                if( site == "gdrive" )
                {
                    fh_GDriveClient v = GDriveClient::getGDriveClient();
                    if( v->haveAPIKey() )
                    {
                        std::string earl = v->requestToken();
                        cerr << "Grant Auth following URL..." << endl
                             << endl
                             << earl
                             << endl
                             << endl;
                        cerr << "Then return here and enter the \"code\" you obtained from Google..."
                             << endl << flush;
                        std::string code;
                        getline( cin, code );
                        cerr << "Getting auth token... code:" << code << endl;
                        v->accessToken( code );
                        cerr << "isAuthenticated():" << v->isAuthenticated() << endl;
                        cerr << "***********" << endl;
                    }
                }
#endif
#ifdef HAVE_IDENTICA_API
                if( site == "identica" )
                {
                    string AuthService = "identica";
                    if( AuthService_CSTR )
                        AuthService = AuthService_CSTR;
                    cerr << "AuthService:" << AuthService << endl;
                    fh_identica v = Factory::getIdentica( AuthService );
                    if( v->haveAPIKey() )
                    {
                        std::string earl = v->requestToken( "oauth/request_token" );
                        cerr << "Grant Auth following URL..." << endl
                             << endl
                             << earl
                             << endl
                             << endl;
                        cerr << "Then enter verifier pin and press return to continue..." << endl << flush;
                        std::string verifier;
                        getline( cin, verifier );
                        cerr << "Getting token..." << endl;
                        // Convert frob to a token
                        v->accessToken( "oauth/access_token", verifier );
                        cerr << "isAuthenticated():" << v->isAuthenticated() << endl;
                    }
                }
#endif
#ifdef HAVE_GOOGLE_API
                if( site == "google" )
                {
                    cerr << "you have to use the GUI for this, sorry!" << endl;
                }
#endif
                exit(0);
            }
            
            cerr << "starting gui..." << endl;
            
//        gtk_idle_add( fire_ls_cb, (void*)&ls );
            make_window();
            LoadData();
            gtk_widget_show_all( gtk_window );
            gtk_main();
        }
        catch( exception& e )
        {
            cerr << PROGRAM_NAME << ": cought e:" << e.what() << endl;
            exit(1);
        }
        
    
        poptFreeContext(optCon);
        return 0;
    }


};

int main( int argc, char** argv )
{
    return Ferris::kickit( argc, argv );
}
 

