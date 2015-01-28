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

    $Id: FerrisHandle.hh,v 1.2 2010/09/24 21:30:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#ifndef INCLUDED_FERRIS_HANDLE_H
#define INCLUDED_FERRIS_HANDLE_H

#include <Ferris/HiddenSymbolSupport.hh>

#include <iostream>

#include <glib.h>

#include <sigc++/sigc++.h>
#include <SmartPtr.h>

#include <FerrisLoki/Extensions.hh>


namespace Ferris
{
    
//     /**
//      * Very much like Loki::DefaultSPStorage except that 
//      * DefaultSPStorage(const StoredType& p) : pointee_(p) {}
//      * will bump the reference count so that code like the following
//      * works
//      * ContextSubType* sc = new ContextSubType(); // 1
//      * fh_context c = sc;                         // 2
//      * the 'c' var will bump the reference count so that when it goes
//      * out of scope then the rc will return to what it was on the line (1)
//      */
//     template <class T>
//         class FerrisSmartPtrStorage
//         {
//         protected:
//             typedef T* StoredType;    // the type of the pointee_ object
//             typedef T* PointerType;   // type returned by operator->
//             typedef T& ReferenceType; // type returned by operator*
        
//         public:
//             FerrisSmartPtrStorage() : pointee_(Default()) 
//                 {}

//             // The storage policy doesn't initialize the stored pointer 
//             //     which will be initialized by the OwnershipPolicy's Clone fn
//             FerrisSmartPtrStorage(const FerrisSmartPtrStorage&)
//                 {}

//             template <class U>
//                 FerrisSmartPtrStorage(const FerrisSmartPtrStorage<U>&) 
//                 {}
        
//             FerrisSmartPtrStorage(const StoredType& p)
//                 : pointee_(p)
//                 {
//                     if( p )
//                     {
//                         p->AddRef();
//                     }
//                 }
        
//             PointerType operator->() const { return pointee_; }
        
//             ReferenceType operator*() const { return *pointee_; }
        
//             void Swap(FerrisSmartPtrStorage& rhs)
//                 { std::swap(pointee_, rhs.pointee_); }
    
//             // Accessors
//             friend inline PointerType GetImpl(const FerrisSmartPtrStorage& sp)
//                 { return sp.pointee_; }
        
//             friend inline const StoredType& GetImplRef(const FerrisSmartPtrStorage& sp)
//                 { return sp.pointee_; }

//             friend inline StoredType& GetImplRef(FerrisSmartPtrStorage& sp)
//                 { return sp.pointee_; }

//         protected:
//             // Destroys the data stored
//             // (Destruction might be taken over by the OwnershipPolicy)
//             void Destroy()
//                 { delete pointee_; }
        
//             // Default value to initialize the pointer
//             static StoredType Default()
//                 { return 0; }
    
//         private:
//             // Data
//             StoredType pointee_;
//         };

    
// /**
//  * SmartPtr<> policy class [OwnershipPolicy] for Handlable objects.
//  * This class implements a intrusive reference count like the COMRefCounted
//  * policy in the Modern C++ design book.
//  */
//     template <class P>
//     class FerrisRefCounted
//     {
//     public:
    
//         FerrisRefCounted()
//             {}

//         FerrisRefCounted( const FerrisRefCounted& r)
//             {}

//         template <class U>
//         FerrisRefCounted(const FerrisRefCounted<U>&)
//             {}

//         /**
//          * Create a new handle
//          */
//         P Clone(const P& val)
//             {
//                 if( val )
//                 {
// //	            cerr << "Adding   ref :" << (void*)val << endl;
//                     val->AddRef();
//                 }
//                 return val;
//             }

//         /**
//          * Release a reference. This may trigger a getClosureSignal() to fire due to
//          * the final reference being dropped.
//          *
//          * @param val Object that we are releasing an intrusive reference to
//          * @return true if the object should die.
//          */
//         bool Release(const P& val)
//             {
// //            cerr << "Removing ref :" << (void*)val << endl;
//                 if( !val )
//                 {
// //                    cerr << "Release for a NULL object, return 0" << endl;
//                     return false;
//                 }
            
//                 // P::ref_count_t
//                 int v = val->AddRef();
//                 if( v == 2 )
//                 {
// //                cerr << "FerrisRefCounted::Release() calling about to delete." << endl;
//                     val->AboutToBeDeleted();
//                 }

//                 v = val->Release();
//                 v = val->Release();
//                 if( !v )
//                 {
//                     /*
//                      * Time to die amigo
//                      */
//                     return true;
//                 }

//                 return false;
//             }
        
//         enum { destructiveCopy = false };
        
//         static void Swap(FerrisRefCounted&)
//             {
//             }
//     };


// /**
//  * Base class for all classes that have handles using the FerrisRefCounted<>
//  * policy.
//  */
//     class Handlable : public SigC::Object // : public HandlableBase
//     {
//         // memory management
//         friend class ContextStreamMemoryManager;
        
//     public:

//         typedef SigC::Signal1< void, Handlable* > GenericCloseSignal_t;

//     protected:

//         /**
//          * Type for reference counts. This could be 16/32/64 bits.
//          */
//         typedef gint16 ref_count_t;

//     public: // NB: testing only
//         /**
//          * shared reference count for all handles that point to this object.
//          * Don't access outside object
//          */
//         ref_count_t ref_count;

//         /*
//          * Don't access outside object
//          */
//         GenericCloseSignal_t GenericCloseSignal;

//     public:

//         Handlable();

//         virtual ref_count_t AddRef();
//         virtual ref_count_t Release();
//         ref_count_t getReferenceCount();



//         GenericCloseSignal_t& getGenericCloseSig();
//         virtual void AboutToBeDeleted();
    
    
//     };



//     ////////////////////////////////////////////////////////////////////////////////
//     // Emits a signal before last reference is dropped.
//     ////////////////////////////////////////////////////////////////////////////////
//     template <class T>
//     class PtrHandlableSigEmitter
//         :
//         public SigC::Object
//     {
//     public:

//         typedef PtrHandlableSigEmitter<T> ThisClass;
    
//         typedef T* PointerType;   // type returned by operator->
//         typedef T& ReferenceType; // type returned by operator*

//         /**
//          * A signal that is emitted when the object is about to die.
//          */
//         typedef SigC::Signal1< void, ReferenceType > CloseSignal_t;
    
//     private:
//         CloseSignal_t CloseSignal;

//     public:
    
//         CloseSignal_t& getCloseSig()
//             {
//                 return CloseSignal;
//             }
    
//         void EmitClose( Handlable* )
//             {
// //            cerr << "PtrHandlableSigEmitter::EmitClose() " << endl;
//                 getCloseSig().emit( (PointerType)this );
//             }

//         PtrHandlableSigEmitter( Handlable* han )
//             {
// //             cerr << "PtrHandlableSigEmitter::HandlableSigEmitter() " << endl;
//                 han->getGenericCloseSig().connect(slot(this, &ThisClass::EmitClose));
// //             cerr << "PtrHandlableSigEmitter::HandlableSigEmitter() " << endl;
//             }

//         virtual ~PtrHandlableSigEmitter()
//             {
//             }


//         typedef PtrHandlableSigEmitter<T> _Self;
    
//     };




//     template <class T>
//     typename T::_Self::CloseSignal_t&
//     getCloseSig( T& obj )
//     {
//         typename T::_Self * hook = &obj;
//         return hook->getCloseSig();
//     }


// //     template <class T>
// //     inline bool isBound(const T& sp)
// //     {
// // //        return ::Loki::GetImpl(sp) != 0 ;
// //         return GetImpl(sp) != 0 ;
// //     }

//     template <class T,
//               class CONVERTER,
//               template <class> class CHECKER>
//     inline bool isBound( const Loki::SmartPtr< T,
//                          FerrisRefCounted, 
//                          CONVERTER,
//                          CHECKER, 
//                          FerrisSmartPtrStorage > & sp)
//     {
//         return GetImpl(sp) != 0 ;
//     }

//     template <class T,
//               class CONVERTER,
//               template <class> class CHECKER,
//               template <class> class SPT>
//     inline bool isBound( const Loki::SmartPtr< T,
//                          Ferris::FerrisRefCounted, 
//                          CONVERTER,
//                          CHECKER, 
//                          SPT >& sp)
//     {
//         return ::Loki::GetImpl(sp) != 0 ;
//     }
    
    
//     template < class HT, class OT >
//     HT& Sink( HT& handle, const OT& obj )
//     {
//         HT th( obj );
//         handle = th;
//         return handle;
//     }


//     template <class To, class From>
//     inline To Upcast( To& dst, From& src )
//     {
//         dst = GetImpl( src );
//     // Solved by using a new storage policy
// //        dst->AddRef();
//         return dst;
//     }

//     template <class To, class From>
//     inline To Upcast( To& dst, From* src )
//     {
//         dst = src;
//     // Solved by using a new storage policy
// //      dst->AddRef();
//         return dst;
//     }
    
//     template <class T>
//     void* toVoid( T smptr )
//     {
//         return (void*)(GetImpl(smptr));
//     }

//     template <class T>
//     void* toVoid( T* obj )
//     {
//         return (void*)(obj);
//     }
    
};




#endif
