/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-create-fca-scale-numeric-ordinal.cpp,v 1.3 2010/09/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/


#include "libferrisfcascaling.hh"

const string PROGRAM_NAME = "ferris-create-fca-scale-numeric-ordinal";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose             = 0;
        unsigned long ReverseOrder = 0;
        unsigned long KeepBothEnds = 0;
        const char*   findexPath_CSTR    = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "reverse", 'r', POPT_ARG_NONE, &ReverseOrder, 0,
                  "reverse the sorting order", "" },
                
                { "keep-ends", 'K', POPT_ARG_NONE, &KeepBothEnds, 0,
                  "retain both ends of the scale", "" },
                
                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
        string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
        string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );

        stringmap_t ffilterStrings;
        readAttributes( ffilterStrings, optCon );

        string oldcomp = "<=";
        string newcomp = ">=";
        if( ReverseOrder )
        {
            swap( oldcomp, newcomp );
        }
        
        
        int i = 0;
        int sz = ffilterStrings.size();
        for( stringmap_t::const_iterator ci = ffilterStrings.begin();
             ci != ffilterStrings.end(); ++ci, ++i )
        {
            if( KeepBothEnds || i )
                cout << ci->first << " '" << ci->second << "'" << endl;

            if( KeepBothEnds || i < (sz-1) )
                cout << ci->first << "_rev "
                     << "'" << Util::replace_all( ci->second, oldcomp, newcomp ) << "'" << endl;
        }
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


        
