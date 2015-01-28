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

    $Id: ClipAPI.cpp,v 1.7 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <ClipAPI.hh>
#include <Ferris.hh>
#include <SignalStreams.hh>
#include <iterator>
#include <Runner.hh>
#include <CursorAPI.hh>
#include "Shell.hh"
#include "General.hh"

#include <string.h>

using namespace std;

namespace Ferris
{
    using namespace Cursor;
    
    string contexts_ea_name       = "contexts";
    string action_ea_name         = "action";
    string pasted_ea_name         = "pasted";
    string mime_history_ea_prefix = "mime-history-";
    string cmds_sloth_ea_name        = "commands-use-sloth";
    string cmds_autoclose_ea_name    = "commands-use-auto-close";
    string cmds_extraoptions_ea_name = "commands-use-extra-options";
    
    int MimeHistorySize = 20;

    static bool useXWin( fh_context clip )
    {
        bool ret = true;
        if( !g_getenv("DISPLAY" ) || !strlen(g_getenv("DISPLAY" )))
        {
            ret = false;
        }
        LG_FCLIP_D << "useXWin() ret:" << ret << endl;
        return ret;
    }
    
    static void ensureEAExists( fh_context clip, const std::string& eaname )
    {
        if( !clip->isAttributeBound( eaname ) )
        {
            Shell::createEA( clip, eaname, "" );
        }
    }

    static void clearClip( fh_context clip )
    {
        setStrAttr( clip, contexts_ea_name, "",  true );
        setStrAttr( clip, pasted_ea_name,   "0", true );
    }
    
    static void setAction( fh_context clip, const std::string& newAction = "none" )
    {
        string oldAction = getStrAttr( clip, action_ea_name, "none" );
//         cerr << "--- setAction() oldAction:" << oldAction
//              << " newAction:" << newAction << endl;

        ensureEAExists( clip, contexts_ea_name );

        if( "1" == getStrAttr( clip, pasted_ea_name, "1" ))
        {
            clearClip( clip );
        }
        
//         if( oldAction != "none" && oldAction != newAction )
//         {
//             if( newAction == "copy" || newAction == "cut" )
//             {
//                 cerr << "---Trunc for new operation" << endl;
// //                fh_iostream ioss = clip->getIOStream( ios::trunc );
//                 setStrAttr( clip, contexts_ea_name, "" );
//             }
//         }

//         cerr << "--- Setting action attribute to:" << newAction
//              << " clip:" << clip->getURL()
//              << " eaname:" << action_ea_name
//              << endl;
        setStrAttr( clip, action_ea_name, newAction, true );
//         cerr << "--- Read back action:";
//         cerr << getStrAttr( clip, action_ea_name, "" ) << endl;
    }

    static void appendContext( fh_context clip, fh_context c )
    {
//         cerr << "---Append context c:" << c->getURL() << endl;
//        fh_iostream ioss = clip->getIOStream( ios::ate );
        ensureEAExists( clip, contexts_ea_name );
        fh_attribute a   = clip->getAttribute( contexts_ea_name );
        {
            fh_iostream ioss = a->getIOStream( ios::ate );
//             cerr << " -- tellp: " << ioss->tellp() << endl;
            ioss << c->getURL() << endl;
        }
        {
            fh_istream iss = a->getIStream();
//             cerr << "---Read back:" << StreamToString(iss) << endl;
        }
    }


    static void exe_sync( fh_context fclip, const std::string& cmd_const )
    {
        std::string cmd = cmd_const;
        
        if( !useXWin( fclip ) )
        {
            string p;

            LG_FCLIP_D << "exe_sync not using xwin. initial cmd:" << cmd << endl;
            
            p = "gfcp";
            if( starts_with( cmd, p ) )
            {
                cmd = "ferriscp " + cmd.substr( p.length()+1 );
            }
            p = "gfmv";
            if( starts_with( cmd, p ) )
            {
                cmd = "ferrismv " + cmd.substr( p.length()+1 );
            }
            p = "gfrm";
            if( starts_with( cmd, p ) )
            {
                cmd = "ferrisrm " + cmd.substr( p.length()+1 );
            }

            LG_FCLIP_D << "exe_sync not using xwin. final cmd:" << cmd << endl;
        }
        
        fh_runner r = new Runner();
        r->setCommandLine( cmd );
        r->setSpawnFlags(
            GSpawnFlags(
                G_SPAWN_SEARCH_PATH |
                G_SPAWN_STDERR_TO_DEV_NULL |
                G_SPAWN_STDOUT_TO_DEV_NULL |
                r->getSpawnFlags()));
//         cerr << "WOULD BE running:" << r->getDescription() << endl;
        r->Run();
//        r->getExitStatus();
    }
    
    namespace Factory
    {
        fh_context getFileClipboard( std::string s )
        {
            if( !s.length() )
            {
                s = "primary";
            }
            
            fh_stringstream ss;
            ss << "file-clipboard://" << s;
            LG_FCLIP_D << "Getting context at ss:" << tostr(ss) << endl;
            try
            {
                fh_context ret = Resolve( tostr(ss) );
                LG_FCLIP_D << "Resolved file clipboard at url:" << ret->getURL() << endl;
                return ret;
            }
            catch( NoSuchSubContext& e )
            {
//                 cerr << "Clipbard does not exist! creating..."
//                            << " s:" << s << endl;
                LG_FCLIP_D << "Clipbard does not exist! creating..."
                           << " s:" << s << endl;

                fh_context parent = Resolve( tostr(ss), RESOLVE_PARENT );
                fh_context newc = Shell::CreateDir( parent, s );
                {
//                     cerr << "---Trunc new clipboard" << endl;
//                    fh_iostream ioss = newc->getIOStream( ios::trunc );
                    setStrAttr( newc, contexts_ea_name, "", true );
                }
//                 cerr << "newc:" << newc->getURL() << endl;
//                fh_context ret = Resolve( tostr(ss) );
                fh_context ret = Resolve( "~/.ferris/file-clipboard.db/primary" );

                LG_FCLIP_D << "Resolved file clipboard at url:" << ret->getURL() << endl;
                return ret;
            }
            catch( ... )
            {
                throw;
            }
        }
    };
    namespace FileClip
    {
        void Clear( fh_context clip )
        {
            setAction( clip, "none" );
            clearClip( clip );
        }
        
        void Cut(   fh_context clip, fh_context c )
        {
            LG_FCLIP_D << "cut operation clip:" << clip->getURL()
                       << " c:" << c->getURL() << endl;
            setAction( clip, "cut" );
            appendContext( clip, c );
        }

        void Copy(  fh_context clip, fh_context c )
        {
            LG_FCLIP_D << "copy operation clip:" << clip->getURL()
                       << " c:" << c->getURL() << endl;
            setAction( clip, "copy" );
            appendContext( clip, c );
        }

        void Link(  fh_context clip, fh_context c )
        {
            LG_FCLIP_D << "link operation clip:" << clip->getURL()
                       << " c:" << c->getURL() << endl;
            setAction( clip, "link" );
            appendContext( clip, c );
        }

        static string createMimeHistoryEAName( const std::string& action,
                                               const std::string& mimetype )
        {
            fh_stringstream ss;
            ss << mime_history_ea_prefix << action << "-"
               << Util::replace_all( mimetype, '/', '-' );
            return tostr(ss);
        }
        

        static void addToClipMimeHistory( fh_context clip,
                                          fh_context dst,
                                          string action,
                                          string url )
        {
            try
            {
                fh_context child = Resolve( url );
                string mimetype  = getMimeName( child );

                /*
                 * Prepend the destination URL for this mimetype, and truncate
                 * to MimeHistorySize in the EA at the same time.
                 */
                if( mimetype.length() )
                {
                    string s;
                    string eaname = createMimeHistoryEAName( action, mimetype );

//                     cerr << "addToClipMimeHistory() "
//                          << " clip:" << clip->getURL()
//                          << " action:" << action
//                          << " url:" << url
//                          << " dst:" << dst->getURL()
//                          << " eaname:" << eaname
//                          << endl;

                    ensureEAExists( clip, eaname );
                    fh_attribute a   = clip->getAttribute( eaname );
                    {
                        fh_iostream ioss = a->getIOStream();

                        list<string> strlist;
                        for( int i=0; getline( ioss, s ) && i < MimeHistorySize; i++)
                        {
                            strlist.push_back(s);
                        }

                        ioss.clear();
                        ioss.seekp(0);
                        ioss << dst->getURL() << endl;
                        copy( strlist.begin(), strlist.end(), ostream_iterator<string>(ioss, "\n") );
                        ioss << flush;
                    }

//                     cerr << "addToClipMimeHistory(2) "
//                          << " clip:" << clip->getURL()
//                          << " action:" << action
//                          << " url:" << url
//                          << " dst:" << dst->getURL()
//                          << " eaname:" << eaname
//                          << " newv:" << getStrAttr( clip, eaname, "", true )
//                          << endl;
                    
                }
            }
            catch(...)
            {}
            
        }

        void setUseSloth( fh_context clip,     bool v )
        {
            setStrAttr( clip, cmds_sloth_ea_name, tostr(v), true );
        }
        
        void setUseAutoClose( fh_context clip, bool v )
        {
            setStrAttr( clip, cmds_autoclose_ea_name, tostr(v), true );
        }
        
        static string getClipBoardCommandOptions( fh_context clip )
        {
            fh_stringstream ss;
            if( isTrue( getStrAttr( clip, cmds_sloth_ea_name, "" )))
                ss << " --sloth ";
            if( isTrue( getStrAttr( clip, cmds_autoclose_ea_name, "" )))
                ss << " --auto-close ";
            ss << " " << getStrAttr( clip, cmds_extraoptions_ea_name, "" ) << " ";
            return tostr(ss);
        }

        
        
        void Paste( fh_context clip, fh_context dst )
        {
            LG_FCLIP_D << "paste operation clip:" << clip->getURL()
                       << " dst:" << dst->getURL() << endl;

            
            string action = getStrAttr( clip, action_ea_name, "none" );
            if( action == "none" )
            {
                fh_stringstream ss;
                ss << "Paste requested when no previous cut or copy operation performed";
                Throw_FileClipboard( tostr(ss), GetImpl(clip) );
            }
            
            fh_istream iss = clip->getAttribute( contexts_ea_name )->getIStream();
            string s;

            typedef list< string > urllist_t;
            urllist_t urllist;
            while( getline( iss, s ) )
            {
                urllist.push_back( s );
                LG_FCLIP_D << "adding object to paste, url:" << s << endl;
            }

            if( urllist.empty() )
            {
                fh_stringstream ss;
                ss << "No files to paste";
                Throw_FileClipboard( tostr(ss), GetImpl(clip) );
            }
            
            string redocmd  = "gfcp -av ";
            string undocmd  = "gfrm -rv ";
            string redo     = "";
            string undo     = "";
            string dsturl   = dst->getURL();

            /*
             * Remember where each context was moved/copyied based on its mime
             * type and the operation performed
             */
            for( urllist_t::iterator iter = urllist.begin();
                 iter != urllist.end(); ++iter )
            {
                addToClipMimeHistory( clip, dst, action, *iter );
            }
            
            
            if( action == "copy" )
            {
                // action:  gfcp all src to dst;
                // undo  :  gfrm dst/rdn(src)

                redocmd = "gfcp -av ";
                undocmd = "gfrm -rv ";

                /* Make undo command */
                fh_stringstream undoss;
                undoss << undocmd << getClipBoardCommandOptions( clip );
                for( urllist_t::iterator iter = urllist.begin();
                     iter != urllist.end(); ++iter )
                {
                    fh_context src = Resolve( *iter );
                    undoss << " " << Shell::quote( dsturl + "\\/" + src->getDirName() ) << " ";
                }
                undo = tostr( undoss );
            }
            else if( action == "cut" )
            {
                // action:  gfmv all src to dst
                // undo  :  gfmv dst/rdn(src) base(src); gfmv dst/rdn(src) base(src); ...

                redocmd = "gfmv -av ";
                undocmd = "gfmv -av ";

                /* Make undo command */
                fh_stringstream undoss;
                for( urllist_t::iterator iter = urllist.begin();
                     iter != urllist.end(); ++iter )
                {
                    string srcurl = *iter;

                    undoss << undocmd << getClipBoardCommandOptions( clip );
                    int slashloc = srcurl.rfind( "/" );
                    if( string::npos != slashloc )
                    {
                        string rdn  = srcurl.substr( slashloc+1 );
                        string base = srcurl.substr( 0, slashloc );
                        
                        undoss << Shell::quote( dsturl + "\\/" + rdn )
                               << " " << Shell::quote( base ) << endl;
                    }
                }
                undo = StreamToString( undoss );
//                cerr << "+++ undo:" << undo << endl;
            }
            else if( action == "link" )
            {
                // action:  gfcp -s all src to dst;
                // undo  :  gfrm dst/rdn(src)

                redocmd = "gfcp -sv ";
                undocmd = "gfrm -rv ";

                /* Make undo command */
                fh_stringstream undoss;
                undoss << undocmd << getClipBoardCommandOptions( clip );
                for( urllist_t::iterator iter = urllist.begin();
                     iter != urllist.end(); ++iter )
                {
                    fh_context src = Resolve( *iter );
                    undoss << " " << Shell::quote( dsturl + "\\/" + src->getDirName() ) << " ";
                }
                undo = tostr( undoss );
            }
            else
            {
                fh_stringstream ss;
                ss << "Paste requested when no previous cut or copy operation performed";
                Throw_FileClipboard( tostr(ss), GetImpl(clip) );
            }

            /* Make redo command */
            fh_stringstream redoss;
            redoss << redocmd << getClipBoardCommandOptions( clip );
//            copy( urllist.begin(), urllist.end(), ostream_iterator<string>(redoss, " ") );
            for( urllist_t::iterator iter = urllist.begin();
                 iter != urllist.end(); ++iter )
            {
                redoss << " " << Shell::quote( *iter ) << " ";
            }
            redoss << " " << Shell::quote( dsturl ) << " ";
            redo = tostr( redoss );
            
            fh_context cursor = Factory::getCursor( clip );
            fh_context entry  = cursorNext( cursor );

//             cerr << "CURSOR    :" << cursor->getURL() << endl;
//             cerr << "NEW ENTRY :" << entry->getURL() << endl;
            setStrAttr( entry, "redo", redo, true );
            setStrAttr( entry, "undo", undo, true );
            LG_FCLIP_D << "redo command:" << redo << endl;
            LG_FCLIP_D << "undo command:" << undo << endl;
            
            setStrAttr( clip, pasted_ea_name,      "1",           true );
            if( action == "cut" )
            {
                LG_FCLIP_D << "cut operation, clearing clipboard contents..." << endl;
                clearClip( clip );
            }
            
//             cerr << "WOULD BE EXECUTING redo:" << redo << endl;
//            cerr << "Paste command:" << redo << endl;
            LG_FCLIP_D << "performing operation:" << redo << endl;
            exe_sync( clip, redo );
        }

        void Undo( fh_context cursor )
        {
            fh_istream iss = cursor->getAttribute("undo")->getIStream();
            string s;
            Util::SingleShot virgin;
            
            while( getline( iss, s ) )
            {
                exe_sync( cursor->getParent(), s );
                virgin();
            }
            if( virgin.value() )
            {
                fh_stringstream ss;
                ss << "No undo command recorded for file-clipboard:"
                   << cursor->getParent()->getURL()
                   << " for object:" << cursor->getURL();
                Throw_FileClipboard( tostr(ss), GetImpl(cursor) );
            }
            
            
//             string undo  = getStrAttr( cursor, "undo", "" );
//             if( !undo.length() )
//             {
//                 fh_stringstream ss;
//                 ss << "No undo command recorded for file-clipboard:"
//                    << cursor->getParent()->getURL()
//                    << " for object:" << cursor->getURL();
//                 Throw_FileClipboard( tostr(ss), GetImpl(cursor) );
//             }
//             exe_sync( undo );
        }
        
        void Redo( fh_context cursor )
        {
            string redo  = getStrAttr( cursor, "redo", "" );
            if( !redo.length() )
            {
                fh_stringstream ss;
                ss << "No redo command recorded for file-clipboard:"
                   << cursor->getParent()->getURL()
                   << " for object:" << cursor->getURL();
                Throw_FileClipboard( tostr(ss), GetImpl(cursor) );
            }
            exe_sync( cursor->getParent(), redo );
        }

        /**
         * Get a list of URLs that files with the same mimetype as 'c' had
         * the given action performed on them before.
         *
         * ie. If .avi files were moved to 10 places in the past, then we return
         * (in crono order) the list of those 10 places
         *
         * Duplicate URLs are pruned out
         */
        stringlist_t getMimeHistory( fh_context clip,
                                     fh_context c,
                                     std::string action )
        {
            stringlist_t ret;
            
            if( !action.length() )
            {
                stringlist_t cpl = getMimeHistory( clip, c, "cut" );
                stringlist_t mvl = getMimeHistory( clip, c, "copy" );
                copy( cpl.begin(), cpl.end(), back_inserter(mvl));
                return mvl;
            }

            try
            {
                
                string mimetype  = getMimeName( c );
                if( mimetype.length() )
                {
                    string s;
                    string eaname = createMimeHistoryEAName( action, mimetype );
//                     cerr << "getMimeHistory() "
//                          << " clip:" << clip->getURL()
//                          << " c:" << c->getURL()
//                          << " action:" << action
//                          << " eaname:" << eaname
//                          << " value:"  << getStrAttr( clip, eaname, "", true )
//                          << endl;
                    
                    fh_attribute a   = clip->getAttribute( eaname );
                    fh_istream   iss = a->getIStream();
                    while( getline( iss, s ) )
                    {
//                         cerr << "getMimeHistory() s:" << s << endl;
                        if( ret.end() == find( ret.begin(), ret.end(), s ))
                            ret.push_back(s);
                    }
                }
            }
            catch( exception& e )
            {
                cerr << "getMimeHistory() e:" << e.what() << endl;
            }
            catch(...)
            {}
            return ret;
        }
        
        
    };
};

