/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cp
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

    $Id: ferriscp.cpp,v 1.7 2010/09/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <FerrisCopy.hh>
#include <FerrisCopy_private.hh>
#include <SyncDelayer.hh>

#include <popt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;


const string PROGRAM_NAME = "ferriscp";


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
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/


int main( int argc, const char** argv )
{
    fh_cp_tty obj = new FerrisCopy_TTY();

    const char* DstNameCSTR              = 0;
    unsigned long ShowMeter              = 0;
    unsigned long UseSyncDelayer         = 0;
    
    struct poptOption optionsTable[] =
        {
            { "show-progress-meter", 0, POPT_ARG_NONE, &ShowMeter, 0,
              "Show a one line progress meter", "" },

            { "use-sync-delayer", 0, POPT_ARG_NONE, &UseSyncDelayer, 0,
              "try to delay final writing until end of execution", "" },
            
            { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
              "Specify destination explicity, all remaining URLs are assumed to be source files",
              "DIR" },

            FERRIS_COPY_OPTIONS( obj )
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ... dst");

    /* Now do options processing */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    obj->getPoptCollector()->ArgProcessingDone( optCon );

    if (argc < 3)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }

    try
    {
        typedef stringlist_t srcs_t;
        srcs_t srcs;

        srcs = expandShellGlobs( srcs, optCon );
        
        /*
         * Setup the destination from either explicit command line or
         * last arg
         */
        string DstName;
        if( DstNameCSTR )
        {
            obj->setDstIsDirectory( true );
            DstName = DstNameCSTR;
        }
        else
        {
            DstName = srcs.back();
            srcs.pop_back();
        }

        /* TTY version */
        obj->setShowMeter( ShowMeter );

        /* Common version */
//        cerr << "dst:" << DstName << endl;
        obj->setDstURL( DstName );

        /************************************************************/
        /************************************************************/
        /************************************************************/


        typedef Loki::SmartPtr< SyncDelayer, 
            Loki::RefLinked, 
            Loki::DisallowConversion, 
            FerrisLoki::FerrisExSmartPointerChecker, 
            Loki::DefaultSPStorage >  fh_SyncDelayer;
        
        fh_SyncDelayer syncd = 0;
        if( UseSyncDelayer )
        {
            syncd = new SyncDelayer();
        }
        
        for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
        {
            string SrcName = *iter;
//            cerr << "src:" << SrcName << endl;

            obj->setSrcURL( SrcName );
            obj->copy();
        }
    }
    catch( NoSuchContextClass& e )
    {
        cerr << "Invalid context class given e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }
    


    
    poptFreeContext(optCon);
    return 0;
}
