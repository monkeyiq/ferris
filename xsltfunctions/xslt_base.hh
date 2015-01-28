/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
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

    $Id: xslt_base.hh,v 1.3 2010/09/24 21:31:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __FERRIS_XSLT_BASE_H__
#define __FERRIS_XSLT_BASE_H__

// Base header file.  Must be first.
#include <Include/PlatformDefinitions.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XPath/XObjectFactory.hpp>
#include <XPath/Function.hpp>

#include <string>

namespace FerrisXSLT 
{
    using namespace XERCES_CPP_NAMESPACE;
    using namespace XALAN_CPP_NAMESPACE;
     
    /*
     * Base class for exporting ferris C++ functions to the XSLT world.
     *
     * Children should parameterize this class with their classname and
     * only need to implement the execute() function.
     */
    template <class DerivedClassName>
    class FerrisXSLTFunctionBase
        :
        public Function
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
                                    XalanNode* /* context */,
                                    const XObjectArgVectorType & args, // vector<XObjectPtr>
                                    const Locator*	/* locator */) const = 0;

        /**
         * Implement clone() so Xalan can copy the square-root function into
         * its own function table.
         *
         * @return pointer to the new object
         */
        virtual Function* clone() const
            {
                return new DerivedClassName( *(dynamic_cast<const DerivedClassName*>(this)) );
            }

    private:
        // The assignment and equality operators are not implemented...
        DerivedClassName& operator=(const DerivedClassName&); 
        bool operator==(const DerivedClassName&) const;
    };


    // The namespace...
    const XalanDOMString& getNamespace();

    // Registry for XSLT function objects.
    // @return true for success (mainly for use in init of static vars)
    bool registerXSLTFunction( const std::string& name, const Function& f );

    XalanDOMString domstr( const std::string& s );
    
    
};

#endif
