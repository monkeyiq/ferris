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

    $Id: FerrisOpenSSL.cpp,v 1.4 2010/09/24 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <SignalStreams.hh>
#include <FerrisOpenSSL.hh>

#include <iostream>
#include <string>

using namespace std;

void InitOpenSSL()
{
    bool virgin = true;

    if( virgin )
    {
        virgin = false;
        SSL_load_error_strings();
//        SSLeay_add_ssl_algorithms();
        SSL_library_init();
//        actions_to_seed_PRNG();
    }
}

namespace Ferris
{
    std::string base64encode( const std::string& v )
    {
        stringstream ret;
        
        BIO *bmem, *b64;
        BUF_MEM *bptr;

        b64 = BIO_new(BIO_f_base64());
        bmem = BIO_new(BIO_s_mem());
        b64 = BIO_push(b64, bmem);
        BIO_write(b64, v.data(), v.size() );
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &bptr);

        ret.write( bptr->data, bptr->length );
//        ret << '\0';
        BIO_free_all(b64);

        return ret.str();
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    template<
        class _CharT,
        class _Traits = std::char_traits < _CharT >,
        class _Alloc  = std::allocator   < _CharT >,
        class _BufferSizers = ferris_basic_streambuf_fourk
        >
    class FERRISEXP_DLLLOCAL basic_digest_streambuf
        :
        public ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
        {
            typedef ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers > _Base;

            // prohibit copy/assign
            basic_digest_streambuf( const basic_digest_streambuf& );
            basic_digest_streambuf& operator = ( const basic_digest_streambuf& );

            EVP_MD_CTX mdctx;
            const EVP_MD *md;
            unsigned char md_value[EVP_MAX_MD_SIZE];
            unsigned int md_len;
            string m_digestValue;
    
        public:
    
            typedef char_traits<_CharT>    traits_type;
            typedef typename traits_type::int_type  int_type;
            typedef typename traits_type::char_type char_type;
            typedef typename traits_type::pos_type  pos_type;
            typedef typename traits_type::off_type  off_type;
            typedef typename _Base::seekd_t seekd_t;


            basic_digest_streambuf( const std::string& DigestName )
            {
                InitOpenSSL();
                md = EVP_get_digestbyname(DigestName.c_str());
                if( !md )
                {
                    m_digestValue = "N/A";
                }
                EVP_DigestInit(&mdctx, md);
            }

            virtual ~basic_digest_streambuf()
            {
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
                EVP_DigestUpdate(&mdctx, buffer, sz );
                return sz;
            }


            virtual int make_new_data_avail( char_type* buffer, streamsize maxsz )
            {
                cerr << "make_new_data_avail() m_digestValue.empty():" << m_digestValue.empty() << " maxsz:" << maxsz << endl;

                if( !m_digestValue.empty() )
                    return -1;
                
                    
                if( m_digestValue.empty() )
                {
                    EVP_DigestFinal(&mdctx, md_value, &md_len);
                    fh_stringstream ret;
                    radixdump( ret, md_value, md_value + md_len, 16 );
                    m_digestValue = ret.str();
                    cerr << "md_len:" << md_len << endl;
                    cerr << "m_digestValue:" << m_digestValue << endl;
                }
                int sz = std::min( (streamsize)m_digestValue.size(), maxsz );
                cerr << "sz:" << sz << endl;
                memcpy( buffer, m_digestValue.c_str(), sz );
                return sz;
            }
        };

    
    /**
     * Digest output to two different streams at once.
     */
    template<
        class _CharT,
        class _Traits = std::char_traits<_CharT>,
        class _Alloc  = std::allocator   < _CharT >,
        class _BufferSizers = ferris_basic_streambuf_fourk
        >
    class FERRISEXP_DLLLOCAL ferris_digest_iostream
        :
        public Ferris_iostream< _CharT, _Traits >,
        public io_ferris_stream_traits< _CharT, _Traits >
        {
            typedef Ferris_iostream< _CharT, _Traits >         _Base;
            typedef ferris_digest_iostream<_CharT, _Traits>    _Self;

        public:
    
            typedef char_traits<_CharT>    traits_type;
            typedef typename traits_type::int_type  int_type;
            typedef typename traits_type::char_type char_type;
            typedef typename traits_type::pos_type  pos_type;
            typedef typename traits_type::off_type  off_type;
            typedef basic_digest_streambuf<_CharT, _Traits, _Alloc, _BufferSizers> ss_impl_t;
            typedef emptystream_methods< char_type, traits_type > delegating_methods;
    
            explicit
                ferris_digest_iostream( const std::string& DigestName )
                :
                _Base( new ss_impl_t( DigestName ) )
            {}

            ferris_digest_iostream( const ferris_digest_iostream& rhs )
                : _Base( rhs.ss )
            {}
            virtual ~ferris_digest_iostream() {}
        };

    
    namespace Factory
    {
        fh_iostream MakeDigestStream( std::string digestName )
        {
            ferris_digest_iostream<char> ret( digestName );
            return ret;
        }
    };
    
    
};
