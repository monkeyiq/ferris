/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: libferrissopranoeashared.hh,v 1.3 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SOPRANO_EA_SHARED_H_
#define _ALREADY_INCLUDED_FERRIS_SOPRANO_EA_SHARED_H_

#include <FerrisEAPlugin.hh>
#include <Ferris/FerrisRDFCore.hh>
#include <Ferris/FerrisSemantic.hh>

using namespace std;

namespace Ferris
{
    using namespace RDFCore;

    

    class SopranoByteArrayAttribute
        :
        public EA_Atom_ReadWrite
    {

    protected:

        /************************************************************/
        /*** methods that subclasses will override ******************/
        /************************************************************/

        /**
         * Get the RDF model that we are working with
         * default is to use getDefaultFerrisModel().
         */
        virtual RDFCore::fh_model getModel( Context* c );
        /**
         * This method allows attributes to have a URI prefix appended
         * to them automatically. You could override this and return rdn
         * to make no extra prefixing occur.
         * Warning the following use sopranoea::getPredicateURI() directly:
         * Create()
         */
        virtual std::string getPredicateURI( const std::string& rdn );
        

        /************************************************************/
        /************************************************************/
        /************************************************************/
        
        ferris_ios::openmode getSupportedOpenModes();
        void createOnDiskAttribute( const fh_context& parent, const std::string& uri );
    
    public:

        fh_iostream priv_getStream( Context* c, const std::string& rdn,
                                    fh_model m, fh_node subj, fh_node pred, EA_Atom* atom );
        virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom );
        virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss );

        SopranoByteArrayAttribute(
            const fh_context& parent,
            const string& rdn,
            const string& uri,
            bool forceCreate = false
            );
        virtual ~SopranoByteArrayAttribute();

        static SopranoByteArrayAttribute* Create( const fh_context& parent,
                                                  const string& rdn,
                                                  bool forceCreate = false );
    };
 
};

#endif
