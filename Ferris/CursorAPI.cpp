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

    $Id: CursorAPI.cpp,v 1.4 2010/09/24 21:30:28 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <CursorAPI.hh>
#include <Ferris.hh>
#include <iomanip>

namespace Ferris
{
    using namespace std;

    namespace Config
    {
        const string cursor_object_always_remake = "cursor-object-always-remake";
        const string cursor_object_list_size     = "cursor-list-size";
        
        void setCursorOption( fh_context parent,
                              const std::string& k,
                              const std::string& v )
        {
            setStrAttr( parent, k, v, true, true );
        }
    };
    using namespace Config;
    
    static const string cursor_object_current_rdn       = "cursor-rdn";
    static const string cursor_object_list_size_default = "20";

    static string adjustClipRdnWithPadding( const std::string& s )
    {
        fh_stringstream ss;
        ss << setfill('0') << setw(5) << s;
        return tostr(ss);
    }
    

    static fh_context adjustCursor( fh_context parent, int offset )
    {
        string cliprdn  = getStrAttr( parent, cursor_object_current_rdn, "-1"   );
        string clipsize = getStrAttr( parent,
                                      cursor_object_list_size,
                                      cursor_object_list_size_default );
        bool alwaysRemake = isTrue( getStrAttr( parent, cursor_object_always_remake, "0" ));
        int  modulas      = ( toint( clipsize ) + 1 );
        
        cliprdn = tostr( toint(cliprdn) + offset );
        while( toint( cliprdn ) > toint( clipsize ) )
        {
//             cerr << "+++ OLD rdn:" << cliprdn << endl;
            cliprdn = tostr( toint( cliprdn ) - modulas );
//             cerr << "+++ NEW rdn:" << cliprdn << endl;
        }
        while( toint( cliprdn ) < 0 )
        {
            cliprdn = tostr( toint( cliprdn ) + modulas );
        }
        
        cliprdn = adjustClipRdnWithPadding( cliprdn );
        setStrAttr( parent, cursor_object_current_rdn, cliprdn, true );
            
        if( parent->isSubContextBound( cliprdn ) )
        {
            if( alwaysRemake && offset != 0 )
            {
                cerr << "parent->remove() cliprdn:" << cliprdn << endl;
                parent->remove( cliprdn );
                // fall through to the code that handles non existant cursor file
            }
            else
            {
                fh_context ret = parent->getSubContext( cliprdn );
//             cerr << "adjustCursor() parent:" << parent->getURL()
//                  << " ret:" << ret->getURL()
//                  << endl;
                return ret;
            }
        }

        
//         cerr << "--- Creating file:" << cliprdn
//              << " for parent:" << parent->getURL()
//              << endl;
        fh_context ret = Shell::CreateFile( parent, cliprdn );
//         cerr << "adjustCursor() parent:" << parent->getURL()
//              << " ret:" << ret->getURL()
//              << endl;
        return ret;
    }
    
    namespace Factory
    {
        fh_context getCursor( fh_context parent )
        {
            return adjustCursor( parent, 0 );
        }
    };
    namespace Cursor
    {
        fh_context cursorNext( fh_context cursor )
        {
//             cerr << "cursorNext(enter) cursor:" << cursor->getURL()
//                  << " cursor->parent:" << cursor->getParent()->getURL()
//                  << endl;
            fh_context ret = cursorOffset( cursor, 1 );
//             cerr << "cursorNext(exit) cursor:" << cursor->getURL()
//                  << " ret:" << ret->getURL()
//                  << endl;
            return ret;
        }
        
        fh_context cursorPrev( fh_context cursor )
        {
            return cursorOffset( cursor, -1 );
        }
        
        fh_context cursorOffset( fh_context cursor, int offset )
        {
            return adjustCursor( cursor->getParent(), offset );
        }

        fh_context cursorSet( fh_context cursor, fh_context newc )
        {
            fh_context parent = cursor->getParent();
            string rdn        = newc->getDirName();

            if( parent->getSubContext( rdn ) )
            {
                setStrAttr( parent, cursor_object_current_rdn, rdn, true );
            }
            else
            {
                fh_stringstream ss;
                ss << "Attempt to explicitly set current object for"
                   << " cursored context at:" << parent->getURL()
                   << " to object:" << newc->getURL()
                   << " when object is not an active member of of the collection";
                Throw_CursorException( tostr(ss), GetImpl(newc) );
            }
        }
    };
};


