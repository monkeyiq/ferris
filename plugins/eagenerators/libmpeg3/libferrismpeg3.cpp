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

    $Id: libferrismpeg3.cpp,v 1.2 2010/09/24 21:31:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <libmpeg3.h>

#define NUMBER_OF_VIDEO_DECOMPRESSION_CPUS  1
#define USE_MMX_FOR_VIDEO                   1

using namespace std;
namespace Ferris
{
    



class FERRISEXP_DLLLOCAL ImageDataEAGenerator_MPG : public MatchedEAGeneratorFactory
{

    mpeg3_t* mpg;


    void make_channels_ea( const fh_context& a, int i, int channels );
    void make_rate_ea( const fh_context& a, int i, int rate );
    void make_samples_ea( const fh_context& a, int i, long samples );
    void make_pcm_ea( const fh_context& a, int i, int channel );

    void make_vw_ea( const fh_context& a, int i, int w );
    void make_vh_ea( const fh_context& a, int i, int h );
    void make_vr_ea( const fh_context& a, int i, float r );
    void make_vf_ea( const fh_context& a, int i, long f );

    int has_audio();
    int audio_stream_count();
    int has_video();
    int video_stream_count();

    fh_istream getHasAudioStream( Context*c, const std::string& rdn, EA_Atom* atom );
    fh_istream getAudioStreamCountStream( Context*c, const std::string& rdn, EA_Atom* atom );
    fh_istream getHasVideoStream( Context*c, const std::string& rdn, EA_Atom* atom );
    fh_istream getVideoStreamCountStream( Context*c, const std::string& rdn, EA_Atom* atom );

protected:

    virtual void Brew( const fh_context& a );

public:

    ImageDataEAGenerator_MPG();
    ~ImageDataEAGenerator_MPG();
    
    inline mpeg3_t* getMPG()
        {
            return mpg;
        }
    
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


ImageDataEAGenerator_MPG::ImageDataEAGenerator_MPG()
    :
    MatchedEAGeneratorFactory()
{
}

ImageDataEAGenerator_MPG::~ImageDataEAGenerator_MPG()
{
    mpeg3_close( mpg );
}


int ImageDataEAGenerator_MPG::has_audio()
{
    return mpeg3_has_audio( mpg );
}

int ImageDataEAGenerator_MPG::audio_stream_count()
{
    return mpeg3_total_astreams( mpg );
}

int ImageDataEAGenerator_MPG::has_video()
{
    return mpeg3_has_video( mpg );
}

int ImageDataEAGenerator_MPG::video_stream_count()
{
    return mpeg3_total_vstreams( mpg );
}


fh_istream
ImageDataEAGenerator_MPG::getHasAudioStream( Context*c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << mpeg3_has_audio( mpg );
    return ss;
}

fh_istream
ImageDataEAGenerator_MPG::getAudioStreamCountStream( Context*c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << mpeg3_total_astreams( mpg );
    return ss;
}
    
fh_istream
ImageDataEAGenerator_MPG::getHasVideoStream( Context*c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << mpeg3_has_video( mpg );
    return ss;
}
    
fh_istream
ImageDataEAGenerator_MPG::getVideoStreamCountStream( Context*c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    ss << mpeg3_total_vstreams( mpg );
    return ss;
}
    


void
ImageDataEAGenerator_MPG::Brew( const fh_context& a )
{
    LG_ID3_D << "ImageDataEAGenerator_MPG" << endl;
    
    try
    {
        string dn = a->getDirPath();
        fh_istream iss = a->getLocalIStream( dn );
        LG_ID3_D << "dn:" << dn << endl;

        if( !mpeg3_check_sig( (char*)dn.c_str() ))
        {
            return;
//             stringstream ss;
//             ss << "Can not be loaded using libmpeg3!";
//             Throw_FerrisStreamLoadFailed( tostr(ss), 0 );
        }
        

        mpg = mpeg3_open( (char *)dn.c_str());

        if(!mpg)
        {
            stringstream ss;
            ss << "libmpeg3 open failed for file:" << dn;
            Throw_FerrisStreamLoadFailed( tostr(ss), 0 );
        }
        
        mpeg3_set_cpus( mpg, NUMBER_OF_VIDEO_DECOMPRESSION_CPUS );
        mpeg3_set_mmx( mpg, USE_MMX_FOR_VIDEO );

        a->addAttribute( "has-audio",
                         EA_Atom_ReadOnly::GetIStream_Func_t(
                             this,
                             &ImageDataEAGenerator_MPG::getHasAudioStream ),
                         XSD_BASIC_BOOL );

        a->addAttribute( "audio-stream-count",
                         EA_Atom_ReadOnly::GetIStream_Func_t(
                             this,
                             &ImageDataEAGenerator_MPG::getAudioStreamCountStream ),
                         XSD_BASIC_INT );

        for( int i=0; i<audio_stream_count(); ++i )
        {
            int  channels = mpeg3_audio_channels( mpg, i );
            int  rate     = mpeg3_sample_rate(    mpg, i );
            long samples  = mpeg3_audio_samples(  mpg, i );

            make_channels_ea( a, i, channels );
            make_rate_ea(     a, i, rate     );
            make_samples_ea(  a, i, samples  );

            for( int ch = 0; ch < channels; ++ch )
            {
                make_pcm_ea( a, i, ch );
            }
        }


        /*
         * Expose video EA
         */
        
        a->addAttribute(
            "has-video",
            EA_Atom_ReadOnly::GetIStream_Func_t(
                this,
                &ImageDataEAGenerator_MPG::getHasVideoStream ),
            XSD_BASIC_BOOL );

        a->addAttribute(
            "video-stream-count",
            EA_Atom_ReadOnly::GetIStream_Func_t(
                this,
                &ImageDataEAGenerator_MPG::getVideoStreamCountStream ),
            XSD_BASIC_INT );
        

        for( int i=0; i<video_stream_count(); ++i )
        {
            int   w = mpeg3_video_width(  mpg, i);
            int   h = mpeg3_video_height( mpg, i);
            float r = mpeg3_frame_rate(   mpg, i);
            long  f = mpeg3_video_frames( mpg, i);

            make_vw_ea( a, i, w );
            make_vh_ea( a, i, h );
            make_vr_ea( a, i, r );
            make_vf_ea( a, i, f );

        }


        
  

    }
    catch( exception& e )
    {
        LG_ID3_W << "Failed to load ID3 EA, error:" << e.what() << endl;
        cerr     << "Failed to load ID3 EA, error:" << e.what() << endl;
    }
}


template <class CharT>
string tostr( const CharT& t )
{
    ostringstream ss;
    ss << t;
    return tostr(ss);
}






void
ImageDataEAGenerator_MPG::make_channels_ea(
    const fh_context& a, int i, int channels )
{
    ostringstream ss;
    ss << "audio-stream-" << i << "-channel-count";

    a->addAttribute( tostr(ss), tostr(channels), XSD_BASIC_INT );
}


void
ImageDataEAGenerator_MPG::make_rate_ea(
    const fh_context& a, int i, int rate )
{
    ostringstream ss;
    ss << "audio-stream-" << i << "-rate";
    
    a->addAttribute( tostr(ss), tostr(rate), XSD_BASIC_INT );
}

void
ImageDataEAGenerator_MPG::make_samples_ea(
    const fh_context& a, int i, long samples )
{
    ostringstream ss;
    ss << "audio-stream-" << i << "-samples";
    
    a->addAttribute( tostr(ss), tostr(samples), XSD_BASIC_INT );
}


class FERRISEXP_DLLLOCAL MPG_PCM
{
protected:

    int Stream;
    int Channel;
    ImageDataEAGenerator_MPG* MP;

    
    mpeg3_t* getMPG() 
        {
            return MP->getMPG();
        }


    MPG_PCM(
        ImageDataEAGenerator_MPG* mp,
        int _stream,
        int _channel
        )
        :
        MP(mp),
        Stream(_stream),
        Channel(_channel)
        {
        }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


template<
    class _CharT,
    class _Traits = std::char_traits < _CharT >,
    class _Alloc  = std::allocator   < _CharT >,
    class _BufferSizers = ferris_basic_streambuf_fourk
    >
class MPG_PCM_Streambuf
    :
    public ferris_basic_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
{

    typedef MPG_PCM_Streambuf<_CharT, _Traits, _Alloc, _BufferSizers> _Self;
    typedef std::basic_string<_CharT, _Traits, _Alloc> _String;
    
public:

    typedef char_traits<_CharT>    traits_type;
    typedef traits_type::int_type  int_type;
    typedef traits_type::char_type char_type;
    typedef traits_type::pos_type  pos_type;
    typedef traits_type::off_type  off_type;


    explicit
    MPG_PCM_Streambuf(
        mpeg3_t* _mpg,
        int _stream,
        int _channel )
        :
        mpg( _mpg ),
        stream( _stream ),
        channel( _channel )
        {
        }

    virtual ~MPG_PCM_Streambuf()
        {
        }

private:
    
    // prohibit copy/assign
    MPG_PCM_Streambuf( const MPG_PCM_Streambuf& );
    MPG_PCM_Streambuf operator=( const MPG_PCM_Streambuf& );

    mpeg3_t* mpg;
    int stream;
    int channel;
    
protected:
    
    /*
     * This is the only methods that really needs to be here. It gets
     * up to maxsz data into buffer and returns how much data was really
     * read.
     */
    int make_new_data_avail( char_type* buffer, streamsize maxsz )
        {
            // no odd amounts
            if( maxsz & 0x1 )
                --maxsz;
            
            int rc = mpeg3_read_audio( mpg, 0, (short*)buffer, channel,
                                       maxsz / sizeof(short),  stream );
            if( rc == 1 )
            {
                return 0;
            }
            return maxsz;
        }

};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_DLLLOCAL MPG_PCM_IStream :
    public MPG_PCM,
    public f_istream
{
    typedef MPG_PCM_IStream _Self;
    
    
    typedef MPG_PCM_Streambuf< char > ss_impl_t;
    FERRIS_SMARTPTR( ss_impl_t, ss_t );
    ss_t ss;
    
public:

    typedef char                   _CharT;
    typedef char_traits<char>      _Traits;
    
    typedef char_traits<_CharT>    traits_type;
    typedef traits_type::int_type  int_type;
    typedef traits_type::char_type char_type;
    typedef traits_type::pos_type  pos_type;
    typedef traits_type::off_type  off_type;

    typedef emptystream_methods< char_type, traits_type > delegating_methods;

    explicit
    MPG_PCM_IStream(
        ImageDataEAGenerator_MPG* mp,
        int _stream,
        int _channel
        )
        :
        MPG_PCM( mp, _stream, _channel ),
        ss( new ss_impl_t( mp->getMPG(), _stream, _channel ) )
        {
            init( rdbuf() );
            setsbT( GetImpl(ss) );
        }
    
    MPG_PCM_IStream( const MPG_PCM_IStream& rhs )
        :
        MPG_PCM( MP, Stream, Channel ),
        ss( rhs.ss )
        {
            init( rdbuf() );
            setsbT( GetImpl(ss) );
        }
    
    virtual ~MPG_PCM_IStream()
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


class FERRISEXP_DLLLOCAL MPG_PCM_StreamingAttribute :
    public MPG_PCM,
    public EA_Atom
{
protected:

    virtual fh_istream getIStream( Context* c,
                                   const std::string& rdn,
                                   ferris_ios::openmode m = ios::in )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
        {
            return MPG_PCM_IStream( MP, Stream, Channel );
        }

public:

    MPG_PCM_StreamingAttribute(
        ImageDataEAGenerator_MPG* mp,
        int _stream,
        int _channel
        )
        :
        EA_Atom(),
        MPG_PCM( mp, _stream, _channel )
        {
        }

};



void
ImageDataEAGenerator_MPG::make_pcm_ea(
    const fh_context& a, int i, int channel )
{
    ostringstream ss;
    ss << "audio-stream-" << i << "-pcm-" << channel;

    a->addAttribute( tostr(ss),
                     (EA_Atom*)new MPG_PCM_StreamingAttribute( this, i, channel ),
                     FXD_BINARY );
    
    
//     ret.push_back( new MPG_PCM_StreamingAttribute(
//                        a, tostr(ss), this, i, channel ));
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
ImageDataEAGenerator_MPG::make_vw_ea(
    const fh_context& a, int i, int w )
{
    ostringstream ss;
    ss << "video-stream-" << i << "-width";
    
    a->addAttribute( tostr(ss), tostr(w), FXD_WIDTH_PIXELS );

    if( !i )
    {
        a->addAttribute( "width", tostr(w), FXD_WIDTH_PIXELS );
    }
    
}

void
ImageDataEAGenerator_MPG::make_vh_ea(
    const fh_context& a, int i, int h )
{
    ostringstream ss;
    ss << "video-stream-" << i << "-height";
    
    a->addAttribute( tostr(ss), tostr(h), FXD_HEIGHT_PIXELS );

    if( !i )
    {
        a->addAttribute( "height", tostr(h), FXD_HEIGHT_PIXELS );
    }
    
        
}

void
ImageDataEAGenerator_MPG::make_vr_ea(
    const fh_context& a, int i, float r )
{
    ostringstream ss;
    ss << "video-stream-" << i << "-rate";
    
    a->addAttribute( tostr(ss), tostr(r), XSD_BASIC_INT );

    if( !i )
    {
        a->addAttribute( "rate", tostr(r), XSD_BASIC_INT );
    }
    
        
}


void
ImageDataEAGenerator_MPG::make_vf_ea(
    const fh_context& a, int i, long f )
{
    ostringstream ss;
    ss << "video-stream-" << i << "-frame-count";
    
    a->addAttribute( tostr(ss), tostr(f), XSD_BASIC_INT );

    if( !i )
    {
        a->addAttribute( "frame-count", tostr(f), XSD_BASIC_INT );
    }
    
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        return new ImageDataEAGenerator_MPG();
    }
};




 
};
