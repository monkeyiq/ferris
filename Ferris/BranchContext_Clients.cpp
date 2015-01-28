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

    $Id: BranchContext_Clients.cpp,v 1.11 2011/06/17 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>

#include <BranchContext_private.hh>
#include <Resolver_private.hh>
#include <Ferris/Context_private.hh> // VirtualSoftlinkContext
#include <Ferris/Medallion.hh>
#include <Ferris/Personalities.hh>
#include <Ferris/Runner.hh>
#include <Ferris/xfsutil.hh>

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility push(hidden)
// #endif

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3
#include <boost/spirit.hpp>
using namespace boost::spirit;

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility pop
// #endif


using namespace std;

namespace Ferris
{
#define DUBCORE_DESCRIPTION "dc:description"

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL BranchParentsInternalContext
        :
        public FerrisBranchInternalContext
    {
        typedef BranchParentsInternalContext _Self;
        typedef FerrisBranchInternalContext  _Base;

    protected:

        virtual void priv_read_leaf()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_context c = Delegate->getParent();
                    fh_context child = new VirtualSoftlinkContext( this, c );
                    addNewChild( child );

                    LG_CTX_D << "BranchParentsInternalContext::read_leaf()"
                             << " this:" << toVoid( this )
                             << " c:" << toVoid( GetImpl(c) )
                             << " child:" << toVoid( GetImpl( child ) )
                             << " child.parent:" << toVoid( child->getParent() )
                             << endl;
                    LG_CTX_D << "    this.url :" << getURL() << endl
                             << "    c.url    :" << c->getURL() << endl
                             << "    child.url:" << child->getURL() << endl
                             << "    child.parent.url:" << child->getParent()->getURL() << endl;
                }
                
            }
        
    public:

        BranchParentsInternalContext( Context* theParent,
                                      const fh_context& theDelegate,
                                      const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~BranchParentsInternalContext()
            {
            }
    };

    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    BranchParentsInternalContext_Creator( Context* ctx,
                                          const fh_context& theDelegate,
                                          const std::string& rdn )
    {
        return new BranchParentsInternalContext( ctx, theDelegate, rdn );
    }
    
    static bool BranchParentsInternalContext_Dropper =
    FerrisBranchRootContext_Register( "branchfs-parents",
                                      BranchInternalContextCreatorFunctor_t(
                                          BranchParentsInternalContext_Creator ) );


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    class FERRISEXP_DLLLOCAL BranchAttributeAsContentsContext
        :
        public leafContext
    {
        typedef BranchAttributeAsContentsContext _Self;
        typedef leafContext                _Base;
        
    protected:
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception);
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);
        virtual ferris_ios::openmode getSupportedOpenModes();
        
    public:
        BranchAttributeAsContentsContext( Context* parent, std::string rdn )
            :
            _Base( parent, rdn )
            {
                setOverMountAttemptHasAlreadyFailed( true );
            }
    };
    
    class FERRISEXP_DLLLOCAL BranchAttributesInternalContext
        :
        public FerrisBranchInternalContext
    {
        typedef BranchAttributesInternalContext  _Self;
        typedef FerrisBranchInternalContext      _Base;

    protected:

        virtual void priv_read_leaf()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();

                typedef AttributeCollection::AttributeNames_t an_t;
                typedef an_t::iterator ani_t;
                
                fh_context c  = Delegate;
                an_t       an;
                c->getAttributeNames( an );
                for( ani_t ani = an.begin(); ani != an.end(); ++ani )
                {
                    string rdn = *ani;

                    LG_CTX_D << "BranchAttributesInternalContext::read() rdn:" << rdn << endl;

                    BranchAttributeAsContentsContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    
//                     fh_context child = new BranchAttributeAsContentsContext( this, rdn );
//                     addNewChild( child );

                    LG_CTX_D << "BranchAttributesInternalContext::read_leaf()"
                             << " this:" << toVoid( this )
                             << " child:" << c
                             << " child.parent:" << toVoid( c->getParent() )
                             << endl;
                    LG_CTX_D << "    this.url :" << getURL() << endl
                             << "    child.url:" << c->getURL() << endl
                             << "    child.parent.url:" << c->getParent()->getURL() << endl;
                }
            }
        
    public:

        BranchAttributesInternalContext( Context* theParent,
                                         const fh_context& theDelegate,
                                         const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~BranchAttributesInternalContext()
            {
            }

        fh_context getDelegate()
            {
                return Delegate;
            }
    };

    fh_istream
    BranchAttributeAsContentsContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        fh_context c = getParent();
        BranchAttributesInternalContext* baic = dynamic_cast<BranchAttributesInternalContext*>( GetImpl(c));
        c = baic->getDelegate();
                
        fh_stringstream ss;
        ss << getStrAttr( c, getDirName(), "", true, true );
        return ss;
    }

    fh_iostream
    BranchAttributeAsContentsContext::priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
    {
        fh_context c = getParent();
        BranchAttributesInternalContext* baic = dynamic_cast<BranchAttributesInternalContext*>( GetImpl(c));
        c = baic->getDelegate();

        fh_attribute  a = c->getAttribute( getDirName() );
        fh_iostream ret = a->getIOStream( m );
        return ret;
    }

    ferris_ios::openmode
    BranchAttributeAsContentsContext::getSupportedOpenModes()
    {
        fh_context c = getParent();
        BranchAttributesInternalContext* baic = dynamic_cast<BranchAttributesInternalContext*>( GetImpl(c));
        c = baic->getDelegate();
        return c->getSupportedOpenModes();
    }
   
   
    
    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    BranchAttributesInternalContext_Creator( Context* ctx,
                                             const fh_context& theDelegate,
                                             const std::string& rdn )
    {
        return new BranchAttributesInternalContext( ctx, theDelegate, rdn );
    }
    
    static bool BranchAttributesInternalContext_Dropper =
    FerrisBranchRootContext_Register( "branchfs-attributes",
                                      BranchInternalContextCreatorFunctor_t(
                                          BranchAttributesInternalContext_Creator ) );


    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Context showing the state of an individual emblem in the medallion
     */
    class FERRISEXP_DLLLOCAL BranchMedallionEmblemContext
        :
        public StateLessEAHolder< BranchMedallionEmblemContext, FakeInternalContext >
    {
        typedef BranchMedallionEmblemContext _Self;
        typedef StateLessEAHolder< BranchMedallionEmblemContext, FakeInternalContext > _Base;

    protected:

        fh_medallion m_med;
        fh_emblem    m_em;

        virtual void priv_read();
        
    public:

        void constructObject( fh_medallion m, fh_emblem em );
        BranchMedallionEmblemContext( Context* parent, const std::string& rdn,
                                      fh_medallion m = 0,
                                      fh_emblem em = 0 );
        virtual ~BranchMedallionEmblemContext();

        virtual std::string priv_getRecommendedEA();
        static fh_stringstream SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getID( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getParents( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getChildren( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPossibleParents( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPossibleChildren( Context* c, const std::string& rdn, EA_Atom* atom );

        static fh_stringstream SL_getName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUniqueName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIconName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getLimitedViewPri( Context* c, const std::string& rdn, EA_Atom* atom );

        void createStateLessAttributes( bool force = false );
    };


    /********************/
    /********************/

    class FERRISEXP_DLLLOCAL BranchMedallionEmblemBeliefContext
        :
        public StateLessEAHolder< BranchMedallionEmblemBeliefContext, leafContext >
    {
        typedef BranchMedallionEmblemBeliefContext _Self;
        typedef StateLessEAHolder< BranchMedallionEmblemBeliefContext, leafContext > _Base;

    protected:

        fh_medallionBelief m_be;
        
    public:

        void constructObject( fh_medallionBelief be );
        BranchMedallionEmblemBeliefContext( Context* parent, const std::string& rdn,
                                            fh_medallionBelief be = 0 );
        virtual ~BranchMedallionEmblemBeliefContext();

        
        virtual std::string priv_getRecommendedEA();
        static fh_stringstream SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPersonalityName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPersonalityID( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getATime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getMTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getSureness( Context* c, const std::string& rdn, EA_Atom* atom );

        void createStateLessAttributes( bool force = false );
    };
    

    /********************/
    /********************/
    
    
    class FERRISEXP_DLLLOCAL BranchMedallionInternalContext
        :
        public FerrisBranchInternalContext
    {
        typedef BranchMedallionInternalContext _Self;
        typedef FerrisBranchInternalContext  _Base;

    protected:

        virtual void priv_read_leaf()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();

                fh_medallion m = Delegate->getMedallion();

                emblems_t el = m->getAllEmblems();
                for( emblems_t::iterator ei = el.begin(); ei!=el.end(); ++ei )
                {
                    fh_emblem     em = *ei;
                    string       rdn = em->getName();

                    BranchMedallionEmblemContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->constructObject( m, em );
//                     fh_context child = new BranchMedallionEmblemContext( this, rdn.c_str(), m, em );
//                     addNewChild( child );
                }
            }
        
    public:

        BranchMedallionInternalContext( Context* theParent,
                                      const fh_context& theDelegate,
                                      const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~BranchMedallionInternalContext()
            {
            }
    };

    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    BranchMedallionInternalContext_Creator( Context* ctx,
                                          const fh_context& theDelegate,
                                          const std::string& rdn )
    {
        return new BranchMedallionInternalContext( ctx, theDelegate, rdn );
    }
    
    static bool BranchMedallionInternalContext_Dropper =
    FerrisBranchRootContext_Register( "branchfs-medallions",
                                      BranchInternalContextCreatorFunctor_t(
                                          BranchMedallionInternalContext_Creator ) );
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    BranchMedallionEmblemContext::constructObject( fh_medallion m, fh_emblem em )
    {
        m_med = m;
        m_em = em;
    }
    
    
    BranchMedallionEmblemContext::BranchMedallionEmblemContext(
        Context* parent, const std::string& rdn,
        fh_medallion m, fh_emblem em )
        :
        _Base( parent, rdn ),
        m_med( m ),
        m_em( em )
    {
        createStateLessAttributes();
    }
    
    BranchMedallionEmblemContext::~BranchMedallionEmblemContext()
    {
    }
    
    std::string
    BranchMedallionEmblemContext::priv_getRecommendedEA()
    {
        return "name,id,icon-name,parent-ids";
    }

    fh_stringstream
    BranchMedallionEmblemContext::SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << "An emblem that has some assertion or retraction collection associated with it." << endl
               << cc->m_em->getDescription();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getID( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << cc->m_em->getID();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getParents( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            emblems_t el = cc->m_em->getParents();
            ss << Util::createSeperatedList( el.begin(), el.end() );
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getChildren( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            emblems_t el = cc->m_em->getChildren();
            ss << Util::createSeperatedList( el.begin(), el.end() );
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getPossibleParents( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            emblems_t el = cc->m_em->getPossibleParents();
            ss << Util::createSeperatedList( el.begin(), el.end() );
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getPossibleChildren( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            emblems_t el = cc->m_em->getPossibleChildren();
            ss << Util::createSeperatedList( el.begin(), el.end() );
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << cc->m_em->getName();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getUniqueName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << cc->m_em->getUniqueName();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getIconName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << cc->m_em->getIconName();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemContext::SL_getLimitedViewPri( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemContext* cc = dynamic_cast<BranchMedallionEmblemContext*>(c))
        {
            ss << cc->m_em->getLimitedViewPriority();
        }
        return ss;
    }
    
    void
    BranchMedallionEmblemContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA tryAddStateLessAttribute

            SLEA( DUBCORE_DESCRIPTION,   &_Self::SL_getDesc,             XSD_BASIC_STRING );
            SLEA( "id",                  &_Self::SL_getID,               XSD_BASIC_INT );
            SLEA( "parent-ids",          &_Self::SL_getParents,          FXD_INTLIST );
            SLEA( "child-ids",           &_Self::SL_getChildren,         FXD_INTLIST );
            SLEA( "possible-parent-ids", &_Self::SL_getPossibleParents,  FXD_INTLIST );
            SLEA( "possible-child-ids",  &_Self::SL_getPossibleChildren, FXD_INTLIST );

            SLEA( "name",                  &_Self::SL_getName,           XSD_BASIC_STRING );
            SLEA( "unique-name",           &_Self::SL_getUniqueName,     XSD_BASIC_STRING );
            SLEA( "icon-name",             &_Self::SL_getIconName,       XSD_BASIC_STRING );
            SLEA( "limited-view-priority", &_Self::SL_getLimitedViewPri, XSD_BASIC_INT );
            
#undef SLEA
            
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
        
    }

    void
    BranchMedallionEmblemContext::priv_read()
    {
        EnsureStartStopReadingIsFiredRAII _raii1( this );
        clearContext();

        typedef std::list< fh_personality > plist_t;
        plist_t pl = m_med->getListOfPersonalitiesWhoHaveOpinion( m_em );
        for( plist_t::iterator pi = pl.begin(); pi!=pl.end(); ++pi )
        {
            fh_medallionBelief be = m_med->getBelief( m_em, *pi );
            string rdn = (*pi)->getName();

            BranchMedallionEmblemBeliefContext* c = 0;
            c = priv_ensureSubContext( rdn, c );
            c->constructObject( be );
                
//                 fh_context child = new BranchMedallionEmblemBeliefContext( this, rdn.c_str(), be );
//                 addNewChild( child );

            LG_CTX_D << "priv_read() url:" << getURL()
                     << " rdn:" << rdn
                     << " child.url:" << c->getURL()
                     << endl;
        }
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    BranchMedallionEmblemBeliefContext::constructObject( fh_medallionBelief be )
    {
        m_be = be;
    }
    

    BranchMedallionEmblemBeliefContext::BranchMedallionEmblemBeliefContext(
        Context* parent, const std::string& rdn,
        fh_medallionBelief be )
        :
        _Base( parent, rdn ),
        m_be( be )
    {
        createStateLessAttributes();
    }
        
    BranchMedallionEmblemBeliefContext::~BranchMedallionEmblemBeliefContext()
    {
    }
    
    std::string
    BranchMedallionEmblemBeliefContext::priv_getRecommendedEA()
    {
        return "name,personality-id,sureness,atime-display,mtime-display";
    }

    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            ss << "A particular assertion or retraction of an emblem for a file.";
        }
        return ss;
    }

    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getPersonalityName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            ss << cc->m_be->getPersonality()->getName();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getPersonalityID( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            ss << cc->m_be->getPersonality()->getID();
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getATime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            time_t tt = cc->m_be->getTimes()->getATime();
            ss << tt;
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getMTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            time_t tt = cc->m_be->getTimes()->getMTime();
            ss << tt;
        }
        return ss;
    }
    fh_stringstream
    BranchMedallionEmblemBeliefContext::SL_getSureness( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( BranchMedallionEmblemBeliefContext* cc = dynamic_cast<BranchMedallionEmblemBeliefContext*>(c))
        {
            ss << cc->m_be->getSureness();
        }
        return ss;
    }
    
    void
    BranchMedallionEmblemBeliefContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA tryAddStateLessAttribute

            SLEA( DUBCORE_DESCRIPTION,   &_Self::SL_getDesc,             XSD_BASIC_STRING );

            SLEA( "personality-name",    &_Self::SL_getPersonalityName,  XSD_BASIC_STRING );
            SLEA( "personality-id",      &_Self::SL_getPersonalityID,    XSD_BASIC_INT    );
            SLEA( "atime",               &_Self::SL_getATime,            FXD_UNIXEPOCH_T  );
            SLEA( "mtime",               &_Self::SL_getMTime,            FXD_UNIXEPOCH_T  );
            SLEA( "sureness",            &_Self::SL_getSureness,         XSD_BASIC_DOUBLE );
            
#undef SLEA
            
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL BranchExtentsContext_SingleExtent
        :
        public StateLessEAHolder< BranchExtentsContext_SingleExtent, leafContext >
    {
        typedef BranchExtentsContext_SingleExtent _Self;
        typedef StateLessEAHolder< BranchExtentsContext_SingleExtent, leafContext > _Base;

        streamsize offbeg;
        streamsize offend;
        streamsize bbeg;
        streamsize bend;
        
    protected:
        std::string
        priv_getRecommendedEA()
            {
                return "name,start-block,end-block,start-address,end-address";
            }


        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_ifstream basess( getDirPath().c_str(), ferris_ios::maskOffFerrisOptions(m) );
                fh_istream ret = Factory::MakeLimitingIStream( basess, offbeg, offend );
                return ret;
            }

        static fh_stringstream
        SL_getStartBlock( BranchExtentsContext_SingleExtent* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->bbeg;
                return ss;
            }
        static fh_stringstream
        SL_getEndBlock( BranchExtentsContext_SingleExtent* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->bend;
                return ss;
            }
        static fh_stringstream
        SL_getStartAddr( BranchExtentsContext_SingleExtent* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->offbeg;
                return ss;
            }
        static fh_stringstream
        SL_getEndAddr( BranchExtentsContext_SingleExtent* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->offend;
                return ss;
            }
        
        void
        createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
#define SLEA tryAddStateLessAttribute

                    SLEA( "start-block",    &_Self::SL_getStartBlock,     XSD_BASIC_INT );
                    SLEA( "end-block",      &_Self::SL_getEndBlock,       XSD_BASIC_INT );
                    SLEA( "start-address",  &_Self::SL_getStartAddr,      XSD_BASIC_INT );
                    SLEA( "end-address",    &_Self::SL_getEndAddr,        XSD_BASIC_INT );
            
#undef SLEA
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
    public:
        void constructObject( streamsize xoffbeg,
                    streamsize xoffend,
                    streamsize xbbeg,
                    streamsize xbend )
            {
                offbeg = xoffbeg;
                offend = xoffend;
                bbeg   = xbbeg;
                bend   = xbend;
            }
        
        BranchExtentsContext_SingleExtent( Context* parent, std::string rdn,
                                           streamsize offbeg = 0,
                                           streamsize offend = 0,
                                           streamsize bbeg = 0,
                                           streamsize bend = 0 )
            :
            _Base( parent, rdn ),
            offbeg( offbeg ), offend( offend ), bbeg( bbeg ), bend( bend )
            {
                createStateLessAttributes();
            }
    };


    class BranchExtentsContext;
    struct FERRISEXP_DLLLOCAL Xfs_Bmap_Functor
    {
        BranchExtentsContext* bec;
        mutable vector< streamsize > r;
        Xfs_Bmap_Functor( BranchExtentsContext* bec, vector< streamsize >& r )
            : bec(bec), r(r)
            {
            }
    
        void operator()(const char* const& , const char* const&) const;
        void setupContiguous();
    };
    
    class FERRISEXP_DLLLOCAL BranchExtentsContext
        :
        public FerrisBranchInternalContext
    {
        typedef BranchExtentsContext _Self;
        typedef FerrisBranchInternalContext  _Base;
        typedef Context _DontDelegateBase;
        

        friend struct Xfs_Bmap_Functor;
        bool m_haveFailed;
        bool m_isContiguous;
        
    protected:

        std::string
        getRecommendedEA()
            {
                return "name,start-block,end-block,start-address,end-address,is-contiguous";
            }

        
        virtual void priv_read_leaf()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();

                if( !m_haveFailed )
                {
                    LG_CTX_D << "priv_read_leaf()" << endl;

                    fh_context c  = Delegate;
                    m_haveFailed |= isXFS( c );
                    if( m_haveFailed )
                    {
                        return;
                    }
                    
                    string   earl = c->getDirPath();
                    stringstream ss;
                    
                    {
                        
                        fh_runner r = new Runner();
                        r->setSpawnFlags(
                            GSpawnFlags(
                                G_SPAWN_SEARCH_PATH |
                                G_SPAWN_STDERR_TO_DEV_NULL |
//                         G_SPAWN_STDOUT_TO_DEV_NULL |
                                r->getSpawnFlags()));
                        r->pushCommandLineArg( "xfs_bmap" );
                        r->pushCommandLineArg( earl );
                    
                        r->Run();
                        fh_istream stdoutss =  r->getStdOut();
                    
                        {
                            string tmp;
                            getline(stdoutss,tmp);
                        }
                        copy( istreambuf_iterator<char>(stdoutss),
                              istreambuf_iterator<char>(),
                              ostreambuf_iterator<char>(ss));
                        int e = r->getExitStatus();
                    }
                
                    LG_CTX_D << "xfs_bmap output:" << ss.str() << endl;
                    string datastring = ss.str();
                    
                    typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
                    typedef rule< scanners > R;

                    vector< streamsize > r;
                    Xfs_Bmap_Functor xfs_bmap_functor( this, r );
                    uint_parser<streamsize, 10, 1, -1> uint64_p; 
                    R xfs_bmap_p
                        = +(uint64_p[push_back_a(r)] >> ch_p(':')
                            >> ch_p('[')
                            >> uint64_p[push_back_a(r)]
                            >> ch_p('.') >> ch_p('.')
                            >> uint64_p[push_back_a(r)]
                            >> ch_p(']')
                            >> ch_p(':')
                            >> uint64_p[push_back_a(r)]
                            >> ch_p('.') >> ch_p('.')
                            >> uint64_p[push_back_a(r)])
                        [xfs_bmap_functor];
    
                    parse_info<> info = parse(
                        datastring.c_str(),
                        xfs_bmap_p,
                        space_p );
            
                    if (info.full)
                    {
                        xfs_bmap_functor.setupContiguous();
                    }
                    else
                    {
                        m_haveFailed = true;
                        stringstream ss;
                        ss  << "Parsing failed" << nl
                            << "input:" << datastring << nl
                            << "stopped at: " << info.stop << "\"" << nl
                            << "char offset:" << ( info.stop - datastring.c_str() ) << nl;
                        LG_CTX_I << ss.str() << endl;
                        cerr << ss.str() << endl;
                    }
                }
            }

        fh_stringstream
        SL_isContiguous( Context*, const std::string& rdn, EA_Atom* atom )
            {
                LG_CTX_D << "SL_isContiguous()" << endl;
                fh_stringstream ss;
                read();
                ss << m_isContiguous;
                return ss;
            }
        
//         void
//         createStateLessAttributes( bool force = false )
//             {
//                 if( force || isStateLessEAVirgin() )
//                 {
// #define SLEA tryAddStateLessAttribute

//                     SLEA( "is-contiguous",    &_Self::SL_isContiguous, XSD_BASIC_BOOL );
            
// #undef SLEA
//                     _Base::createStateLessAttributes( true );
//                     supplementStateLessAttributes( true );
//                 }
//             }

        stringset_t&
        getForceLocalAttributeNames()
            {
                static stringset_t ret;
                if( ret.empty() )
                {
                    ret.insert("is-contiguous");
                    ret.insert("recommended-ea");
                }
                return ret;
            }

        void
        priv_createAttributes()
            {
                LG_CTX_D << "priv_createAttributes(extent fs)" << endl;
                addAttribute( "is-contiguous",
                              this, &_Self::SL_isContiguous,
                              XSD_BASIC_BOOL );
                _Base::priv_createAttributes();
            }

//         std::string
//         private_getStrAttr( const std::string& rdn,
//                                                     const std::string& def,
//                                                     bool getAllLines,
//                                                     bool throwEx )
//             {
//                 stringlist_t& sl = getForceLocalAttributeNames();
//                 if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//                     return _DontDelegateBase::private_getStrAttr( rdn, def, getAllLines, throwEx );

//                 return _Base::private_getStrAttr( rdn, def, getAllLines, throwEx );
//             }
//         fh_attribute
//         getAttribute( const string& rdn ) throw( NoSuchAttribute )
//             {
//                 stringlist_t& sl = getForceLocalAttributeNames();
//                 if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//                     return _DontDelegateBase::getAttribute( rdn );
        
//                 return Delegate->getAttribute(rdn);
//             }
//         AttributeCollection::AttributeNames_t&
//         getAttributeNames( AttributeNames_t& ret )
//             {
//                 AttributeCollection::AttributeNames_t t1;
//                 AttributeCollection::AttributeNames_t t2;
//                 Delegate->getAttributeNames( t1 );
//                 _DontDelegateBase::getAttributeNames( t2 );
//                 return mergeAttributeNames( ret, t1, t2 );
//             }
//         int
//         getAttributeCount()
//             {
//                 return Delegate->getAttributeCount();
//             }
//         bool
//         isAttributeBound( const std::string& rdn,
//                           bool createIfNotThere
//             ) throw( NoSuchAttribute )
//             {
//                 stringlist_t& sl = getForceLocalAttributeNames();
//                 if( sl.end() != find( sl.begin(), sl.end(), rdn ) )
//                     return _DontDelegateBase::isAttributeBound( rdn, createIfNotThere );

//                 return Delegate->isAttributeBound( rdn, createIfNotThere );
//             }
        
    public:

        BranchExtentsContext( Context* theParent,
                              const fh_context& theDelegate,
                              const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn ),
            m_haveFailed( false ),
            m_isContiguous( false )
            {
                createStateLessAttributes();
            }
        
        virtual ~BranchExtentsContext()
            {
            }
    };

    void Xfs_Bmap_Functor::operator()(const char* const& , const char* const&) const
    {
//         cerr << "operator()" << endl;
//         for( vector< int >::const_iterator ri = r.begin(); ri!=r.end();++ri )
//         {
//             cerr << *ri << endl;
//         }

        if( r.size()==5 )
        {
            vector< streamsize >::const_iterator ri = r.begin();
            streamsize rdn    = *ri; ++ri;
            streamsize offbeg = *ri; ++ri;
            streamsize offend = *ri; ++ri;
            streamsize bbeg   = *ri; ++ri;
            streamsize bend   = *ri; ++ri;

            BranchExtentsContext_SingleExtent* c = 0;
            c = bec->priv_ensureSubContext( tostr(rdn), c );
            c->constructObject( offbeg, offend, bbeg, bend );
//             fh_context child = new BranchExtentsContext_SingleExtent( bec, tostr(rdn),
//                                                                       offbeg, offend,
//                                                                       bbeg, bend );
//             bec->addNewChild( child );
            r.clear();
        }
    }

    void Xfs_Bmap_Functor::setupContiguous()
    {
        bec->m_isContiguous = (bec->getSubContextCount()==1);
    }
    
    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    BranchExtentsContext_Creator( Context* ctx,
                                          const fh_context& theDelegate,
                                          const std::string& rdn )
    {
        return new BranchExtentsContext( ctx, theDelegate, rdn );
    }
    
    static bool
    BranchExtentsContext_Dropper =
                        FerrisBranchRootContext_Register(
                            "branchfs-extents",
                            BranchInternalContextCreatorFunctor_t(
                                BranchExtentsContext_Creator ) );


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /**
     * Context showing all the branch filesystems for a context as direct
     * children. This is the context returned by Context::getBranchFileSystem();
     */
    class FERRISEXP_DLLLOCAL FerrisAllBranchesContext
        :
        public FerrisBranchInternalContext
    {
        typedef FerrisAllBranchesContext _Self;
        typedef FerrisBranchInternalContext  _Base;

    protected:

        virtual void priv_read_leaf()
            {
                staticDirContentsRAII _raii1( this );

                LG_CTX_D << "priv_read_leaf() url:" << getURL()
                         << " empty():" << empty()
                         << " branches:" << getStrAttr( Delegate, "associated-branches", "" )
                         << endl;
                
                if( empty() )
                {
                    fh_context theContext = Delegate;
                    stringlist_t sl = Util::parseCommaSeperatedList(
                        getStrAttr( theContext, "associated-branches", "" ));

                    for( stringlist_t::const_iterator si = sl.begin(); si != sl.end(); ++si )
                    {
                        const string& name = *si;
                
                        try
                        {
                            string earl = getStrAttr( theContext, name, "" );
                            fh_context c = Resolve( earl );
                            fh_context child = new VirtualSoftlinkContext( this, c, name );
                            addNewChild( child );

                            LG_CTX_D << "priv_read_leaf() url:" << getURL()
                                 << " adding child for name:" << name << endl
                                 << "     earl     :" << earl         << endl
                                 << "     c.url    :" << c->getURL()  << endl
                                 << "     child.url:" << child->getURL() << endl;
                            LG_CTX_D << "priv_read_leaf() url:" << child->getURL()
                                 << " c.is-dir:" << getStrAttr( c, "is-dir", "none" )
                                 << " child.is-dir:" << getStrAttr( child, "is-dir", "none" )
                                 << endl;
                            
                        }
                        catch( exception& e )
                        {
                            LG_CTX_W << "Error adding branch filesystem for:" << name << endl
                                     << " e:" << e.what() << endl;
                        }
                    }
                }
            }
        
    public:

        FerrisAllBranchesContext( Context* theParent,
                                  const fh_context& theDelegate,
                                  const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~FerrisAllBranchesContext()
            {
            }
    };

    FERRISEXP_DLLLOCAL FerrisBranchInternalContext*
    FerrisAllBranchesContext_Creator( Context* ctx,
                                      const fh_context& theDelegate,
                                      const std::string& rdn )
    {
        return new FerrisAllBranchesContext( ctx, theDelegate, rdn );
    }
    
    static bool FerrisAllBranchesContext_Dropper =
    FerrisBranchRootContext_Register( "branches",
                                      BranchInternalContextCreatorFunctor_t(
                                          FerrisAllBranchesContext_Creator ) );
    
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
        
};
