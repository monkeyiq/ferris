/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
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

    $Id: IndexPrivate.cpp,v 1.2 2010/09/24 21:31:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>

#include <IndexPrivate.hh>

namespace Ferris
{
    namespace FullTextIndex 
    {

        /**
         * Decode a run of document numbers from 'iss'.
         *
         * This mainly works on decoding the 'dgaps' and from there one must
         * use convertFromDGaps() to get the IDs themself.
         *
         * @param iss  Stream to get encoded document numbers from
         * @param algo Algo that was used to encode the numbers
         * @param countOfNumbers How many symbols were encoded
         * @param entirePossibleByteAreaSize How far from where iss is now can
         *                                   possibly contain relevant data
         *                                   (iss+entirePossibleByteAreaSize == end)
         * @param out Where to put decoded docids
         * @param N     The highest document number that we are encoding (max item in chunk)
         * @param p     The number of terms we are encoding              (# items in chunk)
         *
         * @return The number of bytes consumed from iss
         */
        int decodeDocumentNumbers( fh_istream iss,
                                   string algo,
                                   int countOfNumbers,
                                   int entirePossibleByteAreaSize,
                                   std::vector< docid_t >& out,
                                   int N, int p )
        {
            int bread = 0;
            
            if( algo == "Golomb" )
                bread = decode< BitCoder< GolombPolicy > >( istreambuf_iterator<char>(iss),
                                                            entirePossibleByteAreaSize,
                                                            countOfNumbers,
                                                            out, N, p );
            else if( algo == "Elias" )
                bread = decode< BitCoder< EliasPolicy > >( istreambuf_iterator<char>(iss),
                                                           entirePossibleByteAreaSize,
                                                           countOfNumbers,
                                                           out, N, p );
            else if( algo == "Delta" )
                bread = decode< BitCoder< DeltaPolicy > >( istreambuf_iterator<char>(iss),
                                                           entirePossibleByteAreaSize,
                                                           countOfNumbers,
                                                           out, N, p );
            else 
                bread = decode< BitCoder< GammaPolicy > >( istreambuf_iterator<char>(iss),
                                                           entirePossibleByteAreaSize,
                                                           countOfNumbers,
                                                           out, N, p );

            //
            // we have read more bytes than we actually processed, move to the end of the
            // used byte section.
            //
            iss.seekg( (-1 * entirePossibleByteAreaSize) + bread, ios::cur );
            
            return bread;
        }
    };
};
