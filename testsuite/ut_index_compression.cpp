/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code for index compression algos
    Copyright (C) 2003 Ben Martin

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

    $Id: ut_index_compression.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris.hh>
#include <FullTextIndexer.hh>
#include <FullTextIndexer_private.hh>
#include <Ferris/Iterator.hh>
#include <Indexing/IndexPrivate.hh>
#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;
using namespace FullTextIndex;

const string PROGRAM_NAME = "ut_index_compression";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

#ifdef GCC_HASCLASSVISIBILITY
int main( int argc, char** argv )
{
    int exit_status = 0;
    cerr << "ut compression not available on hidden symbol libferris builds!" << endl;
    return 1;
}
#else

template < class Coder, class Iter >
int runtest( Iter begin, Iter end, int N, int p )
{
    fh_stringstream codedstream;
    int SymbolCount = distance( begin, end );
    typedef vector< int > roundtrip_t;
    roundtrip_t roundtrip;
    
    int bwrite = encode< Coder >( begin, end, codedstream, N, p );

    int bread = decode< Coder >( istreambuf_iterator<char>(codedstream),
                                 bwrite, SymbolCount,
                                 roundtrip,
                                 N, p );

    if( bwrite != bread )
    {
        cerr << "Bytes decoded are not the same as bytes encoded!"
             << " bwrite:" << bwrite
             << " bread:" << bread
             << endl;
        exit(1);
    }

    if( roundtrip.size() != SymbolCount )
    {
        cerr << "Symbols decoded are not the same as encoded!"
             << " encoded:" << SymbolCount
             << " decoded:" << roundtrip.size()
             << endl;
        exit(1);
    }


    roundtrip_t::iterator rt = roundtrip.begin();
    for( Iter in = begin; in != end && rt != roundtrip.end(); ++in, ++rt )
    {
        if( *in != *rt )
        {
            cerr << "decode(encode(sym)) != sym"
                 << " input:" << *in
                 << " was decoded as:" << *rt 
                 << endl;
            exit(1);
        }
    }

    cout << "Roundtrip was success. symbols:" << SymbolCount << " bytes:" << bwrite << endl;
    
    return 0;
}


template < class Iter >
int runtest_interpolative( Iter begin, Iter end, int N, int p )
{
    fh_stringstream codedstream;
    int SymbolCount = distance( begin, end );
    typedef vector< int > roundtrip_t;
    roundtrip_t roundtrip;
    int lomark = 0;
    int himark = 4096;
    
    int bwrite = encode_interpolative( begin, end, lomark, himark, codedstream );
    int bread  = decode_interpolative( istreambuf_iterator<char>(codedstream),
                                       bwrite,
                                       SymbolCount,
                                       lomark, himark,
                                       roundtrip );

    if( bwrite != bread )
    {
        cerr << "ERROR, Bytes decoded are not the same as bytes encoded!"
             << " bwrite:" << bwrite
             << " bread:" << bread
             << endl;
        exit(1);
    }

    if( roundtrip.size() != SymbolCount )
    {
        cerr << "ERROR, Symbols decoded are not the same as encoded!"
             << " encoded:" << SymbolCount
             << " decoded:" << roundtrip.size()
             << endl;
        exit(1);
    }


    roundtrip_t::iterator rt = roundtrip.begin();
    for( Iter in = begin; in != end && rt != roundtrip.end(); ++in, ++rt )
    {
        if( *in != *rt )
        {
            cerr << "ERROR, decode(encode(sym)) != sym"
                 << " input:" << *in
                 << " was decoded as:" << *rt 
                 << endl;
            exit(1);
        }
    }

    cout << "Roundtrip was success. symbols:" << SymbolCount << " bytes:" << bwrite << endl;
    
    return 0;
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long TestGamma            = 0;
        unsigned long TestDelta            = 0;
        unsigned long TestGolomb           = 0;
        unsigned long TestInterpolative    = 0;
        unsigned long Verbose              = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "gamma", 0, POPT_ARG_NONE, &TestGamma, 0,
                  "test the gamma code", "" },

                { "delta", 0, POPT_ARG_NONE, &TestDelta, 0,
                  "test the delta code", "" },

                { "golomb", 0, POPT_ARG_NONE, &TestGolomb, 0,
                  "test the golomb code", "" },

                { "interpolative", 0, POPT_ARG_NONE, &TestInterpolative, 0,
                  "test the interpolative code", "" },

                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* src1 src2 ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        if (argc < 2 )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }


        const int outsz = 4096;
        unsigned char out[ outsz + 1 ];
        typedef vector< int > ivt;
        unsigned long bits_out = 0, bits_so_far = 0;
        ivt iv;
        int max = 51;

        bzero( out, outsz );

        for( int i=1; i < max; i+=3 )
            iv.push_back( i );

        iv.push_back( max+2 );
        iv.push_back( max+5 );
        iv.push_back( max+10 );

        std::map< int, int > mapdata;
        for( int i=1; i < max; ++i )
            mapdata[ i ] = i+100;

        int N = max;
        int p = 3 + max / 3;

        if( TestGamma )
            return runtest< BitCoder< GammaPolicy > >( map_domain_iterator(mapdata.begin()),
                                                       map_domain_iterator(mapdata.end()),
                                                       N, p );

        if( TestDelta )
            return runtest< BitCoder< DeltaPolicy > >( map_domain_iterator(mapdata.begin()),
                                                       map_domain_iterator(mapdata.end()),
                                                       N, p );

        if( TestGolomb )
            return runtest< BitCoder< GolombPolicy > >( map_domain_iterator(mapdata.begin()),
                                                        map_domain_iterator(mapdata.end()),
                                                        N, p );

        if( TestInterpolative )
        {
            return runtest_interpolative( map_domain_iterator(mapdata.begin()),
                                          map_domain_iterator(mapdata.end()),
                                          N, p );
        }
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}
#endif

