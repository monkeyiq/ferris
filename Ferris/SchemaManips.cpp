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

    $Id: SchemaManips.cpp,v 1.3 2010/09/24 21:30:58 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <SchemaManips.hh>

namespace Ferris
{
    justification::justification( fh_context schemac )
        : one_arg_manip< fh_context >( justification::fct, schemac )
    {}
        
    void
    justification::fct( std::ios_base& ib, fh_context schemac )
    {
        std::string js = schemac
            ? getStrAttr( schemac, SCHEMA_JUSTIFICATION, "left" )
            : "left";
        
        if( js == "right" )
            ib.setf( std::ios_base::right, std::ios_base::adjustfield );
        else
            ib.setf( std::ios_base::left, std::ios_base::adjustfield );
    }

    justificationC::justificationC( fh_context& c, const std::string& eaname )
        : two_arg_manip< fh_context, std::string >( justificationC::fct, c, eaname )
    {}
    void
    justificationC::fct( std::ios_base& ib,
                         const fh_context& c,
                         const std::string& eaname )
    {
        std::string js = getSchemaJustification( c, eaname, "left" );
        
        if( js == "right" )
            ib.setf( std::ios_base::right, std::ios_base::adjustfield );
        else
            ib.setf( std::ios_base::left, std::ios_base::adjustfield );
    }
    
    width::width( fh_context schemac )
        : one_arg_manip< fh_context >( width::fct, schemac )
    {}
    void
    width::fct( std::ios_base& ib, fh_context schemac )
    {
        int wid = schemac
            ? toint(getStrAttr( schemac, SCHEMA_DEFAULT_DISPLAY_WIDTH, "0" ))
            : 0;
        
        if( wid )
        {
            ib.width( wid );
        }
    }
    widthC::widthC( fh_context c, const std::string& eaname )
        : two_arg_manip< fh_context, std::string >( widthC::fct, c, eaname )
    {}
    void
    widthC::fct( std::ios_base& ib,
                 const fh_context& c,
                 const std::string& eaname )

    {
        int wid = getSchemaDisplayWidth( c, eaname, 0 );
        
        if( wid )
        {
            ib.width( wid );
        }
    }

    precision::precision( fh_context schemac )
        : one_arg_manip< fh_context >( precision::fct, schemac )
    {}
    void
    precision::fct( std::ios_base& ib, fh_context schemac )
    {
        int wid = schemac
            ? toint(getStrAttr( schemac, SCHEMA_DEFAULT_DISPLAY_PRECISION, "0" ))
            : 0;
        std::cerr << "precision::fct() schemac:" << isBound( schemac )
                  << " wid:" << wid
                  << std::endl;
        
        
        if( wid )
        {
            ib.precision( wid );
        }
    }
    precisionC::precisionC( fh_context c, const std::string& eaname )
        : two_arg_manip< fh_context, std::string >( precisionC::fct, c, eaname )
    {}
    void
    precisionC::fct( std::ios_base& ib,
                 const fh_context& c,
                 const std::string& eaname )

    {
        int wid = getSchemaDisplayPrecision( c, eaname, 0 );
        
        if( wid )
        {
            ib.precision( wid );
        }
    }

};
