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

    $Id: Attribute.hh,v 1.17 2010/09/24 21:30:24 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>

#include <TypeDecl.hh>
#include <Ferris.hh>
#include <Functor.h>
#include <AssocVector.h>
#include "Ferris/FerrisStdHashMap.hh"

#ifndef _ALREADY_INCLUDED_FERRIS_ATTRIBUTE_H_
#define _ALREADY_INCLUDED_FERRIS_ATTRIBUTE_H_

namespace Ferris 
{
    FERRISEXP_API void AdjustForOpenMode_Opening( fh_istream  ss, ferris_ios::openmode m );
    FERRISEXP_API void AdjustForOpenMode_Opening( fh_iostream ss, ferris_ios::openmode m );
    FERRISEXP_API void AdjustForOpenMode_Closing( fh_istream& ss, ferris_ios::openmode m, std::streamsize tellp );


    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    
    class Attribute;
    typedef Attribute* AttributePtr;

    class FERRISEXP_API Attribute
    //        :
    //        public Loki::SmallObject<>
    {
    private:
        
        // So that the proxy can set theParent and theDirName
        friend class AttributeProxy;

    public:

        typedef Context* Parent_t;
        friend class RootContextFactory;
        
    protected:

        void setAttributeContext( Parent_t parent );


        virtual fh_istream  priv_getIStream( ferris_ios::openmode m );
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m );
        
        virtual std::pair<std::string,std::string> splitPathAtStart( const std::string& s );
        virtual std::pair<std::string,std::string> splitPathAtEnd( const std::string& s );
        virtual bool canSplitPathAtStart( const std::string& s );
        virtual bool canSplitPathAtEnd( const std::string& s );
        virtual std::string trimLeadingSeps( const std::string& s );
        virtual std::string trimTrailingSeps( const std::string& s );
        virtual std::string trimEdgeSeps( const std::string& s );
        
        const std::string& getSeperator();

        virtual void RegisterStreamWithContextMemoryManagement( fh_istream ss );

    public:

        // FIXME: should these two be public? Should they become Shell::Basename()
        // etc and fix the seperator as a / char? what about files with / in
        // thier names?
        virtual std::string getLastPartOfName( const std::string& s );
        virtual std::string appendToPath( const std::string& p,
                                          const std::string& d,
                                          bool allowDirToBeAbsolute = false );

        Attribute( Parent_t parent = 0 );
        virtual ~Attribute();

        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in );
        
        virtual fh_istream getLocalIStream( std::string& new_dn, ferris_ios::openmode m = std::ios::in );

        virtual fh_iostream getIOStream( ferris_ios::openmode m = std::ios::in|std::ios::out );

        virtual bool checkOpenModeSupported( ferris_ios::openmode userm );
        virtual ferris_ios::openmode getSupportedOpenModes();
        
        virtual Parent_t getParent();
        virtual bool isParentBound();

        /**
         * Get the rdn of this attribute
         *
         * @see getDirPath()
         * @return The rdn.
         */
        virtual const std::string& getDirName() const;
        virtual std::string getDirPath();

        virtual fh_iostream copyTo( fh_iostream oss );
        virtual fh_ostream  copyTo( fh_ostream oss );

    private:

        // kept protected for now so that I can modify the code
        // and not remake all of ferris, later will be public.
        template< class T > T copyTo( fh_istream iss, T oss );
        

        Parent_t theParent;
    };
    



    class FERRISEXP_API EA_Atom
    {
    public:

        EA_Atom();
        virtual ~EA_Atom();

        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m = std::ios::in ) = 0;
        
        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m = std::ios::in|std::ios::out );

        virtual bool checkOpenModeSupported( ferris_ios::openmode userm );
        virtual ferris_ios::openmode getSupportedOpenModes();

        virtual bool havePassedInSteamRead();
        
//         virtual fh_iostream copyTo( fh_iostream oss );
//         virtual fh_ostream  copyTo( fh_ostream oss );
        
//     private:

//         // kept protected for now so that I can modify the code
//         // and not remake all of ferris, later will be public.
//         template< class T > T copyTo( fh_istream iss, T oss );
    };
    
    
    class FERRISEXP_API EA_Atom_ReadOnly_PassedInStream;
    
    /**
     * The functor is called when this attribute is read to get a string representing
     * the ea.
     */
    class FERRISEXP_API EA_Atom_ReadOnly
        :
        public EA_Atom
    {
        typedef EA_Atom_ReadOnly  _Self;
        typedef EA_Atom _Base;

        friend class EA_Atom_ReadOnly_PassedInStream;
        EA_Atom_ReadOnly()
            {}
        
    public:

        typedef Loki::Functor< fh_istream,
                               LOKI_TYPELIST_3( Context*,
                                           const std::string&,
                                           EA_Atom* ) > GetIStream_Func_t;

        EA_Atom_ReadOnly( GetIStream_Func_t f );
        template <typename PointerToObj, typename PointerToMemFn>
        inline EA_Atom_ReadOnly( const PointerToObj& pObj, PointerToMemFn pMemFn )
            :
            GetIStream_Func( GetIStream_Func_t( pObj, pMemFn ) )
            {
            }

        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m );

    protected:

        GetIStream_Func_t GetIStream_Func;
    };


    /**
     * Just like EA_Atom_ReadOnly only more efficient.
     * For reading small EA passing in a stringstream& and using that directly
     * can be much more efficient. This is particularly noticable for sorting
     * where you will have many compare (getEA) operations to sort a directory.
     *
     * Added December-2007.
     */
    class FERRISEXP_API EA_Atom_ReadOnly_PassedInStream
        :
        public EA_Atom_ReadOnly
    {
        typedef EA_Atom_ReadOnly_PassedInStream  _Self;
        typedef EA_Atom_ReadOnly                 _Base;

    public:
        
        typedef Loki::Functor< fh_stringstream&,
                               LOKI_TYPELIST_4( Context*,
                                                const std::string&,
                                                EA_Atom*,
                                                fh_stringstream& ) > GetIStream_PassedInStream_Func_t;
        
        EA_Atom_ReadOnly_PassedInStream( GetIStream_PassedInStream_Func_t f );
        template <typename PointerToObj, typename PointerToMemFn>
        EA_Atom_ReadOnly_PassedInStream( const PointerToObj& pObj, PointerToMemFn pMemFn )
            :
            GetIStream_Func( GetIStream_PassedInStream_Func_t( pObj, pMemFn ) )
            {
            }

        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m );
        virtual fh_stringstream& getIStream( Context* c,
                                             const std::string& rdn,
                                             ferris_ios::openmode m,
                                             fh_stringstream& ss );

        virtual bool havePassedInSteamRead();
        
    protected:

        GetIStream_PassedInStream_Func_t GetIStream_Func;
    };
    
    /******************************/
    /******************************/
    /******************************/
    /******************************/

    template < class ParentClass >
    class FERRISEXP_API EA_Atom_ReadWrite_Base
        :
        public ParentClass
    {
        typedef EA_Atom_ReadWrite_Base _Self;
        
    public:
        typedef Loki::Functor< void,
                                 LOKI_TYPELIST_4( Context*,
                                             const std::string&,
                                             EA_Atom*,
                                             fh_istream ) > IOStreamClosed_Func_t;

    protected:
        
        IOStreamClosed_Func_t IOStreamClosed_Func;
        virtual fh_iostream priv_getIOStream( Context* c, const std::string& rdn ) = 0;
        
    public:

        template <typename PointerToObj, typename PointerToMemFn>
        EA_Atom_ReadWrite_Base( const PointerToObj& pObj, PointerToMemFn pMemFn,
                                const IOStreamClosed_Func_t& f_closed )
            :
            ParentClass( pObj, pMemFn ),
            IOStreamClosed_Func( f_closed )
            {}
        template <typename functorForParent>
        EA_Atom_ReadWrite_Base( functorForParent f,
                                const IOStreamClosed_Func_t& f_closed )
            :
            ParentClass( f ),
            IOStreamClosed_Func( f_closed )
            {}

        
        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m  = std::ios::in|std::ios::out );
        

        virtual void On_IOStreamClosed( fh_istream& ss_param,
                                        std::streamsize tellp,
                                        ferris_ios::openmode m,
                                        Context* c,
                                        std::string rdn ) = 0;
        
        
    };
    
    
    class FERRISEXP_API EA_Atom_ReadWrite
        :
        public sigc::trackable,
        public EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly >
    {
        typedef EA_Atom_ReadWrite _Self;
        typedef EA_Atom_ReadOnly  _Base;

    protected:
        
    public:

        typedef EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly > RWBase_t;
        typedef RWBase_t::IOStreamClosed_Func_t IOStreamClosed_Func_t;
        typedef RWBase_t::GetIStream_Func_t GetIStream_Func_t;

        typedef Loki::Functor< fh_iostream,
                                 LOKI_TYPELIST_3( Context*,
                                             const std::string&,
                                             EA_Atom* ) > GetIOStream_Func_t;

        EA_Atom_ReadWrite( const GetIStream_Func_t& f_i,
                           const GetIOStream_Func_t& f_io,
                           const IOStreamClosed_Func_t& f_closed );
        template <typename PointerToObj_i, typename PointerToMemFn_i,
                  typename PointerToObj_o, typename PointerToMemFn_o,
                  typename PointerToObj_c, typename PointerToMemFn_c >
        inline EA_Atom_ReadWrite( const PointerToObj_i& pObj_i, PointerToMemFn_i pMemFn_i,
                                  const PointerToObj_o& pObj_o, PointerToMemFn_o pMemFn_o,
                                  const PointerToObj_c& pObj_c, PointerToMemFn_c pMemFn_c )
            :
            GetIOStream_Func( GetIOStream_Func_t( pObj_o, pMemFn_o ) ),
//             RWBase_t( GetIStream_Func_t( pObj_i, pMemFn_i),
//                       IOStreamClosed_Func_t( pObj_c, pMemFn_c ) )
            RWBase_t( pObj_i, pMemFn_i,
                      IOStreamClosed_Func_t( pObj_c, pMemFn_c ) )
            {
            }

        virtual fh_iostream priv_getIOStream( Context* c, const std::string& rdn );

        // breaks ambiguous selection issues for subclasses
        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m  = std::ios::in|std::ios::out );

        
    protected:

        GetIOStream_Func_t    GetIOStream_Func;
        virtual ferris_ios::openmode getSupportedOpenModes();
        virtual void On_IOStreamClosed( fh_istream& ss_param,
                                        std::streamsize tellp,
                                        ferris_ios::openmode m,
                                        Context* c,
                                        std::string rdn );
    };

    /******************************/
    /******************************/
    /******************************/
    /******************************/
    
    class FERRISEXP_API EA_Atom_ReadWrite_PassedInStream
        :
        public sigc::trackable,
        public EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly_PassedInStream >
    {
        typedef EA_Atom_ReadWrite_PassedInStream _Self;
        typedef EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly_PassedInStream >  _Base;
        typedef EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly_PassedInStream > RWBase_t;

    public:

        typedef Loki::Functor< void,
                                 LOKI_TYPELIST_4( Context*,
                                             const std::string&,
                                             EA_Atom*,
                                             fh_istream ) > IOStreamClosed_Func_t;
        typedef EA_Atom_ReadWrite_Base< EA_Atom_ReadOnly_PassedInStream >::GetIStream_PassedInStream_Func_t GetIStream_PassedInStream_Func_t;
        

        EA_Atom_ReadWrite_PassedInStream( const GetIStream_PassedInStream_Func_t& f_i,
                                          const IOStreamClosed_Func_t& f_closed );
        template <typename PointerToObj_i, typename PointerToMemFn_i,
                  typename PointerToObj_c, typename PointerToMemFn_c >
        inline EA_Atom_ReadWrite_PassedInStream( const PointerToObj_i& pObj_i, PointerToMemFn_i pMemFn_i,
                                                 const PointerToObj_c& pObj_c, PointerToMemFn_c pMemFn_c )
            :
//             _Base( GetIStream_PassedInStream_Func_t( pObj_i, pMemFn_i),
//                    IOStreamClosed_Func_t( pObj_c, pMemFn_c ) )
            _Base( pObj_i, pMemFn_i,
                   IOStreamClosed_Func_t( pObj_c, pMemFn_c ) )
            {
            }

        virtual fh_iostream priv_getIOStream( Context* c, const std::string& rdn );
        
    protected:

        virtual ferris_ios::openmode getSupportedOpenModes();
        virtual void On_IOStreamClosed( fh_istream& ss_param,
                                        std::streamsize tellp,
                                        ferris_ios::openmode m,
                                        Context* c,
                                        std::string rdn );
    };


    /******************************/
    /******************************/
    /******************************/
    /******************************/
    
    /**
     * This should be used sparingly as it will eat more RAM. It should be OK in
     * stateless EA due to the one object per context type semantics.
     *
     * It was created because the "content" stateless EA needs access to the openmode
     * to perform special tasks. Basically there is a cache of the openmode which can
     * be obtained using atom->getOpenMode() when the functor for handling getXStream()
     * is called.
     */
    class FERRISEXP_API EA_Atom_ReadWrite_OpenModeCached
        :
        public EA_Atom_ReadWrite
    {
        typedef EA_Atom_ReadWrite _Base;
        
    protected:
        ferris_ios::openmode theMode;
        
    public:

        EA_Atom_ReadWrite_OpenModeCached( const GetIStream_Func_t& f_i,
                                          const GetIOStream_Func_t& f_io,
                                          const IOStreamClosed_Func_t& f_closed )
            :
            _Base( f_i, f_io, f_closed ),
            theMode( 0 )
            {}
        
        
        /**
         * Get the openmode that was passed to getXStream() which has called the functor
         */
        ferris_ios::openmode getOpenMode();
        
        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m );

        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m  = std::ios::in|std::ios::out );
    };
    

    /**
     * Special handling to not attach a closed handler for the stream
     */
    class FERRISEXP_API EA_Atom_ReadWrite_Contents
        :
        public EA_Atom_ReadWrite_OpenModeCached
    {
        typedef EA_Atom_ReadWrite_Contents       _Self;
        typedef EA_Atom_ReadWrite_OpenModeCached _Base;
    public:

        EA_Atom_ReadWrite_Contents( const GetIStream_Func_t& f_i,
                                    const GetIOStream_Func_t& f_io )
            :
            _Base(
                f_i,
                f_io,
                IOStreamClosed_Func_t( this, &_Self::nullclosed ) )
            {}
        
        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m  = std::ios::in|std::ios::out );

        void nullclosed( Context*, const std::string&, EA_Atom*, fh_istream )
            {
            }
    };
    

    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    /**
     * Sometimes its very handy to carry a fixed string value.
     */
    class FERRISEXP_API EA_Atom_Static
        :
        public EA_Atom
    {
        std::string theValue;
        
    public:
        
        EA_Atom_Static( const std::string& v );

        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m );
    };


    /**
     * Caching derived information in an RDF store.
     */
    class FERRISEXP_API EA_Atom_RDFCacheAttribute
        :
        public EA_Atom
    {
        bool m_shouldUpdateRDFStore;
        EA_Atom* m_atom;
        std::string m_cache;
        
    public:
        
        EA_Atom_RDFCacheAttribute( EA_Atom* a, bool m_shouldUpdateRDFStore );
        EA_Atom_RDFCacheAttribute( EA_Atom* a, const std::string& m_cache );

        virtual fh_istream getIStream( Context* c,
                                       const std::string& rdn,
                                       ferris_ios::openmode m );

        virtual fh_iostream getIOStream( Context* c,
                                         const std::string& rdn,
                                         ferris_ios::openmode m = std::ios::in|std::ios::out );

        virtual ferris_ios::openmode getSupportedOpenModes();
        
    };
    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
        

    class FERRISEXP_API AttributeProxy
        :
        public Attribute,
        public Handlable
    {
        EA_Atom* atom;

    protected:

        enum
        {
            HIGH_RC = 100
        };

        bool HoldRefCountHigh;
        fh_context theContext;
        std::string theAttributeName;

        virtual EA_Atom* getAttr();


        friend class ContextStreamMemoryManager;
        
    public:

        AttributeProxy( fh_context c, EA_Atom* atom, const std::string& aName );

        virtual ref_count_t AddRef();
        virtual ref_count_t Release();

        virtual fh_istream getIStream( ferris_ios::openmode m = std::ios::in );
        
        
        virtual fh_istream getLocalIStream( std::string& new_dn, ferris_ios::openmode m = std::ios::in );

        virtual fh_iostream getIOStream( ferris_ios::openmode m = std::ios::in|std::ios::out );

        virtual const std::string& getDirName() const;
        virtual std::string getDirPath();

        virtual bool checkOpenModeSupported( ferris_ios::openmode userm );
        virtual ferris_ios::openmode getSupportedOpenModes();
    };
    

    /**
     * A collection of attributes
     *
     * Note that no virtual methods are used so that there is no vtable overhead
     * for Context to inherit from this class.
     */
    class FERRISEXP_API AttributeCollection
        :
        public Versioned
    {
        typedef AttributeCollection _Self;
        typedef Versioned           _Base;

        // AddRef() and Release() access.
        friend class AttributeProxy;

    public:

        AttributeCollection();
        virtual ~AttributeCollection();


        virtual bool getIsNativeContext() const
            {
                return false;
            }
        
        

        typedef Attribute* AttributePtr;
        typedef std::list< std::string > AttributeNames_t; 
//        typedef std::set< std::string > AttributeNames_t; 
        

        virtual fh_attribute getAttribute( const std::string& rdn );
    protected:
        AttributeNames_t& mergeAttributeNames( AttributeNames_t& ret,
                                               const AttributeNames_t& t1,
                                               const AttributeNames_t& t2 ) const;
   public:
        /**
         * Get the attribute names into the 'ret' parameter and return it.
         */
        virtual AttributeNames_t& getAttributeNames( AttributeNames_t& ret );
        virtual int  getAttributeCount();
        virtual bool isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere = true
            );

        void dumpAttributeNames();
        

        inline bool addAttribute( const std::string& rdn,
                                  const EA_Atom_ReadOnly::GetIStream_Func_t& f,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                return setAttribute( rdn,
                                     new EA_Atom_ReadOnly( f ),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }
        template <typename PointerToObj, typename PointerToMemFn>
        inline bool addAttribute( const std::string& rdn,
                                  const PointerToObj& pObj, PointerToMemFn pMemFn,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                   bool addToREA = false )
            {
                typedef EA_Atom_ReadOnly::GetIStream_Func_t Functor_t;
                return setAttribute( rdn,
                                     new EA_Atom_ReadOnly( Functor_t( pObj, pMemFn )),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            typename PointerToMemFn_i
        >
        inline bool addAttribute( const std::string& rdn,
                                  Loki::SmartPtr<T1, OP1, CP1, KP1, SP1> pObj_i, PointerToMemFn_i pMemFn_i,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                   bool addToREA = false )
            {
                typedef EA_Atom_ReadOnly::GetIStream_Func_t Functor_t;
                return setAttribute( rdn,
                                     new EA_Atom_ReadOnly( Functor_t( GetImpl(pObj_i), pMemFn_i )),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }

        
        inline bool addAttribute( const std::string& rdn,
                                  const EA_Atom_ReadOnly::GetIStream_Func_t& f_i,
                                  const EA_Atom_ReadWrite::GetIOStream_Func_t& f_io,
                                  const EA_Atom_ReadWrite::IOStreamClosed_Func_t& f_closed,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                return setAttribute( rdn,
                                     new EA_Atom_ReadWrite( f_i, f_io, f_closed ),
                                     addToREA,
                                     sct );
            }
        template <typename PointerToObj_i, typename PointerToMemFn_i,
                  typename PointerToObj_o, typename PointerToMemFn_o,
                  typename PointerToObj_c, typename PointerToMemFn_c >
        inline bool addAttribute( const std::string& rdn,
                                  const PointerToObj_i& pObj_i, PointerToMemFn_i pMemFn_i,
                                  const PointerToObj_o& pObj_o, PointerToMemFn_o pMemFn_o,
                                  const PointerToObj_c& pObj_c, PointerToMemFn_c pMemFn_c,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                typedef EA_Atom_ReadOnly::GetIStream_Func_t       Fi;
                typedef EA_Atom_ReadWrite::GetIOStream_Func_t     Fo;
                typedef EA_Atom_ReadWrite::IOStreamClosed_Func_t  Fc;
                return setAttribute( rdn,
                                     new EA_Atom_ReadWrite(
                                         Fi( pObj_i, pMemFn_i ),
                                         Fo( pObj_o, pMemFn_o ),
                                         Fc( pObj_c, pMemFn_c )
                                         ),
                                     addToREA,
                                     sct );
            }

        /**
         * If this monster overload is not here and a fh_context is passed as
         * pObj_i, pObj_o or pObj_c then it will retain a reference to the
         * context inside the functor binding. This method strips off the
         * Loki::SmartPtr shell and uses the raw pointer to avoid such extra
         * references.
         */
        template
        <
            typename T1,
            template <class> class OP1,
            class CP1,
            template <class> class KP1,
            template <class> class SP1,
            typename T2,
            template <class> class OP2,
            class CP2,
            template <class> class KP2,
            template <class> class SP2,
            typename T3,
            template <class> class OP3,
            class CP3,
            template <class> class KP3,
            template <class> class SP3,
            typename PointerToMemFn_i,
            typename PointerToMemFn_o,
            typename PointerToMemFn_c
        >
        inline bool addAttribute(
            const std::string& rdn,
            Loki::SmartPtr<T1, OP1, CP1, KP1, SP1> pObj_i, PointerToMemFn_i pMemFn_i,
            Loki::SmartPtr<T2, OP2, CP2, KP2, SP2> pObj_o, PointerToMemFn_o pMemFn_o,
            Loki::SmartPtr<T3, OP3, CP3, KP3, SP3> pObj_c, PointerToMemFn_c pMemFn_c,
            XSDBasic_t sct = XSD_UNKNOWN,
            bool addToREA = false )
            {
                typedef EA_Atom_ReadOnly::GetIStream_Func_t       Fi;
                typedef EA_Atom_ReadWrite::GetIOStream_Func_t     Fo;
                typedef EA_Atom_ReadWrite::IOStreamClosed_Func_t  Fc;
                return setAttribute( rdn,
                                     new EA_Atom_ReadWrite(
                                         Fi( GetImpl(pObj_i), pMemFn_i ),
                                         Fo( GetImpl(pObj_o), pMemFn_o ),
                                         Fc( GetImpl(pObj_c), pMemFn_c )
                                         ),
                                     addToREA,
                                     sct );
            }
        
        inline bool addAttribute( const std::string& rdn,
                                  const std::string& v,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                return setAttribute( rdn,
                                     new EA_Atom_Static( v ),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }
        inline bool addAttribute( const std::string& rdn,
                                  char* v,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                return setAttribute( rdn,
                                     new EA_Atom_Static( v ),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }
        inline bool addAttribute( const std::string& rdn,
                                  const char* v,
                                  XSDBasic_t sct = XSD_UNKNOWN,
                                  bool addToREA = false )
            {
                return setAttribute( rdn,
                                     new EA_Atom_Static( v ),
                                     addToREA,
                                     XSDBasic_t( sct | FXDC_READONLY ) );
            }

        /**
         * Try to add a new Atom that is allocated on the heap. If the new atom
         * can not be added then it will be deleted and false will be returned.
         * Any really major problems then atom will be deleted and an exception
         * will be thrown.
         */
        bool addAttribute( const std::string& rdn,
                           EA_Atom* atom,
                           XSDBasic_t sct = XSD_UNKNOWN,
                           bool addToREA = false );

        
        /******************************************************************************/
        /******************************************************************************/
        
//         virtual EA_Atom* tryAddHeapAttribute( const std::string& rdn,
//                                                         EA_Atom* a,
//                                                         bool addToREA = false );

        /**
         * This is like addAttribute except it does no checking, and can add stateless
         * DONT use this method directly, either use addAttribute() or
         * tryAddStateLessAttribute()
         *
         * returns 1 if the attribute was added
         */
        virtual bool setAttribute( const std::string& rdn,
                                   EA_Atom* atx,
                                   bool addToREA,
                                   XSDBasic_t sct = XSD_UNKNOWN,
                                   bool isStateLess = false );
        
    protected:

        /************************************************************/
        /*** namespaces *********************************************/
        /************************************************************/
    public:
        std::string  expandEAName( const std::string& s,
                                   bool expandInternalFerrisNamespaces );
        void         setNamespace( const std::string& prefix, const std::string& URI );
        std::string  resolveNamespace( const std::string& prefix );
        void         removeNamespace( const std::string& prefix );
        virtual      stringlist_t  getNamespacePrefixes();
    protected:
        typedef      stringmap_t*  m_namespaces_t;
        m_namespaces_t             m_namespaces;
        virtual std::string        resolveFerrisXMLNamespace( const std::string& s );
        virtual void               readNamespaces();
        void            freeNamespaces();
        void            updateNamespacesForNewAttribute( const std::string& s );
        
        /************************************************************/
        /************************************************************/
        /************************************************************/
        
        EA_Atom* getAttributeIfExists( const std::string& rdn );
        void callEnsureAttributesAreCreatedMarshalEToNSA( const std::string& eaname = "" );

        
        /**
         * Returns Attribute* if its there, ensures its created if it can be
         * or throws
         */
        EA_Atom* getAttributePtr( const std::string& rdn );
        
        void clearAttributes();

        virtual void ensureAttributesAreCreated( const std::string& eaname = "" );

        /******************************************************************************/
    public:
        typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > Attributes_t;
//        typedef Loki::AssocVector< std::string, EA_Atom* > SLAttributes_t;
//        typedef std::map< std::string, EA_Atom* > SLAttributes_t;
        typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > SLAttributes_t;
    protected:
        Attributes_t Attributes;
        Attributes_t& getAttributes();

        typedef std::list< Loki::TypeInfo > TypeInfos_t;
        Loki::TypeInfo getDeepestTypeInfo();
        TypeInfos_t getTypeInfos();
        virtual void getTypeInfos( TypeInfos_t& l )
            {
                l.push_back( typeid( _Self ) );
            }

        bool getStateLessAttrs_cache_isRAW;
        SLAttributes_t* getStateLessAttrs_cache;
        SLAttributes_t* getStateLessAttrs();
        void setup_DynamicClassedStateLessEAHolder( const std::string& className );
        bool isStateLessEAVirgin( const std::string& s );

        
    public:
        /**
         * Test if there is an attribute called 's' which is stateless.
         * @see isAttributeBound
         */
        bool isStatelessAttributeBound( const std::string& s );

        /**
         * Remove knowledge of a given attribute for this context.
         */ 
        virtual void unsetAttribute( const std::string& rdn );

        /************************************************************/
        /************************************************************/
        /************************************************************/

    public:
        /**
         * To maintain a quick cache of derived attributes read from
         * the local RDF store the mtime of the collection needs to
         * be able to be read quickly. NativeContext should override
         * this to return part of a stat buffer. The default is to
         * return the "mtime" EA of it exists and can be read or to
         * return 0 to indicate no cachning.
         */
        virtual time_t getMTime();

        time_t getRDFCacheMTime();
        
    protected:

        typedef FERRIS_STD_HASH_MAP< std::string, EA_Atom* > m_RDFCacheAttributes_t;
        m_RDFCacheAttributes_t& getRDFCacheAttributes();

        /**
         * Should a given EA be stored into an RDF cache?
         */
        virtual bool shouldRDFCacheAttribute( const std::string& s );


        virtual void RDFCacheAttributes_priv_createAttributes();

    private:
        m_RDFCacheAttributes_t m_RDFCacheAttributes;
        time_t m_RDFCacheMTime;
        friend class EA_Atom_RDFCacheAttribute;

    };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
    
    
};


#endif
