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

    $Id: FerrisEvent.hh,v 1.3 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EVENT_H_
#define _ALREADY_INCLUDED_FERRIS_EVENT_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

namespace Ferris
{
    
    class FERRISEXP_API NamingEvent
    {
        fh_context source;
        sigc::trackable* uobj;
        
    protected:

        void setSource( fh_context s );
    
    public:
    
        NamingEvent( fh_context _s, sigc::trackable* _uobj );
        fh_context getSource();
    };

    class FERRISEXP_API NamingEvent_Changed : public NamingEvent
    {
    public:
        NamingEvent_Changed( fh_context _s, sigc::trackable* _uobj );
    };

    class FERRISEXP_API NamingEvent_Deleted : public NamingEvent
    {
    public:
        NamingEvent_Deleted( fh_context _s, sigc::trackable* _uobj );
    };

    class FERRISEXP_API NamingEvent_Start_Execute : public NamingEvent
    {
    public:
        NamingEvent_Start_Execute( fh_context _s, sigc::trackable* _uobj );
    };

    class FERRISEXP_API NamingEvent_Stop_Execute : public NamingEvent
    {
    public:
        NamingEvent_Stop_Execute( fh_context _s, sigc::trackable* _uobj );
    };
    
    class FERRISEXP_API NamingEvent_Created : public NamingEvent
    {
    public:
        NamingEvent_Created( fh_context _s, sigc::trackable* _uobj );
    };

    class FERRISEXP_API NamingEvent_Moved : public NamingEvent
    {
    public:
        NamingEvent_Moved( fh_context _s, sigc::trackable* _uobj );
    };
    
    class FERRISEXP_API NamingEvent_Exists : public NamingEvent
    {
    public:
        NamingEvent_Exists( fh_context _s, sigc::trackable* _uobj );
    };
    
    class FERRISEXP_API NamingEvent_Start_Reading_Context : public NamingEvent
    {
    public:
        NamingEvent_Start_Reading_Context( fh_context _s, sigc::trackable* _uobj );
    };

    class FERRISEXP_API NamingEvent_Stop_Reading_Context : public NamingEvent
    {
    public:
        NamingEvent_Stop_Reading_Context( fh_context _s, sigc::trackable* _uobj );
    };

};
#endif
