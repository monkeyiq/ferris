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

    $Id: ContextPropertiesEditor.cpp,v 1.6 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include "ContextPropertiesEditor.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <sys/types.h>

#include <Ferris/General.hh>

#include "config.h"

namespace FerrisUI
{
    static const char* GOBJ_GROUP_ID_K = "GOBJ_GROUP_ID_K";
    static const char* GOBJ_INDICATOR_IMG_K = "GOBJ_INDICATOR_IMG_K";

    static string ICON_NO_EXPLICIT = "icons://unknown.png";
    
    
    static string getMimeIcon( string m )
    {
        fh_stringstream ss;
        ss << "mime://" << m;
        fh_context c = Resolve( tostr(ss) );
        return resolveToIconPath( getStrAttr( c, "ferris-iconname", "" ) );
    }
    
    ContextPropertiesEditor::ContextPropertiesEditor()
        :
        m_context( 0 ),
        m_userWidget( 0 ),
        m_manuallySetting( false ),
        m_showURL( false ),
        m_explicitIcon( 0 ),
        m_thumbIcon( 0 )
//         m_recalc_du_sh( 0 ),
//         m_du_sh( 0 ),
//         m_du_sh_runner( 0 )
    {
    }

    void
    ContextPropertiesEditor::setShowURL( bool v )
    {
        m_showURL = v;
    }
    
    static void cpe_mode_toggled( GtkToggleButton *togglebutton, gpointer user_data )
    {
        ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
        cpe->mode_toggled( togglebutton );
    }
    
    void
    ContextPropertiesEditor::mode_toggled( GtkToggleButton *togglebutton )
    {
        if( m_manuallySetting )
            return;

        try
        {
            string modestr = getStrAttr( m_context, "mode", "-1" );
            if( modestr == "-1" )
            {
                fh_stringstream ss;
                ss << "Can not get file mode for context:" << endl
                   << m_context->getURL()
                   << "so a change can not be made to permissions.";
                FerrisUI::RunErrorDialog( tostr(ss) );
            }
        
            int mode = toType<int>( modestr );

            LG_GTKFERRIS_D << "original mode:" << mode << endl;
            
#define TBCOMP( a, b ) (((GtkWidget*)a) == ((GtkWidget*)b))
            if( TBCOMP( togglebutton, m_owner_read ))  mode ^= S_IRUSR;
            if( TBCOMP( togglebutton, m_owner_write )) mode ^= S_IWUSR;
            if( TBCOMP( togglebutton, m_owner_exec ))  mode ^= S_IXUSR;

            if( TBCOMP( togglebutton, m_group_read ))  mode ^= S_IRGRP;
            if( TBCOMP( togglebutton, m_group_write )) mode ^= S_IWGRP;
            if( TBCOMP( togglebutton, m_group_exec ))  mode ^= S_IXGRP;

            if( TBCOMP( togglebutton, m_other_read ))  mode ^= S_IROTH;
            if( TBCOMP( togglebutton, m_other_write )) mode ^= S_IWOTH;
            if( TBCOMP( togglebutton, m_other_exec ))  mode ^= S_IXOTH;
#undef TBCOMP
            
            LG_GTKFERRIS_D << "new mode:" << mode << endl;

            fh_attribute a = m_context->getAttribute("mode");
            fh_iostream ss = a->getIOStream( ios::trunc );
            ss << mode;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Can not set file mode for context:" << endl
               << m_context->getURL()
               << "so a change can not be made to permissions."
               << "e:" << e.what();
            FerrisUI::RunErrorDialog( tostr(ss) );
        }
        updateProtectionLabels();
    }

    static void cpe_time_activated( GtkEntry *entry, gpointer user_data )
    {
        ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
        cpe->time_activated( entry );
    }

    static bool isValidTimeString( const std::string& s )
    {
        bool validTime = isTrue(s);

        if( !validTime )
        {
            try
            {
                struct tm tm = Time::ParseTimeString( s, Time::getDefaultTimeFormat() );
                validTime = true;
            }
            catch( BadlyFormedTimeString& e )
            {
                LG_GTKFERRIS_D << "isValidTimeString() failing time:" << s << endl;
                validTime = false;
            }
        }
        
        return validTime;
    }
    
    void
    ContextPropertiesEditor::updateTime( GtkEntry *entry,
                                         const std::string& eaname,
                                         const std::string& val )
    {
        if( m_manuallySetting )
            return;
        
        LG_GTKFERRIS_D << "updateTime() eaname:" << eaname << " value:" << val << endl;

        GtkImage* indicator = (GtkImage*)
            g_object_get_data( G_OBJECT(entry), GOBJ_INDICATOR_IMG_K );
            
        if( !isValidTimeString( val ) )
        {
            gtk_image_set_from_stock( indicator, GTK_STOCK_NO, GTK_ICON_SIZE_MENU );
            return;
        }
        gtk_image_set_from_stock( indicator, GTK_STOCK_YES, GTK_ICON_SIZE_MENU );
        
        try
        {
            fh_attribute a = m_context->getAttribute( eaname );
            fh_iostream ss = a->getIOStream( ios::trunc );
            ss << val;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "Can not update time value for " << eaname
               << "e:" << e.what();
            FerrisUI::RunErrorDialog( tostr(ss) );
        }
        gtk_label_set_label( m_ctime,
                             getStrAttr( m_context, "ctime-display", "" ).c_str());
    }
    

    string
    ContextPropertiesEditor::getTimeEAName( GtkEntry* entry )
    {
        string eaname = "";
#define TWCOMP( a, b ) (((GtkWidget*)a) == ((GtkWidget*)b))
        if( TWCOMP( entry, m_atime )) eaname = "atime-display";
        if( TWCOMP( entry, m_mtime )) eaname = "mtime-display";
#undef TWCOMP
        return eaname;
    }
    
    void
    ContextPropertiesEditor::time_activated( GtkEntry *entry )
    {
        if( m_manuallySetting )
            return;
        
        string eaname = getTimeEAName( entry );
        if( !eaname.length() )
            return;

        string val = tostr(entry);
        updateTime( entry, eaname, val );
    }

    static void cpe_time_changed( GtkEditable* editable, gpointer user_data )
    {
        ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
        cpe->time_changed( editable );
    }

    void
    ContextPropertiesEditor::time_changed( GtkEditable* editable )
    {
        LG_GTKFERRIS_D << "time_changed()" << endl;
        
        if( m_manuallySetting )
            return;
        
        string eaname = getTimeEAName( GTK_ENTRY(editable) );
        if( !eaname.length() )
            return;

        string val = tostr( GTK_ENTRY(editable) );
        updateTime( GTK_ENTRY(editable), eaname, val );
    }
    
    
    
    GtkWidget*
    ContextPropertiesEditor::makeProtectionCheckBox()
    {
        GtkWidget* w = gtk_check_button_new();
        g_signal_connect_data( G_OBJECT( w ), "toggled",
                               G_CALLBACK (cpe_mode_toggled), this, 0, GConnectFlags(0));
        return w;
    }


    GtkMenu* 
    makeGroupsMenu( GtkMenu* menu )
    {
        GtkMenuShell* m = GTK_MENU_SHELL(menu);
        const int maxGroups = 100;
        gid_t grouplist[ maxGroups + 1 ];
        int numGroups = 0;
        GtkWidget* w;

        numGroups = getgroups( maxGroups, grouplist );
        
        for( int i=0; i<numGroups; ++i )
        {
            struct group* gr = getgrgid( grouplist[ i ] );
//          w = makeMenuItem( gr->gr_name, "" );
            w = gtk_menu_item_new_with_label( gr->gr_name );
            
            gtk_menu_shell_append( m, w );
            g_object_set_data( G_OBJECT(w), GOBJ_GROUP_ID_K, (void*)grouplist[ i ] );
        }
        
        return menu;
    }

    void
    ContextPropertiesEditor::updateGroupWidget()
    {
        GtkMenu* menu = m_group->getMenu();
        if( menu )
            gtk_widget_destroy( GTK_WIDGET( menu ));

        menu = GTK_MENU( gtk_menu_new() );
        makeGroupsMenu( menu );
        m_group->setMenu( menu );

        if( isBound( m_context ))
            m_group->setLabel( getStrAttr( m_context, "group-owner-name", "" ));
    }
    
    
    bool
    ContextPropertiesEditor::OnChangeGroup(
        FerrisOptionMenu* fopm,
        string oldLabel,
        string newLabel )
    {
        if( m_manuallySetting )
            return false;
        
        try
        {
            fh_attribute a = m_context->getAttribute( "group-owner-name" );
            fh_iostream ss = a->getIOStream( ios::trunc );
            ss << newLabel;
        }
        catch( exception& e )
        {
            updateGroupWidget();

            fh_stringstream ss;
            ss << "Can not change the group of:"
               << m_context->getURL()
               << "e:" << e.what();
            FerrisUI::RunErrorDialog( tostr(ss) );
            return true;
        }
        return false;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    enum
    {
        TARGET_FERRIS_URL_LIST = 0,
        TARGET_STRING
    };

    static GtkTargetEntry row_targets[] = {
        { "text/ferris-url-list", 0,        TARGET_FERRIS_URL_LIST },
        { "STRING",               0,        TARGET_STRING },
        { "text/plain",           0,        TARGET_STRING },
        { "text/uri-list",        0,        TARGET_STRING },
    };
    
    static void  
    cpe_mimeicon_drag_data_received(
        GtkWidget          *widget,
        GdkDragContext     *context,
        gint                x,
        gint                y,
        GtkSelectionData   *data,
        guint               info,
        guint               time,
        gpointer            udata)
    {
        ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)udata;
        cpe->mimeicon_drag_data_received( widget, context, x, y, data, info, time );
    }
    
    void  
    ContextPropertiesEditor::mimeicon_drag_data_received(
        GtkWidget          *widget,
        GdkDragContext     *context,
        gint                x,
        gint                y,
        GtkSelectionData   *data,
        guint               info,
        guint               time )
    {
        bool drag_finished = false;
        
        if ((data->length < 0) || (data->format != 8))
        {
            gtk_drag_finish (context, FALSE, FALSE, time);
            return;
        }

        LG_GTKFERRIS_D << "mimeicon_drag_data_received()" << endl;

        try
        {
            
            if( TARGET_FERRIS_URL_LIST == info && m_context )
            {
                string s;
                fh_stringstream ss;
                ss << ((char*)data->data);
                gtk_drag_finish (context, TRUE, FALSE, time);
                drag_finished = true;

                if( GTK_WIDGET(widget) == GTK_WIDGET(m_explicitIcon) )
                {
                    string newIconPath = "";
                    if( getline( ss, s ))
                    {
                        fh_context iconc = Resolve( s );
                        newIconPath = iconc->getDirPath();
                        LG_GTKFERRIS_D << "setting icon to:" << iconc->getDirPath() << endl;
                        LG_GTKFERRIS_D << " for context:" << m_context->getURL() << endl;

                        setStrAttr( m_context, "ferris-iconname", iconc->getDirPath(), true, true );
                    }
                    if( !newIconPath.empty() )
                        gtk_image_set_from_file( m_explicitIcon, newIconPath.c_str());
                }
                else if( GTK_WIDGET(widget) == GTK_WIDGET(m_mimetypeIcon) )
                {
                    string mimetype = getMimeName( m_context );

                    if( getline( ss, s ))
                    {
                        LG_GTKFERRIS_D << "mimeicon_drag_data_received()"
                             << " new image name:" << s
                             << endl;
                        fh_context iconc = Resolve( s );

                        fh_stringstream ss;
                        ss << "mime://" << mimetype;
                        fh_context c = Shell::acquireContext( tostr(ss) );

                        setStrAttr( c, "ferris-iconname", iconc->getDirPath(), true );
                    }
                    gtk_image_set_from_file( m_mimetypeIcon, getMimeIcon(mimetype).c_str());
                }
            }
            else
            {
                fh_stringstream ss;
                ss << "Got info:" << info << " data:" << ((char*)data->data) << endl;
                RunInfoDialog( tostr(ss) );
            }
        }
        catch( exception& e )
        {
            if( !drag_finished )
            {
                drag_finished = true;
                gtk_drag_finish (context, TRUE, FALSE, time);
            }
            
            fh_stringstream ss;
            ss << "Failed to set icon\n"
               << "reason:" << e.what() << endl;
//            cerr << tostr(ss);
            FerrisUI::RunErrorDialog( tostr(ss) );
        }

        if( !drag_finished )
            gtk_drag_finish (context, TRUE, FALSE, time);
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//     static void recalc_du_sh_cb( GtkButton *button, gpointer user_data )
//     {
//         ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
//         cpe->recalc_du_sh( button );
//     }

//     static gboolean
//     recalc_du_sh_complete_cb( GIOChannel *source,
//                            GIOCondition condition,
//                            gpointer user_data )
//     {
//         ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
//         return cpe->recalc_du_sh_complete( source, condition );
//     }

//     gboolean
//     ContextPropertiesEditor::recalc_du_sh_complete( GIOChannel *source,
//                                                     GIOCondition condition )
//     {
//         string s;
//         getline( m_du_sh_fromChildss, s );
//         //m_du_sh_fromChildss >> s;
//         LG_GTKFERRIS_D << "ContextPropertiesEditor::recalc_du_sh_complete() s:" << s << endl;
//         gtk_label_set_label( m_du_sh, s.c_str() );

//         //
//         // Drain input
//         // 
//         fh_stringstream ss;
//         std::copy( std::istreambuf_iterator<char>(m_du_sh_fromChildss),
//                    std::istreambuf_iterator<char>(),
//                    std::ostreambuf_iterator<char>(ss));

        
//         return 0; // dont call again
//     }
    
//     void
//     ContextPropertiesEditor::recalc_du_sh( GtkButton *button )
//     {
//         fh_stringstream ss;
//         fh_runner r = new Runner();
//         m_du_sh_runner = r;
        
// //        ss << " bash -c 'du -sh " << Shell::quote(m_context->getDirPath()) << " '";
// //        ss << "du -sh " << Shell::quote(m_context->getDirPath());
// //        LG_GTKFERRIS_D << "ContextPropertiesEditor::recalc_du_sh() cmd:" << tostr(ss) << endl;
// //        r->setCommandLine( tostr(ss) );

//         r->getArgv().push_back( "du" );
//         r->getArgv().push_back( "-s" );
//         r->getArgv().push_back( "-h" );
//         r->getArgv().push_back( "-s" );
//         r->getArgv().push_back( m_context->getDirPath() );
        
//         r->setSpawnFlags(
//             GSpawnFlags(
//                 G_SPAWN_SEARCH_PATH |
//                 G_SPAWN_FILE_AND_ARGV_ZERO |
//                 G_SPAWN_STDERR_TO_DEV_NULL |
//                 r->getSpawnFlags()));
//         r->Run();
//         m_du_sh_fromChildss = r->getStdOut();
        
//         //
//         // We need to attach a callback for when data is available from the
//         // child.
//         //
//         guint result;
//         GIOChannel *channel;
//         GIOCondition cond = GIOCondition(G_IO_IN | G_IO_ERR | G_IO_PRI);

//         channel = g_io_channel_unix_new( r->getStdOutFd() );
//         result  = g_io_add_watch( channel, cond, recalc_du_sh_complete_cb, this );
//         g_io_channel_unref (channel);
        
//         gtk_label_set_label( m_du_sh, "working..." );
//     }

//     static gint du_sh_button_release_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
//     {
//         ContextPropertiesEditor* cpe = (ContextPropertiesEditor*)user_data;
//         cpe->du_sh_button_release( widget, event );
//     }

//     gint
//     ContextPropertiesEditor::du_sh_button_release( GtkWidget *widget, GdkEvent *event )
//     {
//         GdkEventButton *event_button;

//         g_return_val_if_fail (widget != NULL, FALSE);
//         g_return_val_if_fail (event != NULL, FALSE);
        
//         if (event->type == GDK_BUTTON_PRESS)
//         {
//         }
//         if( event->type == GDK_BUTTON_RELEASE )
//         {
//             event_button = (GdkEventButton *) event;

//             if (event_button->button == 3 && m_du_sh_runner )
//             {
//                 m_du_sh_runner->killChild();
//                 gtk_label_set_label( m_du_sh, "killed." );
//             }
//         }
//         return FALSE;
//     }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    fh_runner
    ContextPropertiesEditor::getRunner_du_sh( fh_SubprocessButtonAndLabel sp, fh_runner )
    {
        if( !m_context )
            return 0;

        fh_runner rr = new Runner();
        
        rr->getArgv().push_back( "du" );
        rr->getArgv().push_back( "-s" );
        rr->getArgv().push_back( "-h" );
        rr->getArgv().push_back( "-s" );
        rr->getArgv().push_back( m_context->getDirPath() );
        
        rr->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                rr->getSpawnFlags()));

        return rr;
    }
    
    
    void
    ContextPropertiesEditor::ensureWidgetsCreated()
    {
        if( m_userWidget )
            return;

        int r=0;
        GtkWidget* lab;
        GtkWidget* w;
        GtkWidget* b;
        FerrisOptionMenu* fopm;
        GtkWidget*   menubar;
        GtkMenu*     menu;
        GtkMenuItem* mi;
        GtkWidget* vbx;

        m_userWidget = vbx = GTK_WIDGET(gtk_vbox_new(0,0));
        
        
        m_table  = gtk_table_new ( 18, 4, false );

        r=0;
        w = gtk_label_new("");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );

        if( m_showURL )
        {
            ++r;
            w = gtk_label_new("url");
            m_url = GTK_LABEL( w );
            gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        }
        
        ++r;
        w = gtk_label_new("Type icon");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        w = gtk_image_new_from_file( resolveToIconPath("icons://unknown.png").c_str());
        m_mimetypeIcon = GTK_IMAGE( w );
        /* Drop Target */
        gtk_drag_dest_set ( GTK_WIDGET(w),
                            GTK_DEST_DEFAULT_ALL,
                            row_targets,
                            G_N_ELEMENTS (row_targets),
                            GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
        gtk_signal_connect( GTK_OBJECT(w), "drag_data_received",
                            GTK_SIGNAL_FUNC( cpe_mimeicon_drag_data_received ), this );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        w = gtk_label_new("");
        m_mimetype = GTK_LABEL( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );


        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );
        ++r;
        w = gtk_label_new("Explicit icon");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        w = gtk_image_new_from_file( resolveToIconPath( ICON_NO_EXPLICIT ).c_str());
        m_explicitIcon = GTK_IMAGE( w );
        /* Drop Target */
        gtk_drag_dest_set ( GTK_WIDGET(w),
                            GTK_DEST_DEFAULT_ALL,
                            row_targets,
                            G_N_ELEMENTS (row_targets),
                            GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
        gtk_signal_connect( GTK_OBJECT(w), "drag_data_received",
                            GTK_SIGNAL_FUNC( cpe_mimeicon_drag_data_received ), this );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );


#ifdef HAVE_LIBEXIF
        ++r;
        w = gtk_label_new("stored thumbnail");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        w = gtk_image_new_from_file( resolveToIconPath( ICON_NO_EXPLICIT ).c_str() );
        m_thumbIcon = GTK_IMAGE( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );
#endif

        ++r;
        w = gtk_label_new("");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 4, r, r+1 );
        
        
        ++r;
        w = gtk_label_new("Owner");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_label_new("X");
        m_owner = GTK_LABEL( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 4, r, r+1 );

        ++r;
        w = gtk_label_new("Group");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );

        m_group = fopm = new FerrisOptionMenu("");
        updateGroupWidget();
        fopm->getLabelChangeSig().connect( sigc::mem_fun( *this,
                                                       &ContextPropertiesEditor::OnChangeGroup ));
        gtk_table_attach_defaults(GTK_TABLE( m_table ), fopm->getWidget(), 1, 4, r, r+1 );

        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );
        
        ++r;
        w = gtk_label_new("Read");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 2, r, r+1 );
        w = gtk_label_new("Write");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 2, 3, r, r+1 );
        w = gtk_label_new("Exec");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );

        ++r;
        w = gtk_label_new("Owner:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        m_owner_read  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 2, r, r+1 );
        m_owner_write = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 2, 3, r, r+1 );
        m_owner_exec  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );

        ++r;
        w = gtk_label_new("Group:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        m_group_read  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 2, r, r+1 );
        m_group_write = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 2, 3, r, r+1 );
        m_group_exec  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );

        ++r;
        w = gtk_label_new("Other:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        m_other_read  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 2, r, r+1 );
        m_other_write = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 2, 3, r, r+1 );
        m_other_exec  = w = makeProtectionCheckBox();
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );
        
        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );

        ++r;
        w = gtk_label_new("protection:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_label_new("X");
        m_protection_ls_text = GTK_LABEL( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 4, r, r+1 );

        ++r;
        w = gtk_label_new("octal:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_label_new("X");
        m_protection_ls_octal = GTK_LABEL( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 4, r, r+1 );

        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );

        ++r;
        w = gtk_label_new("atime:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_entry_new();
        m_atime = GTK_ENTRY( w );
        g_signal_connect_data( G_OBJECT( w ), "changed",
                               G_CALLBACK( cpe_time_changed ), this,
                               0, GConnectFlags(0));
        g_signal_connect_data( G_OBJECT( w ), "activate",
                               G_CALLBACK( cpe_time_activated ), this,
                               0, GConnectFlags(0));
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 3, r, r+1 );
        w = gtk_image_new_from_stock( GTK_STOCK_YES, GTK_ICON_SIZE_MENU );
        m_atime_is_valid = GTK_IMAGE( w );
        g_object_set_data( G_OBJECT(m_atime), GOBJ_INDICATOR_IMG_K, w );
        
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );
        
        ++r;
        w = gtk_label_new("mtime:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_entry_new();
        m_mtime = GTK_ENTRY( w );
        g_signal_connect_data( G_OBJECT( w ), "changed",
                               G_CALLBACK( cpe_time_changed ), this,
                               0, GConnectFlags(0));
        g_signal_connect_data( G_OBJECT( w ), "activate",
                               G_CALLBACK( cpe_time_activated ), this,
                               0, GConnectFlags(0));
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 3, r, r+1 );
        w = gtk_image_new_from_stock( GTK_STOCK_YES, GTK_ICON_SIZE_MENU );
        m_mtime_is_valid = GTK_IMAGE( w );
        g_object_set_data( G_OBJECT(m_mtime), GOBJ_INDICATOR_IMG_K, w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 3, 4, r, r+1 );

        ++r;
        w = gtk_label_new("ctime:");
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
        w = gtk_label_new("");
        m_ctime = GTK_LABEL( w );
        gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 4, r, r+1 );

        ++r;
        gtk_table_attach_defaults(GTK_TABLE(m_table), gtk_hseparator_new(), 0, 4, r, r+1 );

        
        ++r;
        {
            m_du_sh = new SubprocessButtonAndLabel( 0, "size" );
            m_du_sh->getUpdateRunnerSig().connect( sigc::mem_fun( *this, &_Self::getRunner_du_sh) );
            m_du_sh->setReplaceRegex( "(\\S+)\\s.*" );
            m_du_sh->setReplaceRegexFormat( "$1" );

            gtk_table_attach_defaults(GTK_TABLE(m_table), m_du_sh->getButton(), 0, 1, r, r+1 );
            gtk_table_attach_defaults(GTK_TABLE(m_table), m_du_sh->getLabel(),  1, 4, r, r+1 );
        }
        
//         ++r;
//         w = gtk_button_new_with_label("size");
//         m_recalc_du_sh = GTK_BUTTON( w );
//         gtk_signal_connect(GTK_OBJECT (w), "clicked",
//                            GTK_SIGNAL_FUNC(recalc_du_sh_cb), this );
//         gtk_signal_connect(GTK_OBJECT(w), "button_release_event",
//                            GTK_SIGNAL_FUNC (du_sh_button_release_cb), this );
//         gtk_widget_add_events( GTK_WIDGET( w ),
//                                GdkEventMask( GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK ));
        
//         gtk_table_attach_defaults(GTK_TABLE(m_table), w, 0, 1, r, r+1 );
//         w = gtk_label_new("...");
//         m_du_sh = GTK_LABEL( w );
//         gtk_table_attach_defaults(GTK_TABLE(m_table), w, 1, 4, r, r+1 );

        

        gtk_box_pack_start(GTK_BOX(vbx), GTK_WIDGET(m_table), 0, 0, 0 );
    }

    void
    ContextPropertiesEditor::updateProtectionLabels()
    {
        fh_context c = m_context;
        
        gtk_label_set_label( m_protection_ls_text,  getStrAttr( c, "protection-ls", "" ).c_str());
        fh_stringstream ss;
        ss << oct << toint(getStrAttr( c, "mode", "0" ));
        gtk_label_set_label( m_protection_ls_octal, tostr(ss).c_str());
    }

    void
    ContextPropertiesEditor::setContext( fh_context c )
    {
        m_context = c;

        /* Set all the widgets to the data from the new context */
        m_manuallySetting = true;

        
        LG_GTKFERRIS_D << "ContextPropertiesEditor::setContext() c:" << c->getURL() << endl;
        string mimetype = getMimeName( c );
        
        if( m_showURL )
        {
            gtk_label_set_label( m_url,      c->getURL().c_str() );
        }
        gtk_label_set_label( m_mimetype, mimetype.c_str());
        gtk_image_set_from_file( m_mimetypeIcon, getMimeIcon(mimetype).c_str());
        if( m_explicitIcon )
        {
            try
            {
                string iconpath = getStrAttr( c, "ferris-iconname", ICON_NO_EXPLICIT );
                iconpath = resolveToIconPath( iconpath );
                gtk_image_set_from_file( m_explicitIcon, iconpath.c_str() );
                LG_GTKFERRIS_D << "iconpath:" << iconpath << endl;
            }
            catch(...)
            {}
        }
        if( m_thumbIcon )
        {
            setImageFromExifThumb( m_thumbIcon, c );
        }
        
        
        gtk_label_set_label( m_owner,    getStrAttr( c, "user-owner-name", "" ).c_str());
        m_group->setLabel( getStrAttr( c, "group-owner-name", "" ));

        togButtonSet( m_owner_read,  getStrAttr( c, "user-readable","" ));
        togButtonSet( m_owner_write, getStrAttr( c, "user-writable","" ));
        togButtonSet( m_owner_exec,  getStrAttr( c, "user-executable","" ));

        togButtonSet( m_group_read,  getStrAttr( c, "group-readable","" ));
        togButtonSet( m_group_write, getStrAttr( c, "group-writable","" ));
        togButtonSet( m_group_exec,  getStrAttr( c, "group-executable","" ));

        togButtonSet( m_other_read,  getStrAttr( c, "other-readable","" ));
        togButtonSet( m_other_write, getStrAttr( c, "other-writable","" ));
        togButtonSet( m_other_exec,  getStrAttr( c, "other-executable","" ));
        
        updateProtectionLabels();
        
        gtk_entry_set_text ( m_atime, getStrAttr( c, "atime-display", "" ).c_str());
        gtk_label_set_label( m_ctime, getStrAttr( c, "ctime-display", "" ).c_str());
        gtk_entry_set_text ( m_mtime, getStrAttr( c, "mtime-display", "" ).c_str());

        if( isTrue( getStrAttr( c, "is-file", "0" )))
        {
            m_du_sh->setToFinalState( getStrAttr( c, "size-human-readable", "error" ) );
//            gtk_label_set_label( m_du_sh, getStrAttr( c, "size-human-readable", "error" ).c_str() );
        }
        
        m_manuallySetting = false;
    }
    
    fh_context
    ContextPropertiesEditor::getContext()
    {
        return m_context;
    }
    
    GtkWidget*
    ContextPropertiesEditor::getWidget()
    {
        ensureWidgetsCreated();
        return m_userWidget;
    }
    

    

};
