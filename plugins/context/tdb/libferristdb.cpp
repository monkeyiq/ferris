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

    $Id: libferristdb.cpp,v 1.6 2010/09/24 21:31:48 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <libcommondbapi.hh>
#include <tdb.h>
#include <errno.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    string tostr( TDB_DATA d )
        {
            fh_stringstream ss;
            ss.write( (const char*)d.dptr, d.dsize );
            return tostr(ss);
        }

    void free( TDB_DATA d )
        {
            if( d.dptr )
                ::free( (void*)d.dptr );
        }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_CTXPLUGIN tdbContext
        :
        public CommonDBContext< tdbContext >
{
    typedef tdbContext               _Self;
    typedef CommonDBContext< _Self >  _Base;

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    mtimeNotChangedChecker m_mtimeNotChangedChecker;
    TDB_CONTEXT* db;
    TDB_CONTEXT* getDB()
        {
            if( !db )
            {
                tdbContext* c = getBaseContext();
                if( c == this )
                {
                    LG_TDB_D << "tdb open(1) dbFileName():" << dbFileName()
                             << " this:" << getURL()
                             << " this:" << toVoid(this)
                             << " getBaseContext():" << getBaseContext()
                             << endl;
//                    DEBUG_dumpcl( "tdb open(1)" );
                    
                    db = tdb_open ( (char*)dbFileName().c_str(), 0,
                                    0, //TDB_NOLOCK | TDB_CLEAR_IF_FIRST,
                                    O_RDWR | O_CREAT | O_LARGEFILE, S_IRUSR|S_IWUSR );

                    LG_TDB_D << "tdb open(2) dbFileName():" << dbFileName()
                             << " this:" << getURL()
                             << " this:" << toVoid(this)
                             << " db:" << toVoid(db)
                             << endl;
                    if( !db )
                    {
                        LG_TDB_D << "Can not open tdb file at:" << dbFileName()
                                 << " this:" << getURL()
                                 << endl;

                        fh_stringstream ss;
                        ss << "Can not open tdb file at:" << dbFileName();
                        Throw_tdbException( tostr(ss), this );
                    }
                }
                else
                {
                    LG_TDB_D << "tdb open() dbFileName():" << dbFileName()
                             << " this:" << getURL()
                             << " c:" << c->getURL()
                             << " calling c->getDB()"
                             << endl;
                    db = c->getDB();
                }
            }

            LG_TDB_D << "tdb open(end) dbFileName():" << dbFileName()
                     << " this:" << getURL()
                     << " this:" << toVoid(this)
                     << endl;
            
            return db;
        }
    

    TDB_DATA tod( const std::string& s )
        {
            TDB_DATA ret;
            ret.dsize = s.length();
            ret.dptr = (unsigned char*)malloc( ret.dsize+1 );
            memcpy( ret.dptr, s.c_str(), ret.dsize );
            return ret;
        }

    virtual std::string getValue( const std::string& k ) 
        {
            TDB_DATA dk = tod( k );
            TDB_DATA v = tdb_fetch( getDB(), dk );
            free( dk );
            
            if( !v.dptr )
            {
                fh_stringstream ss;
                ss << "Can not open tdb file at:" << dbFileName()
                   << " url:" << getURL()
                   << " key:" << k
                   << " reason:" << tdb_errorstr( getDB() );
                Throw_tdbException( tostr(ss), this );
            }
                
            string ret = tostr( v );
            free( v );

            return ret;
        }

    virtual void setValue( const std::string& k, const std::string& v ) 
        {
            TDB_DATA dk = tod( k );
            TDB_DATA dv = tod( v );
            int rc = tdb_store( getDB(), dk, dv, TDB_REPLACE );
            free( dk );
            free( dv );

            LG_TDB_D << "setValue() k:" << k << " v:" << v << " rc:" << rc << endl;
//             if( DEBUG && rc == 0 )
//             {
//                 // test it by read back
//                 TDB_DATA dk = tod( k );
//                 TDB_DATA dv = tdb_fetch( getDB(), dk );
//                 string ret = tostr( dv );
                
//                 LG_TDB_D << "setValue() k:" << k << " v:" << v << " rc:" << rc
//                          << " read back:" << ret 
//                          << endl;
                
//             }
            
            if( rc != 0 )
            {
                fh_stringstream ss;
                ss << "Can not set value k:" << k << " to v:" << v
                   << " for URL:" << getURL()
                   << " reason:" << tdb_errorstr( getDB() );
                Throw_tdbException( tostr(ss), this );
            }
        }

    virtual stringlist_t getEAKeys();

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    tdbContext* priv_CreateContext( Context* parent, string rdn )
        {
            tdbContext* ret = new tdbContext();
            ret->setContext( parent, rdn );
            return ret;
        }

protected:

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

    void checkZAndThrow( int n )
        {
            if( n != 0 )
            {
                fh_stringstream ss;
                ss << "Can remove item for URL:" << getURL()
                   << " reason:" << tdb_errorstr( getDB() );
                Throw_tdbException( tostr(ss), this );
            }
        }
    
    virtual void priv_remove( fh_context c_ctx )
        {
            tdbContext* c = dynamic_cast<tdbContext*>( (GetImpl(c_ctx) ) );
            if( !c )
            {
                fh_stringstream ss;
                ss << "Attempt to remove a non edb context! url:" << c_ctx->getURL();
                Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
            }
            string url = c->getURL();
            LG_TDB_D << "remove() url:" << url << " key:" << c->dbKey() << endl;

            TDB_CONTEXT* db = getDB();
            int rc = 0;
            TDB_DATA dk;
            
            dk  = tod( c->dbKey() );
            rc = tdb_delete( db, dk );
            free( dk );
            checkZAndThrow( rc );
            
            stringlist_t ealist = c->getEAKeys();
            for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
            {
                LG_TDB_D << "Removing EA belonging to url:" << url << " k:" << *iter << endl;

                dk  = tod( *iter );
                rc |= tdb_delete( db, dk );
                free( dk );
                checkZAndThrow( rc );
            }
        }
    
public:


    tdbContext()
        :
        db( 0 ),
        _Base()
        {
            createStateLessAttributes();
        }
    
    virtual ~tdbContext()
        {
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
tdbContext::getEAKeys() 
{
    stringlist_t ret;

    fh_stringstream eakeyss;
    eakeyss << "/" << dbKey() << "/";
    string eakey = tostr(eakeyss);

    TDB_CONTEXT* db = getDB();

    TDB_DATA dk = tdb_firstkey( db );
    for( ; dk.dptr ; )
    {
        string k = tostr( dk );
        if( starts_with( k, eakey ) )
        {
            LG_TDB_D << "Got key:" << k << endl;
            ret.push_back( k );
        }
        
        TDB_DATA dx = dk;
        dk = tdb_nextkey( db, dk );
        free( dx );
    }
    
    return ret;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



void
tdbContext::priv_read()
{
    LG_TDB_D << "tdbContext::priv_read() url:" << getURL() << endl;

    if( getBaseContext() != this )
    {
        if( getBaseContext()->getHaveReadDir() )
        {
            LG_TDB_D << "tdbContext::priv_read() reading a fake child. url:"
                     << getURL() << endl;
            
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
        LG_TDB_D << "priv_read() no change since last read.... url:" << getURL() << endl;
        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }
    
    Factory::getPluginOutOfProcNotificationEngine().watchTree( this );
        
    EnsureStartStopReadingIsFiredRAII _raii1( this );
    AlreadyEmittedCacheRAII _raiiec( this );
    TDB_CONTEXT* db = getDB();

    LG_TDB_D << "tdbContext::priv_read(1)  url:" << getURL() << endl;
    LG_TDB_D << "tdbContext::priv_read(1) name:" << getDirName() << endl;
    LG_TDB_D << "tdbContext::priv_read(1) db:"   << toVoid(db) << endl;

    TDB_DATA dk = tdb_firstkey( db );
    for( ; dk.dptr ; )
    {
        string k = tostr( dk );

        LG_TDB_D << "tdbContext::priv_read() k:" << k << endl;
        
        if( k.length() && k[0] == '\0' )
        {
        }
        else
        {
            LG_TDB_D << "Got key:" << k << endl;
            ensureEAorContextCreated( k, false );
        }
        
        TDB_DATA dx = dk;
        dk = tdb_nextkey( db, dk );
        free( dx );
    }

    LG_TDB_D << "tdbContext::priv_read(done) path:" << getDirPath() << endl;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static tdbContext c;
        fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
        return ret;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
