/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2007 Ben Martin

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

    $Id: libferrisflac.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <cassert>
#include "/usr/include/assert.h"

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>

#include <FLAC++/metadata.h>

using namespace std;
using namespace FLAC::Metadata;

namespace Ferris
{
    typedef class EA_Atom_Static FlacByteArrayAttribute;
    typedef class EA_Atom_Static FlacByteArrayAttributeSchema;
    
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

    void set_tags( const std::string& fname,  VorbisComment& tags )
    {
        FLAC::Metadata::SimpleIterator it;
        if (it.is_valid()) {
            if (it.init( fname.c_str(), false, true)) {
                FLAC::Metadata::VorbisComment *vc = 0;
                do {
                    if (it.get_block_type() ==
                        ::FLAC__METADATA_TYPE_VORBIS_COMMENT) {
                        vc =
                            dynamic_cast<FLAC::Metadata::VorbisComment*>(it.get_block());

                        it.set_block( &tags );
                    }
                } while (!vc && it.next());
                //use vc
                delete vc;
            }
        }
        
    }

    // returns false on error
    bool ensure_comment( VorbisComment& tags, VorbisComment::Entry& ne )
    {
        for( int i=0; i<tags.get_num_comments(); ++i )
        {
            VorbisComment::Entry e = tags.get_comment( i );
            string n = e.get_field_name();
            if( n == ne.get_field_name() )
            {
                cerr << "have existing comment..." << endl;
                tags.set_comment( i, ne );
                return true;
            }
        }
        tags.append_comment( ne );
        return true;
    }

    // returns false on error
    bool ensure_comment( const std::string& fname, const std::string& k, const std::string& v )
    {
        VorbisComment tags;
        bool rc = get_tags( fname.c_str(), tags );
        if( !rc )
            return false;
        
        VorbisComment::Entry t( k.c_str(), v.c_str() );
        ensure_comment( tags, t );
        set_tags( fname,  tags );
        return true;
    }
    
    
    class FERRISEXP_DLLLOCAL FlacAttribute
        :
        public EA_Atom_ReadWrite
    {
        std::string m_key;
        
    public:

        fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
//                ss << m_value;

                string fname = c->getDirPath();
                VorbisComment tags;
                bool rc = get_tags( fname.c_str(), tags );
                for( int i=0; i<tags.get_num_comments(); ++i )
                {
                    VorbisComment::Entry e = tags.get_comment( i );
                    string k = e.get_field_name();
                    string v = e.get_field_value() ? e.get_field_value() : "";
                    if( k == m_key )
                    {
                        ss << v;
                    }
                }
                return ss;
            }

        void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream datass )
            {
                string data = StreamToString( datass );

                string fname = c->getDirPath();
                VorbisComment tags;
                bool rc = get_tags( fname.c_str(), tags );
                for( int i=0; i<tags.get_num_comments(); ++i )
                {
                    VorbisComment::Entry e = tags.get_comment( i );
                    string k = e.get_field_name();
                    string v = e.get_field_value() ? e.get_field_value() : "";
                    if( k == m_key )
                    {
                        VorbisComment::Entry ne( k.c_str(), data.c_str() );
                        tags.set_comment( i, ne );
                        break;
                    }
                }
                set_tags( fname, tags );
            }

        FlacAttribute( const std::string& k )
            :
            EA_Atom_ReadWrite( this, &FlacAttribute::getStream,
                               this, &FlacAttribute::getStream,
                               this, &FlacAttribute::setStream )
            ,m_key( k )
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
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL EAGenerator_Flac : public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Flac()
            :
            MatchedEAGeneratorFactory()
            {
            }

        bool
        supportsCreateForContext( fh_context c )
            {
                LG_FLAC_D << "supportsCreateForContext() c:" << c->getURL() << endl;
        
                if( !c->isParentBound() )
                    return false;

                int rc = access( c->getDirPath().c_str(), W_OK );
                return rc == 0;
            }
        
        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 )
            {
                LG_FLAC_D << "CreateAttr() rdn:" << rdn << endl;
                
                bool rc = ensure_comment( a->getDirPath(), rdn, "" );
                if( !rc )
                {
                    fh_stringstream ss;
                    ss << "Failed to create flac EA for c:" << a->getURL()
                       << " attribute name:" << rdn;
                    cerr << tostr( ss ) << endl;
                    Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
                }
                
//                 {
//                     string fname = a->getDirPath();
//                     VorbisComment tags;
//                     bool rc = get_tags( fname.c_str(), tags );
//                     VorbisComment::Entry t( rdn, "" );
//                     ensure_comment( tags, t );
//                     set_tags( fname,  tags );
//                 }

                string eaname = CanonEAName( rdn );
                a->addAttribute( eaname,
                                 (EA_Atom*)new FlacAttribute( rdn ),
                                 XSD_BASIC_STRING );
                fh_attribute ret = a->getAttribute( eaname );
                return ret;
            }
        
        
        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };

    void
    EAGenerator_Flac::Brew( const fh_context& a )
    {

        LG_FLAC_D << "EAGenerator_Flac::Brew() url:" << a->getURL() << endl;

        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        string fname = a->getDirPath();
        VorbisComment tags;
        bool rc = get_tags( fname.c_str(), tags );
        LG_FLAC_D << "rc:" << rc << endl;
        if( rc )
        {
            for( int i=0; i<tags.get_num_comments(); ++i )
            {
                VorbisComment::Entry e = tags.get_comment( i );
                string k = e.get_field_name();
                string v = e.get_field_value() ? e.get_field_value() : "";

                LG_FLAC_D << "k:" << k << " v:" << v << endl;

                string rdn = CanonEAName( k );
                a->addAttribute( rdn, (EA_Atom*)new FlacAttribute( k ), XSD_BASIC_STRING );
                if( rdn == "tracknumber" )
                {
                    a->addAttribute( "track", (EA_Atom*)new FlacAttribute( k ), XSD_BASIC_INT );
                }
                else if( rdn == "cddb" )
                {
                    a->addAttribute( "discid", (EA_Atom*)new FlacAttribute( k ), XSD_BASIC_STRING );
                }
            }
        }
        
        LG_FLAC_D << "Brew() "
                  << " url:" << a->getURL()
                  << " complete"
                  << endl;
    }

    bool
    EAGenerator_Flac::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        LG_FLAC_D << "tryBrew() "
                  << " url:" << ctx->getURL()
                  << endl;

        Brew( ctx );
        return ctx->isAttributeBound( eaname, false );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            LG_FLAC_D << "EAGenerator_Flac::CreateRealFactory()" << endl;
            return new EAGenerator_Flac();
        }
    };
};
