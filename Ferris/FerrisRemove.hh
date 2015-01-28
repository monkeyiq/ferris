/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris
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

    $Id: FerrisRemove.hh,v 1.3 2010/09/24 21:30:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_REMOVE_H_
#define _ALREADY_INCLUDED_FERRIS_REMOVE_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferrisls.hh>
#include <Ferris/FerrisPopt.hh>
#include <Ferris/FerrisBackup.hh>

#include <string>

namespace FerrisUI
{
    class FerrisRm_SignalHandler;
};

namespace Ferris
{
    class RemovePopTableCollector;
    FERRIS_SMARTPTR( RemovePopTableCollector, fh_rm_collector );
    
    class FerrisRm;
    FERRIS_SMARTPTR( FerrisRm, fh_rm );

    
    class FERRISEXP_API RemovePopTableCollector
        :
        public basic_PopTableCollector,
        public Handlable
    {
        fh_rm rm;
        
        unsigned long Force;
        unsigned long Interactive;
        unsigned long Verbose;
        unsigned long ShowVersion;
        unsigned long Recurse;
        unsigned long Sloth;
        unsigned long AutoClose;

    public:
        RemovePopTableCollector();
        
        virtual void poptCallback(poptContext con,
                                  enum poptCallbackReason reason,
                                  const struct poptOption * opt,
                                  const char * arg,
                                  const void * data);
        
        void reset();
        void ArgProcessingDone( poptContext optCon );
        struct ::poptOption* getTable( fh_rm _rm );
    };

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_API FerrisRm
        :
        public Ferrisls_display
    {
        typedef FerrisRm         _Self;
        typedef Ferrisls_display _Base;

        friend class FerrisUI::FerrisRm_SignalHandler;
        
        /**
         * It makes things very difficult if we are removing a context when we just discover
         * its existance, its much easier if we cache the context and remove it when we
         * discover another context, we need to clear this at the start and remove it at
         * then end, but that is a small price to pay.
         */
        fh_context LastRecursiveContext;
        Ferrisls ls;

        virtual void workStarting();
        virtual void workComplete();
        virtual void EnteringContext(fh_context ctx);
        virtual void LeavingContext(fh_context ctx);

    protected:

        virtual void ShowAttributes( fh_context ctx );
        virtual void PrintEA( int i,  const std::string& attr, const std::string& EA );

        
    public:

        typedef sigc::signal3< void,
                               FerrisRm&,       // thisobj,
                               fh_context,      // target,
                               std::string      // target Description,
                               > RemoveVerboseSignal_t;
        RemoveVerboseSignal_t& getRemoveVerboseSignal();

        typedef sigc::signal3< void,
                               FerrisRm&,        // thisobj,
                               std::string,      // target Description,
                               std::string       // reason
                               > SkippingSignal_t;
        SkippingSignal_t& getSkippingSignal();

        typedef sigc::signal3< bool,
                               FerrisRm&,       // thisobj,
                               fh_context,      // target,
                               std::string      // target Description,
                               > AskRemoveSignal_t;
        AskRemoveSignal_t& getAskRemoveSignal();

    protected:

        virtual void OnRemoveVerbose( FerrisRm& thisref, fh_context target, std::string desc );
        virtual void OnSkipping(      FerrisRm& thisref, std::string desc,  std::string reason );
        virtual bool OnAskRemove(     FerrisRm& thisref, fh_context target, std::string desc );


        bool Force;
        bool Interactive;
        bool Verbose;
        bool Recurse;
        bool Sloth;
        bool AutoClose;
        /**
         * If we are 'AutoClose' we dont auto close if we have
         * interacted with the user
         */
        bool hadUserInteraction;
        

    private:

        RemoveVerboseSignal_t  RemoveVerboseSignal;
        SkippingSignal_t       SkippingSignal;
        AskRemoveSignal_t      AskRemoveSignal;
        
        std::string TargetURL;
        fh_rm_collector Collector;

        void removeTree( const std::string& p );
        void removeObject( const std::string& p );
        void removeObject(fh_context ctx);
        void removeObject(fh_context parent, const std::string& rdn, const std::string& p );

        fh_context  getTargetContext( const std::string& p );
        std::string getTargetDescription( const std::string& p );
        
    protected:

        FerrisRm();

    public:        
        
        virtual ~FerrisRm();
        static fh_rm CreateObject();
        
        void setForce( bool v );
        void setVerbose( bool v );
        void setInteractive( bool v );
        void setRecurse( bool v );
        void setSloth( bool v );
        void setAutoClose( bool v );
        bool getSloth();
        void setTarget( const std::string& removeTargetURL );

        void remove();
        void operator()();
        
        fh_rm_collector getPoptCollector();
    };
    
    
    namespace Priv
    {
        FERRISEXP_API struct ::poptOption* getRemovePopTableCollector( fh_rm rm );
    };
    
#define FERRIS_REMOVE_OPTIONS(rm) { 0, 0, POPT_ARG_INCLUDE_TABLE, \
/**/  ::Ferris::Priv::getRemovePopTableCollector(rm),     \
/**/  0, "common remove options:", 0 },
};
#endif
