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

    $Id: libferrisqtsql.cpp,v 1.10 2007/05/24 21:30:08 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisBoost.hh>
#include <iostream>
#include <fstream>

using namespace std;

#define DEBUG LG_RECORDFILE(Ferris::Timber::_SBufT::PRI_DEBUG)

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN tupleContext
        :
        public StateLessEAHolder< tupleContext, leafContext >
    {
        typedef tupleContext                            _Self;
        typedef StateLessEAHolder< _Self, leafContext > _Base;

        stringmap_t EA;
        
    public:

        std::string
        priv_getRecommendedEA()
            {
                static string ret;
                if( ret.empty() )
                {
                    stringstream ss;
                    ss << "name,";
                    for( stringmap_t::const_iterator iter = EA.begin(); iter != EA.end(); ++iter )
                    {
                        string k = iter->first;
                        ss << k << ",";
                    }
                    ret = ss.str();
                }
                DEBUG << "tupleContext::rea ret:" << ret << endl;
                return ret;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ss;
                return ss;
            }

        static fh_iostream SL_getValueStream( tupleContext* c, const std::string& rdn, EA_Atom* atom )
            {
                LG_SQLDB_D << "SL_getValueStream rdn:" << rdn << endl;
                fh_stringstream ss;
                ss << c->EA[rdn];
                return ss;
            }
        
        tupleContext( Context* parent, const std::string& rdn, const stringmap_t& EA )
            :
            _Base( parent, rdn ),
            EA( EA )
            {
                DEBUG << "tupleContext() rdn:" << rdn << endl;

                string className = parent->getURL();
                bool force = AttributeCollection::isStateLessEAVirgin( className );
                DEBUG << "tupleContext() force:" << force << " rdn:" << rdn << endl;
                setup_DynamicClassedStateLessEAHolder( className );

                if( force )
                {
#define SLEA tryAddStateLessAttribute

                    for( stringmap_t::const_iterator iter = EA.begin(); iter != EA.end(); ++iter )
                    {
                        string k = iter->first;
                        SLEA( k, &_Self::SL_getValueStream, XSD_BASIC_STRING );
                    }
#undef SLEA
                    createStateLessAttributes( true );
                }
            }
    };
    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class recordfileContext;
    class recordfileContextRecordParser;
    FERRIS_SMARTPTR( recordfileContextRecordParser, fh_recordfileContextRecordParser );
    
    class FERRISEXP_CTXPLUGIN recordfileContextRecordParser
        :
        public Handlable
    {
    protected:
        int fieldNumber;
        stringvec_t fieldNames;
        stringmap_t EA;
        
        string getFieldName();
        void handleField( const std::string& f );
        
    public:
        recordfileContextRecordParser( recordfileContext* c );
        virtual void parseRecord( recordfileContext* parent, const std::string& record ) = 0;
        fh_context getMetadataContext( recordfileContext* c );

        stringmap_t& getEA()
            {
                return EA;
            }
    };

    class FERRISEXP_CTXPLUGIN recordfileContextRecordParserAWK
        :
        public recordfileContextRecordParser
    {
        string fs;
        boost::regex fs_rex;
    public:
        recordfileContextRecordParserAWK( recordfileContext* parent );
        virtual void parseRecord( recordfileContext* parent, const std::string& record );
    };

    class FERRISEXP_CTXPLUGIN recordfileContextRecordParserFSplit
        :
        public recordfileContextRecordParser
    {
        string fsplit;
        boost::regex fsplit_rex;
    public:
        recordfileContextRecordParserFSplit( recordfileContext* parent );
        virtual void parseRecord( recordfileContext* parent, const std::string& record );
    };
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN recordfileContext
        :
        public StateLessEAHolding_Recommending_ParentPointingTree_Context< recordfileContext, FakeInternalContext >
    {
        typedef recordfileContext                               _Self;
        typedef StateLessEAHolding_Recommending_ParentPointingTree_Context< _Self, FakeInternalContext > _Base;

        bool m_haveTriedToRead;
        fh_recordfileContextRecordParser m_recordParser;
        string m_rs;
        boost::regex rs_rex;
        stringmap_t m_timeparsers;
        string m_grep_filter;
        boost::regex rs_grep_filter;
        
    protected:

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                return getCoveredContext()->priv_getIStream( m );
            }

        void
        priv_read()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;
                    DEBUG << "recordfileContext::read() earl:" << getURL() << endl;

                    m_grep_filter  = getStrAttr( getMetadataContext(), "ferris-recordfile-grep", ".*" );
                    rs_grep_filter = toregex( m_grep_filter );
                    
                    m_rs   = getStrAttr( getMetadataContext(), "ferris-recordfile-rs", "\n" );
                    rs_rex = toregex( m_rs );
            
                    string fsplit = getStrAttr( getMetadataContext(), "ferris-recordfile-fsplit", "" );
                    if( !fsplit.empty() )
                    {
                        m_recordParser = new recordfileContextRecordParserFSplit( this );
                    }
                    else
                    {
                        m_recordParser = new recordfileContextRecordParserAWK( this );
                    }

                    Util::ParseKeyValueString( m_timeparsers,
                                               getStrAttr( getMetadataContext(),
                                                           "ferris-recordfile-field-timeparsers", "" ),
                                               "|" );
                    
                    bool rc = 0;
                    string data = getStrAttr( this, "content", "", true, true );
                    string::iterator iter = data.begin();
                    string::iterator    e = data.end();
                    
                    DEBUG << "data.sz:" << data.size() << endl;
//                    DEBUG << "data:" << data << endl;
                    DEBUG << "m_timeparsers.sz:" << m_timeparsers.size() << endl;

                    string attributeThatNamesFile = getStrAttr(
                        getMetadataContext(), "ferris-recordfile-attribute-that-names-file", "name" );

                    boost::match_results<std::string::iterator> m; 
                    boost::match_flag_type rs_match_flag = boost::match_default;

                    for( int RN = 0; ::boost::regex_search( iter, e, m, rs_rex, rs_match_flag ); ++RN )
                    {
                        string record( iter, m[0].first );
                        iter =  m[0].second;

                        DEBUG << "have record:" << record << endl;
//                        cerr << "RN:" << RN << " sz:" << record.size()
//                             << " record:" << record
//                             << endl;

                        if( !regex_search( record, rs_grep_filter ) )
                        {
                            DEBUG << "grep filter does not match record, skipping it..." << endl;
                            continue;
                        }
                        
                        m_recordParser->getEA().clear();
                        m_recordParser->parseRecord( this, record );

                        string rdn = tostr(RN);
                        stringmap_t& EA = m_recordParser->getEA();

                        if( attributeThatNamesFile != "name" )
                        {
                            stringmap_t::iterator iter = EA.find( attributeThatNamesFile );
                            if( iter != EA.end() )
                            {
                                rdn = iter->second;
                            }
                        }
                        
                        // add EA-epoch augmentation based on the timeparsers for each field
                        for( stringmap_t::iterator iter = m_timeparsers.begin();
                             iter != m_timeparsers.end(); ++iter )
                        {
                            DEBUG << "looking up ea epoch:" << iter->first << endl;
                            
                            stringmap_t::const_iterator EAiter = EA.find( iter->first );
                            if( EAiter != EA.end() )
                            {
                                string v = EAiter->second;
                                string timefmt = iter->second;
                                bool autoFresh = false;

                                struct tm tm = Time::ParseTimeString( v, timefmt, autoFresh );
                                time_t tt = mktime( &tm );

                                DEBUG << "adding date epoch converted time for EA:" << EAiter->first
                                      << " v:" << v << endl
                                      << " timefmt:" << timefmt << " epoch:" << tt << endl;
                                DEBUG << "epoch-as-string:" << Time::toTimeString( tt, "%y %b %e %H:%M:%S" ) << endl;

                                string rdn = EAiter->first + "-epoch";
                                string epochv = tostr(tt);
                                EA[ rdn ] = epochv;
                            }
                        }

                        tupleContext* c = new tupleContext( this, rdn, EA );
                        fh_context subc = c;
                        Insert( c );

                        if( !RN )
                        {
                            for( stringmap_t::const_iterator iter = EA.begin(); iter != EA.end(); ++iter )
                            {
                                DEBUG << "recomending ea:" << iter->first << endl;
                                appendToRecommentedEA( iter->first );
                            }
                        }
                        
                    }
                }
            }

    public:

        
        recordfileContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false ),
            m_recordParser( 0 )
            {
                createStateLessAttributes( true );
                DEBUG << "recordfileContext() rdn:" << rdn << endl;
            }

        fh_context getMetadataContext()
            {
                string earl = getStrAttr( this, "ferris-recordfile-metadata-url", "" );
                if( earl.empty() )
                    return this;

                DEBUG << "getMetadataContext(1) earl:" << earl << endl;
                
                if( string::npos == earl.find(":")
                    && !starts_with( earl, "/") && !starts_with( earl, "~") )
                {
                    earl = "~/.ferris/recordfile-types.db/" + earl;
                }
                DEBUG << "getMetadataContext(2) earl:" << earl << endl;
                
                fh_context c = Resolve( earl );
                try
                {
                    c->read();
                }
                catch( ... )
                {
                }
                return GetImpl(c);
            }
        
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        _Self* priv_CreateContext( Context* parent, string rdn )
            {
                _Self* ret = new _Self();
                ret->setContext( parent, rdn );
                return ret;
            }
        
    };
        
    
    /*******************************************************************************/

    recordfileContextRecordParser::recordfileContextRecordParser( recordfileContext* c )
        :
        fieldNumber(0)
    {
        Util::parseSeperatedList( getStrAttr( getMetadataContext(c), "ferris-recordfile-field-names", "" ),
                                  fieldNames, back_inserter(fieldNames) );

        DEBUG << "getMetadataContext(c):" << getMetadataContext(c)->getURL() << endl;
        DEBUG << "fieldNames.sz:" << fieldNames.size() << endl;
        
    }

    fh_context
    recordfileContextRecordParser::getMetadataContext( recordfileContext* c )
    {
        return c->getMetadataContext();
    }
    
    
    string
    recordfileContextRecordParser::getFieldName()
    {
        if( fieldNames.size() > fieldNumber )
            return fieldNames[ fieldNumber ];

        stringstream ss;
        ss << "field" << fieldNumber;
        return ss.str();
    }
    void
    recordfileContextRecordParser::handleField( const std::string& f )
    {
        DEBUG << "handleField() fnum:" << fieldNumber << " fname:" << getFieldName() << " field:" << f << endl;
        EA[ getFieldName() ] = f;
        ++fieldNumber;
    }

    //////////
    
    recordfileContextRecordParserAWK::recordfileContextRecordParserAWK( recordfileContext* c )
        :
        recordfileContextRecordParser( c )
    {
        fs = getStrAttr( getMetadataContext(c), "ferris-recordfile-fs", "[ \t]" );
        fs_rex = toregex( fs );
    }
    
    void
    recordfileContextRecordParserAWK::parseRecord( recordfileContext* parent, const std::string& record )
    {
            fieldNumber = 0;
            // in awk
            // $0 the records
            // $1 the first field
            boost::match_flag_type fs_match_flag = boost::match_default;
            string::const_iterator iter = record.begin();
            string::const_iterator    e = record.end();
            boost::match_results<std::string::const_iterator> m; 
            while( ::boost::regex_search( iter, e, m, fs_rex, fs_match_flag ) )
            {
                string field( iter, m[0].first );
                iter = m[0].second;
                handleField( field );
            }
            if( iter != e )
            {
                string field( iter, e );
                handleField( field );
            }
        
    }

    ///////////
    
    recordfileContextRecordParserFSplit::recordfileContextRecordParserFSplit( recordfileContext* c )
        :
        recordfileContextRecordParser( c )
    {
        fsplit = getStrAttr( getMetadataContext(c), "ferris-recordfile-fsplit", "" );
        fsplit_rex = toregex( fsplit );
    }
    
    void
    recordfileContextRecordParserFSplit::parseRecord( recordfileContext* parent, const std::string& record )
    {
        DEBUG << "FSPLIT:" << fsplit << endl;
            
        fieldNumber = 0;
        boost::match_flag_type fs_match_flag = boost::match_default;
        string::const_iterator iter = record.begin();
        string::const_iterator    e = record.end();
        boost::match_results<std::string::const_iterator> m; 
        if( ::boost::regex_match( iter, e, m, fsplit_rex, fs_match_flag ) )
        {
            DEBUG << "found matches:" << m.size() << endl;
            for( int i=1; i<m.size(); ++i )
            {
                string field( m[i].first, m[i].second );
                handleField( field );
            }
        }
    }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew()" << endl;
                static recordfileContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
