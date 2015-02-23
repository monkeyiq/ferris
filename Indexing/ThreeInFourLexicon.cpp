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

    $Id: ThreeInFourLexicon.cpp,v 1.4 2010/09/24 21:31:08 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

/**
 * Implementation of partially front coded (currently using "3-in-4")
 * compressed lexicon storage with insertion support.
 *
 * Assumes that the whole lexicon can be stored in main memory temporarily
 *
 * On disk format is
 * number of disk_blockn entries as 4 bytes
 * disk_block1              of disk_block_size bytes
 * disk_block2              of disk_block_size bytes
 * disk_blockn              of disk_block_size bytes
 * disk_block_index         of sizeof(disk_block_index) bytes
 * sizeof(disk_block_index) as 4 bytes
 * version of file format   as 4 bytes
 *
 * disk_block_index format is
 * length(first_string_of_disk_block)  as 1 byte
 * first_string_of_disk_block as       first_string_of_disk_block bytes
 * repeat last 2 items
 * the block_number is infered from the position in the list, starting at block 0
 * and having the final block_number record right up against the
 * sizeof(disk_block_index==4) at eof
 *
 * disk_block format is
 * number of fourpacks                   as sizeof(blockoffset_t) bytes
 * fourpack1 offset from start of block  as sizeof(blockoffset_t) bytes
 * fourpack2 offset                      as sizeof(blockoffset_t) bytes
 * fourpackn offset                      as sizeof(blockoffset_t) bytes
 * fourpack1
 * fourpack2
 * fourpackn
 *
 * fourpack format is
 * s1.length               as 1 byte
 * s1                      as s1.length bytes
 * s1_id                   as sizeof( termid_t ) bytes
 * s2.prefix               as 1 byte (# of bytes from start of s1 to copy as s2.prefix)
 * s2.length               as 1 byte
 * s2                      as s2.length bytes
 * s2_id                   as sizeof( termid_t ) bytes
 * s3.prefix               as 1 byte (# of bytes from start of s2 to copy as s3.prefix)
 * s3.length               as 1 byte
 * s3                      as s3.length bytes
 * s3_id                   as sizeof( termid_t ) bytes
 * s4.prefix               as 1 byte (# of bytes from start of s3 to copy as s3.prefix)
 * s4.length               as 1 byte ( this is redundant, can be invered, maybe remove )
 * s4                      as s4.length bytes
 * s4_id                   as sizeof( termid_t ) bytes
 * 
 */

#include <config.h>

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "IndexPrivate.hh"
#include <Configuration_private.hh>
#include <FerrisBackup.hh>

#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <iomanip>

#include <errno.h>

using namespace std;


namespace Ferris
{
    namespace FullTextIndex 
    {
        typedef gint32 tifl_version_t;
        static const int TIFL_VERSION_BAD = 0;
        static const int TIFL_VERSION_1   = 1;
        
        /**
         * Get a string of the common prefix between two strings
         */
        static string common_prefix( const std::string& s1, const std::string& s2 )
        {
            int numCommon = 0;
            int minlen = min( s1.length(), s2.length() );
            
            for( int i=0; i<minlen; ++i )
            {
                if( s1[i] != s2[i] )
                    break;
                ++numCommon;
            }
            string ret = s1.substr( 0, numCommon );
            return ret;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        typedef guint32 blocknum_t;
        typedef guint16 blockoffset_t;
        static const int disk_block_size = 4096;
        static const int terms_per_fourpack = 4;
        
        class FourPack;
        FERRIS_SMARTPTR( FourPack, fh_fourpack );

        class DiskBlock;
        FERRIS_SMARTPTR( DiskBlock, fh_block );

        class Lexicon_FrontCodedBlocks;
        FERRIS_SMARTPTR( Lexicon_FrontCodedBlocks, fh_fclex );

        class DiskBlock_Reverse;
        FERRIS_SMARTPTR( DiskBlock_Reverse, fh_revblock );

        class ReverseLexicon_FrontCodedBlocks;
        FERRIS_SMARTPTR( ReverseLexicon_FrontCodedBlocks, fh_revfclex );
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Collection of 4 items in a "fourpack".
         *
         * Handles loading and saving and other such methods for a very small
         * collection of items
         */
        class FERRISEXP_DLLLOCAL FourPack
            :
            public Handlable
        {
            /**
             * Store the elements in a small map to minimize compare ops
             * for lookup()
             */
            typedef map< string, termid_t > overflow_t;
            overflow_t overflow;

            FourPack();
            FourPack( fh_iostream& ss );
            FourPack( overflow_t::iterator iter, overflow_t::iterator ei );
            FourPack( map< termid_t, string >::iterator iter,
                      map< termid_t, string >::iterator ei );

            list< termid_t > getSortedTids();
            
        public:

            /**
             * resolve the given term, 0 for failure.
             */
            termid_t lookup( const std::string& term );

            /**
             * Perform a reverse lookup. Note that this method is slightly slower
             * than lookup( string ) because it is currently linear time.
             * (doesn't matter much for 4 items)
             *
             * REVERSE
             */
            const std::string& lookup( termid_t tid );

            /**
             * Create a new fourpack with the elements from iter to ei
             * ei-iter should <= 4 items
             */
            static fh_fourpack Create( overflow_t::iterator iter, overflow_t::iterator ei );
            static fh_fourpack Create( map< termid_t, string >::iterator iter,
                                       map< termid_t, string >::iterator ei );

            /**
             * first term of pack
             */
            string firstTerm();

            /**
             * Load a fourpack from the given data
             */
            static fh_fourpack Read( fh_iostream& ss );

            /**
             * Create a packed version of this fourpack, prefix coding the
             * greatest 3 strings
             */
            string tostr( bool WriteInTidOrder = false );

            /**
             * Creates a reasonable human readable version of the fourpack
             * for debuging sessions
             */
            string getDebugString();
            
            /**
             * return the size of the packed data, ie. tostr().size()
             * use this so that caching can be added or the size explicitly
             * calculated in a faster method than by making the string each time.
             */
            streamsize size( bool WriteInTidOrder = false );

            /**
             * next term from the one given or "" if no next term in this fourpack
             */
            string nextTerm( string s );

            
            /**
             * Allow walking in reverse lookup mode
             *
             * REVERSE
             */
            termid_t firstTid();
            /**
             * Allow walking in reverse lookup mode
             *
             * REVERSE
             */
            termid_t nextTid( termid_t tid );
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        /**
         * Disk block full of fourpack items.
         * Each block contains a small header with pointers to the byte offset
         * within the block of each fourpack
         *
         */
        class FERRISEXP_DLLLOCAL DiskBlock
            :
            public Handlable
        {
            
        protected:
            
            fh_fclex      m_lex;
            blocknum_t    m_blocknum;
            blockoffset_t m_blockUsedBytes; //< mainly used in Create()/append() to keep track of size
            bool          m_readHeader;     //< once the header has been read ensure it isn't again
            bool          m_readFourpacks;  //< to prevent fourpacks being read more than once. also for
                                            //  new blocks both m_read* are set to stop attempts to read

            /**
             * byte offset of each fourpack in the block
             */
            typedef vector< blockoffset_t > fp_addrs_t;
            fp_addrs_t fp_addrs;

            /**
             * cache the decoded fourpacks here.
             */
            typedef map< string, fh_fourpack > m_fps_t;
            m_fps_t m_fps;
            
            DiskBlock( fh_fclex m_lex, blocknum_t blocknum, bool created );
            /**
             * Used by Reverse diskblock to create a block with virtual getStream() binding.
             */
            DiskBlock( blocknum_t blocknum, bool created ); 

            /**
             * Seeks the stream to the start of where this block is stored
             * ready for reading.
             */
            void seekStartBlockG( fh_iostream& ss );

            /**
             * Mainly for debug, reads all the fourpacks off the block into
             * m_fps;
             */
            void readAllFourPacks();

            /**
             * maybe read in the header info from the disk. 
             */
            void readHeader();

            /**
             * As a side effect the m_lex->getStream() will be moved to just
             * after the header of the block
             */
            void skipHeader();

            /**
             * Because subclasses might wish to maintain secondary index(es)
             * fourpacks that are read/added are done via this method.
             * The default creates a primary index with m_fps and should
             * always be called aswell as any subclass impl.
             */
            virtual void priv_insertFourpack( fh_fourpack fp );
            
        public:

            /**
             * Read back a disk block into RAM. Note that this method may not
             * fully read the block, it may instead only read part of the block
             * for example if you lookup() on the block not all fourpacks are
             * always decoded.
             */
            static fh_block Read( fh_fclex m_lex, blocknum_t n );

            /**
             * Create a new diskblock ready to append() with elements
             */
            static fh_block Create( fh_fclex m_lex, blocknum_t n );

            /**
             * try to append the fourpack to this diskblock, if it doesn't
             * fit then false is returned
             */
            bool append( fh_fourpack fp );

            /**
             * After the sequence of Create(), many append() calls, the manager can
             * call Write() with the stream at the correct location for putting this
             * block
             */
            void Write( fh_iostream& ss );

            /**
             * find the given term in the fourpacks contained in this block
             * 0 for failure
             */
            termid_t lookup( const std::string& term );

            /**
             * first term of pack
             */
            string firstTerm();
            
            /**
             * next term from the one given or "" if no next term in this fourpack
             */
            string next( string s );

            /**
             * get the stream associated with the disk block, seeked for reading at the
             * start of block
             */
            virtual fh_iostream getStream();

            virtual bool getWriteInTidOrder();
            
            
            /**
             * What block is this
             */
            blocknum_t getBlockNumber()
                {
                    return m_blocknum;
                }
        };

        /**
         * A DiskBlock that maintains an in memory reverse index so that
         * termid -> fh_fourpack resolution is a fast access method
         */
        class FERRISEXP_DLLLOCAL DiskBlock_Reverse
            :
            public DiskBlock
        {
            typedef DiskBlock _Base;

            fh_revfclex  m_lex;
            
        protected:

            /**
             * reverse lookup cache of the decoded fourpacks.
             */
            typedef map< termid_t, fh_fourpack > m_revfps_t;
            m_revfps_t m_revfps;

            m_revfps_t::iterator m_walkIter; //< used in firstTerm()/next() to walk fourpacks

            DiskBlock_Reverse( fh_revfclex m_lex, blocknum_t blocknum, bool created );
            
            /**
             * Also maintain a secondary index m_revfps
             */
            virtual void priv_insertFourpack( fh_fourpack fp );
            
        public:
            
            /**
             * Read back a disk block into RAM. Note that this method may not
             * fully read the block, it may instead only read part of the block
             * for example if you lookup() on the block not all fourpacks are
             * always decoded.
             */
            static fh_revblock Read( fh_revfclex lex, blocknum_t n );

            /**
             * Create a new diskblock ready to append() with elements
             */
            static fh_revblock Create( fh_revfclex lex, blocknum_t n );
            
            /**
             * find the given term in the fourpacks contained in this block
             * "" for failure
             */
            std::string lookup( termid_t tid );
            
            /**
             * first term of pack
             */
            termid_t firstTid();
            
            /**
             * next term from the one given or "" if no next term in this fourpack
             */
            termid_t next( termid_t s );

            /**
             * get the stream associated with the disk block, seeked for reading at the
             * start of block
             */
            virtual fh_iostream getStream();

            virtual bool getWriteInTidOrder();
        };
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        /**
         * Lexicon for storage in 3in4 front coding
         */
        class FERRISEXP_DLLLOCAL Lexicon_FrontCodedBlocks
            :
            public Lexicon
        {
            typedef Lexicon_FrontCodedBlocks _Self;
            typedef Lexicon                  _Base;

            fh_fstream m_stream;
            /**
             * Overflow is used to store new terms until sync()
             */
            typedef map< string, termid_t > overflow_t;
            overflow_t overflow;

            /**
             * Index to each disk_block_size byte disk block
             * seek address = iter->second * disk_block_size;
             */
            typedef map< string, blocknum_t > blockindex_t;
            blockindex_t blockindex;

        protected:

            
            /**
             * reading and writing the blockindex map to disk.
             * write() assumes we are at the correct location
             * for their data in m_stream
             */
            void blockindex_read();
            void blockindex_write();

            /**
             * acquire the block from disk or cache. Use this in preference
             * to DiskBlock::Read() so that caching may be done on some of
             * the most recent blocks and thus much greater speed.
             */
            fh_block getBlock( blocknum_t b );
            fh_block m_getBlock_cache;
            
            virtual void setIndex( fh_idx idx );
            virtual std::string getFirstTerm();
            virtual std::string getNextTerm( const std::string& s );

        public:
            
            Lexicon_FrontCodedBlocks( fh_idx idx = 0, PathManager* path_mgr = 0 );
            virtual ~Lexicon_FrontCodedBlocks();

            virtual void     insert( const std::string& term, termid_t termid );
            virtual termid_t lookup( const std::string& term );
            virtual void sync();

            fh_iostream getStream();

            virtual void prepareForInsertions();

            
            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "FrontCodedBlocks (3-in-4)"; }
        };
        bool Lexicon_FrontCodedBlocks::reged = LexiconFactory::Instance().
        Register( Lexicon_FrontCodedBlocks::getClassName(),
                  &MakeObject<Lexicon,Lexicon_FrontCodedBlocks>::Create );
        bool Lexicon_FrontCodedBlocks::regedx = appendToLexiconClassNames(
            Lexicon_FrontCodedBlocks::getClassName() );


        static bool alias_rg = LexiconFactory::Instance().
        Register( "FrontCodedBlocks",
                  &MakeObject<Lexicon,Lexicon_FrontCodedBlocks>::Create );
        static bool alias_rx = appendToLexiconAliasNames( "FrontCodedBlocks" );


        /********************/
        /********************/
        /********************/

        class FERRISEXP_DLLLOCAL ReverseLexicon_FrontCodedBlocks
            :
            public  ReverseLexicon
        {
            typedef ReverseLexicon_FrontCodedBlocks _Self;
            typedef ReverseLexicon                  _Base;

            /**
             * Overflow is used to store new terms until sync()
             */
            typedef map< termid_t, string > overflow_t;
            overflow_t overflow;
            overflow_t::iterator m_walkIter; //< used by getFirst/NextTerm()

            /**
             * secondary index to each disk_block_size byte disk block
             * seek address = iter->second * disk_block_size;
             */
            typedef map< termid_t, blocknum_t > blockindex_t;
            blockindex_t blockindex;

            fh_fstream m_stream;
            
            /**
             * reading and writing the blockindex map to disk.
             * write() assumes we are at the correct location
             * for their data in m_stream
             */
            void blockindex_read();
            void blockindex_write();

            /**
             * acquire the block from disk or cache. Use this in preference
             * to DiskBlock::Read() so that caching may be done on some of
             * the most recent blocks and thus much greater speed.
             */
            fh_revblock getBlock( blocknum_t b );
            fh_revblock m_getBlock_cache;
            
        protected:

            virtual void setIndex( fh_idx idx = 0 );
            virtual termid_t getFirstTerm();
            virtual termid_t getNextTerm( termid_t t );

        public:

            ReverseLexicon_FrontCodedBlocks();
            virtual ~ReverseLexicon_FrontCodedBlocks();
            
            virtual string lookup( termid_t tid );
            virtual void insert( const std::string& s, termid_t id );
            virtual bool exists( termid_t id );

            virtual void sync();
            virtual void prepareForInsertions();

            string getStreamFileName();
            fh_fstream getStream();
            fh_fstream getBlockIndexStream();
            
            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "FrontCodedBlocks (3-in-4)"; }
        };
        bool ReverseLexicon_FrontCodedBlocks::reged = ReverseLexiconFactory::Instance().
        Register( ReverseLexicon_FrontCodedBlocks::getClassName(),
                  &MakeObject<ReverseLexicon,ReverseLexicon_FrontCodedBlocks>::Create );
        bool ReverseLexicon_FrontCodedBlocks::regedx = appendToReverseLexiconClassNames(
            ReverseLexicon_FrontCodedBlocks::getClassName() );


        static bool ralias_rg = ReverseLexiconFactory::Instance().
        Register( "FrontCodedBlocks",
                  &MakeObject<ReverseLexicon,ReverseLexicon_FrontCodedBlocks>::Create );
//        static bool ralias_rx = appendToReverseLexiconAliasNames( "FrontCodedBlocks" );
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        FourPack::FourPack( )
        {}

        /**
         * Load a fourpack from the given data
         */
        FourPack::FourPack( fh_iostream& ss )
        {
            string term;
            termid_t termid;
            guint8 len = 0;
            readnum( ss, len );
            term = readstring( ss, len );
            readnum( ss, termid );
            overflow.insert( make_pair( term, termid ));
//            LG_FTXLEXI_D << "FourPack() LEADING term:" << term << " termid:" << termid << endl;

            for( int i = 0; i < (terms_per_fourpack-1); ++i )
            {
                termid = 0;
                len    = 0;
                guint8 prefixsz = 0;
                
                readnum( ss, prefixsz );
                readnum( ss, len );
                string suffix = readstring( ss, len );
                term = term.substr( 0, prefixsz ) + suffix;
                readnum( ss, termid );

                if( !term.empty() )
                    overflow.insert( make_pair( term, termid ));
                
//                 LG_FTXLEXI_D << "FourPack() term:" << term << " termid:" << termid
//                              << " prefixsz:" << (int)prefixsz
//                              << " len:" << (int)len
//                              << endl;
            }
//            LG_FTXLEXI_D << "FourPack::FourPack() from stream size:" << overflow.size() << endl;
        }

        FourPack::FourPack( overflow_t::iterator iter, overflow_t::iterator ei )
        {
            for( int i=0; i < terms_per_fourpack && iter != ei; ++i, ++iter )
            {
                overflow.insert( make_pair( iter->first, iter->second ));
            }
//            LG_FTXLEXI_D << "FourPack::FourPack() from iterators size:" << overflow.size() << endl;
        }


        FourPack::FourPack( map< termid_t, string >::iterator iter,
                            map< termid_t, string >::iterator ei )
        {
            for( int i=0; i < terms_per_fourpack && iter != ei; ++i, ++iter )
            {
                overflow.insert( make_pair( iter->second, iter->first ));
            }
        }
        
        
        fh_fourpack
        FourPack::Create( overflow_t::iterator iter, overflow_t::iterator ei )
        {
            return new FourPack( iter, ei );
        }

        fh_fourpack
        FourPack::Create( map< termid_t, string >::iterator iter,
                          map< termid_t, string >::iterator ei )
        {
            return new FourPack( iter, ei );
        }
        
        
        termid_t
        FourPack::lookup( const std::string& term )
        {
            termid_t ret = 0;
            overflow_t::iterator iter = overflow.find( term );
            if( overflow.end() != iter )
            {
                ret = iter->second;
            }
            return ret;
        }

        const std::string&
        FourPack::lookup( termid_t tid )
        {
            static string nullstr = "";

            for( overflow_t::iterator iter = overflow.begin(); iter != overflow.end(); ++iter )
            {
                if( iter->second == tid )
                    return iter->first;
            }
            return nullstr;
        }
        
        string
        FourPack::firstTerm()
        {
            if( !overflow.empty() )
                return overflow.begin()->first;
            return "";
        }


        fh_fourpack
        FourPack::Read( fh_iostream& ss )
        {
            fh_fourpack ret = new FourPack( ss );
            return ret;
        }

        string
        FourPack::tostr( bool WriteInTidOrder )
        {
            int numberWritten = 1;
            if( overflow.empty() )
            {
                LG_FTXLEXI_W << "WARNING tostr() called on empty fourpack" << endl;
                return "";
            }
            if( overflow.size() > terms_per_fourpack )
            {
                LG_FTXLEXI_W << "WARNING tostr() called on oversized fourpack"
                             << " sz:" << overflow.size()
                             << endl;
            }

            typedef list< pair< string, termid_t > > orderedData_t;
            orderedData_t orderedData;

            //
            // We can store data sorted either by term (the default) or in TID
            // order. We thus have to build a temp orderedlist of data in the
            // order that it should be on disk.
            //
            if( WriteInTidOrder )
            {
                list< termid_t > sc = getSortedTids();
                for( list< termid_t >::iterator li = sc.begin(); li != sc.end(); ++li )
                {
                    termid_t tid  = *li;
                    string   term = "";
                    for( overflow_t::iterator x = overflow.begin();
                         x != overflow.end(); ++x )
                    {
                        if( x->second == tid )
                            term = x->first;
                    }
                    orderedData.push_back( make_pair( term, tid ));
                }
            }
            else
            {
                for( overflow_t::iterator x = overflow.begin();
                     x != overflow.end(); ++x )
                {
                    orderedData.push_back( make_pair( x->first, x->second ));
                }
            }

            orderedData_t::iterator x = orderedData.begin();

            string term = x->first;
            fh_stringstream ss;
            guint8 fulllen = term.length();
            writenum( ss,    fulllen  );
            writestring( ss, term );
            writenum( ss, x->second );
            string last = term;
            
            LG_FTXLEXI_D << "Fourpack::tostr() LEADING term:" << term
                         << " id:" << x->second
                         << " overflow.size():" << overflow.size()
                         << endl;
            
            for( ++x ; x != orderedData.end(); ++x )
            {
                string term       = x->first;
                string prefix     = common_prefix( last, term );
                guint8 prefixlen  = prefix.length();
                guint8 postfixlen = term.length() - prefix.length();

                LG_FTXLEXI_D << "Fourpack::tostr() term:" << term
                             << " prefix:" << prefix
                             << " prefixlen:" << (int)prefixlen
                             << " postfixlen:" << (int)postfixlen
                             << " id:" << x->second
                             << " overflow.size():" << overflow.size()
                             << endl;
                writenum( ss, prefixlen  );
                writenum( ss, postfixlen );
                writestring( ss, term.substr( prefix.length() ) );
                writenum( ss, x->second );

                last = term;
                ++numberWritten;
            }

            //
            // For the last pack on disk we have to pad out non filled entries
            // this is also needed to make sure that the entry fits into the
            // DiskBlock, otherwise we could read over the end of the block and
            // part of the index itself which would result in attempts to read
            // huge strings etc.
            //
            for( ; numberWritten < terms_per_fourpack; ++numberWritten )
            {
                guint8 len = 0;
                termid_t termid = 0;
                
                writenum( ss, len );
                writenum( ss, len );
                writenum( ss, termid );
            }
            
            ss << flush;
            return ::Ferris::tostr(ss);
        }
        
        streamsize
        FourPack::size( bool WriteInTidOrder )
        {
            return this->tostr( WriteInTidOrder ).length();
        }

        string
        FourPack::getDebugString()
        {
            fh_stringstream ss;
            ss << "FourPack::getDebugString(start) size:" << overflow.size() << endl;
            for( overflow_t::iterator x = overflow.begin(); x != overflow.end(); ++x )
            {
                string   term = x->first;
                termid_t tid  = x->second;
                ss << "  tid:" << tid << " term:" << term << endl;
            }
            ss << "FourPack::getDebugString(end) size:" << overflow.size() << endl;
            return ::Ferris::tostr(ss);
        }
        
        string
        FourPack::nextTerm( string s )
        {
            overflow_t::iterator iter = overflow.find( s );
            if( iter != overflow.end() )
            {
                ++iter;
                if( iter != overflow.end() )
                    return iter->first;
            }
            return "";
        }

        list< termid_t >
        FourPack::getSortedTids()
        {
            list< termid_t > ret;
            for( overflow_t::iterator x = overflow.begin(); x != overflow.end(); ++x )
            {
                string   term = x->first;
                termid_t tid  = x->second;
                ret.push_back( tid );
            }
            ret.sort();
            return ret;
        }
        
        
        termid_t
        FourPack::firstTid()
        {
            list< termid_t > sc = FourPack::getSortedTids();
            if( sc.empty() )
                return 0;
            return sc.front();
        }
        
        termid_t
        FourPack::nextTid( termid_t tid )
        {
            list< termid_t > sc = FourPack::getSortedTids();
            termid_t ret = 0;
            
            for( list< termid_t >::iterator iter = sc.begin(); iter != sc.end(); ++iter )
            {
                if( *iter == tid )
                {
                    ++iter;
                    if( iter != sc.end() )
                        ret = *iter;
                    return ret;
                }
            }
            return ret;
        }
        

        /********************************************************************************/
        /********************************************************************************/

        DiskBlock::DiskBlock( fh_fclex m_lex, blocknum_t blocknum, bool created )
            :
            m_blocknum( blocknum ),
            m_lex( m_lex ),
            m_blockUsedBytes( sizeof(blockoffset_t) ), // fixed block overhead
            m_readHeader( created ),
            m_readFourpacks( created )
        {
            if( !created )
                readHeader();
        }

        DiskBlock::DiskBlock( blocknum_t blocknum, bool created )
            :
            m_blocknum( blocknum ),
            m_lex( 0 ),
            m_blockUsedBytes( sizeof(blockoffset_t) ), // fixed block overhead
            m_readHeader( created ),
            m_readFourpacks( created )
        {
        }
        
        void
        DiskBlock::seekStartBlockG( fh_iostream& ss )
        {
            ss.clear();
            ss.seekg( (m_blocknum-1) * disk_block_size, ios::beg );
        }

        fh_iostream
        DiskBlock::getStream()
        {
            fh_iostream ioss = m_lex->getStream();
            seekStartBlockG( ioss );
            return ioss;
        }

        bool
        DiskBlock::getWriteInTidOrder()
        {
            return false;
        }
        
        
        void
        DiskBlock::readHeader()
        {
            if( m_readHeader )
                return;
            m_readHeader = true;
            
//             fh_iostream ioss = m_lex->getStream();
//             seekStartBlockG( ioss );
            fh_iostream ioss = getStream();
            
            fp_addrs.clear();

            blockoffset_t count = 0;
            readnum( ioss, count );
            LG_FTXLEXI_D << "DiskBlock::readHeader() reading blocknum:" << m_blocknum
                         << " starting header at (v+2):" << ioss.tellg()
                         << " have count:" << count
                         << " good:" << ioss.good()
                         << endl;

            for( blockoffset_t i = 0; i < count; ++i )
            {
                blockoffset_t n = 0;
                readnum( ioss, n );
                fp_addrs.push_back( n );
                LG_FTXLEXI_D << "DiskBlock::readHeader() reading blocknum:" << m_blocknum
                             << " offset:" << n << endl;
            }
        }

        void
        DiskBlock::skipHeader()
        {
//             fh_iostream ioss = m_lex->getStream();
//             seekStartBlockG( ioss );

            fh_iostream ioss = getStream();
            ioss.seekg( (fp_addrs.size()+1) * sizeof(blockoffset_t), ios::cur );
        }

        void
        DiskBlock::priv_insertFourpack( fh_fourpack fp )
        {
            m_fps.insert( make_pair( fp->firstTerm(), fp ) );
        }
        
        void
        DiskBlock::readAllFourPacks()
        {
            if( m_readFourpacks )
                return;
            m_readFourpacks = true;
            
            LG_FTXLEXI_D << "DiskBlock::readAllFourPacks() block:" << m_blocknum << endl;

            readHeader();

//             fh_iostream ioss = m_lex->getStream();
//             seekStartBlockG( ioss );
            fh_iostream ioss = getStream();
            
            LG_FTXLEXI_D << "DiskBlock::readAllFourPacks(2) block:" << m_blocknum
                         << " starting payload base at:" << ioss.tellg() << " using offsets from there "
                         << " have fp_addrs.size():" << fp_addrs.size()
                         << endl;
            
            for( fp_addrs_t::iterator iter = fp_addrs.begin(); iter!=fp_addrs.end(); ++iter )
            {
                seekStartBlockG( ioss );
                ioss->seekg( *iter, ios::cur );
                fh_fourpack fp = FourPack::Read( ioss );
                LG_FTXLEXI_D << "DiskBlock::readAllFourPacks(for) offset:" << *iter
                             << " tellg()" << ioss->tellg() << endl;
                priv_insertFourpack( fp );
            }

//             // PURE DEBUG
//             {
//                 cerr << "DiskBlock::readAllFourPacks() bn:" << m_blocknum
//                      << " m_fps.size:" << m_fps.size()
//                      << " after load dump..." << endl;
//                 for( m_fps_t::iterator iter = m_fps.begin(); iter != m_fps.end(); ++iter )
//                 {
//                     cerr << "+++first term:" << iter->first << endl;
//                 }
//             }
        }
        

        fh_block
        DiskBlock::Read( fh_fclex m_lex, blocknum_t n )
        {
            fh_block ret = new DiskBlock( m_lex, n, false );
            return ret;
        }

        
        fh_block
        DiskBlock::Create( fh_fclex m_lex, blocknum_t n )
        {
            fh_block ret = new DiskBlock( m_lex, n, true );
            return ret;
        }
        

        bool
        DiskBlock::append( fh_fourpack fp )
        {
            streamsize fpsz = fp->size() + sizeof(blockoffset_t);

            if( m_blockUsedBytes + fpsz > disk_block_size )
                return false;

            m_blockUsedBytes += fpsz;
            priv_insertFourpack( fp );
            return true;
        }

        void
        DiskBlock::Write( fh_iostream& ss )
        {
            // For writing the header we use RunningDataOffset to keep track of
            // where each fourpack should start relative to the start of this block
            blockoffset_t RunningDataOffset = (m_fps.size()+1) * sizeof(blockoffset_t);
            blockoffset_t count             = m_fps.size();

            LG_FTXLEXI_D << "DiskBlock::Write(top) block:" << m_blocknum
                         << " count:" << count
                         << " should be at offset:" << (m_blocknum-1) * disk_block_size
                         << " are at:" << ss.tellp()
                         << " getWriteInTidOrder:" << getWriteInTidOrder()
                         << endl;
            
            writenum( ss, count );

            typedef list< pair< string, fh_fourpack > > orderedData_t;
            orderedData_t orderedData;

            //
            // We can store data sorted either by term (the default) or in TID
            // order. We thus have to build a temp orderedlist of data in the
            // order that it should be on disk.
            //
            if( getWriteInTidOrder() )
            {
                typedef map< termid_t, fh_fourpack > TidOrdered_t;
                TidOrdered_t TidOrdered;

                //
                // First we copy the fourmap map into a map ordered by termID
                // then we copy that list in order into a list with term string -> fourpack
                //
                for( m_fps_t::iterator iter = m_fps.begin(); iter!=m_fps.end(); ++iter )
                {
                    TidOrdered.insert( make_pair( iter->second->firstTid(), iter->second ));
                }
                for( TidOrdered_t::iterator iter = TidOrdered.begin(); iter != TidOrdered.end(); ++iter )
                {
                    orderedData.push_back( make_pair( iter->second->firstTerm(), iter->second ));
                }

//                 cerr << ">>> DiskBlock::Write() this is the order of the data being written" << endl;
//                 for( orderedData_t::iterator iter = orderedData.begin(); iter != orderedData.end(); ++iter )
//                 {
//                     cerr << "term:" << iter->first << " fp:" << iter->second->getDebugString() << endl;
//                 }
//                 cerr << "<<< DiskBlock::Write() this is the order of the data being written" << endl;
            }
            else
            {
                for( m_fps_t::iterator iter = m_fps.begin(); iter!=m_fps.end(); ++iter )
                {
                    orderedData.push_back( make_pair( iter->first, iter->second ));
                }
            }

            for( orderedData_t::iterator iter = orderedData.begin(); iter != orderedData.end(); ++iter )
            {
                writenum( ss, RunningDataOffset );
                RunningDataOffset += iter->second->size( getWriteInTidOrder() );
            }
            for( orderedData_t::iterator iter = orderedData.begin(); iter != orderedData.end(); ++iter )
            {
                writestring( ss, iter->second->tostr( getWriteInTidOrder() ) );
            }

            // pad the block
            for( ; RunningDataOffset < disk_block_size; ++RunningDataOffset )
            {
                ss << '\0';
            }

            LG_FTXLEXI_D << "DiskBlock::Write(done) block:" << m_blocknum
                         << " count:" << count
                         << " should be at offset:" << (m_blocknum-1) * disk_block_size
                         << " are at:" << ss.tellp()
                         << endl;
        }

        struct check_term_header
            :
            public binary_function< blockoffset_t, blockoffset_t, bool >
        {
            fh_block m_block;
            
            check_term_header( fh_block m_block )
                :
                m_block( m_block )
                {
                }

            string getLeadingFourPackTerm( blockoffset_t offset )
                {
                    fh_iostream ioss = m_block->getStream();
                    ioss.seekg( offset, ios::cur );
                    
                    guint8 len = 0;
                    string fpterm;
                    readnum( ioss, len );
                    fpterm = readstring( ioss, len );
                    return fpterm;
                }

            bool operator()( blockoffset_t x, blockoffset_t y )
                {
                    return getLeadingFourPackTerm( x ) < getLeadingFourPackTerm( y );
                }

            bool operator()( blockoffset_t x, const std::string& y )
                {
                    return getLeadingFourPackTerm( x ) < y;
                }

            bool operator()( const std::string& x, blockoffset_t y )
                {
                    return x < getLeadingFourPackTerm( y );
                }
        };

        struct reverse_check_term_header
            :
            public binary_function< blockoffset_t, blockoffset_t, bool >
        {
            fh_block m_block;
            
            reverse_check_term_header( fh_block m_block )
                :
                m_block( m_block )
                {
                }

            termid_t getLeadingFourPackTid( blockoffset_t offset )
                {
                    fh_iostream ioss = m_block->getStream();
                    ioss.seekg( offset, ios::cur );
                    
                    guint8 len = 0;
                    readnum( ioss, len );
                    string fpterm = readstring( ioss, len );
                    termid_t termid;
                    readnum( ioss, termid );
                    return termid;
                }

            bool operator()( blockoffset_t x, blockoffset_t y )
                {
                    return getLeadingFourPackTid( x ) < getLeadingFourPackTid( y );
                }

            bool operator()( blockoffset_t x, termid_t y )
                {
                    return getLeadingFourPackTid( x ) < y;
                }

            bool operator()( termid_t x, blockoffset_t y )
                {
                    return x < getLeadingFourPackTid( y );
                }
        };
        
        
        termid_t
        DiskBlock::lookup( const std::string& term )
        {
            termid_t ret = 0;
            readHeader();
            skipHeader();

            //
            // Try to find the fourpack needed within the unloaded block. Do this by
            // performing a binary search on the leading strings in each fourpack
            // using the offsets of the start of each fourpack
            //
            fp_addrs_t::iterator iter = upper_bound( fp_addrs.begin(), fp_addrs.end(),
                                                    term, check_term_header( this ) );
            --iter;
            blockoffset_t blockOffset = *iter;

            fh_iostream ioss = getStream();
            ioss.seekg( blockOffset, ios::cur );
            fh_fourpack fp = FourPack::Read( ioss );
            return fp->lookup( term );
            
            

            
 //            //
//             // Try to find the fourpack needed within the unloaded block. Do this by
//             // performing a binary search on the leading strings in each fourpack
//             // using the offsets of the start of each fourpack
//             //
//             // seek: mustard
//             // in:
//             //   alice
//             //   hatter
//             //   wonderland
//             //
//             if( !fp_addrs.empty() && !m_readFourpacks )
//             {
//                 cerr << "DiskBlock::lookup() trying short cut loading of fourpack, term:" << term << endl;
                
//                 fp_addrs_t::iterator iter = fp_addrs.begin();
//                 int fp_addrs_size = fp_addrs.size();
//                 int remainingsz = fp_addrs_size % 2 + fp_addrs_size / 2;
//                 advance( iter, remainingsz );
//                 fh_iostream ioss = m_lex->getStream();

//                 blockoffset_t blockOffset = *iter;
//                 int loopCountDown = 2;

//                 while( loopCountDown )
//                 {
//                     blockOffset = *iter;
                    
//                     seekStartBlockG( ioss );
//                     ioss.seekg( blockOffset, ios::cur );
                    
//                     guint8 len = 0;
//                     string fpterm;
//                     readnum( ioss, len );
//                     fpterm = readstring( ioss, len );

//                     int direction = fpterm < term ? 1 : -1;

//                     cerr << "testing fourpack offset:" << blockOffset
//                          << " for term:" << term
//                          << " leading fourpack term is:" << fpterm
//                          << " fpterm < term:" << (fpterm < term)
//                          << " dir:" << direction
//                          << " remainingsz:" << remainingsz
//                          << " remainingsz/2:" << (remainingsz % 2 + remainingsz / 2)
//                          << " total.sz:" << fp_addrs.size()
//                          << endl;

//                     if( remainingsz==1 )
//                         --loopCountDown;

//                     remainingsz = remainingsz % 2 + remainingsz / 2;
//                     fp_addrs_t::iterator next = iter;
//                     advance( next, direction * remainingsz );
//                     if( iter == next )
//                         break;
//                     iter = next;
//                 }

//                 //
//                 // read just the desired fourpack
//                 //
//                 seekStartBlockG( ioss );
//                 ioss.seekg( blockOffset, ios::cur );
//                 fh_fourpack fp = FourPack::Read( ioss );
//                 return fp->lookup( term );
//             }
            
            readAllFourPacks();

            for( m_fps_t::iterator iter = m_fps.begin(); iter!=m_fps.end(); ++iter )
            {
                if( ret = iter->second->lookup( term ))
                    return ret;
            }
            return ret;
        }
        

        string
        DiskBlock::firstTerm()
        {
            readAllFourPacks();

            if( m_fps.empty() )
            {
                LG_FTXLEXI_D << "DiskBlock::firstTerm() WITH NO FIRST TERM!"
                             << " there are no fourpacks!"
                             << " m_readFourpacks:" << m_readFourpacks
                             << endl;
                return "";
            }
            string ret = m_fps.begin()->second->firstTerm();
            if( !ret.empty() )
                return ret;

            LG_FTXLEXI_D << "DiskBlock::firstTerm() WITH NO FIRST TERM!"
                         << " size:" << m_fps.size()
                         << endl;

            m_fps_t::iterator iter = m_fps.begin();
            for( ; iter != m_fps.end(); ++iter )
            {
                LG_FTXLEXI_D << " fourpack:" << iter->first << " str:" << iter->second->getDebugString() << endl;
            }
            return "";
        }

        string
        DiskBlock::next( string s )
        {
            readAllFourPacks();

            m_fps_t::iterator iter = m_fps.begin();
            while( iter != m_fps.end() )
            {
                fh_fourpack fp = iter->second;
                termid_t id = fp->lookup( s );

                LG_FTXLEXI_D << "DiskBlock::next() m_blocknum:" << m_blocknum
                             << " lookup for s:" << s << " in fp:"
                             << iter->first << " got tid:" << id
                             << endl;
                
                if( id )
                {
                    string tmp = fp->firstTerm();
                    while( tmp != s && tmp != "" )
                        tmp = fp->nextTerm( tmp );
                    tmp = fp->nextTerm( tmp );

                    LG_FTXLEXI_D << "DiskBlock::next(2) m_blocknum:" << m_blocknum
                                 << " finding next term string for:" << id << " tmp:" << tmp << endl;
                    if( tmp == "" )
                    {
                        ++iter;
                        if( iter == m_fps.end() )
                        {
                            LG_FTXLEXI_D << "DiskBlock::next(not in block)"
                                         << " m_blocknum:" << m_blocknum
                                         << " id:" << id
                                         << endl;
                            return "";
                        }
                        fp = iter->second;
                        tmp = fp->firstTerm();
                    }
                    return tmp;
                }
                ++iter;
            }
            return "";
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        fh_revblock
        DiskBlock_Reverse::Read( fh_revfclex lex, blocknum_t n )
        {
            fh_revblock ret = new DiskBlock_Reverse( lex, n, false );
            return ret;
        }

        fh_revblock
        DiskBlock_Reverse::Create( fh_revfclex lex, blocknum_t n )
        {
            fh_revblock ret = new DiskBlock_Reverse( lex, n, true );
            return ret;
        }
        
        DiskBlock_Reverse::DiskBlock_Reverse( fh_revfclex lex, blocknum_t blocknum, bool created )
            :
            DiskBlock( blocknum, created ),
            m_lex( lex )
        {
            if( !created )
                readHeader();
        }
            
        void
        DiskBlock_Reverse::priv_insertFourpack( fh_fourpack fp )
        {
            m_revfps.insert( make_pair( fp->firstTid(), fp ) );
            _Base::priv_insertFourpack( fp );
        }

        
        std::string
        DiskBlock_Reverse::lookup( termid_t tid )
        {
            readHeader();
            skipHeader();

            //
            // Try to find the fourpack needed within the unloaded block. Do this by
            // performing a binary search on the leading strings in each fourpack
            // using the offsets of the start of each fourpack
            //
            fp_addrs_t::iterator iter = upper_bound( fp_addrs.begin(), fp_addrs.end(),
                                                     tid, reverse_check_term_header( this ) );
            --iter;
            blockoffset_t blockOffset = *iter;

            fh_iostream ioss = getStream();
            ioss.seekg( blockOffset, ios::cur );
            fh_fourpack fp = FourPack::Read( ioss );
            string term = fp->lookup( tid );
//             if( term.empty() )
//             {
//                 cerr << "DiskBlock_Reverse::lookup(failed) tid:" << tid
//                      << " blockOffset:" << blockOffset
//                      << " fp:" << fp->getDebugString()
//                      << endl;
//                 readAllFourPacks();
//                 cerr << "lookup(empty) tid:" << tid << " FULL LEXICON DUMP --- BEGIN " << endl;
//                 for( m_revfps_t::iterator iter = m_revfps.begin(); iter != m_revfps.end(); ++iter )
//                 {
//                     cerr << "revfps tid:" << iter->first << endl;
//                     cerr << iter->second->getDebugString();
//                 }
//                 cerr << "FULL LEXICON DUMP --- END " << endl;
                
//             }
            return term;

            
//             readHeader();
//             skipHeader();

//             // FIXME: should try to get the upper_bound() peeking method working for
//             // reverse lexicons
//             readAllFourPacks();
//             m_revfps_t::iterator iter = m_revfps.upper_bound( tid );
//             if( iter != m_revfps.begin() )
//                 --iter;

//             cerr << "lookup() tid:" << tid
//                  << " checking in fourpack starting on tid:" << iter->first
//                  << endl;
            
//             string ret = iter->second->lookup( tid );
//             if( ret.empty() )
//             {
//                 cerr << "lookup(empty) tid:" << tid << " FULL LEXICON DUMP --- BEGIN " << endl;
//                 for( m_revfps_t::iterator iter = m_revfps.begin(); iter != m_revfps.end(); ++iter )
//                 {
//                     cerr << "revfps tid:" << iter->first << endl;
//                     cerr << iter->second->getDebugString();
//                 }
//                 cerr << "FULL LEXICON DUMP --- END " << endl;
//             }
//             cerr << "diskblock_reverse::lookup() tid:" << tid
//                  << " gives term:" << ret
//                  << endl;
            
//             return ret;
        }
            
        termid_t
        DiskBlock_Reverse::firstTid()
        {
            readAllFourPacks();

            if( m_revfps.empty() )
            {
                LG_FTXLEXI_D << "DiskBlock::firstTerm() WITH NO FIRST TERM!"
                             << " there are no fourpacks!"
                             << " m_readFourpacks:" << m_readFourpacks
                             << endl;
                return 0;
            }

            m_walkIter   = m_revfps.begin();
            termid_t ret = m_walkIter->second->firstTid();
            return ret;
        }
        
        termid_t
        DiskBlock_Reverse::next( termid_t tid )
        {
            fh_fourpack fp = m_walkIter->second;

            //
            // check the current fourpack for the tid
            //
            for( termid_t tmp = fp->firstTid(); tmp; tmp = fp->nextTid( tmp ) )
            {
                if( tmp == tid )
                {
                    tmp = fp->nextTid( tmp );
                    if( tmp )
                        return tmp;
                    break;
                }
            }

            //
            // check next fourpack
            //
            ++m_walkIter;
            if( m_walkIter == m_revfps.end() )
                return 0;
            return m_walkIter->second->firstTid();
        }

        
        fh_iostream
        DiskBlock_Reverse::getStream()
        {
            fh_iostream ioss = m_lex->getStream();
            seekStartBlockG( ioss );
            return ioss;
        }

        bool
        DiskBlock_Reverse::getWriteInTidOrder()
        {
            return true;
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        
        Lexicon_FrontCodedBlocks::Lexicon_FrontCodedBlocks( fh_idx idx, PathManager* path_mgr )
            :
            Lexicon( idx, path_mgr ),
            m_getBlock_cache( 0 )
        {
            setFileName( "/lexicon.prefixcoded" );
        }

        void
        Lexicon_FrontCodedBlocks::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );

            string filename = m_path_mgr->getBasePath() + "/" + getFileName();

            Shell::touch( filename );
            m_stream.open( filename, ios::in | ios::out );
            blockindex_read();
        }
        
        Lexicon_FrontCodedBlocks::~Lexicon_FrontCodedBlocks()
        {
            if( !overflow.empty() )
                sync();
        }

        fh_block
        Lexicon_FrontCodedBlocks::getBlock( blocknum_t b )
        {
            if( m_getBlock_cache && m_getBlock_cache->getBlockNumber() == b )
                return m_getBlock_cache;

            m_getBlock_cache = DiskBlock::Read( this, b );
            return m_getBlock_cache;
        }

        //
        // read all terms into overflow map<>
        //
        void
        Lexicon_FrontCodedBlocks::prepareForInsertions()
        {
            m_stream.clear();
            string term = getFirstTerm();
            while( !term.empty() )
            {
                termid_t id = lookup( term );
                overflow.insert( make_pair( term, id ));
                term = getNextTerm( term );
            }
        }
        

        void
        Lexicon_FrontCodedBlocks::insert( const std::string& term, termid_t termid )
        {
            overflow.insert( make_pair( term, termid ));
        }

        termid_t
        Lexicon_FrontCodedBlocks::lookup( const std::string& term )
        {
            termid_t ret = 0;

            //
            // check overflow first.
            // Reason: there should only be an overflow if we are adding terms,
            //         in which case checking the in core overflow first may
            //         save a lot of time.
            //
            if( !overflow.empty() )
            {
                overflow_t::iterator iter = overflow.find( term );
                if( overflow.end() != iter )
                {
                    ret = iter->second;
                    return ret;
                }
            }

            
            //
            // lookup term in prefix blocks
            //
            blockindex_t::iterator bi = blockindex.lower_bound( term );
            if( bi == blockindex.end() )
            {
                // make sure that the last block is not a possible match
                LG_FTXLEXI_D << "lower_bound() for term:" << term << " is eof"
                             << " block index is empty:" << blockindex.empty()
                             << endl;
                if( !blockindex.empty() )
                {
                    blockindex_t::iterator tmp = blockindex.end();
                    --tmp;
                    bi = tmp;
                }
            }
            else
            {
                // Unless we hit on the term itself, lower_bound() will return an iterator
                // to the block after the one we desire.
                bool decrementIterator = (bi != blockindex.begin());
                if( bi != blockindex.end() )
                {
                    LG_FTXLEXI_D << "lower_bound() term:" << term
                                 << " decrementIterator:" << decrementIterator
                                 << " bi->first:" << bi->first
                                 << endl;
                    
                    if( bi->first == term )
                        decrementIterator = false;
                }

                LG_FTXLEXI_D << "lower_bound() term:" << term
                             << " decrementIterator:" << decrementIterator
                             << endl;
                
                if( decrementIterator )
                {
                    --bi;
                }
            }

            blocknum_t blockNum = (bi==blockindex.end()) ? 0 : bi->second;
            LG_FTXLEXI_D << "checking in block:" << blockNum
                      << " for term:" << term
                      << " bi==end:" << (bi==blockindex.end())
                      << endl;

            if( !blockNum )
            {
                LG_FTXLEXI_W << "Cant find term:" << term << " in lexicon!" << endl;
                return 0;
            }
            fh_block b = getBlock( blockNum );
            ret = b->lookup( term );

            return ret;
        }


        

        //
        // consolidate overflow with prefix blocks into new file
        //
        void
        Lexicon_FrontCodedBlocks::sync()
        {
            //
            // don't go doing this if we dont have to.
            //
            if( overflow.empty() )
            {
                return;
            }
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(top)" << endl;
            
            //
            // read all terms into overflow map<>
            //
            prepareForInsertions();
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(2 read all terms off disk)" << endl;

            // low level debugging
            {
                LG_FTXLEXI_D << "FULL LEXICON DUMP --- BEGIN " << endl;
                for( overflow_t::iterator i = overflow.begin(); i!=overflow.end(); ++i )
                {
                    LG_FTXLEXI_D << "term:" << i->first << " id:" << i->second << endl;
                }
                LG_FTXLEXI_D << "FULL LEXICON DUMP --- END " << endl;
            }
            
            
            //
            // start writing out the overflow terms in 4 term blocks
            // we recreate the blockindex as we go
            //
            m_stream.close();
//             Util::BackupMaker bmaker;
//             try
//             {
//                 cerr << "3-in-4 making backup. "
//                      << " m_path_mgr:" << toVoid( m_path_mgr )
//                      << " basePath:" << m_path_mgr->getBasePath()
//                      << " filename:" << getFileName()
//                      << " param:" << (m_path_mgr->getBasePath() + "/" + getFileName())
//                      << endl;
//                 bmaker.perform( m_path_mgr->getBasePath() + "/" + getFileName() );
//             }
//             catch( NoSuchSubContext& e )
//             {
//                 // nothing to backup
//             }
            

            m_stream.open( m_path_mgr->getBasePath() + "/" + getFileName(), ios::out | ios::trunc );
            blockindex.clear();
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(reopened file)" << endl;

            blocknum_t blockNumber = 1; // current block
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(starting to write blocks)" << endl;

            for( overflow_t::iterator iter = overflow.begin(); iter != overflow.end() ;  )
            {
                fh_block block = DiskBlock::Create( this, blockNumber );

                //
                // stuff fourpacks into the current block while we can
                //
                // last is used to contruct a new fourpack from the current iter to last
                // inclusive. If the fourpack doesn't fit then iter is not moved so that
                // the same fourpack will be constructed next time for insertion into
                // the next block. If the fourpack fits then iter is moved to the next
                // item after the fourpack
                //
                while( iter != overflow.end() )
                {
                    overflow_t::iterator last = iter;
                    for( int tmp = 0; last != overflow.end() && tmp < terms_per_fourpack; ++tmp )
                        ++last;
                    
                    fh_fourpack fp = FourPack::Create( iter, last );
                    LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(testing fourpack fits)"
                                 << " blockNum:" << blockNumber
                                 << " fp:" << fp->getDebugString()
                                 << endl;
                    
                    if( !block->append( fp ) )
                    {
                        LG_FTXLEXI_D << "block->append failed on term:" << iter->first << endl;
                        break;
                    }
                    iter = last;
//                     if( iter != overflow.end() )
//                         ++iter;
                }

                LG_FTXLEXI_D << "Adding to block index term:" << block->firstTerm()
                          << " block number:" << blockNumber
                          << endl;
                blockindex.insert( make_pair( block->firstTerm(), blockNumber ));
                
                // move to next new block
                block->Write( m_stream );
                ++blockNumber;
            }
            
//             typedef list<fh_fourpack> fplist_t;
//             fplist_t fplist; // list of fourpacks that will be saved into this block
//             fh_fourpack fp;  // current fourpack we are going to write
//             fh_fourpack overflow_four = 0; // overflow from last disk_block
//             bool atEndOfTerms = false; // if we hit the end we can signal it
//             blocknum_t blockNumber = 1; // current block
                
//             for( overflow_t::iterator iter = overflow.begin();
//                  iter != overflow.end() && !atEndOfTerms ;  )
//             {
//                 //
//                 // prepare for the next block
//                 //
//                 streamsize blockBytes = sizeof(blockoffset_t); // fixed block overhead
//                 fplist.clear();
                
//                 while( blockBytes < disk_block_size )
//                 {
                    
//                     // get next fourpack and calc its size
//                     if( overflow_four )
//                     {
//                         fp = overflow_four;
//                         overflow_four = 0;
//                     }
//                     else
//                     {
//                         if( iter == overflow.end() )
//                         {
//                             atEndOfTerms = true;
//                             fp = 0;
//                         }
//                         else
//                         {
//                             fp = FourPack::Create( iter, overflow.end() );
//                         }
//                     }

//                     streamsize fpsz = 0;
//                     if( fp )
//                         fpsz = fp->size();

//                     // Can we fit this fourpack of data?
//                     if( atEndOfTerms
//                         || blockBytes + fpsz + sizeof(blockoffset_t) > disk_block_size )
//                     {
//                         //
//                         // fp itself won't fit, so we overflow it into the next disk_block
//                         //
//                         overflow_four = fp;

//                         //
//                         // create and write this block
//                         //
//                         blockindex.insert( make_pair( (*fplist.begin())->firstTerm(),
//                                                       blockNumber++ ));
//                         streamsize offset = fplist.size()+1 * sizeof(blockoffset_t);
//                         m_stream << (char)fplist.size();
//                         for( fplist_t::iterator i = fplist.begin(); i!=fplist.end(); ++i )
//                         {
//                             m_stream << offset;
//                             offset += (*i)->size();
//                         }
//                         for( fplist_t::iterator i = fplist.begin(); i!=fplist.end(); ++i )
//                         {
//                             m_stream << (*i)->tostr();
//                         }
//                         break;
//                     }
//                     else
//                     {
//                         fplist.push_back( fp );
//                     }
//                 }
//             }
            
            
            //
            // write the index at the end of file
            //
            blockindex_write();
            
            //
            // cleanup
            //
            m_stream << flush;
        }

        std::string
        Lexicon_FrontCodedBlocks::getFirstTerm()
        {
            if( blockindex.empty() )
                return "";
            
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getFirstTerm() ret:"
                         << blockindex.begin()->first << endl;
            return blockindex.begin()->first;
        }

        
        std::string
        Lexicon_FrontCodedBlocks::getNextTerm( const std::string& s )
        {
            string ret = "";
            
            blockindex_t::iterator bi = blockindex.lower_bound( s );
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getNextTerm() s:" << s
                         << " bi==end:" << ( bi == blockindex.end() )
                         << endl;
            
            if( bi != blockindex.end() )
            {
                LG_FTXLEXI_D << "getNextTerm() s:" << s << " bi->first:" << bi->first
                             << " bi->first > s:" << (bi->first > s)
                             << endl;

                // lower_bound will probably have found the next block to the one we want
                if( bi->first > s )
                    --bi;
            }

            if( bi != blockindex.end() && bi->second )
            {
                blocknum_t blockNum = bi->second;

                LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getNextTerm() s:" << s
                             << " block#:" << blockNum
                             << " blockindex.size():" << blockindex.size()
                             << endl;
                
                fh_block block = getBlock( blockNum );
                ret = block->next( s );
//                if( ret.empty() && blockNum <= blockindex.size() )
                if( ret.empty() && blockNum < blockindex.size() )
                {
                    LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getNextTerm() s:" << s
                                 << " block#:" << blockNum
                                 << " ret.empty() "
                                 << endl;

                    block = getBlock(  ++blockNum );
                    ret = block->firstTerm();

                    LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getNextTerm() s:" << s
                                 << " block#:" << blockNum
                                 << " first term is:" << ret
                                 << endl;
                }
            }
            else
            {
                LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::getNextTerm() s:" << s
                             << " bi==end " << endl;
                // if bi==end then lower_bound() is still allowing it to maybe
                // be in the last block. We should test against the last string
                // in the lexicon to see if it exists.
                
                fh_block block = getBlock( blockindex.size() );
                blockindex_t::iterator last = blockindex.end();
//                 --last;

//                 // If we are just starting on the last block then do so
//                 if( s == last->first )
//                     ret = block->firstTerm();

                // let the block handle the end of block string.
                ret = block->next( s );
            }
            return ret;
        }
        
        void
        Lexicon_FrontCodedBlocks::blockindex_read()
        {
            m_stream.clear();
            blockindex.clear();
            
            string filename = m_path_mgr->getBasePath() + "/" + getFileName();
            if( toint( getStrAttr( Resolve(filename), "size", "0" )) == 0 )
            {
                LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() zero length file" << endl;
                return;
            }
            
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() top" << endl;
            guint32 block_index_size = 0;
            int versionAndLenghtOffset = sizeof( tifl_version_t ) + sizeof( guint32 );
            m_stream.clear();
            m_stream.seekg( -1 * versionAndLenghtOffset , ios::end );
            if( !m_stream.good() )
                return;
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() seeked to:"
                      << m_stream.tellg()
                      << " is_open():" << m_stream.is_open()
                      << endl;

            readnum( m_stream, block_index_size );
            tifl_version_t fileversion = TIFL_VERSION_BAD;
            readnum( m_stream, fileversion );
            if( fileversion != TIFL_VERSION_1 )
            {
                fh_stringstream ss;
                ss << "Lexicon_FrontCodedBlocks::blockindex_read() "
                   << " unknown file version for on disk lexicon!"
                   << " fileversion:" << fileversion
                   << " path:" << filename
                   << " size:" << getStrAttr( Resolve(filename), "size", "0" )
                   << endl;
                Throw_FullTextIndexException( tostr(ss), 0 );
            }
            
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() block_index_size:" << block_index_size << endl;
            LG_FTXLEXI_D << " good:" << m_stream.good()
                      << " eof:" << m_stream.eof()
                      << " fail:" << m_stream.fail()
                      << endl;
            m_stream.clear();
            streamsize offset = -1 * versionAndLenghtOffset;
            offset -= block_index_size;
            m_stream.seekg( offset, ios::end );
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() seeked2: "
                      << " eofminus:" << offset
                      << " tellg():" << m_stream.tellg()
                      << " good:" << m_stream.good()
                      << " eof:" << m_stream.eof()
                      << " fail:" << m_stream.fail()
                      << endl;

            //
            // assertion: we should be at a block boundry now
            //
            if( m_stream.tellg() % disk_block_size != 0 )
            {
                LG_FTXLEXI_W << "Index into lexicon is corrupt. Cant lookup start of index string table from eof" << endl;
            }
            
            guint32 number_of_entries = 0;
            readnum( m_stream, number_of_entries );
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() number_of_entries:" << number_of_entries << endl;
            
            for( int block_number=1; block_number <= number_of_entries; ++block_number )
            {
                guint8 len = 0;
                readnum( m_stream, len );
                const string firststr = readstring( m_stream, len );
                LG_FTXLEXI_D << "adding to block index term:" << firststr << " at block:" << block_number << endl;
                blockindex.insert( make_pair( firststr, block_number ));
            }
        }
        
        void
        Lexicon_FrontCodedBlocks::blockindex_write()
        {
            m_stream.clear();
            guint32 block_index_size = 0;
            guint32 number_of_entries = blockindex.size();
            
            writenum( m_stream, number_of_entries );
            block_index_size += sizeof(number_of_entries);
            
            for( blockindex_t::iterator iter = blockindex.begin();
                 iter != blockindex.end(); ++iter )
            {
                string     firststr     = iter->first;
                blocknum_t block_number = iter->second;

                guint8 len = firststr.length();
                block_index_size += 1 + len;
                writenum( m_stream, len );
                writestring( m_stream, firststr );
                LG_FTXLEXI_D << "blockindex_write() len:" << (int)len << " term:" << firststr << endl;
            }
            writenum( m_stream, block_index_size );
            tifl_version_t fileversion = TIFL_VERSION_1;
            writenum( m_stream, fileversion );
        }

        fh_iostream
        Lexicon_FrontCodedBlocks::getStream()
        {
            return m_stream;
        }
        
        /****************************************/
        /****************************************/
        /****************************************/
        /****************************************/
        /****************************************/
        /****************************************/

        ReverseLexicon_FrontCodedBlocks::ReverseLexicon_FrontCodedBlocks()
            :
            m_getBlock_cache( 0 )
        {
        }
        
        ReverseLexicon_FrontCodedBlocks::~ReverseLexicon_FrontCodedBlocks()
        {
            sync();
        }

        string
        ReverseLexicon_FrontCodedBlocks::getStreamFileName()
        {
            string filename = m_path_mgr->getBasePath() + "/" + getFileName();
            return filename;
        }
        
        
        fh_fstream
        ReverseLexicon_FrontCodedBlocks::getStream()
        {
            string filename = getStreamFileName();
            
            Shell::touch( filename );
            m_stream.close();
            m_stream.open( filename, ios::in | ios::out );
            return m_stream;
        }


        fh_fstream
        ReverseLexicon_FrontCodedBlocks::getBlockIndexStream()
        {
            string filename = getStreamFileName() + ".blockindex";
            Shell::touch( filename );
            fh_fstream ret;
            ret.open( filename, ios::in | ios::out );
            return ret;
        }
        
        void
        ReverseLexicon_FrontCodedBlocks::blockindex_read()
        {
            fh_istream iss = getBlockIndexStream();

            string filename = getStreamFileName();
            if( toint( getStrAttr( Resolve(filename), "size", "0" )) == 0 )
            {
                LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::blockindex_read() zero length file" << endl;
                return;
            }
            

            tifl_version_t fileversion = TIFL_VERSION_BAD;
            readnum( iss, fileversion );
            if( fileversion != TIFL_VERSION_1 )
            {
                fh_stringstream ss;
                ss << "ReverseLexicon_FrontCodedBlocks::blockindex_read() "
                   << " unknown file version for on disk lexicon!"
                   << " fileversion:" << fileversion
                   << endl;
                Throw_FullTextIndexException( tostr(ss), 0 );
            }
            
            guint32 number_of_entries = 0;
            readnum( iss, number_of_entries );
            LG_FTXLEXI_D << "RevLexicon::sec_read() number_of_entries:" << number_of_entries << endl;
            
            for( int block_number=1; block_number <= number_of_entries; ++block_number )
            {
                termid_t tid  = 0;
                readnum( iss, tid );
                LG_FTXLEXI_D << "adding to block index tid:" << tid << " at block:" << block_number << endl;
                blockindex.insert( make_pair( tid, block_number ));
            }
        }
        
        void
        ReverseLexicon_FrontCodedBlocks::blockindex_write()
        {
            fh_iostream ioss = getBlockIndexStream();
            ioss.clear();
            guint32 block_index_size = 0;
            guint32 number_of_entries = blockindex.size();

            tifl_version_t fileversion = TIFL_VERSION_1;
            writenum( ioss, fileversion );
            writenum( ioss, number_of_entries );
            block_index_size += sizeof(number_of_entries);
            
            for( blockindex_t::iterator iter = blockindex.begin();
                 iter != blockindex.end(); ++iter )
            {
                termid_t   tid          = iter->first;
                blocknum_t block_number = iter->second;

                block_index_size += sizeof(tid);
                writenum( ioss, tid );
            }
        }

        void
        ReverseLexicon_FrontCodedBlocks::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );

            string filename = m_path_mgr->getBasePath() + "/" + getFileName();
            Shell::touch( filename );
            m_stream.open( filename, ios::in | ios::out );
            blockindex_read();
        }


        fh_revblock
        ReverseLexicon_FrontCodedBlocks::getBlock( blocknum_t b )
        {
            if( m_getBlock_cache && m_getBlock_cache->getBlockNumber() == b )
                return m_getBlock_cache;

            m_getBlock_cache = DiskBlock_Reverse::Read( this, b );
            return m_getBlock_cache;
        }
        
        void
        ReverseLexicon_FrontCodedBlocks::insert( const std::string& s, termid_t id )
        {
            overflow.insert( make_pair( id, s ));
        }
        
        bool
        ReverseLexicon_FrontCodedBlocks::exists( termid_t id )
        {
            return lookup( id ) != "";
        }
        
        termid_t
        ReverseLexicon_FrontCodedBlocks::getFirstTerm()
        {
            prepareForInsertions();

            if( overflow.empty() )
                return 0;

            LG_FTXLEXI_D << "ReverseLexicon_FrontCodedBlocks::getFirstTerm()"
                         << " overflow.size:" << overflow.size() << endl;
            
            m_walkIter = overflow.begin();
            return m_walkIter->first;
        }

        
        
        termid_t
        ReverseLexicon_FrontCodedBlocks::getNextTerm( termid_t tid )
        {
            ++m_walkIter;
            if( m_walkIter == overflow.end() )
                return 0;
            return m_walkIter->first;
        }
        

        void
        ReverseLexicon_FrontCodedBlocks::sync()
        {
            //
            // don't go doing this if we dont have to.
            //
            if( overflow.empty() )
            {
                return;
            }
            LG_FTXLEXI_D << "ReverseLexicon_FrontCodedBlocks::sync(top)"
                         << " overflow.size:" << overflow.size() << endl;

            //
            // read all terms into overflow map<>
            //
            prepareForInsertions();
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(2 read all terms off disk)" << endl;

//             // low level debugging
//             {
//                 cerr << "FULL LEXICON DUMP --- BEGIN " << endl;
//                 for( overflow_t::iterator i = overflow.begin(); i!=overflow.end(); ++i )
//                 {
//                     cerr << "term:" << i->first << " id:" << i->second << endl;
//                 }
//                 cerr << "FULL LEXICON DUMP --- END " << endl;
//             }
            
            //
            // start writing out the overflow terms in 4 term blocks
            // we recreate the blockindex as we go
            //
            m_stream.close();
            Util::BackupMaker bmaker;
            bmaker.perform( m_path_mgr->getBasePath() + "/" + getFileName() );

            m_stream.open( getStreamFileName(), ios::out | ios::trunc );
            blockindex.clear();
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(reopened file)" << endl;

            blocknum_t blockNumber = 1; // current block
            LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(starting to write blocks)" << endl;

            for( overflow_t::iterator iter = overflow.begin(); iter != overflow.end(); )
            {
                fh_revblock block = DiskBlock_Reverse::Create( this, blockNumber );

                //
                // stuff fourpacks into the current block while we can
                //
                // last is used to contruct a new fourpack from the current iter to last
                // inclusive. If the fourpack doesn't fit then iter is not moved so that
                // the same fourpack will be constructed next time for insertion into
                // the next block. If the fourpack fits then iter is moved to the next
                // item after the fourpack
                //
                while( iter != overflow.end() )
                {
                    overflow_t::iterator last = iter;
                    for( int tmp = 0; last != overflow.end() && tmp < terms_per_fourpack; ++tmp )
                        ++last;
                    
                    fh_fourpack fp = FourPack::Create( iter, last );
                    LG_FTXLEXI_D << "Lexicon_FrontCodedBlocks::sync(testing fourpack fits)"
                                 << " blockNum:" << blockNumber
                                 << " fp:" << fp->getDebugString()
                                 << endl;
                    
                    if( !block->append( fp ) )
                    {
                        LG_FTXLEXI_D << "block->append failed on term:" << iter->first << endl;
                        break;
                    }
                    iter = last;
                }

                LG_FTXLEXI_D << "Adding to block index term:" << block->firstTerm()
                             << " block number:" << blockNumber
                             << endl;
                blockindex.insert( make_pair( block->firstTid(), blockNumber ));
                
                // move to next new block
                block->Write( m_stream );
                ++blockNumber;
            }

            LG_FTXLEXI_D << "ReverseLexicon_FrontCodedBlocks::sync() done writing the blocks. " << endl;
            
            
            //
            // write the index at the end of file
            //
            blockindex_write();
            
            //
            // cleanup
            //
            m_stream << flush;
        }

        void
        ReverseLexicon_FrontCodedBlocks::prepareForInsertions()
        {
            for( blockindex_t::iterator bi = blockindex.begin(); bi != blockindex.end(); ++bi )
            {
                fh_revblock b = getBlock( bi->second );

                for( termid_t tid = b->firstTid(); tid; tid = b->next( tid ) )
                {
                    string term = b->lookup( tid );
//                     cerr << "ReverseLexicon_FrontCodedBlocks::prepareForInsertions()"
//                          << " tid:" << tid << " term:" << term << endl;
                    overflow.insert( make_pair( tid, term ) );
                }
            }

            LG_FTXLEXI_D << "ReverseLexicon_FrontCodedBlocks::prepareForInsertions(end)"
                         << " overflow.sz:" << overflow.size() << endl;
            
        }
        
        
        string
        ReverseLexicon_FrontCodedBlocks::lookup( termid_t tid )
        {
            string ret = "";

            //
            // check overflow first.
            // Reason: there should only be an overflow if we are adding terms,
            //         in which case checking the in core overflow first may
            //         save a lot of time.
            //
            if( !overflow.empty() )
            {
                overflow_t::iterator iter = overflow.find( tid );
                if( overflow.end() != iter )
                {
                    ret = iter->second;
                    return ret;
                }
            }

            //
            // lookup term in prefix blocks
            //
            blockindex_t::iterator bi = blockindex.upper_bound( tid );
            if( bi == blockindex.begin() )
                return "";
            --bi;

            blocknum_t blockNum = bi->second;
            LG_FTXLEXI_D << "checking in block:" << blockNum
                         << " for tid:" << tid
                         << endl;
            
            fh_revblock b = getBlock( blockNum );
            ret = b->lookup( tid );

            LG_FTXLEXI_D << "ReverseLexicon_FrontCodedBlocks::lookup() tid:" << tid
                         << " blockNum:" << blockNum
                         << " ret:" << ret << endl;
            return ret;
        }
        
        
    };
};
