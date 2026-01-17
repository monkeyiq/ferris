/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrissqlitevtable.cpp,v 1.8 2010/09/24 21:31:24 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <string.h>
#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include <string>
#include <iostream>
using namespace std;

#include <Ferris/Ferris.hh>
using namespace Ferris;

//#undef LG_SQLITE_D
//#define LG_SQLITE_D cerr

//#undef LG_SQLITE_W
//#define LG_SQLITE_W cerr

#define DEBUG LG_SQLITE_D

typedef std::vector< sqlite3_value* > sqliteValues_t;

string eanameToSQLColumnName( const std::string& eaname )
{
    string ret = eaname;
    ret = Util::replace_all( ret, "-", "_" );
    ret = Util::replace_all( ret, ":", "_colon_" );
    return ret;
}


string SQLColumnNameToEAName( const std::string& eaname )
{
    string ret = Util::replace_all( eaname, "_", "-" );
    return ret;
}



class virtual_cursor;

class virtual_table
{
    friend class virtual_cursor;
    
    string     m_basepath;
    fh_context m_basec;
    sqlite3*   m_db;

    sqlite_int64 m_highestRowID;

    typedef std::map< int, string > m_AttrID_to_EAName_t;
    m_AttrID_to_EAName_t m_AttrID_to_EAName;

    typedef stringmap_t m_AttributeSQLTypes_t;
    m_AttributeSQLTypes_t m_AttributeSQLTypes;

    typedef stringmap_t m_AttributeSynonumTable_t;
    m_AttributeSynonumTable_t m_AttributeSynonumTable;

    bool m_recursive;
    bool m_addRowIDToBaseFilesystem;
    
public:

    virtual_table( sqlite3 *db )
        :
        m_db( db ),
        m_basepath( "/" ),
        m_recursive( false ),
        m_addRowIDToBaseFilesystem( false ),
        m_highestRowID( 0 )
        {
        }

    fh_context getBase()
        {
            return m_basec;
        }

    bool isRecursive()
        {
            return m_recursive;
        }

    bool shouldAddRowIDsToBaseFilesystem()
        {
            return m_addRowIDToBaseFilesystem;
        }

    sqlite_int64 getNextRowID()
        {
            if( !m_highestRowID )
            {
                LG_SQLITE_D << "AA no highest ID yet, base:" << m_basec->getURL() << endl;

                m_highestRowID = toType<sqlite_int64>(
                    getStrAttr( m_basec, "ferris-sqlite-next-rowid", "0" ) );
            }
            LG_SQLITE_D << "updating rowid to next value and saving m_highestRowID:" << m_highestRowID << endl;
            ++m_highestRowID;
            setStrAttr( m_basec, "ferris-sqlite-next-rowid", tostr(m_highestRowID), true );
            
            LG_SQLITE_D << "returning saved m_highestRowID:" << m_highestRowID << endl;
            return m_highestRowID;
        }
    
    void create( stringvec_t args, void *pAux )
        {
            string tname = args[2];
            string basepath = "/";
            if( args.size() >= 4 )
            {
                basepath = args[3];
                if( starts_with( basepath, "'" ) )
                    basepath = basepath.substr( 1 );
                if( ends_with( basepath, "'" ))
                    basepath = basepath.substr( 0, basepath.length()-1 );
            }
            m_basepath = basepath;
            m_basec    = Resolve( m_basepath );

            if( args.size() >= 5 )
            {
                string s = args[4];
                if( string::npos != s.find("recursive=true") )
                    m_recursive = true;
                m_addRowIDToBaseFilesystem = ( string::npos != s.find("add-rowid-to-base-filesystem=true") );
            }
            
            
            LG_SQLITE_D << "vt_create() tname:" << tname
                        << " basepath:" << basepath
                        << endl;

            int colidx = 0;
            string n;

            n = "url";
            m_AttrID_to_EAName[colidx] = n;
            m_AttributeSQLTypes[n] = "text";
            ++colidx;

            if( args.size() >= 6 )
            {
                for( int idx = 5; idx < args.size(); ++idx )
                {
                    LG_SQLITE_D << "args[" << idx << "] = " << args[idx] << endl;

                    string v = args[idx];
                    int eqpos = v.find(" ");
                    string eaname = v.substr( 0, eqpos );

                    string raweaname = eaname;
                    eaname = eanameToSQLColumnName( eaname );
                    m_AttributeSynonumTable[eaname] = raweaname;
                    
                    string sqltype = "text";
                    if( eqpos != string::npos )
                    {
                        sqltype = v.substr( eqpos+1 );
                    }
                    LG_SQLITE_D << "eaname:" << eaname << " sqltype:" << sqltype << endl;
                    if( m_AttributeSQLTypes.find(eaname) == m_AttributeSQLTypes.end() )
                    {
                        n = eaname;
                        m_AttrID_to_EAName[colidx] = n;
                        m_AttributeSQLTypes[n] = sqltype;
                        ++colidx;
                    }
                    
                }
            }
            

            stringstream ddl;
            
            ddl << "create table fs (" << endl;
            for( m_AttrID_to_EAName_t::iterator iter = m_AttrID_to_EAName.begin();
                 iter != m_AttrID_to_EAName.end(); ++iter )
            {
                m_AttributeSQLTypes_t::iterator x = m_AttributeSQLTypes.find( iter->second );
                ddl << " " << x->first << " " << x->second << " ," <<  endl;
            }
            ddl << " __nothing int hidden " << endl;
            ddl << ")";

            LG_SQLITE_D << "DDL:" << tostr(ddl) << endl;
            
            sqlite3_declare_vtab( m_db, tostr(ddl).c_str() );
        }

    void destroy()
        {
        }


    int has_constraint(sqlite3_index_info *p_info, int col, int opmask)
        {
            int i;
            for(i = 0; i < p_info->nConstraint; i++)
            {
                if(p_info->aConstraint[i].iColumn == col)
                {
                    if(opmask != 0)
                    {
                        if(p_info->aConstraint[i].op & opmask)
                        {
                            return i;
                        }
                    }
                    return i;
                }
            }
    
            return -1;
        }
    
    void best_index( sqlite3_index_info *p_info)
        {
            int i = 0;
            int ops = SQLITE_INDEX_CONSTRAINT_MATCH | SQLITE_INDEX_CONSTRAINT_EQ;

            if((i = has_constraint(p_info, colIndex("name"), ops)) > -1)
            {
                p_info->idxNum = colIndex("name");
                p_info->aConstraintUsage[i].argvIndex = 1;
            }

            if((i = has_constraint(p_info, colIndex("url"), ops)) > -1)
            {
                p_info->idxNum = colIndex("url");
                p_info->aConstraintUsage[i].argvIndex = 1;
            }

            if((i = has_constraint(p_info, colIndex("path"), ops)) > -1)
            {
                p_info->idxNum = colIndex("path");
                p_info->aConstraintUsage[i].argvIndex = 1;
            }
        }

    int colIndex( const string& n )
        {
            string eaname = eanameToSQLColumnName( n );

            for( m_AttrID_to_EAName_t::iterator iter = m_AttrID_to_EAName.begin();
                 iter != m_AttrID_to_EAName.end(); ++iter )
            {
                if( iter->second == eaname )
                    return iter->first;
            }
            LG_SQLITE_W << "colIndex() not found for n:" << n << endl;
            return 0;
        }
    string colName( int col )
        {
            DEBUG << "colname col:" << col << " eanames.sz:" << m_AttrID_to_EAName.size() << endl;
            m_AttrID_to_EAName_t::iterator iter = m_AttrID_to_EAName.find( col );
            if( iter != m_AttrID_to_EAName.end() )
            {
                string eaname = iter->second;
                return eaname;
            }
            return "";
        }

    fh_context findByRowID( sqlite_int64 rowid )
        {
            for( Context::iterator ci = m_basec->begin(); ci != m_basec->end(); ++ci )
            {
                if( rowid == toType<sqlite_int64>(getStrAttr( *ci, "ferris-sqlite-rowid", "0" )))
                {
                    return *ci;
                }
            }
            return 0;
        }
    int updateContext( fh_context c, sqlite3_value **argv )
        {
            stringset_t attributesToSkip;
            attributesToSkip.insert( eanameToSQLColumnName("name" ));
            attributesToSkip.insert( eanameToSQLColumnName("url" ));
            attributesToSkip.insert( eanameToSQLColumnName( "path" ));
            attributesToSkip.insert( eanameToSQLColumnName( "md5" ));
            attributesToSkip.insert( eanameToSQLColumnName( "crc32" ));
            attributesToSkip.insert( eanameToSQLColumnName( "sha1" ));
            attributesToSkip.insert( eanameToSQLColumnName( "size" ));
            attributesToSkip.insert( eanameToSQLColumnName( "is-dir" ));
            attributesToSkip.insert( eanameToSQLColumnName( "is-file" ));
            attributesToSkip.insert( eanameToSQLColumnName( "mtime-display" ));
            attributesToSkip.insert( eanameToSQLColumnName( "ctime" ));
                    
            for( m_AttrID_to_EAName_t::iterator iter = m_AttrID_to_EAName.begin();
                 iter != m_AttrID_to_EAName.end(); ++iter )
            {
                int idx = iter->first;
                string n = iter->second;

                if( attributesToSkip.count(n) )
                    continue;
                        
                string v = (const char*)sqlite3_value_text(argv[2 + idx]);
                LG_SQLITE_D << "idx:" << idx << " n:" << n << " v:" << v << endl;

                try
                {
                    string eaname = SQLColumnNameToEAName( n );
                    setStrAttr( c, eaname, v, true );
                }
                catch( exception& e )
                {
                    cerr << "Warning:" << e.what() << endl;
                }
            }
            return 0;
        }
    
    int update( int argc, sqlite3_value **argv, sqlite_int64 *pRowid)
        {
            int ret = SQLITE_OK;

            LG_SQLITE_D << "vt_update()" << " argc:" << argc << endl;

            if( argc >= 2 )
            {
                LG_SQLITE_D << "vt_update() argv[0] is null:" << ( sqlite3_value_type(argv[0]) == SQLITE_NULL ) << endl;
                LG_SQLITE_D << "vt_update() argv[1] is null:" << ( sqlite3_value_type(argv[1]) == SQLITE_NULL ) << endl;
            }
    
            if( argc == 1 )
            {
                LG_SQLITE_D << "delete row at argv[0]" << endl;
                sqlite_int64 rowid = sqlite3_value_int64(argv[0]);
                LG_SQLITE_D << "rowid:" << rowid << endl;
            }
            else
            {
                if( sqlite3_value_type(argv[0]) == SQLITE_NULL )
                {
                    LG_SQLITE_D << "insert a new row!" << endl;
                    int urlidx = colIndex( "url" );
                    string url = (const char*)sqlite3_value_text(argv[2 + urlidx]);
                    LG_SQLITE_D << "url:" << url << endl;

                    int mode = 0;
                    bool isDir = true;
                    LG_SQLITE_D << "is-dir-index:" << colIndex( "is-dir" )
                                << " mode-index:" << colIndex( "mode" )
                                << endl;
                    if( int tidx = colIndex( "is-dir" ) )
                        isDir = isTrue((const char*)sqlite3_value_text(argv[2 + tidx]));
                    if( int tidx = colIndex( "mode" ) )
                        mode = toint((const char*)sqlite3_value_text(argv[2 + tidx]));
                    fh_context newc = Shell::acquireContext( url, mode, isDir );

                    if( sqlite3_value_type(argv[1]) == SQLITE_NULL )
                    {
                        rowid( *pRowid, newc );
                    }

                    updateContext( newc, argv );
                }
                if( sqlite3_value_type(argv[0]) != SQLITE_NULL &&
                    sqlite3_value_type(argv[1]) != SQLITE_NULL )
                {
                    LG_SQLITE_D << "update a row!" << endl;
                    sqlite_int64 rowid1 = sqlite3_value_int64(argv[0]);
                    sqlite_int64 rowid2 = sqlite3_value_int64(argv[1]);
                    LG_SQLITE_D << "row1:" << rowid1 << " row2:" << rowid2 << endl;

                    if( rowid1 == rowid2 )
                    {
                        fh_context c = findByRowID( rowid1 );
                        LG_SQLITE_D << "rowid:" << rowid1 << endl;
                        LG_SQLITE_D << " context:" << c->getURL() << endl;
                        updateContext( c, argv );
                    }
                    else
                    {
                        LG_SQLITE_ER << "FIXME: changing rowids not implemented!" << endl;
                    }
                }
            }
    
            return ret;
        }

    void rowid( sqlite_int64& rowid, fh_context c )
        {
            LG_SQLITE_D << "rowid()" << endl;
            rowid = 1;

            if( shouldAddRowIDsToBaseFilesystem() )
            {
                LG_SQLITE_D << "rowid() shoud add proper row-ids" << endl;
                rowid = toType<sqlite_int64>(
                    getStrAttr( c, "ferris-sqlite-rowid", "0" ) );

                LG_SQLITE_D << "rowid() next id:" << rowid << endl;
                if( !rowid )
                {
                    rowid = getNextRowID();
                    setStrAttr( c, "ferris-sqlite-rowid", tostr(rowid), true );

                    LG_SQLITE_D << "rowid() created new rowid:" << rowid
                                << " for context:" << c->getURL() << endl;
                }
            }
        }
    
};

struct vtab
{
    sqlite3_vtab   base;
    virtual_table* pp;
};

class virtual_cursor 
{
protected:
    virtual_table* m_table;

    int m_count;
    int m_eof;
    Context::iterator m_iter;
    fh_contextlist m_explicitContextList;


    fh_context m_base;

    
    virtual void remakeExplicitContextList()
        {
            m_explicitContextList = Factory::MakeContextList();
            
            if( SelectionContext* sc = dynamic_cast<SelectionContext*>( GetImpl(m_explicitContextList ) ) )
            {
                sc->setSelectionContextRDNConflictResolver(
                    get_SelectionContextRDNConflictResolver_MonsterName() );
            }
        }
    
public:

    virtual_cursor( virtual_table* table )
        :
        m_table( table ),
        m_count(0),
        m_eof(0)
        {
            remakeExplicitContextList();
            m_base = 0;
        }

    fh_context getBase()
        {
            if( isBound( m_base ) )
            {
                return m_base;
            }
            return m_table->getBase();
        }
    
    Context::iterator end()
        {
            return getBase()->end();
        }
    void close()
        {
            m_iter = end();
            remakeExplicitContextList();
            m_base = 0;
        }
    virtual void next()
        {
            ++m_iter;
        }
    virtual int eof()
        {
            LG_SQLITE_D << "eof()..." << endl;
            
            if( m_iter == end() )
            {
                m_eof = 1;
            }
            else
            {
                LG_SQLITE_D << "eof() iter:" << m_iter->getURL() << endl;
            }
            
            return m_eof;
        }
    void column( sqlite3_context *ctx, int col )
        {
            string eaname = m_table->colName( col );
            LG_SQLITE_D << "col:" << col << " eaname:" << eaname << endl;

            if( !m_table->m_AttributeSynonumTable[eaname].empty() )
            {
                eaname = m_table->m_AttributeSynonumTable[eaname];
            }
            
            if( eaname.empty() )
            {
                LG_SQLITE_W << "Empty eaname for col:" << col << " This is BAD!" << endl;
                sqlite3_result_text(ctx, "", 0, SQLITE_STATIC);
            }
            else
            {
                if( eaname=="url" && m_iter->isAttributeBound( "ferris-delegate-url" ) )
                    eaname = "ferris-delegate-url";
                if( eaname=="path" && m_iter->isAttributeBound( "ferris-delegate-path" ) )
                    eaname = "ferris-delegate-path";

                
                DEBUG << "Getting eaname:" << eaname << " for url:" << m_iter->getURL() << endl;
                string v = getStrAttr( *m_iter, eaname, "" );
                DEBUG << "value:" << v << endl;
                sqlite3_result_text(ctx, v.c_str(), v.length(), SQLITE_TRANSIENT );
            }
            
        }
    
    void rowid( sqlite_int64& rowid )
        {
            m_table->rowid( rowid, *m_iter );
             
//             LG_SQLITE_D << "rowid()" << endl;
//             rowid = 1;

//             if( m_table->shouldAddRowIDsToBaseFilesystem() )
//             {
//                 LG_SQLITE_D << "rowid() shoud add proper row-ids" << endl;
//                 rowid = toType<sqlite_int64>(
//                     getStrAttr( *m_iter, "ferris-sqlite-rowid", "0" ) );

//                 LG_SQLITE_D << "rowid() next id:" << rowid << endl;
//                 if( !rowid )
//                 {
//                     rowid = m_table->getNextRowID();
//                     setStrAttr( *m_iter, "ferris-sqlite-rowid", tostr(rowid) );

//                     LG_SQLITE_D << "rowid() created new rowid:" << rowid
//                                 << " for context:" << m_iter->getURL() << endl;
//                 }
//             }
        }

    void filter( int idxNum, const char *idxStr, sqliteValues_t args )
        {
            m_count = 0;
            m_eof = 0;
            m_base = 0;

            fh_context base = getBase();
            if( args.empty() )
            {
                m_iter = base->begin();
            }
            else
            {
                LG_SQLITE_D << "filter() idxNum:" << idxNum << " args.sz:" << args.size() << endl;

                remakeExplicitContextList();
                for( int i=0; i<args.size(); ++i )
                {
                    string a = (const char*)sqlite3_value_text(args[i]);
                    LG_SQLITE_D << "a:" << a << endl;

                    {
                        stringstream ss;
                        if( idxNum == m_table->colIndex("url") || idxNum == m_table->colIndex("path") )
                        {
                            ss << a;
//                             LG_SQLITE_D << "resolving indexed url or path query:" << tostr(ss) << endl;
//                             fh_context c = Resolve( tostr(ss) );
//                             m_base = c;
//                             m_iter = m_base->begin();
//                             return;
                        }
                        else if( idxNum == m_table->colIndex("name") )
                        {
                            ss << base->getURL() << "/" << a;
                        }
                        else
                        {
                            LG_SQLITE_ER << "filter() unknown index requested!" << endl;
                            return;
                        }
                            
                        LG_SQLITE_D << "resolving indexed query:" << tostr(ss) << endl;
                        fh_context c = Resolve( tostr(ss) );
                        m_explicitContextList->insert(c);
                        LG_SQLITE_D << "adding explicit result... c:" << c->getURL() << endl;
                    }
                }
                m_iter = m_explicitContextList->begin();
            }
            
        }
};

class recursive_virtual_cursor
    :
    public virtual_cursor
{
    Context::iterator m_recursiveLastParent;

protected:
    
    virtual void remakeExplicitContextList()
        {
            virtual_cursor::remakeExplicitContextList();
            m_recursiveLastParent = end();
        }
    
public:
    recursive_virtual_cursor( virtual_table* table )
        :
        virtual_cursor( table )
        {
        }

    virtual void next()
        {
            DEBUG << "next(recursive) is-dir:" << toint( getStrAttr( *m_iter, "is-dir", "0" ))
                 << " context:" << (*m_iter)->getURL() << endl;
            if( toint( getStrAttr( *m_iter, "is-dir", "0" )) )
            {
                m_recursiveLastParent = m_iter;
                m_iter = m_iter->begin();
            }
            else
            {
                DEBUG << "next... context:" << (*m_iter)->getURL() << endl;
                ++m_iter;
            }
        }
    virtual int eof()
        {
            LG_SQLITE_D << "eof()..." << endl;
            
            if( m_iter == end() )
            {
                DEBUG << "at end...m_recursiveLastParent:" << (*m_recursiveLastParent)->getURL() << endl;
                m_iter = m_recursiveLastParent;
                if( (*m_iter)->getURL() == getBase()->getURL() )
                {
                    DEBUG << "done! earl:" << (*m_iter)->getURL() << endl;
                    m_eof = 1;
                }
                else
                {
                    while( true )
                    {
                        m_iter = m_recursiveLastParent;
                        if( (*m_iter)->getURL() == getBase()->getURL() )
                        {
                            DEBUG << "done! earl:" << (*m_iter)->getURL() << endl;
                            m_eof = 1;
                            break;
                        }
                            
                        m_recursiveLastParent = toContextIterator( (*m_iter)->getParent() );
                        DEBUG << "new m_recursiveLastParent:" << (*m_recursiveLastParent)->getURL() << endl;
                        ++m_iter;
                        if( m_iter != end() )
                        {
                            break;
                        }
                    }
                    DEBUG << "new m_iter:" << (*m_iter)->getURL() << endl;
                    DEBUG << "new m_recursiveLastParent:" << (*m_recursiveLastParent)->getURL() << endl;
                }
            }
            else
            {
                LG_SQLITE_D << "eof() iter:" << m_iter->getURL() << endl;
            }
            
            return m_eof;
        }
};



struct plainc_vtab_cursor
{
    sqlite3_vtab_cursor base;
    virtual_cursor* pp;
};


stringvec_t svec( int argc, const char *const*argv )
{
    stringvec_t ret;
    ret.reserve( argc );
    for( int i=0; i<argc; ++i )
        ret.push_back( argv[i] );
    return ret;
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static int vt_destructor(sqlite3_vtab *p_svt)
{
    vtab *p_vt = (vtab*)p_svt;

    sqlite3_free(p_vt);
    return SQLITE_OK;
}

static int vt_create( sqlite3 *db,
                      void *pAux,
                      int argc, const char *const*argv,
                      sqlite3_vtab **pp_vt,
                      char **pzErr )
{
    LG_SQLITE_D << "create(top)" << endl;
    
    int rc = SQLITE_OK;
    vtab* p_vt;

    /* Allocate the sqlite3_vtab/vtab structure itself */
    p_vt = (vtab*)sqlite3_malloc(sizeof(*p_vt));
    if( !p_vt )
    {
        return SQLITE_NOMEM;
    }

    p_vt->pp = new virtual_table( db );
    p_vt->pp->create( svec( argc, argv ), pAux );

    *pp_vt = &p_vt->base;
    (*pp_vt)->zErrMsg = NULL;

    LG_SQLITE_D << "create(end)" << endl;
    return SQLITE_OK;
}

static int vt_destroy(sqlite3_vtab *p_vt)
{
    virtual_table* x = ((vtab*)p_vt)->pp;
    x->destroy();
    return vt_destructor(p_vt);
}

static int vt_connect( sqlite3 *db, void *p_aux,
                       int argc, const char *const*argv,
                       sqlite3_vtab **pp_vt, char **pzErr )
{
    return vt_create(db, p_aux, argc, argv, pp_vt, pzErr);
}

static int vt_disconnect(sqlite3_vtab *pVtab)
{
    return vt_destructor(pVtab);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static int vt_open(sqlite3_vtab *p_svt, sqlite3_vtab_cursor **pp_cursor)
{
    LG_SQLITE_D << "open(top)" << endl;
    vtab* p_vt         = (vtab*)p_svt;
    p_vt->base.zErrMsg = NULL;
    plainc_vtab_cursor *p_cur = 
        (plainc_vtab_cursor*)sqlite3_malloc(sizeof(plainc_vtab_cursor));
    *pp_cursor = (sqlite3_vtab_cursor*)p_cur;
    if( p_vt->pp->isRecursive() )
        p_cur->pp = new recursive_virtual_cursor( p_vt->pp );
    else
        p_cur->pp = new virtual_cursor( p_vt->pp );
    
    LG_SQLITE_D << "open(end)" << endl;
    return (p_cur ? SQLITE_OK : SQLITE_NOMEM);
}

static int vt_close(sqlite3_vtab_cursor *cur)
{
    LG_SQLITE_D << "vt_close(top)" << endl;
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)cur;
    virtual_cursor* pp = p_cur->pp;
    pp->close();

    
    sqlite3_free(p_cur);
    LG_SQLITE_D << "vt_close(end)" << endl;
    return SQLITE_OK;
}


static int vt_eof(sqlite3_vtab_cursor *cur)
{
    LG_SQLITE_D << "vt_eof()" << endl;
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)cur;
    virtual_cursor* pp = p_cur->pp;
    return pp->eof();
}

static int vt_next(sqlite3_vtab_cursor *cur)
{
    LG_SQLITE_D << "vt_next(top)" << endl;
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)cur;
    virtual_cursor* pp = p_cur->pp;
    pp->next();
    
    LG_SQLITE_D << "vt_next(end)" << endl;
    return SQLITE_OK;
}
    
static int vt_column(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int col)
{
    LG_SQLITE_D << "vt_column(top)" << endl;
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)cur;
    virtual_cursor* pp = p_cur->pp;
    pp->column( ctx, col );
    
    LG_SQLITE_D << "vt_column(end)" << endl;
    return SQLITE_OK;
}

static int vt_rowid(sqlite3_vtab_cursor *cur, sqlite_int64 *p_rowid)
{
    LG_SQLITE_D << "rowid(top)" << endl;
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)cur;
    virtual_cursor* pp = p_cur->pp;
    pp->rowid( *p_rowid );

    LG_SQLITE_D << "rowid(end)" << endl;
    return SQLITE_OK;
}


static int vt_best_index(sqlite3_vtab *tab, sqlite3_index_info *p_info)
{
    LG_SQLITE_D << "best_index(top)" << endl;

    virtual_table* x = ((vtab*)tab)->pp;
    x->best_index( p_info );

    LG_SQLITE_D << "best_index(end)" << endl;
    return SQLITE_OK;
}

static int vt_filter( sqlite3_vtab_cursor *p_vtc, 
                      int idxNum, const char *idxStr,
                      int argc, sqlite3_value **argv )
{
    LG_SQLITE_D << "filter(top) argc:" << argc << endl;
    /* Initialize the cursor structure. */
    plainc_vtab_cursor *p_cur = (plainc_vtab_cursor*)p_vtc;
    virtual_cursor* pp = p_cur->pp;
    sqliteValues_t v;
    for( int i=0; i<argc; ++i )
        v.push_back( argv[i] );
    pp->filter( idxNum, idxStr, v );

    LG_SQLITE_D << "filter(end)" << endl;
    return SQLITE_OK;
}

static int vt_update(
    sqlite3_vtab *tab,
    int argc, sqlite3_value **argv,
    sqlite_int64 *pRowid)
{
    int ret = SQLITE_OK;
    virtual_table* x = ((vtab*)tab)->pp;
    return x->update( argc, argv, pRowid );
}


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

void vt_match_function(sqlite3_context* ctx, int argc, sqlite3_value** argv)
{
    sqlite3_result_int(ctx, 1);
}

int vt_find_function( sqlite3_vtab *pVtab,
                      int nArg,
                      const char *zName,
                      void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
                      void **ppArg )
{
    /* Register the match function */
    if(strcmp(zName, "match") == 0)
    {
        *pxFunc = vt_match_function;

        return 1;
    }

    return SQLITE_OK;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static sqlite3_module libferris_module = 
{
    0,              /* iVersion */
    vt_create,      /* xCreate       - create a vtable */
    vt_connect,     /* xConnect      - associate a vtable with a connection */
    vt_best_index,  /* xBestIndex    - best index */
    vt_disconnect,  /* xDisconnect   - disassociate a vtable with a connection */
    vt_destroy,     /* xDestroy      - destroy a vtable */
    vt_open,        /* xOpen         - open a cursor */
    vt_close,       /* xClose        - close a cursor */
    vt_filter,      /* xFilter       - configure scan constraints */
    vt_next,        /* xNext         - advance a cursor */
    vt_eof,         /* xEof          - inidicate end of result set*/
    vt_column,      /* xColumn       - read data */
    vt_rowid,       /* xRowid        - read data */
    vt_update,      /* xUpdate       */
    NULL,           /* xBegin        - begin transaction */
    NULL,           /* xSync         - sync transaction */
    NULL,           /* xCommit       - commit transaction */
    NULL,           /* xRollback     - rollback transaction */
    vt_find_function           /* xFindFunction - function overloading */
};

static int fs_register(sqlite3 *db)
{
    LG_SQLITE_D << "fs_register" << endl;
    return sqlite3_create_module(db, "libferris", &libferris_module, NULL);
}

extern "C"
{
    int sqlite3_extension_init(
        sqlite3 *db,          /* The database connection */
        char **pzErrMsg,      /* Write error messages here */
        const sqlite3_api_routines *pApi  /* API methods */
        )
    {
        SQLITE_EXTENSION_INIT2(pApi);

        if(fs_register(db) != SQLITE_OK)
        {
            fprintf(stderr, "Failed to register fs module\n");
            return SQLITE_ERROR;
        }

        return SQLITE_OK;
    }
};
