/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris remove
    Copyright (C) 2002 Ben Martin

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

    $Id: FerrisRemoveUI.hh,v 1.2 2010/09/24 21:31:05 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_REMOVE_UI_H_
#define _ALREADY_INCLUDED_FERRIS_REMOVE_UI_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <FerrisRemove.hh>
#include <FerrisUI/GtkFerris.hh>

namespace FerrisUI
{
    using namespace Ferris;

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    class FERRISEXP_API FerrisRm_FileCompareWindow
        :
        public FileCompareWindow
    {
        typedef FerrisRm_FileCompareWindow  _Self;
        typedef FileCompareWindow           _Base;

        FerrisRm* frm;
        GTK_TreeWalkClient* twc;
    
    protected:
        virtual void dialog_yes( GtkButton *button );
        virtual void dialog_auto_yes( GtkButton *button );
        virtual void dialog_auto_no( GtkButton *button );
        virtual void dialog_no( GtkButton *button );
    
    public:
        virtual void processAllPendingEvents();
        FerrisRm_FileCompareWindow( GTK_TreeWalkClient* twc, FerrisRm* frm );
        bool perform( fh_context target );

    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API FerrisRm_SignalHandler
        :
        public Handlable
    {
        typedef FerrisRm_SignalHandler _Self;

        FerrisRm_FileCompareWindow fcwin;
        GTK_TreeWalkClient* twc;
        FerrisRm* frm;

        void attach();
    
    public:

        FerrisRm_SignalHandler( GTK_TreeWalkClient* twc, FerrisRm* frm );
        void OnRemoveVerbose( FerrisRm& thisref, fh_context target, std::string desc );
        void OnSkipping( FerrisRm& thisref, std::string desc,  std::string reason );
        bool OnAskRemove( FerrisRm& thisref, fh_context target, std::string desc );
    
    };
    
};
#endif
