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

    $Id: FerrisDevContext.cpp,v 1.4 2010/09/24 21:30:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisDevContext_private.hh>
#include <Resolver_private.hh>
#include <General.hh>
#include <SchemaSupport.hh>
#include <Ferris.hh>

using namespace std;

namespace Ferris
{
#define DUBCORE_DESCRIPTION "dc:description"
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    FerrisDevGeneratorContext::FerrisDevGeneratorContext( Context* parent, const char* rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    FerrisDevGeneratorContext::~FerrisDevGeneratorContext()
    {
    }
    
    void
    FerrisDevGeneratorContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL FerrisDevGeneratorContext_uuid
        :
        public FerrisDevGeneratorContext
    {
        typedef FerrisDevGeneratorContext_uuid _Self;
        typedef FerrisDevGeneratorContext _Base;
        
    protected:

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss << Util::makeUUID();
                return ss;
            }
        
        
    public:

        FerrisDevGeneratorContext_uuid( Context* parent, const char* rdn )
            :
            FerrisDevGeneratorContext( parent, rdn )
            {
            }
        
        virtual ~FerrisDevGeneratorContext_uuid()
            {
            }

        static fh_stringstream
        SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << "A file which generates a new UUID every time it is read";
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );

                    tryAddStateLessAttribute( DUBCORE_DESCRIPTION,
                                              &_Self::SL_getDesc,
                                              XSD_BASIC_STRING );
                }
            }
        
        
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    template< class T, const std::string& name >
    class FERRISEXP_DLLLOCAL FerrisDevGeneratorContext_intX
        :
        public FerrisDevGeneratorContext
    {
        typedef FerrisDevGeneratorContext_intX< T, name > _Self;
        
        T m_state;

        string getKeyPrefix()
            {
                return "ferrisdev-generator-int-";
            }
        
    protected:

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ++m_state;
                setConfigString( FDB_FERRISDEV,
                                 getKeyPrefix() + name,
                                 tostr( m_state ));
                ss << m_state;
                return ss;
            }
        
        
    public:

        FerrisDevGeneratorContext_intX( Context* parent, const char* rdn )
            :
            FerrisDevGeneratorContext( parent, rdn ),
            m_state( 0 )
            {
                m_state = toType<T>( getConfigString( FDB_FERRISDEV,
                                                      getKeyPrefix() + name,
                                                      "0" ));
            }
        
        virtual ~FerrisDevGeneratorContext_intX()
            {
            }

        static fh_stringstream
        SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << "A file which generates a new integer every time it is read."
                   << " The last value returned is persisted by libferris and the integer"
                   << " will eventually overflow and start again";
                return ss;
            }
        
        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );

                    tryAddStateLessAttribute( DUBCORE_DESCRIPTION,
                                              &_Self::SL_getDesc,
                                              XSD_BASIC_STRING );
                }
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    void
    FerrisDevRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }

    string exstr_64 = "64";
    string exstr_32 = "32";
    
    void
    FerrisDevRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            fh_context child = 0;
            fh_fcontext fakedir = 0;

            fakedir = new FakeInternalContext( this, "generators" );
            addNewChild( fakedir );
            
            child = new FerrisDevGeneratorContext_uuid( this, "uuid" );
            fakedir->addNewChild( child );
            child = new FerrisDevGeneratorContext_intX< guint64, exstr_64 >( this, "integer-64" );
            fakedir->addNewChild( child );
            child = new FerrisDevGeneratorContext_intX< guint32, exstr_32 >( this, "integer-32" );
            fakedir->addNewChild( child );
        }
    }
    
        

    FerrisDevRootContext::FerrisDevRootContext()
        :
        _Base( 0, "/" )
    {
        createStateLessAttributes();
    }
    
    FerrisDevRootContext::~FerrisDevRootContext()
    {
    }
    
    void
    FerrisDevRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    class FERRISEXP_DLLLOCAL FerrisDevRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        FerrisDevRootContext_RootContextDropper()
            {
                ImplementationDetail::appendToStaticLinkedRootContextNames("ferrisdev");
                RootContextFactory::Register( "ferrisdev", this );
            }

        fh_context Brew( RootContextFactory* rf )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new FerrisDevRootContext();
                }
                return c;
            }
    };
    static FerrisDevRootContext_RootContextDropper ___FerrisDevRootContext_static_init;

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


};
