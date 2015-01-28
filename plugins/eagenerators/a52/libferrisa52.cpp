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

    $Id: libferrisa52.cpp,v 1.2 2010/09/24 21:31:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

#include <inttypes.h>
extern "C" {
#include <a52dec/a52.h>
#include <a52dec/mm_accel.h>
};


using namespace std;
namespace Ferris
{


class FERRISEXP_DLLLOCAL EAGenerator_A52 : public MatchedEAGeneratorFactory
{
protected:
    virtual void Brew( const fh_context& a );
public:
    EAGenerator_A52();

    void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
        {
            ss << ",a52-has-base,a52-channels";
        }
};


    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


EAGenerator_A52::EAGenerator_A52()
    :
    MatchedEAGeneratorFactory()
{
}


void
EAGenerator_A52::Brew( const fh_context& a )
{
    LG_A52_D << "EAGenerator_A52" << endl;
    
    try
    {
        static a52_state_t* a52State = a52_init( 0 ); //mm_accel() );

        fh_istream iss;

        /* Allow for operation on a52 within mpg */
        if( a->isAttributeBound("a52-raw-0") )
        {
            fh_attribute a52 = a->getAttribute("a52-raw-0");
            iss = a52->getIStream();
        }
        else
        {
            iss = a->getIStream();
        }
        
        const int minsz = 7;
        uint8_t buf[minsz+1];

        iss.read( (char*)buf, minsz );
        if( minsz != iss.gcount() )
        {
            /* Need minsz bytes to enter the game */
            return;
        }
        
        
        int flags       = 0;
        int sample_rate = 0;
        int bit_rate    = 0;
        
        int rc = a52_syncinfo( buf, &flags, &sample_rate, &bit_rate );

        if( !rc )
        {
            /* not a valid a52 frame */
            return;
        }
        

        bool hasBaseChannel = flags & A52_LFE;
        int  FrameLength    = rc;
        int  ChMask         = flags & A52_CHANNEL_MASK;
        int  Channels       = 0;
        int  FrontChannels  = 0;
        int  RearChannels   = 0;
        

        switch( ChMask )
            {
            case A52_CHANNEL:    Channels = 2; break;
            case A52_MONO:       Channels = 1; break;
            case A52_STEREO:     Channels = 2; break;
            case A52_3F:         Channels = 3; break;
            case A52_2F1R:       Channels = 3; RearChannels = 1; break;
            case A52_3F1R:       Channels = 4; RearChannels = 1; break;
            case A52_2F2R:       Channels = 4; RearChannels = 2; break;
            case A52_3F2R:       Channels = 5; RearChannels = 2; break;
            case A52_CHANNEL1:   Channels = 1; break;
            case A52_CHANNEL2:   Channels = 1; break;
            case A52_DOLBY:      Channels = 2; break;
            }

        FrontChannels = Channels - RearChannels;
        if( hasBaseChannel )
        {
            ++Channels;
        }

        a->addAttribute( "a52-bit-rate", tostr(bit_rate),       XSD_BASIC_INT  );
        a->addAttribute( "a52-sample-rate", tostr(sample_rate), XSD_BASIC_INT  );
        a->addAttribute( "a52-has-base", tostr(hasBaseChannel), XSD_BASIC_BOOL );
        a->addAttribute( "a52-frame-length", tostr(FrameLength),XSD_BASIC_INT  );
        a->addAttribute( "a52-channels", tostr(Channels),       XSD_BASIC_INT  );
        a->addAttribute( "a52-channels-front", tostr(FrontChannels), XSD_BASIC_INT );
        a->addAttribute( "a52-channels-rear", tostr(RearChannels),   XSD_BASIC_INT );
    }
    catch( exception& e )
    {
        LG_A52_W << "Failed to load A52 EA, error:" << e.what() << endl;
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        return new EAGenerator_A52();
    }
};
 
};
