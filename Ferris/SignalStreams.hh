/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: SignalStreams.hh,v 1.6 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_SIGNALSTREAMS_H_
#define _ALREADY_INCLUDED_SIGNALSTREAMS_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <glib.h>

#include <sstream>
#include <string>
#include <ios>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <AssocVector.h>

#include <FerrisHandle.hh>
#include <TypeDecl.hh>

#include <SmartPtr.h>
#include <Functor.h>

#include <sigc++/sigc++.h>

#include <stdio.h>
#include <stdlib.h>

#include <FerrisStreams/All.hh>

namespace Ferris
{
template < class T >
struct gStreamPosRAII
{
    std::streampos restore;
    T& stream;
    
    gStreamPosRAII( T& stream )
        :
        stream( stream )
        {
            restore = stream->tellg();
        }
    ~gStreamPosRAII()
        {
            stream->seekg( restore );
        }
};

struct giStreamPosRAII : public gStreamPosRAII< fh_istream >
{
    giStreamPosRAII( fh_istream& stream )
        :
        gStreamPosRAII< fh_istream >( stream )
        {
        }
};

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996-1998
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */
/**
 * This was modified only to put the delimiter between items.
 * ie. foo,bar,baz
 * instead of the standard foo,bar,baz,
 *
 * osid_iterator = Output Stream Internal Delimiter iterator
 */
template <class _TpP,
          class _CharT = char, class _Traits = std::char_traits<_CharT> >
class osid_iterator
//: public std::iterator<std::output_iterator_tag, void, void, void, void>
{
  typedef osid_iterator<_TpP, _CharT, _Traits> _Self;
    bool m_virgin;
public:
//    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void*;
//    using reference = void&;
    
    typedef _CharT                         char_type;
    typedef _Traits                        traits_type;
    typedef std::basic_ostream<_CharT, _Traits> ostream_type;

    typedef std::output_iterator_tag            iterator_category;

    osid_iterator(ostream_type& __s) : _M_stream(&__s), _M_string(0), m_virgin(true) {}
  osid_iterator(ostream_type& __s, const _CharT* __c) 
      : _M_stream(&__s), _M_string(__c), m_virgin(true)  {}
  _Self& operator=(const _TpP& __val) { 
    if (_M_string)
    {
        if( m_virgin )
            m_virgin = false;
        else
            *_M_stream << _M_string;
    }
    *_M_stream << __val;
    return *this;
  }
  _Self& operator*() { return *this; }
  _Self& operator++() { return *this; } 
  _Self& operator++(int) { return *this; } 
private:
  ostream_type* _M_stream;
  const _CharT* _M_string;
};
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
//     namespace Factory
//     {
//         /**
//          * Make a istream for a memory mapped version of fd
//          *
//          * fd is the file descriptor to memory map.
//          * @param m describes extra information relating to the openmode of fd and
//          * how to memory map fd. If ferris_ios::mseq is set then madvise is done to
//          * tell the kernel about access patterns.
//          * @param ctx if provided is used to embed a URL into exceptions that are thrown.
//          */
//         fh_istream  MakeMMapIStream( int fd,
//                                     ferris_ios::openmode m = ios::in,
//                                     fh_context ctx = 0 );

        
//         /**
//          * Make a iostream for a memory mapped version of fd
//          *
//          * Note that the memory map automatically extends itself if you write past
//          * end of file. The file is extended in largish chunks and remaped for you.
//          *
//          * fd is the file descriptor to memory map.
//          * @param m describes extra information relating to the openmode of fd and
//          * how to memory map fd. If ferris_ios::mseq is set then madvise is done to
//          * tell the kernel about access patterns.
//          * @param ctx if provided is used to embed a URL into exceptions that are thrown.
//          */
//         fh_iostream MakeMMapIOStream( int fd,
//                                     ferris_ios::openmode m = ios::in | ios::out,
//                                     fh_context ctx = 0 );
        
//     };

    /**
     * Return true if this context has been compressed.
     */
    FERRISEXP_API bool isCompressedContext( fh_context c );
    
    namespace Factory
    {
        enum {
            COMPRESS_INVALID = 0,
            COMPRESS_NONE  = 1<<0,
            COMPRESS_GZIP  = 1<<1,
            COMPRESS_BZIP2 = 1<<2
        };

        typedef sigc::signal< void ( fh_context, fh_context, int, int ) >
        ConvertToCompressedChunkContextProgress_Sig_t;
        FERRISEXP_API ConvertToCompressedChunkContextProgress_Sig_t&
        getNullConvertToCompressedChunkContextProgress_Sig();

        
        /**
         * Make an iostream that handles compressed "chunks" of data and makes
         * a streambuf that is exactly on decompressed chunk in length.
         *
         * The returned stream allows random access read/write to the chunks.
         *
         * This method uses sane defaults for block size and algo. There is a overloaded
         * method that allows one to select both for converting a file into a compressed
         * chunked file.
         *
         * @param c The context that the compressed chunks exist under. Note that
         * ea is used on this context to setup block sizes and decompression if
         * it is not overriden in the parameters to this function.
         */
        FERRISEXP_API fh_iostream getCompressedChunkIOStream( fh_context c );

        /**
         * Convert a context into a compressed chunked context.
         *
         * @param c The context whos getIOStream() will be chunked and converted
         *          into compressed contents.
         * @param target You can create any type of context that supports EA and subcontexts
         *        and use it as the target. You will have to remove the source 'c' context
         *        yourself after calling the function. if target==0 then a directory target is
         *        assumed and created for you.
         */
        FERRISEXP_API void ConvertToCompressedChunkContext(
            fh_context c,
            fh_context target,
            int blocksize = 512*1024,
            int algo = COMPRESS_GZIP,
            int compress_level = 1,
            ConvertToCompressedChunkContextProgress_Sig_t& =
            Ferris::Factory::getNullConvertToCompressedChunkContextProgress_Sig());
        
        
        /**
         * Convert a context into a compressed chunked context.
         *
         * @param c The context whos getIOStream() will be chunked and converted
         *          into compressed contents.
         */
        FERRISEXP_API void ConvertToCompressedChunkContext(
            fh_context c,
            int blocksize = 512*1024,
            int algo = COMPRESS_GZIP,
            int compress_level = 1,
            ConvertToCompressedChunkContextProgress_Sig_t& =
            Ferris::Factory::getNullConvertToCompressedChunkContextProgress_Sig());

        /**
         * Convert a chunked compressed file back into a normal one and delete the
         * no longer needed compressed chunks.
         *
         * Note that 'c' will still exist after the method call, only the chunks themself
         * are reclaimed. You may wish to use the one param version of this method instead.
         *
         * @param c The compressed context with subcontexts being the chunks
         * @param target Where to decompress to
         */
        FERRISEXP_API void ConvertFromCompressedChunkContext(
            fh_context c,
            fh_context target,
            ConvertToCompressedChunkContextProgress_Sig_t& =
            Ferris::Factory::getNullConvertToCompressedChunkContextProgress_Sig());
        
        /**
         * Convert a chunked compressed file back into a normal one and delete the
         * no longer needed compressed chunks.
         *
         * This is a simple operation but can be handy for defragmenting a context
         * that has changed alot since it was chunked.
         *
         * Note that if the compressed object was a file before compression it will
         * be converted back into a file when the chunks are deleted.
         *
         * @param the context this is compressed, ie. that contains the compressed chunks.
         */
        FERRISEXP_API void ConvertFromCompressedChunkContext(
            fh_context c,
            ConvertToCompressedChunkContextProgress_Sig_t& =
            Ferris::Factory::getNullConvertToCompressedChunkContextProgress_Sig());
    };
    
};


#endif // ifndef _ALREADY_INCLUDED_SIGNALSTREAMS_H_



