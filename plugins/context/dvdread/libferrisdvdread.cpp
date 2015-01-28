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

    $Id: libferrisdvdread.cpp,v 1.4 2010/09/24 21:31:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/**
 * Playback by cell in this pgc, starting at the cell for our chapter.
 * this bit of code is based somewhat on play_title.c
 * Copyright (C) 2001 Billy Biggs <vektor@dumbterm.net>.
 */

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <FerrisContextPlugin.hh>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_types.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>
#include <dvdread/nav_print.h>

using namespace std;
namespace Ferris
{

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    string
    tostr( dvd_time_t* dtime )
    {
        int s = dtime->second;
        int m = dtime->minute;
        int h = dtime->hour;
        int f = dtime->frame_u & 0x3f;

        fh_stringstream ss;
        ss << h << ":" << m << ":" << s << "." << f;
        return tostr(ss);
    }


    guint64
    frameCount( dvd_time_t* dtime )
    {
        double rate = 0;

        int s = dtime->second;
        int m = dtime->minute;
        int h = dtime->hour;
        int f = dtime->frame_u & 0x3f;

        switch((dtime->frame_u & 0xc0) >> 6)
        {
        case 1: rate = 25.00; break;
        case 3: rate = 29.97; break;
        }

        guint64 frameCount = s + (m*60) + (h*3600);
        frameCount  = static_cast<guint64>(frameCount*rate);
        frameCount += f;

        return frameCount;
    }
    

    class dvd;
    class title;
    class chapter;

    FERRIS_SMARTPTR( dvd,     fh_dvd );
    FERRIS_SMARTPTR( title,   fh_title );
    FERRIS_SMARTPTR( chapter, fh_chapter );


    class FERRISEXP_DLLLOCAL dvd
        :
        public Handlable
    {
        dvd_reader_t* dvd_reader;
        ifo_handle_t* vmg_file;
        tt_srpt_t* tt_srpt;

        string getPath();
        void ensureOpen( string path );
        
    public:

        dvd();
        ~dvd();

        int getTitleCount();
        fh_title getTitle( int n );
        dvd_reader_t* getReader();
        ifo_handle_t* getVM();
        
    };

    class FERRISEXP_DLLLOCAL title
        :
        public Handlable
    {
        fh_dvd DVD;
        int titleid;
        title_info_t* tinfo;
        ifo_handle_t* vts_file;

        void validateChapAngleData( int n, int angle=0 );
        
    public:

        title( fh_dvd _dvd, int titleid, title_info_t* _tinfo );
        ~title();
        
        ifo_handle_t* getVTS();
        int get_vts_ttn();

        int getTitleSetNumber();
        int getChapterCount();
        int getAngleCount();
        fh_chapter getChapter( int n, int angle=0 );
        
        fh_istream tostream( int n, int angle=0 );
        fh_istream tostream( int start, int end=-1, int angle=0 );
        fh_istream tostream( int angle=0 );
    };

    class FERRISEXP_DLLLOCAL chapter
        :
        public Handlable
    {
        fh_title t;
        int chapid;
        
    public:
        
        chapter( fh_title _t, int _chapid );
        virtual ~chapter();

        pgc_t* getPGC();
        cell_playback_t* getStartCell();

        streamsize getSize();
        string     getRunTime();
        guint64    getFrameCount();
    };
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_DLLLOCAL ferris_basic_streambuf_dvd
    {
    protected:

        inline static const std::streamsize getPBSize()
            {
                return 4;
            }
        
        inline static const std::streamsize getBufSize()
            {
                return 2 * 1024 * 1024 + DVD_VIDEO_LB_LEN + getPBSize();
            }
    };
    

    /*
     * Note that the data[] backbuffer is not really needed, it is a relic.
     * We pass DVDRead the buffer itself now so that we can avoid having to
     * perform costly memcpy() calls.
     *
     * This assumes that always: cur_output_size * DVD_VIDEO_LB_LEN < 2Mb
     */
    template<
        class _CharT,
        class _Traits = std::char_traits < _CharT >,
        class _Alloc  = std::allocator   < _CharT >,
        class _BufferSizers = ferris_basic_streambuf_dvd
        >
    class basic_dvd_streambuf
        :
        public ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
    {
        typedef basic_dvd_streambuf<_CharT, _Traits, _Alloc, _BufferSizers > _Self;

        /**
         * Setup the reading process at the correct location and open IFO and DVD
         * files.
         */
        void openTrack()
            {
                cur_chapid = startchapid;

                tt_srpt_t* srpt= DVD->getVM()->tt_srpt;
                title_info_t* tinfo = &srpt->title[ titleid ];
                int titlenum = tinfo->title_set_nr;
                
                vts_file = ifoOpen( DVD->getReader(), titlenum );
                if( !vts_file )
                {
                    fh_stringstream ss;
                    ss << "Can't open the title " << titlenum << " info file";
                    Throw_DVDReadException( tostr(ss), 0 );
                }

                int   ttn      = srpt->title[ titleid ].vts_ttn;
                ptt_info_t ppt = vts_file->vts_ptt_srpt->title[ ttn - 1 ].ptt[ cur_chapid ];
                cur_pgc  = vts_file->vts_pgcit->pgci_srp[ ppt.pgcn - 1 ].pgc;
                cur_cell = cur_pgc->program_map[ ppt.pgn - 1 ] - 1;
                
                title = DVDOpenFile( DVD->getReader(), titlenum, DVD_READ_TITLE_VOBS );
                if( !title )
                {
                    fh_stringstream ss;
                    ss << "Can't open the VOBS:" << titlenum;
                    Throw_DVDReadException( tostr(ss), 0 );
                }

                next_cell = cur_cell;
                virgin = true;
                PreCached = 0;
                reading_cell_data = false;
            }
        
    public:
    
        typedef char_traits<_CharT>    traits_type;
        typedef typename traits_type::int_type  int_type;
        typedef typename traits_type::char_type char_type;
        typedef typename traits_type::pos_type  pos_type;
        typedef typename traits_type::off_type  off_type;


        basic_dvd_streambuf( fh_dvd _dvd, int _titleid,
                             int _startchapid = 0, int _endchapid = -1,
                             int _angle = -1 )
            :
            DVD( _dvd ),
            titleid( _titleid ),
            startchapid( _startchapid ),
            endchapid( _endchapid ),
            angle( _angle ),
            vts_file( 0 ),
            title( 0 )
            {
                openTrack();
            }

        virtual ~basic_dvd_streambuf()
            {
                if( vts_file )  ifoClose( vts_file );
                if( title )     DVDCloseFile( title );
            }
    
    private:

        fh_dvd DVD;
        int titleid;
        int startchapid;
        int endchapid;
        int angle;
        bool virgin;
        streamsize PreCached;
        bool reading_cell_data;

        // these are used in the reading process
        int cur_chapid;
        pgc_t *cur_pgc;
        int cur_cell;
        dvd_file_t *title;
        ifo_handle_t *vts_file;

        int next_cell;
        unsigned int cur_pack;
        unsigned char data[ 1024 * DVD_VIDEO_LB_LEN ];
        int len;
        
        
        // prohibit copy/assign
        basic_dvd_streambuf( const basic_dvd_streambuf& );
        basic_dvd_streambuf& operator = ( const basic_dvd_streambuf& );

    protected:

        /**
         * Returns the size of the new data in the data[] var, or 0 if its time
         * to move on.
         *
         * Call read_cells() which calls this method when it needs to read the
         * data of a individual cell
         */
        streamsize read_cell_data( char_type* buffer, streamsize maxsz )
            {
                streamsize navsz = 0;
                LG_DVDREAD_D << "read_cell_data() " << endl;
                
                if( cur_pack >= cur_pgc->cell_playback[ cur_cell ].last_sector )
                {
                    LG_DVDREAD_D << " cur_pack >= last_sector" << endl;
                    return 0;
                }
                
                dsi_t dsi_pack;
                unsigned int next_vobu, next_ilvu_start, cur_output_size;
                
                
                /**
                 * Read NAV packet.
                 */
//                len = DVDReadBlocks( title, (int) cur_pack, 1, data );
                len = DVDReadBlocks( title, (int) cur_pack, 1, (unsigned char*)buffer );
                if( len != 1 )
                {
                    fh_stringstream ss;
                    ss << "Read failed for block:" << cur_pack;
                    Throw_DVDReadException( tostr(ss), 0 );
                }

                /**
                 * Parse the contained dsi packet.
                 */
                navRead_DSI( &dsi_pack, (unsigned char*)&(buffer[ DSI_START_BYTE ]) );
                assert( cur_pack == dsi_pack.dsi_gi.nv_pck_lbn );
                navsz   = DVD_VIDEO_LB_LEN;
                buffer += DVD_VIDEO_LB_LEN;


                /**
                 * Determine where we go next.  These values are the ones we mostly
                 * care about.
                 */
                next_ilvu_start = cur_pack
                    + dsi_pack.sml_agli.data[ angle ].address;
                cur_output_size = dsi_pack.dsi_gi.vobu_ea;


                /**
                 * If we're not at the end of this cell, we can determine the next
                 * VOBU to display using the VOBU_SRI information section of the
                 * DSI.  Using this value correctly follows the current angle,
                 * avoiding the doubled scenes in The Matrix, and makes our life
                 * really happy.
                 *
                 * Otherwise, we set our next address past the end of this cell to
                 * force the code above to go to the next cell in the program.
                 */
                if( dsi_pack.vobu_sri.next_vobu != SRI_END_OF_CELL ) {
                    next_vobu = cur_pack
                        + ( dsi_pack.vobu_sri.next_vobu & 0x7fffffff );
                } else {
                    next_vobu = cur_pack + cur_output_size + 1;
                }

                assert( cur_output_size < 1024 );
                cur_pack++;
                        
                /**
                 * Read in and output cursize packs.
                 */
                len = DVDReadBlocks( title,
                                     (int)cur_pack, cur_output_size,
                                     (unsigned char*)buffer );
                
                if( len != (int) cur_output_size )
                {
                    fh_stringstream ss;
                    ss << "Read failed for " << cur_output_size 
                       << " blocks at " << cur_pack;
                    Throw_DVDReadException( tostr(ss), 0 );
                }
                
//                        fwrite( data, cur_output_size, DVD_VIDEO_LB_LEN, stdout );
                cur_pack = next_vobu;
                return navsz + cur_output_size * DVD_VIDEO_LB_LEN;
            }

        /**
         * Read all the cells in the range of title/chapter/angle the user wants.
         *
         * data is in data[], or return==0 when done.
         */ 
        streamsize read_cells( char_type* buffer, streamsize maxsz )
            {
                LG_DVDREAD_D << "read_cells() reading_cell_data:" << reading_cell_data << endl;

                if( !reading_cell_data )
                {
                    if( next_cell >= cur_pgc->nr_of_cells )
                    {
                        return 0;
                    }
                
                    cur_cell = next_cell;
                    
                    /* Check if we're entering an angle block. */
                    if( cur_pgc->cell_playback[ cur_cell ].block_type
                        == BLOCK_TYPE_ANGLE_BLOCK )
                    {
                        int i;
                        
                        cur_cell += angle;
                        for( i = 0;; ++i ) {
                            if( cur_pgc->cell_playback[ cur_cell + i ].block_mode
                                == BLOCK_MODE_LAST_CELL ) {
                                next_cell = cur_cell + i + 1;
                                break;
                            }
                        }
                    } else {
                        next_cell = cur_cell + 1;
                    }
                    
                    
                    cur_pack = cur_pgc->cell_playback[ cur_cell ].first_sector;

                    /* Have we got an explicit end chapter and are we past it yet? */
                    if( endchapid >= 0 && cur_chapid > endchapid )
                    {
                        LG_DVDREAD_D << "Reached the end of the requested data."
                                     << " endchapid:" << endchapid
                                     << " cur_chapid:" << cur_chapid
                                     << endl;
                        return 0;
                    }
                    
                    ++cur_chapid;
                    reading_cell_data = true;
                }
                
                streamsize sz = read_cell_data( buffer, maxsz );
                if( !sz )
                {
                    reading_cell_data = false;
                    return read_cells( buffer, maxsz );
                }
                return sz;
            }
        
        /**
         * This is the only methods that really needs to be here. It gets
         * up to maxsz data into buffer and returns how much data was really
         * read. Return 0 for a failure, you must read atleast one byte.
         */
        virtual int make_new_data_avail( char_type* buffer, streamsize maxsz )
            {
                LG_DVDREAD_D << "make_new_data_avail() PreCached:" << PreCached
                             << " maxsz:" << maxsz
                             << endl;
                
                if( !PreCached )
                {
                    PreCached = read_cells( buffer, maxsz );
                }

                streamsize ret = PreCached;
                PreCached = 0;
                return ret;

//                 /*
//                  * fallback to copying from a backbuffer.
//                  */
//                 streamsize xfer = maxsz;
//                 if( PreCached < xfer )
//                     xfer = PreCached;

//                 memcpy( buffer, data, xfer );
//                 PreCached -= xfer;
                
//                 return xfer;
            }

        virtual pos_type
        seekoff( off_type offset,
                 typename _Self::seekd_t d,
                 int m )
            {
                return -1;
            }

        virtual pos_type
        seekpos(pos_type pos, int m)
            {
                return -1;
            }

    
    
    };




    template<
        class _CharT,
        class _Traits = std::char_traits<_CharT>
    >
    class ferris_dvd_istream
        :
        public Ferris_istream< _CharT, _Traits >,
        public i_ferris_stream_traits< _CharT, _Traits >
    {
        typedef ferris_dvd_istream<_CharT, _Traits>    _Self;
        typedef basic_dvd_streambuf<_CharT, _Traits> _StreamBuf;

        typedef basic_dvd_streambuf<_CharT, _Traits> ss_impl_t;
        FERRIS_SMARTPTR( ss_impl_t, ss_t );
        ss_t ss;

    public:
    
        typedef char_traits<_CharT>    traits_type;
        typedef typename traits_type::int_type  int_type;
        typedef typename traits_type::char_type char_type;
        typedef typename traits_type::pos_type  pos_type;
        typedef typename traits_type::off_type  off_type;

        typedef emptystream_methods< char_type, traits_type > delegating_methods;
    

        explicit
        ferris_dvd_istream( fh_dvd _dvd, int _titleid,
                            int _startchapid = 0, int _endchapid = -1,
                            int _angle = -1 )
            :
            ss( new ss_impl_t( _dvd, _titleid, _startchapid, _endchapid, _angle ) )
            {
                init( rdbuf() );
                setsbT( GetImpl(ss) );
            }

        ferris_dvd_istream( const ferris_dvd_istream& rhs )
            :
            ss( rhs.ss )
            {
                init( rdbuf() );
                setsbT( GetImpl(ss) );
            }
    
        virtual ~ferris_dvd_istream()
            {
            }
    
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
            stream_writable = false
        };
    };

    typedef ferris_dvd_istream<char>  f_dvd_istream;
    typedef ferris_dvd_istream<char> fh_dvd_istream;
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    dvd::dvd()
        :
        dvd_reader(0),
        vmg_file(0),
        tt_srpt(0)
    {
    }

    dvd::~dvd()
    {
        if( vmg_file )
        {
            ifoClose( vmg_file );
        }
        
        if( dvd_reader )
        {
            DVDClose( dvd_reader );
        }
    }
    
    string
    dvd::getPath()
    {
        return "/dev/dvd";
    }
    
    void
    dvd::ensureOpen( string path )
    {
        if( !dvd_reader )
        {
            dvd_reader = DVDOpen( path.c_str() );
            if( !dvd_reader )
            {
                fh_stringstream ss;
                ss << "Can not open dvd at:" << path;
                Throw_DVDReadException( tostr(ss), 0 );
            }
        }
        
        if( !vmg_file )
        {
            vmg_file = ifoOpen( dvd_reader, 0 );
            if( !vmg_file )
            {
                fh_stringstream ss;
                ss << "Can not open VMG info for dvd at:" << path;
                Throw_DVDReadException( tostr(ss), 0 );
            }
        }
        
        tt_srpt = vmg_file->tt_srpt;
    }
    
    int
    dvd::getTitleCount()
    {
        ensureOpen( getPath() );
        return tt_srpt->nr_of_srpts;
    }
    
    fh_title
    dvd::getTitle( int n )
    {
        ensureOpen( getPath() );
        title_info_t* info = 0;

        if( n < 0 || n > tt_srpt->nr_of_srpts )
        {
            fh_stringstream ss;
            ss << "Invalid title requested:" << n
               << "valid range 0-" << tt_srpt->nr_of_srpts;
            Throw_DVDReadException( tostr(ss), 0 );
        }
        info = &tt_srpt->title[ n ];

        return new title( this, n, info );
    }

    dvd_reader_t*
    dvd::getReader()
    {
        ensureOpen( getPath() );
        return dvd_reader;
    }

    ifo_handle_t*
    dvd::getVM()
    {
        ensureOpen( getPath() );
        return vmg_file;
    }
    
    
    
    /******************************************************************************/
    /******************************************************************************/

    title::title( fh_dvd _dvd, int _titleid, title_info_t* _tinfo )
        :
        DVD( _dvd ),
        titleid( _titleid ),
        tinfo( _tinfo ),
        vts_file( 0 )
    {
    }

    title::~title()
    {
        if( vts_file )
        {
            ifoClose( vts_file );
        }
    }

    fh_chapter
    title::getChapter( int n, int angle )
    {
        fh_chapter chap = new chapter( this, n );
        return chap;
    }
    
    ifo_handle_t*
    title::getVTS()
    {
        if( vts_file )
        {
            return vts_file;
        }
        
        tt_srpt_t*    srpt  = DVD->getVM()->tt_srpt;
        title_info_t* tinfo = &srpt->title[ titleid ];
        int titlenum = tinfo->title_set_nr;

        vts_file = ifoOpen( DVD->getReader(), titlenum );
        if( !vts_file )
        {
            fh_stringstream ss;
            ss << "Can't open the title " << titlenum << " info file";
            Throw_DVDReadException( tostr(ss), 0 );
        }
        
        return vts_file;
    }

    int
    title::get_vts_ttn()
    {
        int ttn = tinfo->vts_ttn;
        return ttn;
    }
    
    int
    title::getTitleSetNumber()
    {
        return tinfo->title_set_nr;
    }
    
    int
    title::getChapterCount()
    {
        return tinfo->nr_of_ptts;
        
    }
    
    int
    title::getAngleCount()
    {
        return tinfo->nr_of_angles;
    }

    void
    title::validateChapAngleData( int n, int angle )
    {
        if( n < 0 || n > getChapterCount() )
        {
            fh_stringstream ss;
            ss << "Invalid chapter requested:" << n << " angle:" << angle
               << " valid range 0-" << getChapterCount();
            Throw_DVDReadException( tostr(ss), 0 );
        }

        if( angle < 0 || angle > getAngleCount() )
        {
            fh_stringstream ss;
            ss << "Invalid angle requested:" << n << " angle:" << angle
               << " valid range 0-" << getAngleCount();
            Throw_DVDReadException( tostr(ss), 0 );
        }
    }
    
    fh_istream
    title::tostream( int n, int angle )
    {
        return tostream( n, n, angle );
    }
    
    fh_istream
    title::tostream( int start, int end, int angle )
    {
        validateChapAngleData( start, angle );
        if( end != -1 )
        {
            validateChapAngleData( end, angle );
        }

        LG_DVDREAD_D << "tostream() title:" << getTitleSetNumber()
                     << " start:" << start
                     << " end:" << end
                     << " angle:" << angle
                     << endl;
        
//        fh_dvd_istream ss( DVD, getTitleSetNumber(), start, end, angle );
        fh_dvd_istream ss( DVD, titleid, start, end, angle );
        return ss;
    }
    
    fh_istream
    title::tostream( int angle )
    {
        return tostream( 0, getChapterCount(), angle );
    }
    

    /******************************************************************************/
    /******************************************************************************/

    chapter::chapter( fh_title _t, int _chapid )
        :
        t(_t),
        chapid( _chapid )
    {
    }

    chapter::~chapter()
    {
    }

    pgc_t*
    chapter::getPGC()
    {
        ifo_handle_t* vts = t->getVTS();
        
        int        ttn = t->get_vts_ttn();
        ptt_info_t ppt = vts->vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ];
        pgc_t*     pgc = vts->vts_pgcit->pgci_srp[ ppt.pgcn - 1 ].pgc;
        return pgc;
    }

    cell_playback_t*
    chapter::getStartCell()
    {
        pgc_t* pgc         = getPGC();
        ifo_handle_t* vts  = t->getVTS();

        int        ttn        = t->get_vts_ttn();
        ptt_info_t ppt        = vts->vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ];
        int start_cell        =  pgc->program_map[ ppt.pgn - 1 ] - 1;
        cell_playback_t* cell = &pgc->cell_playback[ start_cell ];

        return cell;
    }

    string
    chapter::getRunTime()
    {
        cell_playback_t* cell = getStartCell();
        return tostr( &cell->playback_time );
    }
    
    guint64
    chapter::getFrameCount()
    {
        cell_playback_t* cell = getStartCell();
        return frameCount( &cell->playback_time );
    }
    
    streamsize
    chapter::getSize()
    {
        cell_playback_t* cell = getStartCell();
        int secdiff = cell->last_sector - cell->first_sector;

        streamsize sz = 2048 * (secdiff+1);
        return sz;
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

//     template <class ChildContext, class ParentContext>
//     class TwoC
//         :
//         public 
//     ParentPointingTreeContext< ChildContext, RecommendedEACollectingContext< ParentContext > >
//     {
//     protected:
        
//         typedef TwoC< ChildContext, ParentContext > _Self;
//         typedef 
//         ParentPointingTreeContext< ChildContext, RecommendedEACollectingContext< ParentContext > >
//         _Base;

// //         virtual void createStateLessAttributes( bool force = false )
// //             {
// //                 static Util::SingleShot virgin;
// //                 if( force || virgin() )
// //                 {
// //                     cerr << "twoC::createStateLessAttributes() -----------------" << endl;
// //                     _Base::createStateLessAttributes( true );
// //                     supplementStateLessAttributes( true );

// //                     fh_attribute a = getAttribute( "name" );
// //                     cerr << "twoC::createStateLessAttributes(2) -----------------" << endl;
// //                 }
// //             }

//         virtual void getTypeInfos( std::list< Loki::TypeInfo >& l )
//             {
//                 cerr << " --- twoC::getTypeInfos(enter) --- " << endl;
                
//                 l.push_back( typeid( _Self ) );
//                 _Base::getTypeInfos( l );

//                 cerr << ".Self TI:" << typeid( _Self ).name() << endl;
//                 typedef std::list< Loki::TypeInfo >::iterator I;
//                 for( I iter = l.begin(); iter != l.end(); ++iter )
//                 {
//                     cerr << " ti:" << iter->name() << endl;
//                 }

//                 cerr << " --- twoC::getTypeInfos(exit) --- " << endl;
//             }
        
//     public:
        
//         TwoC( Context* parent = 0, const std::string& rdn = "" )
//             :
//             _Base( parent, rdn )
//             {
// //                createStateLessAttributes();
//             }
//     };
    
    
    class FERRISEXP_CTXPLUGIN dvdContext
        :
        public StateLessEAHolding_Recommending_ParentPointingTree_Context< dvdContext >
    {
        typedef dvdContext                                _Self;
        typedef
        StateLessEAHolding_Recommending_ParentPointingTree_Context< dvdContext >
        _Base;
        
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        friend fh_istream SL_getSizeStream( dvdContext* c, const std::string& rdn, EA_Atom* atom );
        friend streamsize getChapterSize( dvdContext* c, int chapid );

        dvdContext*
        priv_CreateContext( Context* parent, string rdn )
            {
                dvdContext* ret = new dvdContext();
                ret->setContext( parent, rdn );
                return ret;
            }
    

    protected:

        virtual void priv_read();
        virtual void createStateLessAttributes( bool force = false );

        virtual string getRecommendedEA()
            {
                string ret = _Base::getRecommendedEA();
                ret += ",width,height,video-encoding,aspect-ratio,frame-count,duration,size";
                return ret;
            }
        
    public:

        /* State for getIStream() */
        int titleid;
        int startchapid;
        int endchapid;
        int angleid;
        fh_dvd DVD;

        void constructObject( int titleid = -1,
                    int startchapid = -1,
                    int endchapid = -1,
                    int angleid = 0 );
        
        dvdContext( Context* parent = 0, const std::string& rdn = "" );
        ~dvdContext();

        virtual fh_istream getIStream( ferris_ios::openmode m = ios::in )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception);
    
        fh_title getTitle();
        fh_chapter getChapter();
        
    };

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

//     void show_time( dvd_time_t* dtime )
//     {
//         double rate = 0;

//         int s = dtime->second;
//         int m = dtime->minute;
//         int h = dtime->hour;
//         int f = dtime->frame_u & 0x3f;

//         cerr << h << ":" << m << ":" << s << "." << f << endl;

//         switch((dtime->frame_u & 0xc0) >> 6)
//         {
//         case 1: rate = 25.00; break;
//         case 3: rate = 29.97; break;
//         }

//         int frameCount = s + (m*60) + (h*3600);
//         frameCount *= rate;
//         frameCount += f;
//         cerr << "Playback rate:" << rate << " frames:" << frameCount
// //             << " fx * rate:" << (frameCount * rate)
//              << endl;
//     }
    
    streamsize getChapterSize( dvdContext* c, int chapid )
    {
        fh_title title = c->getTitle();
        fh_chapter chap = title->getChapter( chapid );
        return chap->getSize();
        
        
//         ifo_handle_t* vfs = title->getVTS();
        
// //        int   ttn      = srpt->title[ titlenum ].vts_ttn;
//         int   ttn      = srpt->title[ c->titleid ].vts_ttn;
//         ptt_info_t ppt = vts_file->vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ];
//         cur_pgc  = vts_file->vts_pgcit->pgci_srp[ ppt.pgcn - 1 ].pgc;
//         int start_cell = cur_pgc->program_map[ ppt.pgn - 1 ] - 1;
//         cell_playback_t* cell = &cur_pgc->cell_playback[ start_cell ];
//         int secdiff = cell->last_sector - cell->first_sector;

// //        show_time( &cur_pgc->playback_time );
//         show_time( &cell->playback_time );

        

//         streamsize sz = 2048 * (secdiff+1);
//         return sz;
    }

    
    fh_istream
    SL_getFrameCount( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        if( c->endchapid >= 0 && c->endchapid == c->startchapid )
        {
            fh_stringstream ss;
            ss << c->getChapter()->getFrameCount();
            return ss;
        }
        if( c->endchapid < 0 && c->titleid >= 0 )
        {
            pgc_t* pgc = c->getTitle()->getChapter( c->startchapid )->getPGC();

            fh_stringstream ss;
            ss << frameCount( &pgc->playback_time );
            return ss;
        }
        fh_stringstream ss;
        return ss;
    }
    
    fh_istream
    SL_getDuration( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        if( c->endchapid >= 0 && c->endchapid == c->startchapid )
        {
            fh_stringstream ss;
            ss << c->getChapter()->getRunTime();
            return ss;
        }
        if( c->endchapid < 0 && c->titleid >= 0 )
        {
            pgc_t* pgc = c->getTitle()->getChapter( c->startchapid )->getPGC();

            fh_stringstream ss;
            ss << tostr( &pgc->playback_time );
            return ss;
        }
        fh_stringstream ss;
        return ss;
    }

    struct Dimension 
    {
        int w;
        int h;
        string encoding;
        string aspect;
        string presentation;
        bool letterbox;

        Dimension()
            {
                reset();
            }
        
        void reset()
            {
                w = 0;
                h = 0;
                encoding = "";
                aspect = "";
                presentation = "";
                letterbox = 0;
            }
        
    };

    Dimension getDimension( dvdContext* c )
    {
        Dimension ret;

        ifo_handle_t* vts = c->getTitle()->getVTS();
        video_attr_t  va  = vts->vtsi_mat->vts_video_attr;

        ret.h = va.video_format ? 576 : 480;
        switch( va.picture_size )
        {
        case 0: ret.w = 720; break;
        case 1: ret.w = 704; break;
        case 2: ret.w = 352; break;
        case 3: ret.w = 352; break;
        }

        ret.letterbox = va.letterboxed;

        switch( va.permitted_df )
        {
        case 0: ret.presentation = "pan&scan,letterboxed"; break;
        case 1: ret.presentation = "pan&scan";             break;
        case 2: ret.presentation = "letterboxed";          break;
        }

        ret.aspect   = (va.display_aspect_ratio ? "16:9" : "4:3");
        ret.encoding = (va.video_format ? "pal" : "ntsc");

        return ret;
    }
    
    
    fh_istream
    SL_getWidth( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).w;
        return ss;
    }

    fh_istream
    SL_getHeight( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).h;
        return ss;
    }

    fh_istream
    SL_getEncoding( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).encoding;
        return ss;
    }

    fh_istream
    SL_getPresentation( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).presentation;
        return ss;
    }

    fh_istream
    SL_getAspectRatio( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).aspect;
        return ss;
    }
    
    fh_istream
    SL_isLetterBox( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << getDimension( c ).letterbox;
        return ss;
    }
    
    
    fh_istream
    SL_getSizeStream( dvdContext* c, const std::string& rdn, EA_Atom* atom )
    {
        if( c->endchapid >= 0 && c->endchapid == c->startchapid )
        {
            fh_stringstream ss;
            ss << getChapterSize( c, c->startchapid );
            return ss;
        }
        if( c->endchapid < 0 && c->titleid >= 0 )
        {
            streamsize sz = 0;

            tt_srpt_t* srpt= c->DVD->getVM()->tt_srpt;
            title_info_t* tinfo = &srpt->title[ c->titleid ];
            int startchap = c->startchapid;
            int maxchap = tinfo->nr_of_ptts;

            if( startchap == -1 )
                startchap = 0;
            
            for( int i=startchap; i<maxchap; ++i )
            {
                streamsize chapsz = getChapterSize( c, i );
                sz = sz + chapsz;
//                 cerr << "i:" << i << " sz:" << sz << " chapsz:" << chapsz
//                      << " size:" << sizeof(sz) << endl;
            }
            
            fh_stringstream ss;
            ss << sz;
            return ss;
        }
        

        fh_stringstream ss;
        ss << "0";
        return ss;
    }

    void
    dvdContext::constructObject( int _titleid,   int _startchapid, 
                       int _endchapid, int _angleid )
    {
        titleid     = _titleid;
        startchapid = _startchapid;
        endchapid   = _endchapid;
        angleid     = _angleid;
    }        

    dvdContext::dvdContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn ),
        DVD( new dvd() ),
        titleid( 0 ),
        startchapid( 0 ),
        endchapid( 0 ),
        angleid( 0 )
    {
        LG_DVDREAD_D << "dvdContext::dvdContext()" << endl;
        createStateLessAttributes();
    }

    dvdContext::~dvdContext()
    {
    }

    fh_title
    dvdContext::getTitle()
    {
        return DVD->getTitle( titleid );
    }
    
    fh_chapter
    dvdContext::getChapter()
    {
        fh_title title  = getTitle();
        fh_chapter chap = title->getChapter( startchapid );
        return chap;
    }
    
    
    void
    dvdContext::createStateLessAttributes( bool force )
    {
        static Util::SingleShot virgin;
        if( virgin() )
        {
#define SLEA tryAddStateLessAttribute         

            SLEA( "size",        SL_getSizeStream, FXD_FILESIZE  );
            SLEA( "frame-count", SL_getFrameCount, XSD_BASIC_INT );
            SLEA( "duration",    SL_getDuration,   XSD_BASIC_INT );

            SLEA( "width",  SL_getWidth,  FXD_WIDTH_PIXELS );
            SLEA( "height", SL_getHeight, FXD_HEIGHT_PIXELS );


            SLEA( "video-encoding", SL_getEncoding,     XSD_BASIC_STRING );
            SLEA( "presentation",   SL_getPresentation, XSD_BASIC_STRING );
            SLEA( "aspect-ratio",   SL_getAspectRatio,  XSD_BASIC_STRING );
            SLEA( "letterbox",      SL_isLetterBox,     XSD_BASIC_BOOL   );
            
#undef  SLEA
            
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );


            fh_attribute a = getAttribute( "name" );
        }
    }

    fh_istream
    dvdContext::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        LG_DVDREAD_D << "getIStream() url:" << getURL() << endl;
        
        if( titleid == -1 || startchapid == -1 )
        {
            fh_stringstream ss;
            ss << "Attempt to get a IStream for a context that is not readable"
               << " url:" << getURL();
            Throw_CanNotGetStream( tostr(ss), this);
        }

        LG_DVDREAD_D << "getIStream() titleid:" << titleid
                     << " start:" << startchapid
                     << " end:" << endchapid
                     << endl;

        fh_title ti = DVD->getTitle( titleid );
        return ti->tostream( startchapid, endchapid,  angleid );
    }


    void
    dvdContext::priv_read()
    {
        LG_DVDREAD_D << "dvdContext::priv_read() path:" << getDirPath() << endl;

        if( getBaseContext() != this )
        {
            LG_DVDREAD_D << "dvdContext::priv_read() reading a fake child. path:"
                         << getDirPath()
                         << endl;

            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
    
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        AlreadyEmittedCacheRAII _raiiec( this );
        LG_DVDREAD_D << "dvdContext::priv_read(1) path:" << getDirPath() << endl;
        LG_DVDREAD_D << "dvdContext::priv_read(1) name:" << getDirName() << endl;

        /******************************************************************************/
        /******************************************************************************/

        int tc = DVD->getTitleCount();
        LG_DVDREAD_D << " titles:" << tc << endl;
        
        for( int i=0; i<tc; ++i )
        {
            fh_title t = DVD->getTitle( i );
            LG_DVDREAD_D << " title:" << i << " has chapter count:"
                         << t->getChapterCount()
                         << endl;
            LG_DVDREAD_D << " title:" << i << " has angle   count:"
                         << t->getAngleCount()
                         << endl;

            /* Add a titleX.vob for complete title play */
            {
                fh_stringstream rdnss;
                rdnss << "title_" << i << ".vob";
                string rdn = tostr(rdnss);

                dvdContext* cc = 0;
                cc = priv_ensureSubContext( rdn, cc );
                cc->constructObject( i, 0 );
                
//                 dvdContext* child = new dvdContext( i, 0 );
//                 child->setContext( this, monsterName( rdn ));
//                 Insert( child, false );
            }

            /* Add a titleX.vob for complete title play of each angle */
            for( int angle = 0; angle < t->getAngleCount(); ++angle )
            {
                fh_stringstream dirss;
                dirss << "angle_" << angle;
                string dirname = tostr(dirss);
                dvdContext* dc = GetImpl(ensureContextCreated( dirname ));

                fh_stringstream rdnss;
                rdnss << "title_" << i << ".vob";
                string rdn = tostr(rdnss);

                dvdContext* cc = 0;
                cc = dc->priv_ensureSubContext( rdn, cc );
                cc->constructObject( i, 0, -1, angle );
                
//                 dvdContext* child = new dvdContext( i, 0, -1, angle );
// //                 cerr << "Adding new child rdn:" << rdn
// //                      << " monstered.rdn:" << dc->monsterName( rdn )
// //                      << endl;
//                 child->setContext( dc, dc->monsterName( rdn ));
//                 dc->Insert( child, false );
            }
        
            /* Add individual VOB files for each chapter */
            {
                fh_stringstream dirss;
                dirss << "title_" << i;
                string dirname = tostr(dirss);
                dvdContext* dc = GetImpl(ensureContextCreated( dirname ));

                for( int chap=0; chap < t->getChapterCount(); ++chap )
                {
                    fh_stringstream rdnss;
                    rdnss << "chapter_" << chap << ".vob";
                    string rdn = tostr(rdnss);

                dvdContext* cc = 0;
                cc = dc->priv_ensureSubContext( rdn, cc );
                cc->constructObject( i, chap, chap );
                    
//                     dvdContext* child = new dvdContext( i, chap, chap );
// //                     cerr << "Adding new child rdn:" << rdn
// //                          << " monstered.rdn:" << dc->monsterName( rdn )
// //                          << endl;
//                     child->setContext( dc, monsterName( rdn ));
//                     dc->Insert( child, false );
                }
            }
        }
        
        /******************************************************************************/
        /******************************************************************************/

        LG_DVDREAD_D << "dvdContext::priv_read(done) path:" << getDirPath() << endl;
    }






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            static dvdContext c( 0, "" );
            fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
            return ret;
        }
    }

};
