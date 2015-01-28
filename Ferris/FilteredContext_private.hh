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

    $Id: FilteredContext_private.hh,v 1.7 2010/09/24 21:30:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FILTERED_CONTEXT_PRIVATE_HH_
#define _ALREADY_INCLUDED_FILTERED_CONTEXT_PRIVATE_HH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FilteredContext.hh>
#include <Ferris/ChainedViewContext.hh>

namespace Ferris
{
    
    FERRISEXP_API std::string guessComparisonOperatorFromData( const std::string& val );

    
    /*
     * A context view that applies a matcher object to all items in the view.
     * Items must pass the matcher to be presented via this view.
     */
    class FERRISEXP_API FilteredContext
        :
        public ChainedViewContext
    {
        typedef FilteredContext    _Self;
        typedef ChainedViewContext _Base;
        
        fh_matcher Matcher;

        struct filteringInsertContextCreator
        {
            fh_context m_ctx;
            fh_matcher m_matcher;
            
            filteringInsertContextCreator( const fh_context& ctx, fh_matcher matcher );
            FilteredContext* create( Context* parent, const std::string& rdn ) const;
            void setupExisting( FilteredContext* fc ) const;
            void setupNew( FilteredContext* fc ) const;
        };

        void setupState( const fh_context& ctx, fh_matcher m );
        FilteredContext( Context* theParent, const fh_context& ctx, fh_matcher matcher );
        virtual void UnPageSubContextsIfNeeded();
        
    public:

        FilteredContext( const fh_context& ctx, fh_matcher matcher );
        virtual ~FilteredContext();

        virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
        virtual void OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );
        virtual void OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn );

        void filteringInsertContext( const fh_context& c, bool created = false );

        virtual void read( bool force = 0 );

        void populateFromDelegate();
    
        // Setup is only to be called from the below function!
        void setup();

    protected:

        virtual Context* priv_CreateContext( Context* parent, std::string rdn );


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /*** Handle ability to have local attributes aswell as those of delegate ********/
        /********************************************************************************/
        
        int getNumberOfLocalAttributes();
        std::list< std::string >& getLocalAttributeNames();
        
        virtual std::string private_getStrAttr( const std::string& rdn,
                                                const std::string& def = "",
                                                bool getAllLines = false ,
                                                bool throwEx = false );
    public:
        
        virtual fh_attribute getAttribute( const std::string& rdn ) throw( NoSuchAttribute );
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            ) throw( NoSuchAttribute );
        
        
    };

    namespace FilteredContextNameSpace
    {
        extern const std::string EqualsToken;
        extern const std::string TypeSafeEqualsToken;
        extern const std::string ReEqualsToken;
        extern const std::string GrEqualsToken;
        extern const std::string TypeSafeGrEqualsToken;
        extern const std::string LtEqualsToken;
        extern const std::string TypeSafeLtEqualsToken;
        extern const std::string GrEqualsTokenFloatingPoint;
        extern const std::string LtEqualsTokenFloatingPoint;
        extern const std::string EndsWithToken;
        extern const std::string NotToken;
        extern const std::string AndToken;
        extern const std::string OrToken;
    };
};


#endif

