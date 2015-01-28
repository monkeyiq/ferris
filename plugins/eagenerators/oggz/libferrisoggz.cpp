/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2009 Ben Martin

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

    $Id: libferrisoggz.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>

#include <oggz/oggz.h>

using namespace std;

namespace Ferris
{
    typedef class EA_Atom_Static OggzByteArrayAttribute;
    typedef class EA_Atom_Static OggzByteArrayAttributeSchema;
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    string CanonEAName( const std::string& s )
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

    class FERRISEXP_DLLLOCAL EAGenerator_Oggz : public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Oggz()
            :
            MatchedEAGeneratorFactory()
            {
            }

        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };


static int
read_comments(OGGZ *oggz, oggz_packet *oggzp, long serialno, void *user_data)
{
    stringmap_t* kvsp = (stringmap_t*)user_data;
    stringmap_t& kvs = *kvsp;
    const OggzComment * comment;
    const char * codec_name;

    if ( oggzp->op.packetno == 1)
    {
        LG_OGGZ_D << "read_comments() oggz:" << oggz << endl;
        LG_OGGZ_D << "pack one..." << endl;

        codec_name = oggz_stream_get_content_type(oggz, serialno);
        if (codec_name)
            kvs["codec"] = codec_name;
        kvs["serialno"] = tostr(serialno);

        for (comment = oggz_comment_first(oggz, serialno); comment;
             comment = oggz_comment_next(oggz, serialno, comment))
        {
            kvs[ comment->name ] = comment->value;
        }
        return OGGZ_STOP_OK;
    }

    return OGGZ_CONTINUE;
}

    void
    EAGenerator_Oggz::Brew( const fh_context& a )
    {
        LG_OGGZ_D << "EAGenerator_Oggz::Brew() url:" << a->getURL() << endl;

        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        stringmap_t kvs;
        int rc = 0;
        string fname = a->getDirPath();
        OGGZ* m_oggz = oggz_open( fname.c_str(), OGGZ_READ );
        LG_OGGZ_D << "EAGenerator_Oggz::Brew() oggz:" << toVoid(m_oggz) << endl;

        oggz_set_read_callback (m_oggz, -1, read_comments, &kvs );
        LG_OGGZ_D << "EAGenerator_Oggz::Brew(3)" << endl;

        rc = oggz_run (m_oggz);
        LG_OGGZ_D << "EAGenerator_Oggz::Brew(4) rc:" << rc << endl;
        if( !rc || rc==OGGZ_ERR_STOP_OK )
            LG_OGGZ_D << "READ WAS OK!" << endl;
        
        rc = oggz_flush( m_oggz );
        rc = oggz_close( m_oggz );

        for( stringmap_t::iterator it = kvs.begin(); it != kvs.end(); ++it )
        {
            string k = it->first;
            string v = it->second;
            LG_OGGZ_D << "k:" << k << " v:" << v << endl;
            
            string rdn = CanonEAName( k );
            a->addAttribute( rdn, v, XSD_BASIC_STRING );
            if( rdn == "tracknumber" )
            {
                a->addAttribute( "track", v, XSD_BASIC_INT );
            }
            else if( rdn == "cddb" )
            {
                a->addAttribute( "discid", v, XSD_BASIC_STRING );
            }
            
            
        }
        LG_OGGZ_D << "Brew() " << " url:" << a->getURL() << " complete" << endl;
    }

    bool
    EAGenerator_Oggz::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        LG_OGGZ_D << "tryBrew() " << " url:" << ctx->getURL() << endl;

        Brew( ctx );
        return ctx->isAttributeBound( eaname, false );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            LG_OGGZ_D << "EAGenerator_Oggz::CreateRealFactory()" << endl;
            return new EAGenerator_Oggz();
        }
    };
};
