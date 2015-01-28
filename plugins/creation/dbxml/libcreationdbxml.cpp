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

    $Id: libcreationdbxml.cpp,v 1.3 2010/09/24 21:31:51 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <config.h>

#ifdef HAVE_DBXML
#include "Ferris/FerrisDOM.hh"
#include <dbxml/DbXml.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include "db.h"
#include "db_cxx.h"
using namespace DbXml;
#endif

#include <FerrisCreationPlugin.hh>

namespace Ferris
{
    using namespace std;

    class CreationStatelessFunctorDBXML
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
            return new CreationStatelessFunctorDBXML();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorDBXML::create( fh_context c, fh_context md )
    {
            int rc;

//             cerr << "DB_INIT_MPOOL:" << DB_INIT_MPOOL << endl;
//             cerr << "DB_THREAD:" << DB_THREAD << endl;
            
//             DB_ENV* envp = 0;
//             rc = db_env_create( &envp, 0 );
//             rc = envp->open( envp, "/tmp/junk", DB_INIT_MPOOL | DB_PRIVATE | DB_THREAD | DB_CREATE, DB_PRIVATE );

//            cerr << "Have C API environment!" << endl;
//             DbEnv* environment_ = new DbEnv(0);
//             cerr << "cxx env:" << toVoid( environment_ ) << endl;
//             environment_->open( "/tmp/junk", DB_INIT_MPOOL | DB_PRIVATE | DB_THREAD | DB_CREATE, DB_PRIVATE );
//             cerr << "CREATE AAAAA " << endl;

        
#ifdef HAVE_DBXML
//        cerr << "Create dbxml at c->getDirPath:" << c->getURL() << endl;

        string rdn        = getStrSubCtx( md, "name", "" );
        string earl       = c->getDirPath() + "/" + rdn;
        u_int32_t dbxmlflags = DB_XA_CREATE;
        mode_t mode = getModeFromMetaData( md );
        mode_t oldumask = 0;
        bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

        if( ignoreUMask ) oldumask = umask( 0 );
        try
        {
//            cerr << "Make container at earl:" << earl << endl;
 //            DbEnv* environment_ = new DbEnv(0);
//             environment_->open( "/tmp/junk", DB_INIT_MPOOL | DB_PRIVATE | DB_THREAD | DB_CREATE, DB_PRIVATE );

            Factory::ensureXMLPlatformInitialized();
            XQillaPlatformUtils::initialize ();
            
            XmlManager myManager;
            myManager.setDefaultContainerType(XmlContainer::WholedocContainer);
            // Create and open the container.
            XmlContainer myContainer = 
                myManager.createContainer( Ferris::CleanupURL( earl ) );
        }
        catch( XmlException& e )
        {
            if( ignoreUMask ) umask( oldumask );
            fh_stringstream ss;
            ss << "SL_SubCreate_dir() error creating dbxml file"
               << " URL:" << earl
               << " dbxml error:" << e.what()
               << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }
        if( ignoreUMask ) umask( oldumask );

        fh_context newc = Resolve( earl );
        
        return newc;
#endif        

        fh_stringstream ss;
        ss << "SL_SubCreate() should not have gotten here, you have no "
           << "dbxml at compile time"
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
                
    }
};
