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

    $Id: libmpeg3.cpp,v 1.3 2010/09/24 21:31:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>

#include <iomanip.h>
#include <errno.h>

#include <libmpeg3.h>
#define NUMBER_OF_VIDEO_DECOMPRESSION_CPUS  1
#define USE_MMX_FOR_VIDEO                   1

using namespace std;
namespace Ferris
{
    



extern "C"
{
    FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_CTXPLUGIN MPGContext : public Context
{
    mpeg3_t* Mpg;

    friend class MPGVFS_RootContextDropper;
    
    Context* priv_CreateContext( Context* parent, string rdn );
    void ExposeFrame( MPGContext* parent, int stream, int frame );
    

protected:

    virtual void priv_read();
    
public:

    MPGContext();
    ~MPGContext();

    mpeg3_t* mpg()
        {
            return Mpg;
        }
        
    
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_CTXPLUGIN MPGFrameContext : public Context
{
    MPGContext* Mpgctx;
    int Stream;
    int Frame;
    unsigned char* fb;
    unsigned char** output_rows;

    mpeg3_t* mpg()
        {
            // ASSERT
            if( !Mpgctx->mpg() )
            {
                LG_MPG_ER << "MPGFrameContext is disconnected!" << endl;
            }
            
            return Mpgctx->mpg();
        }

protected:

     virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
         throw (FerrisParentNotSetError,
                CanNotGetStream,
                exception);

    Context*
    priv_CreateContext( Context* parent, string rdn )
        {
            MPGFrameContext* ret = 0; //new MPGFrameContext();
//             ret->setContext( parent, rdn );
            return ret;
        }

    
public:

    MPGFrameContext(
        MPGContext* parent,
        const string& rdn,
        MPGContext* mpgctx, int stream, int frame );
    
};

MPGFrameContext::MPGFrameContext(
    MPGContext* parent,
    const string& rdn,
    MPGContext* mpgctx, int stream, int frame )
    :
    Mpgctx(mpgctx),
    Stream(stream),
    Frame(frame)
{
    setContext( parent, rdn );
}


fh_istream
MPGFrameContext::priv_getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
//     if(mpeg3_set_frame( mpg(), Frame, Stream))
//     {
//         ostringstream ss;
//         ss << "Can not seek in the mpg file frame:" << Frame << endl;
//         Throw_CanNotGetStream( tostr(ss), this );
//     }
    
 
    int ix = 0;
    int iy = 0;
    const int iw = mpeg3_video_width(  mpg(), Stream );
    const int ih = mpeg3_video_height( mpg(), Stream );
    int ow = iw;
    int oh = ih;
    int obytespp = 3;


    fb = new (unsigned char)[ ow * oh * obytespp + 4 ];
    output_rows = new (unsigned char*) [oh];
    
    for( int i=0; i < oh; ++i )
    {
        output_rows[i] = &fb[ i * ow * obytespp ];
    }
    
    if(mpeg3_read_frame( mpg(), output_rows,
                         ix, iy, iw, ih, ow, oh, MPEG3_RGB888, Stream ))
    {
        LG_MPG_ER << "Error reading the frame:" << endl;
    }


    ostringstream ss;
    ss << "/store/test/ferris-frame-" << Stream << "-" << setw(5) << setfill('0') << Frame;
    cout << "Making frame:" << tostr(ss) << endl;
    ofstream os( tostr(ss).c_str() );
    os.write( (char*)fb, ow * oh * obytespp );
    

    
    fh_istream ret = Factory::MakeMemoryIStream( fb, ow * oh * obytespp );
    return ret;
}



Context*
MPGContext::priv_CreateContext( Context* parent, string rdn )
{
    MPGContext* ret = new MPGContext();
    ret->setContext( parent, rdn );
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




MPGContext::MPGContext()
{
}

MPGContext::~MPGContext()
{
}



void
MPGContext::priv_read()
{
    LG_MPG_I << endl;

    emitExistsEventForEachItemRAII _raii1( this );
    clearContext();

    LG_MPG_I << "path:" << getDirPath() << endl;
    Mpg = mpeg3_open( (char *)getDirPath().c_str());
    
    if(!mpg())
    {
        stringstream ss;
        ss << "libmpeg3 open failed for file:" << getDirPath();
        Throw_FerrisStreamLoadFailed( tostr(ss), 0 );
    }
    
    mpeg3_set_cpus( mpg(), NUMBER_OF_VIDEO_DECOMPRESSION_CPUS );
    mpeg3_set_mmx( mpg(), USE_MMX_FOR_VIDEO );

    int stream = 0;
    long fc = mpeg3_video_frames( mpg(), stream );

//    mpeg3_seek_percentage( mpg(), 5.0 );


//     if(mpeg3_set_frame( mpg(), 100, stream))
//     {
//         ostringstream ss;
//         ss << "Can not seek in the mpg file frame:" << 100 << endl;
//         Throw_FerrisStreamLoadFailed( tostr(ss), this );
//     }
    
    // XXX
//    fc = 200;
    
    for( long i=0; i<fc; ++i )
    {
        ExposeFrame( this, stream, i );
    }
    

    
    LG_MPG_I << "edbContext::priv_read(done) path:" << getDirPath() << endl;
}





void
MPGContext::ExposeFrame(
    MPGContext* parent,
    int stream,
    int frame
    )
{
//     int ix = 0;
//     int iy = 0;
//     const int iw = mpeg3_video_width(  mpg, stream );
//     const int ih = mpeg3_video_height( mpg, stream );
//     int ow = iw;
//     int oh = ih;
//     int obytespp = 3;


//     fb = new (unsigned char)[ ow * oh * obytespp + 4 ];
//     output_rows = new (unsigned char*) [oh];
    
//     for( int i=0; i < oh; ++i )
//     {
//         output_rows[i] = &fb[ i * ow * obytespp ];
//     }
    
//     if(mpeg3_read_frame( mpg, output_rows,
//                          ix, iy, iw, ih, ow, oh, MPEG3_RGB888, stream ))
//     {
//         LG_MPG_ER << "Error reading the frame:" << endl;
//     }


    ostringstream ss;
    ss << "frame-" << frame;
//    Insert( CreateContext( parent, tostr(ss) ) );

    MPGFrameContext* c = new MPGFrameContext( parent, tostr(ss), this, stream, frame );
    Insert( c );

    fh_istream tmp = c->getIStream();
    
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static MPGContext c;
        return c.CreateContext( 0, rf->getInfo( "Root" ));
    }
}
 
};
