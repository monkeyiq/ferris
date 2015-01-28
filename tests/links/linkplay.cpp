/******************************************************************************
*******************************************************************************
*******************************************************************************

    test reading directories which are VirtualSoftLinks

    Copyright (C) 2002 Ben Martin

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

    $Id: linkplay.cpp,v 1.2 2008/04/27 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "linktest";

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
    const char*   inputfilename_CSTR  = 0;
    unsigned int  Y                   = 0;
    
    struct poptOption optionsTable[] =
        {
//             { "explicit-size", 'n', POPT_ARG_INT, &Y, 0,
//               "size of input context", "" },
            
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
            string     srcURL = tmpCSTR;
            fh_context src    = Resolve( srcURL );

            if( src->begin() != src->end() )
            {
                cerr << "src is:" << toVoid( GetImpl( src ))
                     << " src.url:" << src->getURL()
                     << endl;
                Context::iterator citer = src->begin();

                cerr << "citer is:" << toVoid( GetImpl( *citer ))
                     << " citer.url:" << (*citer)->getURL()
                     << endl;

                fh_context p = citer->getParent();
                cerr << "p is:" << toVoid( GetImpl( p ))
                     << " p.url:" << p->getURL()
                     << endl;

                cerr << " p.begin == citer:" << (p->begin() == citer) << endl;
                cerr << " distance( p.begin, citer ):"
                     << distance( p->begin(), citer ) << endl;


                cerr << " src.begin == citer:" << (src->begin() == citer) << endl;
                cerr << " distance( src.begin, citer ):"
                     << distance( src->begin(), citer ) << endl;
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
    
    
