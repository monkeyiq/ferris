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

    $Id: SchemaSupport.cpp,v 1.11 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <SchemaSupport.hh>
#include <Resolver_private.hh>
#include <Native.hh>

using namespace std;

namespace Ferris
{
    const char* SCHEMA_BASE = "schema://xsd/attributes/";

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

//     struct StatelessEAMetaData
//     {
//         XSDBasic_t sct;
//         string     justification;
//         int        displayWidth;
        
//         StatelessEAMetaData( XSDBasic_t sct = XSD_UNKNOWN,
//                              string justification = "left",
//                              int displayWidth = 0 )
//             : sct( sct ), justification( justification ), displayWidth(displayWidth)
//             {
//             }
//     };
//     typedef map< string, StatelessEAMetaData > StatelessEAMetaDataCache_t;
//     static StatelessEAMetaDataCache_t&
//     getStatelessEAMetaDataCache()
//     {
//         static StatelessEAMetaDataCache_t ret;
//         return ret;
//     }

    
//     static const StatelessEAMetaData*
//     ensureStatelessEAMetaDataCachePopulated(
//         const fh_context& c, const std::string& eaname )
//     {
// //        cerr << "ensureStatelessEAMetaDataCachePopulated() eaname:" << eaname << endl;

//         StatelessEAMetaDataCache_t::const_iterator sleaiter =
//             getStatelessEAMetaDataCache().find( eaname );

//         if( sleaiter != getStatelessEAMetaDataCache().end() )
//         {
//             return &(sleaiter->second);
//         }
        
//         fh_context sc = c->getSchema( eaname );
//         XSDBasic_t sct = (toType<XSDBasic_t>(getStrAttr( sc, "ferrisenum", tostr(sct) )));
//         string justification = getStrAttr( sc, SCHEMA_JUSTIFICATION, "left" );
//         int    displayWidth  = toint(getStrAttr( sc, SCHEMA_DEFAULT_DISPLAY_WIDTH, "0" ));
        
//         sleaiter = getStatelessEAMetaDataCache().insert(
//             make_pair( eaname, StatelessEAMetaData( sct, justification, displayWidth ))).first;
//         return &(sleaiter->second);
//     }    

//     static void
//     ensureStatelessEAMetaDataCachePopulated(
//         const fh_context& c, const stringset_t& sl )
//     {
//         for( stringset_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
//         {
//             string eaname = *si;

//             cerr << "ensureStatelessEAMetaDataCachePopulated() eaname:" << eaname << endl;
            
//             fh_context sc = c->getSchema( eaname );
//             XSDBasic_t sct = (toType<XSDBasic_t>(getStrAttr( sc, "ferrisenum", tostr(sct) )));
//             string justification = getStrAttr( sc, SCHEMA_JUSTIFICATION, "left" );
//             int    displayWidth  = toint(getStrAttr( sc, SCHEMA_DEFAULT_DISPLAY_WIDTH, "0" ));
            
//             getStatelessEAMetaDataCache()[ eaname ] =
//                 StatelessEAMetaData( sct, justification, displayWidth );
//         }
//     }    
    
//     static void
//     ensureStatelessEAMetaDataCachePopulated( const fh_context& c )
//     {
//         static bool virgin = true;
//         if( virgin )
//         {
//             virgin = false;
//             ensureStatelessEAMetaDataCachePopulated(
//                 c, getNativeStatelessEANames() );
//             ensureStatelessEAMetaDataCachePopulated(
//                 c, Context::getContextClassStatelessEANames() );
//         }
//     }
        
            
    XSDBasic_t getSchemaType( const fh_context& c, const std::string& eaname, XSDBasic_t sct )
    {
//         if( c->getIsNativeContext() )
//         {
//             ensureStatelessEAMetaDataCachePopulated( c );
//             StatelessEAMetaDataCache_t::const_iterator sleaiter =
//                 getStatelessEAMetaDataCache().find( eaname );
            
//             if( sleaiter != getStatelessEAMetaDataCache().end() )
//                 return sleaiter->second.sct;
//         }

        fh_context sc = c->getSchemaOrDefault( eaname, sct  );
        return XSDBasic_t(toType<long>(getStrAttr( sc, "ferrisenum", tostr(sct) )));
    }
    
    std::string getSchemaJustification( const fh_context& c,
                                        const std::string& eaname,
                                        const std::string& def )
    {
//         if( c->getIsNativeContext() )
//         {
//             const StatelessEAMetaData* sldata
//                 = ensureStatelessEAMetaDataCachePopulated( c, eaname );
//             return sldata->justification;
//         }

        fh_context sc = c->getSchema( eaname );
        if( !isBound( sc ) )
            return def;
        
        return getStrAttr( sc, SCHEMA_JUSTIFICATION, def );
    }
    
    int getSchemaDisplayWidth( const fh_context& c,
                               const std::string& eaname,
                               int def )
    {
//         if( c->getIsNativeContext() )
//         {
//             const StatelessEAMetaData* sldata
//                 = ensureStatelessEAMetaDataCachePopulated( c, eaname );
//             return sldata->displayWidth;
//         }

        fh_context sc = c->getSchema( eaname );
        if( !isBound(sc) )
            return def;
        
        return toint(getStrAttr( sc, SCHEMA_DEFAULT_DISPLAY_WIDTH, tostr(def) ));
    }


    int getSchemaDisplayPrecision( const fh_context& c,
                                   const std::string& eaname,
                                   int def )
    {
        fh_context sc = c->getSchema( eaname );
        if( !isBound(sc) )
            return def;
        
        return toint(getStrAttr( sc, SCHEMA_DEFAULT_DISPLAY_PRECISION, tostr(def) ));
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    typedef map< XSDBasic_t, string > typeToURLMap_t;
    static typeToURLMap_t& getTypeToURLMap()
    {
        static typeToURLMap_t ret;

        if( ret.empty() )
        {
            ret[ XSD_UNKNOWN ]              = "unknown";
            ret[ XSD_SCHEMA ]               = "schema";
            ret[ XSD_BASIC_BOOL ]           = "boolean";
            ret[ XSD_BASIC_FLOAT ]          = "float";
            ret[ XSD_BASIC_DOUBLE ]         = "double";
            ret[ XSD_BASIC_STRING ]         = "string";
            ret[ XSD_BASIC_INT ]            = "decimal/integer/long/int";
            ret[ XSD_BASIC_INTEGER ]        = "decimal/integer";
            ret[ FXD_MODE_T ]               = "decimal/integer/long/fs/mode";
            ret[ FXD_UNIXEPOCH_T ]          = "decimal/integer/long/fs/time";
            ret[ FXD_INODE_T ]              = "decimal/integer/long/fs/inode";
            ret[ FXD_MIMETYPE ]             = "string/mimetype";
            ret[ FXD_GID_T ]                = "decimal/integer/long/fs/gid";
            ret[ FXD_UID_T ]                = "decimal/integer/long/fs/uid";
            ret[ FXD_USERNAME ]             = "string/username";
            ret[ FXD_GROUPNAME ]            = "string/groupname";
            ret[ FXD_PID ]                  = "decimal/integer/long/fs/pid";
            ret[ FXD_FILESIZE ]             = "decimal/integer/width64/filesize";
            ret[ FXD_URL ]                  = "string/url";
            ret[ FXD_URL_IMPLICIT_RESOLVE ] = "string/url/implicitresolve";
            ret[ FXD_MACHINE_NAME ]         = "string/machinename";
            ret[ FXD_FSID_T ]               = "decimal/integer/long";
            ret[ FXD_INT32 ]                = "decimal/integer/width32";
            ret[ FXD_UINT32 ]               = "decimal/integer/unsigned/width32";
            ret[ FXD_LONG ]                 = "decimal/integer/width32";
            ret[ FXD_BINARY ]               = "binary";
            ret[ FXD_BINARY_RGBA32 ]        = "binary/rgba32bpp";
            ret[ FXD_BINARY_NATIVE_EA ]     = "binary/nativeea";
            ret[ FXD_BINARY_PGMPIPE ]       = "binary/pgmpipe";
            ret[ FXD_BINARY_A52PIPE ]       = "binary/a52pipe";
            ret[ FXD_EANAMES ]              = "stringlist/eanames";
            ret[ FXD_DIGEST ]               = "string/digest";
            ret[ FXD_XMLDOCSTRING ]         = "string/xmldoc";
            ret[ FXD_PIXELCOUNT ]           = "decimal/integer/pixelcount";
            ret[ FXD_WIDTH_PIXELS ]         = "decimal/integer/pixelcount/width";
            ret[ FXD_HEIGHT_PIXELS ]        = "decimal/integer/pixelcount/height";
            ret[ FXD_PRIMARY_KEY ]          = "string/primarykey";
            ret[ FXD_PRIMARY_KEY_REAL ]     = "string/primarykey/real";
            ret[ FXD_PRIMARY_KEY_VIRTUAL ]  = "string/primarykey/virtual";
            ret[ FXD_FTX_RANK ]             = "double/query/rank";
            ret[ FXD_FFILTER ]              = "string/ffilter";
            ret[ FXD_STR_IP4ADDR ]          = "string/ipv4address";
            ret[ FXD_IP4PORT ]              = "decimal/integer/unsigned/width16/ipv4port";
            ret[ FXD_DISTINGUISHED_PERSON ] = "string/distinguishedperson";
            ret[ FXD_CIPHER_NAME ]          = "string/ciphername";
            ret[ FXD_CIPHER_VERSION ]       = "string/cipherversion";
            ret[ FXD_CIPHER_NAME_LIST ]     = "stringlist/ciphername";
            ret[ FXD_CIPHER_BITS ]          = "decimal/integer/unsigned/width16/cipherbits";
            ret[ FXD_STRINGLIST ]           = "stringlist";
            ret[ FXD_UNIXEPOCH_STRING ]     = "string/unixepoch";
            ret[ FXD_MODE_STRING_T ]        = "string/modestring";
            ret[ FXD_URLLIST ]              = "stringlist/urllist";
            ret[ FXD_URL_IMPLICIT_RESOLVE_FILESYSTEM ] = "string/url/implicitresolvefs";

            ret[ FXD_LATLONG ]              = "double/latlong";
            ret[ FXD_LATITUDE ]             = "double/latlong/latitude";
            ret[ FXD_LONGITUDE ]            = "double/latlong/longitude";
        }
        return ret;
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    inline static fh_stringstream SL_Schema_T( Context* c, const std::string& typepath )
    {
        fh_stringstream ret;
        ret << SCHEMA_BASE << typepath << endl;
        return ret;
    }

    struct FERRISEXP_DLLLOCAL GetSchemaURLIStream
    {
        XSDBasic_t sct;
        GetSchemaURLIStream( XSDBasic_t sct )
            :
            sct( sct )
            {
            }

        typedef Loki::Functor< fh_istream,
                               LOKI_TYPELIST_3( Context*,
                                           const std::string&,
                                           EA_Atom* ) > SchemaReadFunctor_t;
    SchemaReadFunctor_t getFunctor()
    {
        SchemaReadFunctor_t f( this, &GetSchemaURLIStream::getIStream );
        return f;
    }
    
        string getSchemaURL( Context* c, const std::string& rdn )
        {
                typeToURLMap_t& m           = getTypeToURLMap();
                typeToURLMap_t::iterator mi = m.find( sct );
                string typepath             = "";

                if( mi != m.end() )
                    typepath = mi->second;
                else
                {
                    // PURE DEBUG
                    LG_SCHEMA_ER << "schema type:" << (guint32)sct
                                 << " rdn:" << rdn
                                 << " SCT IS NOT BOUND!" << endl;
                }

                stringstream ret;
                ret << SCHEMA_BASE << typepath << endl;

                LG_SCHEMA_D << "GetSchemaURLIStream() sct:" << sct << " rdn:" << rdn
                            << " GIVES:" << tostr(ret) << endl;
                
                return ret.str();
        }

        fh_istream getIStream( Context* c,
                               const std::string& rdn,
                               EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << getSchemaURL( c, rdn );
                return ss;
                // typeToURLMap_t& m           = getTypeToURLMap();
                // typeToURLMap_t::iterator mi = m.find( sct );
                // string typepath             = "";

                // if( mi != m.end() )
                //     typepath = mi->second;
                // else
                // {
                //     // PURE DEBUG
                //     LG_SCHEMA_ER << "schema type:" << (guint32)sct
                //                  << " rdn:" << rdn
                //                  << " SCT IS NOT BOUND!" << endl;
                // }

                // fh_stringstream ret;
                // ret << SCHEMA_BASE << typepath << endl;

                // LG_SCHEMA_D << "GetSchemaURLIStream() sct:" << sct << " rdn:" << rdn
                //             << " GIVES:" << tostr(ret) << endl;
                
                // return ret;
            }
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    stringlist_t getSchemaDefaultSort( fh_context sc, bool onlyGetFirst )
    {
        stringlist_t ret;
        string s;
        stringstream ss;
        ss << getStrAttr( sc, SCHEMA_DEFAULTSORT, "binary" );

        LG_SCHEMA_D << "getSchemaDefaultSort() sc:" << sc->getURL() << " defaultsort:" << tostr(ss) << endl;
        
        while( getline( ss, s, ',' ))
        {
            ret.push_back( s );
            if( onlyGetFirst )
                break;
        }
        
        return ret;
    }
    stringlist_t getSchemaDefaultSortList( fh_context sc )
    {
        return getSchemaDefaultSort( sc, false );
    }
    
    string getSchemaDefaultSort( fh_context sc )
    {
        return getSchemaDefaultSort( sc, true ).front();
    }
    
    

    
    XSDBasic_t maskOffXSDMeta( XSDBasic_t t )
    {
//        cerr << "maskOffXSDMeta() in:" << t << " out1:" << (t & 0x00FFFFFFL) << endl;
        return (XSDBasic_t)(t & 0x00FFFFFF);
    }

    typedef Loki::Functor< fh_istream,
                           LOKI_TYPELIST_3( Context*,
                                       const std::string&,
                                       EA_Atom* ) > SchemaReadFunctor_t;
    struct FERRISEXP_DLLLOCAL StateLessAttacher
    {
        static void attach( Context* c,
                            const std::string& eaname,
                            const SchemaReadFunctor_t& f )
            {
                c->tryAddStateLessAttribute( eaname, f );
            }
    };
    struct FERRISEXP_DLLLOCAL GenericAttacher
    {
        static void attach( Context* c,
                            const std::string& eaname,
                            const SchemaReadFunctor_t& f )
            {
                c->addAttribute( eaname, f );
            }
    };
    
    template <class Attacher>
    static void attachSchema( Context* c, const std::string& rawname, XSDBasic_t t )
    {
        string sea = "schema:" + rawname;

        if( starts_with( rawname, "schema:" ))
            return;

        LG_SCHEMA_D << endl << "  ******* " << endl
                    << "attachSchema() c:" << c->getURL() << " sea:" << sea
                    << " t:" << t << " mask(t):" << maskOffXSDMeta( t )
                    << endl;

        t = maskOffXSDMeta( t );
        if( !t )
            return;
        
        typedef map< XSDBasic_t, GetSchemaURLIStream* > Functions_t;
        static Functions_t Functions;
        Functions_t::iterator fi = Functions.find( t );
        
        if( fi == Functions.end()  )
        {
            LG_SCHEMA_D << "attachSchema(2) c:" << c->getURL() << " sea:" << sea << " t:" << t << endl;

            GetSchemaURLIStream* obj = new GetSchemaURLIStream( t );
            Functions[ t ] = obj;
            fi = Functions.find( t );
        }

        if( fi != Functions.end() )
        {
//            fh_istream iss     = fi->second->getIStream(c,rawname,0);
//            string     readval = getFirstLine( iss );
            string readval = fi->second->getSchemaURL( c, rawname );
            
            LG_SCHEMA_D << "attachSchema(about to attach()) c:" << c->getURL() << " sea:" << sea << " t:" << t
                        << " GIVES:" << readval << endl;

            Attacher::attach( c,  sea,
                              SchemaReadFunctor_t( fi->second,
                                                   &GetSchemaURLIStream::getIStream ) );

            LG_SCHEMA_D << "attachSchema(final test) getattr:" << getStrAttr( c, sea, "no" ) << endl;
            
        }
        
    }


    void attachStatelessSchema( Context* c, const std::string& rawname, XSDBasic_t t )
    {
        attachSchema<StateLessAttacher>( c, rawname, t );
    }


    void attachGenericSchema( Context* c, const std::string& rawname, XSDBasic_t t )
    {
        attachSchema<GenericAttacher>( c, rawname, t );
    }

    XSDBasic_t getSchemaType( const fh_context& sc, XSDBasic_t sct )
    {
        return XSDBasic_t(toType<long>(getStrAttr( sc, "ferrisenum", tostr(sct) )));
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    namespace Factory 
    {
        string getSchemaURLForType( XSDBasic_t sct )
        {
            const typeToURLMap_t& ttu = getTypeToURLMap();
            typeToURLMap_t::const_iterator ti = ttu.find( sct );
            if( ti == ttu.end() )
            {
                return "schema://xsd/attributes/unknown";
            }
            return ti->second;
        }
        
        
        xsdtypemap_t& makeBasicTypeMap( xsdtypemap_t& m )
        {
            typeToURLMap_t ttu = getTypeToURLMap();
            for( typeToURLMap_t::iterator ti = ttu.begin(); ti != ttu.end(); ++ti )
            {
                fh_stringstream ss;
                ss << SCHEMA_BASE << ti->second;
//                 LG_SCHEMA_D << "type:" << ti->first << " is at url:" << tostr(ss) << endl;

                // Note that RESOLVE_CLOSEST works for this, but we then don't get an indivial uuid
                // and the ferrisenum reverse map doesn't work properly. thus we should have exact
                // resolution. We allow non exact currently but slap the user for doing it.
                m[ ti->first ] = Resolve( tostr(ss), RESOLVE_CLOSEST );

                //
                // Check to make sure we got exact, complain if it is not so.
                //
                bool issueWarning = false;
                try
                {
                    fh_context exact = Resolve( tostr(ss) );
                    if( exact != m[ ti->first ] )
                        issueWarning = true;
                }
                catch( ... )
                {
                    issueWarning = true;
                }

                if( issueWarning )
                {
                    LG_SCHEMA_W << "SCHEMA WARNING: for type" << ti->first << endl
                                << " there should be a schema at:" << tostr(ss) << endl
                                << " using closest match at:" << m[ ti->first ]->getURL() << endl;
                }
                
//                 LG_SCHEMA_D << "type2:" << ti->first << " is at url:" << m[ ti->first ]->getURL() << endl;
            }

            return m;
        }
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL SchemaContextSchemaVFS_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        
        SchemaContextSchemaVFS_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("schema");
                RootContextFactory::Register( "schema", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                try
                {
                    static fh_context ret = 0;
                    static bool v = true;
                    if( v )
                    {
//                        Time::Benchmark bm( "getting schema" );
                        v = false;

                        static fh_context base = Resolve( "~/.ferris/schema.xml" );
                        static fh_context c1 = base;
                        static fh_context c2 = Factory::makeInheritingEAContext( base );
                        ret = c2;
                    }
                    
                    return ret;
                }
                catch( RootContextCreationFailed& e )
                {
                    throw;
                }
                catch( std::exception& e )
                {
                    fh_stringstream ss;
                    ss << "Schema resolution error:" << e.what() << endl;
                    Throw_RootContextCreationFailed( tostr(ss), 0 );
                }
            }
    };
    typedef Loki::SingletonHolder< SchemaContextSchemaVFS_RootContextDropper,
            Loki::CreateUsingNew, Loki::NoDestroy > link_ctx_schema_singleton;
    static RootContextDropper& schemad = link_ctx_schema_singleton::Instance();

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    
};
