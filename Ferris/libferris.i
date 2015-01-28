/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferris.i,v 1.3 2006/08/13 11:39:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


%{
#include <string>
#include <vector>
#include <list>
#include <TypeDecl.hh>
#include <Ferris.hh>
#include <Agent.hh>
#include <Medallion.hh>
#include <FerrisStreams/Exceptions.hh>
#include <FerrisStreams/Shell.hh>
#include <FerrisStreams/Streams.hh>
#include <xfsutil.hh>
#include <Trimming.hh>
#include <Runner.hh>
#include <FCA.hh>
#ifdef Copy
#undef Copy
#endif
#include <ClipAPI.hh>
#include <CursorAPI.hh>
#include <FerrisOpenSSL.hh>
#include <FerrisException.hh>
#include <Attribute.hh>
#include <Ferris.hh>

    using namespace Ferris;
    
%}

namespace Ferris
{
};
%ignore getConMAP();
%ignore getMAP();
%ignore ContextSetCompare;
%ignore RelativeTimeOrIntegerStringParser;
%ignore ContextStreamMemoryManager;
%ignore getCacheManagerImpl();


%ignore Ferris::Context::tryToGetImplicitTreeSmush();
%ignore Ferris::Data( const ContextIterator* ci );
%ignore Ferris::Data();

%include <HiddenSymbolSupport.hh>
%include stl.i
%template(IntVector)      std::vector<int>;
%template(StringVector)   std::vector<std::string>;


%ignore Ferris::Nocase;
%ignore Ferris::ferris_ios::o_direct;
%ignore Ferris::ferris_ios::o_mmap;
%ignore Ferris::ferris_ios::o_mseq;
%ignore Ferris::ferris_ios::all_mask;

%ignore FerrisLoki::Handlable::ref_count;
%ignore FerrisLoki::Handlable::GenericCloseSignal;


%include <FerrisLoki/Extensions.hh>
%include <FerrisStreams/Exceptions.hh>
%include <FerrisStreams/Streams.hh>
%include <FerrisStreams/Shell.hh>

%nodefault;
namespace Ferris
{
    %name(Context) class fh_context
        { public:
            Ferris::Context* operator->();
        };
    %name(AttributeProxy) class fh_attribute
        { public:
            Ferris::AttributeProxy* operator->();
        };
    %name(AttributeCollection) class fh_attrcol
        { public:
            Ferris::AttributeCollection* operator->();
        };
    %name(Runner) class fh_runner
        { public:
            Ferris::Runner* operator->();
        };
    %name(PreprocessedSortString) class fh_sorter
        { public:
            Ferris::PreprocessedSortString* operator->();
        };
    %name(Emblem) class fh_emblem
        { public:
            Ferris::Emblem* operator->();
        };
    %name(ColdEmblem) class fh_cemblem
        { public:
            Ferris::ColdEmblem* operator->();
        };
    %name(Etagere) class fh_etagere
        { public:
            Ferris::Etagere* operator->();
        };
    %name(Medallion) class fh_medallion
        { public:
            Ferris::Medallion* operator->();
        };
    %name(MedallionBelief) class fh_medallionBelief
        { public:
            Ferris::MedallionBelief* operator->();
        };

    namespace FCA
    {
	    %name(ConceptLattice) class fh_conceptLattice
        	{ public:
	            Ferris::FCA::ConceptLattice* operator->();
        	};
     };
};
%ignore Ferris::Context;
%ignore Ferris::AttributeProxy;
%ignore Ferris::AttributeCollection;
%ignore Ferris::Runner;
%ignore Ferris::PreprocessedSortString;
%ignore Ferris::Emblem;
%ignore Ferris::ColdEmblem;
%ignore Ferris::Etagere;
%ignore Ferris::Medallion;
%ignore Ferris::MedallionBelief;
%ignore Ferris::FCA::ConceptLattice;


%ignore getModeFromMetaData( fh_context );
%ignore CreationStatelessFunctor;

%ignore RegisterCreationModule( const std::string& libname,
                                 const std::string& ferristype,
                                 const std::string& xsd,
                                 bool requiresNativeKernelDrive = true,
                                 const std::string& simpleTypes = "" );
%ignore appendExtraGenerateSchemaSimpleTypes( const std::string& s );
%ignore insertAllCreatorModules( Context::CreateSubContextSchemaPart_t& m );
%ignore insertAbstractCreatorModules( Context::CreateSubContextSchemaPart_t& m );

%ignore getCachedContextAggregateData( fh_context c, int m );
%ignore SL_SubCreate_file( fh_context c, fh_context md );
%ignore SL_SubCreate_file( fh_context c, fh_context md );
%ignore SL_SubCreate_ea( fh_context c, fh_context md );
%ignore SL_SubCreate_text ( fh_context c, fh_context md );
%ignore SL_edb_SubCreate_dir( fh_context c, fh_context md );
%ignore SL_edb_SubCreate_file( fh_context c, fh_context md );
%ignore SL_ipc_SubCreate_file( fh_context c, fh_context md );
%ignore SL_ipc_sem_SubCreate_file( fh_context c, fh_context md );
%ignore SL_db4_SubCreate_dir( fh_context c, fh_context md );
%ignore SL_db4_SubCreate_file( fh_context c, fh_context md );
%ignore Ferrisls_aggregate_data;

%ignore  Ferris::getStrAttr( AttributeCollection* c,
                             const std::string& rdn,
                             const std::string& def,
                             bool getAllLines,
                             bool throw_for_errors );

%ignore Ferris::RegexCollection::getRegex();
%ignore Ferris::RegexCollection::getRegex() const;
%ignore Ferris::RegexCollection::clear();
%ignore toStreamChar( wchar_t ch );
%ignore toStreamChar( char ch );
%ignore tryToGetImplicitTreeSmushSet();


namespace Ferris
{
    %ignore  Handlable;
    %ignore  CacheHandlable::OnlyInCacheSignal;
};
%include <TypeDecl.hh>
%include <FerrisException.hh>
%include <Cache.hh>
%include <Versioned.hh>
%include <Attribute.hh>






%include <FerrisSTL.hh>
%include <Shell.hh>
%include <General.hh>
%include <Agent.hh>
%include <Medallion.hh>
%include <Personalities.hh>
%include <xfsutil.hh>
%include <Trimming.hh>
%include <SignalStreams.hh>
%include <SchemaSupport.hh>

%ignore Ferris::Runner::setAsyncStdOutFunctor;
%ignore Ferris::Runner::internal_async_cb;
%include <Runner.hh>

%include <Resolver.hh>
%include <Mime.hh>

 //%include <FCA.hh>
%include <ClipAPI.hh>
%include <CursorAPI.hh>
%include <FerrisOpenSSL.hh>

%ignore Ferris::CacheManager::getInsideResolve();
%ignore Ferris::Context::priv_isSubContextBound( const std::string& rdn, Items_t::iterator& iter );
%ignore Ferris::Context::Emit_MedallionUpdated();
%ignore Ferris::Context::Emit_Changed( NamingEvent_Changed* e, 
                                       const std::string& olddn,
                                       const std::string& newdn, Object* ExtraData);
%ignore Ferris::Context::Emit_Changed( NamingEvent_Changed* e, 
                                       const std::string& olddn,
                                       const std::string& newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Deleted( NamingEvent_Deleted* e,
                                       std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Start_Execute( NamingEvent_Start_Execute* e,
                                             std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Stop_Execute( NamingEvent_Stop_Execute* e,
                                            std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Created( NamingEvent_Created* e,
                                       const fh_context& newc,
                                       std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Moved( NamingEvent_Moved* e,
                                     std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Exists( NamingEvent_Exists* e,
                                      const fh_context& newc,
                                      std::string olddn, std::string newdn, Object* ExtraData );
%ignore Ferris::Context::Emit_Start_Reading_Context( NamingEvent_Start_Reading_Context* e,
                                                     Object* ExtraData );
%ignore Ferris::Context::Emit_Stop_Reading_Context( NamingEvent_Stop_Reading_Context* e,
                                                    Object* ExtraData );
%ignore Ferris::Context::getNamingEvent_MedallionUpdated_Sig();
%ignore Ferris::Context::getNamingEvent_Changed_Sig();
%ignore Ferris::Context::getNamingEvent_Deleted_Sig();
%ignore Ferris::Context::getNamingEvent_Start_Execute_Sig();
%ignore Ferris::Context::getNamingEvent_Stop_Execute_Sig();
%ignore Ferris::Context::getNamingEvent_Created_Sig();
%ignore Ferris::Context::getNamingEvent_Moved_Sig();
%ignore Ferris::Context::getNamingEvent_Exists_Sig();
%ignore Ferris::Context::getNamingEvent_Start_Reading_Context_Sig();
%ignore Ferris::Context::getNamingEvent_Stop_Reading_Context_Sig();
%ignore Ferris::Context::getContextEvent_Headers_Received_Sig();
%ignore Ferris::Context::getAttributeNames( AttributeNames_t& ret );
%ignore insertCreatorModules( Context::CreateSubContextSchemaPart_t& m, bool );
%ignore DepthFirstDeletePCCTS_DropInOderList( childContext* cc );
%ignore DepthFirstDelete( Context* cc, bool callReclaimContextObject = true );
%ignore DepthFirstDelete( Context* cc, std::list< Context* >& l, bool callReclaimContextObject = true );
%ignore Ferris::Context::DepthFirstDeletePCCTS_DropInOderList( childContext* cc );
%ignore Ferris::Context::DepthFirstDelete( Context* cc, bool callReclaimContextObject = true );
%ignore Ferris::Context::DepthFirstDelete( Context* cc, std::list< Context* >& l, bool callReclaimContextObject = true );
%ignore AppendAllStaticEAGeneratorFactories_Stateless( Context::s_StatelessEAGenFactorys_t& SL );
%ignore AppendAllStaticEAGeneratorFactories_Statefull( Context::m_StatefullEAGenFactorys_t& SF );

%ignore Ferris::ContextIterator::ContextIterator( fh_context c, const std::string& rdn );
%ignore Ferris::ContextIterator::ContextIterator();


%include <ContextIterator.hh>
%include <Ferris.hh>








