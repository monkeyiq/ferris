/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris mv
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

    $Id: FerrisMove.cpp,v 1.8 2010/09/24 21:30:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisMove.hh>
#include <FerrisBackup.hh>
#include <Trimming.hh>
#include <FerrisCopy.hh>
#include <FerrisCopy_private.hh>
#include <config.h> // version info
#include <FerrisRemove_private.hh>
#include <FerrisSemantic.hh> // myrdf smushing

using namespace std;

// #undef LG_MOVE_D
// #define LG_MOVE_D cerr

namespace Ferris
{

    FerrisMv::FerrisMv()
        :
        AutoClose( false ),
        hadUserInteraction( false ),
        m_AttributeUpdaterInUse( false ),
        DstIsDirectory( false ),
        SpecialCaseDstIsDirectory( false )
    {
        backupMaker.setMode( "none" );
        
        getSkippingContextSignal().connect( sigc::mem_fun( *this,  &_Self::OnSkippingContext ));
    }

    FerrisMv::SkippingContextSignal_t&
    FerrisMv::getSkippingContextSignal()
    {
        return SkippingContextSignal;
    }


    void
    FerrisMv::OnSkippingContext( FerrisMv& thisobj,
                                 string srcDescription,
                                 string dstDescription,
                                 string reason )
    {
        cout << "skipping " << srcDescription
             << " to " << dstDescription;
        if( reason.length() )
        {
            cout << " reason:" << reason;
        }
        cout << endl;
    }
    
    bool
    FerrisMv::SameFileSystem( const std::string& sname, const std::string& dname  )
    {
        fh_context src = Resolve( sname );
        
        /* Check if destination is a directory */
        fh_context dst;
        bool UseParentForDst = false;
        try
        {
            dst = Resolve( dname );
//            if( !toint( getStrAttr( dst, "is-dir", "0" )))
            if( !toint( getStrAttr( dst, "is-dir-try-automounting", "0" )))
                UseParentForDst = true;
        }
        catch( exception& e )
        {
            UseParentForDst = true;
        }

        if( UseParentForDst )
            dst = Resolve( dname, RESOLVE_PARENT );

        
//        bool ret = getStrAttr( src, "fs-id", "0") == getStrAttr( dst, "fs-id", "-1");
        string srcID = getStrAttr( src, "fs-id", "0");
        string dstID = getStrAttr( dst, "fs-id", "0");
        bool ret = (srcID == dstID);

        //
        // Some filesystems like tmpfs return 0. If we are moving from a tmpfs
        // into an XML file then both will return 0 but clearly this is not able
        // to be preformed within the filesystem itself.
        //
        if( ret && srcID == "0" )
        {
            bool sn = src->getIsNativeContext();
            bool dn = dst->getIsNativeContext();
            bool som = src->getOverMountContext() != dynamic_cast<Context*>(GetImpl(src));
            bool dom = dst->getOverMountContext() != dynamic_cast<Context*>(GetImpl(dst));

            LG_MOVE_D << "FerrisMv::SameFileSystem() sn:" << sn << " dn:" << dn
                      << " som:" << som << " dom:" << dom
                      << endl;

            if( som || dom )
                ret = false;
//             if( sn && !dn || dn && !sn )
//                 ret = false;
        }
        
        LG_MOVE_D << "FerrisMv::SameFileSystem() src:" << src->getURL()
                  << " src-id:" << getStrAttr( src, "fs-id", "0")
                  << " dst:" << dst->getURL()
                  << " dst-id:" << getStrAttr( dst, "fs-id", "0")
                  << " UseParentForDst:" << UseParentForDst
                  << " ret:" << ret
                  << endl;
        
        return ret;
    }

    void
    FerrisMv::handleSingleFileBackup( const std::string& p )
    {
        if( !backupMaker.impotent() )
        {
            fh_context c = Resolve( p );
            backupMaker( c );
        }
    }


    void
    FerrisMv::setMovingToDir( bool v )
    {
        MovingToDir = v;
    }

    void
    FerrisMv::setBackupSuffix( const std::string& s )
    {
        backupMaker.setMode( Util::BackupMaker::MODE_SIMPLE );
        backupMaker.setSuffix( s );
    }

    void
    FerrisMv::setExplicitSELinuxContext( const std::string& s )
    {
        m_ExplicitSELinuxContext = s;
    }

    void
    FerrisMv::setExplicitSELinuxType( const std::string& s )
        {
            m_ExplicitSELinuxType = s;
        }
    
    void
    FerrisMv::setCloneSELinuxContext( bool v )
    {
        m_CloneSELinuxContext = v;
    }
    
    void
    FerrisMv::setPerformBackups( const std::string& s )
    {
        backupMaker.setMode( s );
    }

    void
    FerrisMv::setForce( bool v )
    {
        Force = v;
    }

    void
    FerrisMv::setInteractive( bool v )
    {
        Interactive = v;
    }

    void
    FerrisMv::setVerbose( bool v )
    {
        Verbose = v;
    }

    void
    FerrisMv::setUpdateMode( bool v )
    {
        UpdateMode = v;
    }


    void
    FerrisMv::maybeStripTrailingSlashes()
    {
        if( StripTrailingSlashes )
        {
            PostfixTrimmer trimmer;
            trimmer.push_back( "/" );
            srcURL = trimmer( srcURL );
            dstURL = trimmer( dstURL );
        }
    }

    void
    FerrisMv::setStripTrailingSlashes( bool v )
    {
        StripTrailingSlashes = v;
        maybeStripTrailingSlashes();
    }


    void
    FerrisMv::setShowMeter( bool v )
    {
        ShowMeter = v;
    }

    void
    FerrisMv::setSloth( bool v )
    {
        Sloth = v;
    }

    void
    FerrisMv::setAutoClose( bool v )
    {
        AutoClose = v;
    }

    void
    FerrisMv::setDstIsDirectory( bool v )
    {
        DstIsDirectory = v;
    }
    
    
    
    std::string
    FerrisMv::getSrcURL()
    {
        return srcURL;
    }
    
    void
    FerrisMv::setSrcURL( const string& s )
    {
        srcURL = s;
        maybeStripTrailingSlashes();
    }

    void
    FerrisMv::setDstURL( const string& s )
    {
        dstURL = attemptToAbsoluteURL(s);
        maybeStripTrailingSlashes();
    }

    string
    FerrisMv::getSrcName()
    {
        fh_context c = Resolve( srcURL );
        LG_MOVE_D << "FerrisMv::getSrcName() srcURL:" << srcURL
                  << " dirname:" << c->getDirName()
                  << endl;
        return c->getDirName();
    }

    bool
    FerrisMv::handleUpdateMode( const std::string& oldrdn, const std::string& newrdn )
    {
        if( UpdateMode )
        {
            try
            {
                fh_context sc = Resolve( srcURL );
                fh_context dc = Resolve( dstURL );
                time_t srctt = toType<time_t>(getStrAttr( sc, "mtime", "-1" ));
                time_t dsttt = toType<time_t>(getStrAttr( dc, "mtime", "-1" ));
                    
                if( srctt == -1 || dsttt == -1 )
                {
                    getSkippingContextSignal().emit( *this, oldrdn, newrdn, "" );
                    return true;
                }
                if( srctt <= dsttt )
                {
                    getSkippingContextSignal().emit( *this, oldrdn, newrdn, "src not newer" );
                    return true;
                }
            }
            catch( NoSuchSubContext& e )
            {
            }
            catch( exception& e )
            {
                getSkippingContextSignal().emit( *this, oldrdn, newrdn,
                                                 "problem resolving destination" );
                return true;
            }
        }
        return false;
    }

    bool
    FerrisMv::handleInteractiveMode( const std::string& oldrdn, const std::string& newrdn )
    {
        if( !Interactive )
            return true;

        if( Interactive )
        {
            cout << "move " << oldrdn << " to " << newrdn << " ? " << flush;
            char c;
            cin >> c;
            if( c == 'y' || c == 'Y' )
                return true;
        }
        return false;
    }

    bool
    FerrisMv::handleVerboseMode( const std::string& oldrdn, const std::string& newrdn )
    {
        if( Verbose )
        {
            cout << "moving " << oldrdn << " to " << newrdn << endl;
        }
        return false;
    }

    fh_cp FerrisMv::getCopyObject()
    {
        fh_cp_tty fcp = new FerrisCopy_TTY();
        fcp->setShowMeter( ShowMeter );
        fcp->setDstIsDirectory( DstIsDirectory );
        return fcp;
    }
    
    fh_rm FerrisMv::getRemoveObject()
    {
        fh_rm frm = FerrisRm::CreateObject();
        return frm;
    }
    
    void
    FerrisMv::crossVolumeMove()
    {
        try
        {
//             cerr << "its dir cp/rm code! srcURL:" << srcURL << " dstURL:" << dstURL << endl;

            fh_cp fcp = getCopyObject();
            fh_rm frm = getRemoveObject();

            {
                /* copy it and delete src */

                fcp->setSrcURL( srcURL );
                fcp->setDstURL( dstURL );
                fcp->setForceOverWrite( Force );
                fcp->setInteractive( Interactive );
                fcp->setUpdateMode( UpdateMode );
                fcp->setVerbose( Verbose );
                fcp->setRecurse( true );
                fcp->setFollowOnlyForSrcUrl( 0 );
                fcp->setDontFollowLinks( 1 );
                fcp->setPreserveMTime( true );
                fcp->setPreserveATime( true );
                fcp->setPreserveOwner( true );
                fcp->setPreserveGroup( true );
                fcp->setPreserveObjectMode( true );
                fcp->setStripTrailingSlashes( StripTrailingSlashes );
                fcp->setBackupMaker( backupMaker );
                fcp->setExplicitSELinuxContext( m_ExplicitSELinuxContext );
                fcp->setExplicitSELinuxType( m_ExplicitSELinuxType );
                fcp->setCloneSELinuxContext( m_CloneSELinuxContext );
                fcp->setDstIsDirectory( SpecialCaseDstIsDirectory || DstIsDirectory );
                fcp->copy();
                fcp->DetachAllSignals();
            }
                    

//             cerr << " -*- copy done. -*- " << endl;
//             cerr << " srcURL:" << srcURL << endl;

            frm->setInteractive( Interactive );
            frm->setVerbose( Verbose );
            frm->setTarget( srcURL );
            frm->setRecurse( true );
            frm->remove();

        }
        catch( exception& e )
        {
            cerr << "problem moving files, exiting. e:" << e.what() << endl;
        }
        LG_MOVE_D << "recursive copy complete." << endl;
    }

    void
    FerrisMv::move()
    {
        LG_MOVE_D << "FerrisMv::move() srcURL:" << srcURL << " dstURL:" << dstURL << endl;

        SpecialCaseDstIsDirectory = false;
        
        fh_context c  = Resolve( srcURL, RESOLVE_PARENT );
        string oldrdn = getSrcName();

        string newrdn = dstURL;

        //
        // if moving inside bookmarks:// and other places
        //
        try
        {
            fh_context bm = Resolve("bookmarks://");

            if( starts_with( c->getURL(), bm->getURL() ) )
            {
                fh_context dst = Resolve( dstURL );
                string durl = dst->getURL();
                bool forceDstToDir = false;

                LG_MOVE_D << "Checking if dst is a bookmarks path... dst:" << durl << endl
                          << " bm:" << bm->getURL()
                          << endl;
                forceDstToDir |= starts_with( durl, "bookmarks://" );
                forceDstToDir |= starts_with( durl, bm->getURL() );
                LG_MOVE_D << "forceDstToDir:" << forceDstToDir << endl;

                if( forceDstToDir )
                {
                    MovingToDir = true;
                    SpecialCaseDstIsDirectory = true;
                }
            }
        }
        catch( exception& e )
        {
        }
        
        
        if( MovingToDir )
        {
            fh_stringstream ss;
            ss << dstURL << "/" << oldrdn;
            newrdn = tostr(ss);
        }

        LG_MOVE_D << "oldrdn:" << oldrdn << " newrdn:" << newrdn << endl;

        if( handleUpdateMode( oldrdn, newrdn ) )      return;
        LG_MOVE_D << "move(2) oldrdn:" << oldrdn << " newrdn:" << newrdn << endl;
        if( !handleInteractiveMode( oldrdn, newrdn ) ) return;
        LG_MOVE_D << "move(3) oldrdn:" << oldrdn << " newrdn:" << newrdn << endl;
        if( handleVerboseMode( oldrdn, newrdn ) )     return;
        LG_MOVE_D << "move(4) oldrdn:" << oldrdn << " newrdn:" << newrdn << endl;
    
        if( !SameFileSystem( srcURL, dstURL ) )
        {
            LG_MOVE_D << "cross volume move() srcURL:" << srcURL
                      << " dstURL:" << dstURL << endl;
            crossVolumeMove();
        }
        else
        {
            LG_MOVE_D << "single volume move() srcURL:" << srcURL
                      << " dstURL:" << dstURL << endl;
            handleSingleFileBackup( dstURL );
            LG_MOVE_D << "move() src parent:" << c->getURL()
                      << " oldrdn:" << oldrdn
                      << " newrdn:" << newrdn
                      << endl;
            fh_context oldchild = c->getSubContext( oldrdn );
            string oldearl = oldchild->getURL();
            
//             stringlist_t rdf_ea_sl = Util::parseCommaSeperatedList(
//                 getStrAttr( oldchild, "rdf-ea-names", "" ) );
//             stringmap_t rdf_ea_map;
//             if( !rdf_ea_sl.empty() )
//             {
//                 stringlist_t::const_iterator end = rdf_ea_sl.end();
//                 for( stringlist_t::const_iterator si = rdf_ea_sl.begin(); si!=end; ++si )
//                 {
//                     rdf_ea_map[ *si ] = getStrAttr( oldchild, *si, "", true, true );
//                 }
//             }
            
            
            fh_context child = c->rename( oldrdn, newrdn, true, true ); //Force );

            if( !m_ExplicitSELinuxContext.empty() )
            {
                setStrAttr( child,
                            "dontfollow-selinux-context",
                            m_ExplicitSELinuxContext,
                            true, true );
            }
            if( !m_ExplicitSELinuxType.empty() )
            {
                setStrAttr( child,
                            "dontfollow-selinux-type",
                            m_ExplicitSELinuxType,
                            true, true );
            }

            Semantic::myrdfSmush( child, oldearl );
            
//             if( !rdf_ea_map.empty() )
//             {
//                 stringmap_t::const_iterator end = rdf_ea_map.end();
//                 for( stringmap_t::const_iterator mi = rdf_ea_map.begin(); mi!=end; ++mi )
//                 {
//                     setStrAttr( child, mi->first, mi->second, true, true );
//                 }
//             }
            
            
            if( m_AttributeUpdaterInUse )
            {
                fh_cp fcp = getCopyObject();
                fcp->getAttributeUpdaterSignal().emit(
                    GetImpl(fcp), oldchild, child,
                    oldchild->getURL(), child->getURL() );
            }
        }
    }


    fh_mv_collector
    FerrisMv::getPoptCollector()
    {
        if( !isBound( Collector ) )
        {
            Collector = new MovePopTableCollector();
        }
        return Collector;
    }

    FerrisMv::m_AttributeUpdater_t&
    FerrisMv::getAttributeUpdaterSignal()
    {
        m_AttributeUpdaterInUse = true;
        fh_cp fcp = getCopyObject();
        return fcp->getAttributeUpdaterSignal();
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    MovePopTableCollector::MovePopTableCollector()
    {
        reset();
    }
    

    void MovePopTableCollector::poptCallback(poptContext con,
                                               enum poptCallbackReason reason,
                                               const struct poptOption * opt,
                                               const char * arg,
                                               const void * data)
    {
    }

    void MovePopTableCollector::reset()
    {
        Force                = 0;
        Interactive          = 0;
        Verbose              = 0;
        ShowVersion          = 0;
        Sloth                = 0;
        AutoClose            = 0;
        ShowMeter            = 0;
        UpdateMode           = 0;
        StripTrailingSlashes = 0;
        PerformBackups       = 0;
        ArchiveMode          = 0;
        BackupSuffix         = 0;
        ExplicitSELinuxContext = 0;
        ExplicitSELinuxType    = 0;
        CloneSELinuxContext    = 0;
        PerformBackupsWithMode = 0;
        DstIsDirectory         = 0;
        
    }
    
    void
    MovePopTableCollector::ArgProcessingDone( poptContext optCon )
    {
        if( ShowVersion )
        {
            cout << "ferrismv version: $Id: FerrisMove.cpp,v 1.8 2010/09/24 21:30:39 ben Exp $\n"
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

        mv->setForce( Force );
        mv->setInteractive( Interactive );
        mv->setVerbose( Verbose );
//        mv->setRecurse( Recurse );
        mv->setSloth( Sloth );
        mv->setAutoClose( AutoClose );
        mv->setStripTrailingSlashes( StripTrailingSlashes );
        mv->setShowMeter( ShowMeter );
        mv->setUpdateMode( UpdateMode );
        mv->setDstIsDirectory( DstIsDirectory );

        mv->setPerformBackups( "none" );
        if( PerformBackups )          mv->setPerformBackups( "simple" );
        if( PerformBackupsWithMode )  mv->setPerformBackups( PerformBackupsWithMode );
        if( BackupSuffix )            mv->setBackupSuffix( BackupSuffix );
        if( ExplicitSELinuxContext )
            mv->setExplicitSELinuxContext( ExplicitSELinuxContext );
        if( ExplicitSELinuxType )
            mv->setExplicitSELinuxType( ExplicitSELinuxType );
        if( CloneSELinuxContext )
            mv->setCloneSELinuxContext( CloneSELinuxContext );
    }
            
    struct ::poptOption*
    MovePopTableCollector::getTable( fh_mv _mv )
    {
        mv = _mv;
        int extraTableLines = 5;
        int tablesize = 22;

        tablesize = tablesize + extraTableLines;
        allocTable( tablesize );
                    
        int i=0;
        setToCallbackEntry( &table[i] );
        ++i;

        setEntry(
            &table[i++], "", 'a', POPT_ARG_NONE, &ArchiveMode, 
            "ignored (should become like cp -a", "" );
        setEntry(
            &table[i++], "", 'b', POPT_ARG_NONE, &PerformBackups, 
            "like --backup but does not accept an argument", "" );
        setEntry(
            &table[i++], "backup", 0, POPT_ARG_STRING, &PerformBackupsWithMode, 
            "make a backup of each existing destination file", "CONTROL" );
        setEntry(
            &table[i++], "force", 'f', POPT_ARG_NONE, &Force,
            "ignore non existent files, never prompt", "" );
        setEntry(
            &table[i++], "interactive", 'i', POPT_ARG_NONE, &Interactive, 
            "prompt before any removal", "");
        setEntry(
            &table[i++], "show-progress-meter", 0, POPT_ARG_NONE, &ShowMeter, 
            "Show a one line progress meter", "" );
        setEntry(
            &table[i++], "sloth", 0, POPT_ARG_NONE, &Sloth, 
            "keep the main window closed until it is needed", "" );
        setEntry(
            &table[i++], "auto-close", '0', POPT_ARG_NONE, &AutoClose, 
            "If there is no user interaction or objects skipped then close client automatically", "" );
        setEntry(
            &table[i++], "strip-trailing-slashes", '/', POPT_ARG_NONE, &StripTrailingSlashes,
            "remove any trailing slashes from each SOURCE argument", "" );
        setEntry(
            &table[i++], "suffix", 'S', POPT_ARG_STRING, &BackupSuffix, 
            "override the usual backup suffix", "SUFFIX" );
        setEntry(
            &table[i++], "update", 'u', POPT_ARG_NONE, &UpdateMode, 
            "copy only when the SOURCE file is newer than the destination"
            " file or when the destination file is missing", "" );
        setEntry(
            &table[i++], "verbose", 'v', POPT_ARG_NONE, &Verbose, 
            "explain what is being done", "" );
        setEntry(
            &table[i++], "version", 0, POPT_ARG_NONE, &ShowVersion, 
            "show version information and quit", 0 );
        setEntry(
            &table[i++], "dst-is-dir", 0, POPT_ARG_NONE, &DstIsDirectory,
            "force treating destination is a directory (copy into db4/xml file)", "" );

        setEntry(
            &table[i++], "set", 0, POPT_ARG_STRING, &ExplicitSELinuxType, 
            "set SELinux type of copy to TYPE", 0 );
        setEntry(
            &table[i++], "selinux-type", 0, POPT_ARG_STRING, &ExplicitSELinuxType, 
            "set SELinux type of copy to TYPE", 0 );
        setEntry(
            &table[i++], "context", 'Z', POPT_ARG_STRING, &ExplicitSELinuxContext, 
            "set security context of copy to CONTEXT", 0 );
        setEntry(
            &table[i++], "clone-context", 0, POPT_ARG_NONE, &CloneSELinuxContext, 
            "clone input file security context to output file", 0 );
        
        clearEntry( &table[i] );
        return table;
    }
        

    
    namespace Priv
    {
        struct ::poptOption* getMovePopTableCollector( fh_mv mv )
        {
            return mv->getPoptCollector()->getTable( mv );
        }
    };
    
};
