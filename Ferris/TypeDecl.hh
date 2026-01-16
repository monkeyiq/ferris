/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: TypeDecl.hh,v 1.18 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_TYPEDECL_H_
#define _ALREADY_INCLUDED_TYPEDECL_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <set>
#include <map>
#include <list>
#include <string>

#include <FerrisLoki/Extensions.hh>
#include <Ferris/Debug.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/FerrisSmartPointerChecker.hh>

#include <boost/regex/v4/regex_fwd.hpp>

/**
 * NOTE TO SELF: The macros that define the smart pointer typedefs are
 * wrapped in #ifndef blocks so that cedet can use its own personal
 * definition of those macros which points to Loki::CedetSmartPtr<>
 * instead of the real class. This keeps completion happy in emacs,
 * and the code compiles as normal when not being inspected by emacs.
 */
namespace Ferris
{
    typedef Loki::SmartPtr< ::boost::regex, 
                            Loki::RefLinked, 
                            Loki::DisallowConversion, 
                            FerrisLoki::FerrisExSmartPointerChecker, 
                            Loki::DefaultSPStorage >  fh_rex;

struct RegexCollectionPriv;
    class FERRISEXP_API RegexCollection
    {
        typedef Loki::SmartPtr< RegexCollectionPriv, 
                                Loki::RefLinked, 
                                Loki::AllowConversion, 
                                FerrisLoki::FerrisExSmartPointerChecker, 
                                Loki::DefaultSPStorage >  fh_pdata;
        mutable fh_pdata m_priv;

    public:
        explicit RegexCollection( bool caseSensitive = true );
        ~RegexCollection();
        void append( const std::string& s );
        void append( const std::list< std::string >& s );
        void clear();

        fh_rex getRegex() const;
    };

    namespace FullTextIndex 
    {
        /**
         * Settings for what stemming to perform on tokens in the lexicon
         */
        enum StemMode
        {
            STEM_J_B_LOVINS_68 = 1<<1, // ferris indexing only
            STEM_PORTER        = 1<<2, // lucene indexing only
            STEM_NONE          = 1<<16,
        };
    };
    
    
    // For classes that shouldn't be copied this will disallow
    // assignment and copy ctor at compile time
    // Use it inside class private part.
#define NOT_COPYABLE(EnclosingClassName)                             \
        EnclosingClassName( const EnclosingClassName&  );            \
        EnclosingClassName& operator=( const EnclosingClassName&  )

#ifndef FERRIS_SMARTPTR
#define FERRIS_SMARTPTR( ClassName, HandleName )                        \
    typedef Loki::SmartPtr< ClassName,                                  \
                            FerrisLoki::FerrisExRefCounted,             \
                            Loki::DisallowConversion,                   \
                            FerrisLoki::FerrisExSmartPointerChecker,    \
                            FerrisLoki::FerrisExSmartPtrStorage >  HandleName;
#endif

#ifndef FERRIS_NIREF
#define FERRIS_NIREF( ClassName, HandleName )                        \
    typedef Loki::SmartPtr< ClassName,                               \
                            Loki::RefLinked,                         \
                            Loki::DisallowConversion,                \
                            FerrisLoki::FerrisExSmartPointerChecker, \
                            Loki::DefaultSPStorage >  HandleName
#endif

    class FERRISEXP_API Handlable : public FerrisLoki::Handlable 
    {
    public:
        Handlable()
            {
            }
        virtual ~Handlable()
            {
            }
    };

// #define FerrisRefCounted          FerrisLoki::FerrisExRefCounted
// #define FerrisSmartPointerChecker FerrisLoki::FerrisExSmartPointerChecker
// #define FerrisSmartPtrStorage     FerrisLoki::FerrisExSmartPtrStorage
//     typedef FerrisLoki::Handlable Handlable;
    
//     typedef FerrisLoki::FerrisExRefCounted            FerrisRefCounted;
//     typedef FerrisLoki::FerrisExSmartPointerChecker   FerrisSmartPointerChecker;
//     typedef FerrisLoki::FerrisExSmartPtrStorage       FerrisSmartPtrStorage;

    /**
     * every emblem has a unique numeric identifier
     */
    typedef guint32 emblemID_t;
    
    class Attribute;
    class AttributeCollection;
    class IContext;
    class Context;
    class ContextIterator;
    class NativeContext;
    class SelectionFactoryContext;
    class RootContext;
    class NamingEvent;
    class RootContextFactory;
    class RootContextDropper;
    class CacheManager;
    class ContextVFS_RootContextDropper;
    namespace Private {
        class CacheManagerContextStateInTime;
    };
    namespace FCA {
        class DatabaseResultSetContext;
        class LatticeRootConceptVFS_RootContextDropper;
    };
    class EAGenerator;
    class Versioned;
    class Digest_EAGenerator;
    class Head_EAGenerator;
    class HeadRadix_EAGenerator;
    class MatchedEAGeneratorFactory;
    class CreateMetaDataContext;
    class DelegatingCreateMetaDataContext;
    class FerrisExceptionBase;
    class childContext;
    class SortedContext;
    class Runner;
    class AttributeProxy;
    class PluginOutOfProcNotificationEngine;
    class CreationStatelessFunctor;
    class Ferrisls_aggregate_data;
    class PreprocessedSortString;
    class AsyncIOHandler;
    class XMLMsgStreamCollector;
    class Personality;
    FERRIS_SMARTPTR( Personality, fh_personality );
    class ChildStreamServer;
    class FerrisSlaveProcess;
    FERRIS_SMARTPTR( FerrisSlaveProcess,     fh_FerrisSlaveProcess );
    
    typedef CreateMetaDataContext f_mdcontext;

    class Image;
    FERRIS_SMARTPTR( Image,          fh_image );

    class MetadataWorker;
    class MetadataBrokerDispatchInformation;
    
    
    /**
     * SmartPtr<> policy class [OwnershipPolicy] for Handlable objects.
     * This class implements a intrusive reference count like the COMRefCounted
     * policy in the Modern C++ design book.
     */
    template <class P>
    class FerrisExRefCountedContext
    {
    public:
    
        FerrisExRefCountedContext() {}
        FerrisExRefCountedContext( const FerrisExRefCountedContext& r) {}
        template <class U> FerrisExRefCountedContext(const FerrisExRefCountedContext<U>&) {}

        /**
         * Create a new handle
         */
        P Clone(const P& val)
            {
                if( val )
                {
                    val->AddRef();
                }
                return val;
            }
        
        /**
         * Release a reference. This may trigger a getClosureSignal() to fire due to
         * the final reference being dropped.
         *
         * @param val Object that we are releasing an intrusive reference to
         * @return true if the object should die.
         */
        bool Release(const P& val)
            {
                if( !val )
                {
                    return false;
                }
            
                int v = val->::Ferris::Handlable::AddRef();
                if( v == 2 )
                {
                    val->private_AboutToBeDeleted();
                }
                v = val->::Ferris::Handlable::Release();
                
                v = val->Release();
                if( !v )
                {
                    /*
                     * Time to die amigo
                     */
                    return true;
                }

                return false;
            }
        
        enum { destructiveCopy = false };
        static void Swap(FerrisExRefCountedContext&)
            {}
    };
    template <class T,
              class CONVERTER,
              template <class> class CHECKER>
    inline bool isBound( const Loki::SmartPtr< T,
                         ::Ferris::FerrisExRefCountedContext, 
                         CONVERTER,
                         CHECKER, 
                         FerrisLoki::FerrisExSmartPtrStorage > & sp)
    {
        return GetImpl(sp) != 0 ;
    }

    FERRIS_SMARTPTR( AttributeProxy,          fh_attribute );
#ifndef FERRIS_CTX_SMARTPTR
#define FERRIS_CTX_SMARTPTR( ClassName, HandleName )  \
    typedef Loki::SmartPtr< ClassName,            \
                            ::Ferris::FerrisExRefCountedContext, \
                            Loki::DisallowConversion,            \
                            FerrisLoki::FerrisExSmartPointerChecker,    \
                            FerrisLoki::FerrisExSmartPtrStorage >  HandleName;
#endif
    FERRIS_CTX_SMARTPTR( Context, fh_context );
//    FERRIS_SMARTPTR( Context,                 fh_context   );
    
    FERRIS_CTX_SMARTPTR( CreateMetaDataContext,   fh_mdcontext );
    FERRIS_CTX_SMARTPTR( AttributeCollection,     fh_attrcol   );
    FERRIS_SMARTPTR( Runner,                  fh_runner    );
    FERRIS_SMARTPTR( Ferrisls_aggregate_data, fh_display_aggdata );
    FERRIS_SMARTPTR( PreprocessedSortString,  fh_sorter    );
    typedef PreprocessedSortString f_sorter;   
    typedef std::list<fh_sorter>   fh_sorters; 

    FERRIS_SMARTPTR( AsyncIOHandler,          fh_aiohandler    );
    FERRIS_SMARTPTR( XMLMsgStreamCollector,   fh_xstreamcol    );


    class Emblem;
    class ColdEmblem;
    class Etagere;
    class Medallion;
    class MedallionBelief;
    FERRIS_SMARTPTR( Emblem,     fh_emblem );
    FERRIS_SMARTPTR( ColdEmblem, fh_cemblem );
    FERRIS_SMARTPTR( Etagere,    fh_etagere );
    FERRIS_SMARTPTR( Medallion,       fh_medallion );
    FERRIS_SMARTPTR( MedallionBelief, fh_medallionBelief );
    
    class Regex;
    FERRIS_SMARTPTR( Regex, fh_regex );

    typedef std::list< std::string > stringlist_t;
    typedef std::vector< std::string > stringvec_t;
    typedef std::set< std::string >  stringset_t;
    typedef std::map< std::string, std::string > stringmap_t;
    typedef std::map< int, int > intmap_t;

    typedef std::list< fh_context > ctxlist_t;
    typedef std::pair< std::string, std::string > userpass_t;
    typedef std::pair< std::string, std::string > stringpair_t;


//     FERRIS_SMARTPTR( MatchedEAGeneratorFactory, fh_MatchedEAGeneratorFactory );
//     typedef std::list< fh_MatchedEAGeneratorFactory > EAGenFactorys_t;


    namespace EAIndex 
    {
        // These two need to know about each other in header files so
        // that eaquery can make EAIndexManagerDB4 a friend and
        // EAIndexManagerDB4 can call a method on fh_eaquery
        // to continue processing the query using the db4 based indexing.
        class EAQuery;
        FERRIS_SMARTPTR( EAQuery, fh_eaquery );
        class EAIndexManagerDB4;
        FERRIS_SMARTPTR( EAIndexManagerDB4, fh_db4idx );
        class EAIndexManagerDB4Tree;
        FERRIS_SMARTPTR( EAIndexManagerDB4Tree, fh_db4treeidx );
        class FERRISEXP_API ForwardEAIndexInterface;
        FERRIS_SMARTPTR( ForwardEAIndexInterface, fh_fwdeaidx );
    };

    namespace redlandea
    {
        class SmushSet;
        class TreeSmushing;
        FERRIS_SMARTPTR( SmushSet,     fh_SmushSet );
        FERRIS_SMARTPTR( TreeSmushing, fh_TreeSmushing );
    };

    namespace Semantic
    {
        class SmushSet;
        class TreeSmushing;
        struct tryToGetUUIDNode_StringARM;
        FERRIS_SMARTPTR( SmushSet,     fh_SmushSet );
        FERRIS_SMARTPTR( TreeSmushing, fh_TreeSmushing );
    };
    
};

//#ifdef SIGCXX_HAS_LESS_THAN_FIVE_SIGNAL_ARGS_AS_MAX
#include <sigc++/sigc++.h>

// Using sigc++ 2.x
#ifdef _SIGC_REFERENCE_WRAPPER_H_
#define LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS
#endif

#define LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS

#ifndef LIBFERRIS__DONT_INCLUDE_SIGCC_EXTENSIONS
namespace SigC
{
    
/// @ingroup Signals
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Marsh=Marshal<R> >
class Signal6 : public SignalBase
  {
    public:
      typedef Slot6<R,P1,P2,P3,P4,P5,P6> InSlotType;
      typedef Slot6<typename Marsh::OutType,P1,P2,P3,P4,P5,P6> OutSlotType;
      typedef typename Trait<typename Marsh::OutType>::type OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(
          typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,
          typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,
          typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,
          void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5, typename Trait<P6>::ref p6)
        { return emit_(p1,p2,p3,p4,p5,p6,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5, typename Trait<P6>::ref p6)
        { return emit_(p1,p2,p3,p4,p5,p6,impl_); }
 
      Signal6() 
        : SignalBase() 
        {}

      Signal6(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal6() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
typename Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::OutType
Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::emit_(
    typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,
    typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,
    typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,
    void* data)
{
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl || !impl->begin_)
      return Marsh::default_value();

    Exec exec(impl);
    Marsh rc;
    SlotNode* s = 0;

    for (SignalConnectionNode* i = impl->begin_; i; i=i->next_)
      {
        if (i->blocked()) continue;
        s = i->dest();
        if (rc.marshal(((typename Slot5<R,P1,P2,P3,P4,P5>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,s)))
          return rc.value();
      }
    return rc.value();
  }

    
/// @ingroup Signals
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
class Signal6<void,P1,P2,P3,P4,P5,P6,Marsh> : public SignalBase
  {
    public:
      typedef Slot6<void,P1,P2,P3,P4,P5,P6> InSlotType;
      typedef InSlotType OutSlotType;
      typedef void OutType;
 
    private:
      // Used for both emit and proxy.
      static OutType emit_(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,void* data);

    public:
      OutSlotType slot() const
        { return create_slot((FuncPtr)(&emit_)); }

      operator OutSlotType() const
        { return create_slot((FuncPtr)(&emit_)); }

      /// You can call Connection::disconnect() later.
      Connection connect(const InSlotType& s)
        { return Connection(push_back(s)); }

      /// Call all the connected methods.
      OutType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
        {  emit_(p1,p2,p3,p4,p5,p6,impl_); }

      /// See emit()
      OutType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5, typename Trait<P6>::ref p6)
        {  emit_(p1,p2,p3,p4,p5,p6,impl_); }
 
      Signal6() 
        : SignalBase() 
        {}

      Signal6(const InSlotType& s)
        : SignalBase() 
        { connect(s); }

      ~Signal6() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
void Signal6<void,P1,P2,P3,P4,P5,P6,Marsh>::emit_(
    typename Trait<P1>::ref p1,
    typename Trait<P2>::ref p2,
    typename Trait<P3>::ref p3,
    typename Trait<P4>::ref p4,
    typename Trait<P5>::ref p5,
    typename Trait<P6>::ref p6,
    void* data)
{
    SignalNode* impl = static_cast<SignalNode*>(data);

    if (!impl||!impl->begin_)
      return;

    Exec exec(impl);
    SlotNode* s = 0;
    for (SignalConnectionNode* i = impl->begin_; i; i = i->next_)
      {
        if (i->blocked())
          continue;

        s = i->dest();
        ((typename Slot6<void,P1,P2,P3,P4,P5,P6>::Proxy)(s->proxy_))(p1,p2,p3,p4,p5,p6,s);
      }
    return;
  }
};
//#endif //SIGCXX_HAS_LESS_THAN_FIVE_SIGNAL_ARGS_AS_MAX
#endif

#endif // ifndef _ALREADY_INCLUDED_TYPEDECL_H_
