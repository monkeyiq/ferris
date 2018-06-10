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

    $Id: Configuration.cpp,v 1.19 2010/11/17 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Configuration.hh>
#include <Configuration_private.hh>
#include <General.hh>
#include <Trimming.hh>

#ifdef HAVE_EDB
#include <Edb.h>
#endif


namespace Ferris
{
    using namespace std;
    using namespace ::STLdb4;

    std::string GET_FDB_GENERAL()
    {
        return getDotFerrisPath() + "/general.db";
    }
    std::string GET_FDB_FERRISDEV()
    {
        return getDotFerrisPath() + "/general.db";
    }
    FERRISEXP_API std::string& GET_FDB_LOGGING()
    {
        static string s = getDotFerrisPath() + "/logging.db";
        return s;
    }
    std::string GET_FDB_SECURE()
    {
        return getDotFerrisPath() + "/secure.db";
    }
    std::string GET_FDB_CACHE()
    {
        return getDotFerrisPath() + "/cache.db";
    }
    std::string GET_CREATEHISTORY_RELATIVE()
    {
        return getDotFerrisPath() + "cache.db/create-history";
    }
    const std::string FDB_GENERAL   = GET_FDB_GENERAL();
    const std::string FDB_FERRISDEV = getDotFerrisPath() + "general.db";
    const std::string FDB_LOGGING = GET_FDB_LOGGING();
    const std::string FDB_SECURE  = getDotFerrisPath() + "secure.db";
    const std::string FDB_CACHE   = getDotFerrisPath() + "cache.db";
    const std::string CREATEHISTORY_RELATIVE = getDotFerrisPath() + "cache.db/create-history";


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    string get_db4_string( fh_database db,
                           const string& k,
                           const string& def,
                           bool throw_for_errors )
    {
        if( !db )
        {
            if( !throw_for_errors )
                return def;
            fh_stringstream ss;
            ss << "get_db4_string() invalid database handle given."
               << " k:" << k
               << " def:" << def
               << endl;
            Throw_Db4Exception( tostr(ss), 0, 0 );
        }
        
        try
        {
            string payload;
            db->get( k, payload );
//            cerr << "get_db4_string k:" << k << " ret.l:" << payload.length() << endl;
            return payload;
        }
        catch( exception& e )
        {
            if( throw_for_errors )
            {
                fh_stringstream ss;
                ss << "Can not get data for db4 key:" << k
                   << " e:" << e.what();
                Throw_Db4KeyNotFound( tostr(ss), 0 );
            }
            return def;
        }
        return def;
        
//         try
//         {
//             Dbt key( (void*)k.data(), k.length() );
//             Dbt data( 0, 0 );
//             int rc = db->get( 0, &key, &data, 0 );
            
//             if( 0 != rc )
//             {
//                 if( throw_for_errors )
//                 {
//                     fh_stringstream ss;
//                     ss << "Can not get data for db4 key:" << k
//                        << " data not found";
//                     Throw_Db4KeyNotFound( tostr(ss), 0 );
//                 }
//                 return def;
//             }
        
//             if( data.get_size() )
//             {
//                 stringstream ss;
//                 ss.write( (const char*)data.get_data(), data.get_size() );
//                 string ret = tostr(ss);
//                 return ret;
//             }
        
//         }
//         catch( DbException& e )
//         {
//             if( throw_for_errors )
//             {
//                 fh_stringstream ss;
//                 ss << "Can not get data for db4 key:" << k;
//                 Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
//             }
//         }
//         return def;
    }
    

    string get_db4_string( const string& filename,
                           const string& k,
                           const string& def,
                           bool throw_for_errors,
                           bool read_only )
    {
        try
        {
//            fh_database db = new Database( CleanupURL(filename) );

            fh_database db = new Database( CleanupURL(filename), "", read_only );

//             u_int32_t flags = 0;
//             if( read_only ) flags |= DB_RDONLY;
//             else            flags |= DB_CREATE;
            
//             fh_database db = new Database( DB_UNKNOWN, CleanupURL(filename), "", flags );


            string ret = get_db4_string( db, k, def, throw_for_errors );
            return ret;
        }
        catch( dbException& e )
        {
            if( throw_for_errors )
            {
                fh_stringstream ss;
                ss << "Can not get data for db4 key:" << k
                   << " for file:" << filename
                   << " e:" << e.what();
                Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
            }
        }
        return def;
    }

    string get_db4_string( const string& filename,
                           const string& k,
                           const string& def,
                           bool throw_for_errors )
    {
        return get_db4_string( filename, k, def, throw_for_errors, false );
    }
    
    
    static fh_database getCachedDb4( const string& edbname_relhome,
                                     string filename,
                                     const string& k,
                                     bool throw_for_errors,
                                     const string& opname )
    {
        /* ~/ files are assumed to be config files and should thus be cached. */
        typedef map< string, fh_database > DotFileCache_t;
//        static DotFileCache_t dotCache;
        typedef Loki::SingletonHolder< DotFileCache_t,
            Loki::CreateUsingNew, Loki::NoDestroy > dotCache;

        DotFileCache_t::iterator iter = dotCache::Instance().find( edbname_relhome );
        if( iter == dotCache::Instance().end() )
        {
            fh_database db = 0;
            try
            {
                db = new Database( filename );
//                db->set_cachesize( 0, 128*1024, 1 );
            }
            catch( dbException& e )
            {
                if( throw_for_errors )
                {
                    fh_stringstream ss;
                    ss << "Can not " << opname << " data for db4 key:" << k
                       << " for file:" << filename;
                    Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
                }
                return 0;
            }

            iter = dotCache::Instance().insert( make_pair( edbname_relhome, db ) ).first;
        }
        return iter->second;
            

            
//             Db* db = 0;
//             try
//             {
//                 db = new Db( 0, 0 );
//                 db->open( filename.c_str(), 0, DB_UNKNOWN, 0, 0 );
//             }
//             catch( DbException& e )
//             {
// //                 cerr << "Can not " << opname << " data for db4 key:" << k
// //                      << " for file:" << filename
// //                      << " reason:" << e.what()
// //                      << endl;
                
//                 if( throw_for_errors )
//                 {
//                     fh_stringstream ss;
//                     ss << "Can not " << opname << " data for db4 key:" << k
//                        << " for file:" << filename;
//                     Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
//                 }
//                 return 0;
//             }

//             iter = dotCache.insert( make_pair( edbname_relhome, db ) ).first;
//         }

// //         cerr << "getCachedDb4() rel:" << edbname_relhome
// //              << " fn:" << filename
// //              << " db:" << (void*)iter->second
// //              << endl;
        
//         return iter->second;
    }
    

    /**
     * Note that as this function is called from the logging code, it can not use any
     * LG_XXX_D logging because it might create a cyclic loop.
     *
     */
    string getConfigString( const string& dbname_relhome,
                            const string& k,
                            const string& def,
                            bool throw_for_errors )
    {
        string filename = Shell::getHomeDirPath_nochecks()+dbname_relhome;
        fh_database db = getCachedDb4( dbname_relhome, dbname_relhome, k, throw_for_errors, "get" );
        if( db )
        {
            return get_db4_string( db, k, def, throw_for_errors );
        }
        
        return def;
    }

    void setConfigString( const string& edbname_relhome,
                          const string& k,
                          const string& v,
                          bool throw_for_errors )
    {
        string filename = edbname_relhome;
        fh_database db = getCachedDb4( edbname_relhome, edbname_relhome, k, throw_for_errors, "set" );
        if( db )
            set_db4_string( db, k, v, throw_for_errors );
    }
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    void set_db4_string( fh_database db,
                         const string& k,
                         const string& v,
                         bool throw_for_errors )
    {
        if( !db )
        {
            if( !throw_for_errors )
                return;
            fh_stringstream ss;
            ss << "set_db4_string() invalid database handle given."
               << " k:" << k
               << " v:" << v
               << endl;
            Throw_Db4Exception( tostr(ss), 0, 0 );
        }
        
        
        try
        {
            db->set( k, v );
            db->sync();
        }
        catch( dbException& e )
        {
            if( throw_for_errors )
            {
                fh_stringstream ss;
                ss << "Can not get data for db4 key:" << k;
                Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
            }
        }
        return;
        
//         try
//         {
//             Dbt key ( (void*)k.data(), k.length() );
//             Dbt data( (void*)v.data(), v.length() );
//             db->put( 0, &key, &data, 0 );
//             db->sync( 0 );
// //             cerr << "set_db4_string k:" << k << " v:" << v << ":" << endl;
//         }
//         catch( DbException& e )
//         {
//             if( throw_for_errors )
//             {
//                 fh_stringstream ss;
//                 ss << "Can not get data for db4 key:" << k;
//                 Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
//             }
//         }
//         return;
    }

    void set_db4_string( const string& filename,
                         const string& k,
                         const string& v,
                         bool throw_for_errors )
    {
        fh_database db = 0;
        try
        {
            db = new Database( CleanupURL(filename) );
            set_db4_string( db, k, v, throw_for_errors );
        }
        catch( dbException& e )
        {
            if( throw_for_errors )
            {
                fh_stringstream ss;
                ss << "Can not set data for db4 key:" << k
                   << " for file:" << filename;
                Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
            }
        }
        return;
        
//         Db db( 0, 0 );
//         try
//         {
//             db.open( CleanupURL(filename).c_str(), 0, DB_UNKNOWN, 0, 0 );
//             set_db4_string( &db, k, v, throw_for_errors );
//             db.close(0);
//         }
//         catch( DbException& e )
//         {
//             db.close(0);
//             if( throw_for_errors )
//             {
//                 fh_stringstream ss;
//                 ss << "Can not set data for db4 key:" << k
//                    << " for file:" << filename;
//                 Throw_Db4Exception( tostr(ss), 0, e.get_errno() );
//             }
//         }
//         return;
    }

    

    static STLdb4::fh_database
    ensureFerrisConfigFileExists_sub( const std::string& parentPath,
                                      const std::string& dbName_maybeWithPrefixSlashes,
                                      const std::string& rdn )
    {
        STLdb4::fh_database ret = 0;

        fh_context c = ::Ferris::Shell::acquireContext( parentPath );
        fh_mdcontext md = new f_mdcontext();
        fh_mdcontext child = md->setChild( "db4", "" );
        child->setChild( "name", rdn );
        fh_context newc   = c->createSubContext( "", md );
        ret = new Database( newc->getURL() );
        return ret;
    }
    
    STLdb4::fh_database
    ensureFerrisConfigFileExists( const std::string& parentPath,
                                  const std::string& dbName_maybeWithPrefixSlashes )
    {
        STLdb4::fh_database ret = 0;
        
        string rdn = dbName_maybeWithPrefixSlashes;
        PrefixTrimmer trimmer;
        trimmer.push_back( "/" );
        rdn = trimmer( rdn );
            
        try
        {
//             cerr << "ensureFerrisConfigFileExists() pp:" << parentPath
//                  << " rdn:" << rdn
//                  << endl;
            string fullpath = parentPath + "/" + rdn;
            fh_context cfg = Shell::acquireContext( parentPath );
            ret = new Database();
            ret->create( DB_HASH, fullpath );
            ret->set( "foo", "bar" );
            ret->sync();
//             cerr << "ensureFerrisConfigFileExists() pp:" << parentPath
//                  << " rdn:" << rdn
//                  << " returning1"
//                  << endl;
            return ret;
        }
        catch( NoSuchSubContext& e )
        {
//             cerr << "ensureFerrisConfigFileExists() pp:" << parentPath
//                  << " rdn:" << rdn
//                  << " e:" << e.what()
//                  << endl;
            return ensureFerrisConfigFileExists_sub( parentPath,
                                                     dbName_maybeWithPrefixSlashes,
                                                     rdn );
        }
        catch( dbNonExist& e )
        {
//             cerr << "ensureFerrisConfigFileExists() pp:" << parentPath
//                  << " rdn:" << rdn
//                  << " e:" << e.what()
//                  << endl;
            return ensureFerrisConfigFileExists_sub( parentPath,
                                                     dbName_maybeWithPrefixSlashes,
                                                     rdn );
        }
        catch( dbOldVersionException& e )
        {
//             cerr << "ensureFerrisConfigFileExists() pp:" << parentPath
//                  << " rdn:" << rdn
//                  << " e:" << e.what()
//                  << endl;
            string temprdn = rdn + "-oldformat.db";
            
//             cerr << "Old berkeley database file found at:"
//                  << " path:" << parentPath
//                  << " filename:" << rdn << endl
//                  << "Please db_upgrade this file. For now it will be renamed to:"
//                  << temprdn << endl;
            rename( rdn.c_str(), temprdn.c_str() );

            return ensureFerrisConfigFileExists_sub( parentPath,
                                                     dbName_maybeWithPrefixSlashes,
                                                     rdn );
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    const char* COPY_INPUT_IN_MMAP_MODE_BY_DEFAULT = "copy-input-in-mmap-mode-by-default";
    const char* COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN = "copy-input-in-mmap-mode-for-objects-larger-than";
    const char* COPY_OUTPUT_IN_MMAP_MODE_BY_DEFAULT = "copy-output-in-mmap-mode-by-default";
    const char* COPY_OUTPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN = "copy-output-in-mmap-mode-for-objects-larger-than";
    const char* USE_SENDFILE_IF_POSSIBLE = "use-sendfile-if-possible";
    const char* SENDFILE_CHUNK_SIZE = "sendfile-chunk-size";


    const char* CFG_ATTRIBUTES_TO_AUTO_REA_K = "cfg-attributes-to-auto-rea-k";
    const char* CFG_ATTRIBUTES_TO_AUTO_REA_DEFAULT = "";

    const char* CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_K = "cfg-attributes-always-in-ui-model-k";
    const char* CFG_ATTRIBUTES_ALWAYS_IN_UI_MODEL_DEFAULT = "width,height";
    
    const char* COPY_USES_DIRECTIO_BY_DEFAULT = "copy-uses-directio-by-default";
    const char* COPY_USES_DIRECTIO_FOR_OBJECTS_LARGER_THAN = "copy-uses-directio-for-objects-larger-than";

    const char* SHOW_LOGGING_POPT_OPTIONS_BY_DEFAULT = "show-logging-popt-options-by-default";
    const char* LOGGING_TO_FILENAME_KEY = "log-to-filename";

    const char* CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_K = "cfg-attributes-gnu-diff-files";
    const char* CFG_ATTRIBUTES_GNU_DIFF_CMD_FILES_DEFAULT = "diff -NuarbB ";

    const char* CFG_FORCE_PASSIVE_VIEW_K = "cfg-force-passive-view";
    const char* CFG_FORCE_PASSIVE_VIEW_DEFAULT = "";


    const char* CFG_XSLTFS_STYLESHEET_PATH_K = "cfg-xsltfs-stylesheet-path";
    std::string CFG_XSLTFS_STYLESHEET_PATH_DEFAULT = getDotFerrisPath() + "/xsltfs-stylesheets/";

    const char* CFG_GLOB_SKIP_REGEX_LIST_K = "cfg-glob-skip-regex-list";
    const char* CFG_GLOB_SKIP_REGEX_LIST_DEFAULT = "";

    const char* CFG_GLOB_SKIP_FILE_URLS_K = "cfg-glob-skip-file-urls";
    const char* CFG_GLOB_SKIP_FILE_URLS_DEFAULT = "0";

    const char* CFG_PRECALCULATE_FOR_COPY_K = "cfg-precalculate-for-copy";
    const char* CFG_PRECALCULATE_FOR_COPY_DEFAULT = "1";
    
    const char* CFG_RDFCACHE_ATTRS_LIST_K = "cfg-rdfcache-attrs-list-k";
    const char* CFG_RDFCACHE_ATTRS_LIST_DEFAULT = ""
        "width,height,depth-per-color,depth,gamma,has-alpha,aspect-ratio,"
        "rpm-package,rpm-version,rpm-release,"
        "has-valid-signature,md5,mimetype-from-content,artist,title,album,genre,discid,track,date,cddb,"
        "subtitles,"
        ;
    
    const char* CFG_RDFCACHE_ATTRS_ENABLED_K = "cfg-rdfcache-attrs-enabled-k";
    const char* CFG_RDFCACHE_ATTRS_ENABLED_DEFAULT = "1";

    const char* CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K = "cfg-rdf-global-smush-group-leader-k";
    const char* CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_DEFAULT = "";
    
    const char* CFG_LOWEST_EMBLEM_PRI_TO_SHOW_K        = "cfg-lowest-emblem-pri-to-show-in-ui";
    const char* CFG_LOWEST_EMBLEM_PRI_TO_SHOW_DEFAULT  = "-1";
    
    const char* CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_K = "cfg-thumbnails-max-desired-width-or-height";
    const char* CFG_THUMBNAILS_MAX_DESIRED_WIDTH_OR_HEIGHT_DEFAULT = "128";
    const char* CFG_THUMBNAILS_JPEG_IMG_QUALITY_K = "cfg-thumbnails-jpeg-img-quality";
    const char* CFG_THUMBNAILS_JPEG_IMG_QUALITY_DEFAULT = "80";

    const std::string CFG_CDROM_SIZE_BYTES_K = "cfg-cdrom-size-bytes";
    const std::string CFG_CDROM_SIZE_BYTES_DEFAULT = "734003200";  // 700 Mb
    const std::string CFG_CDROM_SIZE_BYTES_EA_POSTFIX = "-cdrom-count";
    const std::string CFG_DVD_SIZE_BYTES_K = "cfg-dvd-size-bytes";
    const std::string CFG_DVD_SIZE_BYTES_DEFAULT = "4590208000";   // 4590208 KB (kb==1000)
    const std::string CFG_DVD_SIZE_BYTES_EA_POSTFIX = "-dvd-count";

    const char* FERRIS_LUCENE_METADATA_DBNAME = "ferris-metadata.db";

    const char* CFG_ALLOWS_KERNEL_EA_REGEX = "allows-kernel-ea-regex";
    const char* CFG_ALLOWS_KERNEL_EA_REGEX_DEFAULT = "^file://.*";
    

    const char* CFG_WEBPHOTOS_TOKEN_K = "webphotos-token";
    const char* CFG_WEBPHOTOS_TOKEN_DEFAULT = "";

    const char* CFG_WEBPHOTOS_DEFAULT_USERNAME_KEY = "webphotos-default-username";
    const char* CFG_WEBPHOTOS_DEFAULT_USERNAME_DEFAULT = "";

    const char* CFG_WEBPHOTOS_USERNAME_K = "webphotos-username";
    const char* CFG_WEBPHOTOS_USERNAME_DEF = "";
    const char* CFG_WEBPHOTOS_FULLNAME_K = "webphotos-fullname";
    const char* CFG_WEBPHOTOS_FULLNAME_DEF = "";

    const char* CFG_WEBPHOTOS_LARGEST_DIM_K = "webphotos-largest-dimension";
    const char* CFG_WEBPHOTOS_LARGEST_DIM_DEF = "0";

    const char* CFG_WEBPHOTOS_INCLUDE_EA_IS_PRESENT_REGEX_K = "webphotos-include-ea-is-present-regex";
    const char* CFG_WEBPHOTOS_INCLUDE_EA_AND_VALUE_REGEX_K = "webphotos-include-ea-and-value-regex";

    const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_PUBLIC_K = "webphotos-default-protection-public";
    const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_FRIEND_K = "webphotos-default-protection-friend";
    const char* CFG_WEBPHOTOS_DEFAULT_PROTECTION_FAMILY_K = "webphotos-default-protection-family";


    const char* CFG_FSPOT_POSITIVE_OVERLAY_REGEX_K = "fspot-positive-overlay-regex";
    const char* CFG_FSPOT_POSITIVE_OVERLAY_REGEX_DEF = "";

    const char* CFG_STRIGI_POSITIVE_OVERLAY_REGEX_K = "strigi-positive-overlay-regex";
    const char* CFG_STRIGI_POSITIVE_OVERLAY_REGEX_DEF = "^file:.*";
    
};
