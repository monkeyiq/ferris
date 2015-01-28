/******************************************************************************
*******************************************************************************
*******************************************************************************

    test file touching and context memory management

    Copyright (C) 2005 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: touchtest.cpp,v 1.2 2008/04/27 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/General.hh>

using namespace std;
using namespace Ferris;
using namespace Ferris::Time;

const string PROGRAM_NAME = "touchtest";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main( int argc, const char** argv )
{
    int exit_status = 0;
    const char*   rdn_CSTR  = 0;
    unsigned int  childtest = 0;
    unsigned int  Y         = 0;
    
    struct poptOption optionsTable[] =
        {
            { "touch-child", 'c', POPT_ARG_STRING, &rdn_CSTR, 0,
              "child filename of given URL to touch", "" },

            { "make-many-children", 'C', POPT_ARG_NONE, &childtest, 0,
              "Make many children in given URL and test that libferris knows about them", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]*");

    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }


    while( const char* tmpCSTR = poptGetArg(optCon) )
    {
        try
        {
            string earl = tmpCSTR;
            bool create     = true;
            bool isDir      = false;
            int  mode       = 755;

            if( rdn_CSTR )
            {
                string rdn = rdn_CSTR;

                fh_context c = Resolve( earl );
                string earl = c->getURL() + "/" + rdn;
                Shell::touch( earl, create, isDir, mode );

                fh_context newc = c->getSubContext( rdn );
                DEBUG_dumpcl( "contexts" );
            }
            else if( childtest )
            {
                fh_context c = Resolve( earl );

                for( int i=0; i<1; ++i )
                {
                    stringstream ss;
                    ss <<  "touchfile_" << i;
                    string rdn = ss.str();
                    
                    string earl = c->getURL() + "/" + rdn;
                    {
                        Shell::touch( earl, create, isDir, mode );
                    }
                    
                    
                     {
//                          fh_context r = Shell::acquireContext( earl, mode, isDir );
//                          Shell::CreateFile( r, "test1", mode );
//                         Shell::acquireContext( earl+"/test_1", mode, isDir );
                        
// //                    path:tmp/touchtest/foobar root:file://
// //                        Shell::acquireContext( earl, mode, isDir );
// //                         string rootdir = "file://";
// //                         string path = "tmp/touchtest/foobar";
// //                         fh_context rootc = Resolve( rootdir );
// //                         Shell::CreateFile( rootc, path, mode );
                     }
//                    fh_context newc = c->getSubContext( rdn );
                    
                     fh_context newc = Resolve( earl );
                     cerr << "newc:" << newc->getURL() << endl;
                }
                DEBUG_dumpcl( "contexts" );
//                 getCacheManager()->cleanUp();
//                 DEBUG_dumpcl( "contexts clean" );
                
                exit(0);
            }
            else
            {
                Shell::touch( earl, create, isDir, mode );
            }
        }
        catch( exception& e )
        {
            cerr << "error:" << e.what() << endl;
            exit_status = 1;
        }
    }
    
    poptFreeContext(optCon);
    return exit_status;
}
    
    
