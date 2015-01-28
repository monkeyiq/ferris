/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libeaindexodbc.cpp,v 1.1 2006/12/07 06:49:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <typeinfo>
#include <DTL.h>
//using namespace dtl;

#include "../../fulltextindexers/odbc/ODBCIndexHelper_private.hh"
#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"
#include "FactoriesCreationCommon_private.hh"
#include "EAIndexerSQLCommon_private.hh"

using namespace std;

namespace Ferris
{
    string tostr( TIMESTAMP_STRUCT& t )
    {
        fh_stringstream ss;

        ss << t.year << "-"
           << t.month << "-"
           << t.day << "  "
           << t.hour << ":"
           << t.minute << ":"
           << t.second;

        return tostr(ss);
    }

    
    template <>
        TIMESTAMP_STRUCT toType( const std::string& s )
        {
            TIMESTAMP_STRUCT ret;
            time_t tt = 0;
            
            fh_stringstream ss(s);
            ss >> tt;

            struct tm* TM = 0;
            TM = localtime( &tt );

            ret.year    = 1900 + TM->tm_year;
            ret.month   = 1 + TM->tm_mon;
            ret.day     = 1 + TM->tm_wday;
            ret.hour    = TM->tm_hour;
            ret.minute  = TM->tm_min;
            ret.second  = TM->tm_sec;
            ret.fraction = 0;
            
            return ret;
        }

    struct lt_ts
    {
        bool operator()( const TIMESTAMP_STRUCT& a, const TIMESTAMP_STRUCT& b ) const
            {
                return a.year < b.year
                    && a.month < b.month
                    && a.day < b.day
                    && a.hour < b.hour
                    && a.minute < b.minute
                    && a.second < b.second;
            }
    };
    
    namespace EAIndex 
    {
        static const char* CFG_ODBCIDX_MULTIVERSION_K
        = "cfg-odbcidx-multiversion";
        static const char* CFG_ODBCIDX_MULTIVERSION_DEFAULT = "1";


        static const char* CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K
        = CFG_EAIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K;
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        struct AttrMap
            :
            public Handlable
        {
            struct myRow
            {
                int attrid;
                int attrtype;
                string attrname;
    
                myRow( string attrname = "", int attrid = 0, int attrtype = 0 )
                    :
                    attrname( attrname ), attrid( attrid ), attrtype( attrtype )
                    {
                    }
            };
            struct myFixedRow
            {
                int attrid;
                int attrtype;
                tcstring<SQL_ATTRNAME_LENGTH_INT> attrname;

                myFixedRow( string attrname = "", int attrid = 0, int attrtype = 0 )
                    :
                    attrname( attrname ), attrid( attrid ), attrtype( attrtype )
                    {
                    }
            };
            class FERRISEXP_API myBCA
            {
            public:
                void operator()(BoundIOs &cols, myRow &row)
                    {
                        cols["attrid"]   == row.attrid;
                        cols["attrtype"] == row.attrtype;
                        cols["attrname"] == row.attrname;
                    }
            };
            
            typedef DBView<myRow> DBV;
            DBV view;
            IndexedDBView<DBV> indexed_view;
            IndexedDBView<DBV>::iterator idxview_it;
            IndexedDBView<DBV>::iterator end_it;
            int sz;
            
            AttrMap( DBConnection& con, FetchMode fm )
                :
                view( DBV::Args()
                      .tables("attrmap")
                      .bca( myBCA() )
                      .conn( con )
                    ),
                indexed_view(
                    IndexedDBView<DBV>::Args().view( view )
                    .indexes( "UNIQUE PrimaryIndex; attrid; AlternateIndex; attrname, attrtype" )
                    .bound( BOUND )
                    .key_mode( USE_ALL_FIELDS )
                    .fetch_mode( fm )
                    .fetch_records( 1000 )),
                idxview_it( indexed_view.end() ),
                end_it( idxview_it )
                {
                    sz = indexed_view.size();
                }

            void updateSizeCache()
                {
                    sz = indexed_view.size();
                }
            
            long find( const std::string& an, int att )
                {
                    idxview_it = indexed_view.find_AK( "AlternateIndex", an, att );

                    LG_EAIDX_D << "AttrMap::find() an:" << an << " att:" << att
                               << " attrid:" << ( end_it != idxview_it ? idxview_it->attrid : -1 )
                               << endl;

                    LG_EAIDX_D << "AttrMap::find() dumping indexed_view --- BEGIN --- " << endl;
                    for( IndexedDBView<DBV>::iterator dbi = indexed_view.begin();
                         dbi != indexed_view.end(); ++dbi )
                    {
                        string an = dbi->attrname;
                        int attrid = dbi->attrid;
                        int att = dbi->attrtype;
                        
                        LG_EAIDX_D << "an:" << an << " att:" << att << " attrid:" << attrid << endl;
                    }
                    LG_EAIDX_D << "AttrMap::find() dumping indexed_view --- END --- " << endl;
                    
                    if( end_it != idxview_it )
                        return  idxview_it->attrid;
                    return -1;
                }
            
//             long find( const std::string& an )
//                 {
//                     idxview_it = indexed_view.find( an );
//                     if( end_it != idxview_it )
//                         return idxview_it->attrid;
//                     return -1;
//                 }
            
            // FIXME: could use as a 'support' for type inference.
//             AttrType_t getAttrTypeID( const std::string& an )
//                 {
//                     idxview_it = indexed_view.find( an );
//                     if( end_it != idxview_it )
//                         return (AttrType_t)idxview_it->attrtype;
//                     return ATTRTYPEID_STR;
//                 }

            
            long createNextAttrID()
                {
                    long ret = sz;
                    sz++;
                    return ret;
                }
            long ensure( const std::string& an, int attrtype )
                {
                    idxview_it = indexed_view.find_AK( "AlternateIndex", an, attrtype );
                    if( end_it != idxview_it )
                        return idxview_it->attrid;

                    LG_EAIDX_D << "attrmap.add() an:" << an
                               << " id:" << (sz+1)
                               << " attrtype:" << attrtype
                               << endl;
                    
                    // Finally, insert a new item into the container
                    pair<IndexedDBView<DBV>::iterator, bool> p
                        = indexed_view.insert(
                            myRow( an, createNextAttrID(), attrtype ));
                    return p.second ? p.first->attrid : -1;
                }
                
        };
        FERRIS_SMARTPTR( AttrMap, fh_AttrMap );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        template < class T >
        struct ValueMap
            :
            public Handlable
        {
            typedef T ValueType;
            
            struct myRow
            {
                int vid;
                T attrvalue;
    
                myRow( T attrvalue = T(), int vid = 0 )
                    :
                    attrvalue( attrvalue ), vid( vid )
                    {
                    }
            };
            template <class RealT,class X>
            struct myFixedRowSwitch
            {
                int vid;
                RealT attrvalue;
    
                myFixedRowSwitch( RealT attrvalue = RealT(), int vid = 0 )
                    :
                    attrvalue( attrvalue ), vid( vid )
                    {
                    }
            };
            template <class RealT>
            struct myFixedRowSwitch<RealT,string>
            {
                int vid;
                string attrvalue;
//                tcstring<SQL_STRVAL_LENGTH_INT> attrvalue;
    
                myFixedRowSwitch( string attrvalue = "", int vid = 0 )
                //                myFixedRowSwitch( tcstring<SQL_STRVAL_LENGTH_INT> attrvalue = "", int vid = 0 )
                    :
                    attrvalue( attrvalue ), vid( vid )
                    {
                    }
            };
            typedef myFixedRowSwitch<T,T> myFixedRow;
            
            class FERRISEXP_API myBCA
            {
            public:
                void operator()(BoundIOs &cols, myRow &row)
                    {
                        cols["attrvalue"] == row.attrvalue;
                        cols["vid"]       == row.vid;
                    }
            };

//             struct mySelValidate
//             {
//                 bool operator()( BoundIOs &boundIOs, myRow& row ) 
//                     {
// //                         for (BoundIOs::iterator b_it = boundIOs.begin();
// //                              b_it != boundIOs.end(); b_it++)
// //                         {
// //                             BoundIO &boundIO = (*b_it).second;
// //                             if (boundIO.IsColumn() && boundIO.IsNull())
// //                                 return false;  // found null column ... data is invalid
// //                         }

//                         return true;	// no nulls found ... data is OK
//                     }

//                 ~mySelValidate() {};
//             };

            
            typedef DBView<myRow> DBV;
            DBV view;
            IndexedDBView<DBV> indexed_view;
            typename IndexedDBView<DBV>::iterator idxview_it;
            typename IndexedDBView<DBV>::iterator end_it;
            int sz;

            typedef IndexedDBView< DBView< myRow > > DBVI;
            
            ValueMap( DBConnection& con,
                      const std::string& tableName,
                      int TABLEOFFSET,
                      FetchMode fm )
                :
                view( typename DBV::Args()
                      .tables(tableName)
                      .bca( myBCA() )
                      .conn( con )
//                      .SelValidate( mySelValidate() )
                    ),
                indexed_view(
                    typename DBVI::Args().view( view )
                    .indexes( "UNIQUE PrimaryIndex; attrvalue;" )
                    .bound( BOUND )
                    .key_mode( USE_ALL_FIELDS )
                    .fetch_mode( fm )
                    .fetch_records( 1000 )),
                idxview_it( indexed_view.end() ),
                end_it( idxview_it )
                {
                    sz = indexed_view.size() + TABLEOFFSET;
                }

            void updateSizeCache()
                {
                    sz = indexed_view.size();
                }
            
            
            long find( const std::string& Val )
                {
                    idxview_it = indexed_view.find( toType<T>( Val ) );
                    if( end_it != idxview_it )
                        return idxview_it->vid;
                    return -1;
                }
            long createNextAttrID()
                {
                    long ret = sz;
                    sz++;
                    return ret;
                }
            long ensure( const std::string& Val )
                {
                    idxview_it = indexed_view.find( toType<T>( Val ) );
                    if( end_it != idxview_it )
                        return idxview_it->vid;

                    LG_EAIDX_D << "ValueMap::ensure() inserting id:" << (sz+1)
                               << " Val:" << Val << endl;
                    
                    // Finally, insert a new item into the container
                    pair< typename IndexedDBView<DBV>::iterator, bool> p
                        = indexed_view.insert(
                            myRow( toType<T>( Val ),
                                   createNextAttrID() ));
                    return p.second ? p.first->vid : -1;
                }

            void dump()
                {
                    cerr << "valuemap begin------------------------------------" << endl;
                    typedef typename IndexedDBView<DBV>::iterator ITER;
                    
                    for( ITER iter = indexed_view.begin(); iter != indexed_view.end(); ++iter )
                    {
                        cerr << " vid:" <<  iter->vid
                             << " attrvalue:" <<  iter->attrvalue
                             << endl;
                    }
                    cerr << "--------------------------------------------------" << endl;
                }
        };
        typedef ValueMap< string > ValueMapString_t;
        typedef ValueMap< int >    ValueMapInt_t;
        typedef ValueMap< double > ValueMapDouble_t;
        FERRIS_SMARTPTR( ValueMapString_t, fh_ValueMapString );
        FERRIS_SMARTPTR( ValueMapInt_t,    fh_ValueMapInt );
        FERRIS_SMARTPTR( ValueMapDouble_t, fh_ValueMapDouble );

        typedef ValueMap< TIMESTAMP_STRUCT > ValueMapTime_t;
        FERRIS_SMARTPTR( ValueMapTime_t, fh_ValueMapTime );

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        struct FERRISEXP_DLLLOCAL DocAttrs
            :
            public Handlable
        {
        public:
            struct myRow
            {
                int attrid;
                int vid;
                int docid;
    
                myRow( int docid = 0, int attrid = 0, int vid = 0 )
                    : attrid( attrid ), vid( vid ), docid( docid )
                    {
                    }
            };
            typedef myRow myFixedRow;
            
            class FERRISEXP_API myBCA
            {
            public:
                void operator()(BoundIOs &cols, myRow &row)
                    {
                        cols["docid"]  == row.docid;
                        cols["attrid"] == row.attrid;
                        cols["vid"]    == row.vid;
                    }
            };
            
            typedef DBView<myRow> DBV;
            DBV view;
            IndexedDBView<DBV> indexed_view;
            IndexedDBView<DBV>::iterator idxview_it;
            IndexedDBView<DBV>::iterator end_it;
            
            DocAttrs( DBConnection& con )
                :
                view( DBV::Args()
                      .tables("docattrs")
                      .bca( myBCA() )
                      .conn( con )
                    ),
                indexed_view(
                    view,
                    "UNIQUE PrimaryIndex; docid, attrid;", 
                    BOUND, USE_ALL_FIELDS ),
                idxview_it( indexed_view.end() ),
                end_it( idxview_it )
                {
                }
            
            void set( long docid, long aid, long vid )
                {
                    LG_EAIDX_D << " docattrs.set() docid:" << docid
                               << " aid:" << aid << " vid:" << vid << endl;
                    
                    idxview_it = indexed_view.find( docid, aid );
                    if( end_it != idxview_it )
                    {
                        myRow replacement = *idxview_it;
                        replacement.vid = vid;
                        indexed_view.replace( idxview_it, replacement );
                    }
                    else
                    {
                        // Finally, insert a new item into the container
                        pair<IndexedDBView<DBV>::iterator, bool> p
                            = indexed_view.insert(
                                myRow( docid, aid, vid ));
                    }
                }
                
        };
        FERRIS_SMARTPTR( DocAttrs, fh_DocAttrs );
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL EAIndexerODBC
            :
            public Ferris::Index::ODBCIndexHelper< MetaEAIndexerInterface,
                                                   docid_t >
        {
            
            fh_AttrMap m_attrmap;
            fh_ValueMapString vm_string;
            fh_ValueMapString vm_stringnocase;
            fh_ValueMapInt    vm_int;
            fh_ValueMapDouble vm_double;
            fh_ValueMapTime   vm_time;
            fh_DocAttrs       m_docattrs;
            bool              m_multiVersion;
            stringmap_t       m_ExtraColumnsToInlineInDocmap;
            int               m_filesIndexedCount;

            
            
        protected:
            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            virtual std::string asString( IndexableValue& v, AttrType_t att );
            virtual std::string asString( IndexableValue& v )
                {
                    return asString( v, v.getAttrTypeID() );
                }
            
            
            string  getTableName( AttrType_t att );
            
        public:
            EAIndexerODBC();
            virtual ~EAIndexerODBC();

            virtual void sync();

            virtual void reindexingDocument( fh_context c, docid_t docid );
            typedef map< pair< string, AttrType_t >, AttrMap::myRow  > newAttrMap_t;
            virtual long addToIndex_getAID( fh_context c, fh_docindexer di,
                                            const std::string& k,
                                            AttrType_t att,
                                            IndexableValue& iv,
                                            newAttrMap_t& newAttrMap );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            void AddSQLOp( fh_stringstream& sqlss,
                           const std::string& eaname,
                           const std::string& opcode,
                           IndexableValue& iv,
                           AttrType_t att,
                           stringset_t& lookupTablesUsed );
            AttrType_t
            SQLColumnTypeToAttrType( const std::string& coltype,
                                     IndexableValue& iv );
            void AddSQLOpHeur( fh_stringstream& sqlss,
                               const std::string& eaname,
                               const std::string& opcode,
                               IndexableValue& iv,
                               stringset_t& lookupTablesUsed );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );
            virtual docNumSet_t& BuildQuery(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                fh_stringstream& sqlss,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            virtual docNumSet_t& BuildQuerySQL(
                fh_context q,
                docNumSet_t& output,
                fh_eaquery qobj,
                std::stringstream& SQLHeader,
                std::stringstream& SQLWherePredicates,
                std::stringstream& SQLTailer,
                stringset_t& lookupTablesUsed,
                bool& queryHasTimeRestriction,
                std::string& DocIDColumn,
                stringset_t& eanamesUsed,
                MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            virtual std::string resolveDocumentID( docid_t );

            fh_AttrMap  getAttrMap();
            long        ensureValueMap( const IndexableValue& iv );
            fh_DocAttrs getDocAttrs();

            fh_ValueMapString getValueMapString();
            fh_ValueMapString getValueMapCIS();
            fh_ValueMapInt    getValueMapInt();
            fh_ValueMapDouble getValueMapDouble();
            fh_ValueMapTime   getValueMapTime();
            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/


        EAIndexerODBC::EAIndexerODBC()
            :
            m_attrmap( 0 ),
            vm_string( 0 ),
            vm_stringnocase( 0 ),
            vm_int( 0 ),
            vm_double( 0 ),
            vm_time( 0 ),
            m_docattrs( 0 ),
            m_multiVersion( isTrue( CFG_ODBCIDX_MULTIVERSION_DEFAULT )),
            m_filesIndexedCount( 0 )
        {
        }

        

        EAIndexerODBC::~EAIndexerODBC()
        {
        }
        
        void
        EAIndexerODBC::Setup()
        {
            ODBCSetup();
        }
        
        void
        EAIndexerODBC::CreateIndex( fh_context c,
                                    fh_context md )
        {
            ODBCCreateIndex( c, md );

            m_multiVersion = isTrue( getStrSubCtx( md, "multiversion",
                                                   CFG_ODBCIDX_MULTIVERSION_DEFAULT ));
            setConfig( CFG_ODBCIDX_MULTIVERSION_K, tostr( m_multiVersion ) );
            
            m_ExtraColumnsToInlineInDocmap
                = Util::ParseKeyValueString(
                    getStrSubCtx( md, "extra-columns-to-inline-in-docmap",
                                  CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT )
                    );
            setConfig( CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                       Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                );
            LG_EAIDX_D << "Extra columns for docmap:"
                 << Util::CreateKeyValueString( m_ExtraColumnsToInlineInDocmap, "=", ":" )
                 << endl;
            
        

            /*
             * Create a two strings, a docmap extra attributes which contains
             * the create table parts to create the m_ExtraColumnsToInlineInDocmap
             * columns and a indexing create string to index those columns correctly.
             */
            string extraDocmapColumns = "";
            stringlist_t extraDocmapIndexes;

            if( !m_ExtraColumnsToInlineInDocmap.empty() )
            {
                fh_stringstream colss;
                int n = 0;
                
                for( stringmap_t::const_iterator si = m_ExtraColumnsToInlineInDocmap.begin();
                     si != m_ExtraColumnsToInlineInDocmap.end(); ++si )
                {
                    string colname = EANameToSQLColumnName( si->first );
                    string coltype = si->second;
                    
                    colss << "    " << colname << " " << coltype << ", ";

                    fh_stringstream idxss;
                    idxss << "create index dmecti" << ++n << "idx "
                          << " on docmap ( " << colname << " )";
                    extraDocmapIndexes.push_back( tostr( idxss ) );
                }

                extraDocmapColumns = tostr( colss );
            }
            
            if( m_multiVersion )
            {
                {
                    fh_stringstream ss;
                    ss << "CREATE TABLE docmap ("
                       << "    urlid int NOT NULL,"
                       << "    docid int NOT NULL,"
                       << "    docidtime timestamp,"
                       <<      extraDocmapColumns
                       << "    PRIMARY KEY  (docid)"
                       << "    )";
                    LG_EAIDX_D << "DOCMAP SQL:" << nl
                         << tostr(ss)
                         << endl;
                    Execute( tostr(ss) );
                    LG_EAIDX_D << "DOCMAP CREATED!" << endl;
                }
            
                Execute( "CREATE TABLE urlmap ("
                         "   URL varchar(" SQL_URL_LENGTH_STR ") NOT NULL,"
                         "   urlid integer,"
                         "   primary key( urlid, url )"
                         "  ) " );
                Execute( "create index umurlidx      on urlmap ( urlid )" );
                Execute( "create index dmurlidx      on docmap ( urlid )" );
                Execute( "create index docmapidtmidx on docmap ( docidtime )" );
                Execute( "create index dmurlidtidx   on docmap ( docidtime,urlid )" );
            }
            else
            {
                {
                    fh_stringstream ss;
                    ss << "CREATE TABLE docmap ("
                       << "    URL varchar(" SQL_URL_LENGTH_STR ") NOT NULL default '',"
                       << "    docid int NOT NULL,"
                       <<      extraDocmapColumns
                       << "    PRIMARY KEY  (docid)"
                       << "    )";
                    
                    Execute( tostr(ss) );
                }
                
                Execute( "create index docmapurlidx  on docmap ( url )" );
            }

            if( !extraDocmapIndexes.empty() )
                Execute( extraDocmapIndexes );
            
            
            // auto_increment is non SQL92
            stringlist_t commands;

            commands.push_back( 
                "CREATE TABLE attrmap ("
                "    attrid int NOT NULL default 0,"
                "    attrtype int NOT NULL default 1,"
                "    attrname varchar(" SQL_ATTRNAME_LENGTH_STR ") NOT NULL default '',"
                "    PRIMARY KEY  (attrid)"
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE docattrs ("
                "    attrid int NOT NULL default 0,"
                "    vid int NOT NULL default 0,"
                "    docid int NOT NULL default 0,"
                "    PRIMARY KEY  (attrid,docid,vid)"
                "    )" );
            
            commands.push_back( 
                (string)("CREATE TABLE strlookup ("
                         "    vid int NOT NULL,"
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize()) + "),"
                + "    PRIMARY KEY  (attrvalue)"
                + "    )" );
                
            commands.push_back( 
                (string)("CREATE TABLE strlookupnocase ("
                         "    vid int NOT NULL,"
                         "    attrvalue varchar(")
                + tostr(getMaxValueSize()) + "),"
                + "    PRIMARY KEY  (attrvalue)"
                + "    )" );
            
                    
            commands.push_back( 
                "CREATE TABLE doublelookup ("
                "    vid int NOT NULL,"
                "    attrvalue real,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE intlookup ("
                "    vid int NOT NULL,"
                "    attrvalue int,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );
            
            commands.push_back( 
                "CREATE TABLE timelookup ("
                "    vid int NOT NULL,"
                "    attrvalue timestamp,"
                "    PRIMARY KEY  (attrvalue)"
                "    )" );

            Execute( commands );

            /**
             * Create the indexes for using the database effectively.
             */
            static const char* defaultindexing_commands[] = {

                "create index attrnameidx   on attrmap      ( attrtype, attrname )",
                "create index attrididx     on docattrs     ( attrid )",
                "create index vididx        on docattrs     ( vid )",
                "create index avididx       on docattrs     ( attrid,vid )",

                // PK takes care of attrvalue for us, index the vid for joining.
//                 "create index strlookupidx  on strlookup    ( attrvalue )",
//                 "create index strnclkupidx  on strlookupnocase ( attrvalue )",
//                 "create index intvalueidx   on intlookup    ( attrvalue )",
//                 "create index doublelkpidx  on doublelookup ( attrvalue )",
//                 "create index timelkpidx    on timelookup   ( attrvalue )",
                "create index strlookupidx  on strlookup    ( attrvalue,vid )",
                "create index strnclkupidx  on strlookupnocase ( attrvalue,vid )",
                "create index intvalueidx   on intlookup    ( attrvalue,vid )",
                "create index doublelkpidx  on doublelookup ( attrvalue,vid )",
                "create index timelkpidx    on timelookup   ( attrvalue,vid )",
                
                "create index docmapididx   on docmap       ( docid )",
                0
            };
            const char** indexing_commands = defaultindexing_commands;

            Execute( indexing_commands );
            // Don't mix DML and DDL in SQL99 compliant apps.
            getConnection().CommitAll();

            // make sure the empty string is awaiting us in the lookup tables.
            {
                stringlist_t ddl;
                ddl.push_back( "insert into strlookup (vid,attrvalue) values (0,'') " );
                ddl.push_back( "insert into strlookupnocase (vid,attrvalue) values (0,'') " );
                Execute( ddl );
                getConnection().CommitAll();
            }

            // we always have access to the URL string
            // and map queries to use that string instead of strlookup.
            appendToEANamesIgnore("url");
        }
        
        void
        EAIndexerODBC::CommonConstruction()
        {
            m_multiVersion = isTrue(
                getConfig( CFG_ODBCIDX_MULTIVERSION_K,
                           CFG_ODBCIDX_MULTIVERSION_DEFAULT ));

            m_ExtraColumnsToInlineInDocmap =
                Util::ParseKeyValueString(
                    getConfig( CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_K,
                               CFG_ODBCIDX_EXTRA_COLUMNS_TO_INLINE_IN_DOCMAP_DEFAULT ));
        }
        
        
        void
        EAIndexerODBC::sync()
        {
            if( getDBType() == "postgresql" )
            {
                LG_EAIDX_D << "EAIndexerODBC::sync() vacuum analyse" << endl;
                fh_stringstream ss;
                ss << "VACUUM ANALYZE";
                Execute( tostr(ss) );
            }
            ODBCsync();
        }
        
        void
        EAIndexerODBC::reindexingDocument( fh_context c, docid_t docid )
        {
            if( m_multiVersion )
            {
                LG_EAIDX_D << "EAIndexerODBC::reindexingDocument(MV) c:" << c->getURL() << endl;
            }
            else
            {
                LG_EAIDX_D << "EAIndexerODBC::reindexingDocument() c:" << c->getURL() << endl;
                Execute( "delete from docattrs where docid = " + tostr(docid) );
            }
        }


        string
        EAIndexerODBC::asString( IndexableValue& v, AttrType_t att )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                return tostr(convertStringToInteger( v.rawValueString() ));
            case ATTRTYPEID_DBL:
                return v.rawValueString();
            case ATTRTYPEID_TIME:
            {
//                 LG_EAIDX_D << "EAIndexerODBC::asString() "
//                            << " v:" << v.rawValueString()
//                            << " time:" << Time::ParseRelativeTimeString( v.rawValueString() )
//                            << " sqltime:" << toSQLTimeString( Time::ParseRelativeTimeString( v.rawValueString() ))
//                            << endl;
                stringstream ss;
                ss << "'"
//                   << toSQLTimeString( Time::ParseRelativeTimeString( v.rawValueString() ))
                   << toSQLTimeString( convertStringToInteger( v.rawValueString() ))
                   << "'";
                return ss.str();
            }
            case ATTRTYPEID_CIS:
            case ATTRTYPEID_STR:
            {
                fh_stringstream ss;
                ss << "'";
                
//                 if( v.rawValueString().empty() )
//                 {
//                     ss << "NULL";
//                 }
                if( v.isCaseSensitive() )
                    ss << stripNullCharacters( v.rawValueString() );
                else
                    ss << stripNullCharacters( tolowerstr()( v.rawValueString() ) );
                ss << "'";
                return tostr(ss);
            }
            }
            return v.rawValueString();
        }

        string 
        EAIndexerODBC::getTableName( AttrType_t att )
        {
            switch( att )
            {
            case ATTRTYPEID_INT:
                return "intlookup";
            case ATTRTYPEID_DBL:
                return "doublelookup";
            case ATTRTYPEID_TIME:
                return "timelookup";
            case ATTRTYPEID_STR:
                return "strlookup";
            case ATTRTYPEID_CIS:
                return "strlookupnocase";
            }
            return "New table type not defined!";
        }
        

        template<class InputIterator, class InsertIterator> void 
        bulk_insert_helper_with_fallback(InputIterator beg,
                                         InputIterator end,
                                         size_t buffer_size, 
                                         InsertIterator ins_it)
        {
            std::copy( beg, end, ins_it );

//             try
//             {
//                 bulk_insert_helper( beg, end, buffer_size, ins_it );
//             }
//             catch( DBException& e )
//             {
//                 STD_::pair<tstring, tstring> p = e.GetODBCError();
//                 cerr << "bulk_insert_helper_with_fallback() falling back due to"
//                      << " EXCEPTION e:" << e.what()
//                      << " EXK:" << p.first << " EXV:" << p.second << endl;
                
//                 std::copy( beg, end, ins_it );
//             }
        }

        /**
         * postgresql doesn't support ODBC v3.0 bulk writes. An attempt at
         * them taints the cursor aswell.
         */
        template<class InputIterator, class InsertIterator>
        void 
        bulk_insert_with_fallback( InputIterator beg,
                                   InputIterator end,
                                   int collection_size,
                                   InsertIterator ins_it)
        {
            if( !collection_size )
                return;

            for( InputIterator iter = beg; iter != end; ++iter )
            {
                *ins_it = iter->second;
            }
            
//             typedef typename iterator_traits<InputIterator>::value_type::second_type value_type;
//             value_type* rows = new value_type[ collection_size + 1 ];

//             int i = 0;
//             for( InputIterator iter = beg; iter != end; ++iter )
//             {
//                 rows[ i ] = iter->second;
//                 ++i;
//             }
            
//             try
//             {
//                 bulk_copy(&rows[0], &rows[ 1 ], ins_it, false );
// //                bulk_copy(&rows[0], &rows[ collection_size ], ins_it, false );
//                 delete [] rows;
//             }
//             catch( DBException& e )
//             {
//                 delete [] rows;
//                 STD_::pair<tstring, tstring> p = e.GetODBCError();
//                 cerr << "----------------------------------------\n"
//                      << "bulk_insert_helper_with_fallback() falling back due to"
//                      << " EXCEPTION e:" << e.what()
//                      << " EXK:" << p.first << " EXV:" << p.second
//                      << " collection_size:" << collection_size
//                      << " i:" << i
//                      << "----------------------------------------\n"
//                      << endl;

//                 for( InputIterator iter = beg; iter != end; ++iter )
//                 {
//                     *ins_it = iter->second;
//                 }
//             }
        }
        

        long
        EAIndexerODBC::addToIndex_getAID( fh_context c, fh_docindexer di,
                                          const std::string& k,
                                          AttrType_t att,
                                          IndexableValue& iv,
                                          newAttrMap_t& newAttrMap )
        {
            long aid = getAttrMap()->find( k, att );

            LG_EAIDX_D << "addContextToIndex(ea-prep) c:" << c->getURL() << endl
                       << " k:" << k
                       << " att:" << att
                       << " initial-aid:" << aid
                       << " v:" << iv.rawValueString()
                       << endl;
                    
            if( aid == -1 )
            {
                newAttrMap_t::const_iterator iter = newAttrMap.find( make_pair( k, att ) );

                LG_EAIDX_D << "addContextToIndex(create-attr) c:" << c->getURL() << endl
                           << " k:" << k
                           << " att:" << att
                           << " create-attr-test:" <<
                    (iter != newAttrMap.end() ? iter->second.attrid : -1)
                           << endl;

                if( iter != newAttrMap.end() )
                    aid = iter->second.attrid;
                else
                {
                    aid = getAttrMap()->createNextAttrID();

                    LG_EAIDX_D << "addContextToIndex(create-attr) c:" << c->getURL() << endl
                               << " k:" << k
                               << " att:" << att
                               << " create-attr-aid:" << aid
                               << endl;

                    newAttrMap.insert(
                        make_pair(
                            make_pair( k, att ),
                            AttrMap::myRow( k,
                                            aid,
                                            att )
                            )
                        );
                }
            }

            return aid;
        }
        
        void
        EAIndexerODBC::addToIndex( fh_context c,
                                   fh_docindexer di )
        {
            bool hadError = false;
            string driver = getDBType();
            string earl   = c->getURL();
            
            int attributesDone = 0;
            int signalWindow   = 0;
            stringlist_t slist = Util::parseCommaSeperatedList( getStrAttr( c, "ea-names", "" ));
            int totalAttributes = slist.size();

            Time::Benchmark bm( "earl:" + earl );
            bm.start();
            
            LG_EAIDX_D << "EAIndexerODBC::addToIndex() earl:" << earl << endl;
            long docid  = obtainDocumentID( c, m_multiVersion );
            LG_EAIDX_D << "EAIndexerODBC::addToIndex() docid:" << docid
                       << " m_multiVersion:" << m_multiVersion
                       << endl;

            typedef list< IndexableValue > ivl_t;
            ivl_t ivl;

            for( stringlist_t::iterator si = slist.begin(); si != slist.end(); ++si )
            {
                try
                {
                    string attributeName = *si;
                    string k = attributeName;
                    string v = "";
                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v ))
                        continue;

                    LG_EAIDX_D << "EAIndexerODBC::addToIndex() attributeName:" << attributeName << endl;
                    LG_EAIDX_D << "addContextToIndex(a) c:" << c->getURL() << endl
                               << " k:" << k << " v:" << v << endl;

                    IndexableValue iv  = getIndexableValue( c, k, v );
                    ivl.push_back( iv );
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                }
            }

            
            //
            // Add the EA which are ment to be indexed in the docmap table
            //
            {
                stringlist_t insertSQL;
                for( ivl_t::iterator ivi = ivl.begin(); ivi!=ivl.end(); )
                {
                    IndexableValue& iv = *ivi;
                    const std::string& k = iv.rawEANameString();
                    XSDBasic_t     sct = iv.getSchemaType();

                    stringmap_t::const_iterator eci
                        = m_ExtraColumnsToInlineInDocmap.find( k );

                    if( eci != m_ExtraColumnsToInlineInDocmap.end() )
                    {                        
                        fh_stringstream sqlss;
                        sqlss << "update docmap "
                              << " set " << EANameToSQLColumnName(k) << " = " << asString( iv )
                              << " where docid = " << docid << endl;
                        LG_EAIDX_D << "inline docmap attr SQL:" << tostr(sqlss) << endl;
//                         if( k == "width" )
//                         {
//                             cerr << "SQL-WIDTH... " << tostr(sqlss) << endl;
//                         }
                        
                        insertSQL.push_back( tostr( sqlss ) );
                        ivl_t::iterator t = ivi;
                        ++ivi;
                        ivl.erase( t );

                        if( signalWindow > 5 )
                        {
                            signalWindow = 0;
                            di->getProgressSig().emit( c, attributesDone, totalAttributes );
                        }
                        ++attributesDone;
                        ++signalWindow;
                    }
                    else
                    {
                        ++ivi;
                    }
                }
                try
                {
                    Execute( insertSQL );
                }
                catch( DBException& e )
                {
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    STD_::pair<tstring, tstring> p = e.GetODBCError();
                    LG_EAIDX_D << "EXK:" << p.first << " EXV:" << p.second << endl;
                    hadError = true;
                }
            }
            

            //
            // Make the NON INLINED docmap insertions
            //
            // The above loop should have removed all the items that are added
            // inline.
            //
//            typedef map< pair< string, AttrType_t >, AttrMap::myRow  > newAttrMap_t;
            typedef list< DocAttrs::myRow > newDocAttr_t;
            typedef map< string, ValueMapString_t::myRow > newValueMapString_t;
            typedef map< string, ValueMapString_t::myRow > newValueMapCIS_t;
            typedef map< string, ValueMapInt_t::myRow    > newValueMapInt_t;
            typedef map< string, ValueMapDouble_t::myRow > newValueMapDouble_t;
            typedef map< TIMESTAMP_STRUCT, ValueMapTime_t::myRow, lt_ts > newValueMapTime_t;

            newAttrMap_t        newAttrMap;
            newDocAttr_t        newDocAttr;
            newValueMapString_t newValueMapString;
            newValueMapCIS_t    newValueMapCIS;
            newValueMapInt_t    newValueMapInt;
            newValueMapDouble_t newValueMapDouble;
            newValueMapTime_t   newValueMapTime;

            for( ivl_t::iterator ivi = ivl.begin(); ivi!=ivl.end(); ++ivi )
            {
                IndexableValue& iv = *ivi;
                const std::string& k = iv.rawEANameString();
                XSDBasic_t sct = iv.getSchemaType();
                AttrType_t att = iv.getAttrTypeID();
                if( att == ATTRTYPEID_CIS )
                    att = ATTRTYPEID_STR;

                try
                {
                    LG_EAIDX_D << "addContextToIndex(ea-prep) c:" << c->getURL() << endl
                               << " k:" << k
//                               << " aid:" << aid
                               << " sct:" << sct
                               << " attrTypeID:" << iv.getAttrTypeID()
                               << " v:" << iv.rawValueString()
                               << endl;

                    long aid = addToIndex_getAID( c, di, k, att, iv, newAttrMap );
                    
                    LG_EAIDX_D << "addContextToIndex(have aid) c:" << c->getURL()
                               << " k:" << k
                               << " att:" << iv.getAttrTypeID()
                               << " aid:" << aid
                               << endl;
                    
                    long vid = -1;
                    string v = iv.rawValueString();
                    
                    if( sct == FXD_UNIXEPOCH_T || att == ATTRTYPEID_TIME )
                    {
                        vid = getValueMapTime()->find( v );
                        if( vid < 0 )
                        {
                            TIMESTAMP_STRUCT tv = toType<TIMESTAMP_STRUCT>( v );
                            newValueMapTime_t::const_iterator iter = newValueMapTime.find( tv );
                            if( iter != newValueMapTime.end() )
                                vid = iter->second.vid;
                            else
                            {
                                vid = getValueMapTime()->createNextAttrID();
                                newValueMapTime.insert(
                                    make_pair(
                                        tv,
                                        ValueMapTime_t::myRow( 
                                            toType<ValueMapTime_t::ValueType>( v ),
                                            vid )
                                        )
                                    );
                            }
                        }
                    }
                    else if( att == ATTRTYPEID_INT )
                    {
                        vid = getValueMapInt()->find( v );
                        if( vid < 0 )
                        {
                            newValueMapInt_t::const_iterator iter = newValueMapInt.find( v );
                            if( iter != newValueMapInt.end() )
                                vid = iter->second.vid;
                            else
                            {
                                vid = getValueMapInt()->createNextAttrID();
                                newValueMapInt.insert(
                                    make_pair(
                                        v,
                                        ValueMapInt_t::myRow( 
                                            toType<ValueMapInt_t::ValueType>( v ),
                                            vid )
                                        )
                                    );
                            }
                        }
                    }
                    else if( att == ATTRTYPEID_DBL )
                    {
                        vid = getValueMapDouble()->find( v );
                        if( vid < 0 )
                        {
                            newValueMapDouble_t::const_iterator iter = newValueMapDouble.find( v );
                            if( iter != newValueMapDouble.end() )
                                vid = iter->second.vid;
                            else
                            {
                                vid = getValueMapDouble()->createNextAttrID();
                                newValueMapDouble.insert(
                                    make_pair(
                                        v,
                                        ValueMapDouble_t::myRow( 
                                            toType<ValueMapDouble_t::ValueType>( v ),
                                            vid )
                                        )
                                    );
                            }
                        }
                    }
                    else
                    {
                        v = stripNullCharacters( v );
                        vid = getValueMapString()->find( v );
                        
                        if( vid < 0 )
                        {
                            newValueMapString_t::const_iterator iter = newValueMapString.find( v );
                            if( iter != newValueMapString.end() )
                                vid = iter->second.vid;
                            else
                            {
                                LG_EAIDX_D << "addContextToIndex(str-add-vid) c:" << c->getURL() << endl
                                           << " k:" << k
                                           << " v:" << iv.rawValueString()
                                           << " vid:" << vid
                                           << " sct:" << sct
                                           << " attrTypeID:" << iv.getAttrTypeID()
                                           << endl;

                                vid = getValueMapString()->createNextAttrID();
                                LG_EAIDX_D << "Inserting new string k:" << k << " vid:" << vid << " v:" << v << endl;
                                newValueMapString.insert(
                                    make_pair( v, ValueMapString_t::myRow(  v, vid ) ) );
//                                 cerr << "New docattrs row1. docid:" << docid
//                                      << " aid:" << aid << " vid:" << vid << endl;
                                newDocAttr.push_back( DocAttrs::myRow( docid, aid, vid ) ); 
                           }
                        }

                        att = ATTRTYPEID_CIS;
                        aid = addToIndex_getAID( c, di, k, att, iv, newAttrMap );
                        string lowerstr = tolowerstr()( v );
                        long CSvid = vid;
                        vid = getValueMapCIS()->find( lowerstr );
                        if( vid < 0 )
                        {
                            newValueMapString_t::const_iterator iter = newValueMapCIS.find( lowerstr );
                            if( iter != newValueMapCIS.end() )
                                vid = iter->second.vid;
                            else
                            {
                                vid = getValueMapCIS()->createNextAttrID();
                                LG_EAIDX_D << "addContextToIndex(cis) c:" << c->getURL()
                                           << " vid:" << vid << " CSvid:" << CSvid
                                           << endl;
                                
                                if( CSvid == vid )
                                {
                                    fh_stringstream ss;
                                    ss << "Strlookup and nocase tables must be unique. Dummy value."
                                       << " vid:" << vid;
                                    string s = tostr(ss);

                                    newValueMapCIS.insert(
                                        make_pair( s, ValueMapString_t::myRow( s, vid )));
                                    vid = getValueMapCIS()->createNextAttrID();
                                    LG_EAIDX_D << "addContextToIndex(6.2) c:" << c->getURL()
                                               << " vid:" << vid << " CSvid:" << CSvid
                                               << endl;
                                }
                                
                                newValueMapCIS.insert(
                                    make_pair( lowerstr, ValueMapString_t::myRow( lowerstr, vid )));
                            }
                        }
                    }

                    LG_EAIDX_D << "New docattrs row. docid:" << docid
                               << " aid:" << aid << " vid:" << vid << endl;
                    
                    newDocAttr.push_back( DocAttrs::myRow( docid, aid, vid ) );
                
                }
                catch( DBException& e )
                {
                    hadError = true;
                        
                    LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                    STD_::pair<tstring, tstring> p = e.GetODBCError();
                    LG_EAIDX_D << "EXK:" << p.first << " EXV:" << p.second << endl;

                    LG_EAIDX_D << "making non-inline ea tables. "
                         << " e:" << e.what()
                         << " EXK:" << p.first << " EXV:" << p.second << endl;
                }
                
                if( signalWindow > 5 )
                {
                    signalWindow = 0;
                    di->getProgressSig().emit( c, attributesDone, totalAttributes );
                }
                ++attributesDone;
                ++signalWindow;
            }

            //
            // Insert the new rows in some manner.
            // * ODBC 3.0 bulk inserts don't work for psql ODBC driver.
            // * Inserting directly into the DBView means that the
            //   IndexedDBView doesn't update.
            //
            try
            {

                for( newAttrMap_t::iterator it = newAttrMap.begin();
                     it != newAttrMap.end(); ++it )
                {
                    try {
                        AttrMap::myRow& r = it->second;

                        LG_EAIDX_D << "Adding to AttrMap. aid:" << r.attrid
                                   << " att:" << r.attrtype << " attrname:" << r.attrname << endl;


                        pair<IndexedDBView< DBView< AttrMap::myRow> >::iterator,bool> rc;
                        
                        rc = getAttrMap()->indexed_view.insert( r );
                        if( !rc.second )
                            LG_EAIDX_I << "Failed to add to AttrMap. aid:" << r.attrid
                                       << " att:" << r.attrtype << " attrname:" << r.attrname << endl;
                    }
                    catch( exception& e )
                    {
                        LG_EAIDX_D << "ERROR:" << e.what() << endl;
                    }
                }

                LG_EAIDX_D << "Adding other data to db" << endl;

                for( newValueMapString_t::iterator it = newValueMapString.begin();
                     it != newValueMapString.end(); ++it )
                {
                    ValueMapString_t::myRow& r = it->second;
                    getValueMapString()->indexed_view.insert( r );
                }
                
                for( newValueMapString_t::iterator it = newValueMapCIS.begin();
                     it != newValueMapCIS.end(); ++it )
                {
                    ValueMapString_t::myRow& r = it->second;
                    getValueMapCIS()->indexed_view.insert( r );
                }

                for( newValueMapInt_t::iterator it = newValueMapInt.begin();
                     it != newValueMapInt.end(); ++it )
                {
                    ValueMapInt_t::myRow& r = it->second;
                    getValueMapInt()->indexed_view.insert( r );
                }

                for( newValueMapDouble_t::iterator it = newValueMapDouble.begin();
                     it != newValueMapDouble.end(); ++it )
                {
                    ValueMapDouble_t::myRow& r = it->second;
                    getValueMapDouble()->indexed_view.insert( r );
                }

                for( newValueMapTime_t::iterator it = newValueMapTime.begin();
                     it != newValueMapTime.end(); ++it )
                {
                    ValueMapTime_t::myRow& r = it->second;
                    getValueMapTime()->indexed_view.insert( r );
                }

                for( newDocAttr_t::iterator it = newDocAttr.begin();
                     it != newDocAttr.end(); ++it )
                {
                    DocAttrs::myRow& r = *it;
                    getDocAttrs()->indexed_view.insert( r );
                }

                

                /********************************************************************************/
                /********************************************************************************/
                /********************************************************************************/

                
//                 int buffer_size = 64000;

// //                 LG_EAIDX_D << "EAIndexerODBC::addToIndex()"
// //                      << " int attrs.size:" << newValueMapInt.size()
// //                      << " newAttrMap.size:" << newAttrMap.size()
// //                      << endl;
                
//                 {
//                     DBView<AttrMap::myRow>::insert_iterator ins_it = getAttrMap()->view;

// //                     for( newAttrMap_t::iterator it = newAttrMap.begin();
// //                          it != newAttrMap.end(); ++it )
// //                     {
// //                         AttrMap::myRow& r = it->second;

// //                         LG_EAIDX_D << "Inserting attrmap row."
// //                              << " aid:" << r.attrid
// //                              << " attrtype:" << r.attrtype
// //                              << " attrname:" << r.attrname
// //                              << endl;
// //                         *ins_it = r;
                        
// //                     }
                    
//                     bulk_insert_with_fallback( newAttrMap.begin(), newAttrMap.end(),
//                                                newAttrMap.size(), ins_it );
//                 }
//                 {
//                     DBView<ValueMapString_t::myRow>::insert_iterator ins_it = getValueMapString()->view;

//                     for( newValueMapString_t::iterator it = newValueMapString.begin();
//                          it != newValueMapString.end(); ++it )
//                     {
//                         ValueMapString_t::myRow& r = it->second;

//                         try
//                         {
//                             LG_EAIDX_D << "Inserting strlookup row."
//                                  << " vid:" << r.vid
//                                  << " attrvalue:" << r.attrvalue
//                                  << " tostr(attrvalue):" << toType<string>(r.attrvalue)
//                                  << endl;
//                             *ins_it = r;
//                         }
//                         catch( DBException& e )
//                         {
//                             STD_::pair<tstring, tstring> p = e.GetODBCError();
//                             LG_EAIDX_D << "inserting a string..."
//                                  << " EXCEPTION e:" << e.what()
//                                  << " EXK:" << p.first << " EXV:" << p.second << endl;
//                         }
//                     }
                    
// //                     bulk_insert_with_fallback( newValueMapString.begin(), newValueMapString.end(),
// //                                                buffer_size, ins_it );
//                 }
//                 {
// //                    LG_EAIDX_D << "--- Bulk inserting CIS size:" << newValueMapCIS.size() << endl;
//                     DBView<ValueMapString_t::myRow>::insert_iterator ins_it = getValueMapCIS()->view;
// //                    LG_EAIDX_D << "--- Bulk inserting CIS sizeB:" << newValueMapCIS.size() << endl;

//                     for( newValueMapString_t::iterator it = newValueMapCIS.begin();
//                          it != newValueMapCIS.end(); ++it )
//                     {
//                         ValueMapString_t::myRow& r = it->second;

//                         try
//                         {
//                             LG_EAIDX_D << "Inserting CIS strlookup row."
//                                  << " vid:" << r.vid
//                                  << " attrvalue:" << r.attrvalue
//                                  << endl;
//                             *ins_it = r;
//                         }
//                         catch( DBException& e )
//                         {
//                             STD_::pair<tstring, tstring> p = e.GetODBCError();
//                             LG_EAIDX_D << "...falling back due to"
//                                  << " EXCEPTION e:" << e.what()
//                                  << " EXK:" << p.first << " EXV:" << p.second << endl;
//                         }
//                     }
                    

// //                     LG_EAIDX_D << "--- Bulk inserting CIS size2:" << newValueMapCIS.size() << endl;
// //                     bulk_insert_with_fallback( newValueMapCIS.begin(), newValueMapCIS.end(),
// //                                                newValueMapCIS.size(), ins_it );
//                 }
//                 {
// //                    LG_EAIDX_D << "--- Bulk inserting int sz:" << newValueMapInt.size() << endl;
//                     DBView<ValueMapInt_t::myRow>::insert_iterator ins_it = getValueMapInt()->view;
//                     bulk_insert_with_fallback( newValueMapInt.begin(), newValueMapInt.end(),
//                                                newValueMapInt.size(), ins_it );
//                 }
//                 {
// //                    LG_EAIDX_D << "--- Bulk inserting dbl" << endl;
//                     DBView<ValueMapDouble_t::myRow>::insert_iterator ins_it = getValueMapDouble()->view;
//                     bulk_insert_with_fallback( newValueMapDouble.begin(), newValueMapDouble.end(),
//                                                newValueMapDouble.size(), ins_it );
//                 }
//                 {
// //                    LG_EAIDX_D << "--- Bulk inserting time" << endl;
//                     DBView<ValueMapTime_t::myRow>::insert_iterator ins_it = getValueMapTime()->view;

//                     for( newValueMapTime_t::iterator it = newValueMapTime.begin();
//                          it != newValueMapTime.end(); ++it )
//                     {
//                         ValueMapTime_t::myRow& r = it->second;

//                         try
//                         {
//                             LG_EAIDX_D << "Inserting time value."
//                                  << " vid:" << r.vid
//                                  << " attrvalue:" << tostr( r.attrvalue )
//                                  << endl;
//                             *ins_it = r;
//                         }
//                         catch( DBException& e )
//                         {
//                             STD_::pair<tstring, tstring> p = e.GetODBCError();
//                             LG_EAIDX_D << "inserting a TIME VALUE FAILED"
//                                  << " EXCEPTION e:" << e.what()
//                                  << " EXK:" << p.first << " EXV:" << p.second << endl;
//                         }
//                     }

                    
// //                     bulk_insert_with_fallback( newValueMapTime.begin(), newValueMapTime.end(),
// //                                                newValueMapTime.size(), ins_it );
//                 }
//                 {
// //                    LG_EAIDX_D << "--- Bulk inserting docattrs" << endl;
                    
//                     DBView<DocAttrs::myRow>::insert_iterator ins_it = getDocAttrs()->view;

// //                     for( newDocAttr_t::iterator it = newDocAttr.begin();
// //                          it != newDocAttr.end(); ++it )
// //                     {
// //                         DocAttrs::myRow& r = *it;

// //                         LG_EAIDX_D << "Inserting docattrs row."
// //                              << " docid:" << r.docid
// //                              << " aid:" << r.attrid
// //                              << " vid:" << r.vid
// //                              << endl;
// //                         *ins_it = r;
// //                     }
                    
//                     bulk_insert_helper_with_fallback( newDocAttr.begin(), newDocAttr.end(),
//                                                       buffer_size, ins_it );
//                 }
            }
            catch( DBException& e )
            {
                hadError = true;
                
                LG_EAIDX_D << "EXCEPTION e:" << e.what() << endl;
                STD_::pair<tstring, tstring> p = e.GetODBCError();
                LG_EAIDX_D << "EXK:" << p.first << " EXV:" << p.second << endl;

                LG_EAIDX_W << "ERROR EXCEPTION e:" << e.what() << " EXK:" << p.first << " EXV:" << p.second
                           << " url:" << earl << " docid:" << docid
                           << endl;
            }

            if( hadError )
            {
                LG_EAIDX_W << "addToIndex() had an error with file:" << c->getURL() << endl;
                cerr << "addToIndex() had an error with file:" << c->getURL() << endl;
                getConnection().RollbackAll();
            }
            else
                getConnection().CommitAll();

            getAttrMap()->updateSizeCache();
            getValueMapString()->updateSizeCache();
            getValueMapCIS()->updateSizeCache();
            getValueMapInt()->updateSizeCache();
            getValueMapDouble()->updateSizeCache();
            getValueMapTime()->updateSizeCache();
            
            ++m_filesIndexedCount;
        }


        
        void
        EAIndexerODBC::AddSQLOp( fh_stringstream& sqlss,
                                 const std::string& eaname,
                                 const std::string& opcode,
                                 IndexableValue& iv,
                                 AttrType_t att,
                                 stringset_t& lookupTablesUsed )
        {
            string lookupTableName = getTableName( att );
            lookupTablesUsed.insert( lookupTableName );

            stringmap_t::const_iterator eci
                = m_ExtraColumnsToInlineInDocmap.find( eaname );

            LG_EAIDX_D << "EAIndexerODBC::AddSQLOp() "
                       << " v:" << iv.rawValueString()
                       << " v.asstring:" << asString( iv, att )
                       << endl;
            
            if( eaname == "url" )
            {
                if( m_multiVersion )
                {
                    sqlss << "d.docid in" << nl
                          << "   ("
                          << "    SELECT distinct(docid) as docid " << nl
                          << "    FROM docmap, urlmap " << nl
                          << "    WHERE urlmap.urlid = docmap.urlid " << nl
                          << "    AND   urlmap.url " << opcode << asString( iv, att ) << nl
                          << "   )"
                          << endl;
                }
                else
                {
                    sqlss << "d.url " << opcode << asString( iv, att ) << endl;
                }
            }
            else if( eci != m_ExtraColumnsToInlineInDocmap.end() )
            {
                string colname = EANameToSQLColumnName( eci->first );
                string coltype = eci->second;

                sqlss << "d." << colname << " " << opcode << asString( iv, att );
            }
            else
            {
                sqlss << "d.docid in" << nl
                      << "   ("
                      << "    SELECT distinct(docattrs.docid) as docid " << nl
                      << "      FROM docattrs, attrmap , " << lookupTableName << nl
                      << "      WHERE " << nl
                      << "      (( "   << nl
                      << "         " << lookupTableName << ".attrvalue " << opcode << " "
                      << "         " << asString( iv, att ) << " "    << nl
                      << "       and " << lookupTableName << ".vid = docattrs.vid " << nl
                      << "       and " << " attrmap.attrname=\'" << eaname << "\' " << nl
                      << "       and " << " attrmap.attrtype=\'" << att << "\' " << nl
                      << "       and " << " attrmap.attrid=docattrs.attrid "        << nl
                      << "      ))"                                                 << nl
                      << "   )"
                      << endl;
            }
        }

        MetaEAIndexerInterface::AttrType_t
        EAIndexerODBC::SQLColumnTypeToAttrType( const std::string& coltype,
                                                IndexableValue& iv )
        {
            AttrType_t att = ATTRTYPEID_CIS;
            if( starts_with( coltype, "int" ))
                att = ATTRTYPEID_INT;
            if( starts_with( coltype, "double" ) || starts_with( coltype, "float" ) )
                att = ATTRTYPEID_DBL;
            if( starts_with( coltype, "timestamp" ))
                att = ATTRTYPEID_TIME;
            if( iv.isCaseSensitive() )
                att = ATTRTYPEID_STR;
            
            return att;
        }
        
        
        void
        EAIndexerODBC::AddSQLOpHeur( fh_stringstream& sqlss,
                                     const std::string& eaname,
                                     const std::string& opcode,
                                     IndexableValue& iv,
                                     stringset_t& lookupTablesUsed )
        {
            // FIXME: Heuristic lookup for columns inlined into docmap should
            // only cast to the type of that column,

            stringmap_t::const_iterator eci
                = m_ExtraColumnsToInlineInDocmap.find( eaname );

            if( eci != m_ExtraColumnsToInlineInDocmap.end() )
            {
                string colname = ( eci->first );
                string coltype = eci->second;
                AttrType_t att = SQLColumnTypeToAttrType( coltype, iv );
                AddSQLOp( sqlss, eaname, opcode, iv, att, lookupTablesUsed );
                return;
            }
            
            AttrTypeList_t atl = getAllAttrTypes();
            
            bool v = true;

            sqlss << "d.docid in (" << nl
                  << " SELECT distinct(docattrs.docid) as docid FROM docattrs "
                  << " WHERE  " << nl;

            for( AttrTypeList_t::const_iterator attrTypeIter = atl.begin();
                 attrTypeIter!=atl.end(); ++attrTypeIter )
            {
                if( *attrTypeIter == ATTRTYPEID_STR )
                    continue;
                
                if( v ) v = false;
                else    sqlss << " " << nl
                              << " OR d.docid in " << nl
                              << " ";
                AddSQLOp( sqlss, eaname, opcode, iv, *attrTypeIter, lookupTablesUsed );
            }
            sqlss << ")" << endl;
        }

        
        docNumSet_t&
        EAIndexerODBC::BuildQuery( fh_context q,
                                   docNumSet_t& output,
                                   fh_eaquery qobj,
                                   fh_stringstream& sqlss,
                                   stringset_t& lookupTablesUsed,
                                   bool& queryHasTimeRestriction,
                                   stringset_t& eanamesUsed,
                                   MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
            LG_EAIDX_D << "BuildQuery() token:" << tokenfc << endl;
            LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "BuildQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            if( tokenfc == "!" )
            {
                sqlss << "d.docid in" << nl
                      << "( " << nl
                      << "  SELECT distinct(docattrs.docid) as docid FROM docattrs "
                      << "  WHERE docid not in ";
                sqlss << "  (" << nl
                      << "    SELECT distinct(docattrs.docid) as docid FROM docattrs "
                      << "    WHERE  " << nl
                      << "     " << nl;
                BuildQuery( lc, output, qobj, sqlss,
                            lookupTablesUsed, queryHasTimeRestriction,
                            eanamesUsed, termInfo );
                sqlss << " )) " << nl;
                return output;
            }

            getline( orderedtls, s );
            LG_EAIDX_D << "EAQuery_Heur::BuildQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );

            if( tokenfc == "&" )
            {
                LG_EAIDX_D << " operator &, child count:" << q->SubContextCount() << endl;

                bool v = true;
                sqlss << " (" << nl;
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    if( v ) v = false;
                    else    sqlss << " and " << nl;

//                    sqlss << " SELECT d.docid FROM docmap WHERE ";
                    BuildQuery( *ci, output, qobj, sqlss,
                                lookupTablesUsed, queryHasTimeRestriction,
                                eanamesUsed, termInfo );
                }
                sqlss << " ) " << nl;
            }
            else if( tokenfc == "|" )
            {
                bool v = true;
                sqlss << " (" << nl;
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    if( v ) v = false;
                    else    sqlss << " or " << nl;

//                    sqlss << " SELECT d.docid FROM docmap WHERE ";
                    BuildQuery( *ci, output, qobj, sqlss,
                                lookupTablesUsed, queryHasTimeRestriction,
                                eanamesUsed, termInfo );
                }
                sqlss << " ) " << nl;
            }

            string eaname = getStrAttr( lc, "token", "" );
            eanamesUsed.insert( eaname );
            IndexableValue iv = getIndexableValueFromToken( eaname, rc );
            string value = asString( iv );
//            string comparisonOperator = iv.getComparisonOperator();
            string xLookupTableName = "strlookup";
            AttrType_t attrTypeID = inferAttrTypeID( iv );

            xLookupTableName = getTableName( attrTypeID );

            termInfo.insert(
                make_pair( eaname,
                           MetaEAIndexerInterface::QueryTermInfo( attrTypeID,
                                                                  xLookupTableName )));
            
            if( starts_with( eaname, "atime" )
                || starts_with( eaname, "ferris-current-time" ) )
            {
                queryHasTimeRestriction = true;
            }
            if( starts_with( eaname, "multiversion-mtime" )
                || starts_with( eaname, "multiversion-atime" ) 
                )
            {
                queryHasTimeRestriction = true;
                eaname = eaname.substr( strlen( "multiversion-" ));
            }
            

            
//             string value  = getStrAttr( rc, "token", "" );
//             string comparisonOperator = guessComparisonOperatorFromData( value );
//             string xLookupTableName = "strlookup";
//             string valueQuote = "\'";
//             AttrType_t attrTypeID = getAttrMap()->getAttrTypeID( eaname );

//             if( attrTypeID == ATTRTYPEID_TIME )
//             {
//                 valueQuote="";
//                 xLookupTableName = "timelookup";
//             }
//             else if( comparisonOperator == "int" )
//             {
//                 valueQuote="";
//                 xLookupTableName = "intlookup";
//             }
//             else if( comparisonOperator == "double" )
//             {
//                 valueQuote="";
//                 xLookupTableName = "doublelookup";
//             }
//             else if( comparisonOperator == "cis" )
//             {
//                 xLookupTableName = "strlookupnocase";
//                 value = tolowerstr()( value );
//             }
//             if( starts_with( eaname, "atime" )
//                 || starts_with( eaname, "ferris-current-time" ) )
//             {
//                 queryHasTimeRestriction = true;
//             }
            
//             static const char* allLookupTables[] =
//                 { "intlookup", "doublelookup", "timelookup", "strlookup", 0 };
//             static const char* numericLookupTables[] =
//                 { "intlookup", "doublelookup", "timelookup", 0 };
//             static const char* stringLookupTables[] =
//                 { "strlookup", 0 };
            
            if( tokenfc == "==" )
            {
                lookupTablesUsed.insert( xLookupTableName );

                // FIXME: remove table name, add in AttrType from iv, and lookuptables set param
//                 AddSQLOp( sqlss, xLookupTableName, eaname,
//                           "=", valueQuote, value );
                AddSQLOp( sqlss, eaname, "=", iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "=~" )
            {
                string opcode = "like";

                if( getDBType() == "postgresql" )
                    opcode = "~";
                
//                lookupTablesUsed.insert( xLookupTableName );
//                AddSQLOp( sqlss, xLookupTableName, eaname, opcode, valueQuote, value );
                AddSQLOp( sqlss, eaname, opcode, iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == ">=" )
            {
//                 lookupTablesUsed.insert( xLookupTableName );
//                 AddSQLOp( sqlss, xLookupTableName, eaname,
//                           ">=", valueQuote, value );
                AddSQLOp( sqlss, eaname, ">=", iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "<=" )
            {
//                 lookupTablesUsed.insert( xLookupTableName );
//                 AddSQLOp( sqlss, xLookupTableName, eaname,
//                           "<=", valueQuote, value );
                AddSQLOp( sqlss, eaname, "<=", iv, iv.getAttrTypeID(), lookupTablesUsed );
            }
            else if( tokenfc == "=?=" )
            {
                AddSQLOpHeur( sqlss, eaname, "=", iv, lookupTablesUsed );
            }
            else if( tokenfc == ">?=" )
            {
                AddSQLOpHeur( sqlss, eaname, ">=", iv, lookupTablesUsed );
            }
            else if( tokenfc == "<?=" )
            {
                AddSQLOpHeur( sqlss, eaname, "<=", iv, lookupTablesUsed );
            }
        }
        
        struct ExecuteQuery_DocIDRow
        {
            int docid;
        };
        struct ExecuteQuery_BCA_DocIDRow_docid
        {
            void operator()(BoundIOs &cols, ExecuteQuery_DocIDRow &rowbuf)
                {
                    cols["docid"] == rowbuf.docid;
                }
        };
        struct ExecuteQuery_BCA_DocIDRow_urlid
        {
            void operator()(BoundIOs &cols, ExecuteQuery_DocIDRow &rowbuf)
                {
                    cols["urlid"] == rowbuf.docid;
                }
        };

        docNumSet_t&
        EAIndexerODBC::BuildQuerySQL(
            fh_context q,
            docNumSet_t& output,
            fh_eaquery qobj,
            std::stringstream& SQLHeaderSS,
            std::stringstream& SQLWherePredicatesSS,
            std::stringstream& SQLTailerSS,
            stringset_t& lookupTablesUsed,
            bool& queryHasTimeRestriction,
            string& DocIDColumn,
            stringset_t& eanamesUsed,
            MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            fh_stringstream fSQLWherePredicatesSS;
            docNumSet_t& ret = BuildQuery( q,
                                           output,
                                           qobj,
                                           fSQLWherePredicatesSS,
                                           lookupTablesUsed,
                                           queryHasTimeRestriction,
                                           eanamesUsed,
                                           termInfo );
            SQLWherePredicatesSS << tostr(fSQLWherePredicatesSS);

            if( m_multiVersion )
            {
                if( queryHasTimeRestriction )
                {
                    DocIDColumn = "urlid";

//                    SQLHeaderSS << "SELECT max(d.docidtime) as docidtime, urlid" << nl
                    SQLHeaderSS << "SELECT urlid" << nl
                          << "FROM  " << nl
                          << "  (SELECT docidtime, urlid " << nl
                          << "     FROM  docmap d " << nl
                          << "    WHERE  " << nl
                          << "           " << nl;

//                     // FIXME:
//                     SQLHeaderSS << "SELECT d.docid as docid, d.urlid as urlid, d.docidtime as docidtime" << nl
//                           << "FROM docmap d, " << nl
//                           << "  ( SELECT max(docidtime) as ddtime, urlid" << nl
//                           << "      FROM docmap" << nl
//                           << "     GROUP by urlid" << nl
//                           << "  ) dd" << nl
//                           << "WHERE d.urlid=dd.urlid" << nl
//                           << "AND d.docidtime=dd.ddtime" << nl
//                           << "AND " << nl;
                    
                    // FIXME: wulfify this version.
//                     SQLHeaderSS << "SELECT e.docid as docid from docmap e" << nl
//                           << " WHERE e.docid in " << nl
//                           << " ( " << nl
//                           << "   SELECT distinct(d.docid) from docmap d, docmap e" << nl
//                           << "    WHERE d.url=e.url" << nl
//                           << "      AND d.docidtime = " << nl
//                           << "          (select max(docidtime) from docmap where url=d.url)" << nl
//                           << " ) " << nl
//                           << "AND e.url in " << nl
//                           << " (SELECT distinct(url) from docmap d WHERE " << nl;
                }
                else
                {
                    DocIDColumn = "urlid";
//                    SQLHeaderSS << "SELECT d.docid as docid, d.urlid as urlid " << nl
                    SQLHeaderSS << "SELECT d.urlid as urlid " << nl
                          << "FROM docmap d, " << nl
                          << "  ( select max(docidtime) as ddtime, urlid" << nl
                          << "    from docmap " << nl
                          << "    group by urlid  " << nl
                          << "   ) dd " << nl
                          << "WHERE d.urlid=dd.urlid" << nl
                          << "AND d.docidtime=dd.ddtime" << nl
                          << "AND "
                          << nl;
                    

                    
//                     SQLHeaderSS << "SELECT distinct(d.docid) as docid FROM docmap d" << nl
//                           << " WHERE d.docid in "                                          << nl
//                           << "   ( "                                                       << nl
//                           << "    SELECT distinct(d.docid) from docmap d, docmap e "       << nl
//                           << "    WHERE d.url=e.url "                                      << nl
//                           << "    AND d.docidtime = "                                      << nl
//                           << "        (select max(docidtime) from docmap where url=d.url)" << nl
//                           << "   ) "                                                       << nl
//                           << " and d.docid in ";
                    
                }
            }
            else
            {
                SQLHeaderSS << "SELECT distinct(d.docid) as docid FROM docmap d " << nl
                      << " WHERE " << nl;
            }

            if( m_multiVersion )
            {
                if( queryHasTimeRestriction )
                {
                    SQLTailerSS << "  ) d" << nl
                          << " GROUP by  urlid";
                }
            }
            
            return ret;
        }
                
        docNumSet_t&
        EAIndexerODBC::ExecuteQuery( fh_context q,
                                     docNumSet_t& output,
                                     fh_eaquery qobj,
                                     int limit )
        {
            stringset_t eanamesUsed;
            stringset_t lookupTablesUsed;
            bool queryHasTimeRestriction = false;
            std::stringstream HeaderSS;
            std::stringstream whereclauseSS;
            std::stringstream TailerSS;
            string DocIDColumn = "docid";
            MetaEAIndexerInterface::BuildQuerySQLTermInfo_t termInfo;
            BuildQuerySQL( q, output, qobj,
                           HeaderSS,
                           whereclauseSS,
                           TailerSS,
                           lookupTablesUsed,
                           queryHasTimeRestriction,
                           DocIDColumn,
                           eanamesUsed,
                           termInfo );

            fh_stringstream sqlss;
            sqlss << HeaderSS.str() << endl;
            sqlss << whereclauseSS.str() << endl;
            sqlss << TailerSS.str() << endl;
            if( limit )
            {
                sqlss << " limit " << limit << endl;
            }
            
            
//             BuildQuery( q, output, qobj,
//                         whereclauseSS,
//                         lookupTablesUsed,
//                         queryHasTimeRestriction );
//             string DocIDColumn = "docid";

// //            LG_EAIDX_D << "SQL WHERE :" << tostr(whereclauseSS) << endl << endl;

//             fh_stringstream sqlss;
//             if( m_multiVersion )
//             {
//                 if( queryHasTimeRestriction )
//                 {
//                     DocIDColumn = "urlid";

// //                    sqlss << "SELECT max(d.docidtime) as docidtime, urlid" << nl
//                     sqlss << "SELECT urlid" << nl
//                           << "FROM  " << nl
//                           << "  (SELECT docidtime, urlid " << nl
//                           << "     FROM  docmap d " << nl
//                           << "    WHERE  " << nl
//                           << "           " << nl;

// //                     // FIXME:
// //                     sqlss << "SELECT d.docid as docid, d.urlid as urlid, d.docidtime as docidtime" << nl
// //                           << "FROM docmap d, " << nl
// //                           << "  ( SELECT max(docidtime) as ddtime, urlid" << nl
// //                           << "      FROM docmap" << nl
// //                           << "     GROUP by urlid" << nl
// //                           << "  ) dd" << nl
// //                           << "WHERE d.urlid=dd.urlid" << nl
// //                           << "AND d.docidtime=dd.ddtime" << nl
// //                           << "AND " << nl;
                    
//                     // FIXME: wulfify this version.
// //                     sqlss << "SELECT e.docid as docid from docmap e" << nl
// //                           << " WHERE e.docid in " << nl
// //                           << " ( " << nl
// //                           << "   SELECT distinct(d.docid) from docmap d, docmap e" << nl
// //                           << "    WHERE d.url=e.url" << nl
// //                           << "      AND d.docidtime = " << nl
// //                           << "          (select max(docidtime) from docmap where url=d.url)" << nl
// //                           << " ) " << nl
// //                           << "AND e.url in " << nl
// //                           << " (SELECT distinct(url) from docmap d WHERE " << nl;
//                 }
//                 else
//                 {
//                     DocIDColumn = "urlid";
// //                    sqlss << "SELECT d.docid as docid, d.urlid as urlid " << nl
//                     sqlss << "SELECT d.urlid as urlid " << nl
//                           << "FROM docmap d, " << nl
//                           << "  ( select max(docidtime) as ddtime, urlid" << nl
//                           << "    from docmap " << nl
//                           << "    group by urlid  " << nl
//                           << "   ) dd " << nl
//                           << "WHERE d.urlid=dd.urlid" << nl
//                           << "AND d.docidtime=dd.ddtime" << nl
//                           << "AND "
//                           << nl;
                    

                    
// //                     sqlss << "SELECT distinct(d.docid) as docid FROM docmap d" << nl
// //                           << " WHERE d.docid in "                                          << nl
// //                           << "   ( "                                                       << nl
// //                           << "    SELECT distinct(d.docid) from docmap d, docmap e "       << nl
// //                           << "    WHERE d.url=e.url "                                      << nl
// //                           << "    AND d.docidtime = "                                      << nl
// //                           << "        (select max(docidtime) from docmap where url=d.url)" << nl
// //                           << "   ) "                                                       << nl
// //                           << " and d.docid in ";
                    
//                 }
//             }
//             else
//             {
//                 sqlss << "SELECT distinct(d.docid) as docid FROM docmap d " << nl
//                       << " WHERE " << nl;
//             }
            
//             sqlss << tostr( whereclauseSS ) << endl;
//             if( m_multiVersion )
//             {
//                 if( queryHasTimeRestriction )
//                 {
//                     sqlss << "  ) d" << nl
//                           << " GROUP by  urlid";

// //                    sqlss << " ) ";
//                 }
//             }
            LG_EAIDX_D << "DSN:" << getConnection().GetDSN() << endl;
            LG_EAIDX_D << "SQL :" << nl << tostr(sqlss) << endl << endl;

            if( DocIDColumn == "docid" )
            {
                vector<ExecuteQuery_DocIDRow> results;
                DBView<ExecuteQuery_DocIDRow> view( DBView<ExecuteQuery_DocIDRow>::Args()
                                                    .tables( tostr(sqlss) )
                                                    .bca( ExecuteQuery_BCA_DocIDRow_docid() )
                                                    .conn( getConnection() ));

                DBView<ExecuteQuery_DocIDRow>::sql_iterator read_it = view.begin();
                for ( ; read_it != view.end();  ++read_it)
                {
                    addDocID( output, read_it->docid );
                }
            }
            else
            {
                vector<ExecuteQuery_DocIDRow> results;
                DBView<ExecuteQuery_DocIDRow> view( DBView<ExecuteQuery_DocIDRow>::Args()
                                                    .tables( tostr(sqlss) )
                                                    .bca( ExecuteQuery_BCA_DocIDRow_urlid() )
                                                    .conn( getConnection() ));

                DBView<ExecuteQuery_DocIDRow>::sql_iterator read_it = view.begin();
                for ( ; read_it != view.end();  ++read_it)
                {
                    addDocID( output, read_it->docid );
                }
            }


            
//             DynamicDBView<> view( DynamicDBView<>::Args()
//                                   .tables( tostr(sqlss) )
//                                   .conn( getConnection() ));

//             for( DynamicDBView<>::sql_iterator print_it = view.begin();
//                  print_it != view.end(); print_it++)
//             {
//                 variant_row row = *print_it;
//                 string t = row[ DocIDColumn ];
//                 output.insert( toint( t ) );
//             }

            
            return output;
        }
        
        std::string
        EAIndexerODBC::resolveDocumentID( docid_t id )
        {
            return ODBCResolveDocumentID( id, m_multiVersion );
        }

        fh_AttrMap
        EAIndexerODBC::getAttrMap()
        {
            if( !isBound( m_attrmap ) )
            {
                m_attrmap = new AttrMap( getConnection(), getFetchMode() );
            }
            return m_attrmap;
        }
        
        long
        EAIndexerODBC::ensureValueMap( const IndexableValue& iv )
        {
            AttrType_t     att = iv.getAttrTypeID();
            const string&  v   = iv.rawValueString();
            XSDBasic_t     sct = iv.getSchemaType();
            
            LG_EAIDX_D << "EAIndexerODBC::ensureValueMap() att:" << att
                       << " v:" << v
                       << " sct:" << sct
                       << " is epoch:" << ( sct == FXD_UNIXEPOCH_T )
                       << endl;
            
            if( sct == FXD_UNIXEPOCH_T || att == ATTRTYPEID_TIME )
            {
                getValueMapTime()->ensure( v );
            }
            else if( att == ATTRTYPEID_INT )
            {
                getValueMapInt()->ensure( v );
            }
            else if( att == ATTRTYPEID_DBL )
            {
                getValueMapDouble()->ensure( v );
            }
            else
            {
                if( !v.empty() )
                {
                    getValueMapString()->ensure( v );
                    getValueMapCIS()->ensure(  tolowerstr()( v ) );
                }
            }
        }
        
        fh_DocAttrs
        EAIndexerODBC::getDocAttrs()
        {
            if( !isBound( m_docattrs ) )
            {
                m_docattrs = new DocAttrs( getConnection() );
            }
            return m_docattrs;
        }
        
        fh_ValueMapString
        EAIndexerODBC::getValueMapString()
        {
            if( !isBound( vm_string ) )
            {
                vm_string = new ValueMapString_t(
                    getConnection(), "strlookup", 0, getFetchMode() );
            }
            return vm_string;
        }
        
        fh_ValueMapString
        EAIndexerODBC::getValueMapCIS()
        {
            if( !isBound( vm_stringnocase ) )
            {
                vm_stringnocase = new ValueMapString_t(
                    getConnection(), "strlookupnocase", 0, getFetchMode() );
            }
            return vm_stringnocase;
        }
        
        fh_ValueMapInt
        EAIndexerODBC::getValueMapInt()
        {
            if( !isBound( vm_int ) )
            {
                vm_int = new ValueMapInt_t( getConnection(), "intlookup", 0,
                                            getFetchMode() );
            }
            return vm_int;
        }
        
        fh_ValueMapDouble
        EAIndexerODBC::getValueMapDouble()
        {
            if( !isBound( vm_double ) )
            {
                vm_double = new ValueMapDouble_t( getConnection(), "doublelookup", 0,
                                                  getFetchMode() );
            }
            return vm_double;
        }
        
        fh_ValueMapTime
        EAIndexerODBC::getValueMapTime()
        {
            if( !isBound( vm_time ) )
            {
                vm_time = new ValueMapTime_t(
                    getConnection(), "timelookup", 0, getFetchMode() );
            }
            return vm_time;
        }
        
        
        
    };
};



extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerODBC();
    }
};
