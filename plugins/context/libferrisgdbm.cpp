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

    $Id: libferrisgdbm.cpp,v 1.5 2010/09/24 21:31:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * WARNING, due to a poor typedef in the standard gdbm.h file, I include a patched
 * header with the library. Hopefully the API will not change enough to break this
 *
 *
 */

#include <libcommondbapi.hh>
extern "C" {
#include <gdbm.h>
};

#include <errno.h>
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

    string tostr( datum d )
        {
            fh_stringstream ss;
            ss.write( d.dptr, d.dsize );
            return tostr(ss);
        }

    void free( datum d )
        {
            if( d.dptr )
                ::free( (void*)d.dptr );
        }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_CTXPLUGIN gdbmContext
        :
        public CommonDBContext< gdbmContext >
{
    typedef gdbmContext               _Self;
    typedef CommonDBContext< _Self >  _Base;

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    mtimeNotChangedChecker m_mtimeNotChangedChecker;
    GDBM_FILE db;
    GDBM_FILE getDB( int gdbm_flags = GDBM_WRITER ) //GDBM_READER )
        {
            if( !db )
            {
                gdbmContext* c = getBaseContext();
                if( c == this )
                {
                    LG_GDBM_D << "opening file:" << dbFileName() << endl;
                    db = gdbm_open ( (char*)dbFileName().c_str(), 0,
                                     GDBM_NOLOCK | gdbm_flags,
                                     O_RDWR | 644, 0 );
                    if( !db )
                    {
                        cerr << "Can not open gdbm"
                             << " for URL:" << getURL()
                             << " reason:" << gdbm_strerror( gdbm_errno ) << endl;
                        
                        fh_stringstream ss;
                        ss << "Can not open gdbm"
                           << " for URL:" << getURL()
                           << " reason:" << gdbm_strerror( gdbm_errno );
                        Throw_GdbmException( tostr(ss), this );
                    }
                }
                else
                {
                    return c->getDB();
                }
            }
            return db;
        }

    void changeDBFlags( int flags )
        {
//             gdbmContext* c = getBaseContext();
//             if( c->db )
//             {
//                 gdbm_close( c->db );
//                 c->db = 0;
//             }
//             getDB( flags );
        }
    
    datum tod( const std::string& s )
        {
            datum ret;
            ret.dsize = s.length();
            ret.dptr = (char*)malloc( ret.dsize+1 );
            memcpy( ret.dptr, s.c_str(), ret.dsize );
            return ret;
        }

    virtual std::string getValue( const std::string& k ) 
        {
            try
            {
                datum dk = tod( k );
                datum v = gdbm_fetch( getDB(), dk );
                string ret = tostr( v );
                free( v );
                free( dk );

                return ret;
            }
            catch( GdbmException& e )
            {
                fh_stringstream ss;
                ss << "Can not get value k:" << k
                   << " for URL:" << getURL()
                   << " e:" << e.what();
                Throw_GdbmException( tostr(ss), this );
            }
        }

    virtual void setValue( const std::string& k, const std::string& v ) 
        {
            int rc = 0;
            
            try
            {
                changeDBFlags( GDBM_WRITER );
                datum dk = tod( k );
                datum dv = tod( v );
                rc = gdbm_store ( getDB(), dk, dv, GDBM_REPLACE );
                int ecache = gdbm_errno;
                free( dk );
                free( dv );
                changeDBFlags( GDBM_READER );

                LG_GDBM_D << "setValue() k:" << k << " v:" << v << " rc:" << rc << endl;
                if( rc != 0 )
                    LG_GDBM_D << "FAIL reason:" << gdbm_strerror( ecache );
            }
            catch( GdbmException& e )
            {
                fh_stringstream ss;
                ss << "Can not set value k:" << k << " to v:" << v
                   << " for URL:" << getURL()
                   << " e:" << e.what();
                Throw_GdbmException( tostr(ss), this );
            }

            if( rc != 0 )
            {
                fh_stringstream ss;
                ss << "Can not set value k:" << k << " to v:" << v
                   << " for URL:" << getURL()
                   << "reason:" << gdbm_strerror( gdbm_errno );
                Throw_GdbmException( tostr(ss), this );
            }
            
        }

    virtual stringlist_t getEAKeys();

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    gdbmContext* priv_CreateContext( Context* parent, string rdn )
        {
            gdbmContext* ret = new gdbmContext();
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
    
    virtual void priv_remove( fh_context c_ctx )
        {
            int ecache = 0;
            
            gdbmContext* c = dynamic_cast<gdbmContext*>( (GetImpl(c_ctx) ) );
            if( !c )
            {
                fh_stringstream ss;
                ss << "Attempt to remove a non edb context! url:" << c_ctx->getURL();
                Throw_CanNotDelete( tostr(ss), GetImpl(c_ctx) );
            }
            string url = c->getURL();
            LG_GDBM_D << "remove() url:" << url << " key:" << c->dbKey() << endl;

            changeDBFlags( GDBM_WRITER );
            GDBM_FILE db = getDB();
            int rc = 0;
            datum dk;
            
            dk  = tod( c->dbKey() );
            rc |= gdbm_delete( db, dk );
            ecache = gdbm_errno;
            free( dk );
            
            stringlist_t ealist = c->getEAKeys();
            for( stringlist_t::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
            {
                LG_GDBM_D << "Removing EA belonging to url:" << url << " k:" << *iter << endl;

                dk  = tod( *iter );
                rc |= gdbm_delete( db, dk );
                free( dk );
            }
            gdbm_sync( db );
            changeDBFlags( GDBM_READER );

            if( rc != 0 )
            {
                fh_stringstream ss;
                ss << "Can remove item for URL:" << getURL()
                   << " reason:" << gdbm_strerror( ecache );
                Throw_GdbmException( tostr(ss), this );
            }
        }
    
public:


    gdbmContext()
        :
        db( 0 ),
        _Base()
        {
            createStateLessAttributes();
        }
    
    virtual ~gdbmContext()
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
gdbmContext::getEAKeys() 
{
    stringlist_t ret;

    fh_stringstream eakeyss;
    eakeyss << "/" << dbKey() << "/";
    string eakey = tostr(eakeyss);

    GDBM_FILE db = getDB();

    datum dk = gdbm_firstkey( db );
    for( ; dk.dptr ; )
    {
        string k = tostr( dk );
        if( starts_with( k, eakey ) )
        {
            LG_GDBM_D << "Got key:" << k << endl;
            ret.push_back( k );
        }
        
        datum dx = dk;
        dk = gdbm_nextkey( db, dk );
        free( dx );
    }
    
    return ret;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



void
gdbmContext::priv_read()
{
    LG_GDBM_D << "gdbmContext::priv_read() url:" << getURL() << endl;

    if( getBaseContext() != this )
    {
        if( getBaseContext()->getHaveReadDir() )
        {
            LG_GDBM_D << "gdbmContext::priv_read() reading a fake child. url:"
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
        LG_GDBM_D << "db4Context::priv_read() no change since last read.... url:" << getURL() << endl;
        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }

    Factory::getPluginOutOfProcNotificationEngine().watchTree( this );
    EnsureStartStopReadingIsFiredRAII _raii1( this );
    AlreadyEmittedCacheRAII _raiiec( this );
    LG_GDBM_D << "gdbmContext::priv_read(1)  url:" << getURL() << endl;
    LG_GDBM_D << "gdbmContext::priv_read(1) name:" << getDirName() << endl;


    GDBM_FILE db = getDB();

    datum dk = gdbm_firstkey( db );
    for( ; dk.dptr ; )
    {
        string k = tostr( dk );
        if( k.length() && k[0] == '\0' )
        {
        }
        else
        {
            LG_GDBM_D << "Got key:" << k << endl;
            ensureEAorContextCreated( k );
        }
        
        datum dx = dk;
        dk = gdbm_nextkey( db, dk );
        free( dx );
    }

    LG_GDBM_D << "gdbmContext::priv_read(done) path:" << getDirPath() << endl;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static gdbmContext c;
        fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
        return ret;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
