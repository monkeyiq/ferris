/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2003 Ben Martin

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

    $Id: libferrisexifea.cpp,v 1.11 2011/04/28 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-content.h>

#include <config.h>

#ifdef HAVE_LIBJPEG
#include "jpeg-data.h"
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#endif

#ifdef HAVE_EPEG
#include <Epeg.h>
#endif

#include <Cache.hh>

using namespace std;

namespace Ferris
{
    namespace Util
    {
        static std::string convertCapsToDashLower( const std::string& s )
        {
            fh_stringstream ss;
            
            if( !s.empty() )
            {
                string::const_iterator si = s.begin();
                ss << (char)tolower( *si );
                for( ++si; si!=s.end(); ++si )
                {
                    if( isupper( *si ) )
                    {
                        ss << "-" << (char)tolower( *si );
                    }
                    else
                        ss << *si;
                }
            }

            string ret = tostr(ss);
            if( starts_with( ret, "g-p-s-" ))
                ret = (string)"gps-" + ret.substr(strlen("g-p-s-"));
            return ret;
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//    static const std::string ATTR_PREFIX   = "http://witme.sf.net/libferris.web/exif/";
    static const std::string ATTR_PREFIX   = "exif:";
    static const std::string SCHEMA_PREFIX = "schema:" + ATTR_PREFIX;
    
    static const std::string THUMBNAIL_EANAME_HAS_EXIF_THUMB = "exif:has-thumbnail";
    static const std::string THUMBNAIL_EANAME_UPDATE         = "exif:thumbnail-update";
    static const std::string THUMBNAIL_EANAME_RGBA           = "exif:thumbnail-rgba-32bpp";
    static const std::string THUMBNAIL_EANAME_W              = "exif:thumbnail-width";
    static const std::string THUMBNAIL_EANAME_H              = "exif:thumbnail-height";

    const char* my_exif_tag_get_name( ExifEntry* entry )
    {
        ExifTag tag = entry->tag;
        ExifFormat format = entry->format;
        unsigned long components = entry->components;

//         LG_EXIF_D << "my_exif_tag_get_name() tag:" << tag
//                   << " format:" << format << " components:" << components << endl;

        if( tag == EXIF_TAG_INTEROPERABILITY_INDEX
            || tag == EXIF_TAG_INTEROPERABILITY_VERSION )
        {
            if( format == EXIF_FORMAT_ASCII )
                if( tag == EXIF_TAG_INTEROPERABILITY_INDEX )
                    return "gps-latitude-ref";
            
            if( format == EXIF_FORMAT_RATIONAL )
            {
                if( components >= 2 && components <= 3 )
                {
                    if( tag == EXIF_TAG_INTEROPERABILITY_VERSION )
                        return "gps-latitude";
                }
            }
        }
        if( tag == EXIF_TAG_GPS_LONGITUDE_REF )
            return "gps-longitude-ref";
        if( tag == EXIF_TAG_GPS_LONGITUDE )
            return "gps-longitude";

        return exif_tag_get_name( tag );
    }
    

    
    static std::string getURI( const std::string& baseuri,
                               const std::string& ferrisEAName,
                               const std::string& pfx = "" )
    {
        std::string ret = baseuri;
        if( !pfx.empty() && starts_with( ferrisEAName, pfx ) )
            ret += ferrisEAName.substr( pfx.length() );
        else
            ret += ferrisEAName;

        return ret;
    }

    static std::string getPredicateURI( const std::string& rdn )
    {
        if( starts_with( rdn, "schema:" ))
        {
            return getURI( SCHEMA_PREFIX, rdn, "schema:" );
        }
        return getURI( ATTR_PREFIX, rdn );
    }
    
    static string
    stripPrefix( const std::string& s )
    {
        if( starts_with( s, SCHEMA_PREFIX ))
            return s.substr( SCHEMA_PREFIX.length() );
        
        if( starts_with( s, ATTR_PREFIX ))
            return s.substr( ATTR_PREFIX.length() );
        return s;
    }

    class FERRISEXP_DLLLOCAL CachedExifData
        :
        public CacheHandlable
    {
        ExifData* m_ed;
        
    public:

        CachedExifData( ExifData* ed )
            :
            m_ed( ed )
            {
                exif_data_ref( m_ed );
//                cerr << "CREATE CachedExifData() m_ed:" << toVoid( m_ed ) << endl;
            }
        
        virtual ~CachedExifData()
            {
//                cerr << "DESTROY ~CachedExifData(T)" << endl;
                if( m_ed )
                {
//                    cerr << "DESTROY ~CachedExifData() m_ed:" << toVoid( m_ed ) << endl;
                    exif_data_unref( m_ed );
                    m_ed = 0;
                }
            }

        virtual ref_count_t AddRef()
            {
                exif_data_ref( m_ed );
                return CacheHandlable::AddRef();
            }
    
        virtual ref_count_t Release()
            {
                exif_data_unref( m_ed );
                return CacheHandlable::Release();
            }
        
        virtual void sync()
            {
            }

        typedef ExifData* ExifDataPtr;
        operator ExifDataPtr()
            {
                return m_ed;
            }
        
    };
    FERRIS_SMARTPTR( CachedExifData, fh_cachedExifData );

//     static ExifData* getExifData( const fh_context& ctx )
//     {
//         ExifData* ed = exif_data_new_from_file ( c->getDirPath().c_str() );
//         return ed;
//     }

    static fh_cachedExifData getExifData( const fh_context& ctx )
    {
//        static Cache< Context*, fh_cachedExifData > cache;
        static Cache< Context*, fh_cachedExifData >* cache = 0;
        if( !cache )
            cache = new Cache< Context*, fh_cachedExifData >();
        
        cache->setTimerInterval( 3000 );
        
        Context* c = GetImpl(ctx);
        
        if( fh_cachedExifData d = cache->get( c ) )
        {
            return d;
        }

        ExifData* ed = exif_data_new_from_file ( c->getDirPath().c_str() );
        if( !ed )
        {
            fh_istream iss = c->getIStream();
            string    data = StreamToString( iss );
            if( !data.size() )
            {
                cerr << "WARNING: getExifData() no data for ctx:" << ctx->getURL() << endl;
                return 0;
            }
            
            ed = exif_data_new_from_data( (const unsigned char*)data.data(),
                                          data.size() );

            if( !ed )
            {
                cerr << "WARNING: getExifData() no data for ctx:" << ctx->getURL() << endl;
                return 0;
            }
        }
        
        fh_cachedExifData d = new CachedExifData( ed );
        cache->put( c, d );
        return d;
    }

    static void
    updateEntry( fh_context c, fh_cachedExifData edholder, ExifEntry* entry, fh_istream iss )
    {
        ExifData* ed = *edholder;

        ExifByteOrder exif_byte_order = exif_data_get_byte_order (ed);
        int exif_type_sz = exif_format_get_size (entry->format);
              
        for (int i = 0; i < entry->components; i++)
        {
            long iss_as_long = 0;

            if( entry->format != EXIF_FORMAT_ASCII )
                iss_as_long = toType<long>( StreamToString(iss) );
            
            switch (entry->format)
            {
            case EXIF_FORMAT_ASCII:
            {
                if (i != 0)
                {
                    fh_stringstream ss;
                    ss << "Attempt to set many strings for an EXIF ascii attribute" << endl
                       << " rdn:" << Util::convertCapsToDashLower( my_exif_tag_get_name( entry ) )
                       << endl;
                    if( c )
                        ss << " url:" << c->getURL() << endl;
                    Throw_getIOStreamCloseUpdateFailed( tostr(ss), 0 );
                }

                string newstr = StreamToString(iss);
                const int newdatasz = newstr.length() + 1;
                unsigned char* newdata = (unsigned char *)malloc( sizeof (char) * newdatasz );

                if (!newdata)
                {
                    fh_stringstream ss;
                    ss << "Out of RAM when attempting to set many strings for an EXIF ascii attribute" << endl
                       << " rdn:" << Util::convertCapsToDashLower( my_exif_tag_get_name( entry ) )
                       << " newdatasz:" << newdatasz
                       << endl;
                    if( c )
                        ss << " url:" << c->getURL() << endl;
                    Throw_FerrisOutOfMemory( tostr(ss), 0 );
                }
                
                if (entry->data)
                    free (entry->data);
                entry->size = newdatasz;
                entry->data = newdata;
                
                strcpy( (char*)entry->data, newstr.c_str() );
                entry->components = strlen( (const char*)entry->data) + 1;
                i = entry->components - 1;
                break;
            }
            case EXIF_FORMAT_SHORT:
                exif_set_short (entry->data + (exif_type_sz * i), exif_byte_order, iss_as_long );
                break;
            case EXIF_FORMAT_LONG:
                exif_set_long (entry->data + (exif_type_sz * i), exif_byte_order, iss_as_long );
                break;
            case EXIF_FORMAT_SLONG:
                exif_set_slong (entry->data + (exif_type_sz * i), exif_byte_order, iss_as_long );
                break;
            case EXIF_FORMAT_BYTE:
            case EXIF_FORMAT_RATIONAL:
            case EXIF_FORMAT_SRATIONAL:
            default:
            {
                fh_stringstream ss;
                ss << "Unimplemented write support for selected EXIF datatype" << endl
                   << " when attempting to set many strings for an EXIF ascii attribute" << endl
                   << " rdn:" << Util::convertCapsToDashLower( my_exif_tag_get_name( entry ) )
                   << endl;
                if( c )
                    ss << " url:" << c->getURL() << endl;
                Throw_getIOStreamCloseUpdateFailed( tostr(ss), 0 );
            }
            }
        }
    }

    static void
    updateEntry( fh_context c, fh_cachedExifData edholder, ExifEntry* entry, const std::string& s )
    {
        fh_stringstream ss;
        ss << s;
        updateEntry( c, edholder, entry, ss );
    }
    
    static void
    SaveExif( fh_context c, fh_cachedExifData edholder )
    {
        ExifData* ed = *edholder;
        
#ifdef HAVE_LIBJPEG
        string tempfilename = "/tmp/libferris-exif-tempfile.jpg";
        
        JPEGData *jdata;
        unsigned char *d = NULL;
        unsigned int ds;

        /* Parse the JPEG file */
        jdata = jpeg_data_new_from_file ( c->getDirPath().c_str() );
        if (!jdata)
        {
            fh_stringstream ss;
            ss << "Failed to read the original EXIF data from:" << c->getURL() << endl;
            Throw_FerrisImageSaveFailed( tostr(ss), GetImpl(c) );
        }

        /* Make sure the EXIF data is not too big. */
        exif_data_save_data (ed, &d, &ds);
        if (ds) {
            free (d);
            if (ds > 0xffff)
            {
                fh_stringstream ss;
                ss << "Failed to update the EXIF data from:" << c->getURL() << endl
                   << "Too much new EXIF data (" << ds << " bytes)" << endl
                   << " Only " << 0xffff << " bytes are allowed." << endl
                   << " Image will be left untouched"
                   << endl;
                Throw_FerrisImageSaveFailed( tostr(ss), GetImpl(c) );
            }
        };

        jpeg_data_set_exif_data( jdata, ed );
        
        /* Save the modified image. */
        jpeg_data_save_file (jdata, tempfilename.c_str() );
        jpeg_data_unref (jdata);

        
        if( !::rename( tempfilename.c_str(), c->getDirPath().c_str() ) )
        {
            /* ok */
            return;
        }
        int e = errno;
    
        /* failed */
        fh_stringstream ss;
        ss << "Rename attempt failed when moving the new image with updated EXIF over the old file" << endl
           << " tmpfile:" << tempfilename << endl
           << " target:" << c->getURL()
           << endl;
        Throw_RenameFailed( errnum_to_string( tostr(ss), e ), GetImpl(c) );
#endif
        
        {
            fh_stringstream ss;
            ss << "libferris build lacks needed libjpeg support at compile time" << endl
               << " image untouched at:" << c->getURL() << endl;
            Throw_FerrisImageSaveFailed( tostr(ss), GetImpl(c) );
        }
    }
    
    

    
/********************************************************************************/
/********************************************************************************/

    ExifEntry* findTagIfExists( fh_cachedExifData edholder, const string& tagname )
    {
        ExifData* ed = *edholder;
        ExifEntry* ret = 0;

        for( unsigned int edi = 0; edi < EXIF_IFD_COUNT; edi++)
        {
            ExifContent *content = ed->ifd[ edi ];

            for( unsigned int ci = 0; ci < content->count; ci++)
            {
                if( ExifEntry* entry = content->entries[ci] )
                {
                    if( !entry || !entry->tag )
                        continue;

                    const char* tag_CSTR = my_exif_tag_get_name( entry );
                    if( tag_CSTR && 
                        Util::convertCapsToDashLower( tag_CSTR )
                        == Util::convertCapsToDashLower( tagname ) )
                    {
                        ret = entry;
                        return ret;
                    }
                }
            }
        }
        
        return ret;
    }

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

ExifTag
exif_tag_from_string (const char *string)
{
	ExifTag tag;
	unsigned int i, number, factor;
	const char *name;

	if (!string)
		return ((ExifTag)0);

//    LG_EXIF_D << "exif_tag_from_string() string:" << string << endl;

	/* Is the string a tag's name or title? */
	for (tag = (ExifTag)0xffff;
         tag > (ExifTag)0;
         tag = (ExifTag)(((long)tag)-1) ) {
		name = exif_tag_get_name (tag);
		if (name && !strcmp (string, name))
			return (tag);
		name = exif_tag_get_title (tag);
		if (name && !strcmp (string, name))
			return (tag);
	}

	/* Is the string a decimal number? */
	if (strspn (string, "0123456789") == strlen (string))
		return ((ExifTag)atoi (string));

	/* Is the string a hexadecimal number? */
	for (i = 0; i < strlen (string); i++)
		if (string[i] == 'x')
			break;
	if (i == strlen (string))
		return ((ExifTag)0);

	string += i + 1;
        tag = (ExifTag)0;
        for (i = strlen (string); i > 0; i--) {
                switch (string[i - 1]) {
                case '0':
                        number = 0;
                        break;
                case '1':
                        number = 1;
                        break;
                case '2':
                        number = 2;
                        break;
                case '3':
                        number = 3;
                        break;
                case '4':
                        number = 4;
                        break;
                case '5':
                        number = 5;
                        break;
                case '6':
                        number = 6;
                        break;
                case '7':
                        number = 7;
                        break;
                case '8':
                        number = 8;
                        break;
                case '9':
                        number = 9;
                        break;
                case 'a':
                case 'A':
                        number = 10;
                        break;
                case 'b':
                case 'B':
                        number = 11;
                        break;
                case 'c':
                case 'C':
                        number = 12;
                        break;
                case 'd':
                case 'D':
                        number = 13;
                        break;
                case 'e':
                case 'E':
                        number = 14;
                        break; 
                case 'f':
                case 'F':
                        number = 15;
                        break; 
                default:
                    return ((ExifTag)0);
                }
                factor = 1 << ((strlen (string) - i) * 4);
                tag = (ExifTag)((long)tag + (number * factor));
        }

        return (tag);
}
    
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
    
    class FERRISEXP_DLLLOCAL EAGenerator_Exif : public MatchedEAGeneratorFactory
    {
        typedef EAGenerator_Exif _Self;
        
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Exif();
        virtual ~EAGenerator_Exif();

        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 );

        virtual bool isDynamic()
            {
                return true;
            }
        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    
    
        bool
        supportsCreateForContext( fh_context c )
            {
                return true;
            }

        void generateAttribute( ExifEntry* entry,
                                const fh_context& a,
                                const std::string& rdn,
                                bool forceCreate = false );

        /********************************************************************************/
        /********************************************************************************/
        fh_istream getDateTimeEpoch( Context*, const std::string&, EA_Atom* attr );
        fh_istream getDateTimeXML( Context*, const std::string&, EA_Atom* attr );

#ifdef HAVE_EPEG
        int         m_thumbW;
        int         m_thumbH;
        bool        m_nothumb;

        class FERRISEXP_DLLLOCAL CachedEpeg
            :
            public CacheHandlable
        {
            fh_cachedExifData   edholder;
            Epeg_Image* m_epeg;
        
        public:
        
            CachedEpeg( fh_cachedExifData edholder, Epeg_Image* m_epeg )
                :
                edholder( edholder ),
                m_epeg( m_epeg )
                {
//                    cerr << "CREATE CachedEpeg() m_epeg:" << toVoid( m_epeg ) << endl;
                }
        
            virtual ~CachedEpeg()
                {
//                    cerr << "DESTROY ~CachedEpeg() m_epeg:" << toVoid( m_epeg ) << endl;
                    if( m_epeg )
                        epeg_close( m_epeg );
                    m_epeg = 0;
                }

            virtual void sync()
                {
                }

            Epeg_Image* Epeg()
                {
                    return m_epeg;
                }
            Epeg_Image* epeg()
                {
                    return m_epeg;
                }
        };
        FERRIS_SMARTPTR( CachedEpeg, fh_cachedEpeg );

        fh_cachedEpeg getEpeg( Context* c );
        fh_istream getHasThumb( Context*, const std::string&, EA_Atom* attr );
        fh_stringstream getThumbUpdate( Context* c, const std::string&, EA_Atom* attr );
        void getThumbUpdateClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );
        fh_istream getThumbWidthStream( Context*, const std::string&, EA_Atom* attr );
        fh_istream getThumbHeightStream( Context*, const std::string&, EA_Atom* attr );
        fh_istream getThumbRGBAStream( Context* c, const std::string&, EA_Atom* attr );
#endif

        fh_istream getFlashFiredStream( Context* c, const std::string&, EA_Atom* attr );
        
        
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
    class FERRISEXP_DLLLOCAL ExifByteArrayAttribute
        :
        public EA_Atom_ReadWrite
    {

    protected:

        ferris_ios::openmode getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary | ios::ate;
            }

        void createOnDiskAttribute( const fh_context& parent, const std::string& uri )
            {
//                 string v = "";

//                 RDF::fh_model  m = getDefaultFerrisModel();

//                 try
//                 {
//                     m->insert( Node::CreateURI( parent->getURL() ),
//                                Node::CreateURI( uri ),
//                                Node::CreateLiteral( v ) );
//                 }
//                 catch( exception& e )
//                 {
//                     fh_stringstream ss;
//                     ss << "Failed to create RDF EA for c:" << parent->getURL()
//                        << " attribute name:" << uri
//                        << endl;
//                     ThrowFromErrno( 0, tostr(ss), 0 );
//                 }
            }
    
    
    public:

        virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;

                LG_EXIF_D << "ExifByteArrayAttribute::getStream() rdn:" << rdn << endl;
                
                string    tagstr = stripPrefix( rdn );
                fh_cachedExifData ed = getExifData( c );

                ExifEntry* entry = findTagIfExists( ed, tagstr );
                LG_EXIF_D << "ExifByteArrayAttribute::getStream() entry:" << toVoid(entry) << endl;
                if( entry )
                {
                    int i = min( 0, entry->components );
                    ExifByteOrder o = exif_data_get_byte_order( *ed );

                    LG_EXIF_D << "ExifByteArrayAttribute::getStream() format:" << entry->format << endl;
                    LG_EXIF_D << "ExifByteArrayAttribute::getStream() components:" << entry->components << endl;
                    
                    
                    switch (entry->format) {
                    case EXIF_FORMAT_BYTE:
                        ss << entry->data[i];
                        break;
                    case EXIF_FORMAT_SBYTE:
                        ss << entry->data[i];
                        break;
                    case EXIF_FORMAT_SHORT:
                        ss << exif_get_short (entry->data + 2 * i, o);
                        break;
                    case EXIF_FORMAT_SSHORT:
                        ss << exif_get_sshort (entry->data + 2 * i, o);
                        break;
                    case EXIF_FORMAT_LONG:
                        ss <<  exif_get_long (entry->data + 4 * i, o);
                        break;
                    case EXIF_FORMAT_SLONG:
                        ss <<  exif_get_slong (entry->data + 4 * i, o);
                        break;
                    case EXIF_FORMAT_RATIONAL:
                    {
                        if( rdn == "exif:gps-longitude" || rdn == "exif:gps-latitude" )
                        {
                            static const double divs[] = { 1.0, 60.0, 3600.0, 0 };
                            
                            LG_EXIF_D << "EXIF_FORMAT_RAT long/lat" << endl;

                            double v = 0.0;
                            for( int j=0; j < min( 3, entry->components ); j++ )
                            {
                                ExifRational r = exif_get_rational (entry->data + sizeof(ExifRational) * j, o);
                                double rv = 0.0;
                                if( !r.denominator )
                                    rv = (1.0*r.numerator);
                                else
                                    rv = (1.0*r.numerator/r.denominator);
                                rv /= divs[ j ];
                                LG_EXIF_D << "component j:" << j 
                                          << " num:" << r.numerator << " denum:" << r.denominator
                                          << " rv:" << rv
                                          << endl;
                                v += rv;
                            }

                            LG_EXIF_D << "abs(v):" << v << endl;
                            
                            string nesw_ea_name = stripPrefix( rdn ) + "-ref";
                            ExifEntry* nesw_entry = findTagIfExists( ed, nesw_ea_name );
                            LG_EXIF_D << "nesw_ea_name:" << nesw_ea_name << " have nesw entry:" << toVoid( nesw_entry ) << endl;
                            if( nesw_entry )
                            {
                                if( nesw_entry->format == EXIF_FORMAT_ASCII )
                                {
                                    if( nesw_entry->size >= 1 )
                                    {
                                        char c = nesw_entry->data[0];
                                        LG_EXIF_D << "nesw desc:" << c << endl;
                                        if( c == 'w' || c == 'W' )
                                        {
                                            v *= -1;
                                        }
                                        if( c == 's' || c == 'S' )
                                        {
                                            v *= -1;
                                        }
                                    }
                                }
                            }
                            LG_EXIF_D << "ss1:" << ss.str() << endl;
                            ss.precision(15);
                            ss << v;
                            LG_EXIF_D << "ss2:" << ss.str() << endl;
                            break;
                        }
                        else
                        {
                            ExifRational r = exif_get_rational (entry->data + sizeof(ExifRational) * i, o);
                            if( !r.denominator )
                                ss << (1.0*r.numerator);
                            else
                                ss << (1.0*r.numerator/r.denominator);
                            LG_EXIF_D << "EXIF_FORMAT_RAT num:" << r.numerator << " denum:" << r.denominator << endl;
                            break;
                        }
                    }
                    case EXIF_FORMAT_SRATIONAL:
                    {
                        ExifSRational r = exif_get_srational (entry->data + sizeof(ExifSRational) * i, o);
                        if( !r.denominator )
                            ss << (1.0*r.numerator);
                        else
                            ss << (1.0*r.numerator/r.denominator);
                        LG_EXIF_D << "EXIF_FORMAT_SRAT" << endl;
                        break;
                    }
                    case EXIF_FORMAT_FLOAT:
                    {
                        float* f = (float*)(entry->data + sizeof(float) * i);
                        ss << *f;
//                        cerr << "EXIF_FORMAT_FLOAT f:" << *f << endl;
                        break;
                    }
                    case EXIF_FORMAT_DOUBLE:
                    {
                        double* f = (double*)(entry->data + sizeof(double) * i);
//                        cerr << "EXIF_FORMAT_DOUBLE f:" << *f << endl;
                        ss << *f;
                        break;
                    }
                    case EXIF_FORMAT_UNDEFINED:
                    {
                        LG_EXIF_W << "undefined type attribute, not handled as it is buggy"
                                  << " ... i:" << i << " size:" << entry->size << endl;
//                         if( entry->size )
//                         {
//                             ss.write( (const char*)entry->data, entry->size );
//                         }
                        break;
                    }
                    
                        
                    default:
//                        ss << exif_entry_get_value( entry );
                        if( entry->size )
                        {
                            ss.write( (const char*)entry->data, entry->size );
                        }
                        
                        break;
                    }
                }
                else
                {
                    fh_stringstream ss;
                    ss << "reading attribute:" << rdn;
                    Throw_NoSuchAttribute( tostr(ss), c );
                }
                return ss;
            }

        virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                fh_cachedExifData ed = getExifData( c );
                string     tagstr = stripPrefix( rdn );
                ExifEntry*  entry = findTagIfExists( ed, tagstr );
                updateEntry( c, ed, entry, ss );
                
 				SaveExif( c, ed );
            }

    
    
        ExifByteArrayAttribute(
            const fh_context& parent,
            const string& rdn,
            const string& uri,
            bool forceCreate = false
            )
            :
            EA_Atom_ReadWrite( this, &ExifByteArrayAttribute::getStream,
                               this, &ExifByteArrayAttribute::getStream,
                               this, &ExifByteArrayAttribute::setStream )
            {
                if( forceCreate )
                {
                    LG_EXIF_D << "ExifByteArrayAttribute() making EA for earl:" << parent->getURL()
                              << " predicate::" << uri
                              << endl;
                    createOnDiskAttribute( parent, uri );
                }
            }

        ~ExifByteArrayAttribute()
            {
            }

        static ExifByteArrayAttribute* Create( const fh_context& parent,
                                               const string& rdn,
                                               bool forceCreate = false )
            {
                return new ExifByteArrayAttribute( parent,
                                                   rdn,
                                                   getPredicateURI( rdn ),
                                                   forceCreate );
            }
    };


    static fh_stringstream getSchema( Context* c, const std::string& rdn, EA_Atom* )
    {
        fh_cachedExifData   edholder = getExifData( c );
        ExifData*                 ed = *edholder;
        
        string          tagstr = stripPrefix( rdn );
		ExifTag            tag = exif_tag_from_string( tagstr.c_str() );

        Factory::xsdtypemap_t tmap;
        Factory::makeBasicTypeMap( tmap );
        fh_context schema = tmap[ XSD_UNKNOWN ];
        
        for( int ifd = EXIF_IFD_0; ifd < EXIF_IFD_COUNT; ++ifd )
        {
            ExifEntry* e = exif_content_get_entry (ed->ifd[ifd], tag);
            if (e)
            {

                switch (e->format)
                {
                case EXIF_FORMAT_LONG:
                case EXIF_FORMAT_SLONG:
                    schema = tmap[ FXD_INT32 ];
                    break;
                    
                case EXIF_FORMAT_SHORT:
                    schema = tmap[ XSD_BASIC_INT ];
                    break;

                case EXIF_FORMAT_RATIONAL:
                case EXIF_FORMAT_SRATIONAL:
                    schema = tmap[ XSD_BASIC_FLOAT ];
                    break;

                case EXIF_FORMAT_BYTE:
                case EXIF_FORMAT_UNDEFINED:
                    schema = tmap[ XSD_UNKNOWN ];
                    break;

                case EXIF_FORMAT_ASCII:
                    schema = tmap[ XSD_BASIC_STRING ];
                    break;
                }
            }
        }
        
        fh_stringstream ss;
        ss << schema->getURL();
        return ss;
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    EAGenerator_Exif::EAGenerator_Exif()
    :
        MatchedEAGeneratorFactory()
#ifdef HAVE_EPEG
        ,m_thumbW( 0 ),
        m_thumbH( 0 ),
        m_nothumb( false )
#endif
    {
    }

    EAGenerator_Exif::~EAGenerator_Exif()
    {
    }

    fh_istream
    EAGenerator_Exif::getDateTimeXML( Context* c, const std::string& rdn, EA_Atom* attr )
    {
        string base_eaname = rdn.substr( 0, rdn.length() - strlen( "-xml" ));
        base_eaname += "-epoch";
        string tstr = getStrAttr( c, base_eaname, "0", true, true );
        fh_stringstream ss;
        ss << Time::toXMLDateTime( toType<time_t>( tstr ) );
        return ss;
    }
    
    
    fh_istream
    EAGenerator_Exif::getDateTimeEpoch( Context* c , const std::string& full_eaname, EA_Atom* attr )
    {
        string original_eaname = full_eaname.substr( 0, full_eaname.length() - strlen( "-epoch" ));
//        cerr << "EAGenerator_Exif::getDateTimeEpoch(1)" << endl;
        string tstr = getStrAttr( c, original_eaname, "0", true, true );
//        cerr << "EAGenerator_Exif::getDateTimeEpoch(2) tstr:" << tstr << endl;
        fh_stringstream ss;
        struct tm t = Time::ParseTimeString( tstr );
//        cerr << "EAGenerator_Exif::getDateTimeEpoch(3) " << endl;
        time_t tt = Time::toTime( &t );
//        cerr << "EAGenerator_Exif::getDateTimeEpoch(4) tt:" << tt << endl;
        ss << tt;
        return ss;
    }
    
    
#ifdef HAVE_EPEG

    EAGenerator_Exif::fh_cachedEpeg
    EAGenerator_Exif::getEpeg( Context* ctx )
    {
        LG_EXIF_D << "EAGenerator_Exif::getEpeg(top) ctx:" << ctx->getURL() << endl;
        
        if( m_nothumb )
        {
            LG_EXIF_D << "EAGenerator_Exif::getEpeg(no thumb) ctx:" << ctx->getURL() << endl;
            m_thumbW = 0;
            m_thumbH = 0;
            return 0;
        }

        static Cache< Context*, fh_cachedEpeg > cache;
        
        if( fh_cachedEpeg d = cache.get( ctx ) )
        {
            LG_EXIF_D << "EAGenerator_Exif::getEpeg(cached) ctx:" << ctx->getURL()
                      << " epeg:" << toVoid( d->epeg() )
                      << endl;
            return d;
        }
        
        fh_cachedExifData   edholder = getExifData( ctx );
        ExifData*                 ed = *edholder;

        if( !ed->data )
        {
            LG_EXIF_D << "EAGenerator_Exif::getEpeg(no thumb 2) ctx:" << ctx->getURL() << endl;
            m_nothumb = true;
            m_thumbW = 0;
            m_thumbH = 0;
            return 0;
        }
        
        Epeg_Image* im = epeg_memory_open( ed->data, ed->size );
        epeg_size_get( im, &m_thumbW, &m_thumbH );

        fh_cachedEpeg d = new CachedEpeg( edholder, im );
        cache.put( ctx, d );
        LG_EXIF_D << "EAGenerator_Exif::getEpeg(loaded) ctx:" << ctx->getURL()
                  << " epeg:" << toVoid( d->epeg() )
                  << endl;
        return d;
    }
    
    fh_istream
    EAGenerator_Exif::getHasThumb( Context* c , const std::string&, EA_Atom* attr )
    {
        fh_cachedExifData   edholder = getExifData( c );
        ExifData*                 ed = *edholder;

        fh_stringstream ss;
        ss << ( ed->data != 0 );
        return ss;
    }

    fh_istream
    EAGenerator_Exif::getThumbWidthStream( Context* c, const std::string&, EA_Atom* attr )
    {
        bool valid = isBound( getEpeg( c ) );

        LG_EXIF_D << "EAGenerator_Exif::getThumbWidthStream() c:" << c->getURL()
                  << " w:" << m_thumbW
                  << endl;
        fh_stringstream ss;
        ss << m_thumbW;
        return ss;
    }

    fh_istream
    EAGenerator_Exif::getThumbHeightStream( Context* c, const std::string&, EA_Atom* attr )
    {
        bool valid = isBound( getEpeg( c ) );
        
        fh_stringstream ss;
        ss << m_thumbH;
        return ss;
    }

    fh_istream
    EAGenerator_Exif::getThumbRGBAStream( Context* c, const std::string&, EA_Atom* attr )
    {
        LG_EXIF_D << "EAGenerator_Exif::getThumbRGBAStream(top) url:" << c->getURL() << endl;
        
        fh_cachedEpeg im = getEpeg( c );

        if( !isBound(im) || !m_thumbW || !m_thumbH )
        {
            stringstream ss;
            ss << "getThumbRGBAStream() No stored thumbnail for url:" << c->getURL()
               << " epeg:" << toVoid( im )
               << " m_thumbW:" << m_thumbW
               << " m_thumbH:" << m_thumbH
               << endl;
            cerr << tostr(ss) << endl;
            Throw_FerrisImageLoadFailed( tostr(ss), 0 );
        }

        LG_EXIF_D << "EAGenerator_Exif::getThumbRGBAStream(2) url:" << c->getURL() << endl;
        
        // Debug only
//         fh_stringstream ss;
//         ExifData*  ed = getExifData( c );
//         ss.write( (const char*)ed->data, ed->size );

        
        fh_stringstream ss;
        epeg_decode_colorspace_set( im->epeg(), EPEG_ARGB32 );
//        epeg_decode_colorspace_set( im->epeg(), EPEG_RGBA8 );
        const void * pixels = epeg_pixels_get( im->epeg(), 0, 0, m_thumbW, m_thumbH );
        guint32 sz = m_thumbW * m_thumbH * sizeof(gint32);
        LG_EXIF_D << "getThumbRGBAStream() writing thumbnail decoded pixels for:" << c->getURL()
             << " epeg:" << toVoid( im )
             << " m_thumbW:" << m_thumbW
             << " m_thumbH:" << m_thumbH
             << " sz:" << sz
             << endl;
        ss.write( (char*)pixels, sz );
        epeg_pixels_free( im->epeg(), pixels );

        // Debug only
//         epeg_quality_set     (im->epeg(), 80);
//         epeg_file_output_set (im->epeg(), "/tmp/ferris-epeg.jpg" );
//         epeg_encode          (im->epeg());
        
        ss.clear();
        return ss;
    }

    fh_stringstream
    EAGenerator_Exif::getThumbUpdate( Context* c, const std::string&, EA_Atom* attr )
    {
        bool ret = false;

        int w=0;
        int h=0;
        
        Epeg_Image* im = epeg_file_open( c->getDirPath().c_str() );
        epeg_size_get(im, &w, &h);

        int    MaxDesiredWidthOrHeight = toint(
            getConfigString(
                FDB_GENERAL,
                CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_K,
                CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_DEFAULT ));
        double ratio = MaxDesiredWidthOrHeight;

        if( w > MaxDesiredWidthOrHeight )
        {
            ratio = MaxDesiredWidthOrHeight;
            ratio /= w;
            w = MaxDesiredWidthOrHeight;
            h = (int)( ratio * h );
        }
        if( h > MaxDesiredWidthOrHeight )
        {
            ratio = MaxDesiredWidthOrHeight;
            ratio /= h;
            w = (int)( ratio * w );
            h = MaxDesiredWidthOrHeight;
        }

//         w = 160;
//         h = 120;
//         w = 80;
//         h = 60;
        
        epeg_decode_size_set(im, w, h );

        //const int       bufsz = 128 * 128 * 4;
        //unsigned char*    buf = [ bufsz + 1 ];

//         // Debug only
//         epeg_quality_set     (im, 80);
//         epeg_file_output_set (im, "/tmp/ferris-epeg.jpg" );
//         epeg_encode          (im);
        
        int            bufsz = 0;
        unsigned char*   buf = 0;
        epeg_quality_set(
            im, toint( getConfigString( FDB_GENERAL,
                                        CFG_THUMBNAILS_JPEG_IMG_QUALITY_K,
                                        CFG_THUMBNAILS_JPEG_IMG_QUALITY_DEFAULT )));
        epeg_decode_colorspace_set     ( im, EPEG_RGBA8 ); //EPEG_ARGB32 );
        epeg_memory_output_set         ( im, &buf, &bufsz );
        epeg_encode                    ( im );

        // Debug only
//         {
//             FILE* f = fopen("/tmp/ferris-epeg.jpg", "w" );
//             size_t bwrite = fwrite( buf, 1, bufsz, f );
//             fclose(f);

//             LG_EXIF_D << "Wrote " << bwrite << " of " << bufsz << " bytes to tmpfile" << endl;
//         }
        
        fh_cachedExifData   edholder = getExifData( c );
        ExifData*                 ed = *edholder;

        LG_EXIF_D << "old data:" << toVoid( ed->data )
             << " old data size:" << ed->size << endl
             << " new data:" << toVoid( buf )
             << " new data size:" << bufsz
             << endl;
        
        /* Get rid of the old thumbnail */
        if (ed->data) {
            free (ed->data);
            ed->data = NULL;
        }
        ed->size = 0;

        /* Copy the new thumbnail over */
        ed->size = bufsz;
        ed->data = (unsigned char*)malloc (sizeof (char) * ed->size);
        memcpy( ed->data, buf, bufsz );

        epeg_close( im );


        /* Update a little metadata in the EXIF part */
        {
            ExifTag     tag = (ExifTag)0;
            ExifEntry*    e = 0;
            int         ifd = EXIF_IFD_0;
            
            tag = exif_tag_from_string( "0x0131" ); // Software
            e = exif_content_get_entry( ed->ifd[ifd], tag );
            if (!e) {
                e = exif_entry_new ();
                exif_content_add_entry (ed->ifd[ifd], e);
                exif_entry_initialize (e, tag);
                updateEntry( c, edholder, e, "libferris " FERRIS_VERSION );
            }

            tag = exif_tag_from_string( "0x0132" ); // Date and Time
            e = exif_content_get_entry( ed->ifd[ifd], tag );
            if (!e) {
                e = exif_entry_new ();
                exif_content_add_entry (ed->ifd[ifd], e);
                exif_entry_initialize (e, tag);
//                 ExifByteOrder exif_byte_order = exif_data_get_byte_order (ed);
//                 exif_set_long( e->data, exif_byte_order, Time::getTime() );

                updateEntry( c, edholder, e,
                             Time::toTimeString( Time::getTime(), "%Y:%m:%d %H:%M:%S" ) );
            }
        }
        
        SaveExif( c, edholder );

        ret = true;
        
        fh_stringstream ss;
        ss << ret;
        return ss;
    }
    
    void
    EAGenerator_Exif::getThumbUpdateClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
    }
    


#endif    

#define FERRIS_EXIF_FLASH_FIRED 0x0001
    
    fh_istream
    EAGenerator_Exif::getFlashFiredStream( Context* c, const std::string&, EA_Atom* attr )
    {
        long v = toType<long>(getStrAttr( c, "exif:flash", "0" ));
        fh_stringstream ss;
        ss << ( v & FERRIS_EXIF_FLASH_FIRED );
        return ss;
    }

    
    void
    EAGenerator_Exif::generateAttribute( ExifEntry* entry,
                                         const fh_context& a,
                                         const std::string& rdn,
                                         bool forceCreate )
    {
        LG_EXIF_D << "EAGenerator_Exif::generateAttribute() ctx:" << a->getURL()
                  << " entry:" << toVoid( entry )
                  << " rdn:" << rdn
                  << endl;

        if( a->isAttributeBound( rdn, false ))
        {
            LG_EXIF_D << "EAGenerator_Exif::generateAttribute(already bound) ctx:" << a->getURL()
                 << " entry:" << toVoid( entry )
                 << " rdn:" << rdn
                 << endl;
            return;
        }

        XSDBasic_t sct = FXD_BINARY_NATIVE_EA;

        if( entry )
        {
            switch (entry->tag) {
            case EXIF_TAG_EXIF_VERSION:
            case EXIF_TAG_FLASH_PIX_VERSION:
                sct = FXD_EXIF_VERSION;
                break;
                
            case EXIF_TAG_USER_COMMENT:
                sct = FXD_EXIF_USER_COMMENT;
                break;
            case EXIF_TAG_COPYRIGHT:
                sct = FXD_EXIF_COPYRIGHT;
                break;
            case EXIF_TAG_FLASH:
                sct = FXD_EXIF_FLASH;
                break;
            case EXIF_TAG_EXPOSURE_PROGRAM:
                sct = FXD_EXIF_EXPOSURE_PROGRAM;
                break;
            case EXIF_TAG_SENSING_METHOD: 
                sct = FXD_EXIF_SENSING_METHOD;
                break;
            case EXIF_TAG_ORIENTATION: 
                sct = FXD_EXIF_ORIENTATION;
                break;
            case EXIF_TAG_METERING_MODE: 
                sct = FXD_EXIF_METERING_MODE;
                break;
            case EXIF_TAG_YCBCR_POSITIONING:
                sct = FXD_EXIF_YCBCR_POSITIONING;
                break;
            case EXIF_TAG_COMPRESSION: 
                sct = FXD_EXIF_COMPRESSION;
                break;
            case EXIF_TAG_LIGHT_SOURCE:
                sct = FXD_EXIF_LIGHT_SOURCE;
                break;
            case EXIF_TAG_RESOLUTION_UNIT:
            case EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT:
                sct = FXD_UNIT_NAME_LENGTH;
                break;
            case EXIF_TAG_X_RESOLUTION:
            case EXIF_TAG_Y_RESOLUTION:
            case EXIF_TAG_FOCAL_PLANE_X_RESOLUTION:
            case EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION:
                sct = FXD_BINARY_NATIVE_EA;
                break;
            case EXIF_TAG_MAKE:
            case EXIF_TAG_MODEL:
            case EXIF_TAG_IMAGE_DESCRIPTION:
            case EXIF_TAG_SOFTWARE:
            case EXIF_TAG_ARTIST:
                sct = XSD_BASIC_STRING;
                break;
            case EXIF_TAG_DATE_TIME:
            case EXIF_TAG_DATE_TIME_ORIGINAL:
            case EXIF_TAG_DATE_TIME_DIGITIZED:
                sct = FXD_EXIF_DATETIME_STRING;
                break;
            default:
                switch (entry->format) {
                case EXIF_FORMAT_RATIONAL:
                case EXIF_FORMAT_SRATIONAL:
                    sct = XSD_BASIC_DOUBLE;
                    break;
                case EXIF_FORMAT_BYTE:
                case EXIF_FORMAT_SHORT:
                case EXIF_FORMAT_LONG:
                case EXIF_FORMAT_SLONG:
                    sct = XSD_BASIC_INT;
                    break;
                default:
                    sct = FXD_BINARY_NATIVE_EA;
                    break;
                }
                break;
            }
        }

        LG_EXIF_D << "EAGenerator_Exif::generateAttribute() ctx:" << a->getURL()
                  << " entry:" << toVoid( entry )
                  << " rdn:" << rdn
                  << " sct:" << sct
                  << endl;
        
        a->addAttribute( rdn,
                         (EA_Atom*)ExifByteArrayAttribute::Create( a, rdn, forceCreate ),
                         sct
            );

        //
        // Attach schema
        //
        if( !starts_with( rdn, "schema:" ))
        {
            string schema_name      = "schema:" + rdn;

            if( !a->isAttributeBound( schema_name, false ))
            {
                if( ! a->addAttribute(
                        schema_name,
                        EA_Atom_ReadOnly::GetIStream_Func_t( getSchema ),
                        XSD_SCHEMA ))
                {
                    ostringstream ss;
                    ss << "Can't create schema attribute for rdn:" << rdn
                       << " schema_name:" << schema_name
                       << " forceCreate:" << forceCreate;
                    LG_EXIF_D << tostr(ss) << endl;
                    Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
                }
            }
        }
    }

/********************************************************************************/
/********************************************************************************/

    static void generateAttributeForAll( ExifData* ed,
                                         EAGenerator_Exif* exif,
                                         fh_context ctx,
                                         stringlist_t& exif_ea_names )
    {

        for( unsigned int edi = 0; edi < EXIF_IFD_COUNT; edi++)
        {
            ExifContent *content = ed->ifd[ edi ];

            for( unsigned int ci = 0; ci < content->count; ci++)
            {
                ExifEntry* entry = content->entries[ci];

                string rdn = "exif:";
                if( !entry || !entry->tag )
                    continue;
                const char* tag_CSTR = my_exif_tag_get_name( entry );
                if( !tag_CSTR )
                    continue;
                
                rdn += Util::convertCapsToDashLower( tag_CSTR );

                LG_EXIF_D << "generateAttributeForAll tag:"
                          << my_exif_tag_get_name( entry )
                          << " rdn:" << rdn
                          << endl;
                
                exif->generateAttribute( entry, ctx, rdn );
                exif_ea_names.push_back( rdn );
            }
        }
    }

/********************************************************************************/
/********************************************************************************/

    void
    EAGenerator_Exif::Brew( const fh_context& a )
    {
        if( starts_with( a->getURL(), "gstreamer:" ))
            return;
       if( starts_with( a->getURL(), "sane:" ))
           return;
       if( starts_with( a->getURL(), "google:" ))
           return;
       if( starts_with( a->getURL(), "gdrive:" ))
           return;
        
        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );
    
        try
        {
            LG_EXIF_D << "EAGenerator_Exif::Brew() ctx:" << a->getURL() << endl;

            {
                string eaname;

                eaname = "exif:date-time-original-epoch";
                if( !a->isAttributeBound( eaname, false ))
                    a->addAttribute( eaname,
                                     this, &_Self::getDateTimeEpoch,
                                     FXD_UNIXEPOCH_T );

                eaname = "exif:date-time-original-xml";
                if( !a->isAttributeBound( eaname, false ))
                    a->addAttribute( eaname,
                                     this, &_Self::getDateTimeXML,
                                     FXD_UNIXEPOCH_T );
                
            }
            
#ifdef HAVE_EPEG
            if( !a->isAttributeBound( THUMBNAIL_EANAME_HAS_EXIF_THUMB, false ))
                a->addAttribute( THUMBNAIL_EANAME_HAS_EXIF_THUMB,
                                 this, &_Self::getHasThumb,
                                 XSD_BASIC_BOOL );

            if( !a->isAttributeBound( THUMBNAIL_EANAME_UPDATE, false ))
                a->addAttribute( THUMBNAIL_EANAME_UPDATE,
                                 this, &_Self::getThumbUpdate,
                                 this, &_Self::getThumbUpdate,
                                 this, &_Self::getThumbUpdateClosed,
                                 XSD_BASIC_BOOL );
            
            if( !a->isAttributeBound( THUMBNAIL_EANAME_RGBA, false ))
                a->addAttribute( THUMBNAIL_EANAME_RGBA,
                                 this, &_Self::getThumbRGBAStream,
                                 FXD_BINARY_RGBA32 );
            
            if( !a->isAttributeBound( THUMBNAIL_EANAME_W, false ))
                a->addAttribute( THUMBNAIL_EANAME_W,
                                 this, &_Self::getThumbWidthStream,
                                 FXD_WIDTH_PIXELS,  true );
            
            if( !a->isAttributeBound( THUMBNAIL_EANAME_H, false ))
                a->addAttribute( THUMBNAIL_EANAME_H,
                                 this, &_Self::getThumbHeightStream,
                                 FXD_HEIGHT_PIXELS, true );
#endif
            
//            a->setHasDynamicAttributes( true );
            stringlist_t exif_ea_names;

            LG_EXIF_D << "EAGenerator_Exif::Brew(2) ctx:" << a->getURL() << endl;

            
            if( fh_cachedExifData edholder = getExifData( a ) )
            {
                LG_EXIF_D << "EAGenerator_Exif::Brew(3) ctx:" << a->getURL() << endl;
                ExifData* ed = *edholder;
                generateAttributeForAll( ed, this, a, exif_ea_names );
                
                {
                    string rdn;
                    XSDBasic_t sct = FXD_BINARY_NATIVE_EA;

                    rdn = "exif:flash-fired";
                    sct = XSD_BASIC_BOOL;
                    a->addAttribute( rdn, this, &_Self::getFlashFiredStream, sct );
                    exif_ea_names.push_back( rdn );
                    
                }
                
                exif_ea_names.push_back( THUMBNAIL_EANAME_HAS_EXIF_THUMB );
                exif_ea_names.push_back( THUMBNAIL_EANAME_UPDATE );
                exif_ea_names.push_back( THUMBNAIL_EANAME_RGBA );
                exif_ea_names.push_back( THUMBNAIL_EANAME_W );
                exif_ea_names.push_back( THUMBNAIL_EANAME_H );

                exif_ea_names.sort();
            
                /*
                 * Add a new EA showing the names of all the exif EA
                 */
                a->addAttribute( "exif:ea-names",
                                 Util::createCommaSeperatedList( exif_ea_names ),
                                 FXD_EANAMES,
                                 true );
            }
            LG_EXIF_D << "EAGenerator_Exif::Brew(4) ctx:" << a->getURL() << endl;
        }
        catch( exception& e )
        {
            LG_EXIF_ER << "Failed to load xfs EA, error:" << e.what() << endl;
        }
    }

    bool
    EAGenerator_Exif::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        static bool brewing = false;
        if( brewing )
            return false;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        LG_EXIF_D << "tryBrew() "
                  << " url:" << ctx->getURL()
                  << " eaname:" << eaname
                  << endl;
    
        bool      ret = false;
        fh_cachedExifData   edholder = getExifData( ctx );
        ExifData*                 ed = *edholder;

        if (!ed)
        {
            LG_EXIF_D << "tryBrew() no exif data found in file:" << ctx->getURL() << endl;
            return false;
        }
        string tag = stripPrefix( eaname );
        
#ifdef HAVE_EPEG
        if( eaname == THUMBNAIL_EANAME_HAS_EXIF_THUMB )
        {
            if( !ctx->isAttributeBound( eaname, false ))
            {
                ctx->addAttribute( THUMBNAIL_EANAME_HAS_EXIF_THUMB,
                                 this, &_Self::getHasThumb,
                                 XSD_BASIC_BOOL );
            }
            return true;
        }
        if( eaname == THUMBNAIL_EANAME_UPDATE )
        {
            if( !ctx->isAttributeBound( eaname, false ))
            {
                ctx->addAttribute( THUMBNAIL_EANAME_UPDATE,
                                   this, &_Self::getThumbUpdate,
                                   this, &_Self::getThumbUpdate,
                                   this, &_Self::getThumbUpdateClosed,
                                   XSD_BASIC_BOOL );
            }
            return true;
        }
        
        
        
        /**
         * Handle fast loading of thumbnail data from EXIF chunk with epeg.
         */
        if( eaname == THUMBNAIL_EANAME_RGBA 
            || eaname == THUMBNAIL_EANAME_W || eaname == THUMBNAIL_EANAME_H )
        {
            
            if( !ed->data )
                return false;

            if( !ctx->isAttributeBound( eaname, false ))
                ctx->addAttribute( THUMBNAIL_EANAME_RGBA,
                                 this, &_Self::getThumbRGBAStream,
                                 FXD_BINARY_RGBA32 );
            if( !ctx->isAttributeBound( eaname, false ))
                ctx->addAttribute( THUMBNAIL_EANAME_W,
                                 this, &_Self::getThumbWidthStream,
                                 FXD_WIDTH_PIXELS,  true );
            if( !ctx->isAttributeBound( eaname, false ))
                ctx->addAttribute( THUMBNAIL_EANAME_H,
                                 this, &_Self::getThumbHeightStream,
                                 FXD_HEIGHT_PIXELS, true );
            return true;
        }
#endif        

    
        ExifEntry* entry = findTagIfExists( edholder, tag );
    
        if( entry )
        {
            /* we have an attribute */
            ret = true;
            generateAttribute( entry, ctx, eaname );
            return ret;
        }
    
        return ret;
    }



    fh_attribute
    EAGenerator_Exif::CreateAttr( const fh_context& a,
                                  const string& rdn,
                                  fh_context md )
    {
        try
        {
            LG_EXIF_D << "EAGenerator_Exif::CreateAttr() rdn:" << rdn << endl;

            string    tagstr = stripPrefix( rdn );
            ExifTag      tag = exif_tag_from_string( tagstr.c_str() );
            int          ifd = toint(getStrSubCtx( md, "ifd", tostr( EXIF_IFD_EXIF ) ));
            fh_cachedExifData   edholder = getExifData( a );
            ExifData*                 ed = *edholder;

            ExifEntry* e = exif_content_get_entry( ed->ifd[ ifd ], tag );
            if (!e) {
                e = exif_entry_new ();
                exif_content_add_entry (ed->ifd[ifd], e);
                exif_entry_initialize (e, tag);
            }

            generateAttribute( 0, a, rdn, true );
            return a->getAttribute( rdn );
        }
        catch( FerrisCreateAttributeFailed& e )
        {
            throw e;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << e.what();
            LG_EXIF_D << tostr(ss) << endl;
            Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
        }
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            return new EAGenerator_Exif();
        }
    };



 
};
