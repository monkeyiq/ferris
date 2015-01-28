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

    $Id: Ferrisls_AggregateData.hh,v 1.5 2010/09/24 21:30:50 ben Exp $

    *******************************************************************************
*******************************************************************************
******************************************************************************/


#ifndef _ALREADY_INCLUDED_FERRIS_FERRISLS_AGGDATA_H_
#define _ALREADY_INCLUDED_FERRIS_FERRISLS_AGGDATA_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferrisls.hh>

namespace Ferris
{

    class FERRISEXP_API Ferrisls_aggregate_t
    {
    public:
//         Ferrisls_aggregate_t( const Ferrisls_aggregate_t& );
//         Ferrisls_aggregate_t();
        
        
        /* 32 bit align */
        guint32 pad;

        /* generic ones that context class adds */
        guint32 count;
        guint32 maxdepth;

        /* items only available in native VFS module */
        guint32 hardlinkcount;
        guint64 size;
        guint64 sizeFilesOnly;
        guint64 sizeinblocks;
        guint64 filecount;
        guint64 dircount;
        time_t  oldestmtime;
        time_t  oldestctime;
        time_t  oldestatime;
        std::string  oldestmtime_url;
        std::string  oldestctime_url;
        std::string  oldestatime_url;
        time_t  newestmtime;
        time_t  newestctime;
        time_t  newestatime;
        std::string  newestmtime_url;
        std::string  newestctime_url;
        std::string  newestatime_url;

        void reset();
        guint64 sizeIn1KBlocks() const;
        guint64 byteSizeOnDisk() const;
    };

    FERRISEXP_API Ferrisls_aggregate_t operator+( const Ferrisls_aggregate_t& a1, const Ferrisls_aggregate_t& a2 );
    
    
    
    enum aggdata_mode_t
    {
        AGGDATA_DEFAULT   = (1<<0),
        AGGDATA_RECURSIVE = (1<<1),
        
        AGGDATA_SIZE      = (1<<2)
    };
    

    class FERRISEXP_API Ferrisls_aggregate_data
        :
        public Ferrisls_long_display
    {
        /* only direct subcontexts */
        Ferrisls_aggregate_t subd;

        /* all subcontexts recursively */
        Ferrisls_aggregate_t recd;
        
        aggdata_mode_t Mode;

    protected:
        
        friend fh_display_aggdata getAggregateData( fh_context c, aggdata_mode_t m );
        fh_context RootContext;
        bool isDirectSubContextOfRootContext( fh_context c );
        guint32 getDepthOfContext( fh_context c );

        void UpdateAggregateData( Ferrisls_aggregate_t& d, fh_context ctx );
        
    public:

        Ferrisls_aggregate_data();

        virtual void ShowAttributes( fh_context ctx );
        void ShowHeader(fh_context ctx);

        ////////////////
            
        void reset();

        Ferrisls_aggregate_t& getRecursiveData();
        Ferrisls_aggregate_t& getSubData();
        Ferrisls_aggregate_t& getData( int m = 0 );
        void setData( const Ferrisls_aggregate_t& a, aggdata_mode_t m );

        aggdata_mode_t getMode();

        void setRootContext( fh_context c );
    };

    FERRIS_SMARTPTR( Ferrisls_aggregate_data, fh_display_aggdata );

        
    /**
     * Get the info requested on the given context.
     */
    FERRISEXP_API fh_display_aggdata getAggregateData( fh_context c,
                                         aggdata_mode_t m = AGGDATA_DEFAULT );

    FERRISEXP_API fh_display_aggdata createDisplayAggregateData( Ferrisls* fls );
    

};

#endif
