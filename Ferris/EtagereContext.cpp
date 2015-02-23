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

    $Id: EtagereContext.cpp,v 1.7 2010/09/24 21:30:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <EtagereContext_private.hh>
#include <Resolver_private.hh>
#include <EAQuery.hh>
#include <SchemaSupport.hh>
using namespace std;

namespace Ferris
{
    
    static const char* const emblem_XSD =
    "	<elementType name=\"file\">\n"
    "		<elementType name=\"name\" default=\"new emblem\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "		<elementType name=\"iconpath\" default=\"\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "		<elementType name=\"description\" default=\"\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "	</elementType>\n";
    static const char* const emblem_dir_XSD =
    "	<elementType name=\"dir\">\n"
    "		<elementType name=\"name\" default=\"new emblem\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "		<elementType name=\"iconpath\" default=\"\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "		<elementType name=\"description\" default=\"\">\n"
    "			<dataTypeRef name=\"string\"/>\n"
    "		</elementType>\n"
    "	</elementType>\n";

    std::pair< fh_context, fh_emblem >
    EmblemCommonCreator::SubCreate_emblem( fh_context c, fh_context md, bool isRoot )
    {
        string rdn         = getStrSubCtx( md, "name", "" );
        string iconpath    = getStrSubCtx( md, "iconpath", "" );
        string description = getStrSubCtx( md, "description", "" );

        if( !rdn.length() )
        {
            fh_stringstream ss;
            ss << "Attempt to create file with no name" << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }

        if( c->priv_isSubContextBound( rdn ) )
        {
            fh_stringstream ss;
            ss << "Attempt to create file that exists. rdn:" << rdn
               << " at url:" << c->getURL()
               << endl;
            Throw_ContextExists( tostr(ss), GetImpl(c) );
        }

        LG_EMBLEM_D << "EmblemCommonCreator::SubCreate_emblem(1) c:" << c->getURL()
                    << " rdn:" << rdn
                    << " iconpath:" << iconpath
                    << " desc:" << description
                    << endl;
        fh_etagere et    = Factory::getEtagere();
        fh_emblem  em    = et->createColdEmblem( rdn );

        em->setIconName( iconpath );
        em->setDescription( description );

        c->bumpVersion();

        if( isRoot )
        {
            // OnChildAdded() will have been fired on the above createColdEmblem()
            // so we only need to find it now for return
            fh_context child = c->getSubContext( rdn );
            return make_pair( child, em );
        }
        
        LG_EMBLEM_D << "EmblemCommonCreator::SubCreate_emblem(2) c:" << c->getURL() << endl;
        bool m_showFilesAsLeaves = false;
        if( EmblemContext* c_downcast = dynamic_cast<EmblemContext*>(GetImpl(c)))
            m_showFilesAsLeaves = c_downcast->m_showFilesAsLeaves;
        
        EmblemContext* child = new EmblemContext( GetImpl(c), em, m_showFilesAsLeaves );
        c->Insert( child, false, false );
        LG_EMBLEM_D << "EmblemCommonCreator::SubCreate_emblem(3) c:" << c->getURL() << endl;
        return make_pair( child, em );
    }
    
    void
    EtagereRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["file"] = SubContextCreator( SL_SubCreate_file, emblem_XSD );
        m["dir"]  = SubContextCreator( SL_SubCreate_file, emblem_dir_XSD );
    }

    fh_context
    EtagereRootContext::SubCreate_file( fh_context c, fh_context md )
    {
        string rdn         = getStrSubCtx( md, "name", "" );
        LG_EMBLEM_D << "EtagereRootContext::SubCreate_file(top) rdn:" << rdn << endl;
        std::pair< fh_context, fh_emblem > p = SubCreate_emblem( c, md, true );

        fh_etagere et = Factory::getEtagere();
        et->sync();

        return p.first;
    }
    
    void
    EtagereRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            fh_context child = 0;
            fh_etagere et    = Factory::getEtagere();
            emblems_t  el    = et->getAllEmblems();
    
            for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
            {
                fh_emblem em = *ei;
                LG_EMBLEM_D << "--- em:" << em->getName() << " parent.empty:" << em->getParents().empty() << endl;

//                 // PURE DEBUG
//                 {
//                     emblems_t pl = em->getParents();
//                     for( emblems_t::iterator pi = pl.begin(); pi != pl.end(); ++pi )
//                     {
//                         fh_emblem p = *pi;
//                         LG_EMBLEM_D << "em:" << em->getName() << " p:" << p->getName() << endl;
//                     }
//                 }
                
                if( em->getParents().empty() )
                {
                    LG_EMBLEM_D << "adding em:" << em->getName()
                                << " UName:" << em->getUniqueName()
                                << " parent.empty:" << em->getParents().empty()
                                << endl;
                    child = new EmblemContext( this, em, m_showFilesAsLeaves );
                    Insert( GetImpl(child), false, true );
                }
            }
        }
    }
    
        

    EtagereRootContext::EtagereRootContext( bool m_showFilesAsLeaves )
        :
        _Base( 0, "/" ),
        m_showFilesAsLeaves( m_showFilesAsLeaves )
    {
        createStateLessAttributes();

        fh_etagere et = Factory::getEtagere();
        et->getAddedChild_Sig().connect(   sigc::mem_fun( *this, &_Self::OnChildAdded   ) );
        et->getRemovedChild_Sig().connect( sigc::mem_fun( *this, &_Self::OnChildRemoved ) );
    }
    
    EtagereRootContext::~EtagereRootContext()
    {
    }
    
    void
    EtagereRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    bool
    EtagereRootContext::supportsRemove()
    {
        return true;
    }
    
    void
    EtagereRootContext::priv_remove( fh_context c )
    {
        fh_etagere et = Factory::getEtagere();

        if( EmblemContext* ec = dynamic_cast<EmblemContext*>( GetImpl(c) ) )
        {
            // For an emblem to appear at the top level it has no parents
            // so deletion means that we really want to purge the emblem itself.
            if( !ec->m_em->getParents().empty() )
            {
                fh_stringstream ss;
                ss << "SHOULD NEVER HAPPEN. "
                   << "etagere has a direct child with parents url:" << c->getURL();
                Throw_CanNotDelete( tostr(ss), GetImpl(c) );
            }
            
            et->erase( ec->m_em );
            ec->m_em = 0;
            et->sync();
        }
        else
        {
            fh_stringstream ss;
            ss << "SHOULD NEVER HAPPEN. "
               << " etagere has a non emblem child! url:" << c->getURL();
            Throw_CanNotDelete( tostr(ss), GetImpl(c) );
        }
    }

    std::string
    EtagereRootContext::priv_getRecommendedEA()
    {
        return "name,description,latitude,longitude,zoom";
    }
    
    void
    EtagereRootContext::OnChildAdded( fh_etagere et, fh_emblem em )
    {
        if( !priv_isSubContextBound( em->getName() ) )
        {
            fh_context child = new EmblemContext( this, em, m_showFilesAsLeaves );
            Insert( GetImpl(child), false, false );
            bumpVersion();
        }
    }
    
    void
    EtagereRootContext::OnChildRemoved( fh_etagere et, fh_emblem em )
    {
        if( priv_isSubContextBound( em->getName() ) )
        {
            Remove( em->getName() );
            bumpVersion();
        }
    }
    

    class FERRISEXP_DLLLOCAL EtagereRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        EtagereRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("etagere");
                RootContextFactory::Register( "etagere", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new EtagereRootContext();
                }
                return c;
            }
    };
    static EtagereRootContext_RootContextDropper ___EtagereRootContext_static_init;


    class FERRISEXP_DLLLOCAL EmblemQueryEtagereRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        EmblemQueryEtagereRootContext_RootContextDropper()
            {
                RootContextFactory::Register( "emblemquery", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new EtagereRootContext( true );
                }
                return c;
            }
    };
    static EmblemQueryEtagereRootContext_RootContextDropper ___EQEtagereRootContext_static_init;
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    EmblemContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["file"] = SubContextCreator( SL_SubCreate_file, emblem_XSD );
        m["dir"]  = SubContextCreator( SL_SubCreate_file, emblem_dir_XSD );
    }

    fh_context
    EmblemContext::SubCreate_file( fh_context c, fh_context md )
    {
        std::pair< fh_context, fh_emblem > p = SubCreate_emblem( c, md, false );
        link( m_em, p.second );

        fh_etagere et = Factory::getEtagere();
        et->sync();

        LG_EMBLEM_D << "EmblemContext::SubCreate_file() m_em:" << m_em->getID() << endl;
        LG_EMBLEM_D << "EmblemContext::SubCreate_file() newem:" << p.second->getID() << endl;
        LG_EMBLEM_D << "EmblemContext::SubCreate_file() new.parent.sz:" << p.second->getParents().size() << endl;
        
        return p.first;
    }

    bool
    EmblemContext::supportsRemove()
    {
        return true;
    }
    
    void
    EmblemContext::priv_remove( fh_context c )
    {
        fh_etagere et = Factory::getEtagere();

        if( EmblemContext* ec = dynamic_cast<EmblemContext*>( GetImpl(c) ) )
        {
            // break the emblem parent/child link
            unlink( m_em, ec->m_em );
            et->sync();
        }
        else
        {
            fh_stringstream ss;
            ss << "SHOULD NEVER HAPPEN. "
               << " emblem has a non emblem child! url:" << c->getURL();
            Throw_CanNotDelete( tostr(ss), GetImpl(c) );
        }
    }


    bool
    EmblemContext::supportsRename()
    {
        return true;
    }
    
    fh_context
    EmblemContext::priv_rename( const std::string& rdn,
                                const std::string& newPath,
                                bool TryToCopyOverFileSystems,
                                bool OverWriteDstIfExists )
    {
//        string newPath = appendToPath( getURL(), newPathRelative, true );
        LG_CTX_D << "EmblemContext::priv_rename() called" << endl;
        
        fh_etagere et = Factory::getEtagere();

        fh_context src = getSubContext( rdn );
        if( EmblemContext* srcc = dynamic_cast<EmblemContext*>( GetImpl(src) ) )
        {
            LG_CTX_D << "EmblemContext::priv_rename() srcc:" << src->getURL() << endl;
            LG_CTX_D << "EmblemContext::priv_rename() newPath:" << newPath << endl;
            fh_context dst = 0;
            try
            {
                string ppath = newPath.substr( 0, newPath.rfind('/') );
                LG_CTX_D << "EmblemContext::priv_rename() ppath:" << ppath << endl;
//                LG_CTX_D << "EmblemContext::priv_rename() newPathRelative:" << newPathRelative << endl;
                dst = Resolve( newPath, RESOLVE_PARENT );
            }
            catch( exception& e )
            {
                LG_CTX_D << "EmblemContext::priv_rename() e:" << e.what() << endl;
                throw;
            }
            
            LG_CTX_D << "EmblemContext::priv_rename() dst:" << dst->getURL() << endl;
            
            if( EmblemContext* dstc = dynamic_cast<EmblemContext*>( GetImpl(dst) ) )
            {
                LG_CTX_D << "EmblemContext::priv_rename() dstc:" << dstc->getURL() << endl;
                // unlink us and the old rdn
                unlink( m_em, srcc->m_em );

                // link the old rdn to its new parent.
                link( dstc->m_em, srcc->m_em );
                et->sync();
                
                return src;
            }
        }
        
        /* failed */
        fh_stringstream ss;
        ss << "Rename attempt failed. URL:" << getURL() << " src:" << rdn << " dst:" << newPath;
        Throw_RenameFailed( tostr(ss), this );
        
    }
    
    std::string
    EmblemContext::priv_getRecommendedEA()
    {
        return "name,description,latitude,longitude,zoom";
    }

    
    void
    EmblemContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );
        

//         LG_EMBLEM_D << "priv_read(top)"
//              << " url:" << getURL()
//              << endl;
        
        if( empty() )
        {
            emblems_t el = m_em->getChildren();
            LG_EMBLEM_D << "priv_read()"
                 << " url:" << getURL()
                 << " child.sz:" << el.size()
                 << " m_showFilesAsLeaves:" << m_showFilesAsLeaves
                 << endl;
            
            for( emblems_t::iterator ei = el.begin(); ei != el.end(); ++ei )
            {
                fh_emblem em = *ei;

                fh_context child = new EmblemContext( this, em, m_showFilesAsLeaves );
                Insert( GetImpl(child), false );
            }

            if( el.empty() && m_showFilesAsLeaves )
            {
                fh_stringstream qss;

                emblems_t upset = m_em->getUpset();
                for( emblems_t::iterator ei = upset.begin(); ei != upset.end(); ++ei )
                {
                    qss << "(" << "emblem:has-" << (*ei)->getName() << "==1)";
                }

                LG_EMBLEM_D << "query for files with the emblem is:" << tostr(qss) << endl;

                EAIndex::fh_eaquery eq = EAIndex::Factory::makeEAQuery( tostr( qss ) );
                fh_context  c = eq->execute();

                for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
                {
                    Insert( GetImpl(*ci), false );
                }
            }
        }
    }

    EmblemContext::EmblemContext( Context* parent, fh_emblem em, bool m_showFilesAsLeaves )
        :
        _Base( parent, em->getName() ),
        m_em( em ),
        m_showFilesAsLeaves( m_showFilesAsLeaves )
    {
        setContext( parent, monsterName( em->getName() ));
        createStateLessAttributes();
        m_em->getAddedChild_Sig().connect( sigc::mem_fun( *this, &_Self::OnChildAdded ) );
    }
    
    EmblemContext::~EmblemContext()
    {
    }

    fh_istream
    EmblemContext::SL_getFSID( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "10ea3380-83fe-4e12-a671-d2067a10fd76";
        return ss;
    }

    fh_istream
    EmblemContext::SL_getDescription( EmblemContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->m_em->getDescription();
        return ss;
    }

    fh_stringstream
    EmblemContext::SL_getDigitalLatitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->m_em->getDigitalLatitude();
        return ss;
    }

    void
    EmblemContext::sync()
    {
        cerr << "EmblemContext::sync()" << endl;
        fh_etagere et = Factory::getEtagere();
        et->sync();
    }

    bool isDouble( string s )
    {
        if( s.empty() )
            return 0;
        return 1;
    }
    
    
    void
    EmblemContext::SL_updateDigitalLatitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string s = StreamToString( ss );
        if( isDouble( s ) )
        {
//             cerr << "EmblemContext::SL_updateDigitalLatitude() em:" << c->m_em->getName()
//                  << " s:" << s
//                  << " double:" << toType<double>(s)
//                  << endl;
            c->m_em->setDigitalLatitude( toType<double>(s) );
            c->sync();
        }
    }
    

    fh_stringstream
    EmblemContext::SL_getDigitalLongitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->m_em->getDigitalLongitude();
        return ss;
    }

    void
    EmblemContext::SL_updateDigitalLongitude( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string s = StreamToString( ss );
        if( isDouble( s ) )
        {
            c->m_em->setDigitalLongitude( toType<double>(s) );
            c->sync();
        }
    }
    
    fh_stringstream
    EmblemContext::SL_getZoomRange( EmblemContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->m_em->getZoomRange();
        return ss;
    }

    void
    EmblemContext::SL_updateZoomRange( EmblemContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
    {
        string s = StreamToString( ss );
        if( isDouble( s ) )
        {
            c->m_em->setZoomRange( toType<double>(s) );
            c->sync();
        }
    }
    
    void
    EmblemContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            tryAddStateLessAttribute( "fs-id", _Self::SL_getFSID, FXD_BINARY );
            tryAddStateLessAttribute( "description", _Self::SL_getDescription,      XSD_BASIC_STRING );
            tryAddStateLessAttribute( "latitude",
                                      _Self::SL_getDigitalLatitude,
                                      _Self::SL_getDigitalLatitude,
                                      _Self::SL_updateDigitalLatitude,
                                      XSD_BASIC_DOUBLE );
            tryAddStateLessAttribute( "longitude",
                                      _Self::SL_getDigitalLongitude,
                                      _Self::SL_getDigitalLongitude,
                                      _Self::SL_updateDigitalLongitude,
                                      XSD_BASIC_DOUBLE );
            tryAddStateLessAttribute( "zoom",
                                      _Self::SL_getZoomRange,
                                      _Self::SL_getZoomRange,
                                      _Self::SL_updateZoomRange,
                                      XSD_BASIC_DOUBLE );
            
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    void
    EmblemContext::OnChildAdded( fh_emblem em, fh_emblem child )
    {
        if( em != m_em )
            return;
        
        if( !priv_isSubContextBound( child->getName() ) )
        {
            LG_EMBLEM_D << "OnChildAdded(ec) this:" << getURL()
                 << " child:" << child->getName()
                 << endl;

            // We might not be user referenced here, so make sure
            // that we don't trigger the memory management.
//            ValueBumpDrop<ref_count_t> bdobj( ref_count );
            
            EmblemContext* n = new EmblemContext( this, child, m_showFilesAsLeaves );
            Insert( n, false, false );
            bumpVersion();
        }
    }

    void
    EmblemContext::OnChildRemoved( fh_emblem em, fh_emblem child )
    {
        if( priv_isSubContextBound( child->getName() ) )
        {
            Remove( child->getName() );
            bumpVersion();
        }
    }
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
