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

    $Id: Ferrisls_AggregateData.cpp,v 1.5 2010/09/24 21:30:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferrisls_AggregateData.hh>
#include <Ferrisls.hh>
#include <Native.hh>

using namespace std;

namespace Ferris
{
//     Ferrisls_aggregate_t::Ferrisls_aggregate_t( const Ferrisls_aggregate_t& )
//     {
//         reset();
//     }
    
//     Ferrisls_aggregate_t::Ferrisls_aggregate_t()
//     {
//         reset();
//     }
    

    Ferrisls_aggregate_t operator+( const Ferrisls_aggregate_t& a1, const Ferrisls_aggregate_t& a2 )
    {
        Ferrisls_aggregate_t ret;
        ret.reset();

        ret.count = a1.count + a2.count;
        ret.maxdepth = a1.maxdepth + a2.maxdepth;
        ret.hardlinkcount = a1.hardlinkcount + a2.hardlinkcount;
        ret.size = a1.size + a2.size;
        ret.filecount = a1.filecount + a2.filecount;
        ret.dircount = a1.dircount + a2.dircount;
        ret.sizeinblocks = a1.sizeinblocks + a2.sizeinblocks;
        ret.sizeFilesOnly = a1.sizeFilesOnly + a2.sizeFilesOnly;
        ret.oldestmtime = min( a1.oldestmtime, a2.oldestmtime );
        ret.oldestctime = min( a1.oldestctime, a2.oldestctime );
        ret.oldestatime = min( a1.oldestatime, a2.oldestatime );
        ret.oldestmtime_url = "";
        ret.oldestctime_url = "";
        ret.oldestatime_url = "";
        ret.newestmtime = min( a1.newestmtime, a2.newestmtime );
        ret.newestctime = min( a1.newestctime, a2.newestctime );
        ret.newestatime = min( a1.newestatime, a2.newestatime );
        ret.newestmtime_url = "";
        ret.newestctime_url = "";
        ret.newestatime_url = "";
        return ret;
    }
    
    
    Ferrisls_aggregate_data::Ferrisls_aggregate_data()
    {
        reset();
        RecordSeperator = "";
    }


    void
    Ferrisls_aggregate_data::UpdateAggregateData( Ferrisls_aggregate_t& d, fh_context ctx )
    {
        if( (&d == &recd) )
        {
            LG_CTX_D << "UpdateAggregateData() rec:" << (&d == &recd) << " ctx:" << ctx->getURL() << endl;
//            BackTrace();
        }
        
        d.count++;
            
        Context* c = GetImpl(ctx);
        if( NativeContext* nc = dynamic_cast<NativeContext*>(c) )
        {
            const struct stat& sb = nc->getStat_Follow();

            d.hardlinkcount  = sb.st_nlink;
            d.size          += sb.st_size;
            d.sizeinblocks  += sb.st_blocks;

            if(S_ISREG(sb.st_mode))
                d.sizeFilesOnly += sb.st_size;
            
            LG_CTX_D << "UpdateAggregateData() blocks:" << sb.st_blocks << " ctx:" << ctx->getURL() << endl;
            
            if(S_ISREG(sb.st_mode))        d.filecount++;
            else if(S_ISDIR(sb.st_mode))   d.dircount++;

            if( sb.st_atime < d.oldestatime )
            {
                d.oldestatime     = sb.st_atime;
                d.oldestatime_url = ctx->getURL();
            }
            if( sb.st_mtime < d.oldestmtime )
            {
                d.oldestmtime     = sb.st_mtime;
                d.oldestmtime_url = ctx->getURL();
            }
            if( sb.st_ctime < d.oldestctime )
            {
                d.oldestctime     = sb.st_ctime;
                d.oldestctime_url = ctx->getURL();
            }
            if( sb.st_atime > d.newestatime )
            {
                d.newestatime     = sb.st_atime;
                d.newestatime_url = ctx->getURL();
            }
            if( sb.st_mtime > d.newestmtime )
            {
                d.newestmtime     = sb.st_mtime;
                d.newestmtime_url = ctx->getURL();
            }
            if( sb.st_ctime > d.newestctime )
            {
                d.newestctime     = sb.st_ctime;
                d.newestctime_url = ctx->getURL();
            }
        }
    }
    
    
    void
    Ferrisls_aggregate_data::ShowAttributes( fh_context ctx )
    {
//        cerr << "Ferrisls_aggregate_data::ShowAttributes() url:" << ctx->getURL() << endl;
        
        if( isDirectSubContextOfRootContext( ctx ) )
        {
            UpdateAggregateData( subd, ctx );
        }
        
        UpdateAggregateData( recd, ctx );
    }
        
    void
    Ferrisls_aggregate_data::ShowHeader(fh_context ctx)
    {
        recd.maxdepth = std::max( recd.maxdepth, getDepthOfContext( ctx ));
    }

    bool
    Ferrisls_aggregate_data::isDirectSubContextOfRootContext( fh_context c )
    {
        if( !isBound( RootContext ) || !c->isParentBound() )
        {
            return false;
        }

        return c->getParent() == GetImpl(RootContext);
    }

    guint32
    Ferrisls_aggregate_data::getDepthOfContext( fh_context c )
    {
        guint32 d = 0;
        
        while( GetImpl(c) != GetImpl(RootContext) )
        {
            c = c->getParent();
            d++;
        }
        return d;
    }
    
    
    
    void
    Ferrisls_aggregate_data::reset()
    {
        recd.reset();
        subd.reset();
    }

    Ferrisls_aggregate_t&
    Ferrisls_aggregate_data::getRecursiveData()
    {
        return recd;
    }

    Ferrisls_aggregate_t&
    Ferrisls_aggregate_data::getSubData()
    {
        return subd;
    }
    
    Ferrisls_aggregate_t&
    Ferrisls_aggregate_data::getData( int m )
    {
        if( m & AGGDATA_RECURSIVE )
        {
            return getRecursiveData();
        }
        return getSubData();
    }

    void Ferrisls_aggregate_data::setData( const Ferrisls_aggregate_t& a, aggdata_mode_t m )
    {
        if( m & AGGDATA_RECURSIVE )
        {
            recd = a;
        }
        subd = a;
    }
    
    
    
    aggdata_mode_t
    Ferrisls_aggregate_data::getMode()
    {
        return Mode;
    }
    
    void
    Ferrisls_aggregate_data::setRootContext( fh_context c )
    {
        RootContext = c;
    }
    
    
        
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
        
    void
    Ferrisls_aggregate_t::reset()
    {
        count = 0;
        maxdepth = 0;
        hardlinkcount = 0;
        size = 0;
        sizeFilesOnly = 0;
        filecount = 0;
        dircount = 0;
        sizeinblocks = 0;
        oldestmtime = Time::getTime();
        oldestctime = oldestmtime;
        oldestatime = oldestmtime;
        oldestmtime_url = "";
        oldestctime_url = "";
        oldestatime_url = "";
        newestmtime = 0;
        newestctime = 0;
        newestatime = 0;
        newestmtime_url = "";
        newestctime_url = "";
        newestatime_url = "";
    }

    guint64 Ferrisls_aggregate_t::sizeIn1KBlocks() const
    {
        return sizeinblocks/2;
    }
    
    guint64 Ferrisls_aggregate_t::byteSizeOnDisk() const
    {
        return sizeIn1KBlocks() * 1024;
    }
    
    
        

    fh_display_aggdata getAggregateData( fh_context c,
                                         aggdata_mode_t m )
    {
        fh_display_aggdata  d = new Ferrisls_aggregate_data();
        fh_ls     ls          = new Ferrisls();

        d->reset();
        d->setRootContext( c );

        ls->setURL( c->getURL() );
        ls->setDisplay( d );
        ls->setRecursiveList( m & AGGDATA_RECURSIVE );
        ls->setDontDecendIntoFiles( true );
        
        ls->operator()();
        d->UpdateAggregateData( d->getData( m ), c );
            
        return d;
    }
        

    fh_display_aggdata createDisplayAggregateData( Ferrisls* ls )
    {
        ls->setListDirectoryNodeOnly( false );
        ls->setForceReadRootDirectoryNodes( false );
        ls->setRecursiveList( true );
        ls->setDontDecendIntoFiles( true );
        ls->usePreorderTraversal( true );

        fh_display_aggdata d = new Ferrisls_aggregate_data();
        d->reset();
        ls->setDisplay( d );
        return d;
    }
    

};

