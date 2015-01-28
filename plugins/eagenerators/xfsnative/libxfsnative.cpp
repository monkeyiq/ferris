/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libxfsnative.cpp,v 1.5 2009/04/04 21:30:12 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Ferris/FerrisBoost.hh>

#include <asm/types.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <attr/xattr.h>
//#include <attr/attributes.h>

#include "config.h" // XFS_SUPER define
// #include <xfs/xfs_fs.h>
// #include <xfs/libxfs.h>
#include <sys/vfs.h>


using namespace std;
namespace Ferris
{
    const char* const USER_PREFIX = "user.";
//    static const string USER_PREFIX = "user.";

    string StripUserNameSpace( const string& s )
    {
        if( starts_with( s, USER_PREFIX ) )
        {
            return s.substr( strlen(USER_PREFIX) );
//            return s.substr( USER_PREFIX.length() );
        }
        return s;
    }

    string PrependUserNameSpace( const string& s )
    {
//         if( string::npos == s.find(".") )
//         {
            return USER_PREFIX + s;
//         }
//         return s;
    }
    
    

class FERRISEXP_DLLLOCAL EAGenerator_XFSNative : public MatchedEAGeneratorFactory
{
protected:

    virtual void Brew( const fh_context& a );

    void generateAttribute( const fh_context& a,
                            const std::string& rdn,
                            bool forceCreate = false );
    
public:

    EAGenerator_XFSNative();

    fh_attribute CreateAttr(
        const fh_context& a,
        const string& rdn,
        fh_context md = 0 )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            );

    virtual bool isDynamic()
        {
            return true;
        }
    virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    
    
    bool
    supportsCreateForContext( fh_context c )
    {
//        cerr << "XFS::supportsCreateForContext() c:" << c->getURL() << endl;
        
        if( !c->isParentBound() )
            return false;
        
//        cerr << "XFS::supportsCreateForContext(2) c:" << c->getURL() << endl;
        struct statfs fsbuf;
        string p = c->getParent()->getDirPath();
//         cerr << "XFS::supportsCreateForContext(3) c:" << c->getURL()
//              << " p:" << p
//              << endl;

//        string realpath = getStrAttr( c, "realpath", "" );
//         if( !realpath.empty() )
//             p = realpath;
        
//         cerr << "EAGenerator_XFSNative::supportsCreateForContext() p:" << p
// //             << " realpath:" << realpath
//              << endl;
        
        {
            static string regexstr = getEDBString( FDB_GENERAL,
                                                   CFG_ALLOWS_KERNEL_EA_REGEX,
                                                   CFG_ALLOWS_KERNEL_EA_REGEX_DEFAULT );
            static bool emp = regexstr.empty();
            if( !emp )
            {
                static const boost::regex r = toregex( regexstr );
                if( regex_match( c->getURL(), r, boost::match_any ) )
                {
                    return true;
                }
            }
        }
        

        if( !statfs( p.c_str(), &fsbuf ) )
        {
//             cerr << "EAGenerator_XFSNative::supportsCreateForContext() "
//                  << " p:" << p << " is XFS filesystem:"
//                  << (fsbuf.f_type == XFS_SUPER_MAGIC) << endl;
            return fsbuf.f_type == XFS_SUPER_MAGIC;
        }
//        cerr << "XFS::supportsCreateForContext(4) c:" << c->getURL() << endl;

        
//         // We want the statfs() for the filesystem that the file/link is on //
//         int fd = open( p.c_str(), O_LARGEFILE | O_RDONLY );
//         if( fd == -1 )
//         {
//             cerr << "EAGenerator_XFSNative::supportsCreateForContext() p:" << p
//                  << " cant open file" << endl;
//             return false;
//         }
        
//         int rc = fstatfs( fd, &fsbuf );
//         if( -1 == close( fd ))
//             cerr << "FD EEK! Cant close fd for path:" << p << endl;
        
//         if( !rc )
//         {
//             cerr << "EAGenerator_XFSNative::supportsCreateForContext() "
//                  << " p:" << p << " is XFS filesystem:"
//                  << (fsbuf.f_type == XFS_SUPER_MAGIC) << endl;
//             return fsbuf.f_type == XFS_SUPER_MAGIC;
//         }

//         cerr << "EAGenerator_XFSNative::supportsCreateForContext() p:" << p
//              << " statfs() failed" << endl;
        
        return false;
    }


    static stringlist_t& getEANames( Context* c, stringlist_t& ret );
    static fh_istream SL_getXFSEANames( Context* c, const std::string& rdn, EA_Atom* );
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_DLLLOCAL XFSByteArrayAttribute
    :
        public EA_Atom_ReadWrite
{
    bool isUserNamespace;

protected:

    ferris_ios::openmode getSupportedOpenModes()
        {
            return ios::in | ios::out | ios::trunc | ios::binary | ios::ate;
        }

    void createOnDiskAttribute( const fh_context& parent, const std::string& rdn )
        {
            LG_XFS_D << "Create native disk EA for c:" << parent->getURL()
                     << " attribute name:" << rdn
                     << endl;
            
            char dummy[10];
            int rc = lsetxattr ( parent->getDirPath().c_str(),
                                 fullEAName( rdn ).c_str(),
                                 dummy, 0,
                                 XATTR_CREATE );
            if( rc == -1 )
            {
                int eno = errno;

                string es = errnum_to_string( "", eno );
                
                fh_stringstream ss;
                ss << "Failed to create native disk EA for c:" << parent->getURL()
                   << " attribute name:" << rdn
                   << " full attribute name:" << fullEAName( rdn )
                   << " error:" << es << endl;
                cerr << tostr( ss ) << endl;
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
        }
    
    
public:

    virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;

            ssize_t szrc = getxattr ( c->getDirPath().c_str(),
                                       fullEAName( rdn ).c_str(),
//                                       rdn.c_str(),
                                       0, 0 );

            LG_XFS_D << "XFSEA::getStream() c:" << c->getURL() << " rdn:" << rdn << endl;
            
//             cerr << "reading attr rdn:" << rdn
//                  << " full name:" << fullEAName( rdn ).c_str()
//                  << " size:" << szrc
//                  << endl;
            if( szrc < 0 )
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "reading attribute";
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
            if( !szrc )
            {
//                cerr << "zero byte EA. returning." << endl;
                return ss;
            }
            
            int   attrvaluemax = szrc; //ATTR_MAX_VALUELEN;
            char* attrvalue = new char[ attrvaluemax ];

            if( !attrvalue )
            {
                Throw_FerrisOutOfMemory( "", 0 );
            }
            
            ssize_t rc = getxattr ( c->getDirPath().c_str(),
                                     fullEAName( rdn ).c_str(),
                                     attrvalue, attrvaluemax );

            if( rc < 0 )
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "reading attribute";
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
            
            
//             int rc = attr_get ( a->getParent()->getDirPath().c_str(),
//                                 a->getDirName().c_str(),
//                                 attrvalue, &attrvaluemax, ATTR_DONTFOLLOW);

            ss.write( attrvalue, attrvaluemax );
            delete [] attrvalue;

            LG_XFS_D << "read in XFS attribute:" << tostr(ss) << endl;
            return ss;
        }

    virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
        {
            LG_XFS_D << "XFSEA::setStream() c:" << c->getURL() << " rdn:" << rdn << endl;

            string path   = c->getDirPath();
            string eaname = fullEAName( rdn );

            string s = StreamToString(ss);

            LG_XFS_D << "XFSEA::setStream() c:" << c->getURL()
                     << " rdn:" << rdn
                     << " s:" << s
                     << endl;
            
//             copy( istreambuf_iterator<char>(ss), istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(mems));
//             cerr << "XFS setStream (2)" << endl;
            
//             string s = StreamToString(mems);
            LG_XFS_D << "XFSByteArrayAttribute::priv_updateFromIOStream() s:"<< s << endl;

//            cerr << "XFS Set stream rdn:" << rdn << " to:" << s << endl;
            
            int rc = lsetxattr ( path.c_str(), eaname.c_str(), s.c_str(), s.length(), 0 );
            if( rc == -1 )
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "setting EA";
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
            
            
//             int rc = attr_set( path.c_str(),
//                                rdn.c_str(),
//                                s.c_str(), s.length(),
//                                ATTR_DONTFOLLOW);
        }

    string fullEAName( string s )
        {
            if( isUserNamespace )
                return PrependUserNameSpace( s );
            return s;
        }
    
    
    
    XFSByteArrayAttribute(
        const fh_context& parent,
        const string& rdn,
        bool _isUserNamespace,
        bool forceCreate = false
        )
        :
        EA_Atom_ReadWrite( this, &XFSByteArrayAttribute::getStream,
                           this, &XFSByteArrayAttribute::getStream,
                           this, &XFSByteArrayAttribute::setStream ),
        isUserNamespace( _isUserNamespace )
        {
            if( forceCreate )
            {
                LG_XFS_D << " force create rdn:" << rdn << " isuser:" << isUserNamespace << endl;

                createOnDiskAttribute( parent, rdn );
// //                 char dummy[10];
// //                 int rc = lsetxattr ( parent->getDirPath().c_str(),
// //                                      fullEAName( rdn ).c_str(),
// //                                      dummy, 0,
// //                                      XATTR_CREATE );
// //                 if( rc == -1 )
// //                 {
// //                     int eno = errno;
// //                     fh_stringstream ss;
// //                     ss << "creating EA";
// //                     ThrowFromErrno( eno, tostr(ss), 0 );
// //                 }
            }
            
            
        }

    ~XFSByteArrayAttribute()
        {
        }

    static XFSByteArrayAttribute* Create( const fh_context& parent,
                                          const string& user_rdn,
                                          bool forceCreate = false )
        {
            string rdn = StripUserNameSpace( user_rdn );
            return new XFSByteArrayAttribute( parent, rdn, 
                                              rdn != user_rdn,
                                              forceCreate );
        }
};


/**
 * We allow the schema:eaname to be saved on disk as an XFS EA aswell.
 * This is a little tricky, if its not saved on disk then we return
 * the URL for FXD_BINARY_NATIVE_EA as the data in Stream requests.
 * if the user updates the stream with some data then we save it to
 * disk and return the disk data in future Stream requests.
 */
class FERRISEXP_DLLLOCAL XFSByteArrayAttributeSchema
    :
        public XFSByteArrayAttribute
{
    typedef XFSByteArrayAttributeSchema _Self;
    typedef XFSByteArrayAttribute       _Base;

public:

    virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            ssize_t szrc = getxattr ( c->getDirPath().c_str(),
                                      fullEAName( rdn ).c_str(),
                                      0, 0 );

            //
            // attribute exists on disk, we return that data.
            //
            if( szrc > 0 )
            {
                return _Base::getStream( c, rdn, atom );
            }
            
            // doesn't exist?
            int eno = errno;
            if( errno == ENOATTR )
            {
                // try for subtree schema
                if( c->isParentBound() )
                {
                    LG_XFS_D << "xfsschema::getStream() no ea:" << rdn << " url:" << c->getURL() << endl;
                    string subtreeurl = getStrAttr( c->getParent(), "subtree" + rdn, "" );
                    LG_XFS_D << "xfsschema::getStream() no ea:" << rdn
                             << " subtreeurl:" << subtreeurl
                             << endl;
                    
                    if( !subtreeurl.empty() )
                    {
                        fh_stringstream ss;
                        ss << subtreeurl;
                        return ss;
                    }
                }
                
                
                Factory::xsdtypemap_t tmap;
                Factory::makeBasicTypeMap( tmap );
                fh_context schema = tmap[ FXD_BINARY_NATIVE_EA ];
                
                fh_stringstream ss;
                ss << schema->getURL();
                return ss;
            }
            else
            {
                fh_stringstream ss;
                ss << "reading attribute:" << rdn;
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
        }
    virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
        {
            ssize_t szrc = getxattr ( c->getDirPath().c_str(),
                                      fullEAName( rdn ).c_str(),
                                      0, 0 );

            //
            // attribute exists on disk just pass it along to parent
            //
            if( szrc > 0 )
                _Base::setStream( c, rdn, atom, ss );
            else
            {
                // create attribute on disk and set its value
                createOnDiskAttribute( c->getParent(), rdn );
                _Base::setStream( c, rdn, atom, ss );
            }
        }
    
    XFSByteArrayAttributeSchema( const fh_context& parent,
                                 const string& rdn,
                                 bool _isUserNamespace,
                                 bool forceCreate = false
        )
        :
        _Base( parent, rdn, _isUserNamespace, forceCreate )
        {
            LG_XFS_D << "XFSByteArrayAttributeSchema() parent:" << parent->getURL()
                     << " rdn:" << rdn
                     << " isUser:" << _isUserNamespace
                     << " forceCreate:" << forceCreate
                     << endl;
        }

    static XFSByteArrayAttributeSchema* Create( const fh_context& parent,
                                                const string& user_rdn,
                                                bool forceCreate = false )
        {
            string rdn = StripUserNameSpace( user_rdn );
            return new XFSByteArrayAttributeSchema( parent, rdn, 
                                                    rdn != user_rdn,
                                                    forceCreate );
        }
    
};




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


EAGenerator_XFSNative::EAGenerator_XFSNative()
    :
    MatchedEAGeneratorFactory()
{
}

/**
 * expects rdn to have the user namespace prefix included.
 */
void
EAGenerator_XFSNative::generateAttribute( const fh_context& a,
                                          const std::string& rdn,
                                          bool forceCreate )
{
    LG_XFS_D << "EAGenerator_XFSNative::generateAttribute() rdn:" << rdn << endl;
    
    a->addAttribute( StripUserNameSpace(rdn),
                     (EA_Atom*)XFSByteArrayAttribute::Create( a, rdn, forceCreate ),
                     FXD_BINARY_NATIVE_EA
        );

    //
    // Attach schema
    //
    if( !starts_with( StripUserNameSpace(rdn), "schema:" ))
    {
        string schema_name      = "schema:" + StripUserNameSpace(rdn);
        string qual_schema_name = PrependUserNameSpace( schema_name );

        if( ! a->isAttributeBound( schema_name, false ) )
        {
            if( ! a->addAttribute(
                    schema_name,
                    (EA_Atom*)XFSByteArrayAttributeSchema::Create( a, qual_schema_name,
                                                                   forceCreate ),
                    XSD_SCHEMA ))
            {
                ostringstream ss;
                ss << "EAGenerator_XFSNative::CreateAttr() operation failed supported rdn:" << rdn;
                Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
            }
        }
    }
}


stringlist_t&
EAGenerator_XFSNative::getEANames( Context* c, stringlist_t& ret )
{
    int          listmultifactor = 20*1024;
    size_t       listsize        = 1;
    char*        list            = 0;
    ssize_t      attributeSize   = 0;

    while( true )
    {
        listsize *= listmultifactor;
        list      = static_cast<char*>(malloc( listsize ));
            
        if( !list )
        {
            Throw_FerrisOutOfMemory( "", 0 );
        }

        errno = 0;
        ssize_t rc = listxattr( c->getDirPath().c_str(), list, listsize );
        int eno = errno;

        if( rc == -1 && eno == ERANGE )
        {
            free( list );
            continue;
        }
        else if( rc == -1 && eno == ENOTSUP )
        {
            // not supported for this filesystem
            free( list );
            return ret;
        }
        else if( rc > 0 )
        {
            attributeSize = rc;
            break;
        }
        else if( rc == 0 )
        {
            // no xfs ea for this file
            free( list );
            return ret;
        }
        else
        {
            free( list );
            fh_stringstream ss;
            ss << "reading attribute list";
            ThrowFromErrno( eno, tostr(ss), 0 );
        }
    }

    char* p   = list;
    char* max = list + attributeSize;
    while( p < max )
    {
        string rdn = p;
        ret.push_back( rdn );

        /* Move along the string length plus terminating null */
        p+=strlen(p)+1;
    }
        
    free( list );

    return ret;
}


fh_istream
EAGenerator_XFSNative::SL_getXFSEANames( Context* c, const std::string& rdn, EA_Atom* )
{
    fh_stringstream ret;

    stringlist_t sl;
    getEANames( c, sl );

    stringlist_t tmp;
    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
        tmp.push_back( StripUserNameSpace( *si ) );

    ret << Util::createCommaSeperatedList( tmp );
    return ret;
}

void
EAGenerator_XFSNative::Brew( const fh_context& a )
{
    try
    {
        a->setHasDynamicAttributes( true );
        

        stringlist_t sl;
        getEANames( GetImpl(a), sl );
        for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
        {
            generateAttribute( a, *si );
        }

        /*
         * Add a new EA showing the names of all the XFS EA
         */
        a->addAttribute( "xfs-ea-names",
                         EAGenerator_XFSNative::SL_getXFSEANames,
                         FXD_EANAMES,
                         true );

//         typedef list<string> stringlist_t;
//         stringlist_t xfs_ea_names;
//         int          listmultifactor = 20*1024;
//         size_t       listsize        = 1;
//         char*        list            = 0;
//         ssize_t      attributeSize   = 0;
        
//         while( true )
//         {
//             listsize *= listmultifactor;
//             list      = static_cast<char*>(malloc( listsize ));
            
//             if( !list )
//             {
//                 Throw_FerrisOutOfMemory( "", 0 );
//             }

//             errno = 0;
//             ssize_t rc = listxattr( a->getDirPath().c_str(), list, listsize );
//             int eno = errno;

//             if( rc == -1 && eno == ERANGE )
//             {
//                 free( list );
//                 continue;
//             }
//             else if( rc > 0 )
//             {
//                 attributeSize = rc;
//                 break;
//             }
//             else
//             {
//                 free( list );
//                 fh_stringstream ss;
//                 ss << "reading attribute list";
//                 ThrowFromErrno( eno, tostr(ss), 0 );
//             }
//         }

//         char* p   = list;
//         char* max = list + attributeSize;
//         while( p < max )
//         {
//             string rdn = p;
// //             cerr << " count:" << attributeSize
// //                  << " rdn:" << rdn << endl;
// //             cerr << "Found attribute rdn:" << rdn
// //                  << " for c:" << a->getURL()
// //                  << " c addr:" << (void*)GetImpl(a)
// //                  << endl;

//             try
//             {
//                 generateAttribute( a, rdn );
//             }
//             catch( exception& e )
//             {
//                 free( list );
//                 throw;
//             }
            
//             xfs_ea_names.push_back( StripUserNameSpace(rdn) );

//             /* Move along the string length plus terminating null */
//             p+=strlen(p)+1;
//         }
        
//         free( list );


//         /*
//          * Add a new EA showing the names of all the XFS EA
//          */
//         a->addAttribute( "xfs-ea-names",
//                          Util::createCommaSeperatedList( xfs_ea_names ),
//                          FXD_EANAMES,
//                          true );
        
    }
    catch( exception& e )
    {
        LG_XFS_ER << "Failed to load xfs EA, error:" << e.what() << endl;
    }
}

bool
EAGenerator_XFSNative::tryBrew( const fh_context& ctx, const std::string& eaname )
{
    LG_XFS_D << "EAGenerator_XFSNative::tryBrew() "
             << " url:" << ctx->getURL()
             << " eaname:" << eaname
             << endl;
    
    bool ret = false;

    ssize_t szrc = getxattr ( ctx->getDirPath().c_str(),
                               PrependUserNameSpace( eaname ).c_str(),
                               0, 0 );

    if( szrc == -1 )
    {
        LG_XFS_D << "EAGenerator_XFSNative::tryBrew() failed to find EA:" << eaname
                 << " url:" << ctx->getURL()
                 << endl;
//         cerr << "EAGenerator_XFSNative::tryBrew() failed to find EA:" << eaname
//              << " url:" << ctx->getURL()
//              << endl;
//         BackTrace();
    }
    if( szrc != -1 )
    {
        /* we have an attribute */
        ret = true;
        generateAttribute( ctx, PrependUserNameSpace( eaname ) );
    }
    return ret;
}



fh_attribute
EAGenerator_XFSNative::CreateAttr(
    const fh_context& a,
    const string& urdn,
    fh_context md )
    throw(
        FerrisCreateAttributeFailed,
        FerrisCreateAttributeNotSupported
        )
{
    try
    {
        string rdn = urdn;

        LG_XFS_D << "EAGenerator_XFSNative::CreateAttr() rdn:" << rdn
                 << " USER_PREFIX:" << USER_PREFIX
                 << " urdn:" << urdn
                 << endl;
//         cerr  << "EAGenerator_XFSNative::CreateAttr() a:" << a->getURL()
//               << " rdn:" << rdn
//               << " USER_PREFIX:" << USER_PREFIX
//               << " urdn:" << urdn
//               << endl;
        
//      if( string::npos == rdn.find(".") )
        if( !starts_with( rdn, USER_PREFIX ) )
        {
            rdn  = USER_PREFIX;
            rdn += urdn;
        }

        LG_XFS_D << "EAGenerator_XFSNative::CreateAttr() rdn:" << rdn
                 << " USER_PREFIX:" << USER_PREFIX
                 << " urdn:" << urdn
                 << endl;
        XFSByteArrayAttribute* sa;
        generateAttribute( a, rdn, true );

        LG_XFS_D << "XFS::returning new attribute for rdn:" << rdn << endl;

        fh_attribute ret = a->getAttribute( StripUserNameSpace( rdn ) );
        
        LG_CTX_D << "EAGenerator_XFSNative::CreateAttr(4) rdn:" << rdn
                 << " USER_PREFIX:" << USER_PREFIX
                 << " urdn:" << urdn
                 << endl;
        return ret;
    }
    catch( FerrisCreateAttributeFailed& e )
    {
        cerr << "xfs.FerrisCreateAttributeFailed e:" << e.what() << endl;
        throw e;
    }
    catch( exception& e )
    {
        cerr << "xfs.exception e:" << e.what() << endl;
        fh_stringstream ss;
        ss << e.what();
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern "C"
{
    FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
    {
        return new EAGenerator_XFSNative();
    }
};



 
};
