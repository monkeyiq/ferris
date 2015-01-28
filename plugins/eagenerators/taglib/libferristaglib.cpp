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

    $Id: libferristaglib.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

#include <fileref.h>
#include <tag.h>
#include <tstring.h>
#include <flacfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <xiphcomment.h>

using namespace std;
using namespace TagLib;

TagLib::String formatSeconds(int seconds)
{
  char secondsString[3];
  sprintf(secondsString, "%02i", seconds);
  return secondsString;
}

namespace Ferris
{
    typedef class EA_Atom_Static TaglibByteArrayAttribute;
    typedef class EA_Atom_Static TaglibByteArrayAttributeSchema;
    
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

    void ensure_comment( Ogg::XiphComment* c, const std::string& k, const std::string& v )
    {
        if( c->contains( k.c_str() ) )
        {
            c->removeField( k.c_str() );
            c->addField( k.c_str(), v.c_str() );
        }
        else
        {
            string upper = toupperstr()( k );
            c->removeField( k.c_str() );
            c->addField( k.c_str(), v.c_str() );
        }
    }

    class FERRISEXP_API CacheHandlableFileRef
        :
        public CacheHandlable 
    {
    public:
        TagLib::FileRef m_fileref;
        Ogg::XiphComment* m_comment;
        CacheHandlableFileRef( TagLib::FileRef fileref,
                               Ogg::XiphComment* comment = 0 )
            :
            m_fileref( fileref ),
            m_comment( comment )
            {}
    };
    FERRIS_SMARTPTR( CacheHandlableFileRef, fh_CacheHandlableFileRef );

    fh_CacheHandlableFileRef getCachedHandle( fh_context ctx )
    {
        Context* c = GetImpl(ctx);
        static Cache< Context*, fh_CacheHandlableFileRef > cache;
        cache.setTimerInterval( 3000 );

        if( fh_CacheHandlableFileRef d = cache.get( c ) )
        {
            return d;
        }

        string fname = ctx->getDirPath();
        TagLib::FileRef fileref( fname.c_str() );
        Ogg::XiphComment* comment = dynamic_cast<Ogg::XiphComment*>(fileref.tag());

        if( ends_with( tolowerstr()(ctx->getDirName()), ".flac" ))
        {
            TagLib::FLAC::File* f = new TagLib::FLAC::File(fname.c_str() );
            fileref = TagLib::FileRef(f);
            LG_TAGLIB_D << "have flac!" << endl;
            Ogg::XiphComment* c = f->xiphComment();
            comment = c;
            if( c && c->contains( "CDDB" ))
            {
                LG_TAGLIB_D << "have vorbis comment!" << endl;
                const TagLib::Ogg::FieldListMap& m = c->fieldListMap();
                LG_TAGLIB_D << "discid:" << m["CDDB"].toString().to8Bit(true) << endl;
            }
        }
        
        fh_CacheHandlableFileRef d = new CacheHandlableFileRef( fileref, comment );
        cache.put( c, d );
        return d;
    }
    

    TagLib::FileRef getCachedFileRef( fh_context ctx )
    {
        fh_CacheHandlableFileRef d = getCachedHandle( ctx );
        return d->m_fileref;
        
//     Context* c = GetImpl(ctx);
//     static Cache< Context*, fh_CacheHandlableFileRef > cache = getCache();
//         cache.setTimerInterval( 3000 );

//         if( fh_CacheHandlableFileRef d = cache.get( c ) )
//         {
//             return d->m_fileref;
//         }

//         string fname = ctx->getDirPath();
//         TagLib::FileRef fileref( fname.c_str() );
//         Ogg::XiphComment* comment = dynamic_cast<Ogg::XiphComment*>(fileref.tag());

//         if( ends_with( tolowerstr()(ctx->getDirName()), ".flac" ))
//         {
//             TagLib::FLAC::File* f = new TagLib::FLAC::File(fname.c_str() );
//             fileref = TagLib::FileRef(f);
//             LG_TAGLIB_D << "have flac!" << endl;
//             Ogg::XiphComment* c = f->xiphComment();
//             comment = c;
//             if( c && c->contains( "CDDB" ))
//             {
//                 LG_TAGLIB_D << "have vorbis comment!" << endl;
//                 const TagLib::Ogg::FieldListMap& m = c->fieldListMap();
//                 LG_TAGLIB_D << "discid:" << m["CDDB"].toString().to8Bit(true) << endl;
//             }
//         }
        
//         fh_CacheHandlableFileRef d = new CacheHandlableFileRef( fileref, comment );
//         cache.put( c, d );
//         return d->m_fileref;
    }

    Ogg::XiphComment* getCachedXiphComment( fh_context ctx )
    {
        fh_CacheHandlableFileRef d = getCachedHandle( ctx );
        return d->m_comment;
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    class FERRISEXP_DLLLOCAL EAGenerator_Taglib : public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Taglib()
            :
            MatchedEAGeneratorFactory()
            {
            }

        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );

        TagLib::FileRef getFileRef( Context* c )
            {
                TagLib::FileRef ret = getCachedFileRef( c );
                return ret;
            }
        Ogg::XiphComment* getComment( Context* c )
            {
                Ogg::XiphComment* ret = getCachedXiphComment( c );
                return ret;
            }
    };

    
    class FERRISEXP_DLLLOCAL TaglibAttribute
        :
        public EA_Atom_ReadWrite
    {
        EAGenerator_Taglib* m_fstate;
        TagLib::FileRef getFileRef( Context* c )
            {
                return m_fstate->getFileRef(c);
            }
        TagLib::Tag* getTag( Context* c )
            {
                return getFileRef(c).tag();
            }
        Ogg::XiphComment* getComment( Context* c )
            {
                return m_fstate->getComment(c);
            }
        

    public:

        fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                if( TagLib::Tag* t = getTag(c) )
                {
                    if( rdn == "title" )
                        ss << t->title().to8Bit(true);
                    else if( rdn == "artist" )
                        ss << t->artist().to8Bit(true);
                    else if( rdn == "album" )
                        ss << t->album().to8Bit(true);
                    else if( rdn == "year" )
                        ss << t->year();
                    else if( rdn == "comment" )
                        ss << t->comment().to8Bit(true);
                    else if( rdn == "track" )
                        ss << t->track();
                    else if( rdn == "genre" )
                        ss << t->genre().to8Bit(true);
                    else
                    {
                        LG_TAGLIB_D << "getting vorbis comment..." << endl;
                        if( Ogg::XiphComment* com = getComment(c) )
                        {
                            const TagLib::Ogg::FieldListMap& m = com->fieldListMap();
                            
                            LG_TAGLIB_D << "have vorbis comment, looking for rdn:" << rdn << endl;
                            if( rdn == "cddb" || rdn == "discid" )
                            {
                                ss << m["CDDB"].toString().to8Bit(true);
                            }
                            if( rdn == "date" )
                            {
                                ss << m["DATE"].toString().to8Bit(true);
                            }
                        }
                    }
                }
                LG_TAGLIB_D << "getStream() rdn:" << rdn << " ret:" << tostr(ss) << endl;
                return ss;
            }

        void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream datass )
            {
                string data = StreamToString( datass );
                TagLib::FileRef fr = getFileRef( c );

                if( TagLib::Tag* t = getTag(c) )
                {
                    if( rdn == "title" )
                        t->setTitle( data.c_str() );
                    else if( rdn == "artist" )
                        t->setArtist( data.c_str() );
                    else if( rdn == "album" )
                        t->setAlbum( data.c_str() );
                    else if( rdn == "year" )
                        t->setYear( toint(data) );
                    else if( rdn == "comment" )
                        t->setComment( data.c_str() );
                    else if( rdn == "track" )
                        t->setTrack( toint(data) );
                    else if( rdn == "genre" )
                        t->setGenre( data.c_str() );
                    else
                    {
                        if( Ogg::XiphComment* com = getComment(c) )
                        {
                            const TagLib::Ogg::FieldListMap& m = com->fieldListMap();
                            
                            if( rdn == "cddb" || rdn == "discid" )
                            {
                                if( com->contains( "cddb" ) || com->contains( "CDDB" ) )
                                    ensure_comment( com, "cddb", data );
                                if( com->contains( "discid" ) || com->contains( "DISCID" ) )
                                    ensure_comment( com, "discid", data );
                                
//                                ss << m["CDDB"].toString().to8Bit(true);
                            }
                            if( rdn == "date" )
                            {
                                if( com->contains( "date" ) || com->contains( "DATE" ) )
                                    ensure_comment( com, "date", data );
                            }
                        }
                    }
                    
                    fr.file()->save();
                }
            }

        TaglibAttribute( EAGenerator_Taglib* fstate )
            :
            EA_Atom_ReadWrite( this, &TaglibAttribute::getStream,
                               this, &TaglibAttribute::getStream,
                               this, &TaglibAttribute::setStream )
            ,m_fstate( fstate )
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

    void
    EAGenerator_Taglib::Brew( const fh_context& a )
    {
        LG_TAGLIB_D << "EAGenerator_Taglib::Brew() url:" << a->getURL() << endl;

        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );


//         string fname = a->getDirPath();
//         TagLib::FileRef f( fname.c_str() );
//         m_fileref = f;
//         m_comment = dynamic_cast<Ogg::XiphComment*>(f.tag());

//         if( ends_with( tolowerstr()(a->getDirName()), ".flac" ))
//         {
//             TagLib::FLAC::File* f = new TagLib::FLAC::File(fname.c_str() );
//             m_fileref = TagLib::FileRef(f);
//             LG_TAGLIB_D << "have flac!" << endl;
//             Ogg::XiphComment* c = f->xiphComment();
//             m_comment = c;
//             if( c && c->contains( "CDDB" ))
//             {
//                 LG_TAGLIB_D << "have vorbis comment!" << endl;
//                 const TagLib::Ogg::FieldListMap& m = c->fieldListMap();
//                 LG_TAGLIB_D << "discid:" << m["CDDB"].toString().to8Bit(true) << endl;
//             }
//         }

        string fname = a->getDirPath();
        TagLib::FileRef f = getCachedFileRef( a );
        
        
        if(!f.isNull() && f.tag())
        {
            TagLib::Tag *tag = f.tag();
            a->addAttribute( "title",  (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "artist", (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "album",  (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "year",   (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_INT );
            a->addAttribute( "comment",(EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "track",  (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_INT );
            a->addAttribute( "genre",  (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "cddb",   (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "discid", (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );
            a->addAttribute( "date",   (EA_Atom*)new TaglibAttribute( this ), XSD_BASIC_STRING );

//             if( Ogg::XiphComment* c = dynamic_cast<Ogg::XiphComment*>(tag))
//             {
//                 LG_TAGLIB_D << "tag is a vorbis comment!" << endl;
                
//                 const TagLib::Ogg::FieldListMap& m = c->fieldListMap();
//                 TagLib::Ogg::FieldListMap::ConstIterator mi = m.begin();
//                 TagLib::Ogg::FieldListMap::ConstIterator  e = m.end();
//                 for( ; mi != e; ++mi )
//                 {
//                     LG_TAGLIB_D << "key:" << mi->first.to8Bit(true) << endl;
//                 }
//                 if( c && c->contains( "CDDB" ))
//                 {
//                     LG_TAGLIB_D << "have vorbis comment!" << endl;
//                     const TagLib::Ogg::FieldListMap& m = c->fieldListMap();
//                     LG_TAGLIB_D << "discid:" << m["CDDB"].toString().to8Bit(true) << endl;
//                 }
                
//             }
        }
        LG_TAGLIB_D << "Brew() " << " url:" << a->getURL() << " complete" << endl;
    }

    bool
    EAGenerator_Taglib::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        LG_TAGLIB_D << "tryBrew() " << " url:" << ctx->getURL() << endl;

        Brew( ctx );
        return ctx->isAttributeBound( eaname, false );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            LG_TAGLIB_D << "EAGenerator_Taglib::CreateRealFactory()" << endl;
            return new EAGenerator_Taglib();
        }
    };
};
