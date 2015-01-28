/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris cp
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

    $Id: FerrisCopyUI.hh,v 1.4 2011/06/18 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_COPY_UI_H_
#define _ALREADY_INCLUDED_FERRIS_COPY_UI_H_

#include <gmodule.h>

#include <Ferris/HiddenSymbolSupport.hh>

#include <FerrisCopy.hh>
#include <FerrisUI/GtkFerris.hh>


namespace FerrisUI
{
    using namespace Ferris;

    class FERRISEXP_API FerrisCopy_FileCompareWindow
        :
        public FileCompareWindow
    {
        typedef FerrisCopy_FileCompareWindow  _Self;
        typedef FileCompareWindow             _Base;

        GTK_TreeWalkClient* twc;
        FerrisCopy* fc;
    
    protected:
        virtual void dialog_yes( GtkButton *button );
        virtual void dialog_auto_yes( GtkButton *button );
        virtual void dialog_auto_no( GtkButton *button );
        virtual void dialog_no( GtkButton *button );
    
    public:
        virtual void processAllPendingEvents();
        FerrisCopy_FileCompareWindow( GTK_TreeWalkClient* twc, FerrisCopy* fc );
        bool perform( fh_context src, fh_context dst );

    };


    /**
     * Attaches to a  FerrisCopy object's signals and updates a
     * TreeWalkerClient with data from those signals.
     */
    class FERRISEXP_API FerrisCopy_SignalHandler
        :
        public Handlable
    {
        typedef FerrisCopy_SignalHandler _Self;

        int alarmsPerSecond;
        bool alwaysPreserveExisting;

        FerrisCopy_FileCompareWindow  fcwin;
        friend class FerrisMove_GTK;
        
        std::streamsize ocopysz;
        struct timeval ocopytv;
        int copyAlarmCount;
        GTK_TreeWalkClient* twc;
        FerrisCopy* fc;
    
        void attach();

        void updateOverallProgress( FerrisCopy& cp );
        
    public:

        FerrisCopy_SignalHandler( GTK_TreeWalkClient* twc, FerrisCopy* fc );

        void restartTimer();

        void OnCopyStart( FerrisCopy& thisobj, std::streamsize CurrentPosition,
                          std::streamsize BlockSize, std::streamsize FinalSize );
        void OnCopyProgress( FerrisCopy& thisobj, std::streamsize CurrentPosition,
                             std::streamsize BlockSize, std::streamsize FinalSize );
        void OnCopyEnd( FerrisCopy& thisobj, std::streamsize CurrentPosition,
                        std::streamsize BlockSize, std::streamsize FinalSize );

        void OnCopyVerbose( FerrisCopy& thisobj, fh_context src, fh_context dst,
                            std::string srcDescription, std::string dstDescription );
        void OnSkippingContext( FerrisCopy& thisobj, std::string srcDescription,
                                std::string reason );
        bool OnAskReplaceContext( FerrisCopy& thisobj, fh_context src, fh_context dst,
                                  std::string srcDescription, std::string dstDescription );
        bool OnAskReplaceAttribute( FerrisCopy& thisobj, fh_context src, fh_context dst,
                                    std::string srcDescription, std::string dstDescription,
                                    fh_attribute dstattr );
    };

    
};
#endif
