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

    $Id: libferrismpeg2.cpp,v 1.2 2010/09/24 21:31:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Runner.hh>

#include <inttypes.h>	
extern "C"
{
// #include <mpeg2dec/mm_accel.h>
// #include <mpeg2dec/video_out.h>
// #include <mpeg2dec/mpeg2.h>
//#include "video_out_internal.h"
};

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>



// FIXME:
#define HAVE_LIBMPEG2_GETRES_VO 1

using namespace std;
namespace Ferris
{
    
    const char* A52AttrPrefix = "a52-raw-";
    const char* A52AttrPrefixArray[] =
    {
        "a52-raw-0",
        "a52-raw-1",
        "a52-raw-2",
        "a52-raw-3",
        "a52-raw-4",
        "a52-raw-5",
        "a52-raw-6",
        "a52-raw-7",
        0
    };
    
    


    class FERRISEXP_DLLLOCAL EAGenerator_Mpeg2 : public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Mpeg2();
        virtual void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
        {
            ss << ",width,height";
        }
        
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    

    EAGenerator_Mpeg2::EAGenerator_Mpeg2()
        :
        MatchedEAGeneratorFactory()
    {
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


//     vo_instance_t*
//     getVO( const string& s )
//     {
//         vo_instance_t* ret      = 0;
//         vo_open_t*     openflag = 0;
//         vo_driver_t*   drivers  = vo_drivers ();
//         int i = 0;

//         for (i=0; drivers[i].name; i++)
//         {
//             string dn = drivers[i].name;
//             if( dn == s )
//             {
//                 openflag = drivers[i].open;
//             }
//         }
    
//         return ret;
//     }


//     class FERRISEXP_DLLLOCAL ferris_basic_streambuf_mp2sized
//     {
//     protected:

//         inline static const std::streamsize getPBSize()
//             {
//                 return 4;
//             }
        
//         inline static const std::streamsize getBufSize()
//             {
//                 return 4 * 1024 * 1024 + getPBSize();
//             }
//     };
    

    
//     /*
//      *
//      */
//     template<
//         class _CharT,
//         class _Traits = std::char_traits < _CharT >,
//         class _Alloc  = std::allocator   < _CharT >,
//         class _BufferSizers = ferris_basic_streambuf_mp2sized
//         >
//     class basic_mpeg2_streambuf
//         :
//         public ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
//     {
//     public:
//         uint32_t accel;
//         string vo_name;
//         mpeg2dec_t mpeg2dec;
//         bool demux_ps;
//         bool DoneProcessing;
//         vo_instance_t* vo;
//         streamsize AvailableBufferSize;
//         streamsize BytesRead;
//         char_type* AvailableBuffer;
//         fh_stringstream OverFlowStream;

//         /**
//          */
//         void init()
//             {
//                 AvailableBufferSize = 0;
//                 AvailableBuffer     = 0;
                
//                 DoneProcessing = false;
//                 vo_name  = "pgmpipe";
//                 accel    = MM_ACCEL_MLIB | mm_accel();
//                 demux_ps = true;
                
//                 vo_accel( accel );
//                 vo = getVO( vo_name );

//                 if( !vo )
//                 {
//                     fh_stringstream ss;
//                     ss << "error opening VO layer, name:" << vo_name;
//                     Throw_CanNotGetStream( tostr(ss), 0 );
//                 }

//                 mpeg2_init ( &mpeg2dec, accel, vo );
//             }
        
//     public:
    
//         typedef char_traits<_CharT>    traits_type;
//         typedef traits_type::int_type  int_type;
//         typedef traits_type::char_type char_type;
//         typedef traits_type::pos_type  pos_type;
//         typedef traits_type::off_type  off_type;

//         basic_mpeg2_streambuf( fh_istream _rawStream )
//             :
//             rawStream( _rawStream ),
//             vo(0)
//             {
//                 init();
//             }

//         virtual ~basic_mpeg2_streambuf()
//             {
//                 mpeg2_close( &mpeg2dec );
//                 vo_close( vo );
//             }
    
//     private:

//         fh_istream rawStream;
        
//         // prohibit copy/assign
//         basic_mpeg2_streambuf( const basic_mpeg2_streambuf& );
//         basic_mpeg2_streambuf& operator = ( const basic_mpeg2_streambuf& );

//     protected:

//         streamsize ps_loop ( char_type* buffer, streamsize maxsz )
//             {
//                 const int rawbuf_sz=262144;
//                 static uint8_t rawbuf[ rawbuf_sz ];

//                 AvailableBufferSize = maxsz;
//                 AvailableBuffer     = buffer;
//                 BytesRead           = 0;
//                 OverFlowStream.clear();
//                 OverFlowStream.seekg(0);

//                 static int mpeg1_skip_table[16] = {
//                     1, 0xffff,      5,     10, 0xffff, 0xffff, 0xffff, 0xffff,
//                     0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
//                 };

//                 uint8_t * buf;
//                 uint8_t * end;
//                 uint8_t * tmp1;
//                 uint8_t * tmp2;
//                 int complain_loudly;

//                 complain_loudly = 1;
//                 buf = rawbuf;

//                 do {
//                     rawStream.read( (char*)rawbuf, rawbuf_sz );
//                     streamsize sz = rawStream.gcount();
//                     end  = rawbuf + sz;
                    
// //                    end = buf + fread (buf, 1, rawbuf + rawbuf_sz - buf, in_file);
//                     buf = rawbuf;

//                     while (buf + 4 <= end) {
//                         /* check start code */
//                         if (buf[0] || buf[1] || (buf[2] != 0x01)) {
//                             if (complain_loudly) {
// //                                 fprintf (stderr, "missing start code at %#lx\n",
// //                                          ftell (in_file) - (end - buf));
//                                 if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0))
//                                     fprintf (stderr, "this stream appears to use "
//                                              "zero-byte padding before start codes,\n"
//                                              "which is not correct according to the "
//                                              "mpeg system standard.\n"
//                                              "mp1e was one encoder known to do this "
//                                              "before version 1.8.0.\n");
//                                 complain_loudly = 0;
//                             }
//                             buf++;
//                             continue;
//                         }

//                         switch (buf[3]) {
//                         case 0xb9:	/* program end code */
//                             return 0;
//                         case 0xba:	/* pack header */
//                             /* skip */
//                             if ((buf[4] & 0xc0) == 0x40)	/* mpeg2 */
//                                 tmp1 = buf + 14 + (buf[13] & 7);
//                             else if ((buf[4] & 0xf0) == 0x20)	/* mpeg1 */
//                                 tmp1 = buf + 12;
//                             else if (buf + 5 > end)
//                                 goto copy;
//                             else {
//                                 fprintf (stderr, "weird pack header\n");
//                                 exit (1);
//                             }
//                             if (tmp1 > end)
//                                 goto copy;
//                             buf = tmp1;
//                             break;
//                         case 0xe0:	/* video */
//                             tmp2 = buf + 6 + (buf[4] << 8) + buf[5];
//                             if (tmp2 > end)
//                                 goto copy;
//                             if ((buf[6] & 0xc0) == 0x80)	/* mpeg2 */
//                                 tmp1 = buf + 9 + buf[8];
//                             else {	/* mpeg1 */
//                                 for (tmp1 = buf + 6; *tmp1 == 0xff; tmp1++)
//                                     if (tmp1 == buf + 6 + 16) {
//                                         fprintf (stderr, "too much stuffing\n");
//                                         buf = tmp2;
//                                         break;
//                                     }
//                                 if ((*tmp1 & 0xc0) == 0x40)
//                                     tmp1 += 2;
//                                 tmp1 += mpeg1_skip_table [*tmp1 >> 4];
//                             }
//                             if (tmp1 < tmp2) {
//                                 int num_frames;

//                                 num_frames = mpeg2_decode_data (&mpeg2dec, tmp1, tmp2);
//                             }
//                             buf = tmp2;
//                             break;
//                         default:
//                             if (buf[3] < 0xb9) {
//                                 fprintf (stderr,
//                                          "looks like a video stream, not system stream\n");
//                                 exit (1);
//                             }
//                             /* skip */
//                             tmp1 = buf + 6 + (buf[4] << 8) + buf[5];
//                             if (tmp1 > end)
//                                 goto copy;
//                             buf = tmp1;
//                             break;
//                         }
//                     }

//                     if (buf < end) {
//                     copy:
//                         /* we only pass here for mpeg1 ps streams */
//                         memmove (rawbuf, buf, end - buf);
//                     }
//                     buf = rawbuf + (end - buf);

//                 } while (end == rawbuf + rawbuf_sz);
                
//                 return BytesRead;
//             }
        
//         streamsize es_loop ( char_type* buffer, streamsize maxsz )
//             {
//                 const int rawbuf_sz=262144;
//                 static uint8_t rawbuf[ rawbuf_sz ];

//                 AvailableBufferSize = maxsz;
//                 AvailableBuffer     = buffer;
//                 BytesRead           = 0;
//                 OverFlowStream.clear();
//                 OverFlowStream.seekg(0);
                
//                 int frameCount = 0;
//                 while( !frameCount )
//                 {
//                     rawStream.read( (char*)rawbuf, rawbuf_sz );
//                     streamsize sz = rawStream.gcount();
//                     uint8_t* end  = rawbuf + sz;
//                     frameCount    = mpeg2_decode_data ( &mpeg2dec, rawbuf, end );

//                     if( end != rawbuf + rawbuf_sz )
//                     {
//                         DoneProcessing = true;
//                         if( !frameCount )
//                             break;
//                     }
//                 }
                
//                 return BytesRead;
//             }
        
        
//         /**
//          * This is the only methods that really needs to be here. It gets
//          * up to maxsz data into buffer and returns how much data was really
//          * read. Return 0 for a failure, you must read atleast one byte.
//          */
//         virtual int make_new_data_avail( char_type* buffer, streamsize maxsz )
//             {
//                 LG_DVDREAD_D << "make_new_data_avail() "
//                              << " maxsz:" << maxsz
//                              << endl;

//                 if (demux_ps) ps_loop( buffer, maxsz );
//                 else          es_loop( buffer, maxsz );

//                 return BytesRead;
                
                
// //                 if( !PreCached )
// //                 {
// //                     PreCached = read_cells( buffer, maxsz );
// //                 }

// //                 streamsize ret = PreCached;
// //                 PreCached = 0;
// //                 return ret;
//             }

//         virtual pos_type
//         seekoff(off_type offset, seekd_t d, int m)
//             {
//                 return -1;
//             }

//         virtual pos_type
//         seekpos(pos_type pos, int m)
//             {
//                 return -1;
//             }
//     };




//     template<
//         class _CharT,
//         class _Traits = std::char_traits<_CharT>
//     >
//     class ferris_mpeg2_istream
//         :
//         public Ferris_istream< _CharT, _Traits >,
//         public i_ferris_stream_traits< _CharT, _Traits >
//     {
//         typedef ferris_mpeg2_istream<_CharT, _Traits>    _Self;
//         typedef basic_mpeg2_streambuf<_CharT, _Traits> _StreamBuf;

//         typedef basic_mpeg2_streambuf<_CharT, _Traits> ss_impl_t;
//         typedef Loki::SmartPtr<  ss_impl_t,
//                                  FerrisRefCounted,
//                                  Loki::DisallowConversion,
//                                  Loki::AssertCheck,
//                                  FerrisSmartPtrStorage > ss_t;
//         ss_t ss;

//     public:
    
//         typedef char_traits<_CharT>    traits_type;
//         typedef traits_type::int_type  int_type;
//         typedef traits_type::char_type char_type;
//         typedef traits_type::pos_type  pos_type;
//         typedef traits_type::off_type  off_type;

//         typedef emptystream_methods< char_type, traits_type > delegating_methods;
    

//         explicit
//         ferris_mpeg2_istream( fh_istream rawStream )
//             :
//             ss( new ss_impl_t( rawStream ) )
//             {
//                 init( rdbuf() );
//                 setsbT( GetImpl(ss) );
//             }

//         ferris_mpeg2_istream( const ferris_mpeg2_istream& rhs )
//             :
//             ss( rhs.ss )
//             {
//                 init( rdbuf() );
//                 setsbT( GetImpl(ss) );
//             }
    
//         virtual ~ferris_mpeg2_istream()
//             {
//             }
    
//         _Self* operator->()
//             {
//                 return this;
//             }

//         ss_impl_t*
//         rdbuf() const
//             {
//                 return GetImpl(ss);
//             }

//         enum
//         {
//             stream_readable = true,
//             stream_writable = false
//         };
//     };

//     typedef ferris_mpeg2_istream<char>  f_mpeg2_istream;
//     typedef ferris_mpeg2_istream<char> fh_mpeg2_istream;
    


//     static fh_istream getPgmpipeStream( Context* c, const std::string& rdn, EA_Atom* atom )
//     {
// //        string path = attr->getParent()->getDirPath();
// //     fh_runner r = new Runner();
// //     r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags()
// //                                   | G_SPAWN_STDERR_TO_DEV_NULL
// //                                   | G_SPAWN_SEARCH_PATH) );

// //     r->getArgv().push_back( "mpeg2dec" );
// //     r->getArgv().push_back( "-s" );
// //     r->getArgv().push_back( "-o" );
// //     r->getArgv().push_back( "pgmpipe" );
// //     r->getArgv().push_back( path );
    
// //     return Factory::MakePipeEA( r );

//         fh_context c = attr->getParent();
//         fh_istream iss = c->getIStream();
//         fh_mpeg2_istream oss( iss );
//         return oss;
//     }

    static fh_istream getPgmpipeExternalStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string path = c->getDirPath();
        fh_runner r = new Runner();
        r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags()
                                      | G_SPAWN_STDERR_TO_DEV_NULL
                                      | G_SPAWN_SEARCH_PATH) );
        
        r->getArgv().push_back( "mpeg2dec" );
        r->getArgv().push_back( "-s" );
        r->getArgv().push_back( "-o" );
        r->getArgv().push_back( "pgmpipe" );
        r->getArgv().push_back( path );
        
        return Factory::MakePipeEA( r );
    }
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


//     typedef struct pgm_instance_s
//     {
//         vo_instance_t vo;
//         int prediction_index;
//         vo_frame_t * frame_ptr[3];
//         vo_frame_t frame[3];
//         int width;
//         int height;
//         int framenum;
//         char header[1024];
//         char filename[128];

//         typedef Loki::SmartPtr<  basic_mpeg2_streambuf< char >,
//                                  FerrisRefCounted,
//                                  Loki::DisallowConversion,
//                                  Loki::AssertCheck,
//                                  FerrisSmartPtrStorage > ss_t;

//         ss_t ss;
        
//     } pgm_instance_t;

//     static void internal_draw_frame (pgm_instance_t* instance, vo_frame_t * frame)
//     {
//         int i;
//         pgm_instance_t::ss_t& ss = instance->ss;
//         streamsize buffer_sz     = ss->AvailableBufferSize;
//         char* buffer             = ss->AvailableBuffer;
//         fh_stringstream& OverFlowStream = ss->OverFlowStream;

//         fh_stringstream tss;

//         tss.write ((char*)instance->header, strlen (instance->header) * 1);
//         tss.write ((char*)frame->base[0], instance->width * instance->height);
//         for (i = 0; i < instance->height >> 1; i++)
//         {
//             tss.write ((char*)frame->base[1]+i*instance->width/2, instance->width/2 * 1);
//             tss.write ((char*)frame->base[2]+i*instance->width/2, instance->width/2 * 1);
//         }
        
//         string s = tostr(tss);
//         int len = s.length();
//         ss->BytesRead = len;
//         memcpy( buffer, s.c_str(), len );
        

        
// //         fwrite (instance->header, strlen (instance->header), 1, file);
// //         fwrite (frame->base[0], instance->width, instance->height, file);
// //         for (i = 0; i < instance->height >> 1; i++)
// //         {
// //             fwrite (frame->base[1]+i*instance->width/2, instance->width/2, 1, file);
// //             fwrite (frame->base[2]+i*instance->width/2, instance->width/2, 1, file);
// //         }
//     }

//     static int internal_setup (vo_instance_t * _instance, int width, int height,
//                                void (* draw_frame) (vo_frame_t *))
//     {
//         pgm_instance_t * instance;

//         instance = (pgm_instance_t *) _instance;

//         instance->vo.close = libvo_common_free_frames;
//         instance->vo.get_frame = libvo_common_get_frame;
//         instance->width = width;
//         instance->height = height;
//         sprintf (instance->header, "P5\n\n%d %d\n255\n", width, height * 3 / 2);
//         return libvo_common_alloc_frames ((vo_instance_t *) instance,
//                                           width, height, sizeof (vo_frame_t),
//                                           NULL, NULL, draw_frame);
//     }

//     static void pgmpipe_draw_frame (vo_frame_t * frame)
//     {
//         pgm_instance_t * instance;

//         instance = (pgm_instance_t *)frame->instance;
//         if (++(instance->framenum) >= 0)
//             internal_draw_frame (instance, frame);
//     }

//     static int pgmpipe_setup (vo_instance_t * instance, int width, int height)
//     {
//         return internal_setup (instance, width, height, pgmpipe_draw_frame);
//     }

//     vo_instance_t * vo_pgmpipe_open( pgm_instance_s::ss_t& ss )
//     {
//         pgm_instance_t* instance;
        
//         instance = (pgm_instance_t*)malloc (sizeof (pgm_instance_t));
//         if (instance == NULL)
//             return NULL;

//         instance->vo.setup = pgmpipe_setup;
//         instance->framenum = -2;
//         instance->ss       = ss;
//         return (vo_instance_t *) instance;
//     }


    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    static fh_istream getA52pipeStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string path = c->getDirPath();
        fh_runner r = new Runner();
        r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags()
                                      | G_SPAWN_STDERR_TO_DEV_NULL
                                      | G_SPAWN_SEARCH_PATH) );

        /* Get stream number from EA name */
        int StreamNum = 0;
        {
            string d = rdn;
            stringstream ss( d.substr( strlen(A52AttrPrefix) ));
            ss >> StreamNum;
        }
    
    
        r->getArgv().push_back( "extract_a52" );
        stringstream ss;
        ss << "-s" << StreamNum << flush;
        r->getArgv().push_back( tostr(ss) );
        r->getArgv().push_back( path );

        return Factory::MakePipeEA( r );
    }


    void
    EAGenerator_Mpeg2::Brew( const fh_context& a )
    {
        LG_MPEG2_D << "EAGenerator_Mpeg2" << endl;
    
        try
        {
            int i=0;

            a->addAttribute( "pgmpipe",
                             EA_Atom_ReadOnly::GetIStream_Func_t(getPgmpipeExternalStream),
                             FXD_BINARY_PGMPIPE );

            for( i=0; i<8; ++i )
            {
                a->addAttribute( A52AttrPrefixArray[i],
                                 EA_Atom_ReadOnly::GetIStream_Func_t(getA52pipeStream),
                                 FXD_BINARY_A52PIPE );
            }


#ifdef  HAVE_LIBMPEG2_GETRES_VO

            const string& path = a->getDirPath();
            Runner r;
            r.setSpawnFlags( GSpawnFlags(   r.getSpawnFlags()
                                            | G_SPAWN_STDERR_TO_DEV_NULL
                                            | G_SPAWN_SEARCH_PATH) );
            r.getArgv().push_back( "mpeg2dec" );
            r.getArgv().push_back( "-s" );
            r.getArgv().push_back( "-o" );
            r.getArgv().push_back( "getres" );
            r.getArgv().push_back( path );

            r();
            fh_istream iss = r.getStdOut();
            int w=0,h=0;
            iss >> w >> h;

//            cerr << "w:" << w << " h:" << h << endl;
            a->addAttribute( "width",  tostr(w), FXD_WIDTH_PIXELS );
            a->addAttribute( "height", tostr(h), FXD_HEIGHT_PIXELS );
#endif
        }
        catch( exception& e )
        {
            LG_MPEG2_W << "Failed to load mpeg2 EA, error:" << e.what() << endl;
        }
    }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            return new EAGenerator_Mpeg2();
        }
    };
 
};
