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

    $Id: Enamel.hh,v 1.52 2011/10/22 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

//
// libferris.so logging that uses the generic timber.
//
#ifndef _ALREADY_INCLUDED_ENAMEL_H_
#define _ALREADY_INCLUDED_ENAMEL_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/timber.hh>

#include <map>

#include <popt.h>

namespace Ferris
{
    

namespace Enamel 
{
    FERRISEXP_API Timber& get__t_l0em();
    FERRISEXP_API Timber& get__t_l0a();
    FERRISEXP_API Timber& get__t_l0er();
    FERRISEXP_API Timber& get__t_l0w();
    FERRISEXP_API Timber& get__t_l0n();
    FERRISEXP_API Timber& get__t_l0i();
    FERRISEXP_API Timber& get__t_l0d();

    FERRISEXP_API Timber& get__t_l1em();
    FERRISEXP_API Timber& get__t_l1a();
    FERRISEXP_API Timber& get__t_l1er();
    FERRISEXP_API Timber& get__t_l1w();
    FERRISEXP_API Timber& get__t_l1n();
    FERRISEXP_API Timber& get__t_l1i();
    FERRISEXP_API Timber& get__t_l1d();

    FERRISEXP_API Timber& get__t_l2em();
    FERRISEXP_API Timber& get__t_l2a();
    FERRISEXP_API Timber& get__t_l2er();
    FERRISEXP_API Timber& get__t_l2w();
    FERRISEXP_API Timber& get__t_l2n();
    FERRISEXP_API Timber& get__t_l2i();
    FERRISEXP_API Timber& get__t_l2d();

    FERRISEXP_API Timber& get__t_l3em();
    FERRISEXP_API Timber& get__t_l3a();
    FERRISEXP_API Timber& get__t_l3er();
    FERRISEXP_API Timber& get__t_l3w();
    FERRISEXP_API Timber& get__t_l3n();
    FERRISEXP_API Timber& get__t_l3i();
    FERRISEXP_API Timber& get__t_l3d();

    FERRISEXP_API Timber& get__t_l4em();
    FERRISEXP_API Timber& get__t_l4a();
    FERRISEXP_API Timber& get__t_l4er();
    FERRISEXP_API Timber& get__t_l4w();
    FERRISEXP_API Timber& get__t_l4n();
    FERRISEXP_API Timber& get__t_l4i();
    FERRISEXP_API Timber& get__t_l4d();
    
    FERRISEXP_API Timber& get__t_l5em();
    FERRISEXP_API Timber& get__t_l5a();
    FERRISEXP_API Timber& get__t_l5er();
    FERRISEXP_API Timber& get__t_l5w();
    FERRISEXP_API Timber& get__t_l5n();
    FERRISEXP_API Timber& get__t_l5i();
    FERRISEXP_API Timber& get__t_l5d();

    FERRISEXP_API Timber& get__t_l6em();
    FERRISEXP_API Timber& get__t_l6a();
    FERRISEXP_API Timber& get__t_l6er();
    FERRISEXP_API Timber& get__t_l6w();
    FERRISEXP_API Timber& get__t_l6n();
    FERRISEXP_API Timber& get__t_l6i();
    FERRISEXP_API Timber& get__t_l6d();
    
    FERRISEXP_API Timber& get__t_l7em();
    FERRISEXP_API Timber& get__t_l7a();
    FERRISEXP_API Timber& get__t_l7er();
    FERRISEXP_API Timber& get__t_l7w();
    FERRISEXP_API Timber& get__t_l7n();
    FERRISEXP_API Timber& get__t_l7i();
    FERRISEXP_API Timber& get__t_l7d();

};


//#define LG_STATE    "file:" << __FILE__ << " line:" <<  __LINE__ << " "
#define LG_STATE    __FILE__ << ":" <<  __LINE__ << " "

namespace Logging 
{
    typedef Ferris::Timber::_SBufT::Priority_t state_t;

    FERRISEXP_API struct ::poptOption* getPopTable();
    

    
    struct FERRISEXP_API realStreamsKey
        {
            Ferris::Timber::_SBufT::Facility_t F;
            Ferris::Timber::_SBufT::Priority_t S;
            
            realStreamsKey( Ferris::Timber::_SBufT::Facility_t f, Ferris::Timber::_SBufT::Priority_t s )
                :
                F(f),
                S(s)
                {
                }

            friend bool operator<( const realStreamsKey& k1, const realStreamsKey& k2 );
            friend bool operator==( const realStreamsKey& k1, const realStreamsKey& k2 );
        };

    
    
    class FERRISEXP_API LogStateBase
    {
    public:

        virtual std::string getName();
        
    protected:
        
        state_t m_state;
        virtual state_t getDefaultState();

        void setState( state_t v );
        
        
    public:

        LogStateBase();
        virtual ~LogStateBase();

        f_ostream& getNullStream();
        NullStream<>& getNullStreamWithType();

        
        typedef Timber::Facility_t Facility_t;
        typedef std::map< realStreamsKey, fh_timber > realStreams_t;
        realStreams_t realStreams;
        
        fh_ostream& getStream( Ferris::Timber::_SBufT::Facility_t f, state_t d );

        void clear();
        void reset();
        void unset( state_t s );
        void add( state_t s );
        void set( state_t s );
        state_t& state();
        
    };

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /*
     * I need the return type to be determined as the most specialized type
     * so that the NullStream template functions can be used and thus things
     * are more efficient
     */

    namespace Private
    {
        template <class _CharT, class _Traits>
        inline NullStream<_CharT>& 
        ConvertToNullStreamStaticType(std::basic_ostream<_CharT, _Traits>& __os)
        {
            static NullStream<_CharT> x;
            return x;
        }
    };
    
#define INLINE_SWITCH 1    
#ifdef INLINE_SWITCH

#define LG_SWITCH( sing, neededState )           \
     if(sing.state() & neededState)              \
         sing.getRealStream( neededState )

#define LG_SWITCH_VERBOSE( sing, neededState )           \
     std::cerr << "sing.state:" << sing.state() << " needed:" << neededState << endl; \
     if(sing.state() & neededState)              \
         sing.getRealStream( neededState )

    
#define LG_SWITCH_NODEBUG( sing, neededState )   \
         if( 0 ) sing.getNullStreamWithType()

#define LOGGING_SWITCH_FUNCTION     LG_SWITCH
//#define LOGGING_SWITCH_FUNCTION     LG_SWITCH_NODEBUG

#else

    template <class Singleton>
    inline fhl_ostream&
    Switch( Singleton& sing, state_t neededState )
    {
        static null_streambuf<char>  nullbuf;
        static fhl_ostream ss( &nullbuf );
        static int msk = 0x111;
        int    useDebug = sing.state() & neededState;
        
        if( useDebug ^ msk )
        {
            if( useDebug )
            {
                 fh_timber& timb = sing.getRealStream( neededState );

                 ss.rdbuf( timb.rdbuf() );
                 ss.clear();
            }
            else
            {
                ss.clear( ios_base::failbit );
//                return sing.getNullStream();
            }
        }
        
        msk = useDebug;
        return ss;
    }
#define LOGGING_SWITCH_FUNCTION     Ferris::Logging::Switch
#endif    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    template < Timber::Facility_t fac, char const * name >
    class LogState : public LogStateBase
    {
    public:

        LogState()
            {
//                 cerr << "LogState(1) state:" << state() << endl;
//                 cerr << "LogState(1) name :" << getName() << endl;
                setState( getDefaultState() );
//                 cerr << "LogState(2) state:" << state() << endl;
            }
        
        inline fh_ostream& getRealStream( state_t d )
            {
                return getStream( fac, d );
            }

        virtual std::string getName()
            {
//                std::string tmp = name;
//                cerr << "getName() : " << tmp << endl;
                return name;
            }
        
    };
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    extern FERRISEXP_API char const lg_strf_class_name[];
//    extern FERRISEXP_API string lg_strf_class_name;
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_strf_class_name > lg_strf_class;
    typedef Loki::SingletonHolder<lg_strf_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_STRF;

#define LG_STRF(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_STRF::Instance(), x)<<LG_STATE
#define LG_STRF_ER    LG_STRF(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_STRF_W     LG_STRF(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_STRF_I     LG_STRF(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_STRF_D     LG_STRF(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_STRF_D_ACTIVE (Ferris::Logging::LG_STRF::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    

    extern FERRISEXP_API char const lg_attr_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_attr_class_name > lg_attr_class;
    typedef Loki::SingletonHolder<lg_attr_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ATTR;

#define LG_ATTR(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ATTR::Instance(), x)<<LG_STATE
#define LG_ATTR_ER    LG_ATTR(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ATTR_W     LG_ATTR(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ATTR_I     LG_ATTR(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ATTR_D     LG_ATTR(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_ATTR_ACTIVE (Ferris::Logging::LG_ATTR::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_overmount_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_overmount_class_name > lg_overmount_class;
    typedef Loki::SingletonHolder<lg_overmount_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_OVERMOUNT;

#define LG_OVERMOUNT(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_OVERMOUNT::Instance(), x)<<LG_STATE
#define LG_OVERMOUNT_ER    LG_OVERMOUNT(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_OVERMOUNT_W     LG_OVERMOUNT(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_OVERMOUNT_I     LG_OVERMOUNT(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_OVERMOUNT_D     LG_OVERMOUNT(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_sort_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_sort_class_name > lg_sort_class;
    typedef Loki::SingletonHolder<lg_sort_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SORT;

#define LG_SORT(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SORT::Instance(), x)<<LG_STATE
#define LG_SORT_ER    LG_SORT(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SORT_W     LG_SORT(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SORT_I     LG_SORT(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SORT_D     LG_SORT(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_image_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_image_class_name > lg_image_class;
    typedef Loki::SingletonHolder<lg_image_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IMAGE;

#define LG_IMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IMAGE::Instance(), x)<<LG_STATE
#define LG_IMAGE_ER    LG_IMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IMAGE_W     LG_IMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IMAGE_I     LG_IMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IMAGE_D     LG_IMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_pngimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_pngimage_class_name > lg_pngimage_class;
    typedef Loki::SingletonHolder<lg_pngimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PNGIMAGE;

#define LG_PNGIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PNGIMAGE::Instance(), x)<<LG_STATE
#define LG_PNGIMAGE_ER    LG_PNGIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PNGIMAGE_W     LG_PNGIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PNGIMAGE_I     LG_PNGIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PNGIMAGE_D     LG_PNGIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_jasperimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_jasperimage_class_name > lg_jasperimage_class;
    typedef Loki::SingletonHolder<lg_jasperimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_JASPERIMAGE;

#define LG_JASPERIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_JASPERIMAGE::Instance(), x)<<LG_STATE
#define LG_JASPERIMAGE_ER    LG_JASPERIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_JASPERIMAGE_W     LG_JASPERIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_JASPERIMAGE_I     LG_JASPERIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_JASPERIMAGE_D     LG_JASPERIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    extern FERRISEXP_API char const lg_jpegimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_jpegimage_class_name > lg_jpegimage_class;
    typedef Loki::SingletonHolder<lg_jpegimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_JPEGIMAGE;

#define LG_JPEGIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_JPEGIMAGE::Instance(), x)<<LG_STATE
#define LG_JPEGIMAGE_ER    LG_JPEGIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_JPEGIMAGE_W     LG_JPEGIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_JPEGIMAGE_I     LG_JPEGIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_JPEGIMAGE_D     LG_JPEGIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    extern FERRISEXP_API char const lg_djvuimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_djvuimage_class_name > lg_djvuimage_class;
    typedef Loki::SingletonHolder<lg_djvuimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DJVUIMAGE;

#define LG_DJVUIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_DJVUIMAGE::Instance(), x)<<LG_STATE
#define LG_DJVUIMAGE_ER    LG_DJVUIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DJVUIMAGE_W     LG_DJVUIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DJVUIMAGE_I     LG_DJVUIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DJVUIMAGE_D     LG_DJVUIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gimpimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gimpimage_class_name > lg_gimpimage_class;
    typedef Loki::SingletonHolder<lg_gimpimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GIMPIMAGE;

#define LG_GIMPIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GIMPIMAGE::Instance(), x)<<LG_STATE
#define LG_GIMPIMAGE_ER    LG_GIMPIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GIMPIMAGE_W     LG_GIMPIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GIMPIMAGE_I     LG_GIMPIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GIMPIMAGE_D     LG_GIMPIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_magikimage_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_magikimage_class_name > lg_magikimage_class;
    typedef Loki::SingletonHolder<lg_magikimage_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MAGIKIMAGE;

#define LG_MAGIKIMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MAGIKIMAGE::Instance(), x)<<LG_STATE
#define LG_MAGIKIMAGE_ER    LG_MAGIKIMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MAGIKIMAGE_W     LG_MAGIKIMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MAGIKIMAGE_I     LG_MAGIKIMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MAGIKIMAGE_D     LG_MAGIKIMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_imlib2image_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_imlib2image_class_name > lg_imlib2image_class;
    typedef Loki::SingletonHolder<lg_imlib2image_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IMLIB2IMAGE;

#define LG_IMLIB2IMAGE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IMLIB2IMAGE::Instance(), x)<<LG_STATE
#define LG_IMLIB2IMAGE_ER    LG_IMLIB2IMAGE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IMLIB2IMAGE_W     LG_IMLIB2IMAGE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IMLIB2IMAGE_I     LG_IMLIB2IMAGE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IMLIB2IMAGE_D     LG_IMLIB2IMAGE(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_strattr_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_strattr_class_name > lg_strattr_class;
    typedef Loki::SingletonHolder<lg_strattr_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_STRATTR;

#define LG_STRATTR(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_STRATTR::Instance(), x)<<LG_STATE
#define LG_STRATTR_ER    LG_STRATTR(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_STRATTR_W     LG_STRATTR(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_STRATTR_I     LG_STRATTR(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_STRATTR_D     LG_STRATTR(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_byteattr_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_byteattr_class_name > lg_byteattr_class;
    typedef Loki::SingletonHolder<lg_byteattr_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_BYTEATTR;

#define LG_BYTEATTR(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_BYTEATTR::Instance(), x)<<LG_STATE
#define LG_BYTEATTR_ER    LG_BYTEATTR(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_BYTEATTR_W     LG_BYTEATTR(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_BYTEATTR_I     LG_BYTEATTR(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_BYTEATTR_D     LG_BYTEATTR(Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ctx_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ctx_class_name > lg_ctx_class;
    typedef Loki::SingletonHolder<lg_ctx_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_CTX;

#define LG_CTX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_CTX::Instance(), x)<<LG_STATE
#define LG_CTX_ER    LG_CTX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_CTX_W     LG_CTX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_CTX_N     LG_CTX(Ferris::Timber::_SBufT::PRI_NOTICE)
#define LG_CTX_I     LG_CTX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_CTX_D     LG_CTX(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_vm_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_vm_class_name > lg_vm_class;
    typedef Loki::SingletonHolder<lg_vm_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_VM;

#define LG_VM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_VM::Instance(), x)<<LG_STATE
#define LG_VM_ER    LG_VM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_VM_W     LG_VM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_VM_I     LG_VM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_VM_D     LG_VM(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_native_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_native_class_name > lg_native_class;
    typedef Loki::SingletonHolder<lg_native_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_NATIVE;

#define LG_NATIVE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_NATIVE::Instance(), x)<<LG_STATE
#define LG_NATIVE_ER    LG_NATIVE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_NATIVE_W     LG_NATIVE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_NATIVE_I     LG_NATIVE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_NATIVE_D     LG_NATIVE(Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_factory_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_factory_class_name > lg_factory_class;
    typedef Loki::SingletonHolder<lg_factory_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FACTORY;

#define LG_FACTORY(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FACTORY::Instance(), x)<<LG_STATE
#define LG_FACTORY_ER    LG_FACTORY(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FACTORY_W     LG_FACTORY(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FACTORY_I     LG_FACTORY(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FACTORY_D     LG_FACTORY(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_id3_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_id3_class_name > lg_id3_class;
    typedef Loki::SingletonHolder<lg_id3_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ID3;

#define LG_ID3(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ID3::Instance(), x)<<LG_STATE
#define LG_ID3_ER    LG_ID3(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ID3_W     LG_ID3(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ID3_I     LG_ID3(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ID3_D     LG_ID3(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_mpg_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_mpg_class_name > lg_mpg_class;
    typedef Loki::SingletonHolder<lg_mpg_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MPG;

#define LG_MPG(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MPG::Instance(), x)<<LG_STATE
#define LG_MPG_ER    LG_MPG(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MPG_W     LG_MPG(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MPG_I     LG_MPG(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MPG_D     LG_MPG(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_plugin_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_plugin_class_name > lg_plugin_class;
    typedef Loki::SingletonHolder<lg_plugin_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PLUGIN;

#define LG_PLUGIN(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PLUGIN::Instance(), x)<<LG_STATE
#define LG_PLUGIN_ER    LG_PLUGIN(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PLUGIN_W     LG_PLUGIN(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PLUGIN_I     LG_PLUGIN(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PLUGIN_D     LG_PLUGIN(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ffilter_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ffilter_class_name > lg_ffilter_class;
    typedef Loki::SingletonHolder<lg_ffilter_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FFILTER;

#define LG_FFILTER(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FFILTER::Instance(), x)<<LG_STATE
#define LG_FFILTER_ER    LG_FFILTER(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FFILTER_W     LG_FFILTER(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FFILTER_I     LG_FFILTER(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FFILTER_D     LG_FFILTER(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_filterparse_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_filterparse_class_name > lg_filterparse_class;
    typedef Loki::SingletonHolder<lg_filterparse_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FILTERPARSE;

#define LG_FILTERPARSE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FILTERPARSE::Instance(), x)<<LG_STATE
#define LG_FILTERPARSE_ER    LG_FILTERPARSE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FILTERPARSE_W     LG_FILTERPARSE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FILTERPARSE_I     LG_FILTERPARSE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FILTERPARSE_D     LG_FILTERPARSE(Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_mbox_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_mbox_class_name > lg_mbox_class;
    typedef Loki::SingletonHolder<lg_mbox_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MBOX;

#define LG_MBOX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MBOX::Instance(), x)<<LG_STATE
#define LG_MBOX_ER    LG_MBOX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MBOX_W     LG_MBOX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MBOX_I     LG_MBOX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MBOX_D     LG_MBOX(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_edb_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_edb_class_name > lg_edb_class;
    typedef Loki::SingletonHolder<lg_edb_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EDB;

#define LG_EDB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EDB::Instance(), x)<<LG_STATE
#define LG_EDB_ER    LG_EDB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EDB_W     LG_EDB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EDB_I     LG_EDB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EDB_D     LG_EDB(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_paths_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_paths_class_name > lg_paths_class;
    typedef Loki::SingletonHolder<lg_paths_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PATHS;

#define LG_PATHS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PATHS::Instance(), x)<<LG_STATE
#define LG_PATHS_ER    LG_PATHS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PATHS_W     LG_PATHS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PATHS_I     LG_PATHS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PATHS_D     LG_PATHS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_pccts_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_pccts_class_name > lg_pccts_class;
    typedef Loki::SingletonHolder<lg_pccts_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PCCTS;

#define LG_PCCTS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PCCTS::Instance(), x)<<LG_STATE
#define LG_PCCTS_ER    LG_PCCTS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PCCTS_W     LG_PCCTS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PCCTS_I     LG_PCCTS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PCCTS_D     LG_PCCTS(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xfs_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xfs_class_name > lg_xfs_class;
    typedef Loki::SingletonHolder<lg_xfs_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XFS;

#define LG_XFS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_XFS::Instance(), x)<<LG_STATE
#define LG_XFS_ER    LG_XFS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XFS_W     LG_XFS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XFS_I     LG_XFS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XFS_D     LG_XFS(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_socket_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_socket_class_name > lg_socket_class;
    typedef Loki::SingletonHolder<lg_socket_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SOCKET;

#define LG_SOCKET(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SOCKET::Instance(), x)<<LG_STATE
#define LG_SOCKET_ER    LG_SOCKET(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SOCKET_W     LG_SOCKET(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SOCKET_I     LG_SOCKET(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SOCKET_D     LG_SOCKET(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_iostream_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_iostream_class_name > lg_iostream_class;
    typedef Loki::SingletonHolder<lg_iostream_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IOSTREAM;
    
#define LG_IOSTREAM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IOSTREAM::Instance(), x)<<LG_STATE
#define LG_IOSTREAM_ER    LG_IOSTREAM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IOSTREAM_W     LG_IOSTREAM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IOSTREAM_I     LG_IOSTREAM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IOSTREAM_D     LG_IOSTREAM(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xml_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xml_class_name > lg_xml_class;
    typedef Loki::SingletonHolder<lg_xml_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XML;
    
#define LG_XML(x)    LOGGING_SWITCH_FUNCTION(::Ferris::Logging::LG_XML::Instance(), x)<<LG_STATE
#define LG_XML_ER    LG_XML(::Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XML_W     LG_XML(::Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XML_I     LG_XML(::Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XML_D     LG_XML(::Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_dom_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_dom_class_name > lg_dom_class;
    typedef Loki::SingletonHolder<lg_dom_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DOM;
    
#define LG_DOM(x)    LOGGING_SWITCH_FUNCTION(::Ferris::Logging::LG_DOM::Instance(), x)<<LG_STATE
#define LG_DOM_ER    LG_DOM(::Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DOM_W     LG_DOM(::Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DOM_I     LG_DOM(::Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DOM_D     LG_DOM(::Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_DOM_ACTIVE (Ferris::Logging::LG_DOM::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_extfs_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_extfs_class_name > lg_extfs_class;
    typedef Loki::SingletonHolder<lg_extfs_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EXTFS;
    
#define LG_EXTFS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EXTFS::Instance(), x)<<LG_STATE
#define LG_EXTFS_ER    LG_EXTFS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EXTFS_W     LG_EXTFS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EXTFS_I     LG_EXTFS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EXTFS_D     LG_EXTFS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_runner_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_runner_class_name > lg_runner_class;
    typedef Loki::SingletonHolder<lg_runner_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RUNNER;
    
#define LG_RUNNER(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RUNNER::Instance(), x)<<LG_STATE
#define LG_RUNNER_ER    LG_RUNNER(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RUNNER_W     LG_RUNNER(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RUNNER_I     LG_RUNNER(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RUNNER_D     LG_RUNNER(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_a52_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_a52_class_name > lg_a52_class;
    typedef Loki::SingletonHolder<lg_a52_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_A52;
    
#define LG_A52(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_A52::Instance(), x)<<LG_STATE
#define LG_A52_ER    LG_A52(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_A52_W     LG_A52(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_A52_I     LG_A52(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_A52_D     LG_A52(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_mpeg2_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_mpeg2_class_name > lg_mpeg2_class;
    typedef Loki::SingletonHolder<lg_mpeg2_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MPEG2;
    
#define LG_MPEG2(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MPEG2::Instance(), x)<<LG_STATE
#define LG_MPEG2_ER    LG_MPEG2(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MPEG2_W     LG_MPEG2(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MPEG2_I     LG_MPEG2(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MPEG2_D     LG_MPEG2(Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_sqldb_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_sqldb_class_name > lg_sqldb_class;
    typedef Loki::SingletonHolder<lg_sqldb_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SQLDB;

#define LG_SQLDB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SQLDB::Instance(), x)<<LG_STATE
#define LG_SQLDB_ER    LG_SQLDB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SQLDB_W     LG_SQLDB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SQLDB_I     LG_SQLDB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SQLDB_D     LG_SQLDB(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_sqlplus_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_sqlplus_class_name > lg_sqlplus_class;
    typedef Loki::SingletonHolder<lg_sqlplus_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SQLPLUS;

#define LG_SQLPLUS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SQLPLUS::Instance(), x)<<LG_STATE
#define LG_SQLPLUS_ER    LG_SQLPLUS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SQLPLUS_W     LG_SQLPLUS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SQLPLUS_I     LG_SQLPLUS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SQLPLUS_D     LG_SQLPLUS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_dtl_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_dtl_class_name > lg_dtl_class;
    typedef Loki::SingletonHolder<lg_dtl_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DTL;

#define LG_DTL(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_DTL::Instance(), x)<<LG_STATE
#define LG_DTL_ER    LG_DTL(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DTL_W     LG_DTL(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DTL_I     LG_DTL(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DTL_D     LG_DTL(Ferris::Timber::_SBufT::PRI_DEBUG)

    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_curl_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_curl_class_name > lg_curl_class;
    typedef Loki::SingletonHolder<lg_curl_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_CURL;

#define LG_CURL(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_CURL::Instance(), x)<<LG_STATE
#define LG_CURL_ER    LG_CURL(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_CURL_W     LG_CURL(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_CURL_I     LG_CURL(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_CURL_D     LG_CURL(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gtkferris_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gtkferris_class_name > lg_gtkferris_class;
    typedef Loki::SingletonHolder<lg_gtkferris_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GTKFERRIS;

#define LG_GTKFERRIS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GTKFERRIS::Instance(), x)<<LG_STATE
#define LG_GTKFERRIS_ER    LG_GTKFERRIS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GTKFERRIS_W     LG_GTKFERRIS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GTKFERRIS_I     LG_GTKFERRIS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GTKFERRIS_D     LG_GTKFERRIS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ferriscreate_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ferriscreate_class_name > lg_ferriscreate_class;
    typedef Loki::SingletonHolder<lg_ferriscreate_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FERRISCREATE;

#define LG_FERRISCREATE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FERRISCREATE::Instance(), x)<<LG_STATE
#define LG_FERRISCREATE_ER    LG_FERRISCREATE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FERRISCREATE_W     LG_FERRISCREATE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FERRISCREATE_I     LG_FERRISCREATE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FERRISCREATE_D     LG_FERRISCREATE(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_time_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_time_class_name > lg_time_class;
    typedef Loki::SingletonHolder<lg_time_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_TIME;

#define LG_TIME(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_TIME::Instance(), x)<<LG_STATE
#define LG_TIME_ER    LG_TIME(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_TIME_W     LG_TIME(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_TIME_I     LG_TIME(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_TIME_D     LG_TIME(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_copy_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_copy_class_name > lg_copy_class;
    typedef Loki::SingletonHolder<lg_copy_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_COPY;

#define LG_COPY(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_COPY::Instance(), x)<<LG_STATE
#define LG_COPY_ER    LG_COPY(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_COPY_W     LG_COPY(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_COPY_I     LG_COPY(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_COPY_D     LG_COPY(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_move_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_move_class_name > lg_move_class;
    typedef Loki::SingletonHolder<lg_move_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MOVE;

#define LG_MOVE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MOVE::Instance(), x)<<LG_STATE
#define LG_MOVE_ER    LG_MOVE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MOVE_W     LG_MOVE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MOVE_I     LG_MOVE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MOVE_D     LG_MOVE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_rm_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_rm_class_name > lg_rm_class;
    typedef Loki::SingletonHolder<lg_rm_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RM;

#define LG_RM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RM::Instance(), x)<<LG_STATE
#define LG_RM_ER    LG_RM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RM_W     LG_RM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RM_I     LG_RM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RM_D     LG_RM(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_dvdread_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_dvdread_class_name > lg_dvdread_class;
    typedef Loki::SingletonHolder<lg_dvdread_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DVDREAD;

#define LG_DVDREAD(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_DVDREAD::Instance(), x)<<LG_STATE
#define LG_DVDREAD_ER    LG_DVDREAD(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DVDREAD_W     LG_DVDREAD(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DVDREAD_I     LG_DVDREAD(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DVDREAD_D     LG_DVDREAD(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ipcctx_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ipcctx_class_name > lg_ipcctx_class;
    typedef Loki::SingletonHolder<lg_ipcctx_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IPCCTX;

#define LG_IPCCTX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IPCCTX::Instance(), x)<<LG_STATE
#define LG_IPCCTX_ER    LG_IPCCTX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IPCCTX_W     LG_IPCCTX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IPCCTX_I     LG_IPCCTX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IPCCTX_D     LG_IPCCTX(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_fclip_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_fclip_class_name > lg_fclip_class;
    typedef Loki::SingletonHolder<lg_fclip_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FCLIP;

#define LG_FCLIP(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FCLIP::Instance(), x)<<LG_STATE
#define LG_FCLIP_ER    LG_FCLIP(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FCLIP_W     LG_FCLIP(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FCLIP_I     LG_FCLIP(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FCLIP_D     LG_FCLIP(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_db4_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_db4_class_name > lg_db4_class;
    typedef Loki::SingletonHolder<lg_db4_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DB4;

#define LG_DB4(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_DB4::Instance(), x)<<LG_STATE
#define LG_DB4_ER    LG_DB4(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DB4_W     LG_DB4(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DB4_I     LG_DB4(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DB4_D     LG_DB4(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gdbm_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gdbm_class_name > lg_gdbm_class;
    typedef Loki::SingletonHolder<lg_gdbm_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GDBM;

#define LG_GDBM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GDBM::Instance(), x)<<LG_STATE
#define LG_GDBM_ER    LG_GDBM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GDBM_W     LG_GDBM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GDBM_I     LG_GDBM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GDBM_D     LG_GDBM(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_tdb_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_tdb_class_name > lg_tdb_class;
    typedef Loki::SingletonHolder<lg_tdb_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_TDB;

#define LG_TDB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_TDB::Instance(), x)<<LG_STATE
#define LG_TDB_ER    LG_TDB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_TDB_W     LG_TDB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_TDB_I     LG_TDB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_TDB_D     LG_TDB(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_commondb_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_commondb_class_name > lg_commondb_class;
    typedef Loki::SingletonHolder<lg_commondb_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_COMMONDB;

#define LG_COMMONDB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_COMMONDB::Instance(), x)<<LG_STATE
#define LG_COMMONDB_ER    LG_COMMONDB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_COMMONDB_W     LG_COMMONDB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_COMMONDB_I     LG_COMMONDB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_COMMONDB_D     LG_COMMONDB(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_eet_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_eet_class_name > lg_eet_class;
    typedef Loki::SingletonHolder<lg_eet_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EET;

#define LG_EET(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EET::Instance(), x)<<LG_STATE
#define LG_EET_ER    LG_EET(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EET_W     LG_EET(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EET_I     LG_EET(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EET_D     LG_EET(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_fnews_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_fnews_class_name > lg_fnews_class;
    typedef Loki::SingletonHolder<lg_fnews_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FNEWS;

#define LG_FNEWS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FNEWS::Instance(), x)<<LG_STATE
#define LG_FNEWS_ER    LG_FNEWS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FNEWS_W     LG_FNEWS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FNEWS_I     LG_FNEWS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FNEWS_D     LG_FNEWS(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_journal_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_journal_class_name > lg_journal_class;
    typedef Loki::SingletonHolder<lg_journal_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_JOURNAL;

#define LG_JOURNAL(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_JOURNAL::Instance(), x)<<LG_STATE
//#define LG_JOURNAL(x)    LG_SWITCH_VERBOSE(Ferris::Logging::LG_JOURNAL::Instance(), x)<<LG_STATE
#define LG_JOURNAL_ER    LG_JOURNAL(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_JOURNAL_W     LG_JOURNAL(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_JOURNAL_I     LG_JOURNAL(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_JOURNAL_D     LG_JOURNAL(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ldap_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ldap_class_name > lg_ldap_class;
    typedef Loki::SingletonHolder<lg_ldap_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_LDAP;

#define LG_LDAP(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_LDAP::Instance(), x)<<LG_STATE
#define LG_LDAP_ER    LG_LDAP(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_LDAP_W     LG_LDAP(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_LDAP_I     LG_LDAP(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_LDAP_D     LG_LDAP(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_exceptions_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_exceptions_class_name > lg_exceptions_class;
    typedef Loki::SingletonHolder<lg_exceptions_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EXCEPTIONS;

#define LG_EXCEPTIONS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EXCEPTIONS::Instance(), x)<<LG_STATE
#define LG_EXCEPTIONS_ER    LG_EXCEPTIONS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EXCEPTIONS_W     LG_EXCEPTIONS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EXCEPTIONS_I     LG_EXCEPTIONS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EXCEPTIONS_D     LG_EXCEPTIONS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ftxlexi_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ftxlexi_class_name > lg_ftxlexi_class;
    typedef Loki::SingletonHolder<lg_ftxlexi_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FTXLEXI;

#define LG_FTXLEXI(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FTXLEXI::Instance(), x)<<LG_STATE
#define LG_FTXLEXI_ER    LG_FTXLEXI(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FTXLEXI_W     LG_FTXLEXI(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FTXLEXI_I     LG_FTXLEXI(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FTXLEXI_D     LG_FTXLEXI(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_idx_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_idx_class_name > lg_idx_class;
    typedef Loki::SingletonHolder<lg_idx_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IDX;

#define LG_IDX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IDX::Instance(), x)<<LG_STATE
#define LG_IDX_ER    LG_IDX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IDX_W     LG_IDX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IDX_I     LG_IDX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IDX_D     LG_IDX(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_eaidx_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_eaidx_class_name > lg_eaidx_class;
    typedef Loki::SingletonHolder<lg_eaidx_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EAIDX;

#define LG_EAIDX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EAIDX::Instance(), x)<<LG_STATE
#define LG_EAIDX_ER    LG_EAIDX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EAIDX_W     LG_EAIDX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EAIDX_I     LG_EAIDX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EAIDX_D     LG_EAIDX(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_EAIDX_D_ACTIVE (Ferris::Logging::LG_EAIDX::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_create_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_create_class_name > lg_create_class;
    typedef Loki::SingletonHolder<lg_create_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_CREATE;

#define LG_CREATE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_CREATE::Instance(), x)<<LG_STATE
#define LG_CREATE_ER    LG_CREATE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_CREATE_W     LG_CREATE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_CREATE_I     LG_CREATE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_CREATE_D     LG_CREATE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_schema_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_schema_class_name > lg_schema_class;
    typedef Loki::SingletonHolder<lg_schema_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SCHEMA;

#define LG_SCHEMA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SCHEMA::Instance(), x)<<LG_STATE
#define LG_SCHEMA_ER    LG_SCHEMA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SCHEMA_W     LG_SCHEMA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SCHEMA_I     LG_SCHEMA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SCHEMA_D     LG_SCHEMA(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_emblem_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_emblem_class_name > lg_emblem_class;
    typedef Loki::SingletonHolder<lg_emblem_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EMBLEM;

#define LG_EMBLEM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EMBLEM::Instance(), x)<<LG_STATE
#define LG_EMBLEM_ER    LG_EMBLEM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EMBLEM_W     LG_EMBLEM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EMBLEM_I     LG_EMBLEM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EMBLEM_D     LG_EMBLEM(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_rdf_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_rdf_class_name > lg_rdf_class;
    typedef Loki::SingletonHolder<lg_rdf_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RDF;

#define LG_RDF(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RDF::Instance(), x)<<LG_STATE
#define LG_RDF_ER    LG_RDF(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RDF_W     LG_RDF(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RDF_I     LG_RDF(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RDF_D     LG_RDF(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_exif_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_exif_class_name > lg_exif_class;
    typedef Loki::SingletonHolder<lg_exif_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EXIF;

#define LG_EXIF(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EXIF::Instance(), x)<<LG_STATE
#define LG_EXIF_ER    LG_EXIF(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EXIF_W     LG_EXIF(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EXIF_I     LG_EXIF(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EXIF_D     LG_EXIF(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ctxrec_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ctxrec_class_name > lg_ctxrec_class;
    typedef Loki::SingletonHolder<lg_ctxrec_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_CTXREC;

#define LG_CTXREC(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_CTXREC::Instance(), x)<<LG_STATE
#define LG_CTXREC_ER    LG_CTXREC(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_CTXREC_W     LG_CTXREC(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_CTXREC_I     LG_CTXREC(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_CTXREC_D     LG_CTXREC(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_rtimeparse_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_rtimeparse_class_name > lg_rtimeparse_class;
    typedef Loki::SingletonHolder<lg_rtimeparse_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RTIMEPARSE;

#define LG_RTIMEPARSE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RTIMEPARSE::Instance(), x)<<LG_STATE
#define LG_RTIMEPARSE_ER    LG_RTIMEPARSE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RTIMEPARSE_W     LG_RTIMEPARSE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RTIMEPARSE_I     LG_RTIMEPARSE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RTIMEPARSE_D     LG_RTIMEPARSE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_libextractor_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_libextractor_class_name > lg_libextractor_class;
    typedef Loki::SingletonHolder<lg_libextractor_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_LIBEXTRACTOR;

#define LG_LIBEXTRACTOR(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_LIBEXTRACTOR::Instance(), x)<<LG_STATE
#define LG_LIBEXTRACTOR_ER    LG_LIBEXTRACTOR(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_LIBEXTRACTOR_W     LG_LIBEXTRACTOR(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_LIBEXTRACTOR_I     LG_LIBEXTRACTOR(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_LIBEXTRACTOR_D     LG_LIBEXTRACTOR(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gphoto_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gphoto_class_name > lg_gphoto_class;
    typedef Loki::SingletonHolder<lg_gphoto_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GPHOTO;

#define LG_GPHOTO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GPHOTO::Instance(), x)<<LG_STATE
#define LG_GPHOTO_ER    LG_GPHOTO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GPHOTO_W     LG_GPHOTO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GPHOTO_I     LG_GPHOTO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GPHOTO_D     LG_GPHOTO(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_fca_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_fca_class_name > lg_fca_class;
    typedef Loki::SingletonHolder<lg_fca_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FCA;

#define LG_FCA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FCA::Instance(), x)<<LG_STATE
#define LG_FCA_ER    LG_FCA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FCA_W     LG_FCA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FCA_I     LG_FCA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FCA_D     LG_FCA(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FCA_ACTIVE (Ferris::Logging::LG_FCA::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_evo_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_evo_class_name > lg_evo_class;
    typedef Loki::SingletonHolder<lg_evo_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EVO;

#define LG_EVO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EVO::Instance(), x)<<LG_STATE
#define LG_EVO_ER    LG_EVO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EVO_W     LG_EVO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EVO_I     LG_EVO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EVO_D     LG_EVO(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_samba_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_samba_class_name > lg_samba_class;
    typedef Loki::SingletonHolder<lg_samba_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SAMBA;

#define LG_SAMBA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SAMBA::Instance(), x)<<LG_STATE
#define LG_SAMBA_ER    LG_SAMBA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SAMBA_W     LG_SAMBA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SAMBA_I     LG_SAMBA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SAMBA_D     LG_SAMBA(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_pg_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_pg_class_name > lg_pg_class;
    typedef Loki::SingletonHolder<lg_pg_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PG;

#define LG_PG(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PG::Instance(), x)<<LG_STATE
#define LG_PG_ER    LG_PG(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PG_W     LG_PG(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PG_I     LG_PG(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PG_D     LG_PG(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PG_ACTIVE (Ferris::Logging::LG_PG::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_bibtex_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_bibtex_class_name > lg_bibtex_class;
    typedef Loki::SingletonHolder<lg_bibtex_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_BIBTEX;

#define LG_BIBTEX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_BIBTEX::Instance(), x)<<LG_STATE
#define LG_BIBTEX_ER    LG_BIBTEX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_BIBTEX_W     LG_BIBTEX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_BIBTEX_I     LG_BIBTEX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_BIBTEX_D     LG_BIBTEX(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_kde_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_kde_class_name > lg_kde_class;
    typedef Loki::SingletonHolder<lg_kde_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_KDE;

#define LG_KDE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_KDE::Instance(), x)<<LG_STATE
#define LG_KDE_ER    LG_KDE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_KDE_W     LG_KDE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_KDE_I     LG_KDE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_KDE_D     LG_KDE(Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_annodex_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_annodex_class_name > lg_annodex_class;
    typedef Loki::SingletonHolder<lg_annodex_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ANNODEX;

#define LG_ANNODEX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ANNODEX::Instance(), x)<<LG_STATE
#define LG_ANNODEX_ER    LG_ANNODEX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ANNODEX_W     LG_ANNODEX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ANNODEX_I     LG_ANNODEX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ANNODEX_D     LG_ANNODEX(Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_astext_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_astext_class_name > lg_astext_class;
    typedef Loki::SingletonHolder<lg_astext_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ASTEXT;

#define LG_ASTEXT(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ASTEXT::Instance(), x)<<LG_STATE
#define LG_ASTEXT_ER    LG_ASTEXT(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ASTEXT_W     LG_ASTEXT(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ASTEXT_I     LG_ASTEXT(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ASTEXT_D     LG_ASTEXT(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_fileactions_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_fileactions_class_name > lg_fileactions_class;
    typedef Loki::SingletonHolder<lg_fileactions_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FILEACTIONS;

#define LG_FILEACTIONS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FILEACTIONS::Instance(), x)<<LG_STATE
#define LG_FILEACTIONS_ER    LG_FILEACTIONS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FILEACTIONS_W     LG_FILEACTIONS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FILEACTIONS_I     LG_FILEACTIONS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FILEACTIONS_D     LG_FILEACTIONS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_emacs_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_emacs_class_name > lg_emacs_class;
    typedef Loki::SingletonHolder<lg_emacs_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EMACS;

#define LG_EMACS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EMACS::Instance(), x)<<LG_STATE
#define LG_EMACS_ER    LG_EMACS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EMACS_W     LG_EMACS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EMACS_I     LG_EMACS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EMACS_D     LG_EMACS(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_firefox_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_firefox_class_name > lg_firefox_class;
    typedef Loki::SingletonHolder<lg_firefox_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FIREFOX;

#define LG_FIREFOX(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FIREFOX::Instance(), x)<<LG_STATE
#define LG_FIREFOX_ER    LG_FIREFOX(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FIREFOX_W     LG_FIREFOX(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FIREFOX_I     LG_FIREFOX(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FIREFOX_D     LG_FIREFOX(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_iconsrv_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_iconsrv_class_name > lg_iconsrv_class;
    typedef Loki::SingletonHolder<lg_iconsrv_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ICONSRV;

#define LG_ICONSRV(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ICONSRV::Instance(), x)<<LG_STATE
#define LG_ICONSRV_ER    LG_ICONSRV(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ICONSRV_W     LG_ICONSRV(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ICONSRV_I     LG_ICONSRV(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ICONSRV_D     LG_ICONSRV(Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xine_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xine_class_name > lg_xine_class;
    typedef Loki::SingletonHolder<lg_xine_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XINE;

#define LG_XINE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_XINE::Instance(), x)<<LG_STATE
#define LG_XINE_ER    LG_XINE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XINE_W     LG_XINE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XINE_I     LG_XINE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XINE_D     LG_XINE(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_obby_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_obby_class_name > lg_obby_class;
    typedef Loki::SingletonHolder<lg_obby_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_OBBY;

#define LG_OBBY(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_OBBY::Instance(), x)<<LG_STATE
#define LG_OBBY_ER    LG_OBBY(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_OBBY_W     LG_OBBY(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_OBBY_I     LG_OBBY(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_OBBY_D     LG_OBBY(Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xsltfs_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xsltfs_class_name > lg_xsltfs_class;
    typedef Loki::SingletonHolder<lg_xsltfs_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XSLTFS;

#define LG_XSLTFS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_XSLTFS::Instance(), x)<<LG_STATE
#define LG_XSLTFS_ER    LG_XSLTFS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XSLTFS_W     LG_XSLTFS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XSLTFS_I     LG_XSLTFS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XSLTFS_D     LG_XSLTFS(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_XSLTFS_D_ACTIVE (Ferris::Logging::LG_XSLTFS::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_rdfattrcache_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_rdfattrcache_class_name > lg_rdfattrcache_class;
    typedef Loki::SingletonHolder<lg_rdfattrcache_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RDFATTRCACHE;

#define LG_RDFATTRCACHE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RDFATTRCACHE::Instance(), x)<<LG_STATE
#define LG_RDFATTRCACHE_ER    LG_RDFATTRCACHE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RDFATTRCACHE_W     LG_RDFATTRCACHE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RDFATTRCACHE_I     LG_RDFATTRCACHE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RDFATTRCACHE_D     LG_RDFATTRCACHE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_RDFATTRCACHE_D_ACTIVE (Ferris::Logging::LG_RDFATTRCACHE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ffuse_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ffuse_class_name > lg_ffuse_class;
    typedef Loki::SingletonHolder<lg_ffuse_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FFUSE;

#define LG_FFUSE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FFUSE::Instance(), x)<<LG_STATE
#define LG_FFUSE_ER    LG_FFUSE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FFUSE_W     LG_FFUSE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FFUSE_I     LG_FFUSE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FFUSE_D     LG_FFUSE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FFUSE_D_ACTIVE (Ferris::Logging::LG_FFUSE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_beagle_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_beagle_class_name > lg_beagle_class;
    typedef Loki::SingletonHolder<lg_beagle_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_BEAGLE;

#define LG_BEAGLE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_BEAGLE::Instance(), x)<<LG_STATE
#define LG_BEAGLE_ER    LG_BEAGLE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_BEAGLE_W     LG_BEAGLE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_BEAGLE_I     LG_BEAGLE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_BEAGLE_D     LG_BEAGLE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_BEAGLE_D_ACTIVE (Ferris::Logging::LG_BEAGLE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_glob_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_glob_class_name > lg_glob_class;
    typedef Loki::SingletonHolder<lg_glob_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GLOB;

#define LG_GLOB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GLOB::Instance(), x)<<LG_STATE
#define LG_GLOB_ER    LG_GLOB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GLOB_W     LG_GLOB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GLOB_I     LG_GLOB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GLOB_D     LG_GLOB(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_GLOB_D_ACTIVE (Ferris::Logging::LG_GLOB::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_amarok_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_amarok_class_name > lg_amarok_class;
    typedef Loki::SingletonHolder<lg_amarok_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_AMAROK;

#define LG_AMAROK(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_AMAROK::Instance(), x)<<LG_STATE
#define LG_AMAROK_ER    LG_AMAROK(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_AMAROK_W     LG_AMAROK(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_AMAROK_I     LG_AMAROK(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_AMAROK_D     LG_AMAROK(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_AMAROK_D_ACTIVE (Ferris::Logging::LG_AMAROK::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xmms_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xmms_class_name > lg_xmms_class;
    typedef Loki::SingletonHolder<lg_xmms_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XMMS;

#define LG_XMMS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_XMMS::Instance(), x)<<LG_STATE
#define LG_XMMS_ER    LG_XMMS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XMMS_W     LG_XMMS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XMMS_I     LG_XMMS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XMMS_D     LG_XMMS(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_XMMS_D_ACTIVE (Ferris::Logging::LG_XMMS::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_private_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_private_class_name > lg_private_class;
    typedef Loki::SingletonHolder<lg_private_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PRIVATE;

#define LG_PRIVATE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PRIVATE::Instance(), x)<<LG_STATE
#define LG_PRIVATE_ER    LG_PRIVATE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PRIVATE_W     LG_PRIVATE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PRIVATE_I     LG_PRIVATE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PRIVATE_D     LG_PRIVATE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PRIVATE_D_ACTIVE (Ferris::Logging::LG_PRIVATE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_dbus_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_dbus_class_name > lg_dbus_class;
    typedef Loki::SingletonHolder<lg_dbus_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_DBUS;

#define LG_DBUS(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_DBUS::Instance(), x)<<LG_STATE
#define LG_DBUS_ER    LG_DBUS(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_DBUS_W     LG_DBUS(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_DBUS_I     LG_DBUS(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_DBUS_D     LG_DBUS(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_DBUS_D_ACTIVE (Ferris::Logging::LG_DBUS::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_xqilla_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_xqilla_class_name > lg_xqilla_class;
    typedef Loki::SingletonHolder<lg_xqilla_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_XQILLA;

#define LG_XQILLA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_XQILLA::Instance(), x)<<LG_STATE
#define LG_XQILLA_ER    LG_XQILLA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_XQILLA_W     LG_XQILLA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_XQILLA_I     LG_XQILLA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_XQILLA_D     LG_XQILLA(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_XQILLA_D_ACTIVE (Ferris::Logging::LG_XQILLA::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_webphoto_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_webphoto_class_name > lg_webphoto_class;
    typedef Loki::SingletonHolder<lg_webphoto_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_WEBPHOTO;

#define LG_WEBPHOTO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_WEBPHOTO::Instance(), x)<<LG_STATE
#define LG_WEBPHOTO_ER    LG_WEBPHOTO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_WEBPHOTO_W     LG_WEBPHOTO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_WEBPHOTO_I     LG_WEBPHOTO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_WEBPHOTO_D     LG_WEBPHOTO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_WEBPHOTO_D_ACTIVE (Ferris::Logging::LG_WEBPHOTO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_fspot_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_fspot_class_name > lg_fspot_class;
    typedef Loki::SingletonHolder<lg_fspot_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FSPOT;

#define LG_FSPOT(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FSPOT::Instance(), x)<<LG_STATE
#define LG_FSPOT_ER    LG_FSPOT(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FSPOT_W     LG_FSPOT(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FSPOT_I     LG_FSPOT(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FSPOT_D     LG_FSPOT(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FSPOT_D_ACTIVE (Ferris::Logging::LG_FSPOT::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_strigi_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_strigi_class_name > lg_strigi_class;
    typedef Loki::SingletonHolder<lg_strigi_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_STRIGI;

#define LG_STRIGI(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_STRIGI::Instance(), x)<<LG_STATE
#define LG_STRIGI_ER    LG_STRIGI(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_STRIGI_W     LG_STRIGI(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_STRIGI_I     LG_STRIGI(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_STRIGI_D     LG_STRIGI(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_STRIGI_D_ACTIVE (Ferris::Logging::LG_STRIGI::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_mdserv_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_mdserv_class_name > lg_mdserv_class;
    typedef Loki::SingletonHolder<lg_mdserv_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_MDSERV;

#define LG_MDSERV(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_MDSERV::Instance(), x)<<LG_STATE
#define LG_MDSERV_ER    LG_MDSERV(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_MDSERV_W     LG_MDSERV(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_MDSERV_I     LG_MDSERV(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_MDSERV_D     LG_MDSERV(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_MDSERV_D_ACTIVE (Ferris::Logging::LG_MDSERV::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_volman_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_volman_class_name > lg_volman_class;
    typedef Loki::SingletonHolder<lg_volman_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_VOLMAN;

#define LG_VOLMAN(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_VOLMAN::Instance(), x)<<LG_STATE
#define LG_VOLMAN_ER    LG_VOLMAN(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_VOLMAN_W     LG_VOLMAN(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_VOLMAN_I     LG_VOLMAN(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_VOLMAN_D     LG_VOLMAN(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_VOLMAN_D_ACTIVE (Ferris::Logging::LG_VOLMAN::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_hal_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_hal_class_name > lg_hal_class;
    typedef Loki::SingletonHolder<lg_hal_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_HAL;

#define LG_HAL(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_HAL::Instance(), x)<<LG_STATE
#define LG_HAL_ER    LG_HAL(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_HAL_W     LG_HAL(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_HAL_I     LG_HAL(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_HAL_D     LG_HAL(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_HAL_D_ACTIVE (Ferris::Logging::LG_HAL::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ego_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ego_class_name > lg_ego_class;
    typedef Loki::SingletonHolder<lg_ego_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_EGO;

#define LG_EGO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_EGO::Instance(), x)<<LG_STATE
#define LG_EGO_ER    LG_EGO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_EGO_W     LG_EGO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_EGO_I     LG_EGO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_EGO_D     LG_EGO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_EGO_D_ACTIVE (Ferris::Logging::LG_EGO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_bgproc_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_bgproc_class_name > lg_bgproc_class;
    typedef Loki::SingletonHolder<lg_bgproc_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_BGPROC;

#define LG_BGPROC(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_BGPROC::Instance(), x)<<LG_STATE
#define LG_BGPROC_ER    LG_BGPROC(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_BGPROC_W     LG_BGPROC(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_BGPROC_I     LG_BGPROC(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_BGPROC_D     LG_BGPROC(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_BGPROC_D_ACTIVE (Ferris::Logging::LG_BGPROC::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_flac_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_flac_class_name > lg_flac_class;
    typedef Loki::SingletonHolder<lg_flac_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FLAC;

#define LG_FLAC(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FLAC::Instance(), x)<<LG_STATE
#define LG_FLAC_ER    LG_FLAC(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FLAC_W     LG_FLAC(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FLAC_I     LG_FLAC(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FLAC_D     LG_FLAC(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FLAC_D_ACTIVE (Ferris::Logging::LG_FLAC::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_oggz_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_oggz_class_name > lg_oggz_class;
    typedef Loki::SingletonHolder<lg_oggz_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_OGGZ;

#define LG_OGGZ(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_OGGZ::Instance(), x)<<LG_STATE
#define LG_OGGZ_ER    LG_OGGZ(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_OGGZ_W     LG_OGGZ(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_OGGZ_I     LG_OGGZ(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_OGGZ_D     LG_OGGZ(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_OGGZ_D_ACTIVE (Ferris::Logging::LG_OGGZ::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_taglib_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_taglib_class_name > lg_taglib_class;
    typedef Loki::SingletonHolder<lg_taglib_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_TAGLIB;

#define LG_TAGLIB(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_TAGLIB::Instance(), x)<<LG_STATE
#define LG_TAGLIB_ER    LG_TAGLIB(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_TAGLIB_W     LG_TAGLIB(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_TAGLIB_I     LG_TAGLIB(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_TAGLIB_D     LG_TAGLIB(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_TAGLIB_D_ACTIVE (Ferris::Logging::LG_TAGLIB::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ffind_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ffind_class_name > lg_ffind_class;
    typedef Loki::SingletonHolder<lg_ffind_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FFIND;

#define LG_FFIND(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FFIND::Instance(), x)<<LG_STATE
#define LG_FFIND_ER    LG_FFIND(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FFIND_W     LG_FFIND(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FFIND_I     LG_FFIND(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FFIND_D     LG_FFIND(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FFIND_D_ACTIVE (Ferris::Logging::LG_FFIND::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_qtsql_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_qtsql_class_name > lg_qtsql_class;
    typedef Loki::SingletonHolder<lg_qtsql_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_QTSQL;

#define LG_QTSQL(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_QTSQL::Instance(), x)<<LG_STATE
#define LG_QTSQL_ER    LG_QTSQL(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_QTSQL_W     LG_QTSQL(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_QTSQL_I     LG_QTSQL(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_QTSQL_D     LG_QTSQL(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_QTSQL_D_ACTIVE (Ferris::Logging::LG_QTSQL::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_sqlite_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_sqlite_class_name > lg_sqlite_class;
    typedef Loki::SingletonHolder<lg_sqlite_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SQLITE;

#define LG_SQLITE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SQLITE::Instance(), x)<<LG_STATE
#define LG_SQLITE_ER    LG_SQLITE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SQLITE_W     LG_SQLITE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SQLITE_I     LG_SQLITE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SQLITE_D     LG_SQLITE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_SQLITE_D_ACTIVE (Ferris::Logging::LG_SQLITE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_recordfile_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_recordfile_class_name > lg_recordfile_class;
    typedef Loki::SingletonHolder<lg_recordfile_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_RECORDFILE;

#define LG_RECORDFILE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_RECORDFILE::Instance(), x)<<LG_STATE
#define LG_RECORDFILE_ER    LG_RECORDFILE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_RECORDFILE_W     LG_RECORDFILE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_RECORDFILE_I     LG_RECORDFILE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_RECORDFILE_D     LG_RECORDFILE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_RECORDFILE_D_ACTIVE (Ferris::Logging::LG_RECORDFILE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_google_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_google_class_name > lg_google_class;
    typedef Loki::SingletonHolder<lg_google_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GOOGLE;

#define LG_GOOGLE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GOOGLE::Instance(), x)<<LG_STATE
#define LG_GOOGLE_ER    LG_GOOGLE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GOOGLE_W     LG_GOOGLE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GOOGLE_I     LG_GOOGLE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GOOGLE_D     LG_GOOGLE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_GOOGLE_D_ACTIVE (Ferris::Logging::LG_GOOGLE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_boxcom_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_boxcom_class_name > lg_boxcom_class;
    typedef Loki::SingletonHolder<lg_boxcom_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_BOXCOM;

#define LG_BOXCOM(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_BOXCOM::Instance(), x)<<LG_STATE
#define LG_BOXCOM_ER    LG_BOXCOM(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_BOXCOM_W     LG_BOXCOM(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_BOXCOM_I     LG_BOXCOM(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_BOXCOM_D     LG_BOXCOM(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_BOXCOM_D_ACTIVE (Ferris::Logging::LG_BOXCOM::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_facebook_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_facebook_class_name > lg_facebook_class;
    typedef Loki::SingletonHolder<lg_facebook_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FACEBOOK;

#define LG_FACEBOOK(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FACEBOOK::Instance(), x)<<LG_STATE
#define LG_FACEBOOK_ER    LG_FACEBOOK(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FACEBOOK_W     LG_FACEBOOK(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FACEBOOK_I     LG_FACEBOOK(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FACEBOOK_D     LG_FACEBOOK(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FACEBOOK_D_ACTIVE (Ferris::Logging::LG_FACEBOOK::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_vimeo_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_vimeo_class_name > lg_vimeo_class;
    typedef Loki::SingletonHolder<lg_vimeo_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_VIMEO;

#define LG_VIMEO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_VIMEO::Instance(), x)<<LG_STATE
#define LG_VIMEO_ER    LG_VIMEO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_VIMEO_W     LG_VIMEO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_VIMEO_I     LG_VIMEO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_VIMEO_D     LG_VIMEO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_VIMEO_D_ACTIVE (Ferris::Logging::LG_VIMEO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_qio_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_qio_class_name > lg_qio_class;
    typedef Loki::SingletonHolder<lg_qio_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_QIO;

#define LG_QIO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_QIO::Instance(), x)<<LG_STATE
#define LG_QIO_ER    LG_QIO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_QIO_W     LG_QIO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_QIO_I     LG_QIO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_QIO_D     LG_QIO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_QIO_D_ACTIVE (Ferris::Logging::LG_QIO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_webservice_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_webservice_class_name > lg_webservice_class;
    typedef Loki::SingletonHolder<lg_webservice_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_WEBSERVICE;

#define LG_WEBSERVICE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_WEBSERVICE::Instance(), x)<<LG_STATE
#define LG_WEBSERVICE_ER    LG_WEBSERVICE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_WEBSERVICE_W     LG_WEBSERVICE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_WEBSERVICE_I     LG_WEBSERVICE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_WEBSERVICE_D     LG_WEBSERVICE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_WEBSERVICE_D_ACTIVE (Ferris::Logging::LG_WEBSERVICE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gstreamer_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gstreamer_class_name > lg_gstreamer_class;
    typedef Loki::SingletonHolder<lg_gstreamer_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GSTREAMER;

#define LG_GSTREAMER(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GSTREAMER::Instance(), x)<<LG_STATE
#define LG_GSTREAMER_ER    LG_GSTREAMER(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GSTREAMER_W     LG_GSTREAMER(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GSTREAMER_I     LG_GSTREAMER(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GSTREAMER_D     LG_GSTREAMER(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_GSTREAMER_D_ACTIVE (Ferris::Logging::LG_GSTREAMER::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_upnp_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_upnp_class_name > lg_upnp_class;
    typedef Loki::SingletonHolder<lg_upnp_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_UPNP;

#define LG_UPNP(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_UPNP::Instance(), x)<<LG_STATE
#define LG_UPNP_ER    LG_UPNP(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_UPNP_W     LG_UPNP(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_UPNP_I     LG_UPNP(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_UPNP_D     LG_UPNP(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_UPNP_D_ACTIVE (Ferris::Logging::LG_UPNP::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_useroverlay_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_useroverlay_class_name > lg_useroverlay_class;
    typedef Loki::SingletonHolder<lg_useroverlay_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_USEROVERLAY;

#define LG_USEROVERLAY(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_USEROVERLAY::Instance(), x)<<LG_STATE
#define LG_USEROVERLAY_ER    LG_USEROVERLAY(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_USEROVERLAY_W     LG_USEROVERLAY(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_USEROVERLAY_I     LG_USEROVERLAY(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_USEROVERLAY_D     LG_USEROVERLAY(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_USEROVERLAY_D_ACTIVE (Ferris::Logging::LG_USEROVERLAY::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_plasma_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_plasma_class_name > lg_plasma_class;
    typedef Loki::SingletonHolder<lg_plasma_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PLASMA;

#define LG_PLASMA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PLASMA::Instance(), x)<<LG_STATE
#define LG_PLASMA_ER    LG_PLASMA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PLASMA_W     LG_PLASMA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PLASMA_I     LG_PLASMA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PLASMA_D     LG_PLASMA(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PLASMA_D_ACTIVE (Ferris::Logging::LG_PLASMA::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_libmediainfo_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_libmediainfo_class_name > lg_libmediainfo_class;
    typedef Loki::SingletonHolder<lg_libmediainfo_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_LIBMEDIAINFO;

#define LG_LIBMEDIAINFO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_LIBMEDIAINFO::Instance(), x)<<LG_STATE
#define LG_LIBMEDIAINFO_ER    LG_LIBMEDIAINFO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_LIBMEDIAINFO_W     LG_LIBMEDIAINFO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_LIBMEDIAINFO_I     LG_LIBMEDIAINFO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_LIBMEDIAINFO_D     LG_LIBMEDIAINFO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_LIBMEDIAINFO_D_ACTIVE (Ferris::Logging::LG_LIBMEDIAINFO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_subtitles_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_subtitles_class_name > lg_subtitles_class;
    typedef Loki::SingletonHolder<lg_subtitles_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SUBTITLES;

#define LG_SUBTITLES(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SUBTITLES::Instance(), x)<<LG_STATE
#define LG_SUBTITLES_ER    LG_SUBTITLES(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SUBTITLES_W     LG_SUBTITLES(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SUBTITLES_I     LG_SUBTITLES(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SUBTITLES_D     LG_SUBTITLES(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_SUBTITLES_D_ACTIVE (Ferris::Logging::LG_SUBTITLES::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ferrisqtgui_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ferrisqtgui_class_name > lg_ferrisqtgui_class;
    typedef Loki::SingletonHolder<lg_ferrisqtgui_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FERRISQTGUI;

#define LG_FERRISQTGUI(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FERRISQTGUI::Instance(), x)<<LG_STATE
#define LG_FERRISQTGUI_ER    LG_FERRISQTGUI(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FERRISQTGUI_W     LG_FERRISQTGUI(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FERRISQTGUI_I     LG_FERRISQTGUI(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FERRISQTGUI_D     LG_FERRISQTGUI(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FERRISQTGUI_D_ACTIVE (Ferris::Logging::LG_FERRISQTGUI::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_sane_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_sane_class_name > lg_sane_class;
    typedef Loki::SingletonHolder<lg_sane_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SANE;

#define LG_SANE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SANE::Instance(), x)<<LG_STATE
#define LG_SANE_ER    LG_SANE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SANE_W     LG_SANE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SANE_I     LG_SANE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SANE_D     LG_SANE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_SANE_D_ACTIVE (Ferris::Logging::LG_SANE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_printer_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_printer_class_name > lg_printer_class;
    typedef Loki::SingletonHolder<lg_printer_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PRINTER;

#define LG_PRINTER(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PRINTER::Instance(), x)<<LG_STATE
#define LG_PRINTER_ER    LG_PRINTER(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PRINTER_W     LG_PRINTER(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PRINTER_I     LG_PRINTER(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PRINTER_D     LG_PRINTER(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PRINTER_D_ACTIVE (Ferris::Logging::LG_PRINTER::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_pulseaudio_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_pulseaudio_class_name > lg_pulseaudio_class;
    typedef Loki::SingletonHolder<lg_pulseaudio_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PULSEAUDIO;

#define LG_PULSEAUDIO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PULSEAUDIO::Instance(), x)<<LG_STATE
#define LG_PULSEAUDIO_ER    LG_PULSEAUDIO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PULSEAUDIO_W     LG_PULSEAUDIO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PULSEAUDIO_I     LG_PULSEAUDIO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PULSEAUDIO_D     LG_PULSEAUDIO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PULSEAUDIO_D_ACTIVE (Ferris::Logging::LG_PULSEAUDIO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_wiki_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_wiki_class_name > lg_wiki_class;
    typedef Loki::SingletonHolder<lg_wiki_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_WIKI;

#define LG_WIKI(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_WIKI::Instance(), x)<<LG_STATE
#define LG_WIKI_ER    LG_WIKI(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_WIKI_W     LG_WIKI(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_WIKI_I     LG_WIKI(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_WIKI_D     LG_WIKI(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_WIKI_D_ACTIVE (Ferris::Logging::LG_WIKI::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_spiritcontext_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_spiritcontext_class_name > lg_spiritcontext_class;
    typedef Loki::SingletonHolder<lg_spiritcontext_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_SPIRITCONTEXT;

#define LG_SPIRITCONTEXT(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_SPIRITCONTEXT::Instance(), x)<<LG_STATE
#define LG_SPIRITCONTEXT_ER    LG_SPIRITCONTEXT(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_SPIRITCONTEXT_W     LG_SPIRITCONTEXT(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_SPIRITCONTEXT_I     LG_SPIRITCONTEXT(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_SPIRITCONTEXT_D     LG_SPIRITCONTEXT(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_SPIRITCONTEXT_D_ACTIVE (Ferris::Logging::LG_SPIRITCONTEXT::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_pastebin_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_pastebin_class_name > lg_pastebin_class;
    typedef Loki::SingletonHolder<lg_pastebin_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_PASTEBIN;

#define LG_PASTEBIN(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_PASTEBIN::Instance(), x)<<LG_STATE
#define LG_PASTEBIN_ER    LG_PASTEBIN(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_PASTEBIN_W     LG_PASTEBIN(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_PASTEBIN_I     LG_PASTEBIN(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_PASTEBIN_D     LG_PASTEBIN(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_PASTEBIN_D_ACTIVE (Ferris::Logging::LG_PASTEBIN::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_identica_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_identica_class_name > lg_identica_class;
    typedef Loki::SingletonHolder<lg_identica_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_IDENTICA;

#define LG_IDENTICA(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_IDENTICA::Instance(), x)<<LG_STATE
#define LG_IDENTICA_ER    LG_IDENTICA(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_IDENTICA_W     LG_IDENTICA(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_IDENTICA_I     LG_IDENTICA(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_IDENTICA_D     LG_IDENTICA(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_IDENTICA_D_ACTIVE (Ferris::Logging::LG_IDENTICA::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_zoneminder_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_zoneminder_class_name > lg_zoneminder_class;
    typedef Loki::SingletonHolder<lg_zoneminder_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_ZONEMINDER;

#define LG_ZONEMINDER(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_ZONEMINDER::Instance(), x)<<LG_STATE
#define LG_ZONEMINDER_ER    LG_ZONEMINDER(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_ZONEMINDER_W     LG_ZONEMINDER(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_ZONEMINDER_I     LG_ZONEMINDER(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_ZONEMINDER_D     LG_ZONEMINDER(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_ZONEMINDER_D_ACTIVE (Ferris::Logging::LG_ZONEMINDER::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_ferrisrest_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_ferrisrest_class_name > lg_ferrisrest_class;
    typedef Loki::SingletonHolder<lg_ferrisrest_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_FERRISREST;

#define LG_FERRISREST(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_FERRISREST::Instance(), x)<<LG_STATE
#define LG_FERRISREST_ER    LG_FERRISREST(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_FERRISREST_W     LG_FERRISREST(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_FERRISREST_I     LG_FERRISREST(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_FERRISREST_D     LG_FERRISREST(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_FERRISREST_D_ACTIVE (Ferris::Logging::LG_FERRISREST::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_kio_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_kio_class_name > lg_kio_class;
    typedef Loki::SingletonHolder<lg_kio_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_KIO;

#define LG_KIO(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_KIO::Instance(), x)<<LG_STATE
#define LG_KIO_ER    LG_KIO(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_KIO_W     LG_KIO(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_KIO_I     LG_KIO(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_KIO_D     LG_KIO(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_KIO_D_ACTIVE (Ferris::Logging::LG_KIO::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    extern FERRISEXP_API char const lg_gdrive_class_name[];
    typedef LogState< Ferris::Timber::_SBufT::FAC_L6, lg_gdrive_class_name > lg_gdrive_class;
    typedef Loki::SingletonHolder<lg_gdrive_class, Loki::CreateUsingNew, Loki::NoDestroy > LG_GDRIVE;

#define LG_GDRIVE(x)    LOGGING_SWITCH_FUNCTION(Ferris::Logging::LG_GDRIVE::Instance(), x)<<LG_STATE
#define LG_GDRIVE_ER    LG_GDRIVE(Ferris::Timber::_SBufT::PRI_ERR)
#define LG_GDRIVE_W     LG_GDRIVE(Ferris::Timber::_SBufT::PRI_WARNING)
#define LG_GDRIVE_I     LG_GDRIVE(Ferris::Timber::_SBufT::PRI_INFO)
#define LG_GDRIVE_D     LG_GDRIVE(Ferris::Timber::_SBufT::PRI_DEBUG)
#define LG_GDRIVE_D_ACTIVE (Ferris::Logging::LG_GDRIVE::Instance().state() & Ferris::Timber::_SBufT::PRI_DEBUG)

    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    
    
    
};
 
};


#endif // ifndef _ALREADY_INCLUDED_ENAMEL_H_

