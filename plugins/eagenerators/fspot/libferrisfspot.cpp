/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

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

    $Id: libferrisfspot.cpp,v 1.2 2009/10/08 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>
#include <sqlite3.h>

using namespace std;

namespace Ferris
{
    struct FSpotDirInfo : public CacheHandlable
    {
        stringset_t allTagsForDir;
        typedef map< string, stringset_t > fileTags_t;
        fileTags_t fileTags;

        stringset_t& getFileTags( string rdn )
            {
                LG_FSPOT_D << "getFileTags(top) rdn:" << rdn  << endl;
                LG_FSPOT_D << "fileTags.sz:" << fileTags.size() << endl;
                
                
                fileTags_t::iterator iter = fileTags.find( rdn );
                if( iter != fileTags.end() )
                {
                    LG_FSPOT_D << "getFileTags() rdn:" << rdn << " tags.sz:" << iter->second.size() << endl;
                    return iter->second;
                }
                static stringset_t empty;
                return empty;
            }

        stringset_t& getFileTagsNegative( string rdn, stringset_t& ret )
            {
                set_difference( allTagsForDir.begin(), allTagsForDir.end(),
                                getFileTags( rdn ).begin(), getFileTags( rdn ).end(),
                                inserter( ret, ret.end() ));
                return ret;
            }
        

        void insertTag( string& rdn, string& tag )
            {
                fileTags[ rdn ].insert( tag );
                allTagsForDir.insert( tag );
            }
    };
    FERRIS_SMARTPTR( FSpotDirInfo, fh_FSpotDirInfo );
    
    static Cache< Context*, fh_FSpotDirInfo >& getFSpotDirInfoCacheMap()
    {
        static Cache< Context*, fh_FSpotDirInfo > ret;
        static bool v = true;
        if( v )
        {
            v = false;
            ret.setMaxCollectableSize( 20 );
            ret.setTimerInterval( 12000 );
        }
        return ret;
    }
    
    
    class FERRISEXP_DLLLOCAL EAGenerator_Fspot : public MatchedEAGeneratorFactory
    {
        fh_FSpotDirInfo getDirInfo( fh_context a );

        bool haveFSpotDatabase();
        bool shouldOverlayFSpotForURL( fh_context a );
        
        
    protected:

        virtual void Brew( const fh_context& a );

    public:

        EAGenerator_Fspot();
        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef class EA_Atom_Static FspotByteArrayAttribute;
    typedef class EA_Atom_Static FspotByteArrayAttributeSchema;
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    sqlite3* getSQLiteConnection()
    {
        static bool haveWarned = false;
        static sqlite3* ret = 0;
        if( ret )
            return ret;
        
        string path = Shell::getHomeDirPath() + "/.gnome2/f-spot/photos.db";
        int rc = SQLITE_OK;

        if( 0 != access( path.c_str(), R_OK ))
        {
            stringstream ss;
            ss << "No f-spot database found!" << endl;
            if( !haveWarned )
            {
                haveWarned = true;
                LG_FSPOT_W << tostr(ss) << endl;
            }
            Throw_SQLiteException( tostr(ss), 0 );
        }
        
        rc = sqlite3_open( path.c_str(), &ret );
        if( rc != SQLITE_OK )
        {
            stringstream ss;
            ss << "Error opening f-spot database!" << endl;
            ss << "reason:" << sqlite3_errmsg( ret ) << endl;
            if( !haveWarned )
            {
                haveWarned = true;
                LG_FSPOT_W << tostr(ss) << endl;
            }
            Throw_SQLiteException( tostr(ss), 0 );
        }

        return ret;
    }
    

    bool
    EAGenerator_Fspot::haveFSpotDatabase()
    {
        static bool alreadyFailed = false;
        bool ret = false;

        if( alreadyFailed )
        {
            return false;
        }

        try
        {
            sqlite3* con = getSQLiteConnection();

            int rc = SQLITE_OK;
            sqlite3_stmt* stmt = 0;
            const char* pzTail = 0;
            string q = "select data from meta where name = 'F-Spot Database Version';";
            rc = sqlite3_prepare( con, q.c_str(), q.length(), &stmt, &pzTail );
            while( (rc = sqlite3_step( stmt )) == SQLITE_ROW )
            {
                string vers = (const char*)sqlite3_column_text( stmt, 0 );
                if( starts_with( vers, "3" ) || starts_with( vers, "4" ) )
                {
                    ret = true;
                }
            }
            sqlite3_finalize( stmt );
            alreadyFailed = !ret;
            return ret;
        }
        catch(...)
        {
            alreadyFailed = true;
            return false;
        }
    }

    bool
    EAGenerator_Fspot::shouldOverlayFSpotForURL( fh_context a )
    {
        bool ret = false;
        string earl = a->getURL();

        static fh_rex r = 0;

        if( !r )
        {
            string regexstr = getConfigString( FDB_GENERAL,
                                               CFG_FSPOT_POSITIVE_OVERLAY_REGEX_K,
                                               CFG_FSPOT_POSITIVE_OVERLAY_REGEX_DEF );
            if( !regexstr.empty() )
            {
                r = toregexh( regexstr );
            }
        }

        if( r )
        {
            if( regex_match( earl, r ) )
            {
                ret = true;
            }
        }
        
        return ret;
    }
    
    

    fh_FSpotDirInfo
    EAGenerator_Fspot::getDirInfo( fh_context a )
    {
        // static Cache< Context*, fh_FSpotDirInfo >& getFSpotDirInfoCacheMap()
        Context* cacheKey = dynamic_cast<Context*>( GetImpl(a) );
        LG_FSPOT_D << "getDirInfo(1)" << endl;
        {
            fh_FSpotDirInfo st = getFSpotDirInfoCacheMap().get( cacheKey );
            if( st )
            {
                LG_FSPOT_D << "getDirInfo(ok) have st from cache:" << toVoid(st) << endl;
                return st;
            }
        }

        fh_FSpotDirInfo st = new FSpotDirInfo();
        string path = a->getDirPath();
        stringstream qss;
        qss << "SELECT p.name,p.id,t.name " << endl
            << "FROM photos p, photo_tags pt, tags t " << endl
            << "WHERE p.directory_path = '" << path << "'  " << endl
            << "AND p.id=pt.photo_id " << endl
            << "AND pt.tag_id = t.id;" << endl;
        LG_FSPOT_D << "Generating dir info for path:" << path
                   << " SQL:" << tostr(qss) << endl;
        
        sqlite3* con = getSQLiteConnection();
        int rc = SQLITE_OK;

        LG_FSPOT_D << "About to execute query..." << endl;
        sqlite3_stmt* stmt = 0;
        const char* pzTail = 0;
        string q = tostr(qss);

        rc = sqlite3_prepare( con, q.c_str(), q.length(), &stmt, &pzTail );

        while( (rc = sqlite3_step( stmt )) == SQLITE_ROW )
        {
            LG_FSPOT_D << "Have tuple..." << endl;
            string rdn = (const char*)sqlite3_column_text( stmt, 0 );
            string id  = (const char*)sqlite3_column_text( stmt, 1 );
            string tag = (const char*)sqlite3_column_text( stmt, 2 );

            LG_FSPOT_D << "Have tuple...rdn:" << rdn << " tag:" << tag << endl;
            st->insertTag( rdn, tag );
        }
        sqlite3_finalize( stmt );

        LG_FSPOT_D << "done with tuples...st:" << toVoid(st) << endl;
        getFSpotDirInfoCacheMap().put( cacheKey, st );
        return st;
    }
    
    

EAGenerator_Fspot::EAGenerator_Fspot()
    :
    MatchedEAGeneratorFactory()
{
}



void
EAGenerator_Fspot::Brew( const fh_context& a )
{
    if( !haveFSpotDatabase() )
        return;
    if( !shouldOverlayFSpotForURL( a ) )
        return;
    
    
//    Time::Benchmark("Fspot::Brew() " + a->getURL() );
    LG_FSPOT_D << "EAGenerator_Fspot::Brew() url:" << a->getURL() << endl;
    
    static bool brewing = false;
    if( brewing )
        return;
    Util::ValueRestorer< bool > dummy1( brewing, true );
    
    try
    {
        stringlist_t fspot_ea_names;

        LG_FSPOT_D << "EAGenerator_Fspot::Brew(2) url:" << a->getURL() << endl;
        LG_FSPOT_D << "is-native:" << a->getIsNativeContext() << endl;
        LG_FSPOT_D << "has-parent:" << a->isParentBound() << endl;

        if( a->getIsNativeContext() && a->isParentBound() )
        {
            fh_FSpotDirInfo di = getDirInfo( a->getParent() );
            LG_FSPOT_D << "EAGenerator_Fspot::Brew(3) di:" << toVoid(di) << endl;
            stringset_t& sl = di->getFileTags( a->getDirName() );

            LG_FSPOT_D << "EAGenerator_Fspot::Brew(4) sl.sz:" << sl.size() << endl;
            stringset_t::iterator si = sl.begin();
            stringset_t::iterator se = sl.end();
            for( ; si != se; ++si )
            {
                string rdn = *si;
                string qrdn = (string)"fspot:" + rdn;

                LG_FSPOT_D << "EAGenerator_Fspot::Brew(add) a:" << a->getURL()
                           << " rdn:" << rdn
                           << " qrdn:" << qrdn
                           << endl;
                
                a->addAttribute( qrdn, "1", XSD_BASIC_BOOL, false );
                fspot_ea_names.push_back( qrdn );
            }

            
            {
                stringset_t neg;
                di->getFileTagsNegative( a->getDirName(), neg );
                stringset_t::iterator si = neg.begin();
                stringset_t::iterator se = neg.end();
                for( ; si != se; ++si )
                {
                    string rdn = *si;
                    string qrdn = (string)"fspot:" + rdn;

                    LG_FSPOT_D << "EAGenerator_Fspot::Brew(add neg) a:" << a->getURL()
                               << " rdn:" << rdn
                               << " qrdn:" << qrdn
                               << endl;
                
                    a->addAttribute( qrdn, "0", XSD_BASIC_BOOL, false );
                    fspot_ea_names.push_back( qrdn );
                }
            }
            
            
            a->addAttribute( "fspot-ea-names",
                             Util::createCommaSeperatedList( fspot_ea_names ),
                             FXD_EANAMES,
                             true );
        }
    }
    catch( exception& e )
    {
        LG_RDF_ER << "Failed to load RDF EA, error:" << e.what() << endl;
    }

    LG_RDF_D << "Brew() "
             << " url:" << a->getURL()
             << " complete"
             << endl;
}

bool
EAGenerator_Fspot::tryBrew( const fh_context& ctx, const std::string& eaname )
{
    if( !haveFSpotDatabase() )
        return false;
    if( !shouldOverlayFSpotForURL( ctx ) )
        return false;
    
    Brew( ctx );
    return ctx->isAttributeBound( eaname, false );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        LG_FSPOT_D << "EAGenerator_Fspot::CreateRealFactory()" << endl;
        return new EAGenerator_Fspot();
    }
};



 
};
