/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
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

    $Id: xslt_fs.cpp,v 1.2 2005/12/20 11:41:33 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "xslt_base.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/Shell.hh>
#include <Ferris/FerrisDOM.hh>

#include <uuid/uuid.h>

using namespace Ferris;
using namespace std;

namespace FerrisXSLT 
{
    
    /**
     * Return a new uuid
     */
    class GenerateUUIDFunction
        :
        public FerrisXSLTFunctionBase< GenerateUUIDFunction >
    {
    public:

        /**
         * Execute an XPath function object.  The function must return a valid
         * XObject.
         *
         * @param executionContext executing context
         * @param context          current context node
         * @param opPos            current op position
         * @param args             vector of pointers to XObject arguments
         * @return                 pointer to the result XObject
         */
        virtual XObjectPtr execute( XPathExecutionContext& executionContext,
                                    XalanNode* context,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator* locator ) const
            {
                XObjectFactory& xfac = executionContext.getXObjectFactory();
                try
                {
                    uuid_t uu;
                    char out[500];
                    
                    uuid_generate( uu );
                    uuid_unparse(  uu, out );
                    string id = out;
                    
                    cerr << "generate-uuid id:" << id << endl;
                    
                    return xfac.createString( domstr( id ));
                }
                catch( exception& e )
                {
                    executionContext.error( domstr(e.what()), context);
                }
            }

    protected:
        virtual const XalanDOMString&
        getError(XalanDOMString& theBuffer) const
            {
                theBuffer = domstr( "generate-uuid requires no args" ); return theBuffer;
            }
        virtual Function*
        clone(MemoryManagerType&  theManager) const
            {
                return new GenerateUUIDFunction();
            }
    };

    // Register the function. 
    namespace { static bool v1 = registerXSLTFunction( "generate-uuid", GenerateUUIDFunction() ); };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    
};

