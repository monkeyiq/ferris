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

    $Id: Mime.cpp,v 1.3 2010/09/24 21:30:55 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "Ferris.hh"
#include "Mime.hh"
#include "Trimming.hh"

using namespace std;

namespace Ferris
{
    
    static string getDesktopFileValue( fh_context c,
                                       const std::string& k,
                                       const std::string& def = "",
                                       bool throwEx = true )
    {
        fh_istream ss = c->getIStream();
        string s;

        while( getline(ss,s) )
        {
            if( string::npos == s.find("=") )
                continue;
            
            if( s.substr( 0, s.find("=") ) == k )
                return s.substr( s.find("=")+1 );
        }

        if( !throwEx )
            return def;
        
        fh_stringstream ess;
        ess << "Desktop file:" << c->getURL() << "\ncontains no data for key:" << k;
        Throw_DesktopFileKeyNotFound( tostr(ess), 0 );
    }


    /**
     * Import a .desktop file (desktopc) into the location (destc)
     *
     * @param destc The db4 directory where the desktopc file should be imported into
     *              note that this is the parent of where the new context should be created
     * @param desktopc The .desktop file to import
     */
    void importDesktopFileTo( fh_context parentc, fh_context desktopc )
    {
        fh_context dbc = Shell::CreateDir( parentc,
                                           getDesktopFileValue( desktopc, "Name" ) );
        
//        cerr << "importDesktopFile(p,d) dbc:" << dbc->getURL() << endl;

        if( getDesktopFileValue(desktopc,"Type", "", false ) == "Scheme" )
        {
            setStrAttr( dbc, "ferris-scheme",
                        getDesktopFileValue(desktopc,"Exec"), true, true );
        }
        else
        {
            setStrAttr( dbc, "ferris-exe",
                        getDesktopFileValue(desktopc,"Exec"), true, true );
        }            

        setStrAttr( dbc, "ferris-handles-urls",
                    tostr(isTrue(getDesktopFileValue(desktopc,"Handles_Urls", "0",false))),
                    true, true );

        setStrAttr( dbc, "ferris-open-dir-when-no-files-selected",
                    tostr(isTrue(getDesktopFileValue(
                                     desktopc,"Open_Directory_When_No_Files_Selected", "0",false))),
                    true, true );
        
        setStrAttr( dbc, "ferris-opens-many",
                    tostr(isTrue( getDesktopFileValue(desktopc,"MultipleArgs","false",false))),
                    true, true );

        string iconpath = getDesktopFileValue(desktopc,"Icon");
        if( !starts_with( iconpath, "/" )
            && !starts_with( iconpath, "icons://" )
            && !starts_with( iconpath, "file://" )
            && !starts_with( iconpath, "root://" )
            && !starts_with( iconpath, "~" ))
        {
            iconpath = "gnomeicons://" + iconpath;
        }
        setStrAttr( dbc, "ferris-iconname", iconpath, true, true );
    }
    

    /**
     * Import the data from desktopc into the local db4 applications://
     * config section. This is the private version, use the public one!
     */
    static void private_importDesktopFile( fh_context basec, fh_context desktopc )
    {
        string     basep    = basec->getDirPath();
        string     desktopp = desktopc->getDirPath();

        string subdirname = "";
        
        if( desktopc->getParent() != basec )
        {
            subdirname  = desktopc->getParent()->getDirPath().substr( basep.length()+1 );
        }
        
//         cerr << "importDesktopFile() base:" << basep
//              << " desktop:" << desktopp
//              << " subdirname:" << subdirname
//              << " ImportedContext:" << "applications://" << subdirname
//              << endl;

        importDesktopFileTo( Shell::acquireContext( "applications://" + subdirname ),
                             desktopc );
    }

    /*
     * Grab the icon path and other interesting stuff from a .directory file for nice menus.
     *
     * @param basec is the root of the .desktop tree
     * @param c is the .desktop file itself.
     */
    static void importDotDirectoryFile( fh_context basec, fh_context c )
    {
        string basep = basec->getDirPath();
        string path  = c->getDirPath();

        string subdirname = path.substr( basep.length()+1 );
        PostfixTrimmer trimmer;
        trimmer.push_back( "/.directory" );
        subdirname = trimmer( subdirname );
        
        
//         cerr << "importDotDirectoryFile()"
//              << " basep:" << basep
//              << " path:" << path
//              << " subdirname:" << subdirname
//              << endl;
        
        fh_context dbc = Shell::acquireContext( "applications://" + subdirname );
//        cerr << "importDesktopFile() dbc:" << dbc->getURL() << endl;
        
        setStrAttr( dbc, "ferris-iconname", getDesktopFileValue(c,"Icon"), true, true );
        setStrAttr( dbc, "ferris-comment",  getDesktopFileValue(c,"Comment"), true, true );
    }
    
    
    /**
     * Import the .desktop file found at the specified location
     *
     * This function tries to find an implied subdir name from the parent directories
     * of the given .desktop file. If no tree structure can be implied from the given
     * context then the .desktop file is imported into the root of applications://
     *
     * For finer control use importDesktopFileTo() to explicitly say where the new
     * application entry is created
     */
    void importDesktopFile( fh_context c )
    {
        fh_context p = c->getParent();
        
        while( p->isParentBound() )
        {
            bool shouldStop = false;

            if( p->getDirName() == "apps" ||
                p->getDirName() == "applnk" ||
                p->getDirName() == "dot-desktop-files" ||
                p->getDirName() == "Programs" )
                shouldStop = true;
            
            
//             if( p->end() == p->find(".directory") )
//                 shouldStop = true;
//             else
//             {
//                 fh_context dotdc = p->getSubContext(".directory");

//                 if( p->getDirName() == "apps" &&
//                     getDesktopFileValue( dotdc, "Name", "", false ) == "Programs" )
//                 {
//                     shouldStop = true;
//                 }
//             }
            
            if( shouldStop )
            {
//                 cerr << "importDesktopFile(c) going to make the call!"
//                      << " c:" << c->getURL()
//                      << " p:" << p->getURL()
//                      << endl;
                private_importDesktopFile( p, c );

                /* Get all the .directory files too */
                c = c->getParent();
                while( c != p )
                {
                    if( c->isSubContextBound( ".directory" ))
                        importDotDirectoryFile( p, c->getSubContext( ".directory" ) );
                    c = c->getParent();
                }
                return;
            }
            p = p->getParent();
        }

        /*
         * The desktop file was not in a directory hive, its just a solo desktop file
         */
        private_importDesktopFile( c->getParent(), c );
    }
    
};
