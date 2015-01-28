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

    $Id: ContextContext.cpp,v 1.4 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <ContextContext.hh>

// required for RootContextDropper
#include <Resolver_private.hh>


using namespace std;

namespace Ferris
{
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /** INIT STUFF ******************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    ContextContext::ContextContext()
    {
        createStateLessAttributes();
    }
    

    Context*
    ContextContext::priv_CreateContext( Context* parent, string rdn )
    {
        ContextContext* ret = new ContextContext();
        ret->setContext( parent, rdn );
        return ret;
    }


class FERRISEXP_DLLLOCAL ContextVFS_RootContextDropper : public RootContextDropper
{
public:
    ContextVFS_RootContextDropper()
        {
            ImplementationDetail::appendToStaticLinkedRootContextNames("context");
            RootContextFactory::Register("context", this);
        }

    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
        {
            static ContextContext raw_obj;
            
            Context* ctx = raw_obj.CreateContext( 0, "context");

            for( RootContextFactory::Droppers_t::iterator iter = rf->getDroppers().begin();
                 iter != rf->getDroppers().end(); iter++)
            {
//                cerr << "Adding ctx:" << iter->first << endl;
                Context* child = raw_obj.CreateContext( 0, iter->first );
                ctx->Insert( child );
            }
            
            return ctx;
        }
    
    
};

static ContextVFS_RootContextDropper ___ContextVFS_static_init;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Context names are always known.
 */
void
ContextContext::priv_read()
{
    emitExistsEventForEachItemRAII _raii1( this );
}

 
};
