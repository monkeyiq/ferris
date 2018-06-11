/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris mv
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

    $Id: ferrismv.cpp,v 1.5 2010/09/24 21:31:18 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris.hh>
#include <FerrisMove.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferrismv";

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
    fh_mv fmv = new FerrisMv();
    
    const char*   DstNameCSTR            = 0;
    
    struct poptOption optionsTable[] =
        {
            { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
              "Specify destination explicity, all remaining URLs are assumed to be source files",
              "DIR" },

            FERRIS_MOVE_OPTIONS( fmv )
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ... dst");

    if (argc < 3)
    {
        poptPrintHelp(optCon, stderr, 0);
//        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }
    
    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    fmv->getPoptCollector()->ArgProcessingDone( optCon );
    
    try
    {
        typedef stringlist_t srcs_t;
        srcs_t srcs;

        srcs = expandShellGlobs( srcs, optCon );
        
        if( srcs.empty() )
        {
            cerr << "No objects need to be moved\n" << endl;
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

        fmv->setDstURL( DstName );

        /*
         * See if user is smoking crack
         */
        if( srcs.empty() )
        {
            cerr << "not enough source urls given" << endl;
            poptPrintUsage(optCon, stderr, 0);
            exit(1);
        }

        bool MovingToDir = srcs.size() > 1;
        try
        {
            fh_context c = Resolve( DstName );
            if( toint( getStrAttr( c, "is-dir-try-automounting", "0" )))
            {
                MovingToDir = true;
            }
        }
        catch( NoSuchSubContext& e )
        {
        }
        fmv->setMovingToDir( MovingToDir );

        
//        cerr << "dst:" << DstName << endl;

        for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
        {
            string SrcName = *iter;
//            cerr << "src:" << SrcName << endl;

            fmv->setSrcURL( SrcName );
            fmv->move();
        }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
