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

    $Id: libcreationredland.cpp,v 1.2 2009/10/02 21:30:16 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <config.h>

#include <FerrisCreationPlugin.hh>

#include <Ferris/FerrisRDFCore.hh>
using namespace Ferris::RDFCore;

namespace Ferris
{
    using namespace std;

    class CreationStatelessFunctorRedland
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
            return new CreationStatelessFunctorRedland();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorRedland::create( fh_context c, fh_context md )
    {
#ifdef HAVE_REDLAND 
        LG_RDF_D << "Create redland at c->getDirPath:" << c->getURL() << endl;

        string rdn     = getStrSubCtx( md, "name",   "" );
        string format  = getStrSubCtx( md, "format", "RDF/XML" );
        string earl    = c->getDirPath() + "/" + rdn;

        mode_t mode = getModeFromMetaData( md );
        mode_t oldumask = 0;
        bool ignoreUMask = toint(getStrSubCtx( md, "ignore-umask", "0" ));

        if( ignoreUMask ) oldumask = umask( 0 );
        try
        {
            LG_RDF_D << "Make container at earl:" << earl << endl;

            fh_statement st = new Statement( Node::CreateURI( RDF_FERRIS_BASE + "/" + earl ),
                                             Node::CreateURI( RDF_FERRIS_BASE + "/mtime" ),
                                             Node::CreateLiteral( tostr( Time::getTime() )));
            if( format == "RDF/XML" )
            {
                LG_RDF_D << "Create redland(1) at c->getDirPath:" << c->getURL() << endl;
                fh_model   m = Model::MemoryOnlyModel();
                m->insert( st );

                fh_context c = Shell::acquireContext( earl, 0, false );
                fh_iostream ioss = c->getIOStream();
                m->write( ioss );
                ioss << flush;
            }
            else if( format == "Berkeley DB" )
            {
                fh_model m = Model::FromRedland( c, rdn );
                m->insert( st );
                m->sync();
                earl = c->getDirPath() + "/" + rdn + "-sp2o.db";
            }
            else
            {
                if( ignoreUMask ) umask( oldumask );
                fh_stringstream ss;
                ss << "SL_SubCreate_dir() error creating RDF file(s)"
                   << " URL:" << earl
                   << " unknown repository format requested:" << format
                   << endl;
                Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
            }
        }
        catch( exception& e )
        {
            if( ignoreUMask ) umask( oldumask );
            fh_stringstream ss;
            ss << "SL_SubCreate_dir() error creating RDF file(s)"
               << " URL:" << earl
               << " error:" << e.what()
               << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }
        if( ignoreUMask ) umask( oldumask );

        c->read( true );
        LG_RDF_D << "Create redland(end1) at c->getDirPath:" << c->getURL()
                 << " earl:" << earl
                 << endl;

        fh_context newc = 0;
        try
        {
            newc = Resolve( earl );
        }
        catch( exception& e )
        {
            LG_RDF_ER << "Create redland(endERR) at c->getDirPath:" << c->getURL()
                      << " earl:" << earl
                      << " e:" << e.what()
                      << endl;
            throw;
            
        }
        
        LG_RDF_D << "Create redland(end2) at c->getDirPath:" << c->getURL()
                 << " newc:" << newc->getURL()
                 << endl;
        return newc;
#endif        

        fh_stringstream ss;
        ss << "SL_SubCreate() should not have gotten here, you have no "
           << "redland at compile time"
           << " url:" << c->getURL()
           << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
                
    }
};
