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

    $Id: BranchRemembranceHistoryInternalContext.cpp,v 1.5 2010/09/24 21:30:24 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/HiddenSymbolSupport.hh>

#include <BranchContext_private.hh>
#include <Resolver_private.hh>
#include <FerrisSemantic.hh>
#include <Ferris/Context_private.hh> // VirtualSoftlinkContext


namespace Ferris
{
    using namespace std;
    using namespace RDFCore;
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL BranchRemembranceHistoryContext
        :
        public StateLessEAHolder< BranchRemembranceHistoryContext, VirtualSoftlinkContext >
    {
        typedef StateLessEAHolder< BranchRemembranceHistoryContext, VirtualSoftlinkContext > _Base;
        typedef BranchRemembranceHistoryContext _Self;

    protected:
        
        virtual std::string getRecommendedEA()
            {
                return Delegate->getRecommendedEA() + ",name-display";
            }
        
    public:

        BranchRemembranceHistoryContext( const fh_context& parent,
                                         const fh_context& target,
                                         const std::string& localName,
                                         bool setupEventConnections = true )
            :
            _Base( parent, target, localName, setupEventConnections )
            {
                createStateLessAttributes();
            }

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                    supplementStateLessAttributes_timet( "name" );
                }
            }

        virtual stringset_t& getForceLocalAttributeNames()
            {
                static stringset_t ret;
                if( ret.empty() )
                {
                    const stringset_t& t = _Base::getForceLocalAttributeNames();
                    copy( t.begin(), t.end(), inserter( ret, ret.end() ) );
                    ret.insert("recommended-ea");
                    ret.insert("name-display");
                }
                return ret;
            }
        
    };
    
    
    class FERRISEXP_DLLLOCAL BranchRemembranceHistoryInternalContext
        :
        public FerrisBranchInternalContext
    {
        typedef BranchRemembranceHistoryInternalContext  _Self;
        typedef FerrisBranchInternalContext              _Base;

    protected:

        virtual void priv_read_leaf()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_fcontext fc = new FakeInternalContext( this, "view" );
                    addNewChild( fc );

                    fh_model m = RDFCore::getDefaultFerrisModel();
                    string pfx = Semantic::getAttrPrefix();
//                    string pfx = "";
                    fh_node historyNode         = RDFCore::Node::CreateURI( pfx + "ferris-file-view-history" );
                    fh_node mostRecentTimeNode  = RDFCore::Node::CreateURI( pfx + "most-recent-view-time" );
                    fh_node originalCmdNodePred = RDFCore::Node::CreateURI( pfx + "view-command" );
                    fh_node actionTime          = RDFCore::Node::CreateURI( pfx + "view-time" );

                    fh_context selfc = Delegate;
                    fh_node selfnode = RDFCore::Node::CreateURI( selfc->getURL() );

                    LG_CTX_D << "looking up url node:" << selfnode->toString() << endl;
                    if( fh_node n = m->getObject( selfnode, historyNode ) )
                    {
                        LG_CTX_D << "have n:" << n->toString() << endl;
                    }
                    

                    NodeIterator iter = m->findObjects( selfnode, historyNode );
                    for( ; iter != NodeIterator(); ++iter )
                    {
                        fh_node anode = *iter;
                        
                        LG_CTX_D << "have anode:" << anode->toString() << endl;
                        NodeIterator atime_iter = m->findObjects( anode, actionTime );
                        for( ; atime_iter != NodeIterator(); ++atime_iter )
                        {
                            time_t tt = toType<time_t>( (*atime_iter)->toString() );
                            LG_CTX_D << "have tt:" << tt << endl;

                            fh_context targetc = selfc;
                            string local_rdn = tostr( tt );
                            
                            fh_context child = new BranchRemembranceHistoryContext( GetImpl(fc),
                                                                                    targetc,
                                                                                    local_rdn );
                            fc->addNewChild( child );
                        }
                    }
                }
            }
        
    public:

        BranchRemembranceHistoryInternalContext( Context* theParent,
                                         const fh_context& theDelegate,
                                         const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~BranchRemembranceHistoryInternalContext()
            {
            }

        fh_context getDelegate()
            {
                return Delegate;
            }
    };

    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    BranchRemembranceHistoryInternalContext_Creator( Context* ctx,
                                             const fh_context& theDelegate,
                                             const std::string& rdn )
    {
        return new BranchRemembranceHistoryInternalContext( ctx, theDelegate, rdn );
    }
    
    static bool BranchRemembranceHistoryInternalContext_Dropper =
              FerrisBranchRootContext_Register( "branchfs-remembrance",
                                                BranchInternalContextCreatorFunctor_t(
                                                    BranchRemembranceHistoryInternalContext_Creator ) );
    

    
};
