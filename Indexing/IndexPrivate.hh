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

    $Id: IndexPrivate.hh,v 1.6 2010/09/24 21:31:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_PRIVATE_IMPL_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTIDX_PRIVATE_IMPL_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <FerrisLoki/loki/Singleton.h>
#include <FerrisLoki/loki/Factory.h>
#include <FerrisLoki/loki/Functor.h>

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

#include <mg/stem.h>
#include <mg/bitio_m.h>
#include <mg/bitio_m_mem.h>
#include <mg/bitio_mem.h>
#include <mg/bitio_gen.h>

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif

#include <string>
#include <numeric>
#include <algorithm>

#include <Ferris/FullTextIndexer.hh>

using namespace std;

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

namespace Ferris
{
    namespace FullTextIndex 
    {

        template < class Base,class Sub >
        struct MakeObject
        {
            static Base* Create()
                { return new Sub(); }
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        stringlist_t& getLexiconClassNames();
        bool appendToLexiconClassNames( const std::string& s );
        typedef Loki::SingletonHolder<
            Loki::Factory< Lexicon, string >, Loki::CreateUsingNew, Loki::NoDestroy >
        LexiconFactory;
        stringlist_t& getLexiconAliasNames();
        bool appendToLexiconAliasNames( const std::string& s );


        
        stringlist_t& getReverseLexiconClassNames();
        bool appendToReverseLexiconClassNames( const std::string& s );
        typedef Loki::SingletonHolder<
            Loki::Factory< ReverseLexicon, string >, Loki::CreateUsingNew, Loki::NoDestroy >
        ReverseLexiconFactory;
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        struct FERRISEXP_DLLLOCAL CommonCodingPolicy
        {
            int N;
            int p;
            int blk;
            unsigned long m_bitsProcessed;
            mem_bitio_state m_state;

            inline CommonCodingPolicy( int N, int p )
                :
                N( N ),
                p( p ),
                blk( 0 ),
                m_bitsProcessed( 0 )
                {
                }
        };


        struct FERRISEXP_DLLLOCAL GolombPolicy
            :
            public CommonCodingPolicy
        {
            inline GolombPolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p )
                {
                    blk = BIO_Bblock_Init ( N,  p );
                    LG_IDX_D << "Golomb() N:" << N << " p:" << p << " gives b:" << blk << endl;
                }
    
            inline void encode( const int v )
                {
                    ENCODE_CONTINUE ( m_state )
                        BBLOCK_ENCODE_L( v, blk, m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        LG_IDX_D << "Golomb.encode() v:" << v << " m_bitsProcessed:" << m_bitsProcessed << endl;
                        }

            inline int decode()
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        BBLOCK_DECODE_L (v, blk, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )

//                int v = BIO_Mem_Bblock_Decode( blk, &m_state, &m_bitsProcessed );
                        return v;
                }
        };


        struct FERRISEXP_DLLLOCAL BinaryPolicy
            :
            public CommonCodingPolicy
        {
            inline BinaryPolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p )
                {
                }
    
            inline void encode( const int v )
                {
                    ENCODE_CONTINUE ( m_state )
                        blk = BIO_Binary_Length( v, 8  );
                    BINARY_ENCODE_L( v, blk,  m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        }

            inline void encode( const int v, const int lo, const int hi )
                {
                    ENCODE_CONTINUE ( m_state )
                        BINARY_ENCODE_L( v + 1 - lo , hi + 1 - lo,  m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        }
    
            inline int decode()
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        BINARY_DECODE_L (v, blk, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )
                        return v;
                }

            inline int decode( const int lo, const int hi )
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        BINARY_DECODE_L ( v, hi + 1 - lo, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )
                        return v - 1 + lo;
                }
    
        };


        struct FERRISEXP_DLLLOCAL EliasPolicy
            :
            public CommonCodingPolicy
        {
            double s;

            inline EliasPolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p ),
                s(2)
                {
                    blk = 10;
                }
    
            inline void encode( const int v )
                {
                    ENCODE_CONTINUE ( m_state )
                        ELIAS_ENCODE_L(  v, blk, s, m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        }

            inline int decode()
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        ELIAS_DECODE_L ( v, blk, s, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )
                        return v;
                }
        };


        /**
         * This is the Y code from page 118 of MG. ie. The one that outputs
         * the unary then binary code.
         */
        struct FERRISEXP_DLLLOCAL GammaPolicy
            :
            public CommonCodingPolicy
        {
            inline GammaPolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p )
                {
                }

            inline void encode( const int v )
                {
                    ENCODE_CONTINUE ( m_state )
                        GAMMA_ENCODE_L( v, m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        }

            inline int decode()
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        GAMMA_DECODE_L( v, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )
                        return v;
                }
        };


        /**
         * This is the little bomb code from page 119 of MG. ie. The one that outputs
         * the gamma code then binary code.
         */
        struct FERRISEXP_DLLLOCAL DeltaPolicy
            :
            public CommonCodingPolicy
        {
            inline DeltaPolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p )
                {
                }

            inline void encode( const int v )
                {
                    ENCODE_CONTINUE ( m_state )
                        DELTA_ENCODE_L( v, m_bitsProcessed );
                    ENCODE_PAUSE ( m_state )
                        }

            inline int decode()
                {
                    register unsigned long v;
                    DECODE_CONTINUE ( m_state )
                        DELTA_DECODE_L( v, m_bitsProcessed );
                    DECODE_PAUSE ( m_state )
                        return v;
                }
        };

        /**
         * Dummy policy. interpolitve coding is done with a different API because
         * it traverses its data in a binary search pattern and doesn't use dgaps
         */
        struct FERRISEXP_DLLLOCAL InterpolativePolicy
            :
            public CommonCodingPolicy
        {
            inline InterpolativePolicy( int N = 0 , int p = 0 )
                :
                CommonCodingPolicy( N, p )
                {
                }
    
            inline void encode( const int v )
                {}

            inline int decode()
                { return 0; }
        };


        template < class CoderPolicy >
        class BitCoder
            :
            public CoderPolicy
        {
            bool m_ended;
            bool m_encoding;

            enum {
                bufsz = 65000 
            };
            unsigned char buf[ 65001 ];

            /**
             * Called by begin() / end() to flush any temp data into the buffer 'buf'
             */
            inline void endOperation()
                {
                    if( m_ended )
                        return;
                    
                    m_ended = true;

                    if( m_encoding )
                        BIO_Mem_Encode_Done( &this->m_state );
                }
            
        public:

            inline BitCoder( int N = 100, int p = 10 )
                :
                CoderPolicy( N, p )
                {
                }

            /**
             * flush buffers and setup for a new encode run
             */
            inline void resetForEncode()
                {
                    m_ended    = false;
                    m_encoding = true;
                    
                    this->m_bitsProcessed = 0;
                    bzero( buf, bufsz );
                    BIO_Mem_Encode_Start ( buf, bufsz,
                                           &this->m_state );
                }

            /**
             * Setup buffers with data from begin to begin+count
             */
            template <class Iter>
            void resetForDecode( Iter begin, int count )
                {
                    m_ended = false;
                    this->m_bitsProcessed = 0;
                    bzero( buf, bufsz );
                    BIO_Mem_Decode_Start ( buf, bufsz,
                                           &this->m_state );

                    int i=0;
                    for( Iter iter = begin; count--; ++iter, ++i )
                    {
//                        cerr << "resetForDecode i:" << i << " data:" << (int)*iter << endl;
                        buf[i] = *iter;
                    }
                }

            /**
             * number of bits encoded/decoded so far
             */
            inline int getBitsProcessed()
                {
                    return this->m_bitsProcessed;
                }

            /**
             * number of bytes needed to store the number of bits encoded/decoded so far.
             * ie. roof of bits/8
             */
            inline int getBytesProcessed()
                {
                    return (int)ceil(((double)getBitsProcessed())/8);
                }
    

            typedef unsigned char* iterator;

            inline iterator begin()
                {
                    endOperation();
                    return &buf[ 0 ];
                }

            inline iterator end()
                {
                    endOperation();
                    return &buf[ getBytesProcessed() ];
                }
        };


        /**
         * Decode data using the coding policy Coder from begin to begin+iterDistance
         * decoding should stop after SymbolCount symbols have been recovered and output
         * is to be placed in 'out' which is assumed to posses a vector/list push_back() method.
         * N and p are used to initialize the codec.
         *
         * @return The number of bytes processed from begin.
         */
        template < class Coder, class Iter, class OutClass >
        int decode( Iter begin, int iterDistance, int SymbolCount, OutClass& out, int N = 100, int p = 10 )
        {
            Coder c( N, p );
            c.resetForDecode( begin, iterDistance );
    
            while( SymbolCount-- )
            {
                int v = c.decode();
//                cerr << "decode()ed v:" << v << " SymbolCount:" << SymbolCount << endl;
                out.push_back( v );
            }

//            cerr << "decode() bits:" << c.getBitsProcessed() << endl;
            return c.getBytesProcessed();
        }

    template < class STREAM, class N >
    int writenum( STREAM& ss, N& v )
    {
        ss.write( (char*)&v, sizeof(N) );
        return( sizeof(N) );
    }
        
        /**
         * encode data from begin to end non inclusive of end. Place the coded
         * payload into the stream outss starting at the current put position.
         * N and p are used to init the codec.
         *
         * @return the number of bytes written to outss
         */
        template < class Coder, class Iter, class OutStream >
        int encode( Iter begin, Iter end, OutStream outss, int N = 100, int p = 10 )
        {
            Coder c( N, p );
            c.resetForEncode();
    
            int UncodedCount = 0;
    
            for( Iter iter = begin; iter != end; ++iter )
            {
//                cerr << "encode() n:" << *iter << " bits:" << c.getBitsProcessed() << endl;
                c.encode( *iter );
                ++UncodedCount;
            }

            // plain copy() fails to load again
            //  copy( c.begin(), c.end(), out );
            // have to manually copy it for some reason.
            for( unsigned char* ci = c.begin(); ci != c.end(); ++ci )
            {
                unsigned char v = *ci;
                writenum( outss, v );
            }

//             cerr << "encode() bits:" << c.getBitsProcessed()
//                  << " distance:" << distance( c.begin(), c.end() )
//                  << endl;

            return c.getBytesProcessed();
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        /**
         * See pp 127 of managing gigabytes
         */
        template < class Coder, class Iter >
        void encode_interpolative_recurse( Coder& c, Iter begin, Iter end, int f, int lo, int hi )
        {
            if( f==0 ) return;
            if( f==1 )
            {
//        cerr << "binary_code( f==1 ) value:" << *begin << " low:" << lo << " high:" << hi << endl;
                c.encode( *begin, lo, hi );
                return;
            }

            int h         = f / 2;
            Iter midway   = begin;
            advance( midway, h );
            int m         = *midway;
            int f1        = h;
            int f2        = f - h - 1;
            Iter L1_begin = begin;
            Iter L1_end   = midway-1;
            Iter L2_begin = midway+1;
            Iter L2_end   = end;

//     cerr << "encode_interpolative() f:" << f
//          << " lo:" << lo << " hi:" << hi
//          << " m:" << m << " f:" << f << " h:" << h
//          << " f1:" << f1 << " f2:" << f2
//          << endl;

//     cerr << "binary_code( B ) value:" << m << " low:" << lo + f1 << " high:" << hi - f2 << endl;
            c.encode( m, lo + f1, hi - f2 );
            encode_interpolative_recurse( c, L1_begin, L1_end, f1, lo,  m-1 );
            encode_interpolative_recurse( c, L2_begin, L2_end, f2, m+1, hi );
        }

/**
 *
 * @param lowmark is a out of band number below any in begin to end
 * @param himark  is a out of band number above any in begin to end
 *
 * @return The number of bytes written to outss
 */
        template < class Iter, class OutClass >
        int encode_interpolative( Iter begin, Iter end, int lowmark, int himark, OutClass& outss )
        {
            int BitsProcessed = 0;
            int UncodedCount  = distance( begin, end );
    

            int lo=lowmark;
            int hi=himark;

//     cerr << "encode() for interpolative mode." << endl;
//     copy( begin, end, ostream_iterator<int>( cerr, " " ));
//     cerr << endl;
    

            typedef BitCoder< BinaryPolicy > Coder;
            Coder c( 100, 10 );
            c.resetForEncode();
//     cerr << "About to encode_interpolative. distance:" << distance( begin, end )
//          << " count:" << UncodedCount
//          << " lo:" << lo
//          << " hi:" << hi
//          << endl;
    
            encode_interpolative_recurse( c, begin, end, UncodedCount, lo, hi );
            int bw = c.getBytesProcessed();
//     cerr << "encode() bytes:" << bw << endl;
//     cerr << "encode() bits :" << c.getBitsProcessed() << endl;

            for( unsigned char* ci = c.begin(); ci != c.end(); ++ci )
            {
                unsigned char v = *ci;
//         cerr << "encode() write num:" << (int)v << endl;
                writenum( outss, v );
            }
            outss << flush;
            return c.getBytesProcessed();
        }


        template < class Coder, class OutClass >
        void decode_interpolative_recurse( Coder& c, int f, int lo, int hi, OutClass& out, int arrayIndex )
        {
            if( f==0 ) return;
            if( f==1 )
            {
                int m = 0;
                m = c.decode( lo, hi );
//        out.push_back( m );
                out[ arrayIndex ] = m;
//        cerr << "binary_code( f==1 ) value:" << m << " low:" << lo << " high:" << hi
//             << " arrayIndex:" << arrayIndex << endl;
                return;
            }

            int h   = f / 2;
            int f1  = h;
            int f2  = f - h - 1;

            int m = c.decode( lo + f1, hi - f2 );
//    out.push_back( m );
            out[ arrayIndex ] = m;
//     cerr << "decode()ed m:" << m << " lo:" << lo+f1 << " hi:" << hi - f2 << endl;
//     cerr << "decode_interpolative() f:" << f << " h:" << h << " f1:" << f1 << " f2:" << f2 << " lo:" << lo
//          << " m:" << m << " hi:" << hi << " arrayIndex:" << arrayIndex << endl;
    
            decode_interpolative_recurse( c, f1, lo,  m-1, out, arrayIndex - (int)(ceil(1.0 * (f-1)/4)) );
            decode_interpolative_recurse( c, f2, m+1, hi , out, arrayIndex + (int)(ceil(1.0 * (f)/4)) );
        }


        /**
         *
         * @param lowmark is a out of band number below any in begin to end
         * @param himark  is a out of band number above any in begin to end
         *
         * @return The number of bytes actually consumed from begin
         */
        template < class Iter, class OutClass >
        int decode_interpolative( Iter begin, int iterDistance, int SymbolCount, int lomark, int himark, OutClass& out )
        {
            typedef BitCoder< BinaryPolicy > Coder;
            Coder c( 100, 10 );
            c.resetForDecode( begin, iterDistance );

            out.resize( SymbolCount );
            decode_interpolative_recurse( c, SymbolCount, lomark, himark, out, (int)(ceil(1.0 * (SymbolCount-1)/2)) );
    
//    cerr << "decode() out.size:" << out.size() << endl;
        
//     int lo = lomark;
//     int hi = himark;
//     int f  = SymbolCount;

//     int h         = f / 2;
//     int f1        = h;
//     int f2        = f - h - 1;
        
//     while( SymbolCount-- )
//     {
//         int v = c.decode( f1 - lo, hi - f2 );
//         cerr << "decode()ed v:" << v << " lo:" << f1 - lo << " hi:" << hi - f2 << endl;
//     }
//    cerr << "decode() bits:" << c.getBitsProcessed() << " by:" << c.getBytesProcessed() << endl;
            return c.getBytesProcessed();
        }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * read a fixed length string from a stream
     */
    template < class STREAM >
    std::string readstring( STREAM& ss, int len )
    {
        // FIXME
        const int bufsz = 64*1024;
        char buf[ bufsz+1 ];
        ss.read( buf, len );
        buf[ len ] = '\0';
        std::string s = buf;
        return s;
    }

    template < class STREAM >
    STREAM& writestring( STREAM& ss, const std::string& s )
    {
        ss << s;
        return ss;
    }
        

    template < class STREAM, class N >
    int readnum( STREAM& ss, N& v )
    {
        N tmp;
        ss.read( (char*)&tmp, sizeof(N) );
        if( ss.good() )
        {
            v = tmp;
            return sizeof(N);
        }
        else
        {
            LG_FTXLEXI_W << "readnum failed" << endl;
        }
        return 0;
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

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
                               int N, int p );
    
    /**
     * Use the coding method identified by algo to encode begin to end
     * and put the coded version into oss.
     *
     * @param oss Stream to put bit coded data into
     * @param algo Algo to use during encode
     * @param begin First dgap to encode
     * @param end   Last + 1 dgap
     * @param N     The highest document number that we are encoding (max item in chunk)
     * @param p     The number of terms we are encoding              (# items in chunk)
     *
     * @return The number of bytes written.
     */
    template < class Iter >
    int encodeDocumentNumbers( fh_ostream oss, string algo, Iter begin, Iter end, int N, int p )
    {
        int ret = 0;

        if     ( algo == "Golomb" ) ret = encode< BitCoder< GolombPolicy > >( begin, end, oss, N, p );
        else if( algo == "Elias" )  ret = encode< BitCoder< EliasPolicy > > ( begin, end, oss, N, p );
        else if( algo == "Delta" )  ret = encode< BitCoder< DeltaPolicy > > ( begin, end, oss, N, p );
        else                        ret = encode< BitCoder< GammaPolicy > > ( begin, end, oss, N, p );

        return ret;
    }

    /**
     * converts from a sequence of dgaps to the document number collection
     */
    template < class Iter, class Coll >
    void convertFromDGaps( Iter begin, Iter end, Coll& out )
    {
        typedef list< docid_t > tmp_t;
        tmp_t tmp;
        std::partial_sum( begin, end, back_inserter(tmp) );

        for( tmp_t::iterator ti = tmp.begin(); ti != tmp.end(); ++ti )
        {
            out[ *ti ] = 0;
        }
    }
        

    /**
     * converts a range into another range which is the gaps between each element in
     * the original. ie. makes a document gap range.
     * n  n+1     n+2
     * n (n+1)-n (n+2)-(n+1)
     */
    template < class Iter >
    void convertToDGaps( Iter begin, Iter end, list< docid_t >& out )
    {
        typedef list< docid_t > out_t;

        if( begin == end )
            return;
        if( (begin+1) == end )
        {
            out.push_back( *begin );
            return;
        }

        //
        // first number is pushed out whole
        //
        out.push_back( *begin );
            
        //
        // start working from the second number onward
        //
        Iter prev = begin;
        Iter iter = begin;
        for( ++iter; iter != end;  )
        {
            docid_t v = *iter - *prev;
            out.push_back( v );
                
            prev = iter;
            ++iter;
        }
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    };
};
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
