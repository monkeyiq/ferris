/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: SchemaManips.hh,v 1.3 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/SignalStreams.hh>

#ifndef _ALREADY_INCLUDED_FERRIS_SCHEMA_MANIPS_H_
#define _ALREADY_INCLUDED_FERRIS_SCHEMA_MANIPS_H_

namespace Ferris
{
    /**
     * The following template is based on the one_arg_manip<> code
     * from the "Standard C++ IOStreams and Locales" book.
     */
    template < class Argument1, class Argument2 >
    class two_arg_manip
    {
    public:
        typedef void (*manipFct)(std::ios_base&, const Argument1&, const Argument2& );

        two_arg_manip( manipFct pf, const Argument1& arg1, const Argument2& arg2 )
            : pf_(pf), arg1_(arg1), arg2_(arg2) 
            {}

    private:

        manipFct pf_;
        const Argument1 arg1_;
        const Argument2 arg2_;

        template < class CharT, class Traits >
        friend std::basic_istream< CharT, Traits >& operator>>
        (std::basic_istream< CharT, Traits >& is, const two_arg_manip& oam )
            {
                if( !is.good()) return is;
                (*(oam.pf_))( is, oam.arg1_, oam.arg2_ );
                return is;
            }

        template < class CharT, class Traits >
        friend std::basic_ostream< CharT, Traits >& operator<<
        (std::basic_ostream< CharT, Traits >& os, const two_arg_manip& oam )
            {
                if( !os.good()) return os;
                (*(oam.pf_))( os, oam.arg1_, oam.arg2_ );
                return os;
            }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Convert a schema justification indication into an iostream manipulator.
     * Note that this class can gracefully handle the case where schemac == 0
     */
    struct FERRISEXP_API justification : public one_arg_manip< fh_context >
    {
        explicit justification( fh_context schemac );
        static void fct( std::ios_base& ib, fh_context schemac );
    };
    struct FERRISEXP_API justificationC : public two_arg_manip< fh_context, std::string >
    {
        explicit justificationC( fh_context& c, const std::string& eaname );
        static void fct( std::ios_base& ib, 
                         const fh_context& c,
                         const std::string& eaname );
    };

    /**
     * Convert a schema SCHEMA_DEFAULT_DISPLAY_WIDTH indication into an
     * iostream manipulator which operates similarly to setw().
     * Note that this class can gracefully handle the case where schemac == 0
     */
    struct FERRISEXP_API width : public one_arg_manip< fh_context >
    {
        explicit width( fh_context schemac );
        static void fct( std::ios_base& ib, fh_context schemac );
    };
    struct FERRISEXP_API widthC : public two_arg_manip< fh_context, std::string >
    {
        explicit widthC( fh_context c, const std::string& eaname );
        static void fct( std::ios_base& ib,
                         const fh_context& c,
                         const std::string& eaname );
        

    };

    /**
     * Convert a schema SCHEMA_DEFAULT_DISPLAY_PRECISION indication into an
     * iostream manipulator which operates similarly to precision().
     * Note that this class can gracefully handle the case where schemac == 0
     */
    struct FERRISEXP_API precision : public one_arg_manip< fh_context >
    {
        explicit precision( fh_context schemac );
        static void fct( std::ios_base& ib, fh_context schemac );
    };
    struct FERRISEXP_API precisionC : public two_arg_manip< fh_context, std::string >
    {
        explicit precisionC( fh_context c, const std::string& eaname );
        static void fct( std::ios_base& ib,
                         const fh_context& c,
                         const std::string& eaname );
        

    };

};
#endif
