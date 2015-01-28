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

    $Id: FilteredContext.hh,v 1.5 2011/05/04 21:31:01 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/SM.hh>
#include <Ferris/Regex.hh>
#include <string>

#ifndef _ALREADY_INCLUDED_FERRIS_FILTERED_CONTEXT_HH_
#define _ALREADY_INCLUDED_FERRIS_FILTERED_CONTEXT_HH_

namespace Ferris
{

    namespace Factory
    {
        FERRISEXP_API fh_matcher ComposeEqualsMatcher( const EndingList& el );
        FERRISEXP_API fh_matcher ComposeEndsWithMatcher( const EndingList& el, bool caseSensitive = false );
        FERRISEXP_API fh_matcher ComposeEndsWithMatcher( const char** beg, bool caseSensitive = false );
        FERRISEXP_API fh_matcher ComposeRegexMatcher( const EndingList& el );
        FERRISEXP_API fh_matcher MakeAlwaysTrueMatcher();
        FERRISEXP_API fh_matcher MakeAlwaysFalseMatcher();
        FERRISEXP_API fh_matcher MakeNotMatcher( const fh_matcher& m );
        FERRISEXP_API fh_matcher MakeOrMatcher( const fh_matcher& leftm, const fh_matcher& rightm );
        FERRISEXP_API fh_matcher MakeAndMatcher( const fh_matcher& leftm, const fh_matcher& rightm );
        FERRISEXP_API fh_matcher MakePresenceMatcher( const std::string& eaname );
        FERRISEXP_API fh_matcher MakeHasOneOrMoreBytesMatcher( const std::string& eaname,
                                                 bool createIfNotThere = true );
        FERRISEXP_API fh_matcher MakeEAValueGreaterEqMTime( const std::string& eaname );


        FERRISEXP_API fh_matcher MakeEqualMatcher( const std::string& eaname,
                                     const std::string& value );
        FERRISEXP_API fh_matcher MakeEqualIntegerMatcher( const std::string& eaname,
                                            long value );
        FERRISEXP_API fh_matcher MakeEqualDoubleMatcher( const std::string& eaname, double value );
        FERRISEXP_API fh_matcher MakeEqualBinaryMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeEqualCISMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeTypeSafeEqualMatcher( const std::string& eaname, const std::string& value );

        
        FERRISEXP_API fh_matcher MakeEndsWithMatcher( const std::string& eaname,
                                                      const std::string& value );
        FERRISEXP_API fh_matcher MakeStartsWithMatcher( const std::string& eaname,
                                                        const std::string& value );
        FERRISEXP_API fh_matcher MakeRegexMatcher( const std::string& eaname,
                                                   const std::string& value );

        FERRISEXP_API fh_matcher MakeLtEqMatcher( const std::string& eaname, long value );
        FERRISEXP_API fh_matcher MakeLtEqDoubleMatcher( const std::string& eaname, double value );
        FERRISEXP_API fh_matcher MakeLtEqBinaryMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeLtEqCISMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeTypeSafeLtEqMatcher( const std::string& eaname, const std::string& value );

        FERRISEXP_API fh_matcher MakeGrEqMatcher( const std::string& eaname, long value );
        FERRISEXP_API fh_matcher MakeGrEqDoubleMatcher( const std::string& eaname, double value );
        FERRISEXP_API fh_matcher MakeGrEqBinaryMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeGrEqCISMatcher( const std::string& eaname, const std::string& value );
        FERRISEXP_API fh_matcher MakeTypeSafeGrEqMatcher( const std::string& eaname, const std::string& value );

    }
    

    
    class FilteredContext;
    FERRIS_SMARTPTR( FilteredContext, fh_filtContext );
    
    namespace Factory
    {
        /**
         * Create a filtered view of the given context using the matcher predicate
         *
         * @param ctx base of the new view
         * @param matcher predicate
         */
        FERRISEXP_API fh_filtContext FilteredView( const fh_context& ctx, const fh_matcher& matcher );

        /**
         * Turn a filter string into a filesystem ready to be used by
         * MakeFilteredContext() to filter a view
         */
        FERRISEXP_API fh_context MakeFilter( const std::string& v );

        /**
         * Read the file at 'v' and turn it into a filesystem ready to be used by
         * MakeFilteredContext() to filter a view
         */
        FERRISEXP_API fh_context MakeFilterFromFile( const std::string& v );

        /**
         * Take a filesystem created via either
         * MakeFilter()
         * MakeFilterFromFile()
         * and a base filesystem 'ctx' and create a view with only objects
         * passing the filter being shown
         */
        FERRISEXP_API fh_context MakeFilteredContext( fh_context& ctx, fh_context& filter );
        FERRISEXP_API fh_context MakeFilteredContext( fh_context& ctx, const std::string& filterString );

        /**
         * Create a fh_matcher for external use on contexts
         * nice for eg. selecting stuff in a GUI
         */
        FERRISEXP_API fh_matcher MakeMatcherFromContext( fh_context c );
        
    };
};
#endif

