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

    $Id: FerrisHandle.cpp,v 1.2 2010/09/24 21:30:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisHandle.hh>

namespace Ferris
{
    

// /**
//  * We start with a single reference already to prevent boundary race
//  * conditions.
//  */
// Handlable::Handlable()
//     :
//     ref_count(0)
// {
// //    cerr << "Handlable::Handlable() " << endl;
// }

// /**
//  * Add another reference
//  */
// Handlable::ref_count_t
// Handlable::AddRef()
// {
//     return ++ref_count;
// }

// /**
//  * Release a reference
//  */
// Handlable::ref_count_t
// Handlable::Release()
// {
//     g_return_val_if_fail( ref_count >= 1 , 1 );
//     return --ref_count;
// }

// Handlable::ref_count_t
// Handlable::getReferenceCount()
// {
//     return ref_count;
// }



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// /**
//  * Called by FerrisRefCounted<> when this object is about
//  * to be deleted. This abstracts the AddRef() and Release()
//  * from knowing when the policy decides to delete objects.
//  */
// void
// Handlable::AboutToBeDeleted()
// {
// //    cerr << "HandlableBase::AboutToBeDeleted()" << endl;
//     getGenericCloseSig().emit( this );
// }

// Handlable::GenericCloseSignal_t&
// Handlable::getGenericCloseSig()
// {
//     return GenericCloseSignal;
// }





 
};
