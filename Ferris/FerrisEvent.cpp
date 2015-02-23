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

    $Id: FerrisEvent.cpp,v 1.3 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisEvent.hh>
#include <Ferris.hh>

namespace Ferris
{
    

    void NamingEvent::setSource( fh_context s ) 
    {
        source = s;
    }
    
    NamingEvent::NamingEvent( fh_context _s, sigc::trackable* _uobj )
        :
        source(_s),
        uobj(_uobj)
    {
    }
    
    fh_context
    NamingEvent::getSource() 
    {
        return source;
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    NamingEvent_Changed::NamingEvent_Changed( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }
    

    NamingEvent_Deleted::NamingEvent_Deleted( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }
    

    NamingEvent_Start_Execute::NamingEvent_Start_Execute( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    NamingEvent_Stop_Execute::NamingEvent_Stop_Execute( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    NamingEvent_Created::NamingEvent_Created( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    NamingEvent_Moved::NamingEvent_Moved( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }


    NamingEvent_Exists::NamingEvent_Exists( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    NamingEvent_Start_Reading_Context::NamingEvent_Start_Reading_Context( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    NamingEvent_Stop_Reading_Context::NamingEvent_Stop_Reading_Context( fh_context _s, sigc::trackable* _uobj )
        :
        NamingEvent( _s, _uobj )
    {
    }

    
};
