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

    $Id: SignalStreams.cpp,v 1.4 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>
#include <Ferris.hh> // only for loki
#include <SignalStreams.hh>
#include <Enamel.hh>
#include <algorithm>

#include <errno.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#ifdef FERRIS_HAVE_LIBZ
#include <zlib.h>
#endif

#ifdef FERRIS_HAVE_BZIP2
#include <bzlib.h>
#endif

using namespace std;
using namespace FerrisLoki;

namespace Ferris
{

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

static const char* COMPRESSION_SENTINAL = "is-ferris-compressed";
static const char* COMPRESSION_BITMAP   = "compression-bitmap";
static const char* COMPRESSION_ALGO     = "compression-algo";
static const char* COMPRESSION_BLOCKSZ  = "compression-block-size";
static const char* COMPRESSION_LEVEL    = "compression-level";

static void readBitmap(  fh_context c, intmap_t& b )
{
    fh_stringstream ss;
    ss << getStrAttr( c, COMPRESSION_BITMAP, "", true, true );

    while( ss->good() )
    {
        string k, v;
        getline( ss, k, '=' );
        getline( ss, v );
        b[ toint(k) ] = toint(v);
    }
}
static void writeBitmap( fh_context c, const intmap_t& b )
{
    fh_attribute a = c->acquireAttribute( COMPRESSION_BITMAP );
    fh_ostream  ss = a->getIOStream( ios::trunc | ios::out );
    
    for( intmap_t::const_iterator bi = b.begin(); bi!=b.end(); ++bi )
    {
        ss << bi->first << "=" << bi->second << endl;
    }

//     bool TESTING = 1;
//     if( TESTING )
//     {
//         intmap_t x;
//         readBitmap( c, x );
//         cerr << "Write test, should have saved " << endl;
//         for( intmap_t::iterator bi = b.begin(); bi != b.end(); ++bi )
//         {
//             cerr << "logical:" << bi->first << " phy:" << bi->second << endl;
//         }
        
//         cerr << "write test. READ BACK FOLLOWING dump_bitmap_to_cerr()" << endl;
//         for( intmap_t::iterator bi = x.begin(); bi != x.end(); ++bi )
//         {
//             cerr << "logical:" << bi->first << " phy:" << bi->second << endl;
//         }
        
//     }
}

static int compressed_buffer_size( std::streamsize src_sz )
{
    // bzip2 has the largest compressed buffer size requirements of no compression/gzip/bzip2
    // ftp://sources.redhat.com/pub/bzip2/docs/manual_3.html
    //
    // 1% larger than the uncompressed data, plus six hundred extra bytes.
    double sz = src_sz;
    sz *= 1.01;
    sz += 600;
    return (int)sz;
}

static char* compressed_buffer_alloc( std::streamsize src_sz )
{
    char* b = new char[ compressed_buffer_size( src_sz ) ];
    return b;
}

static void compressed_buffer_free( char* p )
{
    delete [] p;
}


/**
 * Use the given algo to compress src_sz bytes from src to dst.
 * level is an algo specific hint as the CPU/compression trade off
 *
 * return the size of data from dst that has been used for the compressed version
 */
static std::streamsize cf_compress( int algo,
                                    void* dst,       std::streamsize dst_sz,
                                    const void* src, std::streamsize src_sz,
                                    int level )
{
    std::streamsize ret = 0;
    
#ifdef FERRIS_HAVE_LIBZ
    if( algo == Factory::COMPRESS_GZIP )
    {
        // compress the buffer here
        static const char* myVersion = ZLIB_VERSION;

        if (zlibVersion()[0] != myVersion[0])
        {
            fh_stringstream ss;
            ss << "incompatible zlib version";
            Throw_CompressionAlgoNotFoundException( tostr(ss), 0 );
        }
        else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0)
        {
            cerr << "warning: different zlib version" << endl;
        }

        LG_IOSTREAM_D << "compressing src_sz:" << src_sz
                      << " dst_sz:" << dst_sz
                      << " dst:" << toVoid(dst)
                      << " src:" << toVoid(src)
                      << " level:" << level
                      << " algo:" << algo
                      << endl;
        
        uLongf gz_sz = dst_sz;
        int rc = compress2( (Bytef*)dst,        &gz_sz,
                            (const Bytef *)src, (uLong)src_sz,
                            level );
        ret = gz_sz;

        if( rc != Z_OK )
        {
            switch( rc )
            {
            case Z_MEM_ERROR:
            case Z_BUF_ERROR:
            default:
                fh_stringstream ss;
                ss << "strange problem with compress2() rc:" << rc;
                Throw_CompressionException( tostr(ss), 0 );
            }
        }
    }
#endif
#ifdef FERRIS_HAVE_BZIP2
    if( algo == Factory::COMPRESS_BZIP2 )
    {
        // compress the buffer here
        int blockSize100k = min( level, 9 );
        int workFactor = 0;

        unsigned int dsz = dst_sz;
        int rc = BZ2_bzBuffToBuffCompress( (char*)dst, &dsz,
                                           (char*)src, src_sz,
                                           blockSize100k, 0, workFactor );
        ret = dsz;

        if( rc != BZ_OK )
        {
            fh_stringstream ss;
            ss << "Problem with bzip2 compression rc:" << rc;
            Throw_CompressionException( tostr(ss), 0 );
        }
    }
#endif

    if( algo == Factory::COMPRESS_NONE )
    {
        memcpy( dst, src, src_sz );
        ret = src_sz;
        LG_IOSTREAM_D << "cf_compress() src_sz:" << src_sz << " ret:" << ret << endl;
    }
    return ret;
}

/**
 * opposive of cf_compress()
 *
 * Use the given algo to decompress src_sz bytes from src to dst.
 * level is an algo specific hint as the CPU/compression trade off
 *
 * return the size of data from dst that has been used for the compressed version
 */
static std::streamsize cf_decompress( int algo,
                                      gpointer dst, std::streamsize dst_sz,
                                      gpointer src, std::streamsize src_sz,
                                      int level )
{
    std::streamsize ret = 0;
    
#ifdef FERRIS_HAVE_LIBZ
    if( algo == Factory::COMPRESS_GZIP )
    {
        static const char* myVersion = ZLIB_VERSION;

        if (zlibVersion()[0] != myVersion[0])
        {
            fh_stringstream ss;
            ss << "incompatible zlib version";
            Throw_CompressionAlgoNotFoundException( tostr(ss), 0 );
        }
        else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0)
        {
            cerr << "warning: different zlib version" << endl;
        }

        LG_IOSTREAM_D << "cf_decompress() src_sz:" << src_sz
                      << " dst_sz:" << dst_sz
                      << endl;

        uLongf gz_sz = dst_sz;
        int rc = uncompress( (Bytef*)dst, &gz_sz, (const Bytef *)src, src_sz );
        ret = gz_sz;

        if( rc != Z_OK )
        {
            switch( rc )
            {
            case Z_MEM_ERROR:
            case Z_BUF_ERROR:
            default:
                fh_stringstream ss;
                ss << "strange problem with decompress() rc:" << rc;
                Throw_CompressionException( tostr(ss), 0 );
            }
        }
    }
#endif
#ifdef FERRIS_HAVE_BZIP2
    if( algo == Factory::COMPRESS_BZIP2 )
    {
        int verbosity = 0;
        int small = 0;
        unsigned int dsz = dst_sz;
        
        int rc = BZ2_bzBuffToBuffDecompress ( (char*)dst, &dsz,
                                              (char*)src, src_sz,
                                              small, verbosity );
        ret = dsz;

        if( rc != BZ_OK )
        {
            fh_stringstream ss;
            ss << "Problem with bzip2 decompression rc:" << rc;
            Throw_CompressionException( tostr(ss), 0 );
        }
    }
#endif

    if( algo == Factory::COMPRESS_NONE )
    {
        memcpy( dst, src, src_sz );
        ret = src_sz;
    }
    return ret;
}



template<
    class _CharT,
    class _Traits = std::char_traits < _CharT >,
    class _Alloc  = std::allocator   < _CharT >,
    class _BufferSizers = ferris_basic_streambuf_virtual
//    class _BufferSizers = ferris_basic_streambuf_fourk
    >
class FERRISEXP_DLLLOCAL basic_chunkedfile_streambuf
    :
    public ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
{
    typedef basic_chunkedfile_streambuf< _CharT, _Traits, _Alloc, _BufferSizers > _Self;
    typedef ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers > _Base;
    
public:

    /**
     * Mapping of logical block number to file name number
     */
    intmap_t m_bitmap;

    /**
     * Current logical block being used.
     */
    int m_currentBlock;

    /**
     * size that each block should be close to in ideal world
     */
    int m_blocksize;

    /**
     * algo to use
     */
    int m_algo;

    /**
     * level of compression to use
     */
    int m_level;
    
    /**
     * Context that contains the chunks
     */
    fh_context m_ctx;
    
    typedef char_traits<_CharT>    traits_type;
    typedef typename traits_type::int_type  int_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::pos_type  pos_type;
    typedef typename traits_type::off_type  off_type;

    basic_chunkedfile_streambuf( fh_context c, int blocksize )
        :
        _Base( new char_type[ blocksize ], blocksize ),
        m_ctx( c ),
        m_blocksize( blocksize ),
        m_currentBlock( 0 ),
        m_algo( Factory::COMPRESS_NONE ),
        m_level( 1 )
        {
            this->setBufSize( blocksize );

            m_algo  = toint(getStrAttr( c, COMPRESSION_ALGO,  tostr(Factory::COMPRESS_NONE), true, true ));
            m_level = toint(getStrAttr( c, COMPRESSION_LEVEL, tostr(1), true, true ));

            readBitmap( m_ctx, m_bitmap );
        }

    virtual ~basic_chunkedfile_streambuf()
        {
//            cerr << "~basic_chunkedfile_streambuf(1)" << endl;
            this->ensureMode( this->mode_mute );
            writeBitmap( m_ctx, m_bitmap );
        }

    void delete_all_chunks()
        {
            for( int i=0; m_bitmap.end() != m_bitmap.find( i ); ++i )
            {
                int phyBlock = m_bitmap[ i ];
                string rdn = tostr( phyBlock );
                LG_IOSTREAM_D << "Removing chunk at:" << rdn << endl;
                m_ctx->remove( rdn );
            }
        }
    

protected:

    /**
     * Write out the data starting at buffer of length sz to the "external"
     * device.
     * 
     * return -1 for error or 0 for success
     */
    virtual int write_out_given_data( const char_type* buffer, std::streamsize sz )
        {
            if( m_bitmap.end() == m_bitmap.find( m_currentBlock ) )
            {
                m_bitmap[ m_currentBlock ] = m_currentBlock;
            }
            int phyBlock = m_bitmap[ m_currentBlock ];
            m_currentBlock++;
            string rdn = tostr( phyBlock );
            
            if( !m_ctx->isSubContextBound( rdn ))
            {
                Shell::CreateFile( m_ctx, rdn );
            }
            fh_context pc   = m_ctx->getSubContext( rdn );
            fh_iostream ioss = pc->getIOStream( ios::trunc | ios::out );

            char* compressed_buffer = compressed_buffer_alloc( m_blocksize );
            int compressed_sz       = cf_compress( m_algo,
                                                   compressed_buffer,
                                                   compressed_buffer_size( m_blocksize ),
                                                   buffer, sz,
                                                   m_level );
            ioss.write( compressed_buffer, compressed_sz );
            LG_IOSTREAM_D << "write_out_given_data() blocknum:" << m_currentBlock
                          << " compressed_sz:" << compressed_sz
                          << " input_sz:" << sz
                          << endl;
            compressed_buffer_free( compressed_buffer );
            
            return ioss.good() ? 0 : -1; 
        }

    void dump_bitmap_to_cerr()
        {
            LG_IOSTREAM_ER << "dump_bitmap_to_cerr()" << endl;
            for( intmap_t::iterator bi = m_bitmap.begin(); bi != m_bitmap.end(); ++bi )
            {
                LG_IOSTREAM_ER << "logical:" << bi->first << " phy:" << bi->second << endl;
            }
        }
    
    /**
     * This is the only methods that really needs to be here. It gets
     * up to maxsz data into buffer and returns how much data was really
     * read. Return 0 for a failure, you must read atleast one byte.
     */
    virtual int make_new_data_avail( char_type* buffer, std::streamsize maxsz )
        {
            if( m_bitmap.end() == m_bitmap.find( m_currentBlock ) )
            {
                LG_IOSTREAM_ER << "make_new_data_avail() m_currentBlock:" << m_currentBlock
                              << " not in bitmap" << endl;
                dump_bitmap_to_cerr();
                return 0;
            }
            int phyBlock = m_bitmap[ m_currentBlock ];
            m_currentBlock++;
            fh_context pc = m_ctx->getSubContext( tostr( phyBlock ) );
            int pc_sz = toint(getStrAttr( pc, "size", tostr(maxsz) ));
//            if( pc_sz > maxsz ) pc_sz = maxsz;

            fh_istream iss = pc->getIStream();
            char* compressed_buffer = new char[ pc_sz + 1 ];
            iss.read( compressed_buffer, pc_sz );
            int real_sz = cf_decompress( m_algo,
                                         buffer,            m_blocksize,
                                         compressed_buffer, iss.gcount(),
                                         m_level );
            delete [] compressed_buffer;
            
            LG_IOSTREAM_D << "make_new_data_avail() block:" << (m_currentBlock-1)
                          << " read:" << iss.gcount() << endl;
            
            return real_sz;
        }
    
private:

    // prohibit copy/assign
    basic_chunkedfile_streambuf( const basic_chunkedfile_streambuf& );
    basic_chunkedfile_streambuf& operator = ( const basic_chunkedfile_streambuf& );
};


template<
    typename _CharT,
    typename _Traits = std::char_traits < _CharT >,
    typename _Alloc  = std::allocator   < _CharT >
>

class FERRISEXP_DLLLOCAL ChunkedIOStream
    :
    public Ferris_iostream< _CharT, _Traits >
{
    typedef ChunkedIOStream<_CharT, _Traits,_Alloc>   _Self;

    typedef basic_chunkedfile_streambuf<_CharT, _Traits> ss_impl_t;
    FERRIS_SMARTPTR( ss_impl_t, ss_t );
    ss_t ss;
    
public:

    typedef typename std::char_traits<_CharT> traits_type;
    typedef typename traits_type::int_type    int_type;
    typedef typename traits_type::char_type   char_type;
    typedef typename traits_type::pos_type    pos_type;
    typedef typename traits_type::off_type    off_type;
    typedef emptystream_methods< _CharT, _Traits > delegating_methods;

    ChunkedIOStream( fh_context c, int blocksize )
        :
        ss( new ss_impl_t( c, blocksize ))
        {
            this->init( rdbuf() );
            this->setsbT( GetImpl(ss) );
//            cerr << "ChunkedIOStream() c:" << c->getURL() << endl;
        }

    ChunkedIOStream( const ChunkedIOStream& rhs )
        :
        ss( rhs.ss )
        {
            this->init( rdbuf() );
            this->setsbT( GetImpl(ss) );
        }

    
    
    virtual ~ChunkedIOStream()
        {}
    
    _Self* operator->()
        {
            return this;
        }

    ss_impl_t*
    rdbuf() const
        {
            return GetImpl(ss);
        }


    enum
    {
        stream_readable = true,
        stream_writable = true
    };
};


/********************************************************************************/
/********************************************************************************/

bool isCompressedContext( fh_context c )
{
    return c->isCompressedContext();
}

    namespace Factory
    {
        ConvertToCompressedChunkContextProgress_Sig_t& getNullConvertToCompressedChunkContextProgress_Sig()
        {
            static ConvertToCompressedChunkContextProgress_Sig_t s;
            return s;
        }
        
        
        static void checkIfSupportingAlgo( fh_context c, int algo )
        {
            if( algo == COMPRESS_INVALID )
            {
                fh_stringstream ss;
                ss << "Invalid compression chosen";
                Throw_CompressionAlgoNotFoundException( tostr(ss), GetImpl(c) );
            }
#ifndef FERRIS_HAVE_LIBZ
            if( algo == COMPRESS_GZIP )
            {
                fh_stringstream ss;
                ss << "This version of libferris can not handle the algo chosen for compression";
                Throw_CompressionAlgoNotFoundException( tostr(ss), GetImpl(c) );
            }
#endif
#ifndef FERRIS_HAVE_BZIP2
            if( algo == COMPRESS_BZIP2 )
            {
                fh_stringstream ss;
                ss << "This version of libferris can not handle the algo chosen for compression";
                Throw_CompressionAlgoNotFoundException( tostr(ss), GetImpl(c) );
            }
#endif
        }
        
        static ChunkedIOStream<char> priv_getCompressedChunkIOStream( fh_context c )
        {
            int algo      = toint(getStrAttr( c, COMPRESSION_ALGO,    "0", true, true ));
            int blocksize = toint(getStrAttr( c, COMPRESSION_BLOCKSZ, "0", true, true ));
            checkIfSupportingAlgo( c, algo );

//             intmap_t bm;
//             readBitmap( c, bm );

            return ChunkedIOStream<char>( c, blocksize );
        }

        
        fh_iostream getCompressedChunkIOStream( fh_context c )
        {
            return priv_getCompressedChunkIOStream( c );
        }

        static void priv_ConvertToCompressedChunkContext( fh_context c,
                                                          fh_context target,
                                                          int blocksize,
                                                          int algo,
                                                          int compress_level,
                                                          ConvertToCompressedChunkContextProgress_Sig_t& progress_sig )
        {
            checkIfSupportingAlgo( c, algo );

            if( blocksize == 0 )
            {
                blocksize = compress_level * 100 * 1024;
            }
            
            fh_istream iss = c->getIStream();
            int mode  = toint( getStrAttr( target, "mode", "644" ));
            mode     |= S_IXUSR;
            intmap_t bm;

            int totalblocks = toint(getStrAttr( c, "size", "0" )) / blocksize;
            
            char* buffer              = compressed_buffer_alloc( blocksize );
            char* uncompressed_buffer = new char[ blocksize + 1 ];
            for( int n=0; iss.good(); ++n )
            {
                iss.read( uncompressed_buffer, blocksize );

                // possibly compress buffer
                int compressedsz = cf_compress( algo,
                                                buffer,
                                                compressed_buffer_size( blocksize ),
                                                uncompressed_buffer, iss.gcount(),
                                                compress_level );

                LG_IOSTREAM_D << "ConvertToCompressedChunkContext() source block:" << blocksize
                              << " gcount:" << iss.gcount()
                              << " compressedsize:" << compressedsz
                              << endl;
                
                // write compressed chunk
                fh_context chunk = Shell::CreateFile( target, tostr(n), mode );
                fh_iostream oss  = chunk->getIOStream( ios::trunc | ios::out );
                oss.write( buffer, compressedsz );
//                 int wsz = oss.wcount();
//                 if( wsz != compressedsz )
                if( !oss->good() )
                {
                    fh_stringstream ss;
                    ss << "Error writing compressed chunk for:" << chunk->getURL();
                    Throw_CompressionAlgoNotFoundException( tostr(ss), GetImpl(c) );
                }

                // add to bitmap for compressed extents
                bm[ n ] = n;
                progress_sig.emit( c, target, n, totalblocks );
            }
            compressed_buffer_free( buffer );
            delete [] uncompressed_buffer;

            // store the bitmap of block num to file extent number from bm to an EA
            writeBitmap( target, bm );
            setStrAttr( target, COMPRESSION_SENTINAL, tostr(true),           true, true );
            setStrAttr( target, COMPRESSION_ALGO,     tostr(algo),           true, true );
            setStrAttr( target, COMPRESSION_BLOCKSZ,  tostr(blocksize),      true, true );
            setStrAttr( target, COMPRESSION_LEVEL,    tostr(compress_level), true, true );
        }
        
        void ConvertToCompressedChunkContext( fh_context c,
                                              fh_context target,
                                              int blocksize,
                                              int algo,
                                              int compress_level,
                                              ConvertToCompressedChunkContextProgress_Sig_t& progress_sig )
        {
            string rdn = c->getDirName();
            int mode   = toint( getStrAttr( c, "mode", "644" ));
            mode |= S_IXUSR;
            if( !target )
            {
                target = Shell::CreateDir( c->getParent(),
                                           c->getDirName() + ".compressed",
                                           false, mode );
            }
            priv_ConvertToCompressedChunkContext( c, target, blocksize, algo, compress_level, progress_sig );
            c->getParent()->remove( c->getDirName() );
            target->getParent()->rename( target->getDirName(), rdn );
        }
        
        

        void ConvertToCompressedChunkContext( fh_context c,
                                              int blocksize,
                                              int algo,
                                              int compress_level,
                                              ConvertToCompressedChunkContextProgress_Sig_t& progress_sig )
        {
            ConvertToCompressedChunkContext( c, 0, blocksize, algo, compress_level, progress_sig );
        }
        
        void ConvertFromCompressedChunkContext( fh_context c,
                                                fh_context target,
                                                ConvertToCompressedChunkContextProgress_Sig_t& progress_sig )
        {
            ChunkedIOStream<char> iss = priv_getCompressedChunkIOStream( c );
            fh_iostream oss = target->getIOStream( ios::trunc | ios::out );
            std::copy( std::istreambuf_iterator<char>(iss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(oss));
            iss->rdbuf()->delete_all_chunks();
        }
        
        void ConvertFromCompressedChunkContext( fh_context c,
                                                ConvertToCompressedChunkContextProgress_Sig_t& progress_sig )
        {
            string rdn = c->getDirName();
            
            int mode       = toint( getStrAttr( c, "mode", "644" ));
            fh_context target = Shell::CreateFile( c->getParent(),
                                                   c->getDirName() + ".uncompressed",
                                                   mode );
            ConvertFromCompressedChunkContext( c, target, progress_sig );
            c->getParent()->remove( c );
            target->getParent()->rename( target->getDirName(), rdn );
        }
        
    };


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


};
