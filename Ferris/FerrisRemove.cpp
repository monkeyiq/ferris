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

    $Id: FerrisRemove.cpp,v 1.4 2010/09/24 21:30:41 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <boost/regex.hpp>
#include <FerrisRemove_private.hh>
#include <config.h>

using namespace std;

namespace Ferris
{

    FerrisRm::RemoveVerboseSignal_t&
    FerrisRm::getRemoveVerboseSignal()
    {
        return RemoveVerboseSignal;
    }

    FerrisRm::SkippingSignal_t&
    FerrisRm::getSkippingSignal()
    {
        return SkippingSignal;
    }
    
    FerrisRm::AskRemoveSignal_t&
    FerrisRm::getAskRemoveSignal()
    {
        return AskRemoveSignal;
    }

    void
    FerrisRm::OnRemoveVerbose( FerrisRm& thisref, fh_context target, std::string desc )
    {
        cout << "removing " << desc << endl;
    }
    
    void
    FerrisRm::OnSkipping( FerrisRm& thisref, std::string desc,  std::string reason )
    {
        if( Verbose )
        {
            cerr << "skipping " << desc << " reason:" << reason << endl;
        }
    }
    
    bool
    FerrisRm::OnAskRemove( FerrisRm& thisref, fh_context target, std::string desc )
    {
        cout << "delete " << desc << " ? " << flush;
        char c;
        cin >> c;
        return ( c == 'y' || c == 'Y' );
    }
    
    fh_context
    FerrisRm::getTargetContext( const std::string& p )
    {
        return Resolve( p );
    }
    
    std::string
    FerrisRm::getTargetDescription( const std::string& p )
    {
        return p;
    }
    
    /**
     * Remove all the files under p
     */
    void
    FerrisRm::removeTree( const std::string& p )
    {
//         Ferrisls ls;
//         fh_lsdisplay_rm ld = new Ferrisls_rm_display();
//         ld->setInteractive( Interactive );
//         ld->setVerbose( Verbose );
//         ls.setDisplay( ld );
//         ls.setURL( p );
//         ls.setRecursiveList( true );
//         ls.usePreorderTraversal( true );
//         ls.setDontFollowLinks( false );
//         ls();

        ls.setDisplay( this );
        ls.setURL( p );
        ls.setRecursiveList( true );
        ls.usePreorderTraversal( true );
        ls.setDontFollowLinks( true ); //false );
        ls();
    }

    /******************************************************************************/
    /*** Tree methods *************************************************************/
    /******************************************************************************/

    void
    FerrisRm::PrintEA( int i,  const std::string& attr, const std::string& EA )
    {
    }

    void
    FerrisRm::ShowAttributes( fh_context ctx )
    {
        if( isBound(LastRecursiveContext) )
            removeObject( LastRecursiveContext );
        
        LastRecursiveContext = ctx;
    }

    void
    FerrisRm::workStarting()
    {
        LastRecursiveContext = 0;
    }

    void
    FerrisRm::workComplete()
    {
        if( isBound(LastRecursiveContext) )
        {
            removeObject( LastRecursiveContext );
            LastRecursiveContext = 0;
        }
    }

    void
    FerrisRm::EnteringContext(fh_context ctx)
    {
    }

    void
    FerrisRm::LeavingContext(fh_context ctx)
    {
    }

    void
    FerrisRm::removeObject(fh_context ctx)
    {
        if( ! ctx->isParentBound() )
        {
            string msg = "attempt to remove root entry denied!";
            cerr << msg << endl;
            getSkippingSignal().emit( *this, getTargetDescription(ctx->getURL()), msg );
            return;
        }
        removeObject( ctx->getParent(), ctx->getDirName(), ctx->getURL() );
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    void
    FerrisRm::removeObject(fh_context parent, const std::string& rdn, const std::string& p )
    {
        if( Force )
        {
            if( !parent->isSubContextBound( rdn ) )
            {
                getSkippingSignal().emit( *this,
                                          getTargetDescription(p),
                                          "Not existant object" );
                return;
            }
        }

        if( Interactive )
        {
            if( ! getAskRemoveSignal().emit( *this,
                                             getTargetContext(p),
                                             getTargetDescription(p) ))
            {
                return;
            }
        }
            
        if( Verbose )
        {
            getRemoveVerboseSignal().emit( *this,
                                           getTargetContext(p),
                                           getTargetDescription(p) );
        }
        parent->remove( rdn );
    }
    

    /**
     * Remove the object at p
     */
    void
    FerrisRm::removeObject( const std::string& p )
    {
        fh_context c;
            
        try
        {
            c = Resolve( p, RESOLVE_PARENT );
            c->read();
        }
        catch( exception& e )
        {
            if( Force )
            {
                getSkippingSignal().emit( *this,
                                          getTargetDescription(p),
                                          "Not existant object" );
                return;
            }
        }

        string rdn = p;
        if( string::npos != p.rfind( "/" ) )
        {
            rdn = p.substr( p.rfind( "/" )+1 );
        }

        removeObject( c, rdn, p );
            
//         if( Force )
//         {
//             if( !c->isSubContextBound( rdn ) )
//             {
//                 getSkippingSignal().emit( *this,
//                                           getTargetDescription(p),
//                                           "Not existant object" );
//                 return;
//             }
//         }

//         if( Interactive )
//         {
// //             cout << "delete " << p << " ? " << flush;
// //             char c;
// //             cin >> c;
// //             if(!( c == 'y' || c == 'Y' ))
// //                 return;

//             if( ! getAskRemoveSignal().emit( *this,
//                                              getTargetContext(p),
//                                              getTargetDescription(p) ))
//             {
//                 return;
//             }
//         }
            
//         if( Verbose )
//         {
// //            cout << "removing " << p << endl;
//             getRemoveVerboseSignal().emit( *this,
//                                            getTargetContext(p),
//                                            getTargetDescription(p) );
//         }
//         c->remove( rdn );
    }
    
    FerrisRm::FerrisRm()
        :
        Force( true ),
        Interactive( true ),
        Verbose( true ),
        Recurse( false ),
        Sloth( false ),
        AutoClose( false ),
        TargetURL(""),
        LastRecursiveContext(0),
        hadUserInteraction( false )
    {
        getRemoveVerboseSignal().connect( sigc::mem_fun( *this, &_Self::OnRemoveVerbose ));
        getSkippingSignal().connect(      sigc::mem_fun( *this, &_Self::OnSkipping ));
        getAskRemoveSignal().connect(     sigc::mem_fun( *this, &_Self::OnAskRemove ));
    }

    FerrisRm::~FerrisRm()
    {
    }

    fh_rm
    FerrisRm::CreateObject()
    {
        return new FerrisRm();
    }
    
    void
    FerrisRm::setForce( bool v )
    {
        Force = v;
    }
    
    void
    FerrisRm::setVerbose( bool v )
    {
        Verbose = v;
    }
    
    void
    FerrisRm::setInteractive( bool v )
    {
        Interactive = v;
    }
    
    void
    FerrisRm::setRecurse( bool v )
    {
        Recurse = v;
    }

    void
    FerrisRm::setSloth( bool v )
    {
        Sloth = v;
    }

    void
    FerrisRm::setAutoClose( bool v )
    {
        AutoClose = v;
    }
    
    

    bool
    FerrisRm::getSloth()
    {
        return Sloth;
    }
    
    void
    FerrisRm::setTarget( const std::string& removeTargetURL )
    {
        TargetURL = removeTargetURL;
    }
    
    void
    FerrisRm::remove()
    {
//        cerr << "FerrisRm::remove() target:" << TargetURL << endl;
        
        if( !TargetURL.length() )
        {
            fh_stringstream ss;
            ss << "No target URL given for removal." << endl;
            Throw_RemoveFailed( tostr(ss), 0 );
        }

        fh_context c;

        if( Recurse )
        {
            removeTree( TargetURL );
            /* Recursive falls through to remove the dir node itself. */
        }
            
        removeObject( TargetURL );
    }

    void
    FerrisRm::operator()()
    {
        remove();
    }

    fh_rm_collector
    FerrisRm::getPoptCollector()
    {
        if( !isBound( Collector ) )
        {
            Collector = new RemovePopTableCollector();
        }
        return Collector;
    }


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    
    RemovePopTableCollector::RemovePopTableCollector()
    {
        reset();
    }
    

    void RemovePopTableCollector::poptCallback(poptContext con,
                                               enum poptCallbackReason reason,
                                               const struct poptOption * opt,
                                               const char * arg,
                                               const void * data)
    {
        const string key = opt->longName;
//        LG_COPY_D << "poptCallback() key:" << key << " -a" << ArchiveMode << endl;
    }

    void RemovePopTableCollector::reset()
    {
        Force                  = 0;
        Interactive            = 0;
        Verbose                = 0;
        ShowVersion            = 0;
        Recurse                = 0;
        Sloth                  = 0;
        AutoClose              = 0;
    }
    
    void
    RemovePopTableCollector::ArgProcessingDone( poptContext optCon )
    {
        if( ShowVersion )
        {
            cout << "ferrisrm version: $Id: FerrisRemove.cpp,v 1.4 2010/09/24 21:30:41 ben Exp $\n"
                 << "ferris   version: " << VERSION << nl
                 << "Written by Ben Martin, aka monkeyiq" << nl
                 << nl
                 << "Copyright (C) 2001 Ben Martin" << nl
                 << "This is free software; see the source for copying conditions.  There is NO\n"
                 << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                 << endl;
            exit(0);
        }

        if( Force )
        {
            Interactive = 0;
        }
        
        rm->setForce( Force );
        rm->setInteractive( Interactive );
        rm->setVerbose( Verbose );
        rm->setRecurse( Recurse );
        rm->setSloth( Sloth );
        rm->setAutoClose( AutoClose );
    }
            
    struct ::poptOption*
    RemovePopTableCollector::getTable( fh_rm _rm )
    {
        rm = _rm;
        int extraTableLines = 5;
        int tablesize = 20;

        tablesize = tablesize + extraTableLines;
        allocTable( tablesize );
                    
        int i=0;
        setToCallbackEntry( &table[i] );
        ++i;

        setEntry(
            &table[i++], "force", 'f', POPT_ARG_NONE, &Force,
            "ignore non existent files, never prompt", "" );
        setEntry(
            &table[i++], "interactive", 'i', POPT_ARG_NONE, &Interactive, 
            "prompt before any removal", "");
        setEntry(
            &table[i++], "recursive", 'R', POPT_ARG_NONE, &Recurse, 
            "remove the contents of directories recursively", "" );
        setEntry(
            &table[i++], "recursive", 'r', POPT_ARG_NONE, &Recurse, 
            "remove the contents of directories recursively", "" );
        setEntry(
            &table[i++], "sloth", 0, POPT_ARG_NONE, &Sloth, 
            "keep the main window closed until it is needed", "" );
        setEntry(
            &table[i++], "auto-close", '0', POPT_ARG_NONE, &AutoClose, 
            "If there is no user interaction or objects skipped then close client automatically", "" );
        setEntry(
            &table[i++], "verbose", 'v', POPT_ARG_NONE, &Verbose, 
            "explain what is being done", "" );
        setEntry(
            &table[i++], "version", 0, POPT_ARG_NONE, &ShowVersion, 
            "show version information and quit", 0 );
 
        clearEntry( &table[i] );
        return table;
    }
        

    
    namespace Priv
    {
        struct ::poptOption* getRemovePopTableCollector( fh_rm rm )
        {
            return rm->getPoptCollector()->getTable( rm );
        }
    };
};
