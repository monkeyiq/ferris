/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris rm
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

    $Id: ferrisrm.cpp,v 1.4 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisRemove_private.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ferrisrm";

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
    fh_rm rmobj = FerrisRm::CreateObject();

    struct poptOption optionsTable[] =
        {
            FERRIS_REMOVE_OPTIONS( rmobj )
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* url1 url2 ...");

    /* Now do options processing */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
    }

    rmobj->getPoptCollector()->ArgProcessingDone( optCon );

    if (argc < 2)
    {
        poptPrintHelp(optCon, stderr, 0);
        exit(1);
    }

    try
    {
        stringlist_t srcs;
        srcs = expandShellGlobs( srcs, optCon );
        stringlist_t::iterator srcsiter = srcs.begin();
        stringlist_t::iterator srcsend  = srcs.end();
        for( ; srcsiter != srcsend ; ++srcsiter )
        {
            string RootName = *srcsiter;
//            cerr << "rootname:" << RootName << endl;
            
            rmobj->setTarget( RootName );
            rmobj->remove();
        }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
