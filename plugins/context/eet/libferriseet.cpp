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

    $Id: libferriseet.cpp,v 1.5 2010/09/24 21:31:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libcommondbapi.hh>
#include <Eet.h>

using namespace std;


namespace Ferris
{

    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    class eetContext;
    
    class FERRISEXP_DLLLOCAL EET_Image : public Image
    {
        eetContext* m_ctx;
        bool m_imageLoadHasAlreadyFailed;
        bool m_imageLoadAttempted;
        /**
         * If rgba == m_ourRGBA then we allocated it and must free() it,
         * otherwise the parent new[] allocated it and we don't touch it.
         */
        guint32* m_ourRGBA;
        
        virtual void priv_ensureDataLoaded( LoadType loadType = LOAD_ONLY_METADATA );
        virtual void freeRGBA();

    public:
        EET_Image( eetContext* m_ctx, fh_istream ss );
        ~EET_Image();
    };


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    class FERRISEXP_CTXPLUGIN eetContext
        :
        public CommonDBContext< eetContext >
    {
        typedef eetContext                    _Self;
        typedef CommonDBContext< eetContext > _Base;

        friend class EET_Image;
        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

        mtimeNotChangedChecker m_mtimeNotChangedChecker;
        Eet_File* m_eet;
        Eet_File* getDB()
            {
                if( !m_eet )
                {
                    eetContext* c = getBaseContext();
                    if( c == this )
                    {
                        LG_EET_D << "opening file:" << dbFileName() << endl;
                        m_eet = eet_open( (char*)dbFileName().c_str(), EET_FILE_MODE_READ );
                    }
                    else
                    {
                        m_eet = c->getDB();
                    }
                }
                return m_eet;
            }
             
        
        virtual std::string getValue( const std::string& k ) 
            {
                try
                {
                    int sz = 0;
                    void* d = eet_read( getDB(), (char*)k.c_str(), &sz );
                    if( !d )
                    {
                        fh_stringstream ss;
                        ss << "Can not get value k:" << k
                           << " for URL:" << getURL();
                        Throw_eetException( tostr(ss), this );
                    }

                    fh_stringstream ss;
                    ss.write( (const char*)d, sz );
                    free(d);
                    return tostr(ss);
                }
                catch( Db4Exception& e )
                {
                    fh_stringstream ss;
                    ss << "Can not get value k:" << k
                       << " for URL:" << getURL()
                       << " e:" << e.what();
                    Throw_eetException( tostr(ss), this );
                }
            }

        virtual void setValue( const std::string& k, const std::string& v ) 
            {
                try
                {
                    int compress = 1;
                    int rc = eet_write( getDB(),
                                        (char*)k.c_str(),
                                        (char*)v.data(),
                                        v.length(),
                                        compress );
                    if( !rc )
                    {
                        fh_stringstream ss;
                        ss << "Can not set value k:" << k << " to v:" << v
                           << " for URL:" << getURL()
                           << " WRITING FOR EET IS NOT IMPLEMENTED YET";
                        Throw_eetException( tostr(ss), this );
                    }
                }
                catch( Db4Exception& e )
                {
                    fh_stringstream ss;
                    ss << "Can not set value k:" << k << " to v:" << v
                       << " for URL:" << getURL()
                       << " e:" << e.what();
                    Throw_eetException( tostr(ss), this );
                }
            }

        virtual stringlist_t getEAKeys()
            {
                stringlist_t ret;

                fh_stringstream eakeyss;
                eakeyss << "/" << dbKey() << "/*";
                string eakey = tostr(eakeyss);
                Eet_File*  db = getDB();

                int keys_sz = 0;
                char** keys = eet_list( db, (char*)eakey.c_str(), &keys_sz );

                for( int i=0; i<keys_sz; ++i )
                {
                    string k = keys[i];
                    LG_EET_D << "Got key:" << k << endl;
                    ret.push_back( k );
                }
                free( keys );
                return ret;
            }
        
        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        eetContext* priv_CreateContext( Context* parent, string rdn )
            {
                eetContext* ret = new eetContext();
                ret->setContext( parent, rdn );
                return ret;
            }

    protected:
        
        virtual void priv_read();

//         virtual bool getHasSubContextsGuess()
//             {
//                 if( getBaseContext() != this )
//                     return hasSubContexts();
//                 return _Base::getHasSubContextsGuess();
//             }

        virtual bool
        supportsRename()
            {
                return false;
            }

        virtual bool supportsRemove()
            {
                return false;
            }
        
        virtual void priv_remove( fh_context c_ctx )
            {
                eetContext* c = dynamic_cast<eetContext*>( (GetImpl(c_ctx) ) );
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Attempt to remove a non edb context! url:" << c_ctx->getURL();
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                string url = c->getURL();
                LG_EET_D << "remove() url:" << url << " key:" << c->dbKey() << endl;

                Eet_File* db = c->getDB();
                eet_write( db, (char*)c->dbKey().c_str(), 0, 0, 0 );

                stringlist_t ealist = c->getEAKeys();
                for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
                {
                    LG_EET_D << "Removing EA belonging to url:" << url << " k:" << *iter << endl;
                    eet_write( db, (char*)(*iter).c_str(), 0, 0, 0 );
                }
            }

        virtual void imageEAGenerator_priv_createAttributes( bool checkForImageLoader = true )
            {
                _Base::imageEAGenerator_priv_createAttributes( false );
            }

        virtual fh_image priv_getImage()
            {
                static fh_stringstream ss;
                fh_image image = new EET_Image( this, ss );
                return image;
            }
        
    public:
        
        eetContext()
            :
            m_eet( 0 )
            {
                createStateLessAttributes();
                createAttributes();
            }

        virtual ~eetContext()
            {
            }

        virtual void priv_createAttributes()
            {
            }
        

        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
//            addStandardFileSubContextSchema(m);
                m["dir"] = SubContextCreator( _Base::SL_commondb_SubCreate_dir,
                                             "	<elementType name=\"dir\">\n"
                                             "		<elementType name=\"name\" default=\"new directory\">\n"
                                             "			<dataTypeRef name=\"string\"/>\n"
                                             "		</elementType>\n"
                                             "	</elementType>\n");
                m["file"] = SubContextCreator( _Base::SL_commondb_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");

                /* Note that EA created for the root of the db4 file will not show up
                 *  until after the db4 has been read()
                 */
                m["ea"] = SubContextCreator(
                    _Base::SL_commondb_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }

    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    eetContext::priv_read()
    {
        LG_EET_D << "eetContext::priv_read() url:" << getURL() << endl;

        if( m_mtimeNotChangedChecker( this ) )
        {
            LG_EET_D << "db4Context::priv_read() no change since last read.... url:" << getURL() << endl;
            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
        
        Factory::getPluginOutOfProcNotificationEngine().watchTree( this );
        emitExistsEventForEachItemRAII _raii1( this );
        AlreadyEmittedCacheRAII _raiiec( this );

        
        PrefixTrimmer pretrimmer;
        pretrimmer.push_back( "/" );
        PostfixTrimmer postrimmer;
        postrimmer.push_back( "/" );
        string dbkey = postrimmer( pretrimmer( dbKey() ));
        if( !dbkey.empty() )
            dbkey = dbkey + "/";
        
        int dbkeylen = dbkey.length();
        int keys_sz = 0;
        char** keys = 0;
        Eet_File*  db = getDB();

        LG_EET_D << "eetContext::priv_read(1)  url:" << getURL() << endl;
        LG_EET_D << "eetContext::priv_read(1) name:" << getDirName() << endl;
        LG_EET_D << "eetContext::priv_read(1) dbkey:" << dbkey << endl;
        LG_EET_D << "eetContext::priv_read(1) dbfile:" << dbFileName() << endl;
        
        {
            fh_stringstream eakeyss;
            eakeyss << dbkey << "*";
            string eakey = tostr(eakeyss);

            keys = eet_list( db, (char*)eakey.c_str(), &keys_sz );

            for( int i=0; i<keys_sz; ++i )
            {
                string k = keys[i];
                if( k.length() && k[0] == '\0' )
                    continue;
                LG_EET_D << "Got key:" << k << endl;

                string rhs = k.substr( dbkeylen );
                int slashPos = rhs.find( "/" );

                string dn = rhs.substr( 0, slashPos );
                LG_EET_D << "creating dir for:" << dn << endl;
                ensureEAorContextCreated( dn, false );
            }
            free( keys );
        }
        
        
        {
            fh_stringstream eakeyss;
            eakeyss << "/" << dbkey << "*";
            string eakey = tostr(eakeyss);

            keys = eet_list( db, (char*)eakey.c_str(), &keys_sz );

            for( int i=0; i<keys_sz; ++i )
            {
                string k = keys[i];
                if( k.length() && k[0] == '\0' )
                    continue;
                LG_EET_D << "Got key:" << k << " dbkeylen:" << dbkeylen << endl;
                if( string::npos == k.substr( dbkeylen+1 ).find( "/" ) )
                    ensureEAorContextCreated( k, false );
            }
            free( keys );
        }
        
        LG_EET_D << "eetContext::priv_read(done) path:" << getDirPath() << endl;
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    EET_Image::EET_Image( eetContext* m_ctx, fh_istream ss )
        :
        m_ourRGBA( 0 ),
        Image(ss),
        m_ctx( m_ctx ),
        m_imageLoadHasAlreadyFailed( false ),
        m_imageLoadAttempted( false )
    {
    }

    EET_Image::~EET_Image()
    {
        if( rgba && rgba == m_ourRGBA )
        {
            free( rgba );
            rgba = 0;
            m_ourRGBA = 0;
        }
    }

    void
    EET_Image::freeRGBA()
    {
        if( rgba && rgba == m_ourRGBA )
        {
            free( rgba );
            rgba = 0;
            m_ourRGBA = 0;
            m_imageLoadAttempted = false;
        }
    }
    
    void
    EET_Image::priv_ensureDataLoaded( LoadType loadType )
    {
        LG_EET_D << "EET_Image::priv_ensureDataLoaded() "
             << " this:" << toVoid( this )
             << " rgba:" << toVoid( rgba )
             << " url:" << m_ctx->getURL()
             << " m_imageLoadAttempted:" << m_imageLoadAttempted
             << " m_imageLoadHasAlreadyFailed:" << m_imageLoadHasAlreadyFailed
             << endl;
        
        if( !m_imageLoadAttempted )
        {
            if( rgba )
            {
                delete [] rgba;
                rgba = 0;

                LG_EET_D << "EET_Image::priv_ensureDataLoaded(2) "
                     << " this:" << toVoid( this )
                     << " rgba:" << toVoid( rgba )
                     << " url:" << m_ctx->getURL()
                     << " m_imageLoadAttempted:" << m_imageLoadAttempted
                     << " m_imageLoadHasAlreadyFailed:" << m_imageLoadHasAlreadyFailed
                     << endl;
            }
            
            if( m_imageLoadHasAlreadyFailed )
                return;

            GotMetaData = 1;
            unsigned int w=0;
            unsigned int h=0;
            int alpha=0;
            int compress=0;
            int quality=0;
            int lossy=0;

            void* d = eet_data_image_read( m_ctx->getDB(), (char*)m_ctx->dbKey().c_str(),
                                           &w, &h, &alpha, &compress, &quality, &lossy );
            if( !d )
            {
                LG_EET_I << "Failed to load eet image for:" << m_ctx->getURL() << endl;
                m_imageLoadHasAlreadyFailed = true;
                w = h = 0;
                alpha = 0;
            }
            
            setWidth ( w );
            setHeight( h );
            setAlpha ( alpha ? 8 : 0 );
            setDepthPerColor( 8 );
            setValid( d );

            if( loadType == LOAD_ONLY_METADATA )
            {
                free(d);
            }
            else
            {
                LG_EET_D << "SETTING RGBA TO d:" << toVoid(d) << endl;
                m_ourRGBA = (guint32*)d;
                rgba = (guint32*)d;
                m_imageLoadAttempted = true;
            }
        }
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static eetContext c;
        fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
        return ret;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
