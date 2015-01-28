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

    $Id: FerrisRemoveUI.cpp,v 1.5 2011/06/18 21:30:22 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <gmodule.h>
#include <FerrisRemoveUI.hh>
#include <FilteredContext.hh>

using namespace std;

namespace FerrisUI
{

    FerrisRm_FileCompareWindow::FerrisRm_FileCompareWindow(
        GTK_TreeWalkClient* twc,
        FerrisRm* frm )
        :
        twc( twc ), frm( frm )
    {
    }

    bool
    FerrisRm_FileCompareWindow::perform( fh_context target )
    {
        if( isContextInList( AlwaysPermitMatchers, target ) )
        {
            return true;
        }

        if( isContextInList( AlwaysDenyMatchers, target ) )
        {
            return false;
        }

        twc->showMainWindow( frm->getSloth(), true );
        createMainWindow( "Remove?" );
        populateList( target );
        return processMainWindow();
    }



    void
    FerrisRm_FileCompareWindow::dialog_yes( GtkButton *button )
    {
        m_result  = true;
        m_looping = false;
    }

    void
    FerrisRm_FileCompareWindow::dialog_auto_yes( GtkButton *button )
    {
        const MatchData_t& endlist = getMatchingDests();
        if( endlist.empty() )
        {
            fh_stringstream ss;
            ss << "By not selecting a predicate you are wishing to remove all files"
               << nl << " Automatically remove all?";
            if( !RunQuestionDialog( tostr(ss), GTK_WIDGET(m_win) ) )
            {
                return;
            }
            frm->setInteractive( false );
            frm->setForce( true );
        }
        else
        {
            fh_matcher m = Ferris::Factory::ComposeEqualsMatcher( endlist );
//             cerr << "AlwaysPermitMatchers original size:" << AlwaysPermitMatchers.size() << endl;
            AlwaysPermitMatchers.push_back( m );
//             cerr << "AlwaysPermitMatchers new      size:" << AlwaysPermitMatchers.size() << endl;
        }
        m_result  = true;
        m_looping = false;
    }


    void
    FerrisRm_FileCompareWindow::dialog_auto_no( GtkButton *button )
    {
        const MatchData_t& endlist = getMatchingDests();
        if( endlist.empty() )
        {
            fh_stringstream ss;
            ss << "You must highlight atleast one row of the auto column";
            RunInfoDialog( tostr(ss), GTK_WIDGET(m_win) );
            return;
        }
        else
        {
            fh_matcher m = Ferris::Factory::ComposeEqualsMatcher( endlist );
            AlwaysDenyMatchers.push_back( m );
        }
    
        m_result = false;
        m_looping = false;
    }

    void
    FerrisRm_FileCompareWindow::dialog_no( GtkButton *button )
    {
        m_result = false;
        m_looping = false;
    }

    
    void
    FerrisRm_FileCompareWindow::processAllPendingEvents()
    {
        twc->processAllPendingEvents();
//     while (gtk_events_pending ())
//         gtk_main_iteration ();
    
//     if( gtk_window_is_closed )
//     {
//         throw Quit_Requested();
//     }
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    void
    FerrisRm_SignalHandler::attach()
    {
        frm->getRemoveVerboseSignal().connect( sigc::mem_fun( *this, &_Self::OnRemoveVerbose ));
        frm->getSkippingSignal().connect( sigc::mem_fun( *this, &_Self::OnSkipping ));
        frm->getAskRemoveSignal().connect( sigc::mem_fun( *this, &_Self::OnAskRemove ));
    }

    FerrisRm_SignalHandler::FerrisRm_SignalHandler( GTK_TreeWalkClient* twc, FerrisRm* frm )
        :
        twc( twc ), frm( frm ),
        fcwin( twc, frm )
    {
        attach();
    }

    void
    FerrisRm_SignalHandler::OnRemoveVerbose( FerrisRm& thisref,
                                             fh_context target,
                                             std::string desc )
    {
//        cerr << "OnRemoveVerbose() desc:" << desc << endl;
        gtk_label_set_text( GTK_LABEL(twc->m_currentObjectLab), desc.c_str() );
        twc->processAllPendingEvents();
    }

    void
    FerrisRm_SignalHandler::OnSkipping( FerrisRm& thisref,
                                        std::string desc,
                                        std::string reason )
    {
        twc->addToSkipped( desc, reason );
    }

    bool
    FerrisRm_SignalHandler::OnAskRemove( FerrisRm& thisref,
                                         fh_context target,
                                         std::string desc )
    {
        gtk_label_set_text( GTK_LABEL(twc->m_currentObjectLab), desc.c_str() );
        frm->hadUserInteraction = true;
        return fcwin.perform( target );
    }

    

};

