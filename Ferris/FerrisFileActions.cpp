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

    $Id: FerrisFileActions.cpp,v 1.6 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include "Ferris.hh"
#include "Runner.hh"
#include "FerrisFileActions.hh"
#include "FilteredContext.hh"

#ifdef HAVE_DBUS
#include "DBus_private.hh"
#endif

using namespace std;

namespace Ferris
{
    static string tostr( FileActions::KnownFileOperations opcode )
    {
        string opname = "unknown";

        switch( opcode )
        {
        case FileActions::OP_VIEW:
            opname = "open";
            break;
        case FileActions::OP_EDIT:
            opname = "edit";
            break;
        }

        return opname;
    }

    namespace FileActions
    {
        KnownFileOperations fromString( const std::string& s, KnownFileOperations def )
        {
            KnownFileOperations opcode = def;
            if( s == "view" )
                opcode = OP_VIEW;
            if( s == "edit" )
                opcode = OP_EDIT;

            return opcode;
        }

        
        static long ExecuteExternalApp( std::string exename,
                                        ctxlist_t l ,
                                        bool supportsURL = false ,
                                        bool opensMany = true,
                                        KnownFileOperations opcode = OP_UNKNOWN,
                                        bool openDirWhenNothingSelected = false )
        {
            fh_ExternalAppRunner h = new ExternalAppRunner( exename, supportsURL, opensMany );
            h->setOpCode( opcode );
            h->setOpenDirWhenNothingSelected( openDirWhenNothingSelected );
            return h->run( l );
        }

        static fh_context
        CreateMimeType( const std::string& mimetype,
                        const std::string& mimepostfix )
        {
            fh_stringstream ss;
            ss << mimetype << mimepostfix << endl;
            fh_context c = Resolve( "mime://" );
            c = Shell::CreateDir( c, tostr(ss), true );
            return c;
        }
        

        fh_context getOpenWithContext( const ctxlist_t& l )
        {
            if( l.empty() )
                return 0;
            string mimepostfix = "/actions";

            LG_FILEACTIONS_D << "getOpenWithContext()" << endl;
            
            try
            {
                fh_context frontc = l.front();
                fh_context c = Resolve( "mime://filtered-bindings" );

                LG_FILEACTIONS_D << "getOpenWithContext() have filtered bindings!" << endl;

                Context::iterator ce = c->end();
                for( Context::iterator ci = c->begin(); ci != ce; ++ci )
                {
                    fh_context ffc = *ci;
                    string ffilter = getStrSubCtx( ffc, "ffilter", "" );
                    
                    if( !ffilter.empty() )
                    {
                        fh_context ffilter_c = Factory::MakeFilter( ffilter );
                        fh_matcher m = Factory::MakeMatcherFromContext( ffilter_c );

                        LG_FILEACTIONS_D << "Trying filtered binding at:" << ffc->getURL()
                                         << " ffilter:" << ffilter
                                         << endl;
                        
                        if( m(frontc) )
                        {
                            LG_FILEACTIONS_D << "Found matching filtered binding at:" << ffc->getURL()
                                             << " ffilter:" << ffilter
                                             << endl;
                            fh_context ret = Shell::acquireContext(
                                ffc->getURL() + mimepostfix );
                            return ret;
                        }
                    }
                }
            }
            catch( exception& e )
            {
                LG_FILEACTIONS_D << "getOpenWithContext() filtered bindings e:" << e.what() << endl;
            }
            
            string mimename = getMimeName(l.front());
            
            if( !mimename.length())
            {
                fh_stringstream ss;
                if( l.empty() )
                {
                    ss << "Nothing to get the mime type of." << endl;
                }
                else
                {
                    ss << "Can not determine mime type of\n"
                       << l.front()->getURL()
                       << endl;
                }
                Throw_NoOpenWithContext( tostr(ss), 0 );
            }
        
//        cerr << "ContextView::getOpenWithContext() mime:" << mimename << endl;

            fh_stringstream ss;
            ss << "mime://" << mimename << mimepostfix;
            string mimeURL = tostr(ss);

        
            try
            {
//             cerr << "ContextView::getOpenWithContext() mimeURL:"
//                  << mimeURL << endl;
                fh_context c = Resolve( mimeURL );
                return c;
            }
            catch( exception& e )
            {
                try
                {
                    return CreateMimeType( mimename, mimepostfix );
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Cant setup mime type\n"
                       << "e:" << e.what()
                       << endl;
                    Throw_NoOpenWithContext( tostr(ss), 0 );
                }
            }
            catch(...)
            {}
        
            {
                fh_stringstream ss;
                ss << "No mimetype found for selection.";
                Throw_NoOpenWithContext( tostr(ss), 0 );
            }
        }
        
        long OpenWith( const std::string& n_const, const ctxlist_t& l )
        {
            KnownFileOperations opcode = OP_UNKNOWN;
            string n = n_const;
            if( n == "view" )
            {
                opcode = OP_VIEW;
                n = "open";
            }
            if( n == "edit" )
                opcode = OP_EDIT;
            
            cerr << "ContextView::open_with() n:" << n << endl;
            fh_context mc = getOpenWithContext( l );
            fh_context  c = mc->getSubContext( n );
            string appname = getStrAttr( c, "ferris-appname", "" );
            cerr << "ContextView::open_with() appname:" << appname << endl;
            ExecuteApplicaionByShortcutName( appname, l, opcode );
        }
        
        
        long ExecuteOperation( KnownFileOperations opcode, const ctxlist_t& l )
        {
            if( l.empty() )
                return 0;

            string opname = tostr( opcode );

            fh_context hc = l.front();
            string hc_mimename = getMimeName(hc);
            fh_context mimebindc;

            string s;
            fh_stringstream ss;
            ss << "mime://" << hc_mimename << "/actions/" << opname;
            LG_FILEACTIONS_D << "looking for action at:" << tostr(ss) << endl;

            try
            {
//                 fh_context mc = getOpenWithContext( l );
//                 fh_context  c = mc->getSubContext( opname );
//                 string appname = getStrAttr( c, "ferris-appname", "" );

//                 if( !appname.length() )
//                 {
//                     fh_stringstream ss;
//                     ss << "No " << opname << " action is defined for this type"
//                        <<" mimetype:" << hc_mimename;
//                     Throw_OpenActionIsNotDefined( tostr(ss), 0 );
//                 }
                
//                 ExecuteApplicaionByShortcutName( appname, l );

                fh_context owc = getOpenWithContext( l );
                fh_context mbc = owc->getSubContext( opname );
                
//                fh_context mbc = Resolve( tostr(ss) );
                string appname = getStrAttr( mbc, "ferris-appname", "" );
                LG_FILEACTIONS_D << "appname:" << appname << endl;
            
                if( !appname.length() )
                {
                    fh_stringstream ss;
                    ss << "No " << opname << " action is defined for this type"
                       <<" mimetype:" << hc_mimename;
                    Throw_OpenActionIsNotDefined( tostr(ss), 0 );
                }
                return ExecuteApplicaionByShortcutName( appname, l, opcode );
            }
            catch( OpenActionIsNotDefined& e )
            {
                throw;
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "No " << opname << " action is defined for this type"
                   << " mimetype:" << hc_mimename
                   << " e:" << e.what();
                Throw_OpenActionIsNotDefined( tostr(ss), 0 );
            }
        }
        

        long ExecuteApplicaionByShortcutName( string appname, ctxlist_t l,
                                              KnownFileOperations opcode )
        {
            fh_stringstream ss;
            ss << "applications://" << appname;
            LG_FILEACTIONS_D << "ExecuteApplicaionByShortcutName() app:" << tostr(ss) << endl;
            fh_context ac = Resolve( tostr(ss) );

            string exename      = getStrAttr( ac, "ferris-exe", "" );
            int supportsURL     = toint( getStrAttr( ac, "ferris-handles-urls", "0"));
            int opensMany       = toint( getStrAttr( ac, "ferris-opens-many", "0"));
            int ignoreSelection = toint( getStrAttr( ac, "ferris-ignore-selection", "0"));
            int openDirWhenNothingSelected = toint(
                getStrAttr( ac, "ferris-open-dir-when-no-files-selected", "0"));

            LG_FILEACTIONS_D << "ExecuteApplication() exename:" << exename
                             << " supportsURL:" << supportsURL
                             << " opensMany:" << opensMany
                             << " ignoreSelection:" << ignoreSelection
                             << endl;

            if( string::npos != exename.rfind("\"") )
            {
                exename = exename.substr( 0, exename.rfind("\"") );
            }
            else
            {
                while( string::npos != exename.find("%f") )
                {
                    exename = exename.replace( exename.find("%f"), 2, "" );
                }
            }

            return ExecuteExternalApp( exename,
                                       ignoreSelection ? ctxlist_t() : l,
                                       supportsURL, opensMany, opcode, openDirWhenNothingSelected );
            
        }

        void setApplicationForOperationOnType( const ctxlist_t& l,
                                               const std::string& appname,
                                               KnownFileOperations opcode
            )
        {
            if( opcode == OP_UNKNOWN )
            {
                stringstream ss;
                ss << "Can not set unknown operation for file to app:" << appname;
                Throw_NoSuchObject( tostr(ss), 0 );
            }

            string opname = tostr( opcode );
            fh_context mc = getOpenWithContext( l );
            fh_context c = Shell::acquireSubContext( mc, opname );
            try
            {
                fh_stringstream ss;
                ss << "applications://" << appname;
                LG_FILEACTIONS_D << "setApplicationForOperationOnType()"
                                 << " checking app exists for app:" << tostr(ss) << endl;
                fh_context ac = Resolve( tostr(ss) );
            }
            catch( exception& e )
            {
                stringstream ss;
                ss << "Application doesn't exist! app:" << appname << " e:" << e.what();
                Throw_NoSuchObject( tostr(ss), 0 );
            }
            setStrAttr( c, "ferris-appname", appname, true, true );
        }
        
        void setApplicationForOperationOnType( fh_context samplec,
                                               const std::string& appname,
                                               KnownFileOperations opcode
            )
        {
            ctxlist_t l;
            l.push_back( samplec );
            setApplicationForOperationOnType( l, appname, opcode );
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        ExternalAppRunner::ExternalAppRunner( std::string exename, 
                                              bool supportsURL,
                                              bool opensMany )
            :
            m_exename( exename ),
            m_supportsURL( supportsURL ),
            m_opensMany( opensMany ),
            m_rootContext( 0 ),
            m_opcode( OP_UNKNOWN ),
            m_openDirWhenNothingSelected( false )
        {
            setupDefaultInheritedEnvironmentNames();
        }

        void
        ExternalAppRunner::setupDefaultInheritedEnvironmentNames()
        {
            try
            {
                string envNamesCSV = getConfigString(
                    FDB_GENERAL,
                    "default-inherited-environment-names",
                    "",
                    true );

                set<string> theSet;
                addEAToSet( theSet, envNamesCSV );
                copy( theSet.begin(), theSet.end(), back_inserter( m_environmentVariablesToInherit ));
            }
            catch(...)
            {
                m_environmentVariablesToInherit.push_back( "PATH" );
                m_environmentVariablesToInherit.push_back( "PS1" );
                m_environmentVariablesToInherit.push_back( "PS2" );
                m_environmentVariablesToInherit.push_back( "PS3" );
                m_environmentVariablesToInherit.push_back( "PS4" );
                m_environmentVariablesToInherit.push_back( "TERM" );
                m_environmentVariablesToInherit.push_back( "HOME" );
                m_environmentVariablesToInherit.push_back( "DISPLAY" );
            }
        }
        
        void
        ExternalAppRunner::setSupportsURL( bool v )
        {
            m_supportsURL = v;
        }

        void
        ExternalAppRunner::setOpensMany( bool v )
        {
            m_opensMany = v;
        }

        void
        ExternalAppRunner::setRootContext( fh_context c )
        {
            m_rootContext = c;
        }
        
        stringlist_t&
        ExternalAppRunner::getEnvironmentVariablesToInherit()
        {
            return m_environmentVariablesToInherit;
        }

        void
        ExternalAppRunner::setOpCode( KnownFileOperations opcode )
        {
            m_opcode = opcode;
        }
        void
        ExternalAppRunner::setOpenDirWhenNothingSelected( bool v )
        {
            m_openDirWhenNothingSelected = v;
        }
        
        
        
        long
        ExternalAppRunner::run( const ctxlist_t& l )
        {
            LG_FILEACTIONS_D << "ExternalAppRunner::run(top)" << endl;
            LG_FILEACTIONS_D << "m_openDirWhenNothingSelected:" << m_openDirWhenNothingSelected << endl;
            
            if( l.empty() )
            {
                LG_FILEACTIONS_D << "ExternalAppRunner::run() warning selection is empty..." << endl;
            }
            

            if( l.empty() )
                return 0;

            fh_context hc = l.front();
            LG_FILEACTIONS_D << "ExternalAppRunner::run(2)" << endl;

            if( !m_rootContext )
            {
                if( hc->isParentBound() )
                    m_rootContext = hc->getParent();
                else
                    m_rootContext = hc;
            }
            
            LG_FILEACTIONS_D << "ExternalAppRunner::run(3)" << endl;
            
            typedef ctxlist_t OCL;
            typedef OCL::const_iterator OCLI;
            stringstream cmdss;

            fh_runner r   = new Runner();
            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                           | G_SPAWN_STDOUT_TO_DEV_NULL
                                           | G_SPAWN_STDERR_TO_DEV_NULL
                                           | G_SPAWN_SEARCH_PATH) );
            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() & ~(G_SPAWN_DO_NOT_REAP_CHILD)));

            /*
             * Resolve ~ paths explicitly
             */
            if( starts_with( m_exename, "~/" ))
            {
                m_exename = Shell::getHomeDirPath() + m_exename.substr(1);
            }

            r->setCommandLine( m_exename );
            cmdss << m_exename << " ";

            /****************************************/
            /****************************************/
            /****************************************/
            
            r->setInheritENV( false );
            stringlist_t& envPreserve = getEnvironmentVariablesToInherit();
            for( stringlist_t::iterator iter = envPreserve.begin();
                 iter != envPreserve.end(); ++iter )
            {
                const char* value = g_getenv( iter->c_str() );
                if( value )
                {
                    string e = *iter + "=" + value;
                    LG_FILEACTIONS_D << "ExecuteExternalApp() exporting e:" << e << endl;
                    r->getEnvp().push_back( e );
                }
            }

            string rooturl = m_rootContext->getURL();
            r->getEnvp().push_back( "EGO_SCRIPT_CURRENT_URL="+Shell::quote( rooturl ));

            string rootpath = m_rootContext->getDirPath();
            r->getEnvp().push_back( "EGO_SCRIPT_CURRENT_PATH="+Shell::quote( rootpath ));
            LG_FILEACTIONS_D << "ExecuteExternalApp() setting env var:"
                             << "EGO_SCRIPT_CURRENT_URL=" << Shell::quote( rooturl )
                             << "EGO_SCRIPT_CURRENT_PATH="<< Shell::quote( rootpath )
                             << endl;
            fh_stringstream pathss;
            fh_stringstream urlss;
            fh_stringstream pathssnl;
            fh_stringstream urlssnl;
            
            for( OCLI iter = l.begin(); iter != l.end(); ++iter )
            {
                pathss << " " << Shell::quote( (*iter)->getDirPath() ) << " ";
                urlss  << " " << Shell::quote( (*iter)->getURL() ) << " ";

                pathssnl << (*iter)->getDirPath() << endl;
                urlssnl  << (*iter)->getURL() << endl;
            }

            r->getEnvp().push_back( "EGO_SCRIPT_SELECTED_FILE_PATHS="+tostr(pathss));
            r->getEnvp().push_back( "EGO_SCRIPT_SELECTED_URLS="+tostr(urlss));

            r->getEnvp().push_back( "EGO_SCRIPT_SELECTED_FILE_PATHS_NL="+tostr(pathssnl));
            r->getEnvp().push_back( "EGO_SCRIPT_SELECTED_URLS_NL="+tostr(urlssnl));
            
            LG_FILEACTIONS_D << "ExecuteExternalApp() setting env vars l.size:" << l.size()
                             << " EGO_SCRIPT_SELECTED_FILE_PATHS=" << tostr(pathss)
                             << " EGO_SCRIPT_SELECTED_URLS=" << tostr(urlss)
                             << endl;

            //
            // For some actions, like image viewing, when nothing is explicitly selected
            // we pass in the directory path itself so that all images in the directory
            // can be shown
            //
            if( l.empty() && m_openDirWhenNothingSelected )
            {
                string p;
                
                if( m_supportsURL )
                {
                    p = Shell::quote( rooturl );
                }
                else
                {
                    p = Shell::quote( rootpath );
                }
                LG_FILEACTIONS_D << "adding:" << p << endl;
                r->getArgv().push_back( p );
                cmdss << " \"" << p << "\" ";
            }
            
            /****************************************/
            /****************************************/
            /****************************************/

            for( OCLI iter = l.begin(); iter != l.end(); ++iter )
            {
                string p;
            
                if( m_supportsURL )
                {
                    p = (*iter)->getURL();
                }
                else
                {
                    p = (*iter)->getDirPath();
                }
                LG_FILEACTIONS_D << "adding:" << p << endl;
                r->argvPushPath( p );
                cmdss << " \"" << p << "\" ";
            }

            /****************************************/
            /****************************************/
            /****************************************/

            r->Run();
            LG_FILEACTIONS_D << "execution of app:" << m_exename << " was OK" << endl;

            //
            // Tell libferris to update the file-view EA
            //
            if( m_opcode != OP_UNKNOWN )
            {
                string cmd = cmdss.str();
                for( OCLI iter = l.begin(); iter != l.end(); ++iter )
                {
                    string earl = (*iter)->getURL();
                    if( contains( earl, "remembrance://" ) )
                        continue;

                    switch( m_opcode )
                    {
                    case OP_VIEW:
                        setStrAttr( *iter, "file-view-command", cmd );
#ifdef HAVE_DBUS
                        {
                            LG_DBUS_D << "Emitting desktop.Viewed for earl:" << earl << endl;
                            DBusConnection* conn = DBus::getSessionBus();
                            DBus::Signal sig( "/", "org.libferris.desktop", "Viewed");
                            sig.push_back( earl );
                            sig.push_back( (string)(*iter)->getDirPath() );
                            sig.push_back( cmd );
                            sig.send( conn );
                            LG_DBUS_D << "Done Emitting desktop.Viewed for earl:" << earl << endl;
                        }
#endif                        
                        break;
                    case OP_EDIT:
                        setStrAttr( *iter, "file-edit-command", cmd );
#ifdef HAVE_DBUS
                        {
                            LG_DBUS_D << "Emitting desktop.Edited for earl:" << earl << endl;
                            DBusConnection* conn = DBus::getSessionBus();
                            DBus::Signal sig( "/", "org.libferris.desktop", "Edited");
                            sig.push_back( earl );
                            sig.push_back( (*iter)->getDirPath() );
                            sig.push_back( cmd );
                            sig.send( conn );
                        }
#endif                        
                        break;
                    }
                }
            }
            return 0;
        }
    };
};
