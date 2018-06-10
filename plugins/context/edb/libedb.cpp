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

    $Id: libedb.cpp,v 1.4 2010/09/24 21:31:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>

#include <Edb.h>
#include <errno.h>
//extern int errno;

using namespace std;
namespace Ferris
{

extern "C"
{
    FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed );
};
    


    FERRISEXP_API std::string getEDBString( const std::string& edbname_relhome,
                                            const std::string& k,
                                            const std::string& def,
                                            bool isEdbName_RelativeToHome = true,
                                            bool throw_for_errors = false );
    
    FERRISEXP_API void setEDBString( const std::string& edbname_relhome,
                                     const std::string& k,
                                     const std::string& v,
                                     bool isEdbName_RelativeToHome = true,
                                     bool throw_for_errors = false );

    
    /**
     * Note that as this function is called from the logging code, it can not use any
     * LG_XXX_D logging because it might create a cyclic loop.
     *
     */
    string getEDBString( const string& edbname_relhome,
                         const string& k,
                         const string& def,
                         bool isEdbName_RelativeToHome,
                         bool throw_for_errors )
    {
        /* Dont cache non local files */
        if( !isEdbName_RelativeToHome )
        {
            return get_db4_string( edbname_relhome, k, def, throw_for_errors );
        }
        
        string filename = Shell::getHomeDirPath_nochecks()+edbname_relhome;
        fh_database db = getCachedDb4( edbname_relhome, filename, k, throw_for_errors, "get" );
        if( db )
        {
            return get_db4_string( db, k, def, throw_for_errors );
        }
        
        return def;
    }
    

    void setEDBString( const string& edbname_relhome,
                       const string& k,
                       const string& v,
                       bool isEdbName_RelativeToHome,
                       bool throw_for_errors )
    {
        string filename = edbname_relhome;
        if( isEdbName_RelativeToHome )
        {
            filename = Shell::getHomeDirPath_nochecks()+edbname_relhome;
        }

//        cerr << "setEDBString(A) k:" << k << " v:" << v << endl;

        /* Dont cache non local files */
        if( !isEdbName_RelativeToHome )
        {
            return set_db4_string( filename, k, v, throw_for_errors );
        }

        fh_database db = getCachedDb4( edbname_relhome, filename, k, throw_for_errors, "set" );
        if( db )
            set_db4_string( db, k, v, throw_for_errors );
    }

    
class FERRISEXP_CTXPLUGIN edbContext
    :
    public ParentPointingTreeContext< edbContext,
                                      RecommendedEACollectingContext< FakeInternalContext > >
{
    typedef edbContext                                _Self;
//    typedef RecommendedEACollectingContext<Context>   _Base;
//    typedef ParentPointingTreeContext< edbContext, RecommendedEACollectingContext<Context> > _Base;
    typedef ParentPointingTreeContext< edbContext, RecommendedEACollectingContext< FakeInternalContext > > _Base;

    friend fh_context SL_edb_SubCreate_dir( fh_context c, fh_context md );
    friend fh_context SL_edb_SubCreate_file( fh_context c, fh_context md );
    friend fh_context SL_edb_SubCreate_ea( fh_context c, fh_context md );
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    
    edbContext*
    priv_CreateContext( Context* parent, string rdn )
        {
            edbContext* ret = new edbContext();
            ret->setContext( parent, rdn );
            return ret;
        }
    
    E_DB_File* edb;
    mtimeNotChangedChecker m_mtimeNotChangedChecker;

protected:


    
    typedef f_stringstream ss_t;

    void writeStream( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );

    char* dbFileName();
    char* dbKey( Attribute* a = 0, const std::string& rdn = "" );
    
    E_DB_File* edbContext::openEdb();


    virtual ss_t real_getIOStream()
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               exception);

    
    virtual void priv_read();

    virtual bool getHasSubContextsGuess()
        {
            if( getBaseContext() != this )
                return hasSubContexts();
            return _Base::getHasSubContextsGuess();
        }


    virtual fh_context SubCreate_file( fh_context c, fh_context md )
        {
            string rdn      = getStrSubCtx( md, "name", "" );
            string v        = "<new>";

            string edbfn;
            string k;
            if( edbContext* edbc = dynamic_cast<edbContext*>(GetImpl(c)))
            {
                edbfn       = edbc->dbFileName();
                fh_stringstream kss;
                kss << edbc->dbKey( GetImpl(c) ) << "/" << rdn;
                string k        = tostr(kss);

                PrefixTrimmer pretrimmer;
                pretrimmer.push_back( "/" );
                k = pretrimmer( k );

                LG_EDB_D << "edb::subcreate_file... c:" << c->getURL()
                     << " db:" << edbc->dbFileName()
                     << " dbkey:" << edbc->dbKey( GetImpl(c) )
                     << " k:" << k
                     << " rdn:" << rdn
                     << endl;
                setEDBString( edbfn, k, v, false );

//                fh_context childc = edbc->Insert( edbc->CreateContext( GetImpl(c), k ));
                fh_context childc = edbc->ensureContextCreated( k );
                return childc;
            }

            fh_stringstream ss;
            ss << "Attempt to create an edb EA attribute on a context that is not an edb module."
               << " url:" << c->getURL()
               << endl;
            Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
        }
    virtual void createStateLessAttributes();

    typedef std::list< std::string > stringlist;
    stringlist getEAKeys( edbContext* c )
        {
            stringlist ret;
            
            fh_stringstream eakeyss;
            eakeyss << "/" << c->dbKey() << "/";
            string eakey = tostr(eakeyss);

            int keys_num=0;
            char **keys=0;
            if(keys = e_db_dump_key_list( c->dbFileName(), &keys_num))
            {
                for (int i = 0; i < keys_num; i++)
                {
                    string k = keys[i];

                    if( starts_with( k, eakey ) )
                    {
                        ret.push_back( k );
                    }
                    free(keys[i]);
                }
                free(keys);
            }

            return ret;
        }

    /**
     * Given a path anywhere in the edb, create a file for that path. note that the
     * path can have relative parts in it. The path must already have a dir to exist in!
     */
    fh_context create_edb_file( edbContext* c, string dirty_path )
        {
            fh_context dbc     = Resolve( dbFileName() );
            string dbfn        = dbc->getDirPath();
            fh_context parent  = Resolve( dirty_path, RESOLVE_PARENT );
            string     parentp = parent->getDirPath();

            fh_stringstream ss;
            ss << parentp << "/" << dirty_path.substr( parentp.length()+1 );
            string path = tostr(ss);
                    
            if( starts_with( path, dbfn ) )
            {
                path = path.substr( dbfn.length() + 1 );
            }
            LG_EDB_D << " ... path:" << path << " dbfn:" << dbfn << endl;
                
            fh_context newc = Shell::CreateFile( getBaseContext(), path );
            return newc;
        }
    

    
    
    virtual bool
    supportsRename()
        {
            return true;
        }

    virtual fh_context
    priv_rename( const std::string& rdn,
                 const std::string& newPathRelative,
                 bool TryToCopyOverFileSystems,
                 bool OverWriteDstIfExists )
        {
            string oldPath = appendToPath( getDirPath(), rdn );
            string newPath = appendToPath( getDirPath(), newPathRelative, true );

            try
            {
                LG_EDB_D << "priv_rename() url:" << getURL()
                         << " rdn:" << rdn
                         << " oldPath:" << oldPath
                         << " newPath:" << newPath
                         << " TryToCopyOverFileSystems:" << TryToCopyOverFileSystems
                         << " OverWriteDstIfExists:" << OverWriteDstIfExists
                         << " dbFileName:" << dbFileName()
                         << endl;

                fh_context dbc = Resolve( dbFileName() );
                string dbfn = dbc->getDirPath();

                fh_context newc = create_edb_file( getBaseContext(), newPath );
                {
                    fh_iostream ioss  = newc->getIOStream();
                    fh_context rdn_ctx = getSubContext( rdn );
                    fh_attribute attr  = rdn_ctx->getAttribute( "." );
                
                    ioss = attr->copyTo( ioss );
                    if( !ioss.good() )
                    {
                        fh_stringstream ss;
                        ss << "Rename attempt failed. URL:" << getURL()
                           << " error writing to new destination. "
                           << " src:" << rdn
                           << " dst:" << newPath;
                        LG_EDB_D << tostr(ss) << endl;
                        Throw_RenameFailed( tostr(ss), this );
                    }
                }

                /* Rename all the EA for the old context too */
                edbContext* c = 0;
                {
                    fh_context c_ctx = getSubContext( rdn );
                    c = dynamic_cast<edbContext*>( (GetImpl(c_ctx) ) );
                }
                if( !c )
                {
                    fh_stringstream ss;
                    ss << "Rename attempt failed. URL:" << getURL()
                       << " src:" << rdn << " dst:" << newPath
                       << " attempt to rename a non edb subcontext."
                       << endl;
                    Throw_RenameFailed( tostr(ss), this );
                }

                
                stringlist ealist = getEAKeys( c );
//                 LG_EDB_D << " c:" << c->getDirPath()
//                      << " listsize:" << ealist.size()
//                      << endl;
                for( stringlist::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
                {
                    int size=0;
                    string oldeaname = *iter;
                    string eaname    = oldeaname.substr( oldeaname.rfind( "/" )+1 );

                    fh_stringstream neweanamess;
                    neweanamess << newc->getDirPath() << "/" << eaname;
                    string neweaname = tostr(neweanamess);
                    neweaname = neweaname.substr( dbfn.length() );
                    

                    LG_EDB_D << "rename EA. oldeaname:" << oldeaname
                             << " neweaname:" << neweaname
                             << " eaname:" << eaname
                             << endl;

                    string data = getEDBString( dbfn, oldeaname, "", false );
                    setEDBString( dbfn, neweaname, data, false );
                }

                LG_EDB_D << "now just remove old, return new" << endl;
                
                /* Remove old context and return the new */
                remove( rdn );
                return Resolve( newPath );
            }
            catch( exception& e )
            {
                fh_stringstream ss;
                ss << "Rename attempt failed. URL:" << getURL()
                   << " src:" << rdn << " dst:" << newPath
                   << " e:" << e.what()
                   << endl;
                Throw_RenameFailed( tostr(ss), this );
            }
        }
    
    virtual bool supportsRemove()
        {
            return true;
        }
    
    virtual void priv_remove( fh_context c_ctx )
        {
            edbContext* c = dynamic_cast<edbContext*>( (GetImpl(c_ctx) ) );
            if( !c )
            {
                fh_stringstream ss;
                ss << "Attempt to remove a non edb context! url:" << c_ctx->getURL();
                Throw_CanNotDelete( tostr(ss), c );
            }

            LG_EDB_D << "edb_remove() path:" << c->getDirPath()
                 << " key:" << c->dbKey()
                 << " file:" << c->dbFileName()
                 << endl;
            
            E_DB_File* edb = c->openEdb();

            if( !edb )
            {
                fh_stringstream ss;
                ss << "Can not open edb file:" << dbFileName();
                Throw_CanNotDelete( tostr(ss), c );
            }
    
            e_db_data_del( edb, c->dbKey() );

            stringlist ealist = getEAKeys( c );
            for( stringlist::iterator iter = ealist.begin(); iter != ealist.end(); ++iter )
            {
                e_db_data_del( edb, (char*)iter->c_str() );
            }
            
            e_db_close(edb);
        }
    
public:

    edbContext();
    ~edbContext();

    virtual fh_istream getIStream( ferris_ios::openmode m = ios::in )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception);
    
    virtual fh_iostream getIOStream( ferris_ios::openmode m = ios::in|ios::out )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception);
    
    
    fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    void
    priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
//            addStandardFileSubContextSchema(m);
            m["dir"] = SubContextCreator(SL_edb_SubCreate_dir,
                                         "	<elementType name=\"dir\">\n"
                                         "		<elementType name=\"name\" default=\"new directory\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");
            m["file"] = SubContextCreator(SL_edb_SubCreate_file,
                                         "	<elementType name=\"file\">\n"
                                         "		<elementType name=\"name\" default=\"new file\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");

            /* Can not make ea for the edb file iteself by ourself */
            if( getBaseContext() == this )
            {
                addEAGeneratorCreateSubContextSchema( m );
            }
            else
            {
                m["ea"] = SubContextCreator(
                    SL_edb_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }
        }

    
};

string ensureStartsWithSlash( const string& s )
{
    if( s.length() && s[0] == '/' )
    {
        return s;
    }

    fh_stringstream ss;
    ss << "/" << s;
    return tostr(ss);
}



fh_context SL_edb_SubCreate_dir( fh_context c, fh_context md )
{
    return c->SubCreate_file( c, md );
}

fh_context SL_edb_SubCreate_file( fh_context c, fh_context md )
{
    return c->SubCreate_file( c, md );
}

fh_context SL_edb_SubCreate_ea( fh_context c, fh_context md )
{
    string edbfn;

    LG_EDB_D << "SL_edb_SubCreate_ea() c:" << c->getURL() << endl;

    if( edbContext* edbc = dynamic_cast<edbContext*>(GetImpl(c)))
    {
        edbfn       = edbc->dbFileName();

        string rdn      = getStrSubCtx( md, "name", "" );
        string v        = getStrSubCtx( md, "value", "" );
        string dbKey    = edbc->dbKey( GetImpl(c) );

        fh_stringstream kss;
        if( dbKey.length() ) kss << ensureStartsWithSlash( dbKey ) << "/" << rdn;
        else                 kss << "/" << rdn;
        string k        = tostr(kss);

        LG_EDB_D << "SL_edb_SubCreate_ea() c:" << c->getURL()
                 << " edbfn:" << edbfn
                 << " k:" << k
                 << " v:" << v
                 << " rdn:" << rdn
                 << endl;
        
        setEDBString( edbfn, k, v, false );

        edbc->ensureEACreated( rdn, false );
        
        
//         typedef list< string > StaticStringList_t;
//         static StaticStringList_t StaticStringList;

//         StaticStringList.push_back( rdn );
                            
//         typedef CStringAttribute::theFunctor_t Funct1;
//         typedef WritableCStringAttribute::theFromStreamFunctor_t Funct2;
//         typedef edbContext _Self;

//         edbc->tryAddWritableExternalAttribute(
//             StaticStringList.back().c_str(),
//             Funct1(edbc, &_Self::getEAStream),
//             Funct2(edbc, &_Self::setEAStream),
//             true );
    }
    
    return c;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


edbContext::edbContext()
    :
    edb(0),
    _Base( 0, "" )
//     ParentPointingTreeContext< edbContext, RecommendedEACollectingContext< FakeInternalContext > >( 0, "" )
{
//    LG_EDB_D << "edbContext::edbContext() this:" << (void*) this << endl;

    createStateLessAttributes();
}

edbContext::~edbContext()
{
    if(edb)
    {
        e_db_flush();
        e_db_close(edb);
    }
}





void
edbContext::createStateLessAttributes()
{
    static Util::SingleShot virgin;
    if( virgin() )
    {
//         LG_EDB_D << "edbContext::createStateLessAttributes() path:" << getDirPath()
//              << " deepest:" << getDeepestTypeInfo().name()
//              << endl;
        
        _Base::createStateLessAttributes( true );
        supplementStateLessAttributes( true );
    }
}


char*
edbContext::dbFileName()
{
    edbContext* c = getBaseContext();
    
    static string edb_filename;
    edb_filename = c->getDirPath();

    LG_EDB_D << "dbFileName() path:" << getDirPath()
             << " filename:" << edb_filename << endl;
    
    return (char*)edb_filename.c_str();
}

char*
edbContext::dbKey( Attribute* a, const std::string& rdn )
{
    if( !a )
        a = this;
    
    string path     = a->getDirPath();
    if( rdn.length() )
    {
        PostfixTrimmer trimmer;
        trimmer.push_back( "/" );
        path = trimmer( path );
        path += "/";
        path += rdn;
    }
    string edbfn    = dbFileName();
    string k        = path.substr( edbfn.length() );

    PrefixTrimmer pretrimmer;
    pretrimmer.push_back( "/" );
    k = pretrimmer( k );

    LG_EDB_D << "dbKey() path:" << path
             << " edbfn:" << edbfn
             << " k:" << k << endl;
    
    static string cachestr;
    cachestr = k;
    return (char*) cachestr.c_str();
}

E_DB_File*
edbContext::openEdb()
{
    return e_db_open( dbFileName() );
}


edbContext::ss_t
edbContext::real_getIOStream()
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           exception)
{
    ss_t ss;

    string s = getEDBString( dbFileName(), dbKey(), "", false );
    LG_EDB_D << "real_getIOStream() s:" << s << endl;
    ss << s;
    return ss;
    
    
//     E_DB_File* edb = openEdb();

//     if( !edb )
//     {
//         Throw_CanNotGetStream("Can not open edb file",this);
//     }

//     int size_ret = 0;
//     fh_char data((char*)e_db_data_get( edb, dbKey(), &size_ret ));
    
//     if( isBound(data) )
//     {
//         const string s( GetImpl(data), size_ret );
//         ss << s;
//         //ss = new ss_t( s );
//     }
//     else
//     {
//         fh_stringstream ss;
//         ss << "Can not read key:" << dbKey()
//            << " dirname:" << getDirName()
//            << " dbFileName:" << dbFileName()
//            << endl;
//         Throw_CanNotGetStream(tostr(ss),this);
//     }
    
//     e_db_close(edb);
//     return ss;
}



fh_istream
edbContext::getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    return real_getIOStream();
}



void
edbContext::writeStream( fh_istream& ss_param, std::streamsize tellp, ferris_ios::openmode m )
{
    LG_EDB_D << "edbContext::writeStream( ss_t* ss )" << endl;
    fh_istream& ss = ss_param;

    if( m & ios::trunc )
    {
        std::streampos be = 0;
        std::streampos en = tellp;
        ss = Factory::MakeLimitingIStream( ss_param, be, en );
    }
    
    ss->clear();
    ss->seekg(0);
    ss->clear();

    const string s = StreamToString(ss);
    LG_EDB_D << "edbContext::writeStream( ss_t* ss ) len:" << s.length()
             << " s:" << s << endl;

    E_DB_File* edb = e_db_open( dbFileName() );
    
    if( !edb )
    {
        fh_stringstream ss;
        ss << "Can not open edb file:" << dbFileName();
        Throw_CanNotGetStream(tostr(ss),this);
    }
    
//    e_db_data_set(edb, dbKey(), (void*)s.data(), s.length());
    e_db_close(edb);
    setEDBString( dbFileName(), dbKey(), s, false );
    LG_EDB_D << "read back:" << getEDBString( dbFileName(), dbKey(), "", false ) << endl;
}


fh_iostream
edbContext::getIOStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           CanNotGetStream,
           exception)
{
    ss_t ret = real_getIOStream();
//    AdjustForOpenMode( ret, m );

    if( m & ios::trunc )
    {
        ret->clear();
        ret->seekp(0);
        ret->clear();
    }
    if( m & ios::ate || m & ios::app )
    {
        ret->clear();
        ret->seekg(0, ios::end);
        ret->seekp(0, ios::end);
        ret->clear();
    }

    LG_EDB_D << "edbContext::getIOStream url:" << getURL()
         << " tellp:" << ret->tellp()
         << " tellg:"  << ret->tellg()
         << endl;
    
    ret.getCloseSig().connect( bind( slot(*this, &edbContext::writeStream ), m ));

    return ret;
}






void
edbContext::priv_read()
{
    LG_EDB_D << "edbContext::priv_read() path:" << getDirPath() << endl;

    if( getBaseContext() != this )
    {
        if( getBaseContext()->getHaveReadDir() )
        {
            LG_EDB_D << "edbContext::priv_read() reading a fake child. path:"
                     << getDirPath()
                     << endl;
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
        LG_EDB_D << "priv_read() no change since last read.... url:" << getURL() << endl;
        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }
    
    EnsureStartStopReadingIsFiredRAII _raii1( this );
    AlreadyEmittedCacheRAII _raiiec( this );
    LG_EDB_D << "edbContext::priv_read(1) path:" << getDirPath() << endl;
    LG_EDB_D << "edbContext::priv_read(1) name:" << getDirName() << endl;

    edb = e_db_open_read( (char*)getDirPath().c_str());
    if(edb)
    {
        int i;
        int keys_num=0;
        char **keys=0;
        
        LG_EDB_D << "edbContext::priv_read(2) path:" << getDirPath() << endl;
        if(keys = e_db_dump_key_list((char*)getDirPath().c_str(), &keys_num))
        {
            for (i = 0; i < keys_num; i++)
            {
                string k = keys[i];

                LG_EDB_D << "Key:" << k << endl;

                ensureEAorContextCreated( k );
                free(keys[i]);
            }
            free(keys);
        }
        
    }
    else
    {
        // Allow a Zero byte edb file.
        LG_EDB_W << "Allowing a edb with no keys rdn :" << getDirPath() << endl;
    }
    

    LG_EDB_D << "edbContext::priv_read(done) path:" << getDirPath() << endl;
}


fh_iostream
edbContext::getEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    string edbfn    = dbFileName();
    string k        = ensureStartsWithSlash( dbKey( c, rdn ) );

    LG_EDB_D << "getEAStream() path:" << c->getDirPath()
             << " edbfn:" << edbfn
             << " k:" << k
             << endl;
    
    fh_stringstream ss;
    ss << getEDBString( edbfn, k, "", false );
    return ss;
}

void
edbContext::setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream zz )
{
    string edbfn    = dbFileName();
    string k        = ensureStartsWithSlash( dbKey( c, rdn ) );
    string v        = StreamToString( zz );

    LG_EDB_D << "setEAStream() url:" << c->getURL()
         << " edbfn:" << edbfn
         << " k:" << k
         << " v:" << v
         << endl;
    setEDBString( edbfn, k, v, false );
}








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static edbContext c;
        fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
        return ret;
    }
}

};
