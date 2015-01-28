/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris istream forward sweep test
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

    $Id: contextsweep.cpp,v 1.2 2008/04/27 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <FerrisOpenSSL.hh>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "forwardsweep";

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
    const char*   DstNameCSTR            = 0;
    unsigned int  makeMD5                = 0;
    unsigned int  backwardSweep          = 0;
    unsigned int  randomSweep            = 0;
    unsigned int  explicitSize           = 0;
    
    struct poptOption optionsTable[] =
        {
//             { "target-directory", 0, POPT_ARG_STRING, &DstNameCSTR, 0,
//               "Specify destination explicity, all remaining URLs are assumed to be source files",
//               "DIR" },

            { "md5", '5', POPT_ARG_NONE, &makeMD5, 0,
              "Generate the MD5 for each context", "" },

            { "backward-sweep", 'b', POPT_ARG_NONE, &backwardSweep, 0,
              "read bytes from eof to start of file", "" },

            { "random-sweep", 'r', POPT_ARG_NONE, &randomSweep, 0,
              "read bytes in random blocks. Input must be even multiple of 32Kbytes", "" },

            { "explicit-size", 'n', POPT_ARG_INT, &explicitSize, 0,
              "size of input context", "" },
            
            FERRIS_POPT_OPTIONS
            POPT_AUTOHELP
            POPT_TABLEEND
        };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ... dst");

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

    try
    {
        typedef vector<string> srcs_t;
        srcs_t srcs;
        
        while( const char* RootNameCSTR = poptGetArg(optCon) )
        {
            string RootName = RootNameCSTR;
            srcs.push_back( RootName );
        }

        if( srcs.empty() )
        {
            cerr << "No objects to test\n" << endl;
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        for( srcs_t::iterator iter = srcs.begin(); iter != srcs.end(); ++iter )
        {
            string SrcName = *iter;
            fh_context c = Resolve( SrcName );

            fh_istream iss = c->getIStream();
            string wholefile;

            if( backwardSweep )
            {
                fh_stringstream ss;
                iss.seekg( -1, ios::end );

                while( iss )
                {
                    char c=0;
                    if( iss >> noskipws >> c )
                    {
                        ss  << c;
                        iss.seekg( -2, ios::cur );
                    }
                    else
                        break;
                }
                wholefile = tostr(ss);
            }
            else if( randomSweep )
            {
                fh_stringstream ss;
                const int block_size=8*1024;
                int size = explicitSize
                    ? explicitSize
                    : toint( getStrAttr( c, "size", "0" ));
                if( !size )
                {
                    cerr << "Cant do random seeking test on context without size EA" << endl;
                    continue;
                }
                int nblocks = size / block_size;
//                 for( int i=0; i<size; ++i )
//                     ss << ' ';
//                 ss << flush;

                // cleanup the last block first
//                 iss.seekg( ios::beg, nblocks * block_size );
//                 ss.seekp(  ios::beg, nblocks * block_size );
//                 copy( istreambuf_iterator<char>(iss),
//                       istreambuf_iterator<char>(),
//                       ostreambuf_iterator<char>(ss));

                // copy other blocks in multiple passes
                int n_passes = 4;
                cerr << "end block starts at:" << (nblocks * block_size) << endl;
                cerr << " nblocks:" << nblocks << endl;
                cerr << " block_size:" << block_size << endl;
                cerr << " nblocks/n_passes:" << (nblocks/n_passes) << endl;
                
                for( int pass=0; pass < n_passes; ++pass )
                {
                    for( int cb=0; cb < (nblocks/n_passes); ++cb )
                    {
                        char buf[ block_size + 1 ];
                        iss.seekg( (cb+pass)*block_size, ios::beg );
                        ss.seekp(  (cb+pass)*block_size, ios::beg );
                        iss.read( buf, block_size );
                        ss.write( buf, block_size );
                    }
                }

                wholefile = tostr(ss);
            }
            else
            {
                wholefile = StreamToString( iss );
            }
            
            cerr << "Read in " << wholefile.length() << " bytes" << endl;
            if( makeMD5 )
            {
                string md5 = digest( wholefile, "md5" );
                cerr << md5 << " <= MD5 " << endl;
            }
        }
    }
    catch( exception& e )
    {
        cerr << "Error:" << e.what() << endl;
    }
    
    poptFreeContext(optCon);
    return 0;
}
    
    
