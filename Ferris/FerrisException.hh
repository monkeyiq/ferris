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

    $Id: FerrisException.hh,v 1.11 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#ifndef _ALREADY_INCLUDED_FERRIS_EXCEPTION_H_
#define _ALREADY_INCLUDED_FERRIS_EXCEPTION_H_

#include <exception>
#include <string>

#include <Ferris/TypeDecl.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/Enamel.hh>
#include <Fampp2.hh>

namespace Ferris
{

    /**
     * Exception logging is a little different from using LG_XXX_D because we
     * need to provide both a +ve and -ve stream and be able to have the macro
     * inlined for passing the code as a function argument.
     */
#define LG_EXCEPTION_SWITCH( neededState )                                \
     (Logging::LG_EXCEPTIONS::Instance().state() & neededState)           \
     ? Logging::LG_EXCEPTIONS::Instance().getRealStream( neededState )    \
     : ::Ferris::Factory::getNullTimber()    


    
    
class FERRISEXP_EXCEPTION FerrisVFSExceptionBase  : public FerrisExceptionBase
{
    typedef FerrisExceptionBase _Base;
    
    Attribute* Attr;
    mutable std::string m_cache;
    
protected:

    inline void setAttribute( Attribute* a )
        {
            Attr = a;
        }
    
public:

    FerrisVFSExceptionBase( const FerrisException_CodeState& state,
                         fh_ostream log,
                         const char* e = "",
                         Attribute* a = 0 );
    virtual ~FerrisVFSExceptionBase() throw ();
    
    virtual const std::string& whats() const;
};


class FERRISEXP_EXCEPTION FerrisGLibCException : public FerrisVFSExceptionBase
{
public:

    inline FerrisGLibCException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisGLibCException");
        }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisStatException : public FerrisGLibCException
{
public:

    inline FerrisStatException(
        const FerrisException_CodeState& state,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisGLibCException( state, (Enamel::get__t_l1er()), e, a )
        {
            setExceptionName("FerrisStatException");
        }
};
#define Throw_FerrisStatException(e,a) \
throw FerrisStatException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisSetCWDException : public FerrisGLibCException
{
public:

    inline FerrisSetCWDException(
        const FerrisException_CodeState& state,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisGLibCException( state, (Enamel::get__t_l1er()), e, a )
        {
            setExceptionName("FerrisSetCWDException");
        }
};
#define Throw_FerrisSetCWDException(e,a) \
throw FerrisSetCWDException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_EXCEPTION CanNotGetStream : public FerrisVFSExceptionBase
{
public:

    inline CanNotGetStream(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotGetStream");
        }
};
#define Throw_CanNotGetStream(e,a) \
throw CanNotGetStream( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NotSupported : public FerrisVFSExceptionBase
{
public:

    inline NotSupported(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NotSupported");
        }
};
#define Throw_NotSupported(e,a) \
throw NotSupported( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisNotSupportedInThisContext : public NotSupported
{
public:

    inline FerrisNotSupportedInThisContext(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        NotSupported( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisNotSupportedInThisContext");
        }
};
#define Throw_FerrisNotSupportedInThisContext(e,a) \
throw FerrisNotSupportedInThisContext( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION RootContextCreationFailed : public FerrisVFSExceptionBase
{
public:

    inline RootContextCreationFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("RootContextCreationFailed");
        }
};
#define Throw_RootContextCreationFailed(e,a) \
throw RootContextCreationFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotDelete : public FerrisVFSExceptionBase
{
public:

    inline CanNotDelete(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotDelete");
        }
};
#define Throw_CanNotDelete(e,a) \
throw CanNotDelete( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchObject : public FerrisVFSExceptionBase
{
public:

    inline NoSuchObject(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchObject");
        }
};
#define Throw_NoSuchObject(e,a) \
throw NoSuchObject( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchUser : public NoSuchObject
{
public:

    inline NoSuchUser(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        NoSuchObject( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchUser");
        }
};
#define Throw_NoSuchUser(e,a) \
throw NoSuchUser( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchGroup : public NoSuchObject
{
public:

    inline NoSuchGroup(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        NoSuchObject( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchGroup");
        }
};
#define Throw_NoSuchGroup(e,a) \
throw NoSuchGroup( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ObjectExists : public FerrisVFSExceptionBase
{
public:

    inline ObjectExists(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("ObjectExists");
        }
};
#define Throw_ObjectExists(e,a) \
throw ObjectExists( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ContextExists : public ObjectExists
{
public:

    inline ContextExists(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        ObjectExists( state, log, e.c_str(), a )
        {
            setExceptionName("ContextExists");
        }
};
#define Throw_ContextExists(e,a) \
throw ContextExists( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CopyFailed : public FerrisVFSExceptionBase
{
public:

    inline CopyFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CopyFailed");
        }
};
#define Throw_CopyFailed(e,a) \
throw CopyFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION RemoveFailed : public FerrisVFSExceptionBase
{
public:

    inline RemoveFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("RemoveFailed");
        }
};
#define Throw_RemoveFailed(e,a) \
throw RemoveFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION AttributeNotWritable : public FerrisVFSExceptionBase
{
public:

    inline AttributeNotWritable(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("AttributeNotWritable");
        }
};
#define Throw_AttributeNotWritable(e,a) \
throw AttributeNotWritable( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION SubContextAlreadyInUse : public FerrisVFSExceptionBase
{
public:

    inline SubContextAlreadyInUse(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("SubContextAlreadyInUse");
        }
};
#define Throw_SubContextAlreadyInUse(e,a) \
throw SubContextAlreadyInUse( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION AttributeAlreadyInUse : public FerrisVFSExceptionBase
{
public:

    inline AttributeAlreadyInUse(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("AttributeAlreadyInUse");
        }
};
#define Throw_AttributeAlreadyInUse(e,a) \
throw AttributeAlreadyInUse( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCreateSubContextFailed : public FerrisVFSExceptionBase
{
public:

    inline FerrisCreateSubContextFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCreateSubContextFailed");
        }
};
#define Throw_FerrisCreateSubContextFailed(e,a) \
throw FerrisCreateSubContextFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCreateSubContextNotSupported : public FerrisNotSupportedInThisContext
{
public:

    inline FerrisCreateSubContextNotSupported(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisNotSupportedInThisContext( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCreateSubContextNotSupported");
        }
};
#define Throw_FerrisCreateSubContextNotSupported(e,a) \
throw FerrisCreateSubContextNotSupported( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCreateAttributeFailed : public FerrisVFSExceptionBase
{
public:

    inline FerrisCreateAttributeFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCreateAttributeFailed");
        }
};
#define Throw_FerrisCreateAttributeFailed(e,a) \
throw FerrisCreateAttributeFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCreateAttributeNotSupported : public FerrisNotSupportedInThisContext
{
public:

    inline FerrisCreateAttributeNotSupported(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisNotSupportedInThisContext( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCreateAttributeNotSupported");
        }
};
#define Throw_FerrisCreateAttributeNotSupported(e,a) \
throw FerrisCreateAttributeNotSupported( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchSubContext : public FerrisVFSExceptionBase
{
public:

    inline NoSuchSubContext(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchSubContext");
        }
};
#define Throw_NoSuchSubContext(e,a) \
throw NoSuchSubContext( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchAttribute : public FerrisVFSExceptionBase
{
public:

    inline NoSuchAttribute(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchAttribute");
        }
};
#define Throw_NoSuchAttribute(e,a) \
throw NoSuchAttribute( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
 (LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION NoSuchContextClass : public FerrisVFSExceptionBase
{
public:

    inline NoSuchContextClass(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchContextClass");
        }
};
#define Throw_NoSuchContextClass(e,a) \
throw NoSuchContextClass( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotReadContext : public FerrisVFSExceptionBase
{
public:

    inline CanNotReadContext(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotReadContext");
        }
};
#define Throw_CanNotReadContext(e,a) \
throw CanNotReadContext( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotReadContextPcctsParseFailed : public CanNotReadContext
{
    stringlist_t m_syntaxErrorList;
public:

    inline CanNotReadContextPcctsParseFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a,
        stringlist_t syntaxErrorList )
        :
        CanNotReadContext( state, log, e.c_str(), a ),
        m_syntaxErrorList( syntaxErrorList )
        {
            setExceptionName("CanNotReadContextPcctsParseFailed");
        }
    virtual ~CanNotReadContextPcctsParseFailed() throw()
        {
        }
    const stringlist_t& getSyntaxErrorList()
        {
            return m_syntaxErrorList;
        }
};
#define Throw_CanNotReadContextPcctsParseFailed( e, a, strlist )   \
throw CanNotReadContextPcctsParseFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a), (strlist) )
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotDereferenceDanglingSoftLink : public FerrisVFSExceptionBase
{
public:

    inline CanNotDereferenceDanglingSoftLink(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotDereferenceDanglingSoftLink");
        }
};
#define Throw_CanNotDereferenceDanglingSoftLink(e,a) \
throw CanNotDereferenceDanglingSoftLink( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotMonitorDirWithFAM : public CanNotReadContext
{
    Fampp::FamppDirMonitorInitFailedException& ne;
    
public:

    inline CanNotMonitorDirWithFAM(
        const FerrisException_CodeState& state,
        fh_ostream log,
        Fampp::FamppDirMonitorInitFailedException& _ne,
        const std::string& e,
        Attribute* a=0)
        :
        CanNotReadContext( state, log, e.c_str(), a ),
        ne(_ne)
        {
            setExceptionName("CanNotMonitorDirWithFAM");
        }
};
#define Throw_CanNotMonitorDirWithFAM(e,a,ne) \
throw CanNotMonitorDirWithFAM( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (ne), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisNotReadableAsContext : public FerrisVFSExceptionBase
{
public:

    inline FerrisNotReadableAsContext(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisNotReadableAsContext");
        }
};
#define Throw_FerrisNotReadableAsContext(e,a) \
throw FerrisNotReadableAsContext( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisImageSaveFailed : public FerrisVFSExceptionBase
{
public:

    inline FerrisImageSaveFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisImageSaveFailed");
        }
};
#define Throw_FerrisImageSaveFailed(e,a) \
throw FerrisImageSaveFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisImageLoadFailed : public FerrisVFSExceptionBase
{
public:

    inline FerrisImageLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisImageLoadFailed");
        }
};
#define Throw_FerrisImageLoadFailed(e,a) \
throw FerrisImageLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisPNGImageLoadFailed : public FerrisImageLoadFailed
{
public:

    inline FerrisPNGImageLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisImageLoadFailed( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisPNGImageLoadFailed");
        }
};
#define Throw_FerrisPNGImageLoadFailed(e,a) \
throw FerrisPNGImageLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisJPEGImageLoadFailed : public FerrisImageLoadFailed
{
public:

    inline FerrisJPEGImageLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisImageLoadFailed( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisJPEGImageLoadFailed");
        }
};
#define Throw_FerrisJPEGImageLoadFailed(e,a) \
throw FerrisJPEGImageLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisGIMPImageLoadFailed : public FerrisImageLoadFailed
{
public:

    inline FerrisGIMPImageLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisImageLoadFailed( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisGIMPImageLoadFailed");
        }
};
#define Throw_FerrisGIMPImageLoadFailed(e,a) \
throw FerrisGIMPImageLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_EXCEPTION FerrisImlib2ImageLoadFailed : public FerrisImageLoadFailed
{
public:

    inline FerrisImlib2ImageLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisImageLoadFailed( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisImlib2ImageLoadFailed");
        }
};
#define Throw_FerrisImlib2ImageLoadFailed(e,a) \
throw FerrisImlib2ImageLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisWaitTimedOut : public FerrisVFSExceptionBase
{
public:

    inline FerrisWaitTimedOut(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisWaitTimedOut");
        }
};
#define Throw_FerrisWaitTimedOut(e,a) \
throw FerrisWaitTimedOut( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisSqlServerNameNotFound : public FerrisVFSExceptionBase
{
public:

    inline FerrisSqlServerNameNotFound(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisSqlServerNameNotFound");
        }
};
#define Throw_FerrisSqlServerNameNotFound(e,a) \
throw FerrisSqlServerNameNotFound( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))



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

class FERRISEXP_EXCEPTION GModuleOpenFailed : public FerrisVFSExceptionBase
{
public:

    inline GModuleOpenFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("GModuleOpenFailed");
        }
};
#define Throw_GModuleOpenFailed(e,a) \
throw GModuleOpenFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


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
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisStreamLoadFailed : public FerrisVFSExceptionBase
{
public:

    inline FerrisStreamLoadFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisStreamLoadFailed");
        }
};
#define Throw_FerrisStreamLoadFailed(e,a) \
throw FerrisStreamLoadFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



class FERRISEXP_EXCEPTION FerrisCanNotCreateLeafOfLeaf : public FerrisVFSExceptionBase
{
public:

    inline FerrisCanNotCreateLeafOfLeaf(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCanNotCreateLeafOfLeaf");
        }
};
#define Throw_FerrisCanNotCreateLeafOfLeaf(e,a) \
throw FerrisCanNotCreateLeafOfLeaf( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisOutOfMemory : public FerrisVFSExceptionBase
{
public:

    inline FerrisOutOfMemory(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisOutOfMemory");
        }
};
#define Throw_FerrisOutOfMemory(e,a) \
throw FerrisOutOfMemory( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CanNotAddExternalAttribute : public FerrisVFSExceptionBase
{
public:

    inline CanNotAddExternalAttribute(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotAddExternalAttribute");
        }
};
#define Throw_CanNotAddExternalAttribute(e,a) \
throw CanNotAddExternalAttribute( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CurlStaticInitFailed : public FerrisVFSExceptionBase
{
public:

    inline CurlStaticInitFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CurlStaticInitFailed");
        }
};
#define Throw_CurlStaticInitFailed(e,a) \
throw CurlStaticInitFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CurlEasyInitFailed : public FerrisVFSExceptionBase
{
public:

    inline CurlEasyInitFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CurlEasyInitFailed");
        }
};
#define Throw_CurlEasyInitFailed(e,a) \
throw CurlEasyInitFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CurlPerformFailed : public FerrisVFSExceptionBase
{
    int r;
public:

    int getResultCode()
        {
            return r;
        }
    
    inline CurlPerformFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0,
        int _r=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a ),
        r(_r)
        {
            setExceptionName("CurlPerformFailed");
        }
};
#define Throw_CurlPerformFailed(e,a,r) \
throw CurlPerformFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a), int(r))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CurlStateError : public FerrisVFSExceptionBase
{
public:

    inline CurlStateError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CurlStateError");
        }
};
#define Throw_CurlStateError(e,a) \
throw CurlStateError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCurlServerNameNotFound : public FerrisVFSExceptionBase
{
public:

    inline FerrisCurlServerNameNotFound(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCurlServerNameNotFound");
        }
};
#define Throw_FerrisCurlServerNameNotFound(e,a) \
throw FerrisCurlServerNameNotFound( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION getIOStreamCloseUpdateFailed : public FerrisVFSExceptionBase
{
public:

    inline getIOStreamCloseUpdateFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("getIOStreamCloseUpdateFailed");
        }
};
#define Throw_getIOStreamCloseUpdateFailed(e,a) \
throw getIOStreamCloseUpdateFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION getIOStreamCloseUpdatePermissionDenied : public getIOStreamCloseUpdateFailed
{
public:

    inline getIOStreamCloseUpdatePermissionDenied(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        getIOStreamCloseUpdateFailed( state, log, e.c_str(), a )
        {
            setExceptionName("getIOStreamCloseUpdatePermissionDenied");
        }
};
#define Throw_getIOStreamCloseUpdatePermissionDenied(e,a) \
throw getIOStreamCloseUpdatePermissionDenied( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION DTDCreationFailed : public FerrisVFSExceptionBase
{
public:

    inline DTDCreationFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("DTDCreationFailed");
        }
};
#define Throw_DTDCreationFailed(e,a) \
throw DTDCreationFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION UnknownConfigLocation : public FerrisVFSExceptionBase
{
public:

    inline UnknownConfigLocation(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("UnknownConfigLocation");
        }
};
#define Throw_UnknownConfigLocation(e,a) \
throw UnknownConfigLocation( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION RenameFailed : public FerrisVFSExceptionBase
{
public:

    inline RenameFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("RenameFailed");
        }
};
#define Throw_RenameFailed(e,a) \
throw RenameFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION AccessDenied : public FerrisVFSExceptionBase
{
public:

    inline AccessDenied(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("AccessDenied");
        }
};
#define Throw_AccessDenied(e,a) \
throw AccessDenied( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*
 * Use this with care, only as an action in cases that you dont expect
 * to ever see.
 */
class FERRISEXP_EXCEPTION GenericError : public FerrisVFSExceptionBase
{
public:

    inline GenericError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("GenericError");
        }
};
#define Throw_GenericError(e,a) \
throw GenericError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION BadlyFormedTime : public FerrisVFSExceptionBase
{
public:

    inline BadlyFormedTime(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("BadlyFormedTime");
        }
};
#define Throw_BadlyFormedTime(e,a) \
throw BadlyFormedTime( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION BadlyFormedTimeString : public BadlyFormedTime
{
public:

    inline BadlyFormedTimeString(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        BadlyFormedTime( state, log, e.c_str(), a )
        {
            setExceptionName("BadlyFormedTimeString");
        }
};
#define Throw_BadlyFormedTimeString(e,a) \
throw BadlyFormedTimeString( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION SocketOptionsException : public FerrisVFSExceptionBase
{
public:

    inline SocketOptionsException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("SocketOptionsException");
        }
};
#define Throw_SocketOptionsException(e,a) \
throw SocketOptionsException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION BackupException : public FerrisVFSExceptionBase
{
public:

    inline BackupException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("BackupException");
        }
};
#define Throw_BackupException(e,a) \
throw BackupException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION UnknownBackupMode : public BackupException
{
public:

    inline UnknownBackupMode(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        BackupException( state, log, e.c_str(), a )
        {
            setExceptionName("UnknownBackupMode");
        }
};
#define Throw_UnknownBackupMode(e,a) \
throw UnknownBackupMode( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION BackupNameWouldBeTooLong : public BackupException
{
public:

    inline BackupNameWouldBeTooLong(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        BackupException( state, log, e.c_str(), a )
        {
            setExceptionName("BackupNameWouldBeTooLong");
        }
};
#define Throw_BackupNameWouldBeTooLong(e,a) \
throw BackupNameWouldBeTooLong( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION DVDReadException : public FerrisVFSExceptionBase
{
public:

    inline DVDReadException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("DVDReadException");
        }
};
#define Throw_DVDReadException(e,a) \
throw DVDReadException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION StorageFull : public FerrisVFSExceptionBase
{
public:

    inline StorageFull(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("StorageFull");
        }
};
#define Throw_StorageFull(e,a) \
throw StorageFull( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION QuotaStorageFull : public StorageFull
{
public:

    inline QuotaStorageFull(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        StorageFull( state, log, e.c_str(), a )
        {
            setExceptionName("QuotaStorageFull");
        }
};
#define Throw_QuotaStorageFull(e,a) \
throw QuotaStorageFull( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FileClipboard : public FerrisVFSExceptionBase
{
public:

    inline FileClipboard(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FileClipboard");
        }
};
#define Throw_FileClipboard(e,a) \
throw FileClipboard( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CursorException : public FerrisVFSExceptionBase
{
public:

    inline CursorException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CursorException");
        }
};
#define Throw_CursorException(e,a) \
throw CursorException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ParseError : public FerrisVFSExceptionBase
{
public:

    inline ParseError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("ParseError");
        }
};
#define Throw_ParseError(e,a) \
throw ParseError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ProgramSpawn : public FerrisVFSExceptionBase
{
public:

    inline ProgramSpawn(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("ProgramSpawn");
        }
};
#define Throw_ProgramSpawn(e,a) \
throw ProgramSpawn( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION Db4Exception : public FerrisVFSExceptionBase
{
    int db_errno;
public:

    inline Db4Exception(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0,
        int db_errno = 0 )
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a ),
        db_errno( db_errno )
        {
            setExceptionName("Db4Exception");
        }

    int get_errno()
        {
            return db_errno;
        }
    
};
#define Throw_Db4Exception(e,a,eno) \
throw Db4Exception( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a),(eno))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION eetException : public FerrisVFSExceptionBase
{
public:

    inline eetException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("eetException");
        }
};
#define Throw_eetException(e,a) \
throw eetException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION GdbmException : public FerrisVFSExceptionBase
{
public:

    inline GdbmException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("GdbmException");
        }
};
#define Throw_GdbmException(e,a) \
throw GdbmException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION tdbException : public FerrisVFSExceptionBase
{
public:

    inline tdbException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("tdbException");
        }
};
#define Throw_tdbException(e,a) \
throw tdbException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION Db4KeyNotFound : public Db4Exception
{
public:

    inline Db4KeyNotFound(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        Db4Exception( state, log, e.c_str(), a )
        {
            setExceptionName("Db4KeyNotFound");
        }
};
#define Throw_Db4KeyNotFound(e,a) \
throw Db4KeyNotFound( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION eetKeyNotFound : public Db4Exception
{
public:

    inline eetKeyNotFound(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        Db4Exception( state, log, e.c_str(), a )
        {
            setExceptionName("eetKeyNotFound");
        }
};
#define Throw_eetKeyNotFound(e,a) \
throw eetKeyNotFound( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION InvalidSortSpecification : public FerrisVFSExceptionBase
{
public:

    inline InvalidSortSpecification(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("InvalidSortSpecification");
        }
};
#define Throw_InvalidSortSpecification(e,a) \
throw InvalidSortSpecification( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ContentNotModified : public FerrisVFSExceptionBase
{
public:

    inline ContentNotModified(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("ContentNotModified");
        }
};
#define Throw_ContentNotModified(e,a) \
throw ContentNotModified( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION ChildNotRunning : public FerrisVFSExceptionBase
{
public:

    inline ChildNotRunning(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("ChildNotRunning");
        }
};
#define Throw_ChildNotRunning(e,a) \
throw ChildNotRunning( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION InvalidModeString : public FerrisVFSExceptionBase
{
public:

    inline InvalidModeString(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("InvalidModeString");
        }
};
#define Throw_InvalidModeString(e,a) \
throw InvalidModeString( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION LDAPException : public FerrisVFSExceptionBase
{
public:

    inline LDAPException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("LDAPException");
        }
};
#define Throw_LDAPException(e,a) \
throw LDAPException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CompressionException : public FerrisVFSExceptionBase
{
public:

    inline CompressionException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CompressionException");
        }
};
#define Throw_CompressionException(e,a) \
throw CompressionException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION CompressionAlgoNotFoundException : public CompressionException
{
public:

    inline CompressionAlgoNotFoundException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        CompressionException( state, log, e.c_str(), a )
        {
            setExceptionName("CompressionAlgoNotFoundException");
        }
};
#define Throw_CompressionAlgoNotFoundException(e,a) \
throw CompressionAlgoNotFoundException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION IndexException : public FerrisVFSExceptionBase
{
public:

    inline IndexException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("IndexException");
        }
};
#define Throw_IndexException(e,a) \
throw IndexException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FullTextIndexException : public IndexException
{
public:

    inline FullTextIndexException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        IndexException( state, log, e.c_str(), a )
        {
            setExceptionName("FullTextIndexException");
        }
};
#define Throw_FullTextIndexException(e,a) \
throw FullTextIndexException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION EAIndexException : public IndexException
{
public:

    inline EAIndexException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        IndexException( state, log, e.c_str(), a )
        {
            setExceptionName("EAIndexException");
        }
};
#define Throw_EAIndexException(e,a) \
throw EAIndexException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Thrown on assertions for the params given to functions / methods.
 * You shouldn't get one of these if you follow the API docs.
 */
class FERRISEXP_EXCEPTION BadParam : public FerrisVFSExceptionBase
{
public:

    inline BadParam(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("BadParam");
        }
};
#define Throw_BadParam(e,a) \
throw BadParam( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Exception involving Schemas
 */
class FERRISEXP_EXCEPTION SchemaException : public FerrisVFSExceptionBase
{
public:

    inline SchemaException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("SchemaException");
        }
};
#define Throw_SchemaException(e,a) \
throw SchemaException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * When a schema is not available but is required by the method called then
 * this exception is thrown
 */
class FERRISEXP_EXCEPTION SchemaNotFoundException : public SchemaException
{
public:

    inline SchemaNotFoundException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        SchemaException( state, log, e.c_str(), a )
        {
            setExceptionName("SchemaNotFoundException");
        }
};
#define Throw_SchemaNotFoundException(e,a) \
throw SchemaNotFoundException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Exception involving medallions
 */
class FERRISEXP_EXCEPTION MedallionException : public FerrisVFSExceptionBase
{
public:

    inline MedallionException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("MedallionException");
        }
};
#define Throw_MedallionException(e,a) \
throw MedallionException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Exception involving etagere
 */
class FERRISEXP_EXCEPTION EtagereException : public FerrisVFSExceptionBase
{
public:

    inline EtagereException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("EtagereException");
        }
};
#define Throw_EtagereException(e,a) \
throw EtagereException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Exception involving emblems
 */
class FERRISEXP_EXCEPTION EmblemException : public FerrisVFSExceptionBase
{
public:

    inline EmblemException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("EmblemException");
        }
};
#define Throw_EmblemException(e,a) \
throw EmblemException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * When an emblem is not available but is required by the method called then
 * this exception is thrown
 */
class FERRISEXP_EXCEPTION EmblemNotFoundException : public EmblemException
{
public:

    inline EmblemNotFoundException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        EmblemException( state, log, e.c_str(), a )
        {
            setExceptionName("EmblemNotFoundException");
        }
};
#define Throw_EmblemNotFoundException(e,a) \
throw EmblemNotFoundException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * If the add parent/child will violate the partial order then you get one
 * of these. For example attempt to add a parent that is already a transitive parent.
 */
class FERRISEXP_EXCEPTION CanNotAddEmblemException : public EmblemException
{
public:

    inline CanNotAddEmblemException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        EmblemException( state, log, e.c_str(), a )
        {
            setExceptionName("CanNotAddEmblemException");
        }
};
#define Throw_CanNotAddEmblemException(e,a) \
throw CanNotAddEmblemException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * If a personality can not be found
 */
class FERRISEXP_EXCEPTION NoSuchPersonalityException : public FerrisVFSExceptionBase
{
public:

    inline NoSuchPersonalityException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchPersonalityException");
        }
};
#define Throw_NoSuchPersonalityException(e,a) \
throw NoSuchPersonalityException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * If a belief has not been expressed
 */
class FERRISEXP_EXCEPTION NoSuchBeliefException : public FerrisVFSExceptionBase
{
public:

    inline NoSuchBeliefException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchBeliefException");
        }
};
#define Throw_NoSuchBeliefException(e,a) \
throw NoSuchBeliefException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Top level exception for agent errors
 */
class FERRISEXP_EXCEPTION AgentException : public FerrisVFSExceptionBase
{
public:

    inline AgentException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("AgentException");
        }
};
#define Throw_AgentException(e,a) \
throw AgentException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Attempt to create an agent when there is already an
 * agent with its name.
 */
class FERRISEXP_EXCEPTION AgentAlreadyExistsException : public AgentException
{
public:

    inline AgentAlreadyExistsException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        AgentException( state, log, e.c_str(), a )
        {
            setExceptionName("AgentAlreadyExistsException");
        }
};
#define Throw_AgentAlreadyExistsException(e,a) \
throw AgentAlreadyExistsException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Attempt to find an agent that doesn't exist
 */
class FERRISEXP_EXCEPTION NoSuchAgentException : public AgentException
{
public:

    inline NoSuchAgentException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        AgentException( state, log, e.c_str(), a )
        {
            setExceptionName("NoSuchAgentException");
        }
};
#define Throw_NoSuchAgentException(e,a) \
throw NoSuchAgentException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Agent is only binary classifier but attempt was made to set
 * many emblems to classify with
 */
class FERRISEXP_EXCEPTION AgentOnlyHandlesOneEmblemException : public AgentException
{
public:

    inline AgentOnlyHandlesOneEmblemException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        AgentException( state, log, e.c_str(), a )
        {
            setExceptionName("AgentOnlyHandlesOneEmblemException");
        }
};
#define Throw_AgentOnlyHandlesOneEmblemException(e,a) \
throw AgentOnlyHandlesOneEmblemException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * XML Base
 */
class FERRISEXP_EXCEPTION XMLBase : public FerrisVFSExceptionBase
{
public:

    inline XMLBase(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("XMLBase");
        }
};
#define Throw_XMLBase(e,a) \
throw XMLBase( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * XML Parse error
 */
class FERRISEXP_EXCEPTION XMLParseError : public XMLBase
{
public:

    inline XMLParseError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        XMLBase( state, log, e.c_str(), a )
        {
            setExceptionName("XMLParseError");
        }
};
#define Throw_XMLParseError(e,a) \
throw XMLParseError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * XML Fatal
 */
class FERRISEXP_EXCEPTION XMLFatalError : public XMLBase
{
public:

    inline XMLFatalError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        XMLBase( state, log, e.c_str(), a )
        {
            setExceptionName("XMLFatalError");
        }
};
#define Throw_XMLFatalError(e,a) \
throw XMLFatalError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * dbXML problem
 */
class FERRISEXP_EXCEPTION dbXMLException : public XMLBase
{
public:

    inline dbXMLException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        XMLBase( state, log, e.c_str(), a )
        {
            setExceptionName("dbXMLException");
        }
};
#define Throw_dbXMLException(e,a) \
throw dbXMLException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Problem with branch filesystem
 */
class FERRISEXP_EXCEPTION BranchFileSystem : public FerrisVFSExceptionBase
{
public:

    inline BranchFileSystem(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("BranchFileSystem");
        }
};
#define Throw_BranchFileSystem(e,a) \
throw BranchFileSystem( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Problem with parsing a relative time string
 */
class FERRISEXP_EXCEPTION RelativeTimeParsing : public FerrisVFSExceptionBase
{
public:

    inline RelativeTimeParsing(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("RelativeTimeParsing");
        }
};
#define Throw_RelativeTimeParsing(e,a) \
throw RelativeTimeParsing( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * Crypto Base
 */
class FERRISEXP_EXCEPTION CryptoBase : public FerrisVFSExceptionBase
{
public:

    inline CryptoBase(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("CryptoBase");
        }
};
#define Throw_CryptoBase(e,a) \
throw CryptoBase( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * GPG initialization failure
 */
class FERRISEXP_EXCEPTION GPGMEInitFailed : public CryptoBase
{
public:

    inline GPGMEInitFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        CryptoBase( state, log, e.c_str(), a )
        {
            setExceptionName("GPGMEInitFailed");
        }
};
#define Throw_GPGMEInitFailed(e,a) \
throw GPGMEInitFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * bad signature
 */
class FERRISEXP_EXCEPTION BadSignature : public CryptoBase
{
public:

    inline BadSignature(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        CryptoBase( state, log, e.c_str(), a )
        {
            setExceptionName("BadSignature");
        }
};
#define Throw_BadSignature(e,a) \
throw BadSignature( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * key listing error
 */
class FERRISEXP_EXCEPTION KeylistException : public CryptoBase
{
public:

    inline KeylistException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        CryptoBase( state, log, e.c_str(), a )
        {
            setExceptionName("KeylistException");
        }
};
#define Throw_KeylistException(e,a) \
throw KeylistException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * XMP Metadata
 */
class FERRISEXP_EXCEPTION XMPBase : public FerrisVFSExceptionBase
{
public:

    inline XMPBase(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("XMPBase");
        }
};
#define Throw_XMPBase(e,a) \
throw XMPBase( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * XMP Metadata
 */
class FERRISEXP_EXCEPTION XMPPacketScanFailed : public XMPBase
{
public:

    inline XMPPacketScanFailed(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        XMPBase( state, log, e.c_str(), a )
        {
            setExceptionName("XMPPacketScanFailed");
        }
};
#define Throw_XMPPacketScanFailed(e,a) \
throw XMPPacketScanFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * GPhoto2
 */
class FERRISEXP_EXCEPTION GPhoto2 : public FerrisVFSExceptionBase
{
public:

    inline GPhoto2(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("GPhoto2");
        }
};
#define Throw_GPhoto2(e,a) \
throw GPhoto2( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * setup error in ~/.ferris
 */

class NoOpenWithContext : public FerrisVFSExceptionBase
{
public:

    inline NoOpenWithContext(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("NoOpenWithContext");
        }
};
#define Throw_NoOpenWithContext(e,a) \
throw NoOpenWithContext( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * no view/edit action defined for selection
 */

class OpenActionIsNotDefined : public FerrisVFSExceptionBase
{
public:

    inline OpenActionIsNotDefined(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("OpenActionIsNotDefined");
        }
};
#define Throw_OpenActionIsNotDefined(e,a) \
throw OpenActionIsNotDefined( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * something with DBus
 */

class DBusException : public FerrisVFSExceptionBase
{
public:

    inline DBusException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("DBusException");
        }
};
#define Throw_DBusException(e,a) \
throw DBusException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * something with DBus
 */

class DBusConnectionException : public DBusException
{
public:

    inline DBusConnectionException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        DBusException( state, log, e.c_str(), a )
        {
            setExceptionName("DBusConnectionException");
        }
};
#define Throw_DBusConnectionException(e,a) \
throw DBusConnectionException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * something with web photo server, eg flickr
 */

class WebPhotoException : public FerrisVFSExceptionBase
{
public:

    inline WebPhotoException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("WebPhotoException");
        }
};
#define Throw_WebPhotoException(e,a) \
throw WebPhotoException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * something with web photo server, eg flickr
 */

class WebServiceException : public FerrisVFSExceptionBase
{
public:

    inline WebServiceException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("WebServiceException");
        }
};
#define Throw_WebServiceException(e,a) \
throw WebServiceException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * something with sqlite3 database
 */

class SQLiteException : public FerrisVFSExceptionBase
{
public:

    inline SQLiteException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("SQLiteException");
        }
};
#define Throw_SQLiteException(e,a) \
throw SQLiteException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * web api call gone crazy
 */

class WebAPIException : public FerrisVFSExceptionBase
{
public:

    inline WebAPIException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("WebAPIException");
        }
};
#define Throw_WebAPIException(e,a) \
throw WebAPIException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))
    

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 */

class FerrisHALException : public FerrisVFSExceptionBase
{
public:

    inline FerrisHALException(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisHALException");
        }
};
#define Throw_FerrisHALException(e,a) \
throw FerrisHALException( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

    
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * invalid syntax for a string -> filesystem
 */

class SyntaxError : public FerrisVFSExceptionBase
{
public:

    inline SyntaxError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("SyntaxError");
        }
};
#define Throw_SyntaxError(e,a) \
throw SyntaxError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * invalid ffilter syntax
 */

class FFilterSyntaxError : public SyntaxError
{
    std::string m_data;
public:

    inline FFilterSyntaxError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        SyntaxError( state, log, e.c_str(), a )
        {
            setExceptionName("FFilterSyntaxError");
            m_data = e;
        }
    virtual ~FFilterSyntaxError() throw()
        {
        }
    virtual const std::string& whats() const
        {
            return m_data;
        }
};
#define Throw_FFilterSyntaxError(e,a) \
throw FFilterSyntaxError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * invalid fulltext query syntax
 */

class FulltextQuerySyntaxError : public SyntaxError
{
public:

    inline FulltextQuerySyntaxError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        SyntaxError( state, log, e.c_str(), a )
        {
            setExceptionName("FulltextQuerySyntaxError");
        }
};
#define Throw_FulltextQuerySyntaxError(e,a) \
throw FulltextQuerySyntaxError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * A URL/URI was given which was badly encoded.
 */

class URLDecodeSyntaxError : public SyntaxError
{
public:

    inline URLDecodeSyntaxError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        SyntaxError( state, log, e.c_str(), a )
        {
            setExceptionName("URLDecodeSyntaxError");
        }
};
#define Throw_URLDecodeSyntaxError(e,a) \
throw URLDecodeSyntaxError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Errors (Really bad things, mainly errors with libferris.so itself)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisInternalError : public FerrisVFSExceptionBase
{
public:

    inline FerrisInternalError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisVFSExceptionBase( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisInternalError");
        }
};
#define Throw_FerrisInternalError(e,a) \
throw FerrisInternalError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(LG_EXCEPTION_SWITCH( Timber::_SBufT::PRI_DEBUG )), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisParentNotSetError : public FerrisInternalError
{
public:

    inline FerrisParentNotSetError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisInternalError( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisParentNotSetError");
        }
};
#define Throw_FerrisParentNotSetError(e,a) \
throw FerrisParentNotSetError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1em()), (e), (a))

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisCanNotGetScriptNameError : public FerrisInternalError
{
public:

    inline FerrisCanNotGetScriptNameError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& e,
        Attribute* a=0)
        :
        FerrisInternalError( state, log, e.c_str(), a )
        {
            setExceptionName("FerrisCanNotGetScriptNameError");
        }
};
#define Throw_FerrisCanNotGetScriptNameError(e,a) \
throw FerrisCanNotGetScriptNameError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1em()), (e), (a))



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FERRISEXP_API void ThrowFromErrno( int eno, const std::string& e, Attribute* a=0);

};



#endif // ifndef _ALREADY_INCLUDED_FERRIS_EXCEPTION_H_
