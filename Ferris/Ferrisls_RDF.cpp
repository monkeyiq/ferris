/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferrisls client helper code.

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

    $Id: Ferrisls_RDF.cpp,v 1.3 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <Ferrisls.hh>
#include <FilteredContext.hh>
#include <Context.hh>
#include <iomanip>
#include <ContextSetCompare_private.hh>
#include "Trimming.hh"
#include "ValueRestorer.hh"
#include "FerrisSemantic.hh"
#include <FerrisBoost.hh>

using namespace std;


namespace Ferris
{
    using namespace RDFCore;



struct Ferrisls_rdf_display_private
{
    fh_model model;
    Ferrisls_rdf_display_private()
        :
        model( 0 )
        {
            model = Model::MemoryOnlyModel();
        }
};
    

    void
    Ferrisls_rdf_display::PrintEA( fh_context ctx,
                                   int i,
                                   const std::string& attr,
                                   const std::string& EA )
    {
        string earl = ctx->getURL();

        fh_node subj = Node::CreateURI( earl );
        fh_node pred = Node::CreateURI( Semantic::getPredicateURI(attr) );
        fh_node obj  = Node::CreateLiteral( EA );

        P->model->insert( subj, pred, obj );
    }

    void
    Ferrisls_rdf_display::ShowAttributes( fh_context ctx )
    {
        _Base::ShowAttributes( ctx );
    }

    void
    Ferrisls_rdf_display::workStarting()
    {
    }

    void
    Ferrisls_rdf_display::workComplete()
    {
        P->model->write( Factory::fcout() );
    }

    void
    Ferrisls_rdf_display::EnteringContext(fh_context ctx)
    {
    }

    void
    Ferrisls_rdf_display::LeavingContext(fh_context ctx)
    {
    }

    Ferrisls_rdf_display::Ferrisls_rdf_display()
        :
        P( new Ferrisls_rdf_display_private() )
    {
    }
    
    Ferrisls_rdf_display::~Ferrisls_rdf_display()
    {
        if( P )
            delete P;
    }

};
