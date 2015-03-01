/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2002 Ben Martin

    This file is part of libferris.

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

    $Id: FerrisCopy.cpp,v 1.27 2011/04/27 21:31:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#define _XOPEN_SOURCE

#include <Native.hh>
#include <FerrisCopy.hh>

#include <Trimming.hh>
#include <Ferrisls.hh>
#include <config.h> // version info
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <sigc++/object.h>
#include <sigc++/object_slot.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <linux/falloc.h>

// // sendfile(2)
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <sys/sendfile.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
#ifndef POSIX_FADV_SEQUENTIAL
#define POSIX_FADV_SEQUENTIAL 0
#endif
#ifndef POSIX_FADV_DONTNEED
#define POSIX_FADV_DONTNEED 0
#endif



using namespace std;

namespace Ferris
{

    FerrisCopy::CopyVerboseSignal_t&
    FerrisCopy::getCopyVerboseSignal()
    {
        return CopyVerboseSignal;
    }

    FerrisCopy::SkippingContextSignal_t&
    FerrisCopy::getSkippingContextSignal()
    {
        return SkippingContextSignal;
    }
    
    FerrisCopy::AskReplaceContextSignal_t&
    FerrisCopy::getAskReplaceContextSignal()
    {
        return AskReplaceContextSignal;
    }
    
    FerrisCopy::AskReplaceAttributeSignal_t&
    FerrisCopy::getAskReplaceAttributeSignal()
    {
        return AskReplaceAttributeSignal;
    }
    
    fh_cp_collector
    FerrisCopy::getPoptCollector()
        {
            if( !isBound( Collector ) )
            {
                Collector = new ContextPopTableCollector();
            }

            return Collector;
        }

    static string toOctalString( int v )
    {
        stringstream ss;
        ss << oct << v;
        return ss.str();
    }
    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    

    void
    FerrisCopy::workStarting()
        {
            RecursiveIsInitial = true;

            ls.setDontFollowLinks( FollowOnlyForSrcUrl || DontFollowLinks );
            
            LG_COPY_D << "work starting!" << endl;
            LG_COPY_D << "srcURL:" << srcURL << endl;
            LG_COPY_D << "dstURL:" << dstURL << endl;
            LG_COPY_D << "About to read source context..." << endl;
        }
    
    void
    FerrisCopy::workComplete()
        {
        }

    string
    FerrisCopy::dirname( fh_context ctx, string s )
        {
            std::string n = ctx->getLastPartOfName( s );
            return s.substr( 0, s.length() - n.length() - 1 );
        }

    bool
    FerrisCopy::ShouldEnterContext(fh_context ctx)
    {
        string earl = dstURL;
        if( !RecursiveIsInitial )
        {
            earl = ctx->appendToPath( earl, ctx->getDirName() );
        }

        LG_COPY_D << "ctx:" << ctx->getURL() << endl;
        LG_COPY_D << "dst parent earl:" << earl << endl;
        
        fh_context tsrc = ctx;
        fh_context tdstparent = Resolve( earl, RESOLVE_PARENT );
        
        LG_COPY_D << "+++ should enter ctx:" << ctx->getDirPath()
                  << " src:" << tsrc->getDirPath()
                  << " dstURL:" << earl
                  << " dstparent:" << tdstparent->getDirPath()
                  << endl;

        if( AttemptToCopyIntoSelf( tsrc, tdstparent ) )
        {
            fh_stringstream ss;
            ss << "Attempt to copy into self";
            getSkippingContextSignal().emit(
                *this,
                getSourceDescription(),
                tostr(ss) );
            LG_COPY_D << "ShouldEnterContext() failing..." << endl;
            cerr << "ShouldEnterContext() failing..." << endl;
            return false;
        }
        
        LG_COPY_D << "ShouldEnterContext() is true, src:" << tsrc->getDirPath() << endl;
        return true;
    }
    
    void
    FerrisCopy::EnteringContext(fh_context ctx)
        {
            bool wasInitialCall = RecursiveIsInitial;

            if( RecursiveIsInitial )
            {
                RecursiveIsInitial = false;
            }
            else
            {
                dstURL = ctx->appendToPath( dstURL, ctx->getDirName() );
            }

            src = ctx;
            priv_setSrcContext( src, wasInitialCall );
            priv_setDstContext( dstURL, wasInitialCall );
            LG_COPY_D << "EnteringContext() ctx:" << ctx->getDirPath()
                      << " src:" << src->getDirPath()
                      << " dstURL:" << dstURL
                      << " dstparent:" << dstparent->getDirPath()
                      << endl;
            perform_copy();
        }
    
    void
    FerrisCopy::LeavingContext(fh_context ctx)
        {
//            dstURL = ctx->appendToPath( dstURL, ctx->getDirName() );
            LG_COPY_D << "START exit ctx:" << ctx->getDirPath() << " dstURL:" << dstURL << endl;

            bool isDir = toint( getStrAttr( ctx, "is-dir", "0" ) );

            if( isDir )
            {
                src = ctx;
                dst = Resolve( dstURL );
                PreserveEA( "mode" );
            }
            if( isDir && DstAttr.empty() )
            {
                src = ctx;
                dst = Resolve( dstURL );
                LG_COPY_D << "Leaving dir... " << endl
                          << "   src:" << src->getURL() << endl
                          << "   dstp:" << dstparent->getURL() << endl
                          << "   dst:" << dst->getURL() << endl
                          << "   getDstName():" << getDstName()
                          << " PreserveMTime:" << PreserveMTime
                          << " PreserveATime:" << PreserveATime
                          << endl;
                if( PreserveMTime )      PreserveEA( "mtime" );
                if( PreserveATime )      PreserveEA( "atime" );
            }

            dstURL = dirname( ctx, dstURL );
            LG_COPY_D << "DONE exit ctx:" << ctx->getDirPath() << " dstURL:" << dstURL << endl;
        }
    
    void
    FerrisCopy::ShowAttributes( fh_context ctx )
        {
            dstURL = ctx->appendToPath( dstURL, ctx->getDirName() );

            fh_context src = ctx;
            priv_setSrcContext( src, false );
            priv_setDstContext( dstURL, false );
            LG_COPY_D << "FerrisCopy::ShowAttributes() src:" << src->getURL()
                      << " dstURL:" << dstURL
                      << " dstparent:" << dstparent->getURL()
                      << endl;
            perform_copy();
            dstURL = dirname( ctx, dstURL );
        }
    
    void
    FerrisCopy::PrintEA( int i,  const std::string& attr, const std::string& EA )
        {
        }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    void
    FerrisCopy::priv_setSrcContext( fh_context c, bool isInitial )
        {
            src = c;
//            cerr << "priv_setSrcContext() old src:" << srcURL << endl;
            srcURL = src->getURL();
//            cerr << "priv_setSrcContext() clean src:" << srcURL << endl;
        }

    void
    FerrisCopy::priv_setDstContext( const string& _dstURL, bool isInitial )
        {
            dstURL = _dstURL;
            LG_COPY_D << "About to read destination context...dstURL:" << dstURL << endl;

            if( MakeSoftLinksForNonDirs || MakeHardLinksForNonDirs )
            {
                if( toType<int>(getEA( src, "is-dir", "0" )))
                {
                    return;
                }
            }
            
            if( DstAttr.length() )
            {
                dstparent = Resolve( dstURL );
            }
            else
            {
                LG_COPY_D << "Resolve parent of url:" << dstURL << endl;
                dstparent = Resolve( dstURL, RESOLVE_PARENT );
                LG_COPY_D << "Resolve parent, got dstparent:"
                          << dstparent->getURL() << endl;
            }
        }

    FerrisCopy::CopyPorgressSignal_t&
    FerrisCopy::getCopyPorgressSignal()
        {
            return CopyPorgressSignal;
        }

    FerrisCopy::CopyStartSignal_t&
    FerrisCopy::getCopyStartSignal()
        {
            return CopyStartSignal;
        }

    FerrisCopy::CopyEndSignal_t&
    FerrisCopy::getCopyEndSignal()
        {
            return CopyEndSignal;
        }
    


    /*
     * return true if we are trying to copy into ourself.
     */
    bool
    FerrisCopy::AttemptToCopyIntoSelf( fh_context src, fh_context dstparent )
        {
            LG_COPY_D << "AttemptToCopyIntoSelf() CopyIntoSelfTests:" << CopyIntoSelfTests << endl;

            
            /*
             * Assume that the user is not smoking crack. This could get them into
             * big problems when the system has many symlinks and they are not 100%
             * sure that the src != transative.parent( dst )
             */
            if( !CopyIntoSelfTests )
            {
                return false;
            }

            /**
             * Make sure that the src != any of the destination parent directories.
             */
            if( src->getIsNativeContext() && dstparent->getIsNativeContext() )
            {
                string srcdev   = getStrAttr( src, "dontfollow-device", "" );
                string srcinode = getStrAttr( src, "dontfollow-inode", "" );
                fh_context d = dstparent;
                while( d )
                {
                    string dev   = getStrAttr( d, "dontfollow-device", "" );
                    string inode = getStrAttr( d, "dontfollow-inode", "" );

                    if( srcdev == dev && !dev.empty()
                        && srcinode == inode && !inode.empty() )
                    {
                        return true;
                    }
                    
                    if( d->isParentBound() )
                        d = d->getParent();
                    else
                        break;
                }
            }
            
            
            string realsrc = getStrAttr( src,       "realpath", "" );
            string realdst = getStrAttr( dstparent, "realpath", "" );

            LG_COPY_D << "AttemptToCopyIntoSelf() "
                      << " DstIsDirectory:" << DstIsDirectory
                      << " SrcIsDirectory:" << SrcIsDirectory
                      << " src:" << src->getURL()
                      << " dst:" << dstparent->getURL()
                      << " realsrc:" << realsrc
                      << " realdst:" << realdst
                      << " startwith:" << starts_with( realdst, realsrc )
                      << endl;

            if( realsrc.empty() || realdst.empty() )
            {
                bool attemptRecursiveCopy = false;

                if( realsrc.empty() && src->getIsNativeContext() )
                {
                    if( getStrAttr( src, "link-target-relative", "" ).empty() )
                    {
                        attemptRecursiveCopy = true;
                    }
                }
                if( realdst.empty() && dstparent->getIsNativeContext() )
                {
                    if( dstparent->getOverMountContext() != dynamic_cast<Context*>(GetImpl(dstparent)))
                    {
                    }
                    else
                    {
                        attemptRecursiveCopy = true;
                    }
                }
                
                if( attemptRecursiveCopy )
                {
                    getSkippingContextSignal().emit(
                        *this,
                        getSourceDescription(),
                        "error getting real paths" );
                    return true;
                }
                return false;
            }

            /*
             * We could be in trouble if the src is a prefix of the destination
             */
            if( starts_with( realdst, realsrc ) )
            {
                /*
                 * preserving links of sources is ok
                 */
                if( FollowOnlyForSrcUrl || DontFollowLinks )
                {
                    if( toint( getEA( src, "is-link", "0" )))
                    {
                        return false;
                    }
                }

                /*
                 * Assume this is a prefix.
                 */
//                return true;
            }
            
            return false;
        }
    
    
    void
    FerrisCopy::perform_copy()
        {
            try
            {
                LG_COPY_D << "perform_copy(top)" << endl;

                if( AttemptToCopyIntoSelf( src, dstparent ) )
                {
                    LG_COPY_D << "perform_copy() AttemptToCopyIntoSelf is true, src:" << getSourceDescription() << endl;
                    fh_stringstream ss;
                    ss << "Attempt to copy into self";
                    getSkippingContextSignal().emit(
                        *this,
                        getSourceDescription(),
                        tostr(ss) );
                    return;
                }
                
                if( OneFileSystem )
                {
                    dev_t d = toType<dev_t>(getEA( src, "device", "0" ));
                    if( CurrentFileSystemDev != d )
                    {
                        getSkippingContextSignal().emit(
                            *this,
                            getSourceDescription(),
                            "Attempt to move to a different filesystem" );
                        return;
                    }
                }
                LG_COPY_D << "perform_copy(2)" << endl;

                if( MakeSoftLinksForNonDirs || MakeHardLinksForNonDirs )
                {
                    if( toType<int>(getEA( src, "is-dir", "0" )))
                    {
                        getSkippingContextSignal().emit(
                            *this,
                            getSourceDescription(),
                            "omitting directory" );
                        return;
                    }
                }
                
                LG_COPY_D << "perform_copy(3)" << endl;
                LG_COPY_D << "dstparent isBound: " << isBound( dstparent ) << endl;
                LG_COPY_D << "dstparent    path: " << dstparent->getDirPath() << endl;

                /*
                 * Some checks on the destination
                 */
                if( DstAttr.length() )
                {
                    if( Interactive || UpdateMode )
                    {
                        try
                        {
                            fh_attribute attr = dstparent->getAttribute( DstAttr );

                            if( !getAskReplaceAttributeSignal().emit(
                                    *this, src, dst,
                                    getSourceDescription(),
                                    getDestinationDescription( dstparent ),
                                    attr ))
                            {
                                getSkippingContextSignal().emit(
                                    *this,
                                    getSourceDescription(),
                                    "skipping" );
                                return;
                            }
                        }
                        catch(...)
                        {}
                    }
                    if( !backupMaker.impotent() )
                    {
                        try
                        {
                            fh_attribute attr = dstparent->getAttribute( DstAttr );
                            backupMaker( attr );
                        }
                        catch(...)
                        {}
                    }
                    
                    dstparent->acquireAttribute( DstAttr );
                    dst = dstparent;
                }
                else
                {
                    string rdn = getDstName();
                    LG_COPY_D << "copying to context dstparent:" << dstparent->getURL()
                              << " rdn:" << rdn
                              << endl;
                    if( dstparent->isSubContextBound( rdn ) )
                    {
                        fh_context c = dstparent->getSubContext( rdn );
                        int isDir = toint(getStrAttr( c, "is-dir", "0" ));

                        LG_COPY_D << "copying to existing c:" << c->getURL()
                                  << " running checks isDir:" << isDir << endl;

                        /*
                         * We need to run some extra checks and code if the destination
                         * already exists. We check again if the subcontext is bound
                         * because if the destination was a dir, then dstparent will
                         * have changed.
                         */
                        if( !isDir || dstparent->isSubContextBound( rdn ) )
                        {
                            if( !isDir )
                            {
                                if( Interactive )
                                {
                                    fh_context dst = dstparent->getSubContext( rdn );

                                    LG_COPY_D << "calling getAskReplaceContextSignal() emit" << endl;
                                    LG_COPY_D << " srcdesc:" << getSourceDescription()
                                              << " dstdesc:" << getDestinationDescription( dst )
                                              << endl;
                                
                                    if( !getAskReplaceContextSignal().emit(
                                            *this, src, dst,
                                            getSourceDescription(),
                                            getDestinationDescription( dst ) ))
                                    {
                                        getSkippingContextSignal().emit(
                                            *this,
                                            getSourceDescription(),
                                            "skipping" );
                                        return;
                                    }
                                }
                                if( !backupMaker.impotent() )
                                {
                                    fh_context c = dstparent->getSubContext( rdn );
                                    backupMaker( c );
                                }
                            
                                if( UpdateMode )
                                {
                                    fh_context dc = dstparent->getSubContext( rdn );
                                    time_t srctt = toType<time_t>(getEA( src, "mtime", "-1" ));
                                    time_t dsttt = toType<time_t>(getEA( dc, "mtime", "-1" ));

                                    if( srctt == -1 || dsttt == -1 )
                                    {
                                        getSkippingContextSignal().emit(
                                            *this,
                                            getSourceDescription(),
                                            "skipping" );
                                        return;
                                    }
                                    if( srctt <= dsttt )
                                    {
                                        getSkippingContextSignal().emit(
                                            *this,
                                            getSourceDescription(),
                                            "src not newer, skipping" );
                                        return;
                                    }
                                }
                            }

                            /*
                             * We only try to keep file/dir objects around if the src and
                             * dst are the same type.
                             */
                            bool RemoveDst = false;
                            {
                                fh_context c = dstparent->getSubContext( rdn );
                                string isf = "is-file";
                                string isd = "is-dir";
                                
                                if( getEA( src, isf, "" ) != getStrAttr( c, isf, "" ) ||
                                    getEA( src, isd, "" ) != getStrAttr( c, isd, "" ) )
                                {
                                    RemoveDst = true;
                                }
                            }
                            
                            LG_COPY_D << "About to maybe remove dst."
                                      << " dstparent:" << dstparent->getURL()
                                      << " rdn:" << rdn
                                      << " RemoveDst:" << RemoveDst
                                      << " ForceRemovePremptively:" << ForceRemovePremptively
                                      << " src.file:" << getEA( src, "is-file", "" )
                                      << " src.dir:" << getEA( src, "is-dir", "" )
                                      << " dst.file:" << getStrAttr( src, "is-file", "" )
                                      << " dst.dir:" << getStrAttr( src, "is-dir", "" )
                                      << endl;
                            
                            if( ForceRemovePremptively || RemoveDst )
                            {
                                dstparent->remove( rdn );
//                                CreateObj( dstparent, rdn );
                            }
                        }
                    }

                    LG_COPY_D << "Setting dst. rdn:" << rdn
                              << " dstparent:" << dstparent->getURL()
                              << endl;
                    
                    if( dstparent->isSubContextBound( rdn ) )
                    {
                        dst = dstparent->getSubContext( rdn );
                    }
                    else
                    {
                        fh_context dc = CreateObj( dstparent, rdn );
                        dst = dc;
                    }
                }

                LG_COPY_D << "perform_copy() checking self copy" << endl;
                
                {
                    fh_attribute realdst;
                    if( DstAttr.length() )
                    {
                        if( DstAttrNoCreate )
                            realdst = dst->getAttribute( DstAttr );
                        else
                            realdst = dst->acquireAttribute( DstAttr );
                    }
                    else
                    {
                        realdst = dst->toAttribute();
                    }


                    LG_COPY_D << "cp "  << getSourceDescription()
                              << " to " << getDestinationDescription() << nl;

                    if( Verbose )
                    {
                        getCopyVerboseSignal().emit( *this, src, dst,
                                                     getSourceDescription(),
                                                     getDestinationDescription() );
                    }
                    
                    
                    /*
                     * do we need to copy any bytes: make sure we
                     * follow links working out of the source has content.
                     */
                    bool CopyFileBytes = toType<int>(getEA( src, "is-file", "0" ));
                    if( FlattenSpecialToFile )
                    {
                        CopyFileBytes = !toType<int>(getEA( src, "is-dir", "0" ));
                    }
                    if( !src->isAttributeBound( "is-file" ) )
                    {
//                        CopyFileBytes = true;
                        CopyFileBytes = isFalse(getEA( src, "is-dir", "1" ));
                    }
                    LG_COPY_D << "src.is-file:" << toType<int>(getEA( src, "is-file", "0" )) << endl;
                    LG_COPY_D << "CopyFileBytes:" << CopyFileBytes << endl;
                    
                    
                    if( CopyFileBytes )
                    {
                        m_DestinationFDCache = 0;
                        
                        LG_COPY_D << "destination is common file:"
                                  << realdst->getDirPath() << endl;

                        if( src && dst )
                        {
                            //
                            // Give the destination context a chance to perform specific
                            // extra tasks knowing what the source is.
                            // eg. webphoto classes might want to update online tags from
                            //     the source image.
                            //
                            LG_WEBPHOTO_D << "Setting ferris-pre-copy-action for dst:" << dst->getURL()
                                          << " the source is:" << src->getURL()
                                          << endl;
                            setStrAttr( dst, "ferris-pre-copy-action",
                                        src->getURL(), false, false );
                        }

// ///////////////////////////////////////////////////////                        
// ///////////////////////////////////////////////////////                        
// ///////////////////////////////////////////////////////                        
// ///////////////////////////////////////////////////////                        
// ///////////////////////////////////////////////////////                        
//                         /// FIXME: TESTING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//                         {
//                             std::streamsize inputsz = toType<streamsize>(getEA( src, "size", "-1" ));
//                             fh_ostream  oss = OpenDestination( realdst, inputsz );
// //                            fh_ifstream iss( "/home/ben/Documents/small-video-test.mp4" );
//                             fh_ifstream iss( "/tmp/medium-test-image.jpg" );

     
//                             oss = copyTo(
//                                 iss, oss,
//                                 toType<streamsize>( getEA( src, "block-size", "4096" )),
//                                 toType<streamsize>( getEA( src, "size", "-1" )));
                            
//                             oss << flush;
//                             exit(0);
//                         }
                        
                        
                        std::streamsize inputsz = toType<streamsize>(getEA( src, "size", "-1" ));
                        fh_ostream oss = OpenDestination( realdst, inputsz );
                        oss = WrapForHoles( src, oss );

                        if( SrcAttr.length() )
                        {
                            fh_attribute attr = src->getAttribute( SrcAttr );
                            fh_istream iss  = attr->getIStream();
                            oss = copyTo( iss, oss );
                        }
                        else
                        {
                            ferris_ios::openmode imode = ios::in;

                            bool canUseSendfile = false;
//                             if( m_useSendfileIfPossible )
//                             {
//                                 if( src->getIsNativeContext() &&
//                                     dst->getIsNativeContext() )
//                                 {
//                                     canUseSendfile = true;
//                                 }
//                             }

//                             if( canUseSendfile )
//                             {
//                                 off_t offset = 0;
//                                 size_t bytesToCopyInOneCall = m_sendfileChunkSize;
//                                 ssize_t bytesDone = 0;
//                                 ssize_t sz = -1;

//                                 int in_fd  = open( src->getDirPath().c_str(),
//                                                    O_LARGEFILE | O_RDONLY );
//                                 int out_fd = open( dst->getDirPath().c_str(),
//                                                    O_LARGEFILE | O_TRUNC | O_WRONLY );

//                                 while( bytesDone < inputsz )
//                                 {
//                                     sz = sendfile( out_fd, in_fd,
//                                                    &offset,
//                                                    bytesToCopyInOneCall );
//                                     if( sz < 0 )
//                                     {
//                                         int eno = errno;
//                                         stringstream ss;
//                                         ss << "Error copying with sendfile(2) src:" << src->getURL()
//                                            << " to dst:" << dst->getURL()
//                                            << endl;
//                                         cerr << ss.str() << endl;
//                                         cerr << " in_fd:" << in_fd  << endl;
//                                         cerr << " out_fd:" << out_fd  << endl;
//                                         cerr << "bytesToCopyInOneCall:" << bytesToCopyInOneCall << endl;
//                                         cerr << "eno:" << eno << endl;
//                                         ThrowFromErrno( eno, ss.str(), 0 );
//                                     }

//                                     getCopyPorgressSignal().emit( *this,
//                                                                   bytesDone,
//                                                                   bytesToCopyInOneCall,
//                                                                   inputsz );
                                    
//                                     bytesDone += sz;
//                                 }

//                                 cerr << "sendfile(2) copy complete"
//                                      << " bytesDone:" << bytesDone
//                                      << " inputsz:" << inputsz
//                                      << endl;
//                                 if( bytesDone != inputsz )
//                                 {
//                                     stringstream ss;
//                                     ss << "Final bytes copied count is incorrect."
//                                        << " Wanted:" << inputsz
//                                        << " But really copied:" << bytesDone
//                                        << "\n Copying with sendfile(2) src:" << src->getURL()
//                                        << " to dst:" << dst->getURL()
//                                        << endl;
//                                     Throw_CopyFailed( ss.str(), 0 );
//                                 }
//                             }
//                             else

                            {
                                bool inInMMap = InputInMemoryMappedMode;
                                if( AutoInputInMemoryMappedModeSize )
                                    inInMMap |= inputsz > AutoInputInMemoryMappedModeSize;
                                
                                if( inInMMap )
                                {
                                    LG_COPY_D << "Turning on memory mapping for input file:"
                                              << src->getURL() << endl;
                                    imode |= ferris_ios::o_mmap;
                                    imode |= ferris_ios::o_mseq;
                                }

                                LG_COPY_D << "copying bytes from src:" << src->getDirPath() << endl;
                                fh_istream iss  = src->getIStream( imode );
                                oss = copyTo(
                                    iss, oss,
                                    toType<streamsize>( getEA( src, "block-size", "4096" )),
                                    toType<streamsize>( getEA( src, "size", "-1" )));
                                oss << flush;
                            }

                            if( perform_fsyncAfterEachFile )
                            {
                                int fd = open( dst->getDirPath().c_str(), O_WRONLY | O_LARGEFILE, S_IRWXU );
                                if( fd == -1 )
                                {
                                    string es = errnum_to_string( "", errno );
                                    cerr << "open for fsync failed... reason:" << es << endl;
                                }
                                if( fd > 0 )
                                {
                                    if( -1 == fsync( fd ) )
                                    {
                                        string es = errnum_to_string( "", errno );
                                        cerr << "fsync failed... reason:" << es << endl;
                                    }
                                    close( fd );
                                }
                            }
                        }
                        
//                         /* Maybe read from an attribute not a context */
//                         fh_attribute realsrc = src->toAttribute();
//                         if( SrcAttr.length() )
//                         {
//                             realsrc = src->getAttribute( SrcAttr );
//                         }
//                        oss = realsrc->copyTo( oss );

                        if( m_DestinationFDCache )
                        {
                            close( m_DestinationFDCache );
                            m_DestinationFDCache = 0;
                        }
                    }
                }
                
                PreserveEA();
            }
            catch( FerrisCreateSubContextFailed& e )
            {
                LG_COPY_D << "FerrisCreateSubContextFailed exception& e:" << e.what() << endl;
                throw;
            }
            
            catch (FerrisParentNotSetError& e)
            {
                LG_COPY_D << "pns exception& e:" << e.what() << endl;
                throw;
            }
            catch( AttributeNotWritable& e)
            {
                LG_COPY_D << "anw exception& e:" << e.what() << endl;
                throw;
            }
            catch( CanNotGetStream& e )
            {
                LG_COPY_D << "ngs exception& e:" << e.what() << endl;
                throw;
            }
            catch( NoSuchSubContext& e )
            {
                LG_COPY_D << "no such sub exception& e:" << e.what() << endl;
                throw;
            }
            catch( exception& e )
            {
                LG_COPY_D << "exception& e:" << e.what() << endl;
            }
        }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    static void m_AttributeUpdater_NULL( FerrisCopy*,
                                         fh_context,
                                         fh_context,
                                         std::string,
                                         std::string )
    {
    }
    
    void
    FerrisCopy::maybeStripTrailingSlashes()
        {
            if( StripTrailingSlashes )
            {
                PostfixTrimmer trimmer;
                trimmer.push_back( "/" );
                srcURL = trimmer( srcURL );
                dstURL = trimmer( dstURL );
            }
        }
    
    FerrisCopy::FerrisCopy()
        :
        SrcAttr(""),
        DstAttr(""),
        DstAttrNoCreate( false ),
        srcURL(""),
        dstURL(""),
        ForceOverWrite( false ),
        ForceRemovePremptively( false ),
        Interactive( false ),
        Verbose( false ),
        UpdateMode( false ),
        StripTrailingSlashes( false ),
        CopySrcWithParents( false ),
        OneFileSystem( false ),
        CurrentFileSystemDev( 0 ),
        FlattenSpecialToFile( false ),
        PreserveObjectMode( false ),
        PreserveMTime( false ),
        PreserveATime( false ),
        PreserveOwner( false ),
        PreserveGroup( false ),
        MakeSoftLinksForNonDirs( false ),
        MakeHardLinksForNonDirs( false ),
        FollowOnlyForSrcUrl( false ),
        DontFollowLinks( false ),
        HoleMode( HOLEMODE_NEVER ),
        MakeSoftLinksForNonDirsAbsolute( false ),
        Recurse( false ),
        Sloth( false ),
        CopyIntoSelfTests( true ),
        AutoClose( false ),
        hadUserInteraction( false ),
        DstIsDirectory( false ),
        SrcIsDirectory( false ),
        DoNotTryToOverMountDst( false ),
//        m_AttributeUpdater( SigC::ptr_fun( &m_AttributeUpdater_NULL ) ), 
        m_AttributeUpdaterInUse( false ),
        m_ExplicitSELinuxType( "" ),
        m_ExplicitSELinuxContext( "" ),
        m_CloneSELinuxContext( 0 ),
        m_PrecacheSourceSize( false ),
        m_PreserveRecommendedEA( 0 ),
        m_PreserveRDFEA( true ),
        m_PreserveFerrisTypeEA( true ),
        perform_fsyncAfterEachFile( false ),
        perform_preallocation( false ),
        preallocate_with_fallocate( false ),
        preallocate_with_ftruncate( false ),
        InputInMemoryMappedMode( 
            isTrue( getEDBString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_BY_DEFAULT, "0" ))),
        AutoInputInMemoryMappedModeSize(
            Util::convertByteString(
                getEDBString( FDB_GENERAL, COPY_INPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN, "0" ))),
        OutputInMemoryMappedMode(
            isTrue( getEDBString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_BY_DEFAULT, "0" ))),
        AutoOutputInMemoryMappedModeSize(
            Util::convertByteString(
                getEDBString( FDB_GENERAL, COPY_OUTPUT_IN_MMAP_MODE_FOR_OBJECTS_LARGER_THAN, "0" ))),
        m_useSendfileIfPossible(
            Util::convertByteString(
                getEDBString( FDB_GENERAL, USE_SENDFILE_IF_POSSIBLE, "0" ))),
        m_sendfileChunkSize(
            Util::convertByteString(
                getEDBString( FDB_GENERAL, SENDFILE_CHUNK_SIZE, "262144" ))),
        TotalBytesCopied( 0 ),
        TotalBytesToCopy( 0 )
//         OutputInDirectMode( false ),
//         AutoOutputInDirectModeSize( 0 )
    {
        if( m_sendfileChunkSize < 4096 )
            m_sendfileChunkSize = 262144;
        
        setCloneSELinuxContext( 0 );
        backupMaker.setMode( Util::BackupMaker::MODE_NONE );
    }

    bool
    FerrisCopy::setCopyIntoSelfTests( bool v )
        {
            CopyIntoSelfTests = v;
            return CopyIntoSelfTests;
        }

    bool
    FerrisCopy::setFSyncAfterEachFile( bool v )
    {
        perform_fsyncAfterEachFile = v;
    }
    
    bool
    FerrisCopy::setPreallocateWith_fallocate( bool v )
    {
        perform_preallocation |= v;
        preallocate_with_fallocate = v;
    }

    bool
    FerrisCopy::setPreallocateWith_ftruncate( bool v )
    {
        perform_preallocation |= v;
        preallocate_with_ftruncate = v;
    }
    
    
    void
    FerrisCopy::setRecurse( bool v )
        {
            Recurse = v;
        }

    void
    FerrisCopy::setSloth( bool v )
    {
        Sloth = v;
    }

    void
    FerrisCopy::setAutoClose( bool v )
    {
        AutoClose = v;
    }
    
    

    void
    FerrisCopy::setHoleMode( HoleMode_t m )
        {
            HoleMode = m;
        }

    void
    FerrisCopy::setPreserveAttributeList( const std::string& v )
    {
//        cerr << "setPreserveAttributeList() v:" << v << endl;
        istringstream iss( v );
        string x;
        while(getline( iss, x, ',' ))
        {
            PreserveAttributeList.push_back( x );
        }
    }
    
    void
    FerrisCopy::setFollowOnlyForSrcUrl( bool v )
        {
            FollowOnlyForSrcUrl = v;
        }
    
    void
    FerrisCopy::setDontFollowLinks( bool v )
        {
            DontFollowLinks = v;
        }

    void
    FerrisCopy::setMakeSoftLinksForNonDirsAbsolute( bool v )
        {
            MakeSoftLinksForNonDirsAbsolute = v;
        }
    
    void
    FerrisCopy::setMakeSoftLinksForNonDirs( bool v )
        {
            MakeSoftLinksForNonDirs = v;
        }

    void
    FerrisCopy::setMakeHardLinksForNonDirs( bool v )
        {
            MakeHardLinksForNonDirs = v;
        }
    
    void
    FerrisCopy::setPreserveMTime( bool v )
        {
            PreserveMTime = v;
        }

    void
    FerrisCopy::setPreserveATime( bool v )
        {
            PreserveATime = v;
        }

    void
    FerrisCopy::setPreserveOwner( bool v )
        {
            PreserveOwner = v;
        }

    void
    FerrisCopy::setPreserveGroup( bool v )
        {
            PreserveGroup = v;
        }
    
    void
    FerrisCopy::setPreserveObjectMode( bool v )
        {
            PreserveObjectMode = v;
        }
    
    void
    FerrisCopy::setFlattenSpecialToFile( bool v )
        {
            FlattenSpecialToFile = v;
        }


    void
    FerrisCopy::setBackupMaker( const Util::BackupMaker& b )
    {
        backupMaker = b;
    }
    
    void
    FerrisCopy::setBackupSuffix( const std::string& s )
        {
            backupMaker.setMode( Util::BackupMaker::MODE_SIMPLE );
            backupMaker.setSuffix( s );
        }

    void
    FerrisCopy::setExplicitSELinuxContext( const std::string& s )
        {
            m_ExplicitSELinuxContext = s;
        }

    void
    FerrisCopy::setExplicitSELinuxType( const std::string& s )
        {
            m_ExplicitSELinuxType = s;
        }

    void
    FerrisCopy::setCloneSELinuxContext( bool v )
        {
            m_CloneSELinuxContext = v;
        }

    void
    FerrisCopy::setPrecacheSourceSize( bool v )
        {
            m_PrecacheSourceSize = v;
        }

    bool
    FerrisCopy::shouldPrecacheSourceSize()
    {
        return m_PrecacheSourceSize;
    }
    
    void
    FerrisCopy::setPreserveRecommendedEA( bool v )
        {
            m_PreserveRecommendedEA = v;
        }

    void
    FerrisCopy::setPreserveRDFEA( bool v )
        {
            m_PreserveRDFEA = v;
        }
    
    void
    FerrisCopy::setPreserveFerrisTypeEA( bool v )
        {
            m_PreserveFerrisTypeEA = v;
        }
    
    
    void
    FerrisCopy::setPerformBackups( const std::string& s )
        {
            backupMaker.setMode( s );
        }
    
    void
    FerrisCopy::setOneFileSystem( bool v )
        {
            OneFileSystem = v;
            CurrentFileSystemDev = 0;
        }
    
    void
    FerrisCopy::setForceOverWrite( bool v )
        {
            ForceOverWrite = v;
        }

    void
    FerrisCopy::setForceRemovePremptively( bool v )
        {
            ForceRemovePremptively = v;
        }
    
    void
    FerrisCopy::setVerbose( bool v )
        {
            Verbose = v;
        }

    void
    FerrisCopy::setStripTrailingSlashes( bool v )
        {
            StripTrailingSlashes = v;
            maybeStripTrailingSlashes();
        }

    void
    FerrisCopy::setCopySrcWithParents( bool v )
        {
            CopySrcWithParents = v;
        }

    void
    FerrisCopy::setInteractive( bool v )
        {
            Interactive = v;
        }

    void
    FerrisCopy::setUpdateMode( bool v )
        {
            UpdateMode = v;
        }
    
    void
    FerrisCopy::setSrcAttr( const string& s )
        {
            SrcAttr = s;
        }

    void
    FerrisCopy::setDstAttr( const string& s )
        {
            DstAttr = s;
        }
    
    void
    FerrisCopy::setDstAttrNoCreate( bool v )
    {
        DstAttrNoCreate = v;
    }

    void
    FerrisCopy::setSrcURL( const string& s )
        {
            srcURL = s;
            maybeStripTrailingSlashes();
            LG_COPY_D << "setSrcURL() have:" << srcURL << endl;
        }

    void
    FerrisCopy::setDstURL( const string& s )
        {
            dstURL = s;
            dstURL_setByUser = s;
            maybeStripTrailingSlashes();
            LG_COPY_D << "setDstURL() have:" << dstURL << endl;
        }
    
    void
    FerrisCopy::setDstIsDirectory( bool v )
    {
        DstIsDirectory = v;
    }

    void
    FerrisCopy::setSrcIsDirectory( bool v )
    {
        SrcIsDirectory = v;
    }

    void
    FerrisCopy::setDoNotTryToOverMountDst( bool v )
    {
        DoNotTryToOverMountDst = v;
    }
    

    void
    FerrisCopy::setInputInMemoryMappedMode( bool v )
    {
        InputInMemoryMappedMode = v;
    }
    
    void
    FerrisCopy::setAutoInputInMemoryMappedModeSize( std::streamsize v )
    {
        AutoInputInMemoryMappedModeSize = v;
    }

    void
    FerrisCopy::setOutputInMemoryMappedMode( bool v )
    {
        OutputInMemoryMappedMode = v;
    }

    void
    FerrisCopy::setAutoOutputInMemoryMappedModeSize( std::streamsize v )
    {
        AutoOutputInMemoryMappedModeSize = v;
    }

    void
    FerrisCopy::setUseSendfileIfPossible( bool v )
    {
        m_useSendfileIfPossible = v;
    }

    void
    FerrisCopy::setSendfileChunkSize( long v )
    {
        m_sendfileChunkSize = v;
    }
    
    FerrisCopy::m_AttributeUpdater_t&
    FerrisCopy::getAttributeUpdaterSignal()
    {
        m_AttributeUpdaterInUse = true;
        return m_AttributeUpdater;
    }

//     void
//     FerrisCopy::setOutputInDirectMode( bool v )
//     {
//         OutputInDirectMode = v;
//     }

//     void
//     FerrisCopy::setAutoOutputInDirectModeSize( std::streamsize v )
//     {
//         AutoOutputInDirectModeSize = v;
//     }
    

    string
    FerrisCopy::getDstName()
        {
            string rdn = dstparent->getLastPartOfName( dstURL );
//             if( string::npos != dstURL.rfind( "/" ) )
//             {
//                 rdn = dstURL.substr( dstURL.rfind( "/" )+1 );
//             }

            LG_COPY_D << "getDstName() dstURL:" << dstURL
                      << " rdn:" << rdn
                      << endl;
            return rdn;
        }
    
    fh_iostream
    FerrisCopy::OpenDestination( fh_attribute& realdst,
                                 std::streamsize inputsz,
                                 bool firstcall )
        {
            fh_iostream ret;
            try
            {
                LG_COPY_D << "FerrisCopy::OpenDestination() realdst:" << realdst->getDirPath() << endl;
                
                /*
                 * open the target iostream taking into account any flags
                 * that the user may have set effecting the output file
                 */
                ferris_ios::openmode m = ios::out | ios::trunc;

                bool outInMMap = OutputInMemoryMappedMode;
                if( AutoOutputInMemoryMappedModeSize )
                    outInMMap |= inputsz > AutoOutputInMemoryMappedModeSize;

                LG_COPY_D << "FerrisCopy::OpenDestination() DstAttr:" << DstAttr
                          << " outInMMap:" << outInMMap << endl;

                if( DstAttr.empty() && outInMMap )
                {
                    LG_COPY_D << "Turning on memory mapping for output file:"
                              << src->getURL() << endl;
                    m |= ferris_ios::o_mmap;
                    m |= ferris_ios::o_mseq;

                    LG_COPY_D << "Testing if we can trunc the output file to"
                              << "the size of the input file to improve speed"
                              << " dst:" << dst->getURL()
                              << " is-native:" << getStrAttr( dst, "is-native", "0" )
                              << " inputsize:" << inputsz
                              << endl;
                    //
                    // It will help performance greatly to trunc the output file to the
                    // desired size right off the bat
                    //
                    if( isTrue( getStrAttr( dst, "is-native", "0" )))
                    {
                        int rc = truncate( dst->getDirPath().c_str(), inputsz );
                        if( rc )
                        {
                            int en = errno;
                            fh_stringstream ss;
                            ss << "Failed to set the output file to the input file size:" << inputsz
                               << " src:" << getSourceDescription()
                               << " dst:" << getDestinationDescription()
                               << endl;
                            ThrowFromErrno( en, tostr(ss) );
                        }

                        // Dont retruncate the output file we just created
                        if( m & ios::trunc )
                            m ^= ios::trunc;
                    }
                }

                LG_COPY_D << "DstAttr.empty():" << DstAttr.empty() << endl;
                LG_COPY_D << "outInMMap:" << outInMMap << endl;

                //
                // try to preallocate the file bytes.
                //
                if( perform_preallocation && DstAttr.empty() && !outInMMap )
                {
                    LG_COPY_D << "is native:" << isTrue( getStrAttr( dst, "is-native", "0" )) << endl;
                    
                    if( isTrue( getStrAttr( dst, "is-native", "0" )))
                    {
                        LG_COPY_D << "dst:" << dst->getDirPath().c_str() << endl;
                        
                        int fd = open( dst->getDirPath().c_str(),
                                       O_WRONLY | O_CREAT| O_LARGEFILE | O_TRUNC, S_IRWXU );
                        if( fd == -1 )
                        {
                            string es = errnum_to_string( "", errno );
                            LG_COPY_D << "open for preallocation failed... reason:" << es << endl;
                            cerr << "open for preallocation failed... reason:" << es << endl;
                        }
                        if( fd > 0 )
                        {
#ifdef PLATFORM_OSX
                            cerr << "WARNING: preallocation not implemented on osx." << endl;
#else
                            if( preallocate_with_fallocate )
                            {
                                cerr << "have fd, tring to fallocate()" << endl;
                                long rc = posix_fallocate( fd, 0, inputsz );
                                cerr << "fallocate() rc:" << rc << endl;
                                if( rc )
                                {
                                    string es = errnum_to_string( "", errno );
                                    LG_COPY_W << "failed to preallocate space for file copy, fragmentation might result." << endl
                                              << " reason:" << es
                                              << endl;
                                    cerr << "failed to preallocate space for file copy, fragmentation might result." << endl
                                         << " reason:" << es
                                         << endl;
                                }
                            }
                            if( preallocate_with_ftruncate )
                            {
                                int rc = ftruncate( fd, inputsz );
                                if( rc == -1 )
                                {
                                    string es = errnum_to_string( "", errno );
                                    LG_COPY_W << "failed to preallocate space for file copy with ftruncate, fragmentation might result." << endl
                                              << " reason:" << es
                                              << endl;
                                    cerr << "failed to preallocate space for file copy with ftruncate, fragmentation might result." << endl
                                         << " reason:" << es
                                         << endl;
                                }
                            }
                            posix_fadvise( fd, 0, inputsz, POSIX_FADV_SEQUENTIAL | POSIX_FADV_DONTNEED);
                            // closed via
//                            m_DestinationFDCache = fd;
#endif // not osx
                            close( fd );
                        }
                    }
                }
                
//                 if( OutputInDirectMode )
//                     m |= ferris_ios::o_direct;

//                 if( inputsz > AutoOutputInDirectModeSize )
//                 {
//                     m |= ferris_ios::o_direct;
//                 }

//                 cerr << "FerrisCopy::OpenDestination() dst:" << realdst->getDirPath()
//                      << " mode:" << int(m)
//                      << " r:" << ios::in
//                      << " w:" << ios::out
//                      << " t:" << ios::trunc
//                      << endl;

                if( DstAttr.empty() )
                {
                    int newmode = S_IWUSR | S_IRUSR | S_IXUSR;

                    LG_COPY_D << "FerrisCopy::OpenDestination() no dstattr. setting mode." << endl;
                    LG_COPY_D << "FerrisCopy::OpenDestination() dst:" << dst << endl;
                    LG_COPY_D << "FerrisCopy::OpenDestination() newmode:" << toOctalString(newmode) << endl;
                    LG_COPY_D << "FerrisCopy::OpenDestination() dst-is-native:"
                              << dst->getIsNativeContext() << endl;

                    try
                    {
                       setStrAttr( dst, "mode", toOctalString(newmode),
                                   false, dst->getIsNativeContext()  );
                    }
                    catch( exception& e )
                    {
                       LG_COPY_W << "Error setting mode to:" << toOctalString(newmode)
                                 << " for target:" << dst->getURL()
                                 << endl;
                    }
                }

                //
                // let the destination file know the source
                // in case it wants to perform additional procesisng.
                // FIXME:
                //
                if( DstAttr.empty() )
                {
                }
                
                LG_COPY_D << "FerrisCopy::OpenDestination() getting ostream..." << endl;
                ret = realdst->getIOStream( m );
                
            }
            catch( exception& e )
            {
                if( ForceOverWrite && firstcall )
                {
                    if( DstAttr.length() )
                    {
                        throw e;
                    }
                    
                    fh_context parent = dstparent;
                    string rdn  = getDstName();
                    LG_COPY_D << " -f option. realdst:" << realdst->getDirPath()
                              << " parent:" << parent->getDirPath()
                              << " rdn:" << rdn
                              << endl;
                    
                    parent->remove( rdn );
                    realdst = CreateObj( parent, rdn )->toAttribute();
                    return OpenDestination( realdst, inputsz, false );
                }
                throw;
            }
            return ret;
        }


    /**
     * Used by GetEA() to not follow for EA
     */
    string
    FerrisCopy::getEA_DontFollow( fh_context c, const std::string& n, string d )
        {
                fh_stringstream ss;
                ss << "dontfollow-" << n;
                string eaname = tostr(ss);

                LG_COPY_D << "getEA_DontFollow() c:" << c->getURL()
                          << " n:" << n
                          << " n.bound:" << c->isAttributeBound( n )
                          << " eaname:" << eaname
                          << " eaname.bound:" << c->isAttributeBound( eaname )
                          << endl;
                
                if( c->isAttributeBound( eaname ) )
                {
                    return getStrAttr( c, eaname, d );
                }
                return getStrAttr( c, n, d );
        }
    
    /**
     * Obtain EA from either the lstat() data or stat() data depending
     * on what options the user has set.
     *
     * Params are the same as for the getStrAttr() function.
     */
    string
    FerrisCopy::getEA( fh_context c, const std::string& n, string d )
        {
            if( FollowOnlyForSrcUrl )
            {
                if( srcIsCmdLineParam() )
                {
                    return getStrAttr( c, n, d );
                }
                return getEA_DontFollow( c, n, d );
            }

            if( DontFollowLinks )
            {
                return getEA_DontFollow( c, n, d );
            }
            return getStrAttr( c, n, d );
        }

    /**
     * Returns 1 if the current src is one that was given as setSrcURL()
     */
    bool
    FerrisCopy::srcIsCmdLineParam()
        {
            fh_context c = Resolve( srcURL_Root );
            return c->getURL() == srcURL;
        }

    fh_context
    FerrisCopy::CreateObj( fh_context c, string s )
    {
        fh_context ret = priv_CreateObj( c, s );
        if( !m_ExplicitSELinuxContext.empty() )
        {
            setStrAttr( ret,
                        "dontfollow-selinux-context",
                        m_ExplicitSELinuxContext,
                        true, true );
        }
        if( !m_ExplicitSELinuxType.empty() )
        {
            LG_COPY_D << "setting SEType to:" << m_ExplicitSELinuxType << endl;
            setStrAttr( ret,
                        "dontfollow-selinux-type",
                        m_ExplicitSELinuxType,
                        true, true );
        }
        return ret;
    }
    
    fh_context
    FerrisCopy::priv_CreateObj( fh_context c, string s )
        {
            LG_COPY_D << "CreateObj() c:" << c->getURL()
                      << " s:" << s
                      << endl;

            if( c->isSubContextBound( s ) )
            {
                fh_context dc = c->getSubContext( s );
                string srctype = getEA( src, "filesystem-filetype", "" );
                string dsttype = getStrAttr( dc, "filesystem-filetype", "" );
                LG_COPY_D << "CreateObj() dc:" << dc->getURL()
                          << " srctype:" << srctype
                          << " dsttype:" << dsttype
                          << endl;

                /* If the attribute is there then it must be the same for both */
                if( srctype == dsttype )
                {
                    return dc;
                }
            }
            
            if( toType<int>(getEA( src, "is-dir", "0" )))
            {
//                 fh_mdcontext md = new f_mdcontext();
//                 fh_mdcontext child = md->setChild( "dir", "" );

                static fh_mdcontext md = new f_mdcontext();
                static fh_mdcontext child = md->setChild( "dir", "" );
                md->AddRef();
                child->AddRef();
                child->setChild( "mode", "700" );

                
                child->setChild( "name", s );

                LG_COPY_D << "Create dir s:" << s << endl;

                if( PreserveObjectMode )
                {
//                     string modestr = getEA( src, "mode", "-1" );
//                     LG_COPY_D << "Create dir mode:" << modestr << endl;
//                     child->setChild( "mode", modestr);

                    int newmode = Factory::MakeInitializationMode( getEA( src, "mode", "0" ) );
                    newmode |= S_IWUSR | S_IRUSR | S_IXUSR;
                    LG_COPY_D << "Create dir mode:" << toOctalString(newmode) << endl;
                    child->setChild( "mode", toOctalString(newmode) );
                    
                }
                
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }

            if( MakeSoftLinksForNonDirs || MakeHardLinksForNonDirs )
            {
                if( c->isSubContextBound( s ) && ForceOverWrite )
                {
                    c->remove( s );
                }
            }
            
            if( MakeSoftLinksForNonDirs )
            {
                string target = srcURL_Root;
                
                /* switch modes if the user is doing a -R/-r copy */
                if( srcIsCmdLineParam() )
                {
                    LG_COPY_D << "making softlinks for recursive mode always"
                         << " makes absolute links, switching to"
                         << " --symbolic-link-absolute mode" << endl;
                    MakeSoftLinksForNonDirsAbsolute = true;
                }
                if( MakeSoftLinksForNonDirsAbsolute )
                {
                    target = src->getDirPath();
                }
                
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "softlink", "" );
                child->setChild( "name", s );
                child->setChild( "link-target", target );

                LG_COPY_D << "Create softlink s:" << s
                          << " c:" << c->getURL()
                          << " link-target:" << src->getDirPath()
                          << endl;
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }
            if( MakeHardLinksForNonDirs )
            {
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "hardlink", "" );
                child->setChild( "name", s );
                child->setChild( "link-target", src->getDirPath() );

                LG_COPY_D << "Create hardlink s:" << s
                          << " link-target:" << src->getDirPath()
                          << endl;
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }
            
            LG_COPY_D << "working out what type of destination file to make" << endl;
            if( FlattenSpecialToFile )
            {
                LG_COPY_D << "Flatten all non-dirs to a file. s:" << s << endl;
                return Shell::CreateFile( c, s );
            }

            string objtype = getEA( src, "filesystem-filetype", "regular file" );
            LG_COPY_D << "objtype:" << objtype << endl;
            if( objtype == "unknown" )
            {
                /* FIXME: */
                objtype = "regular file";
            }

            LG_COPY_D << "objtype:" << objtype << endl;
            if( objtype == "regular file" )
            {
                LG_COPY_D << "Create a regular file c:" << c->getURL()
                          << " name:" << s
                          << endl;
                
//                 fh_mdcontext md = new f_mdcontext();
//                 fh_mdcontext child = md->setChild( "file", "" );
                static fh_mdcontext md = new f_mdcontext();
                static fh_mdcontext child = md->setChild( "file", "" );
                md->AddRef();
                child->AddRef();
                md->AddRef();
                child->AddRef();
                child->setChild( "mode", "600" );
                child->setChild( "preallocate", "" );
                child->setChild( "size", "" );



                
                
                child->setChild( "name", s );
                if( PreserveObjectMode )
                {
                    int newmode = Factory::MakeInitializationMode( getEA( src, "mode", "0" ) );
//                     cerr << "FCP: New file mode:" << newmode 
//                          << " oct:" << oct << newmode << dec << endl;
                    newmode |= S_IWUSR | S_IRUSR;
//                     cerr << "FCP2: New file mode:" << newmode 
//                          << " oct:" << oct << newmode << dec
//                          << " str:" << tostr(newmode)
//                          << " Ostr:" << toOctalString(newmode)
//                          << endl;

                    child->setChild( "mode", toOctalString(newmode) );
                }
                    
                string preallocsize = getEA( src, "size", "" );
                if( preallocsize.length() )
                {
                    child->setChild( "preallocate", preallocsize );
                    child->setChild( "size", preallocsize );
                }
                
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }

            LG_COPY_D << "objtype:" << objtype << endl;
            if( objtype == "fifo" )
            {
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "fifo", "" );
                child->setChild( "name", s );
                if( PreserveObjectMode )
                    child->setChild( "mode",   getEA( src, "mode", "-1" ));
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }

            LG_COPY_D << "objtype4:" << objtype << endl;
            if( objtype == "symbolic link" )
            {
                LG_COPY_D << "Create a symlink for s:" << s << endl;
                string target = getEA( src, "link-target-relative", "" );
                if( target.empty() )
                    target = getEA( src, "link-target", "" );
                
                LG_COPY_D << "target:" << target << endl;

                cerr  << "Create a symlink for s:" << s
                      << " src:" << getSourceDescription()
                      << " c:" << c->getURL()
                      << " target:" << target
                      << " size:" << getEA( src, "size", "-1" )
                      << endl;
                
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "softlink", "" );
                child->setChild( "name", s );
                child->setChild( "link-target", target );
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }
            if( toType<int>(getEA( src, "is-special", "0" )))
            {
                fh_mdcontext md = new f_mdcontext();
                fh_mdcontext child = md->setChild( "special", "" );
                child->setChild( "name", s );
                child->setChild( "device", getEA( src, "device", "0" ));
                if( PreserveObjectMode )
                    child->setChild( "mode",   getEA( src, "mode", "-1" ));
                fh_context newc = c->createSubContext( "", md );
                return newc;
            }

            /* FIXME: what to do here! */
        }

    void
    FerrisCopy::PreserveEA( const std::string& eaname,
                            bool tryCreate,
                            const std::string& ExplicitPluginName )
        {
            string s = getEA( src, eaname, "" );
            LG_COPY_D << "PreserveEA() eaname:" << eaname
                      << " src:" << src->getURL()
                      << " dst:" << dst->getURL()
                      << " s:" << s 
                      << endl;

            if( s.length() )
            {
                if( tryCreate )
                {
//                     fh_mdcontext md = new f_mdcontext();
//                     fh_mdcontext child = md->setChild( "ea", "" );
//                     if( !ExplicitPluginName.empty() )
//                         child->setChild( "explicit-plugin-name", ExplicitPluginName );
//                     child->setChild( "name", eaname );
//                     child->setChild( "value", s );

//                    cerr << "create ea..." << eaname << endl;
                    static fh_mdcontext md = new f_mdcontext();
                    static fh_mdcontext child = md->setChild( "ea", "" );
                    md->AddRef();
                    child->AddRef();
                    md->AddRef();
                    child->AddRef();
                    child->setChild( "explicit-plugin-name", ExplicitPluginName );
                    child->setChild( "name", eaname );
                    child->setChild( "value", s );
                    
                    dst->createSubContext( "", md );
                }
                setStrAttr( dst, eaname, s, true, true );
            }
        }

//     void FerrisCopy::ResetUserNumberEA()
//         {
//                 string eaname = "user-owner-number";
//                 fh_stringstream ss;
//                 ss << getuid();
//                 setStrAttr( dst, eaname, tostr(ss) );
//         }

//     void FerrisCopy::ResetGroupNumberEA()
//         {
//             string eaname = "group-owner-number";
//             fh_stringstream ss;
//             ss << getgid();
//             setStrAttr( dst, eaname, tostr(ss) );
//         }
    
    /**
     * Note that mtime and atime should be last and in that order.
     */
    void
    FerrisCopy::PreserveEA()
        {
            /* cant do anything for preserving the EA of EA */
            if( !isBound(dst) && DstAttr.length() || SrcAttr.length() )
                return;

            if( src && dst )
                LG_COPY_D << "FerrisCopy::PreserveEA() src:" << src->getURL()
                          << " dst:" << dst->getURL()
                          << endl;

            if( src && dst )
            {
                //
                // Give the destination context a chance to perform specific
                // extra tasks knowing what the source is.
                // eg. webphoto classes might want to update online tags from
                //     the source image.
                //
                LG_WEBPHOTO_D << "Setting ferris-post-copy-action for dst:" << dst->getURL()
                              << " the source is:" << src->getURL()
                              << endl;
                setStrAttr( dst, "ferris-post-copy-action",
                            src->getURL(), false, false );
            }
            
            /*
             * We try to change the user even if we are not root to allow
             * for future capabilities to be added to the proc for CHOWN
             */
            if( PreserveOwner )
            {
                string eaname = "user-owner-number";
                string s      = getEA( src, eaname, "-1" );
                uid_t u = toType<uid_t>( s );
                if( u != -1 )
                {
                    bool canChange = Shell::canChangeFileToUser( u );
                    if( System::gotRoot() || canChange )
                    {
                        setStrAttr( dst, eaname, s );
                    }
//                     else if( !canChange )
//                     {
//                         ResetUserNumberEA();
//                     }
                }
            }

            LG_WEBPHOTO_D << "TEST1" << endl;
            
            if( PreserveGroup )
            {
                string eaname = "group-owner-number";
                string s      = getEA( src, eaname, "-1" );
                gid_t g = toType<gid_t>( s );
                if( g != -1 )
                {
                    bool canChange = Shell::canChangeFileToGroup( g );
                    
                    LG_COPY_D << "Shell::canChangeFileToGroup( g ) g:" << g
                              << " ret:" << Shell::canChangeFileToGroup( g )
                              << endl;
                    
                    if( System::gotRoot() || canChange )
                    {
                        setStrAttr( dst, eaname, s );
                    }
//                     else if( !canChange )
//                     {
//                         ResetGroupNumberEA();
//                     }
                }
            }

            if( m_PreserveFerrisTypeEA )
            {
                PreserveEA( "ferris-type",1 );
            }
            
            if( !PreserveAttributeList.empty() )
            {
                typedef PreserveAttributeList_t::iterator ITER;
                for( ITER iter = PreserveAttributeList.begin();
                     iter != PreserveAttributeList.end(); ++iter )
                {
                    cerr << "attempt to preserve:" << *iter << endl;
                    PreserveEA( *iter, 1 );
                }
            }

            LG_WEBPHOTO_D << "TEST2" << endl;
            
            if( m_PreserveRDFEA )
            {
                LG_COPY_D << "rdf-ea-names:" << getEA( src, "rdf-ea-names", "" ) << endl;
                stringlist_t sl = Util::parseCommaSeperatedList( getEA( src, "rdf-ea-names", "" ) );
                stringlist_t::const_iterator end = sl.end();
                LG_COPY_D << "rdf preserve list sz:" << sl.size() << endl;

                for( stringlist_t::const_iterator si = sl.begin(); si!=end; ++si )
                {
                    PreserveEA( *si, 1, "rdf" );
                }
            }

            LG_COPY_D << "m_PreserveRecommendedEA:" << m_PreserveRecommendedEA << endl;
            if( m_PreserveRecommendedEA )
            {
                string rea = getStrAttr( src, "recommended-ea", "" );
                LG_COPY_D << "m_PreserveRecommendedEA:" << m_PreserveRecommendedEA
                          << " rea:" << rea << endl;
                stringlist_t sl = Util::parseCommaSeperatedList( getEA( src, "rdf-ea-names", "" ) );
                stringlist_t::const_iterator end = sl.end();
                for( stringlist_t::const_iterator si = sl.begin(); si!=end; ++si )
                {
                    PreserveEA( *si, 1 );
                }
            }
            LG_WEBPHOTO_D << "TEST5" << endl;
            
            // handle the case that the old file is on a local disk which supports
            // kernel-ea and the new file is on an NFS share which doesn't by
            // downgrading the EA to live in RDF for the new file.
            if( DstAttr.empty() )
            {
                string xfseanames = getEA( src, "xfs-ea-names", "" );
                LG_COPY_D << "xfs-ea-names:" << xfseanames << endl;
                stringlist_t sl = Util::parseCommaSeperatedList( xfseanames );
                LG_COPY_D << "xfs preserve list sz:" << sl.size() << endl;

                //
                // If the transfer is
                // local-disk -> local-disk
                // then the kernel itself will have preserved the EA for us.
                // As such a test for the first xfs-ea will tell is that they are already
                // copied.
                //
                // If the transfer is NFS -> local-disk
                // then we will have already handled RDF EA above and the remote
                // file will not have any kernel-ea attached.
                //
                // If the transfer is local-disk -> NFS then the xfs-ea will not
                // have been preserved and this test will find that the first xfs-ea
                // is indeed not already existing for the destination.
                //
                if( !sl.empty() )
                {
                    const std::string& ea = sl.front();
                    if( !dst->isAttributeBound( ea ) )
                    {
                        //
                        // The xfs-ea are not already preserved by the kernel
                        // lets do it ourselves.
                        //
                        stringlist_t::const_iterator end = sl.end();
                        for( stringlist_t::const_iterator si = sl.begin(); si!=end; ++si )
                        {
                            PreserveEA( *si, 1 );
                        }
                    }
                }
            }
            
            LG_WEBPHOTO_D << "TEST6" << endl;


            // try to preserve the SELinux security context
            if( m_CloneSELinuxContext )
            {
                string ea = getStrAttr( src, "dontfollow-selinux-context", "" );
                LG_COPY_D << "m_CloneSELinuxContext:" << m_CloneSELinuxContext
                          << " setting to ea:" << ea << endl;
                
                if( !ea.empty() )
                {
                    setStrAttr( dst, "dontfollow-selinux-context",
                                ea, true, true );
                }
            }

            if( m_AttributeUpdaterInUse )
            {
                m_AttributeUpdater( this, src, dst,
                                    getSourceDescription(),
                                    getDestinationDescription() );
            }

            LG_WEBPHOTO_D << "TEST7" << endl;
            
            
            /* this is mode_t preserve */
            LG_COPY_D << "PreserveObjectMode:" << PreserveObjectMode << endl;
            if( PreserveObjectMode )
            {
                LG_COPY_D << "src-is-dir:" << isTrue( getStrAttr( src, "is-dir", "0" ) ) << endl;

                if( !isTrue( getStrAttr( src, "is-dir", "0" ) ) )
                    PreserveEA( "mode" );
            }
            LG_WEBPHOTO_D << "TEST8" << endl;
            
            if( PreserveMTime )      PreserveEA( "mtime" );
            if( PreserveATime )      PreserveEA( "atime" );
        }


    /**
     * Depending on the user settings try to make holes in the output file too.
     */
    fh_ostream
    FerrisCopy::WrapForHoles( fh_context src, fh_ostream ss )
        {
            switch( HoleMode )
            {
            case HOLEMODE_NEVER: return ss;
            case HOLEMODE_HEURISTIC:
                if( !toint(getEA( src, "has-holes", "0" )))
                {
                    return ss;
                }
                /* fall through */
            case HOLEMODE_ALWAYS:
                return Factory::MakeHoleyOStream(
                    ss, toint(getEA( src, "block-size", "4096" )));
            }
        }

    string
    FerrisCopy::getSourceDescription()
        {
            if( SrcAttr.length() )
            {
                fh_stringstream ss;
                ss << src->getURL() << "@" << SrcAttr << endl;
                return tostr(ss);
            }
            return src->getURL();
        }

    string
    FerrisCopy::getDestinationDescription( fh_context c )
        {
            if( !isBound(c) )
            {
                c = dst;
            }
            
            if( DstAttr.length() )
            {
                fh_stringstream ss;
                ss << c->getURL() << "@" << DstAttr << endl;
                return tostr(ss);
            }
            return c->getURL();
        }


    void
    FerrisCopy::perform()
    {
        copy();
    }
    
    void
    FerrisCopy::copy()
        {
//            TotalBytesCopied = 0;
//            TotalBytesToCopy = 0;
            
            dstURL = dstURL_setByUser;

            LG_COPY_D << "srcURL:" << srcURL << endl;
            LG_COPY_D << "dstURL:" << dstURL << endl;

                
            src = Resolve( srcURL );
            LG_COPY_D << "dstURL:" << dstURL << endl;

//             /*
//              * Should we work out the statistics for the src dir?
//              */
//             if( m_PrecacheSourceSize )
//             {
//                 LG_COPY_D << "Precaching the size and other metadata for the src:" << src->getURL() << endl;
//                 Ferrisls ls;
//                 fh_display_aggdata d = createDisplayAggregateData( &ls );
                
//                 if( isTrue( getStrAttr( src, "is-dir", "0" )))
//                     d->ShowAttributes( src );
//                 ls.setURL( src->getURL() );
//                 ls();
//                 const Ferrisls_aggregate_t& data = d->getData( AGGDATA_RECURSIVE );

//                 LG_COPY_D << "total bytes in source:" << Util::convertByteString(data.size) << endl;
//                 TotalBytesToCopy = data.sizeFilesOnly;
//             }
            
            /* If the destination exists and is a dir, then we want to copy
             * into that dir. */
            try
            {
                fh_context c = Resolve( dstURL );

                string isDirEAName = "is-dir-try-automounting";
                if( DoNotTryToOverMountDst )
                    isDirEAName = "is-dir";
                    
                if( DstIsDirectory || toType<int>(getEA( c, isDirEAName, "0" )))
                {
                    LG_COPY_D << "concluded DstIsDirectory. "
                              << " isDirEAName:" << isDirEAName
                              << " dstURL:" << dstURL
                              << " srcURL:" << srcURL
                              << " src:" << src->getURL()
                              << " dirname:" << src->getDirName()
                              << endl;
                    dstURL = src->appendToPath( dstURL, src->getDirName() );
                }
            }
            catch( exception& e )
            {
                LG_COPY_D << "cought 1:" << e.what() << endl;
            }

            LG_COPY_D << "Check onefs:" << OneFileSystem << endl;
            if( OneFileSystem )
            {
                dev_t d = toType<dev_t>(getEA( src, "device", "0" ));
                CurrentFileSystemDev = d;
            }


            LG_COPY_D << "CopySrcWithParents:" << CopySrcWithParents << endl;
            /*
             * Monster the destination for --parents option.
             */
            if( CopySrcWithParents )
            {
                if( DstAttr.length() )
                {
                    dstparent = Resolve( dstURL );
                    dstparent = Shell::CreateDir( dstparent, srcURL, 1 );
                }
                else
                {
                    dstparent = Resolve( dstURL );
                    dstparent = Shell::CreateDir( dstparent, srcURL, 1 );
                    dstparent = dstparent->getParent();
                }
                dstURL = dstparent->appendToPath( dstURL, srcURL );
            }

            /* update this for link following code */
            srcURL_Root = srcURL;
                
            LG_COPY_D << "Recurse:" << Recurse << endl;
            /*
             *
             * Start the copy action
             *
             */
            if( Recurse )
            {
                bool shouldReallyRecurse = toint(getEA( src, "is-dir", "0" ));

                LG_COPY_D << "cp-prep1 Recurse:" << Recurse
                          << " SrcIsDirectory:" << SrcIsDirectory
                          << " shouldReallyRecurse:" << shouldReallyRecurse
                          << endl;
                
                if( SrcIsDirectory )
                {
                    try
                    {
                        src->read();
                        shouldReallyRecurse = true;
                    }
                    catch( ... )
                    {}
                }

                LG_COPY_D << "cp-prep2 Recurse:" << Recurse
                          << " SrcIsDirectory:" << SrcIsDirectory
                          << " DontFollowLinks:" << DontFollowLinks
                          << " shouldReallyRecurse:" << shouldReallyRecurse
                          << endl;
                
                /*
                 * The other posibility is if we are starting on a link an it
                 * is ok to follow cmd line softlinks.
                 */
                if( !DontFollowLinks && !shouldReallyRecurse )
                {
                    shouldReallyRecurse |= toint(getEA( src, "dontfollow-is-link", "0" ));
                }
                    
                if( shouldReallyRecurse )
                {
                    LG_COPY_D << "cp-prep3 Recurse:" << Recurse
                              << " SrcIsDirectory:" << SrcIsDirectory
                              << " DontFollowLinks:" << DontFollowLinks
                              << " shouldReallyRecurse:" << shouldReallyRecurse
                              << " srcURL:" << srcURL
                              << endl;
                    
                    ls.setDisplay( this );
                    ls.setURL( srcURL );
                    ls.setRecursiveList( true );
                    ls.setDontFollowLinks( DontFollowLinks );
                    ls();
                    return;
                }
                    
                /* Fall through for copying a file/special object */
            }

            /* Normal single obj -> single object copy */
            priv_setSrcContext( src );
            priv_setDstContext( dstURL );

            LG_COPY_D << "About to perform_copy()"
                      << " src:" << src->getURL() << endl
                      << " dstURL:" << dstURL << endl;
            perform_copy();
            LG_COPY_D << "perform_copy() complete!"
                      << " src:" << src->getURL() << endl
                      << " dstURL:" << dstURL << endl;
        }

    std::streamsize
    FerrisCopy::getTotalBytesCopied() 
    {
        return TotalBytesCopied;
    }
    std::streamsize
    FerrisCopy::getTotalBytesToCopy()
    {
        return TotalBytesToCopy;
    }
    gdouble
    FerrisCopy::getTotalPercentageOfBytesCopied()
    {
        return getTotalBytesCopied() * 1.0/ getTotalBytesToCopy();
    }
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    ContextPopTableCollector::ContextPopTableCollector()
    {
        reset();
    }
    

    void ContextPopTableCollector::poptCallback(poptContext con,
                                                enum poptCallbackReason reason,
                                                const struct poptOption * opt,
                                                const char * arg,
                                                const void * data)
    {
        const string key = opt->longName;
        LG_COPY_D << "poptCallback() key:" << key << " -a" << ArchiveMode << endl;
    }

    void ContextPopTableCollector::reset()
    {
        DstNameCSTR              = 0;
        SrcAttr                  = "";
        DstAttr                  = "";
        BackupSuffix             = 0;
        PerformBackupsWithMode   = 0;
        HoleCreation             = 0;
        PreserveAttributeList    = 0;
        ExplicitSELinuxType      = 0;
        ExplicitSELinuxContext   = 0;
        CloneSELinuxContext    = 0;
        PrecacheSourceSize     = 0;
        DontPrecacheSourceSize = 0;
        PreserveRecommendedEA  = 0;
        DontPreserveRDFEA      = 0;
        DontPreserveFerrisTypeEA = 0;
        Force                  = 0;
        ForceRemovePremptively = 0;
        Interactive            = 0;
        CopySrcWithParents     = 0;
        StripTrailingSlashes   = 0;
        UpdateMode             = 0;
        Verbose                = 0;
        ShowVersion            = 0;
        OneFileSystem          = 0;
        PerformBackups         = 0;
        PreserveFileAttrs      = 0;
        MakeSoftLinksForNonDirs= 0;
        MakeSoftLinksForNonDirsAbsolute= 0;
        MakeHardLinksForNonDirs= 0;
        FollowOnlyForSrcUrl    = 0;
        DontFollowLinks        = 0;
        FollowLinks            = 0;
        Recurse                = 0;
        Sloth                  = 0;
        AutoClose              = 0;
        RecurseAndFlatten      = 0;
        ArchiveMode            = 0;
        ShowMeter              = 0;
        DisableCopyIntoSelfTests= 0;
        DstIsDirectory         = 0;
        SrcIsDirectory         = 0;
        DoNotTryToOverMountDst = 0;
        InputInMemoryMappedMode = 0;
        AutoInputInMemoryMappedModeSize_CSTR = 0;
        OutputInMemoryMappedMode = 0;
        AutoOutputInMemoryMappedModeSize_CSTR = 0;
        useSendfileIfPossible = 0;
        SendfileChunkSize = 0;

        FSyncAfterEachFile = 0;
        DontFSyncAfterEachFile = 0;
        PreallocateWith_fallocate = 0;
        PreallocateWith_ftruncate = 0;
        
//         OutputInDirectMode     = 0;
//         AutoOutputInDirectModeSize_CSTR = 0;
    }
    
    
    void
    ContextPopTableCollector::ArgProcessingDone( poptContext optCon )
    {
        if( ShowVersion )
        {
            cout << "ferriscp version: $Id: FerrisCopy.cpp,v 1.27 2011/04/27 21:31:10 ben Exp $\n"
                 << "ferris   version: " << VERSION << nl
                 << "Written by Ben Martin, aka monkeyiq" << nl
                 << nl
                 << "Copyright (C) 2001 Ben Martin" << nl
                 << "This is free software; see the source for copying conditions.  There is NO\n"
                 << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                 << endl;
            exit(0);
        }
    
        if( ArchiveMode )
        {
            FollowOnlyForSrcUrl = 0;
            FollowLinks = 0;
            DontFollowLinks = 1;
            PreserveFileAttrs = 1;
            Recurse = 1;
        }
                    
        if( Force || ForceRemovePremptively )
        {
            Interactive = 0;
        }
    
        if( RecurseAndFlatten )
        {
            Recurse = 1;
        }
    
        cp->setForceOverWrite( Force );
        cp->setForceRemovePremptively( ForceRemovePremptively );
        cp->setInteractive( Interactive );
        cp->setUpdateMode( UpdateMode );
        cp->setVerbose( Verbose );
        cp->setStripTrailingSlashes( StripTrailingSlashes );
        cp->setCopySrcWithParents( CopySrcWithParents );
            
        cp->setPerformBackups( "none" );
        if( PerformBackups )
            cp->setPerformBackups( "simple" );
        if( PerformBackupsWithMode )
            cp->setPerformBackups( PerformBackupsWithMode );
                    
        if( BackupSuffix ) cp->setBackupSuffix( BackupSuffix );
        if( DstAttr )      cp->setDstAttr( DstAttr );
        if( SrcAttr )      cp->setSrcAttr( SrcAttr );
        if( ExplicitSELinuxType )    cp->setExplicitSELinuxType( ExplicitSELinuxType );
        if( ExplicitSELinuxContext ) cp->setExplicitSELinuxContext( ExplicitSELinuxContext );
        if( CloneSELinuxContext )    cp->setCloneSELinuxContext( CloneSELinuxContext );
        {
            bool p = isTrue( getEDBString( FDB_GENERAL, CFG_PRECALCULATE_FOR_COPY_K, CFG_PRECALCULATE_FOR_COPY_DEFAULT ) );
            if( DontPrecacheSourceSize )
                p = false;
            if( PrecacheSourceSize )
                p = true;
            cp->setPrecacheSourceSize( p );
        }
        
        if( PreserveRecommendedEA )  cp->setPreserveRecommendedEA( PreserveRecommendedEA );
        if( DontPreserveRDFEA )      cp->setPreserveRDFEA( !DontPreserveRDFEA );
        if( DontPreserveFerrisTypeEA ) cp->setPreserveFerrisTypeEA( !DontPreserveFerrisTypeEA );
        
        if( PreserveFileAttrs )
        {
            cp->setPreserveMTime( true );
            cp->setPreserveATime( true );
            cp->setPreserveOwner( true );
            cp->setPreserveGroup( true );
            cp->setPreserveObjectMode( true );
        }

        if( (FollowLinks && ( FollowOnlyForSrcUrl || DontFollowLinks ))
            || ( FollowOnlyForSrcUrl && DontFollowLinks )
            )
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }
        if( !FollowLinks )
        {
            if( DontFollowLinks     ) cp->setDontFollowLinks( DontFollowLinks );
            if( FollowOnlyForSrcUrl ) cp->setFollowOnlyForSrcUrl( FollowOnlyForSrcUrl );
        }
        
        if( MakeSoftLinksForNonDirs || MakeSoftLinksForNonDirsAbsolute )
        {
            cp->setMakeSoftLinksForNonDirs( MakeSoftLinksForNonDirs );
            cp->setMakeSoftLinksForNonDirsAbsolute( MakeSoftLinksForNonDirsAbsolute );

            if( MakeSoftLinksForNonDirs && Recurse )
            {
                LG_COPY_D << "Can not Recurse and make relative links,"
                          << " use --symbolic-link-absolute instead" << nl << endl;
                poptPrintHelp(optCon, stderr, 0);
                exit(1);
            }
        }
        if( MakeHardLinksForNonDirs )
        {
            cp->setMakeHardLinksForNonDirs( MakeHardLinksForNonDirs );
        }

        FerrisCopy::HoleMode_t m = FerrisCopy::HOLEMODE_NEVER;
        if( HoleCreation == "auto" )        m = FerrisCopy::HOLEMODE_HEURISTIC;
        else if( HoleCreation == "always" ) m = FerrisCopy::HOLEMODE_ALWAYS;
        cp->setHoleMode( m );

        if( PreserveAttributeList )
        {
            cp->setPreserveAttributeList( PreserveAttributeList );
        }
        
        cp->setRecurse( Recurse );
        cp->setFlattenSpecialToFile( RecurseAndFlatten );

        if( FSyncAfterEachFile )
            cp->setFSyncAfterEachFile( FSyncAfterEachFile );
        if( DontFSyncAfterEachFile )
            cp->setFSyncAfterEachFile( !DontFSyncAfterEachFile );

        cp->setPreallocateWith_fallocate( PreallocateWith_fallocate );
        cp->setPreallocateWith_ftruncate( PreallocateWith_ftruncate );
        
        cp->setCopyIntoSelfTests( !DisableCopyIntoSelfTests );
        cp->setDstIsDirectory( DstIsDirectory );
        cp->setSrcIsDirectory( SrcIsDirectory );
        cp->setDoNotTryToOverMountDst( DoNotTryToOverMountDst );
        cp->setInputInMemoryMappedMode( InputInMemoryMappedMode );
        if( AutoInputInMemoryMappedModeSize_CSTR )
        {
            guint64 v = Util::convertByteString( AutoInputInMemoryMappedModeSize_CSTR );
            cp->setAutoInputInMemoryMappedModeSize( v );
        }

        cp->setOutputInMemoryMappedMode( OutputInMemoryMappedMode );
        if( AutoOutputInMemoryMappedModeSize_CSTR )
        {
            guint64 v = Util::convertByteString( AutoOutputInMemoryMappedModeSize_CSTR );
            cp->setAutoOutputInMemoryMappedModeSize( v );
        }
        if( useSendfileIfPossible )
            cp->setUseSendfileIfPossible( useSendfileIfPossible );
        if( SendfileChunkSize )
            cp->setSendfileChunkSize( SendfileChunkSize );
        
//         cp->setOutputInDirectMode( OutputInDirectMode );
//         if( AutoOutputInDirectModeSize_CSTR )
//         {
//             guint64 v = Util::convertByteString( AutoOutputInDirectModeSize_CSTR );
//             cp->setAutoOutputInDirectModeSize( v );
//         }
        
        cp->setOneFileSystem( OneFileSystem );
        cp->setSloth( Sloth );
        cp->setAutoClose( AutoClose );
    }
            
    struct ::poptOption*
    ContextPopTableCollector::getTable( fh_cp _cp )
    {
        cp = _cp;
        int extraTableLines = 5;
        int tablesize = 90;

        tablesize = tablesize + extraTableLines;
        allocTable( tablesize );
                    
        int i=0;
        setToCallbackEntry( &table[i] );
        ++i;

        setEntry(
            &table[i++], "src-attr", 0, POPT_ARG_STRING, &SrcAttr,
            "Take an attribute off the source path rather than the context itself",
            "" );
        setEntry(
            &table[i++], "dst-attr", 0, POPT_ARG_STRING, &DstAttr,
            "Write to an attribute in the destination context rather than the context itself",
            "" );
        setEntry(
            &table[i++], "dst-attr-no-create", 0, POPT_ARG_STRING, &DstAttrNoCreate,
            "It is an error if the chosen dst-attr does not already exist",
            "" );

        setEntry(
            &table[i++], "archive", 'a', POPT_ARG_NONE, &ArchiveMode, 
            "same as -dpR", "" );
        setEntry(
            &table[i++], "", 'b', POPT_ARG_NONE, &PerformBackups, 
            "like --backup but does not accept an argument", "" );
        setEntry(
            &table[i++], "backup", 0, POPT_ARG_STRING, &PerformBackupsWithMode, 
            "make a backup of each existing destination file", "CONTROL" );
        setEntry(
            &table[i++], "disable-copyintoself-tests", 0, POPT_ARG_NONE, &DisableCopyIntoSelfTests,
            "gain a little time by assuming the user is always right", "" );
        setEntry(
            &table[i++], "dereference", 'L', POPT_ARG_NONE, &FollowLinks,
            "always follow symbolic links", "" );
        setEntry(
            &table[i++], "no-dereference", 'd', POPT_ARG_NONE, &DontFollowLinks,
            "Copy symbolic links as symbolic links rather than copying the"
            "files that they point to, and preserve hard links between source"
            "files in the copies", "" );
        setEntry(
            &table[i++], "dst-is-dir", 0, POPT_ARG_NONE, &DstIsDirectory,
            "force treating destination as a directory (copy into db4/xml file)", "" );
        setEntry(
            &table[i++], "src-is-dir", 0, POPT_ARG_NONE, &SrcIsDirectory,
            "force treating source as a directory (copy from db4/xml file)", "" );
        setEntry(
            &table[i++], "dont-try-to-mount-dst", 0, POPT_ARG_NONE, &DoNotTryToOverMountDst,
            "Do not attempt to mount dst as a composite file", "0" );

        setEntry(
            &table[i++], "input-in-mmap-mode", 0, POPT_ARG_NONE, &InputInMemoryMappedMode,
            "memory map the input file for reading. Using MADV_SEQUENTIAL madvise(2)", "" );

        setEntry(
            &table[i++], "auto-input-in-mmap-mode", 0,
            POPT_ARG_STRING, &AutoInputInMemoryMappedModeSize_CSTR,
            "input-in-mmap-mode input if file size is greater than given human readable value", "" );

        setEntry(
            &table[i++], "output-in-mmap-mode", 0, POPT_ARG_NONE, &OutputInMemoryMappedMode,
            "memory map the output file for writing. Using MADV_SEQUENTIAL madvise(2)", "" );

        setEntry(
            &table[i++], "auto-output-in-mmap-mode", 0,
            POPT_ARG_STRING, &AutoOutputInMemoryMappedModeSize_CSTR,
            "output-in-mmap-mode output if file size of input is greater than given human readable value", "" );

//         setEntry(
//             &table[i++], "use-sendfile-if-possible", 0,
//             POPT_ARG_NONE, &useSendfileIfPossible,
//             "if sendfile(2) can be used for the copy then do it", "0" );
//         setEntry(
//             &table[i++], "sendfile-chunk-size", 0,
//             POPT_ARG_INT, &SendfileChunkSize,
//             "override size of each chunk that is copied at once using sendfile(2)", "0" );
        
//         setEntry(
//             &table[i++], "output-in-direct-mode", 0, POPT_ARG_NONE, &OutputInDirectMode,
//             "use O_DIRECT output to try to avoid any caching of data", "" );

//         setEntry(
//             &table[i++], "auto-output-in-direct-mode", 0,
//             POPT_ARG_STRING, &AutoOutputInDirectModeSize_CSTR,
//             "use O_DIRECT output if file size is greater than given human readable value", "" );
        
        setEntry(
            &table[i++], "force", 'f', POPT_ARG_NONE, &Force,
            "if an existing destination file cannot be opened, remove it and try again", "" );

        setEntry(
            &table[i++], "follow", 'H', POPT_ARG_NONE, &FollowOnlyForSrcUrl,
            "If a command line argument specifies a symbolic link, then copy the"
            "file it points to rather than the symbolic link itself.  However,"
            "copy (preserving its nature) any symbolic link that is encountered"
            "via recursive traversal", "" );
        setEntry(
            &table[i++], "interactive", 'i', POPT_ARG_NONE, &Interactive, 
            "prompt before overwrite", "");
        setEntry(
            &table[i++], "link", 'l', POPT_ARG_NONE, &MakeHardLinksForNonDirs, 
            "Make hard links instead of copies of non-directories", "" );
        setEntry(
            &table[i++], "preserve", 'p', POPT_ARG_NONE, &PreserveFileAttrs, 
            "preserve file attributes if possible", "" );
        setEntry(
            &table[i++], "preserve-list", 0, POPT_ARG_STRING, &PreserveAttributeList, 
            "Also preserve these attributes if they exist in the src", "attr1,attr2,..." );
        setEntry(
            &table[i++], "preserve-recommended-ea", 0, POPT_ARG_NONE, &PreserveRecommendedEA, 
            "Also preserve the attributes listed in recommended-ea for the source", "" );
        setEntry(
            &table[i++], "dont-preserve-rdf-ea", 0, POPT_ARG_NONE, &DontPreserveRDFEA, 
            "Skip attempts to copy/link RDF EA", "" );
        setEntry(
            &table[i++], "dont-preserve-ferris-filetype", 0, POPT_ARG_NONE, &DontPreserveFerrisTypeEA, 
            "Skip attempt to preserve ferris-type EA", "" );
        setEntry(
            &table[i++], "parents", 0, POPT_ARG_NONE, &CopySrcWithParents, 
            "Form the name of each destination file by appending to the target"
            " directory a slash and the specified name of the source file DIRECTORY", "" );
        setEntry(
            &table[i++], "recursive", 'R', POPT_ARG_NONE, &Recurse, 
            "copy the contents of directories recursively", "" );
        setEntry(
            &table[i++], "", 'r', POPT_ARG_NONE, &RecurseAndFlatten, 
            "copy the contents of directories recursively, flatten non-dir to files", "" );
        setEntry(
            &table[i++], "remove-destination", 0, POPT_ARG_NONE, &ForceRemovePremptively, 
            "Remove each existing destination file before attempting to "
            "open it (contrast with -f)", "" );
        setEntry(
            &table[i++], "sloth", 0, POPT_ARG_NONE, &Sloth, 
            "keep the main window closed until it is needed", "" );
        setEntry(
            &table[i++], "auto-close", '0', POPT_ARG_NONE, &AutoClose, 
            "If there is no user interaction or objects skipped then close client automatically", "" );
        setEntry(
            &table[i++], "sparse", 0, POPT_ARG_STRING, &HoleCreation, 
            "control creation of sparse files", "WHEN" );
        setEntry(
            &table[i++], "strip-trailing-slashes", '/', POPT_ARG_NONE, &StripTrailingSlashes,
            "remove any trailing slashes from each SOURCE argument", "" );
        setEntry(
            &table[i++], "suffix", 'S', POPT_ARG_STRING, &BackupSuffix, 
            "override the usual backup suffix", "SUFFIX" );
        setEntry(
            &table[i++], "symbolic-link", 's', POPT_ARG_NONE, &MakeSoftLinksForNonDirs, 
            "Make symbolic links instead of copies of non-directories", "" );
        setEntry(
            &table[i++], "symbolic-link-absolute", 0, POPT_ARG_NONE,
            &MakeSoftLinksForNonDirsAbsolute, 
            "Make absolute symbolic links instead of copies of non-directories", "" );
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
            &table[i++], "one-file-system", 'x', POPT_ARG_NONE, &OneFileSystem, 
            "stay on this file system", 0 );

        
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

        setEntry(
            &table[i++], "precache-copy-size", 0, POPT_ARG_NONE, &PrecacheSourceSize, 
            "workout how large the source is before copying", 0 );

        setEntry(
            &table[i++], "dont-precache-copy-size", 0, POPT_ARG_NONE, &DontPrecacheSourceSize, 
            "do not workout how large the source is before copying", 0 );


        setEntry(
            &table[i++], "preallocate-with-fallocate", 0, POPT_ARG_NONE, &PreallocateWith_fallocate,
            "preallocate byte range for destination files using fallocate", "" );

        setEntry(
            &table[i++], "preallocate-with-ftruncate", 0, POPT_ARG_NONE, &PreallocateWith_ftruncate,
            "preallocate byte range for destination files using ftruncate", "" );


        setEntry(
            &table[i++], "dont-fsync-after-each-file", 0, POPT_ARG_NONE, &DontFSyncAfterEachFile,
            "disable fsync call after each file is copied", "" );

        setEntry(
            &table[i++], "fsync-after-each-file", 0, POPT_ARG_NONE, &FSyncAfterEachFile,
            "enable fsync call after each file is copied", "" );
        

        
        clearEntry( &table[i] );
        return table;
    }
        
        
    namespace Priv
    {
        struct ::poptOption* getCopyPopTableCollector( fh_cp cp )
        {
            return cp->getPoptCollector()->getTable( cp );
        }
    };


};
