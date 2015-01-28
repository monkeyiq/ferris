/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: libcreationdb4.cpp,v 1.5 2010/09/24 21:31:51 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

// #ifdef FERRIS_HAVE_DB4
// #include <db_cxx.h>
// #endif
#include <STLdb4/stldb4.hh>

namespace Ferris
{
    using namespace std;
    using namespace ::STLdb4;

    class CreationStatelessFunctorDB4
        :
        public CreationStatelessFunctor
    {
    public:
        virtual fh_context create( fh_context c, fh_context md );
    };

    extern "C"
    {
        fh_CreationStatelessFunctor
        Create()
        {
            return new CreationStatelessFunctorDB4();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorDB4::create( fh_context c, fh_context md )
    {
#ifdef FERRIS_HAVE_DB4
//        cerr << "Create db4 at c->getDirPath:" << c->getURL() << endl;
//        fh_context newc = SubCreate_file( c, md );
//        LG_FERRISCREATE_D << "Create db4 at newc->getDirPath:" << newc->getURL() << endl;

        string rdn       = getStrSubCtx( md, "name", "" );
        string earl      = c->getDirPath() + "/" + rdn;
        string dbname    = getStrSubCtx( md, "database-name", "" );
        string dbtypestr = getStrSubCtx( md, "database-type", "Btree" );
        DBTYPE dbtype    = DB_BTREE;
        if( dbtypestr == "Hash" )
        {
            dbtype = DB_HASH;
        }
        else if( dbtypestr == "Queue" )
        {
            dbtype = DB_QUEUE;
        }
        else if( dbtypestr == "Recno" )
        {
            dbtype = DB_RECNO;
        }
        u_int32_t dbflags = DB_CREATE | DB_TRUNCATE;
        mode_t mode = getModeFromMetaData( md );
        mode_t oldumask = 0;
        bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

        fh_database db = 0;
        if( ignoreUMask ) oldumask = umask( 0 );
        try
        {
            db = new Database( dbtype,
                               earl.c_str(),
                               dbname,
                               dbflags,
                               mode );
//            db[ "///ferris-metadata-format-version" ] = "1";
        }
        catch( dbException& e )
        {
            if( ignoreUMask ) umask( oldumask );
//         cerr << "e:" << e.what() << endl;
            fh_stringstream ss;
            ss << "SL_SubCreate_dir() error creating db4 file"
               << " URL:" << earl
               << " db4 error:" << e.what()
               << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }
        if( ignoreUMask ) umask( oldumask );

        fh_context newc = Resolve( earl );
        
            
//         Db db( 0, 0 );

//         if( ignoreUMask ) oldumask = umask( 0 );
//         try
//         {
// //         cerr << "doing db::open for url:" << newc->getDirPath() << endl;
// //         cerr << " newc->getDirPath().c_str():" << newc->getDirPath().c_str() << endl;
// //         cerr << " dbname.length:" << dbname.length() << endl;
// //         cerr << " dbtype:" << dbtype
// //              << " dbflags:" << dbflags
// //              << " mode:" << mode
// //              << endl;

//             db.open( newc->getDirPath().c_str(),
//                      dbname.length() ? dbname.c_str() : 0,
//                      dbtype, 
//                      dbflags,
//                      mode );
//         }
//         catch( DbException& e )
//         {
//             if( ignoreUMask ) umask( oldumask );
// //         cerr << "e:" << e.what() << endl;
//             fh_stringstream ss;
//             ss << "SL_SubCreate_dir() error creating db4 file"
//                << " URL:" << newc->getURL()
//                << " db4 error:" << e.what()
//                << endl;
//             Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
//         }
//         if( ignoreUMask ) umask( oldumask );
        
//         db.close( 0 );
        
        return newc;
#endif        

        fh_stringstream ss;
        ss << "SL_SubCreate() should not have gotten here, you have no "
           << "db4 at compile time"
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
                
    }
};
