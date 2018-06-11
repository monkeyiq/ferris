/******************************************************************************
*******************************************************************************
*******************************************************************************

    cursor manipulation and reading command line client
    Copyright (C) 2004 Ben Martin

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

    $Id: ferris-cursor.cpp,v 1.3 2010/09/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/CursorAPI.hh>
#include <SignalStreams.hh>

#include <popt.h>
#include <unistd.h>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace Ferris::Cursor;

const string PROGRAM_NAME = "ferris-cursor";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void show( const std::string& msg, fh_context c, const std::string& eanames )
{
    cout << msg << c->getURL() << endl;
    
    stringlist_t sl = Util::parseCommaSeperatedList( eanames );
    for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
    {
        cout << "    " << getStrAttr( c, *si, "" ) << endl;
    }
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    const char*   NewCursorContent_CSTR = 0;
    const char*   NewCursorRdn_CSTR  = 0;
    unsigned long ListMode           = 0;
    unsigned long SetMode            = 0;
    unsigned long Verbose            = 0;
    unsigned long MoveNext           = 0;
    unsigned long MovePrev           = 0;
    unsigned long CursorOptionRemake = 0;
    unsigned long CursorOptionSize   = 0;
    
    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "rdn", 'R', POPT_ARG_STRING, &NewCursorRdn_CSTR, 0,
                  "rdn for what to set the cursor to point to. "
                  "Must be a child of the cursor directory", "" },

                { "list", 'l', POPT_ARG_NONE, &ListMode, 0,
                  "show all the items that the cursor will move to in cursor order", "" },

                { "set", 's', POPT_ARG_NONE, &SetMode, 0,
                  "Set the cursor to the file at prefix", "" },

                { "content", 'c', POPT_ARG_STRING, &NewCursorContent_CSTR, 0,
                  "Set the content of the cursor to the given string."
                  "This is applied after any prev/next movements if they are present", "" },

                { "next", 'n', POPT_ARG_NONE, &MoveNext, 0,
                  "move cursor to next item", "" },

                { "prev", 'p', POPT_ARG_NONE, &MovePrev, 0,
                  "move cursor to prev item", "" },

                { "cursor-option-remake", 0, POPT_ARG_NONE, &CursorOptionRemake, 0,
                  "set the cursor option to always remake the cursor file", "" },

                { "cursor-option-size", 0, POPT_ARG_INT, &CursorOptionSize, 0,
                  "set the cursor option to have the cursor loop size items", "" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {
//         switch (c) {
//         }
        }

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if( SetMode && !NewCursorRdn_CSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
            
        while( const char* tmpCSTR = poptGetArg(optCon) )
        {
            try
            {
                string     srcURL = tmpCSTR;
                fh_context parent = Resolve( srcURL );
                fh_context cursor = Factory::getCursor( parent );

                if( CursorOptionRemake )
                {
                    Config::setCursorOption( parent,
                                             Config::cursor_object_always_remake,
                                             "1" );
                }
                if( CursorOptionSize > 0 )
                {
                    Config::setCursorOption( parent,
                                             Config::cursor_object_list_size,
                                             tostr( CursorOptionSize ) );
                }
                if( ListMode )
                {
                    show( "Cursor itself is at ", cursor, "content" );
                    cout << "--------------------------------------------" << endl;
                    
                    Context::iterator cursor_iter = parent->find( cursor->getDirName() );
                    for( circular_iterator< Context, Context::iterator >
                             ci  = make_circular_iterator( parent, cursor_iter );
                         ci != make_circular_iterator( parent ); ++ci )
                    {
                        show( "", *ci, "content" );
                    }
                }
                else if( SetMode )
                {
                    fh_context newposition = parent->getSubContext( NewCursorRdn_CSTR );
                    cursorSet( cursor, newposition );
                }
                else if( MoveNext )
                {
                    cursor = cursorNext( cursor );
                }
                else if( MovePrev )
                {
                    cursor = cursorPrev( cursor );
                }
                else
                {
                    if( !NewCursorContent_CSTR )
                    {
                        show( "Cursor is at ", cursor, "content" );
                    }
                }
                
                if( NewCursorContent_CSTR )
                {
                    show( "setting data for cursor at ", cursor, "content" );
                    setStrAttr( cursor, "content", NewCursorContent_CSTR, true, true );
                    cout << "read back:" << getStrAttr( cursor, "content", "<error>" ) << endl;
                    
                    try
                    {
                        setStrAttr( cursor, "mtime", Ferris::tostr(time(0)), true, true );
                    }
                    catch( exception& e )
                    {
                        cerr << "ERROR SETTING MTIME! e:" << e.what() << endl;
                    }
                }
            }
            catch( exception& e )
            {
                cerr << "error:" << e.what() << endl;
                exit_status = 1;
            }
        }
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


