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

    $Id: PcctsChildContext.hh,v 1.3 2010/09/24 21:31:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_PCCTS_CHILD_CONTEXT_H_
#define _ALREADY_INCLUDED_PCCTS_CHILD_CONTEXT_H_

namespace Ferris
{

    class childContext;
    FERRIS_SMARTPTR( childContext, fh_chcontext );

    class childContext : public leafContext
    {
        typedef leafContext  Super;
        typedef childContext ThisClass;

    protected:
    
        childContext( const fh_context& parent, const std::string& token )
            :
            leafContext()
            {
                addAttribute( "token", token );
                addAttribute( "in-order-insert-list",
                              this, &ThisClass::getInOrderInsertListStream );
            }

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        typedef std::list< fh_context > InOrderInsertList_t;
        InOrderInsertList_t InOrderInsertList;
        friend void DepthFirstDeletePCCTS( childContext* cc );
        friend void DepthFirstDeletePCCTS_DropInOderList( childContext* cc );
        

//     /*
//      * Std min is based on child count + 1, as we have no parent reference,
//      * we account for that and then add a extra reference for each child to
//      * compensate for the InOrderList
//      */
//     virtual int  getMinimumReferenceCount()
//         {
//             return Super::getMinimumReferenceCount() - 1 + InOrderInsertList.size();
//         }
    


        virtual fh_context Insert( Context* ctx, bool created = false, bool emit = true )
            throw( SubContextAlreadyInUse )
            {
                LG_PCCTS_D << "Insert() Path:" << getDirPath()
                           << " ctx path:" << ctx->getDirPath()
                           << endl;
                InOrderInsertList.push_back( ctx );
                fh_context ret = Super::Insert( ctx );
                return ret;
            }

        fh_istream getInOrderInsertListStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                bool virgin = true;

                LG_PCCTS_D << "get_InOrderInsertList_AsString() " << endl;
                for( InOrderInsertList_t::const_iterator iter = InOrderInsertList.begin();
                     iter != InOrderInsertList.end();
                     ++iter )
                {
                    virgin = false;
                    ss << (*iter)->getDirName() << endl;
                    LG_PCCTS_D << "get_InOrderInsertList_AsString() adding:"
                               << (*iter)->getDirName()
                               << endl;
                }

                if(virgin)
                {
                    ss << "";
                }
            
                return ss;
            }
    
        virtual void priv_createAttributes()
            {
                LG_PCCTS_D << "priv_createAttributes() Path:" << getDirPath() << endl;

            
                LG_PCCTS_D << "priv_createAttributes(3) Path:" << getDirPath() << endl;
                Super::priv_createAttributes();
            }
    
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
    
    
    public:

        static fh_chcontext Create( fh_chcontext parent, const std::string& rdn )
            {
                fh_chcontext ret(new childContext( GetImpl(parent), rdn ));
                ret->setContext( GetImpl(parent), parent->monsterName(rdn) );

                parent->Insert( GetImpl(ret) );
                ret->addAttribute( (std::string)"token",
                                   SL_getDirNameStream );

                return ret;
            }
    
    

        virtual fh_istream getIStream( ferris_ios::openmode m = ios::in )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                f_istringstream ss( getDirPath() );
                return ss;
            }
    
    };
    
};

#endif
