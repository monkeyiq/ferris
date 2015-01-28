/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2010 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrismediainfo.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#define UNICODE     
#define _UNICODE

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>
#include <Ferris/General.hh>

#include <MediaInfo/MediaInfo.h>

using namespace std;
using namespace MediaInfoLib;
using Ferris::Util::utf8_to_wstring;
using Ferris::Util::wstring_to_utf8;

#define DEBUG LG_LIBMEDIAINFO_D

namespace Ferris
{
    typedef class EA_Atom_Static OggzByteArrayAttribute;
    typedef class EA_Atom_Static OggzByteArrayAttributeSchema;
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    static string CanonEAName( const std::string& s )
    {
        string ret = tolowerstr()( s );
        return ret;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL EAGenerator_Mediainfo : public MatchedEAGeneratorFactory
    {
        typedef EAGenerator_Mediainfo _Self;
        
    protected:

        virtual void Brew( const fh_context& a );
        void getData( stringmap_t& kvs,
                      MediaInfo& MI,
                      std::string eaname,
                      ::MediaInfoLib::stream_t streamType,
                      std::string mediainfoAttribute );

    public:

        // fh_istream get_aspect_ratio( Context* c, const std::string& eaname, EA_Atom* atom )
        // {
        //     fh_stringstream ss;
        //     double w = toType<double>(getStrAttr( c, "width",  "0" ));
        //     double h = toType<double>(getStrAttr( c, "height", "0" ));
        //     double v = w / std::max( 1.0, h );
        //     ss << v;
        //     DEBUG << "get_aspect_ratio() v:" << v << endl;
        //     return ss;
        // }

        fh_istream get_has_video( Context* c, const std::string& eaname, EA_Atom* atom )
        {
            fh_stringstream ss;
            int count = toType<int>(getStrAttr( c, "video-stream-count",  "0" ));
            ss << (count>0);
            DEBUG << "get_has_video() ret:" << (count>0) << endl;
            return ss;
        }
        fh_istream get_has_audio( Context* c, const std::string& eaname, EA_Atom* atom )
        {
            fh_stringstream ss;
            int count = toType<int>(getStrAttr( c, "audio-stream-count",  "0" ));
            ss << (count>0);
            DEBUG << "get_has_audio() ret:" << (count>0) << endl;
            return ss;
        }
        fh_istream get_has_subtitles( Context* c, const std::string& eaname, EA_Atom* atom )
        {
            fh_stringstream ss;
            string lang = getStrAttr( c, "subtitle-language",  "" );
            bool v = !lang.empty();
            ss << v;
            DEBUG << "get_has_subtitles() ret:" << v << endl;
            return ss;
        }
        
        
        
        EAGenerator_Mediainfo()
            :
            MatchedEAGeneratorFactory()
            {
            }

        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };

    void
    EAGenerator_Mediainfo::getData( stringmap_t& kvs,
                                    MediaInfo& MI,
                                    std::string eaname,
                                    ::MediaInfoLib::stream_t streamType,
                                    std::string mediainfoAttribute )
    {
        std::wstring value = MI.Get( streamType,
                                     0,
                                     utf8_to_wstring( mediainfoAttribute ),
                                     Info_Text,
                                     Info_Name).c_str();
        kvs[ eaname ] = wstring_to_utf8( value );
    }

    
    void
    EAGenerator_Mediainfo::Brew( const fh_context& a )
    {
        DEBUG << "EAGenerator_Mediainfo::Brew() url:" << a->getURL() << endl;

        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        stringmap_t kvs;
        MediaInfo MI;
        MI.Open( utf8_to_wstring( a->getDirPath().c_str()) );

        getData( kvs, MI, "width-local",  Stream_Video, "Width" );
        getData( kvs, MI, "height-local", Stream_Video, "Height" );
        getData( kvs, MI, "h",         Stream_Video, "Height" );
        getData( kvs, MI, "bitrate",   Stream_General, "BitRate" );
        getData( kvs, MI, "title",     Stream_General, "Movie" );
        getData( kvs, MI, "format",    Stream_General, "Format" );
        getData( kvs, MI, "duration",  Stream_General, "Duration" );
        getData( kvs, MI, "written-by", Stream_General, "Encoded_Application" );
        getData( kvs, MI, "written-by-library", Stream_General, "Encoded_Library" );

        getData( kvs, MI, "video-stream-count", Stream_Video, "StreamCount" );
        getData( kvs, MI, "video-bitrate",      Stream_Video, "BitRate" );
        getData( kvs, MI, "video-format",       Stream_Video, "Format" );
        getData( kvs, MI, "video-profile",      Stream_Video, "Format_Profile" );
        getData( kvs, MI, ".video-format",       Stream_Video, "Format" );
        getData( kvs, MI, ".video-profile",      Stream_Video, "Format_Profile" );
        getData( kvs, MI, "video-language",     Stream_Video, "Language" );

        getData( kvs, MI, "audio-stream-count", Stream_Audio, "StreamCount" );
        getData( kvs, MI, "audio-bitrate",      Stream_Audio, "BitRate" );
        getData( kvs, MI, "audio-format",       Stream_Audio, "Format" );
        getData( kvs, MI, "audio-profile",      Stream_Audio, "Format_Profile" );
        getData( kvs, MI, ".audio-format",       Stream_Audio, "Format" );
        getData( kvs, MI, ".audio-profile",      Stream_Audio, "Format_Profile" );
        getData( kvs, MI, "audio-language",     Stream_Audio, "Language" );
        
        getData( kvs, MI, "subtitle-language",     Stream_Text,  "Language" );

//        a->addAttribute( "aspect-ratio",  this, &_Self::get_aspect_ratio,  XSD_BASIC_DOUBLE );
        a->addAttribute( "has-video",     this, &_Self::get_has_video,     XSD_BASIC_BOOL );
        a->addAttribute( "has-audio",     this, &_Self::get_has_audio,     XSD_BASIC_BOOL );
        a->addAttribute( "has-subtitles", this, &_Self::get_has_subtitles, XSD_BASIC_BOOL );
        

        if( kvs["width-local"].empty() )
            kvs["width-local"] = "0";
        if( kvs["height-local"].empty() )
            kvs["height-local"] = "0";
        
        for( stringmap_t::iterator it = kvs.begin(); it != kvs.end(); ++it )
        {
            string k = it->first;
            string v = it->second;
            DEBUG << "k:" << k << " v:" << v << endl;
            
//            string rdn = CanonEAName( k );
            string rdn = k;
            
            // set schema based on regex on v 

            a->addAttribute( rdn, v, XSD_BASIC_STRING );
        }
        DEBUG << "Brew() " << " url:" << a->getURL() << " complete" << endl;
    }

    bool
    EAGenerator_Mediainfo::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        DEBUG << "tryBrew() " << " url:" << ctx->getURL() << endl;

        Brew( ctx );
        return ctx->isAttributeBound( eaname, false );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            DEBUG << "EAGenerator_Mediainfo::CreateRealFactory()" << endl;
            return new EAGenerator_Mediainfo();
        }
    };
};
