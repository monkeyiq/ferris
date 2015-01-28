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

    $Id: ferris-create-fca-scale-splice-in-ffitler.cpp,v 1.3 2010/09/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/


#include "libferrisfcascaling.hh"

const string PROGRAM_NAME = "ferris-create-fca-scale-splice-in-ffitler";

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
        unsigned long Verbose              = 0;
        unsigned long Clone                = 0;
        const char*   splice_opcode_CSTR   = "&";
        const char*   splice_ffilter_CSTR  = 0;
        const char*   splice_attrname_postfix_CSTR  = 0;
        const char*   findexPath_CSTR      = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "clone", 'a', POPT_ARG_NONE, &Clone, 0,
                  "output the input data aswell as the new attributes", "" },
                
                { "opcode", 'e', POPT_ARG_STRING, &splice_opcode_CSTR, 0,
                  "opcode to use to join new ffitler with existing ffilters", "&" },

                { "ffilter", 'f', POPT_ARG_STRING, &splice_ffilter_CSTR, 0,
                  "new ffitler to splice in", "" },

                { "attr-postfix", 'o', POPT_ARG_STRING, &splice_attrname_postfix_CSTR, 0,
                  "postfix to add to the new attribute name", "" },
                
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

        if( Clone && !splice_attrname_postfix_CSTR )
        {
            splice_attrname_postfix_CSTR = "_new";
        }
        if( !splice_attrname_postfix_CSTR )
            splice_attrname_postfix_CSTR = "";
        
        string splice_opcode = splice_opcode_CSTR;
        if( !splice_ffilter_CSTR )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        string splice_ffilter = splice_ffilter_CSTR;
        string splice_attrname_postfix = splice_attrname_postfix_CSTR;
        
//         EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
//         string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
//         string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );

        stringmap_t ffilterStrings;
        readAttributes( ffilterStrings, optCon );

        
        int i = 0;
        for( stringmap_t::const_iterator ci = ffilterStrings.begin();
             ci != ffilterStrings.end(); ++ci, ++i )
        {
            if( Clone )
                cout << ci->first << " '(" << ci->second << ")'" << endl;
                
            cout << ci->first << splice_attrname_postfix << " '(" << splice_opcode << splice_ffilter
                 << ci->second << ")'" << endl;
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


        
