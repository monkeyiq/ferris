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

    $Id: FerrisCopyUI.cpp,v 1.5 2010/09/24 21:31:04 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisCopyUI.hh>
#include <General.hh>
#include <FilteredContext.hh>

#include <sys/time.h>
#include <signal.h>

using namespace std;
using namespace Ferris;

namespace FerrisUI
{
    static int alarmed = 0;

    static void sig_alarm( int signo )
    {
        alarmed = 1;
        return;
    }

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    FerrisCopy_FileCompareWindow::FerrisCopy_FileCompareWindow(
        GTK_TreeWalkClient* twc,
        FerrisCopy* fc )
        :
        twc( twc ), fc( fc )
    {
    }

    bool
    FerrisCopy_FileCompareWindow::perform( fh_context src, fh_context dst )
    {
        if( isContextInList( AlwaysPermitMatchers, dst ) )
        {
            return true;
        }

        if( isContextInList( AlwaysDenyMatchers, dst ) )
        {
            return false;
        }

        twc->showMainWindow( true );
        createMainWindow( "Replace?" );
        populateList( src, dst );
        return processMainWindow();
    }



    void
    FerrisCopy_FileCompareWindow::dialog_yes( GtkButton *button )
    {
        m_result  = true;
        m_looping = false;
    }

    void
    FerrisCopy_FileCompareWindow::dialog_auto_yes( GtkButton *button )
    {
        const MatchData_t& endlist = getMatchingDests();
        if( endlist.empty() )
        {
            fh_stringstream ss;
            ss << "By not selecting a predicate you are wishing to copy all files"
               << nl << " Automatically copy all?";
            if( !RunQuestionDialog( tostr(ss), GTK_WIDGET(m_win) ) )
            {
                return;
            }
            fc->setInteractive( false );
            fc->setForceOverWrite( true );
        }
        else
        {
            fh_matcher m = Ferris::Factory::ComposeEqualsMatcher( endlist );
            AlwaysPermitMatchers.push_back( m );
        }
        m_result  = true;
        m_looping = false;
    }


    void
    FerrisCopy_FileCompareWindow::dialog_auto_no( GtkButton *button )
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
    FerrisCopy_FileCompareWindow::dialog_no( GtkButton *button )
    {
        m_result = false;
        m_looping = false;
    }

    
    void
    FerrisCopy_FileCompareWindow::processAllPendingEvents()
    {
        twc->processAllPendingEvents();
    }

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    FerrisCopy_SignalHandler::FerrisCopy_SignalHandler(
        GTK_TreeWalkClient* twc,
        FerrisCopy* fc )
        :
        twc( twc ), fc( fc ),
        fcwin( twc, fc ),
        alwaysPreserveExisting( false ),
        alarmsPerSecond(4)
    {
        attach();
    }

    void
    FerrisCopy_SignalHandler::attach()
    {
        fc->getCopyStartSignal().connect( sigc::mem_fun( *this,    &_Self::OnCopyStart ));
        fc->getCopyPorgressSignal().connect( sigc::mem_fun( *this, &_Self::OnCopyProgress ));
        fc->getCopyEndSignal().connect( sigc::mem_fun( *this,      &_Self::OnCopyEnd ));
        fc->getCopyVerboseSignal().connect( sigc::mem_fun( *this,  &_Self::OnCopyVerbose ));
        fc->getSkippingContextSignal().connect( sigc::mem_fun( *this,  &_Self::OnSkippingContext ));
        fc->getAskReplaceContextSignal().connect( sigc::mem_fun( *this,&_Self::OnAskReplaceContext));
        fc->getAskReplaceAttributeSignal().connect( sigc::mem_fun( *this,&_Self::OnAskReplaceAttribute));
    }


    void
    FerrisCopy_SignalHandler::OnCopyStart( FerrisCopy& thisobj,
                                           std::streamsize CurrentPosition,
                                           std::streamsize BlockSize,
                                           std::streamsize FinalSize )
    {
        gtk_progress_bar_set_fraction( twc->m_progress, 0.0 );
        gtk_progress_bar_set_text( twc->m_progress, "0" );
        twc->updateStartTime();
        twc->processAllPendingEvents();
        gettimeofday( &ocopytv, 0 );
        ocopysz = 0;
        copyAlarmCount = 0;
    }

    void
    FerrisCopy_SignalHandler::OnCopyProgress( FerrisCopy& cp,
                                              std::streamsize CurrentPosition,
                                              std::streamsize BlockSize,
                                              std::streamsize FinalSize )
    {
        gdouble perc = CurrentPosition;
        perc /= FinalSize;
            
        if( alarmed )
        {
            if( GtkProgressBar* p = twc->m_progress )
            {
                gtk_progress_bar_set_fraction( p, perc );
                fh_stringstream ss;
                ss << Util::convertByteString(CurrentPosition)
                   << " / "
                   << Util::convertByteString(FinalSize)
                   << flush;
                gtk_progress_bar_set_text( p, tostr(ss).c_str() );
            }
            updateOverallProgress( cp );
            

            ++copyAlarmCount;

            if( !( copyAlarmCount % (alarmsPerSecond*3) ))
            {
                std::streamsize sizedelta = (CurrentPosition - ocopysz);
                struct timeval ncopytv;
                gettimeofday( &ncopytv, 0 );
                double secdelta = ncopytv.tv_sec - ocopytv.tv_sec;
                double ntimed = ncopytv.tv_usec / 1000000.0 + secdelta;
                double otimed = ocopytv.tv_usec / 1000000.0;
                double timedelta = ntimed - otimed;
                double CPS = sizedelta / timedelta;
                        
//                 cerr << " sizedelta:" << sizedelta
//                      << " ns:" << ncopytv.tv_sec << " nus:" <<  ncopytv.tv_usec
//                      << " os:" << ocopytv.tv_sec << " ous:" <<  ocopytv.tv_usec
//                      << " ntimed:" << ntimed
//                      << " otimed:" << otimed
//                      << " timedelta:" << timedelta
//                      << " CPS:" << CPS
//                      << endl;
                        
                gtk_label_set_text( GTK_LABEL(twc->m_speedlab),
                                    Util::convertByteString( (guint64)CPS ).c_str() );

                ocopysz = CurrentPosition;
                ocopytv = ncopytv;
            }
                    
                    
            if( !( copyAlarmCount % (alarmsPerSecond) ))
            {
                twc->updateElapsedTime();
            }
                    
            twc->processAllPendingEvents();
            restartTimer();
        }
    }

    void
    FerrisCopy_SignalHandler::OnCopyEnd( FerrisCopy& cp,
                                         std::streamsize CurrentPosition,
                                         std::streamsize BlockSize,
                                         std::streamsize FinalSize )
    {
        gtk_label_set_text( GTK_LABEL(twc->m_srclab), "" );
        gtk_label_set_text( GTK_LABEL(twc->m_dstlab), "" );
        gtk_label_set_text( GTK_LABEL(twc->m_speedlab), "" );
        gtk_progress_bar_set_fraction( twc->m_progress, 1.0 );
        gtk_progress_bar_set_text( twc->m_progress, "100%" );
        updateOverallProgress( cp );
    }

    void
    FerrisCopy_SignalHandler::updateOverallProgress( FerrisCopy& cp )
    {
        if( GtkProgressBar* p = twc->m_overall_progress )
        {
            gtk_progress_bar_set_fraction( p, cp.getTotalPercentageOfBytesCopied() );
            fh_stringstream ss;
            ss << Util::convertByteString(cp.getTotalBytesCopied())
               << " / "
               << Util::convertByteString(cp.getTotalBytesToCopy())
               << flush;
//             cerr << "updateOverallProgress() total:" << cp.getTotalBytesToCopy()
//                  << " done:" << cp.getTotalBytesCopied()
//                  << " remaining:" << (cp.getTotalBytesToCopy()-cp.getTotalBytesCopied())
//                  << endl;
            gtk_progress_bar_set_text( p, tostr(ss).c_str() );
        }
    }
    
    void
    FerrisCopy_SignalHandler::restartTimer()
    {
        Util::SingleShot virgin;

        if( virgin() )
        {
            if( signal( SIGALRM, sig_alarm ) == SIG_ERR )
            {
                cerr << "Can not attach sigalarm" << endl;
            }
        }
        
        alarmed = 0;
        struct itimerval t;
        t.it_interval.tv_sec = 0;
        t.it_interval.tv_usec = 0;
        t.it_value.tv_sec = 0;
        t.it_value.tv_usec = 1000000/alarmsPerSecond;
        setitimer( ITIMER_REAL, &t, 0 );
    }
    
    void
    FerrisCopy_SignalHandler::OnCopyVerbose( FerrisCopy& thisobj,
                                             fh_context src,
                                             fh_context dst,
                                             string srcDescription,
                                             string dstDescription )
    {
        gtk_label_set_text( GTK_LABEL(twc->m_srclab), srcDescription.c_str() );
        gtk_label_set_text( GTK_LABEL(twc->m_dstlab), dstDescription.c_str() );
        twc->processAllPendingEvents();
    }

    void
    FerrisCopy_SignalHandler::OnSkippingContext( FerrisCopy& thisobj,
                                                 string srcDescription,
                                                 string reason )
    {
        GtkTreeIter iter;
        gtk_tree_store_append( twc->m_skipmodel, &iter, 0 );
        gtk_tree_store_set( twc->m_skipmodel, &iter,
                            GTK_TreeWalkClient::SKIP_REASON, reason.c_str(),
                            GTK_TreeWalkClient::SKIP_DESC, srcDescription.c_str(),
                            -1 );
    }

    bool
    FerrisCopy_SignalHandler::OnAskReplaceContext( FerrisCopy& thisobj,
                                                   fh_context src,
                                                   fh_context dst,
                                                   string srcDescription,
                                                   string dstDescription )
    {
        if( alwaysPreserveExisting )
        {
            return false;
        }

        fc->hadUserInteraction = true;
        gtk_label_set_text( GTK_LABEL(twc->m_srclab), srcDescription.c_str() );
        gtk_label_set_text( GTK_LABEL(twc->m_dstlab), dstDescription.c_str() );
        return fcwin.perform( src, dst );
    }
    
    bool
    FerrisCopy_SignalHandler::OnAskReplaceAttribute( FerrisCopy& thisobj,
                                                     fh_context src,
                                                     fh_context dst,
                                                     string srcDescription,
                                                     string dstDescription,
                                                     fh_attribute dstattr )
    {
        if( alwaysPreserveExisting )
        {
            return false;
        }

        fc->hadUserInteraction = true;
        fh_stringstream msgss;
        msgss << "Replace " << dstDescription;
        GtkWidget* d = gtk_message_dialog_new
            ( 0,
              GTK_DIALOG_MODAL,
              GTK_MESSAGE_QUESTION,
              GTK_BUTTONS_NONE,
              tostr(msgss).c_str(),
              0 );

        enum 
            {
                RESP_SMALLEST = 1,
                RESP_ALWAYS = 1,
                RESP_NO,
                RESP_PRESERVE,
                RESP_STOP,
                RESP_YES,
                RESP_SIZE
            };
        gtk_dialog_add_buttons( GTK_DIALOG(d),
                                "_always",                 RESP_ALWAYS,
                                "_no",                     RESP_NO,
                                "_preserve all existing",  RESP_PRESERVE,
                                "_stop everything",        RESP_STOP,
                                "_yes",                    RESP_YES,
                                0 );
        gtk_widget_show_all( d );
        gint rc = -1;
        while( rc < RESP_SMALLEST || rc > RESP_SIZE )
        {
            rc = gtk_dialog_run (GTK_DIALOG (d));
        }
        gtk_widget_destroy( d );

        switch( rc )
        {
        case RESP_ALWAYS:
            fc->setInteractive( false );
            return true;
        case RESP_NO:
            return false;
                
        case RESP_PRESERVE:
            alwaysPreserveExisting = true;
            return false;
                
        case RESP_STOP:
            cout << "User requested to stop" << endl;
            exit(0);
                
        case RESP_YES:
            return true;
        }
            
        return false;
    }


};
