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

    $Id: libferrisdb4.cpp,v 1.20 2010/09/24 21:31:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libcommondbapi.hh>
#include <STLdb4/stldb4.hh>
#include <SyncDelayer.hh>
#include <errno.h>
#include "config.h"

namespace Ferris
{
    using namespace std;
    using namespace ::STLdb4;
 
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    static void db4Sync( Database* db, SyncDelayer* )
    {
        LG_DB4_D << "db4Sync() db:" << toVoid(db) << endl;
        db->sync();
    }
    
    
    class FERRISEXP_CTXPLUGIN db4Context
        :
        public CommonDBContext< db4Context >
    {
        typedef db4Context               _Self;
        typedef CommonDBContext< _Self > _Base;

        
        fh_database  m_db;
        fh_database  getDB();
        mtimeNotChangedChecker m_mtimeNotChangedChecker;

        virtual std::string getValue( const std::string& k ) 
            {
                try
                {
//                    cerr << "getValue() k:" << k << endl;
                    return get_db4_string( getDB(), k );

//                     std::string ret = get_db4_string( getDB(), k );
//                     cerr << "db4::getValue() k:" << k << " ret:" << ret << endl;
//                     return ret;
                }
                catch( dbException& e )
                {
                    fh_stringstream ss;
                    ss << "Can not get value k:" << k
                       << " for URL:" << getURL()
                       << " e:" << e.what();
                    Throw_Db4Exception( tostr(ss), this, e.get_errno() );
                }
            }

        virtual void setValue( const std::string& k, const std::string& v ) 
            {
                try
                {
//                    set_db4_string( getDB(), k, v );
                    getDB()->set( k, v );
                    if( !SyncDelayer::exists() )
                    {
                        LG_DB4_D << "setValue, no delay for sync..." << endl;
                        getDB()->sync();
                    }
                    else
                    {
                        LG_DB4_D << "setValue, waiting to sync later....." << endl;
                        typedef Loki::Functor< void, LOKI_TYPELIST_2( Database*, SyncDelayer* ) > F;
                        F f( db4Sync );
                        SyncDelayer::ensure( GetImpl(getDB()), Loki::BindFirst( f, GetImpl(getDB()) ) );
                    }
                    
//                    cerr << "db4::setValue() k:" << k << " v:" << v << endl;
//                    BackTrace();
                }
                catch( dbException& e )
                {
                    fh_stringstream ss;
                    ss << "Can not set value k:" << k << " to v:" << v
                       << " for URL:" << getURL()
                       << " e:" << e.what();
                    Throw_Db4Exception( tostr(ss), this, e.get_errno() );
                }
            }

        virtual stringlist_t getEAKeys();

        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/
    
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        db4Context* priv_CreateContext( Context* parent, string rdn )
            {
                db4Context* ret = new db4Context();
                ret->setContext( parent, rdn );
                return ret;
            }

    protected:

        virtual bool priv_supportsShortCutLoading()
            { return true; }
    public:
        virtual void setupEAForShortCutLoadedContext()
            {
                bool   checkNulls = false;
                fh_database db = getDB();

                if( !db->isBTree() )
                {
                    static bool v = false;
                    if( v )
                        return;
                    Util::ValueRestorer< bool > __v(v,true);

                    db4Context* c = getBaseContext();
                    c->read();
                    return;
                }
                
                string k = eaKey() + eaKeySeparator();
                LG_DB4_D << "setupEAForShortCutLoadedContext() k:" << k << endl;
//                 cerr << "setupEAForShortCutLoadedContext(1) earl:" << getURL() << endl;
//                 BackTrace();
//                 cerr << "setupEAForShortCutLoadedContext(1) k:" << k << endl;
//                 cerr << "setupEAForShortCutLoadedContext(1) dbKey:" << dbKey() << endl;
//                 cerr << "setupEAForShortCutLoadedContext(1) eaKey:" << eaKey() << endl;
                
                std::pair<Database::iterator, Database::iterator> ipair = db->equal_range_partial( k );
                Database::iterator di = ipair.first;
                Database::iterator  e = ipair.second;
                
                for( ; di != e; ++di )
                {
                    LG_DB4_D << "looping..." << endl;
                    string k;
                    di.getKey( k );

                    if( k.length() && k[0] == '\0' )
                        continue;
                    LG_DB4_D << "Got key:" << k << endl;
//                    cerr << "Got key:" << k << endl;

                    if( checkNulls )
                    {
                        string prefix = k.substr( 0, k.find( '\0' ) );
                        if( priv_isSubContextBound( prefix ))
                            continue;
                    }

                    k = Util::replace_all( k, "//", "/" );
                    getBaseContext()->ensureEAorContextCreated( k );
                }
//                cerr << "setupEAForShortCutLoadedContext(2) k:" << k << endl;
            }
    protected:
        
        
//         virtual bool priv_supportsShortCutLoading()  { return true; }
//         virtual fh_context priv_getSubContext( const std::string& rdn )
//             throw( NoSuchSubContext );
        
        virtual void priv_read();

        virtual bool getHasSubContextsGuess()
            {
                if( getBaseContext() != this )
                    return hasSubContexts();
                return _Base::getHasSubContextsGuess();
            }

        virtual bool supportsRemove()
            {
                return true;
            }
    
        virtual void priv_remove( fh_context c_ctx )
            {
                db4Context* c = dynamic_cast<db4Context*>( (GetImpl(c_ctx) ) );
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Attempt to remove a non db4 context! url:" << c_ctx->getURL();
                    Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
                }
                string url = c->getURL();
                LG_DB4_D << "remove() url:" << url << " key:" << c->dbKey() << endl;

                fh_database db = c->getDB();
                db->erase( c->dbKey() );

                stringlist_t ealist = c->getEAKeys();
                for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
                {
                    LG_DB4_D << "Removing EA belonging to url:" << url << " k:" << *iter << endl;
                    db->erase( *iter );
                }
                db->sync();
            }
    
    public:


    db4Context()
        :
        _Base()
        {
            createStateLessAttributes(false);
            this->supplementStateLessAttributes( true );
        }
    
    virtual ~db4Context()
        {
//            cerr << "~db4Context() path:" << getDirPath() << endl;
            
        }
    
    virtual ferris_ios::openmode getSupportedOpenModes()
        {
            return ios::in | ios::out | ios::binary | ios::ate | ios::trunc;
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

stringlist_t
db4Context::getEAKeys() 
{
    stringlist_t ret;

    fh_stringstream eakeyss;
    eakeyss << "/" << dbKey() << "/";
    string eakey = tostr(eakeyss);

    fh_database db = getDB();
    LG_DB4_D << "getEAKeys url:" << getURL() << endl;
    for( Database::iterator di = db->begin(); di != db->end(); ++di )
    {
        string k;
        di.getKey( k );
        if( starts_with( k, eakey ) )
        {
            LG_DB4_D << "Got key:" << k << endl;
            ret.push_back( k );
        }
    }
    
    return ret;
}


fh_database
db4Context::getDB()
{
    if( !isBound( m_db ) )
    {
        db4Context* c = getBaseContext();
        if( c == this )
        {
            int rc = access( dbFileName().c_str(), W_OK );

            bool read_only = false;
            if( rc != 0 )
                read_only = true;
            
            m_db = new Database( dbFileName(), "", read_only );
        }
        else
        {
            m_db = c->getDB();
        }
    }
    return m_db;
}





/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

//     //
//     // Short cut loading each dir unless absolutely needed.
//     //
//     fh_context
//     db4Context::priv_getSubContext( const string& rdn )
//         throw( NoSuchSubContext )
//     {
//         try
//         {
//             LG_DB4_D << "NativeContext::priv_getSubContext() p:" << getDirPath()
//                         << " rdn:" << rdn
//                         << endl;

//             Items_t::iterator isSubContextBoundCache;
//             if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
//             {
//                 LG_DB4_D << "NativeContext::priv_getSubContext(bound already) p:" << getDirPath()
//                             << " rdn:" << rdn
//                             << endl;
// //                return _Base::priv_getSubContext( rdn );
//                 return *isSubContextBoundCache;
//             }

//             if( rdn.empty() )
//             {
//                 fh_stringstream ss;
//                 ss << "NoSuchSubContext no rdn given";
//                 Throw_NoSuchSubContext( tostr(ss), this );
//             }
//             else if( rdn[0] == '/' )
//             {
//                 fh_stringstream ss;
//                 ss << "NoSuchSubContext no files start with unescaped '/' as filename";
//                 Throw_NoSuchSubContext( tostr(ss), this );
//             }

//             string k = dbKey( this, rdn );

//             try
//             {
//                 getValue( k );
//                 ensureEAorContextCreated( k );
//                 if( priv_isSubContextBound( rdn, isSubContextBoundCache ) )
//                     return *isSubContextBoundCache;
//             }
//             catch( Db4Exception& e )
//             {
//                 fh_stringstream ss;
//                 ss << "NoSuchSubContext k:" << k;
//                 Throw_NoSuchSubContext( tostr(ss), this );
//             }
//         }
//         catch( NoSuchSubContext& e )
//         {
//             throw e;
//         }
//         catch( exception& e )
//         {
//             string s = e.what();
// //            cerr << "NativeContext::priv_getSubContext() e:" << e.what() << endl;
//             Throw_NoSuchSubContext( s, this );
//         }
//         catch(...)
//         {}
//         fh_stringstream ss;
//         ss << "NoSuchSubContext:" << rdn;
//         Throw_NoSuchSubContext( tostr(ss), this );
//     }
        
    

void
db4Context::priv_read()
{
    LG_DB4_D << "db4Context::priv_read() url:" << getURL() << endl;
//    cerr << "db4Context::priv_read(T) url:" << getURL() << endl;

    if( getBaseContext() != this )
    {
        if( getBaseContext()->getHaveReadDir() )
        {
            LG_DB4_D << "db4Context::priv_read() reading a fake child. url:" << getURL() << endl;
            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
        else
        {
            getBaseContext()->priv_read();
            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
    }

    if( m_mtimeNotChangedChecker( this ) )
    {
        LG_DB4_D << "priv_read() no change since last read.... url:" << getURL() << endl;
        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }

//     cerr << "db4Context::priv_read() url:" << getURL() << endl;
//     BackTrace();
    
    
    try
    {

#ifndef DEBUGGING_DB4_MODULE    
        Factory::getPluginOutOfProcNotificationEngine().watchTree( this );
#endif
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        AlreadyEmittedCacheRAII _raiiec( this );
    
        LG_DB4_D << "db4Context::priv_read(1)  url:" << getURL() << endl;
        LG_DB4_D << "db4Context::priv_read(1) name:" << getDirName() << endl;
//        BackTrace();
    
        // Handle reading db4 files that redland creates
        bool   checkNulls = false;
        {
            string rdn = getDirName();
            if( ends_with( rdn, "-po2s.db" )
                || ends_with( rdn, "-so2p.db" )
                || ends_with( rdn, "-sp2o.db" )
                || ends_with( rdn, "-contexts.db" ) )
            {
                checkNulls = true;
            }
        }
    
        fh_database db = getDB();
        bool   haveKey= true;
        for( Database::iterator di = db->begin(); di != db->end(); ++di )
        {
            string k;
            di.getKey( k );

            if( k.length() && k[0] == '\0' )
                continue;
            LG_DB4_D << "Got key:" << k << endl;
//         cerr << "url:" << getURL() << " got key:" << k << endl;
//         dumpOutItems();

            if( checkNulls )
            {
                string prefix = k.substr( 0, k.find( '\0' ) );
                if( priv_isSubContextBound( prefix ))
                    continue;
            }
        
            ensureEAorContextCreated( k );
        }

        LG_DB4_D << "db4Context::priv_read(done) path:" << getDirPath() << endl;
    }
    catch( exception& e )
    {
        stringstream ss;
        ss << "Failed to read:" << getURL() << "\n"
           << "Reason:" << e.what() << endl;
        Throw_FerrisNotReadableAsContext( ss.str(), this );
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static db4Context c;
        fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
        return ret;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
