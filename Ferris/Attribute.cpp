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

    $Id: Attribute.cpp,v 1.32 2011/07/31 21:30:48 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <EAN.hh>
#include <Attribute_private.hh>
#include <Runner.hh>
#include <Singleton.h>
#include <SignalStreams.hh>

#include <Ferris_private.hh>  // VM debug
//#include <Shell.hh>           // createEA
#include <Iterator.hh>        //

#include <FerrisGPG_private.hh>

#include <FerrisSemantic.hh>

#include <sstream>

using namespace std;

//#define RDFCACHEATTRS_DISABLED 1

namespace Ferris
{
    using namespace RDFCore;
    
    static fh_model getRDFCacheModel()
    {
        static fh_model m = 0;

        if( !m )
        {
            fh_context rdfdbc = Shell::acquireContext( "~/.ferris/rdfcacheattrs" );
            std::string storage_name = getStrSubCtx( rdfdbc, "ferris-storage-name", "" );

            string dboptionsDefault = "";
            {
                stringstream ss;
                ss << "db-environment-dir='" << Shell::getHomeDirPath() << "/.ferris/rdfcacheattrs/" << "'";
                dboptionsDefault = tostr(ss);
            }

            m = Model::FromMetadataContext( "~/.ferris/rdfcacheattrs/metadata" );
                
            // if( !storage_name.empty() )
            // {
            //     std::string db_name      = getStrSubCtx( rdfdbc, "ferris-db-name", "~/.ferris/rdfcacheattrs/cacheonly" );
            //     std::string db_options   = getStrSubCtx( rdfdbc, "ferris-db-options", dboptionsDefault );
            //     m = Model::ObtainDB( storage_name, db_name, db_options );
            // }
            // else
            // {
            //     m = Model::ObtainDB( rdfdbc, "cacheonly", "~/.ferris/rdfcacheattrs" );
            // }
        }
        
//         if( !m )
//             m = RDFCore::getDefaultFerrisModel();

        return m;
    }
    static fh_node getRDFCacheMTimePred()
    {
        static fh_node ret = 0;
        if( !ret )
            ret = Node::CreateURI( Semantic::getPredicateURI( "rdf-cache-mtime" ) );
        return ret;
    }
    static fh_node getRDFCacheEANamesListPred()
    {
        static fh_node ret = 0;
        if( !ret )
            ret = Node::CreateURI( Semantic::getPredicateURI( "rdf-cache-ea-names" ) );
        return ret;
    }
    static string getRDFCacheNodeURIPostfix()
    {
        return "-rdfcache-attributes";
    }
    static fh_node getRDFCacheNodeURIPostfix( AttributeCollection* c )
    {
        Context* cp = dynamic_cast<Context*>(c);
        return Node::CreateURI( cp->getURL() + getRDFCacheNodeURIPostfix() );
    }
    static fh_node tryToGetRDFCacheNode( AttributeCollection* c )
    {
        fh_model m = getRDFCacheModel();
        fh_node earlnode = getRDFCacheNodeURIPostfix( c );
        fh_node obj = m->getObject( earlnode, getRDFCacheMTimePred() );
        if( obj )
        {
            LG_RDFATTRCACHE_D << "tryToGetRDFCacheNode() found node for c:" << ((Context*)c)->getURL() << endl;
            return earlnode;
        }
        return 0;


//         fh_node uuid = Semantic::tryToGetUUIDNode( dynamic_cast<Context*>( c ) );
//         if( uuid )
//         {
//             static fh_node pred = 0;
//             if( !pred )
//                 pred = Node::CreateURI( Semantic::getPredicateURI( "rdf-attribute-cache" ) );

//             fh_node ret = m->getObject( uuid, pred );
//             return ret;
//         }
//         return 0;
    }
    static fh_node ensureRDFCacheNode( AttributeCollection* c )
    {
        fh_node ret = tryToGetRDFCacheNode( c );
        if( !ret )
        {
            LG_RDFATTRCACHE_D << "ensureRDFCacheNode() making node for c:" << ((Context*)c)->getURL() << endl;
            fh_model m = getRDFCacheModel();
            ret = getRDFCacheNodeURIPostfix( c );
            m->insert( ret,
                       getRDFCacheMTimePred(),
                       Node::CreateLiteral( tostr( c->getRDFCacheMTime() ) ) );
        }

//         fh_model m = getRDFCacheModel();
//         fh_node uuid = Semantic::ensureUUIDNode( dynamic_cast<Context*>( c ) );
//         static fh_node pred = 0;
//         if( !pred )
//             pred = Node::CreateURI( Semantic::getPredicateURI( "rdf-attribute-cache" ) );

//         fh_node ret = m->getObject( uuid, pred );
//         if( !ret )
//         {
//             string uu = Util::makeUUID();
//             ret = RDFCore::Node::CreateURI( uu );
//             m->insert( uuid, pred, ret );

// //             m->set( Node::CreateURI( ((Context*)c)->getURL() ),
// //                     getRDFCacheMTimePred(),
// //                     Node::CreateLiteral( tostr( c->getRDFCacheMTime() ) ) );
//         }

        
        return ret;
    }
    static bool Inside_RDFCacheAttributes_priv_createAttributes = false;

    static void addToRDFCacheNode( AttributeCollection* c, const string& rdn )
    {
        fh_model m = getRDFCacheModel();
        fh_node rdfcachenode = ensureRDFCacheNode( c );

        LG_RDFATTRCACHE_D << "addToRDFCacheNode... rdn:" << rdn << endl;
        m->insert( rdfcachenode,
                   getRDFCacheEANamesListPred(),
                   Node::CreateURI( Semantic::getPredicateURI( rdn ) ) );
    }
    
    
    static bool isRDFCacheAttributesEnabled()
    {
        static bool ret = true;
        static bool v = true;
        if( v )
        {
            v = false;
            ret = isTrue( getEDBString( FDB_GENERAL,
                                        CFG_RDFCACHE_ATTRS_ENABLED_K,
                                        CFG_RDFCACHE_ATTRS_ENABLED_DEFAULT ) );
        }
        return ret;
    }
    
    
    namespace Private 
    {
        static bool checkOpenModeSupported( ferris_ios::openmode supported,
                                            ferris_ios::openmode userRequested )
        {
            ferris_ios::openmode r = ferris_ios::maskOffFerrisOptions( userRequested );
            // cerr << "checkOpenModeSupported() sup:" << supported
            //      << " UR:" << userRequested
            //      << " r:" << r
            //      << " r&s:" << (r & supported)
            //      << endl;
            
            if( (r & supported) != r )
            {
                return false;
            }
            return true;
        }
    };


    class StaticStringGenerator
    {
        typedef map< const char*, string > m_cache_t;

        static m_cache_t& getCache()
            {
                static m_cache_t ret;
                return ret;
            }
        
        static string* getNullStringPointer()
            {
                static string ret;
                return &ret;
            }
        
    public:
        static string* Ptr( const char* cptr )
            {
                if( !cptr || cptr[0] == '\0' )
                    return getNullStringPointer();

                cerr << "StaticStringGenerator() sptr:" << cptr << endl;
                
                m_cache_t::iterator iter = getCache().find( cptr );
                if( iter == getCache().end() )
                {
                    cerr << "StaticStringGenerator() adding:" << cptr << endl;
                    iter = getCache().insert( make_pair( cptr, string( cptr ) ) ).first;
                }
                return &iter->second;
            }
    };
    

    
    /**
     * This seeks the stream to adjust for the open mode given.
     */
    void
    AdjustForOpenMode_Opening( fh_istream ss, ferris_ios::openmode m )
    {
        ss->seekg(0, ios::cur);
        if( m & ios::ate || m & ios::app )
        {
            ss->clear();
            ss->seekg(0, ios::end);
            ss->clear();
        }
        ss->clear();
    }
    
    /**
     * This seeks the stream to adjust for the open mode given.
     */
    void
    AdjustForOpenMode_Opening( fh_iostream ss, ferris_ios::openmode m )
    {
        ss->seekg(0, ios::cur);
        ss->clear();

        if( m & ios::trunc )
        {
            ss->clear();
            ss->seekp(0);
            ss->clear();
        }
        if( m & ios::ate || m & ios::app )
        {
            ss->clear();
            ss->seekg(0, ios::end);
            ss->seekp(0, ios::end);
            ss->clear();
        }
        
//        AdjustForOpenMode_Opening( fh_istream(ss), m );
    }
    
    void
    AdjustForOpenMode_Closing( fh_istream& ss, ferris_ios::openmode m, std::streamsize tellp )
    {
        ss->clear();
        ss->seekg(0);
        
        if( m & ios::trunc )
        {
            std::streampos be = 0;
            std::streampos en = tellp;
//            cerr << "AdjustForOpenMode_Closing() be:" << be << " en:" << en << endl;
            ss = Factory::MakeLimitingIStream( ss, be, en );
        }
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    
    /**
     * Create a new attribute. 
     */
    Attribute::Attribute( Parent_t parent )
        :
        theParent( parent )
    {
    }

    /**
     * Clean up the attribute
     */
    Attribute::~Attribute()
    {
    }

    /**
     * Get the parent attribute.
     *
     * @throws FerrisParentNotSetError If there is no parent set.
     * @see isParentBound()
     * @return The parent attribute
     */
    Attribute::Parent_t
    Attribute::getParent() throw (FerrisParentNotSetError)
    {
        if( !isParentBound() )
        {
            LG_ATTR_ER << "Parent context is not bound when it should be!" << endl;
            cerr << "Parent context is not bound when it should be! this:" << toVoid(this) << endl;
            DEBUG_dumpcl("Parent context is not bound when it should be!");
            BackTrace();
            g_on_error_query(0);
            
            LG_ATTR_ER << "Parent context is not bound when it should be!" << endl;
            Throw_FerrisParentNotSetError("", this);
        }
        return theParent;
    }

    /**
     * Check to see if this attribute is part of a tree or not.
     *
     * @return true if this attribute knows what its parent attribute is.
     */
    bool
    Attribute::isParentBound()
    {
        return theParent != 0;
    }



    /**
     * Set the parent context and rdn of this attribute. Note that this method
     * is a needed call for the lifetime of any useful attribute or context.
     *
     * @param parent The parent attribute or context. This can be NULL.
     * @param rdn Name of this context. Note that rdn must not clash with
     *        any of the other siblings in parent. This should be checked before
     *        calling this method.
     * @param emit If true then a getDirNameInitiallySet_Sig() signal is emmited to
     *        signal the addition of this new context.
     *
     */
    void
    Attribute::setAttributeContext( Parent_t parent )
    {
//         if( rdn )
//         {
//             LG_ATTR_D << "Attribute::setAttributeContext() this:" << this
//                       << " rdn:" << rdn
//                       << endl;
//         }
        
        theParent  = parent;
    }
    
    

    const std::string&
    Attribute::getDirName() const
    {
        stringstream ss;
        ss << "Getting the name of an Attribute class object. Should never happen" << endl;
        Throw_FerrisInternalError( tostr(ss), 0 );
    }
    


    /**
     * Get the entire path for this attribute.
     *
     * @throws FerrisParentNotSetError if called on a contex that does not know its
     *         parent
     * @see getDirName()
     * @returns The fully resolved path for this attribute.
     */
    std::string
    Attribute::getDirPath() throw (FerrisParentNotSetError)
    {
        try
        {
            if( isParentBound() )
            {
                return appendToPath( getParent()->getDirPath(), getDirName());
            }
            else
            {
                return getDirName();
            }
        }
        catch( FerrisParentNotSetError& e )
        {
            throw;
        }
        catch( exception& e )
        {
            LG_ATTR_ER
                << "getDirPath() cought a unchecked excpetion that it should not have"
                << endl
                << "e:" << e.what()
                << endl;
        }
        LG_ATTR_ER << "Returning default value of nothing." << endl;
        return "";
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    template< class T > T Attribute::copyTo( fh_istream iss, T oss )
    {
        LG_ATTR_D << "---------Start copy-----------" << endl;
        LG_ATTR_D << " oss is good():"<< oss->good() << endl;
        copy( istreambuf_iterator<char>(iss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(oss));
        
        LG_ATTR_D << " iss is good():"<< iss->good() << endl;
        LG_ATTR_D << " iss is eof():"<< iss->eof() << endl;
        LG_ATTR_D << " oss is good():"<< oss->good() << endl;
        LG_ATTR_D << " iss is state:"<< iss->rdstate() << endl;
        LG_ATTR_D << " oss is state:"<< oss->rdstate() << endl;

        char xch;
        iss >> xch;
        if( !iss->eof() )
        {
            fh_stringstream ss;
            ss << "Failed to copy file, path: " << getDirPath()
               << " iss is not at eof.";
            Throw_CopyFailed( tostr(ss), this );
        }
        return oss;
    }

    fh_iostream
    Attribute::copyTo( fh_iostream oss )
    {
        fh_istream iss  = getIStream();
        return copyTo( iss, oss );
    }
    
    fh_ostream
    Attribute::copyTo( fh_ostream oss )
    {
        fh_istream iss  = getIStream();
        return copyTo( iss, oss );
    }
     

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    /**
     * Get the openmodes that this object supports.
     *
     * @see checkOpenModeSupported()
     * @return The supported open modes.
     */
    ferris_ios::openmode
    Attribute::getSupportedOpenModes()
    {
//        cerr << "Attribute::getSupportedOpenModes() path:" << getDirPath() << endl;
        return
            ios_base::in        |
            ios_base::binary    ;
    }


    /**
     * Check if this attribute supports the openmode userm.
     *
     * @see getSupportedOpenModes()
     * @param userm The openmode to check support for
     * @return true if this attribute supports that openmode.
     */
    bool
    Attribute::checkOpenModeSupported( ferris_ios::openmode userm )
    {
        ferris_ios::openmode m = getSupportedOpenModes();
        
        LG_ATTR_D << "Attribute::checkOpenModeSupported()     m:" << m << endl;
        LG_ATTR_D << "Attribute::checkOpenModeSupported() userm:" << userm << endl;
        LG_ATTR_D << "Attribute::checkOpenModeSupported()      :" << (userm & m) << endl;

//         cerr << "Attribute::checkOpenModeSupported() path:" << getDirPath() << endl;
//         cerr << "Attribute::checkOpenModeSupported()     m:" << m << endl;
//         cerr << "Attribute::checkOpenModeSupported() userm:" << userm << endl;
//         cerr << "Attribute::checkOpenModeSupported()      :" << (userm & m) << endl;
//         cerr << "Attribute::checkOpenModeSupported()  ret :"
//              << Private::checkOpenModeSupported( m, userm ) << endl;
        
        return Private::checkOpenModeSupported( m, userm );
    }

    void
    Attribute::RegisterStreamWithContextMemoryManagement( fh_istream ss )
    {
        if( isParentBound() )
        {
            /* Parent is always a context, so this will not recurse */
            getParent()->RegisterStreamWithContextMemoryManagement( ss );
        }
    }

    /**
     * Get a IStream from this attribute.
     *
     * @see     getIOStream()
     * @see     getLocalIStream()
     *
     * @param   m Standard IO openmode that is desired for this stream
     *          note that a mode of ios::app is generally not supported
     *
     * @throws  FerrisParentNotSetError
     * @throws  CanNotGetStream Creating the stream failed
     * @throws  exception 
     * @returns a IStream
     */
    fh_istream
    Attribute::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        if( !checkOpenModeSupported(m))
            Throw_CanNotGetStream("openmode not supported", this);

        if( Context* c = dynamic_cast<Context*>(this))
        {
            LG_ATTR_D << "getIStream() url:" << c->getURL()
                      << " isCompressed:" << isCompressedContext( c )
                      << endl;
            
            if( isCompressedContext( c ) )
            {
                return Factory::getCompressedChunkIOStream( c );
            }
        }
        

        /*
         * handle download-if-mtime-since EA in a global fashion
         */
        if( Context* c = dynamic_cast< Context* >( this ) )
        {
            c->testDownloadIfMTimeSince( 0, false );
        }

//        cerr << "Attribute::getIStream(2) p:" << getDirPath() << endl;

//         if( time_t dlt = getDownloadIfMTimeSince() )
//         {
//             if( Context* c = dynamic_cast< Context* >( this ) )
//             {
//                 if( c->isAttributeBound( "mtime", false ) )
//                 {
//                     time_t m = toType< time_t >(getStrAttr( c, "mtime", "0" ));
//                     if( m <= dlt )
//                     {
//                         fh_stringstream ss;
//                         ss << "Remote document not modified, no source for:"
//                            << c->getURL() << endl;
//                         Throw_ContentNotModified( tostr(ss), 0 );
//                     }
//                 }
//             }
//         }
        
        
        fh_istream ret = priv_getIStream(m);

        AdjustForOpenMode_Opening( ret, m );
        RegisterStreamWithContextMemoryManagement( ret );

        LG_ATTR_D << "Attribute::getIStream() m & o_nouncrypt:" << (m & ferris_ios::o_nouncrypt)
                  << " castable:" << toVoid( dynamic_cast< Context* >( this ) )
                  << " ends_with:" << ends_with( getDirName(), ".gpg" )
                  << endl;
        
        if( (m & ferris_ios::o_nouncrypt) == 0 )
        {
            LG_ATTR_D << "Attribute::getIStream(1)." << endl;
            if( Context* c = dynamic_cast< Context* >( this ) )
            {
                LG_ATTR_D << "Attribute::getIStream(2)." << endl;
                if( ends_with( getDirName(), ".gpg" ))
                {
                    LG_ATTR_D << "Attribute::getIStream(3)." << endl;
                    LG_ATTR_D << "Attribute::getIStream() getting signed document stream." << endl;
                    fh_istream cc = getSignedDocumentStream( c->ThisContext(), ret );
                    return cc;
                }
            }
        }
        
        LG_ATTR_D << "Attribute::getIStream(no gpg returning raw stream)." << endl;
        return ret;
    }

    /**
     * If getIStream() was going to return an IStream that was not on local
     * disk, then this method creates a proxy IStream that is on local disk
     * from the attribute.
     *
     * @param   new_dn Contains the name of the local disk file. This can be
     *          used to pass data to a non-ferris API that requires a local
     *          disk file name.
     * @param   m Standard IO openmode that is desired for this stream
     * @see     getIOStream()
     * @see     getIStream()
     * @return  Local stream
     */
    fh_istream
    Attribute::getLocalIStream( std::string& new_dn, ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        fh_istream ret = getIStream(m);
        
        if( !g_file_test( getDirPath().c_str(),
                          GFileTest(
                              G_FILE_TEST_IS_REGULAR |
                              G_FILE_TEST_EXISTS) ))
        {
            f_ififostream fifos(ret);
            new_dn = fifos.getFileName();
            return fifos;
        }
        
        new_dn = getDirPath();
        return ret;
    }


    /**
     * Get a IOStream from this attribute.
     *
     * @see     getIStream()
     * @see     getLocalIStream()
     *
     * @param   m Standard IO openmode that is desired for this stream
     * @throws  FerrisParentNotSetError
     * @throws  CanNotGetStream Creating the stream failed
     * @throws  exception 
     * @returns a IOStream
     */
    fh_iostream
    Attribute::getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               AttributeNotWritable,
               exception)
    {
//        LG_ATTR_D << "Attribute::getIOStream() m:" << int(m) << endl;

        if( !checkOpenModeSupported(m))
        {
//            BackTrace();
            stringstream ss;
            ss << "openmode not supported m:" << m
               << " m.masked:" << ferris_ios::maskOffFerrisOptions( m )
               << " suported modes are:" << getSupportedOpenModes() << endl
               << " standard modes are... "
               << " in:" << ios::in
               << " out:" << ios::out
               << " ate:" << ios::ate
               << " app:" << ios::app
               << " trunc:" << ios::trunc
               << " bin:" << ios::binary
               << " o_mseq:" << ferris_ios::o_mseq
               << endl;
            Throw_CanNotGetStream( ss.str(), this);
        }
        

        if( Context* c = dynamic_cast<Context*>(this))
        {
            if( isCompressedContext( c ) )
            {
                return Factory::getCompressedChunkIOStream( c );
            }
        }
        
        /*
         * We want to throw an exception if there is something
         * wrong with the stream
         */
        fh_iostream ret = priv_getIOStream(m);
        LG_ATTR_D << "getIOStream() ret:" << ret << endl;

//         ret->exceptions( ios::badbit | ios::failbit );
//         ret->exceptions( ios::goodbit);
        if( !ret->good() )
        {
//            BackTrace();
            fh_stringstream ss;
            ss << "Can not get stream for path:" << getDirPath() << endl;
            Throw_CanNotGetStream(tostr(ss), this);
        }
        
        AdjustForOpenMode_Opening( ret, m );
        RegisterStreamWithContextMemoryManagement( ret );
        return ret;
    }


    /**
     * If this method is called then the specified open mode 'm' has already
     * passed a check to see if the attribute supports that mode. The mode might
     * still fail when the attempt is made to create the IStream.
     *
     * @see     getSupportedOpenModes()
     * @see     checkOpenModeSupported()
     * @see     getIStream()
     * @see     getIOStream()
     * @see     priv_getIOStream()
     * @param   m Standard IO openmode that is desired for this stream
     * @throws  FerrisParentNotSetError
     * @throws  CanNotGetStream Creating the stream failed
     * @throws  exception 
     * @returns a IStream
     */
    fh_istream
    Attribute::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        f_istringstream iss("");
        return iss;
    }
    
    /**
     * If this method is called then the specified open mode 'm' has already
     * passed a check to see if the attribute supports that mode. The mode might
     * still fail when the attempt is made to create the IOStream.
     *
     * @see     getSupportedOpenModes()
     * @see     checkOpenModeSupported()
     * @see     getIOStream()
     * @see     getIStream()
     * @see     priv_getIStream()
     * @param   m Standard IO openmode that is desired for this stream
     * @throws  FerrisParentNotSetError
     * @throws  CanNotGetStream Creating the stream failed
     * @throws  exception 
     * @returns a IStream
     */
    fh_iostream
    Attribute::priv_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception)
    {
        ferris_ios::openmode writeBits = ios::trunc | ios::app | ios::out;
            
        if( !(m & writeBits) )
        {
            LG_ATTR_D << "priv_getIOStream() mode is read only and getIOStream"
                      << " is not valid for this context, switching to read only"
                      << " stream instead" << endl;
            fh_istream iret = priv_getIStream( m );
            LG_ATTR_D << "priv_getIOStream() have istream" << endl;
            fh_iostream ret = Factory::MakeReadOnlyIOStream( iret );
            LG_ATTR_D << "priv_getIOStream() have iostream wrapper" << endl;
            return ret;
        }
        
        fh_stringstream ss;
        ss << "Attribute::priv_getIOStream() rdn:" << getDirName()
           << " path:" << getDirPath()
           << endl;
//        BackTrace();
        Throw_AttributeNotWritable(tostr(ss),0);//this);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    
    /**
     * Split the path into two chunks: The first context level and the rest.
     * This is a method of attribute because different context classes can
     * handle namespace seperation differently. In particular it is not always
     * the case that a "/" character is used to seperate context levels.
     *
     * @param s string to split into start, rest.
     * @see canSplitPathAtStart()
     * @return a string pair where first is the leading context name and second is
     *           the remaining context names strung together.
     */
    std::pair<std::string,std::string>
    Attribute::splitPathAtStart( const std::string& s )
    {
        std::pair<std::string,std::string> ret;
        std::string::size_type e = std::string::npos;
    
//    LG_ATTR_D << "Attribute::splitPathAtStart() s:" << s << endl;

        e = s.find(getSeperator());
        if( std::string::npos == e )
        {
            ret.first  = s; ret.second = "";
        }
        else
        {
            ret.first  = s.substr( 0, e );
            ret.second = s.substr( e+1 );
        }
    
    

//     LG_ATTR_D << "Attribute::splitPathAtStart() ret.first  :" << ret.first << endl;
//     LG_ATTR_D << "Attribute::splitPathAtStart() ret.second :" << ret.second << endl;
    
        return ret;
    }


    /**
     * Split the path into two chunks: The first part is like dirname(1) and the
     * second part is the rdn of the final context level.
     * This is a method of attribute because different context classes can
     * handle namespace seperation differently. In particular it is not always
     * the case that a "/" character is used to seperate context levels.
     *
     * @param s string to split into dirname(1), rdn.
     * @see canSplitPathAtEnd()
     * @return a string pair where first is the dirname(1) and second is
     *           the rdn of the final context in s.
     */
    std::pair< std::string, std::string >
    Attribute::splitPathAtEnd( const std::string& s )
    {
        std::pair< std::string, std::string > ret;
        std::string::size_type e = std::string::npos;
    
//     LG_ATTR_D << "Attribute::splitPathAtEnd() s:" << s << endl;

        e = s.find_last_of(getSeperator());

        if( std::string::npos == e || e == s.length()-1 )
        {
            ret.first  = s;
            ret.second = "";
        }
        else
        {
            ret.first  = s.substr( 0, e );
            ret.second = s.substr( e+1 );
        }
    

//     LG_ATTR_D << "Attribute::splitPathAtEnd() ret.first  :" << ret.first << endl;
//     LG_ATTR_D << "Attribute::splitPathAtEnd() ret.second :" << ret.second << endl;
    
        return ret;
    }

    /**
     * Check to see if a given path has enough information in it to be split
     * into two chunks using splitPathAtStart().
     *
     * @param s Path to split
     * @return true if the path is splittable.
     */
    bool
    Attribute::canSplitPathAtStart( const std::string& s )
    {
        std::pair<std::string,std::string> sp = splitPathAtStart( s );

        if( sp.first == s )
            return 0;

        return 1;
    }

    /**
     * Check to see if a given path has enough information in it to be split
     * into two chunks using splitPathAtEnd().
     *
     * @param s Path to split
     * @return true if the path is splittable.
     */
    bool
    Attribute::canSplitPathAtEnd( const std::string& s )
    {
        std::pair<std::string,std::string> sp = splitPathAtEnd( s );
    
        if( sp.first == s )
            return 0;

        return 1;
    }


    /**
     * Create a copy of a path with all the leading context seperators
     * removed. For a normal unix directory naming scheme this is equal to
     * trimming all leading "/" characters. This method exists because "/"
     * may not be the context seperator char, and indeed there may be more
     * than one seperator char.
     *
     * @param s The path to trim leading seperators from
     * @see trimTrailingSeps()
     * @see trimEdgeSeps()
     * @return A string without the leading seperators
     */
    std::string
    Attribute::trimLeadingSeps( const std::string& s )
    {
        std::string::size_type e = s.find_first_not_of( getSeperator() );
        if( e == std::string::npos )
            return s;
        return s.substr( e );
    }

    /**
     * Create a copy of a path with all the trailing context seperators
     * removed. For a normal unix directory naming scheme this is equal to
     * trimming all "/" characters from the end of string.
     * This method exists because "/"
     * may not be the context seperator char, and indeed there may be more
     * than one seperator char.
     *
     * @param s The path to trim trailing seperators from
     * @see trimLeadingSeps()
     * @see trimEdgeSeps()
     * @return A string without the trailing seperators
     */
    std::string
    Attribute::trimTrailingSeps( const std::string& s )
    {
        std::string::size_type e = s.find_last_not_of( getSeperator() );
        if( e == std::string::npos )
            return s;
//    LG_ATTR_D << "trimTrailingSeps(s:" << s << ") gives:" << s.substr( e ) << endl;
        return s.substr( 0, e+1 );
    }

    /**
     * Create a copy of a path with all the trailing and leading context seperators
     * removed. For a normal unix directory naming scheme this is equal to
     * trimming all "/" characters from the end and start of string.
     * This method exists because "/"
     * may not be the context seperator char, and indeed there may be more
     * than one seperator char.
     *
     * @param s The path to trim trailing seperators from
     * @see trimLeadingSeps()
     * @see trimTrailingSeps()
     * @return A string without the trailing or leading seperators
     */
    std::string
    Attribute::trimEdgeSeps( const std::string& s )
    {
        return trimLeadingSeps(trimTrailingSeps(s));
    }


    /**
     * Equal to basename(1). This method will return the final rdn in
     * the given path, if the path only contains the final rdn and no
     * context seperators then the path itself will be returned.
     *
     * @param s the path to get the basename of.
     * @see canSplitPathAtEnd()
     * @see splitPathAtEnd()
     * @return rdn of the final path component of the given path.
     */
    std::string
    Attribute::getLastPartOfName( const std::string& s )
    {
        std::pair< std::string, std::string > p = splitPathAtEnd( s );
        if( p.second.length() )
            return p.second;
        return p.first;
    }



    /**
     * Append a rdn to a path. This method also handles cases where the
     * given path ends in a path seperator already and ensures that only
     * one path seperator component seperates the given path and rdn.
     *
     * @param p The path to append to
     * @param d the rdn to append to p
     * @param allowDirToBeAbsolute If d starts with a path seperator it is assumed
     *        to be an absolute path and thus the appended path is just 'd'.
     *
     * @see canSplitPathAtEnd()
     * @see splitPathAtEnd()
     * @see canSplitPathAtStart()
     * @see splitPathAtStart()
     * @return p + getSeperator() + d
     */
    std::string
    Attribute::appendToPath( const std::string& p, 
                             const std::string& d,
                             bool allowDirToBeAbsolute )
    {
        std::string sep = getSeperator();

        if( allowDirToBeAbsolute &&
            ( starts_with( d, sep ) ||
              starts_with( d, "file:" ) ||
              starts_with( d, "x-ferris:" )))
        {
            return d;
        }

        if( p.empty() )
            return d;
        
        std::string s = p.substr( p.length() - sep.length());
        if( s == sep )
        {
            return p+d;
        }
        else
        {
            return p+sep+d;
        }
    }

    /** 
     * Used to implement a bunch of functions with default code that uses one
     * seperator only and allows that seperator to be a string.
     *
     * @see canSplitPathAtEnd()
     * @see splitPathAtEnd()
     * @see canSplitPathAtStart()
     * @see splitPathAtStart()
     * @see appendToPath()
     * @return The seperator for contexts
     */
    const std::string DefaultAttrSeperator = "/";
    const std::string&
    Attribute::getSeperator()
    {
        return DefaultAttrSeperator;
    }
    
    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    EA_Atom::EA_Atom()
    {
    }

    EA_Atom::~EA_Atom()
    {
    }
    
    fh_iostream
    EA_Atom::getIOStream( Context* c, 
                          const std::string& rdn,
                          ferris_ios::openmode m  )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception)
    {
        fh_stringstream ss;
        ss << "Can not get stream for path:" << c->getDirPath()
           << " rdn:" << rdn
           << endl;
        BackTrace();
        cerr << tostr(ss) << endl;
        Throw_CanNotGetStream( tostr(ss), c );
    }
    

    bool
    EA_Atom::checkOpenModeSupported( ferris_ios::openmode userm )
    {
        ferris_ios::openmode m = getSupportedOpenModes();
        return Private::checkOpenModeSupported( m, userm );
    }
    
    ferris_ios::openmode
    EA_Atom::getSupportedOpenModes()
    {
        return
            ios_base::in        |
            ios_base::binary    ;
    }

    bool
    EA_Atom::havePassedInSteamRead()
    {
        return false;
    }

    

    
    /******************************************************************************/
    /******************************************************************************/

    EA_Atom_ReadOnly::EA_Atom_ReadOnly( GetIStream_Func_t f )
        :
        GetIStream_Func( f )
    {
    }
    
    fh_istream
    EA_Atom_ReadOnly::getIStream( Context* c,
                                  const std::string& rdn,
                                  ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        return GetIStream_Func( c, rdn, this );
    }

    /******************************/
    /******************************/

    EA_Atom_ReadOnly_PassedInStream::EA_Atom_ReadOnly_PassedInStream( GetIStream_PassedInStream_Func_t f )
        :
        GetIStream_Func( f )
    {
    }

    fh_istream
    EA_Atom_ReadOnly_PassedInStream::getIStream( Context* c,
                                                 const std::string& rdn,
                                                 ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        fh_stringstream ss;
        getIStream( c, rdn, m, ss );
        return ss;
    }
    
    
    fh_stringstream&
    EA_Atom_ReadOnly_PassedInStream::getIStream( Context* c,
                                                 const std::string& rdn,
                                                 ferris_ios::openmode m,
                                                 fh_stringstream& ss )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        return GetIStream_Func( c, rdn, this, ss );
    }

    bool
    EA_Atom_ReadOnly_PassedInStream::havePassedInSteamRead()
    {
        return true;
    }
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    template < class ParentClass >
    fh_iostream
    EA_Atom_ReadWrite_Base< ParentClass >::getIOStream( Context* c,
                                                        const std::string& rdn,
                                                        ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception)
        {
            LG_ATTR_D << "EA_Atom_ReadWrite::getIOStream() m:" << int(m) << endl;
            
//             fh_stringstream ss;
//             {
//                 fh_iostream iss = GetIOStream_Func( c, rdn, this );

//                 std::copy( std::istreambuf_iterator<char>(iss),
//                            std::istreambuf_iterator<char>(),
//                            std::ostreambuf_iterator<char>(ss));
//             }
            fh_iostream ss;
            
            if( !( m & ios::trunc ))
                ss = priv_getIOStream( c, rdn );
            else
            {
                fh_stringstream z;
                ss = z;
            }
            
            
            ss << flush;
            ss->clear();
            ss->seekg(0);
            ss->seekp(0);
            ss->clear();

//             cerr << "EA_Atom_ReadWrite::getIOStream c:" << c->getURL() << " rdn:" << rdn << endl;
//             cerr << "EA_Atom_ReadWrite::getIOStream(7)" << endl;
//             cerr << " sbuf:" << (void*)(ss->rdbuf())
//                  << " rc:" << ss->getReferenceCount()
//                  << endl;
//             cerr << "EA_Atom_ReadWrite::getIOStream(8)" << endl;
//             cerr << "EA_Atom_ReadWrite::getIOStream(8) ss.rc:" << ss->rdbuf()->getReferenceCount() << endl;
            ss->getCloseSig().connect(
                bind(
                    bind(
                        bind(
                            sigc::mem_fun( *this, &_Self::On_IOStreamClosed),
                            rdn ),
                        c ),
                    m )
                );

            AdjustForOpenMode_Opening( ss, m );
            return ss;
        }


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/


    
    EA_Atom_ReadWrite::EA_Atom_ReadWrite(
        const GetIStream_Func_t& f_i,
        const GetIOStream_Func_t& f_io,
        const IOStreamClosed_Func_t& f_closed )
        :
//        _Base( f_i ),
        GetIOStream_Func( f_io ),
        RWBase_t( f_i, f_closed )
    {
    }
    
    fh_iostream
    EA_Atom_ReadWrite::priv_getIOStream( Context* c, const std::string& rdn )
    {
        return GetIOStream_Func( c, rdn, this );
    }

    fh_iostream
    EA_Atom_ReadWrite::getIOStream( Context* c,
                                    const std::string& rdn,
                                    ferris_ios::openmode m  )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        return RWBase_t::getIOStream( c, rdn, m );
    }
    
    
    ferris_ios::openmode
    EA_Atom_ReadWrite::getSupportedOpenModes()
    {
        return ios::in | ios::out | ios::binary;
    }

    
    void
    EA_Atom_ReadWrite::On_IOStreamClosed( fh_istream& ss_param,
                                          std::streamsize tellp,
                                          ferris_ios::openmode m,
                                          Context* c,
                                          std::string rdn )
    {
        fh_istream ss = ss_param;
        
        LG_ATTR_D << "EA_Atom_ReadWrite::IOStreamClosed() gtell:" << ss.tellg() << endl;
        LG_ATTR_D << "EA_Atom_ReadWrite::IOStreamClosed() rdn:" << rdn << endl;

        AdjustForOpenMode_Closing( ss, m, tellp );
        EA_Atom* atom = 0;
        IOStreamClosed_Func( c, rdn, atom, ss );
    }


    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    EA_Atom_ReadWrite_PassedInStream::EA_Atom_ReadWrite_PassedInStream(
        const GetIStream_PassedInStream_Func_t& f_i,
        const IOStreamClosed_Func_t& f_closed )
        :
        RWBase_t( f_i, f_closed )
    {
    }
    
    fh_iostream
    EA_Atom_ReadWrite_PassedInStream::priv_getIOStream( Context* c, const std::string& rdn )
    {
        fh_stringstream ss;
        return getIStream( c, rdn, ios::in | ios::out | ios::binary, ss );
    }
    
    
    ferris_ios::openmode
    EA_Atom_ReadWrite_PassedInStream::getSupportedOpenModes()
    {
        return ios::in | ios::out | ios::binary;
    }

    void
    EA_Atom_ReadWrite_PassedInStream::On_IOStreamClosed( fh_istream& ss_param,
                                                         std::streamsize tellp,
                                                         ferris_ios::openmode m,
                                                         Context* c,
                                                         std::string rdn )
    {
        fh_istream ss = ss_param;
        
        LG_ATTR_D << "EA_Atom_ReadWrite::IOStreamClosed() gtell:" << ss.tellg() << endl;
        LG_ATTR_D << "EA_Atom_ReadWrite::IOStreamClosed() rdn:" << rdn << endl;

        AdjustForOpenMode_Closing( ss, m, tellp );
        EA_Atom* atom = 0;
        IOStreamClosed_Func( c, rdn, atom, ss );
    }

    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    ferris_ios::openmode
    EA_Atom_ReadWrite_OpenModeCached::getOpenMode()
    {
        return theMode;
    }
    

    fh_istream
    EA_Atom_ReadWrite_OpenModeCached::getIStream( Context* c,
                                   const std::string& rdn,
                                   ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        theMode = m;
        return _Base::getIStream( c, rdn, m );
    }
    
    fh_iostream EA_Atom_ReadWrite_OpenModeCached::getIOStream( Context* c,
                                                               const std::string& rdn,
                                                               ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        theMode = m;
        return _Base::getIOStream( c, rdn, m );
    }
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    fh_iostream
    EA_Atom_ReadWrite_Contents::getIOStream( Context* c,
                                             const std::string& rdn,
                                             ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        theMode = m;
        fh_iostream ss = GetIOStream_Func( c, rdn, this );
        ss->clear();
        ss->seekg(0);
        ss->seekp(0);
        ss->clear();
        AdjustForOpenMode_Opening( ss, m );
        return ss;
    }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    

    

    EA_Atom_Static::EA_Atom_Static( const std::string& v )
        :
        theValue( v )
    {
    }
    
    fh_istream
    EA_Atom_Static::getIStream( Context* c,
                                      const std::string& rdn,
                                      ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
    {
        fh_stringstream ss;
        ss << theValue;
        return ss;
    }


    EA_Atom_RDFCacheAttribute::EA_Atom_RDFCacheAttribute( EA_Atom* a,
                                                          bool m_shouldUpdateRDFStore )
        :
        m_atom( a ),
        m_shouldUpdateRDFStore( m_shouldUpdateRDFStore )
    {
    }
    
    EA_Atom_RDFCacheAttribute::EA_Atom_RDFCacheAttribute( EA_Atom* a,
                                                          const std::string& m_cache )
        :
        m_atom( a ),
        m_cache( m_cache ),
        m_shouldUpdateRDFStore( false )
    {
    }
    
    fh_istream
    EA_Atom_RDFCacheAttribute::getIStream( Context* c,
                                           const std::string& rdn,
                                           ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        LG_RDFATTRCACHE_D << "getIStream() c:" << c->getURL()
                          << " m_shouldUpdateRDFStore:" << m_shouldUpdateRDFStore
                          << " rdn:" << rdn
                          << " rdfCacheTime:" << c->getRDFCacheMTime()
                          << endl;
        if( m_shouldUpdateRDFStore )
        {
            fh_istream iss = m_atom->getIStream( c, rdn, m );
            m_cache = StreamToString( iss );

            fh_model m = getRDFCacheModel();
            fh_node subj = ensureRDFCacheNode( c );
            fh_node pred = Node::CreateURI( Semantic::getPredicateURI( rdn ) );
            fh_node obj  = Node::CreateLiteral( m_cache );
            m->insert( subj, pred, obj );
            addToRDFCacheNode( c, rdn );

            LG_RDFATTRCACHE_D << "getIStream() c:" << c->getURL()
                              << " caching rdn:" << rdn
                              << " value:" << m_cache
                              << endl;

//             m->set( Node::CreateURI( ((Context*)c)->getURL() ),
//                     getRDFCacheMTimePred(),
//                     Node::CreateLiteral( tostr( c->getRDFCacheMTime() ) ) );
            m->set( subj, getRDFCacheMTimePred(),
                    Node::CreateLiteral( tostr( c->getRDFCacheMTime() ) ) );
//            m->sync();
            m_shouldUpdateRDFStore = false;
        }


        LG_RDFATTRCACHE_D << "getIStream() c:" << c->getURL()
                          << " rdn:" << rdn
                          << " cache:" << m_cache
                          << endl;
        fh_stringstream ss;
        ss << m_cache;
        return ss;
    }
    

    fh_iostream
    EA_Atom_RDFCacheAttribute::getIOStream( Context* c,
                                            const std::string& rdn,
                                            ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        return m_atom->getIOStream( c, rdn, m );
    }
    
    ferris_ios::openmode
    EA_Atom_RDFCacheAttribute::getSupportedOpenModes()
    {
        return ios::in | ios::out | ios::binary;
    }
    
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    AttributeProxy::AttributeProxy( fh_context c,
                                    EA_Atom* _atom,
                                    const std::string& aName )
        :
        atom( _atom ),
        HoldRefCountHigh( false ),
        theAttributeName( aName ),
        theContext( c )
    {
        if( !aName.length() || aName == "." )
        {
            theAttributeName = "content";
        }
        
        theParent  = GetImpl( c );
    }
    

    
    Handlable::ref_count_t
    AttributeProxy::AddRef()
    {
        if( HoldRefCountHigh )
        {
            return ref_count_t(HIGH_RC);
        }
        return Handlable::AddRef();
    }
    
    Handlable::ref_count_t
    AttributeProxy::Release()
    {
        if( HoldRefCountHigh )
        {
            return ref_count_t(HIGH_RC);
        }
        return Handlable::Release();
    }

    
    EA_Atom*
    AttributeProxy::getAttr()
    {
         EA_Atom* ret = atom;
        
//         ret = theContext->getAttributePtr(theAttributeName);
        
        if( !ret )
        {
            fh_stringstream ss;
            ss << "attempt to get EA that is not existant rdn:" << theAttributeName;
            Throw_NoSuchAttribute( tostr(ss), this );
        }
        return ret;
    }
    
    const std::string&
    AttributeProxy::getDirName() const
    {
        return theAttributeName;
    }
    
    std::string
    AttributeProxy::getDirPath() throw (FerrisParentNotSetError)
    {
        std::string ret = theContext->getDirPath();
        ret += theAttributeName;
        return ret;
    }
    
    fh_istream
    AttributeProxy::getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {

        fh_istream ss = getAttr()->getIStream( GetImpl(theContext), theAttributeName, m );
        ContextStreamMemoryManager::StreamIsOpeningHandler( getParent(), this, ss );
        return ss;
    }
    
        
    fh_istream AttributeProxy::getLocalIStream( std::string& new_dn, ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        fh_istream ret = getAttr()->getIStream( GetImpl(theContext), theAttributeName, m );
        ContextStreamMemoryManager::StreamIsOpeningHandler( getParent(), this, ret );
        
        if( !g_file_test( getDirPath().c_str(),
                          GFileTest(
                              G_FILE_TEST_IS_REGULAR |
                              G_FILE_TEST_EXISTS) ))
        {
            f_ififostream fifos(ret);
            new_dn = fifos.getFileName();
            return fifos;
        }
        
        new_dn = getDirPath();
        return ret;
    }
    
    fh_iostream AttributeProxy::getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception)
    {
//        cerr << "AttributeProxy::getIOStream() m:" << int(m) << endl;
        
        fh_iostream ss = getAttr()->getIOStream( GetImpl(theContext), theAttributeName, m );
        ContextStreamMemoryManager::StreamIsOpeningHandler( getParent(), this, ss );
        return ss;
    }

    bool
    AttributeProxy::checkOpenModeSupported( ferris_ios::openmode userm )
    {
        return getAttr()->checkOpenModeSupported( userm );
    }
    
    ferris_ios::openmode
    AttributeProxy::getSupportedOpenModes()
    {
        return getAttr()->getSupportedOpenModes();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    typedef Loki::SingletonHolder<
        Loki::AssocVector< Loki::TypeInfo,
                           AttributeCollection::SLAttributes_t* >,
        Loki::CreateUsingNew, Loki::NoDestroy > StateLessAttrsHolder;
    
    typedef Loki::SingletonHolder<
        AttributeCollection::SLAttributes_t,
        Loki::CreateUsingNew, Loki::NoDestroy > getStateLessAttrs_NULL_Holder;

    typedef FERRIS_STD_HASH_MAP< string, AttributeCollection::SLAttributes_t* > DynamicClassStateLessAttrsHolder_t;
    static DynamicClassStateLessAttrsHolder_t& getDynamicClassStateLessAttrsHolder()
    {
        static DynamicClassStateLessAttrsHolder_t ret;
        return ret;
    }
    
    

    AttributeCollection::AttributeCollection()
        :
        getStateLessAttrs_cache( &getStateLessAttrs_NULL_Holder::Instance() ),
        getStateLessAttrs_cache_isRAW( true ),
        m_namespaces( 0 ),
        m_RDFCacheMTime( 0 )
    {
    }
    
    AttributeCollection::~AttributeCollection()
    {
        clearAttributes();
        freeNamespaces();
    }

    AttributeCollection::SLAttributes_t*
    AttributeCollection::getStateLessAttrs()
    {
        if( getStateLessAttrs_cache_isRAW )
        {
            getStateLessAttrs_cache_isRAW = false;

//             cerr << "Setting up SL cache"
//                  << " this:" << dynamic_cast<Context*>(this)
//                  << " deep:" << getDeepestTypeInfo().name() << endl;
            SLAttributes_t* newcol = StateLessAttrsHolder::Instance()[ getDeepestTypeInfo() ];
            if( !newcol )
            {
                newcol = new SLAttributes_t();
                StateLessAttrsHolder::Instance()[ getDeepestTypeInfo() ] = newcol;
            }
            getStateLessAttrs_cache = newcol;
        }
        else
        {
//             cerr << "getStateLessAttrs(havecache)"
//                  << " this:" << dynamic_cast<Context*>(this)
//                  << " cache:" << (void*)getStateLessAttrs_cache
//                  << " deep:" << getDeepestTypeInfo().name()
//                  << " size:" << getStateLessAttrs_cache->size()
//                  << endl;
        }
        
        return getStateLessAttrs_cache;
    }


    void
    AttributeCollection::setup_DynamicClassedStateLessEAHolder( const std::string& className )
    {
        SLAttributes_t* newcol = getDynamicClassStateLessAttrsHolder()[ className ];
//        cerr << "setup_DynamicClassedStateLessEAHolder(1) cn:" << className << " newcol:" << newcol << endl;
        if( !newcol )
        {
            newcol = new SLAttributes_t();
            getDynamicClassStateLessAttrsHolder()[ className ] = newcol;
        }
//        cerr << "setup_DynamicClassedStateLessEAHolder(2) cn:" << className << " newcol:" << newcol << endl;
        getStateLessAttrs_cache = newcol;
        getStateLessAttrs_cache_isRAW = false;
    }
    bool
    AttributeCollection::isStateLessEAVirgin( const std::string& s )
    {
        bool ret = !getDynamicClassStateLessAttrsHolder().count( s );
//        cerr << "isStateLessEAVirgin() s:" << s << " ret:" << ret << endl;
        return ret;
    }
    
    AttributeCollection::Attributes_t&
    AttributeCollection::getAttributes()
    {
        return Attributes;
    }

    
    Loki::TypeInfo
    AttributeCollection::getDeepestTypeInfo()
    {
        TypeInfos_t ti = getTypeInfos();
        return ti.front();
    }
    
    
    std::list< Loki::TypeInfo >
    AttributeCollection::getTypeInfos()
    {
        TypeInfos_t TypeInfos;
        getTypeInfos( TypeInfos );
        return TypeInfos;
    }
    
    void
    AttributeCollection::callEnsureAttributesAreCreatedMarshalEToNSA( const std::string& eaname )
        throw( NoSuchAttribute )
    {
        try
        {
            ensureAttributesAreCreated( eaname );
        }
        catch( NoSuchAttribute& e )
        {
            throw;
        }
        catch( exception& e )
        {
            Throw_NoSuchAttribute( e.what(), 0);//this );
        }
    }

    /**
     * Get a handle for the attribute with a given rdn. If there is not attribute
     * with the given rdn then NoSuchAttribute is thrown.
     *
     * @see    getAttributeNames()
     * @param  rdn Name of attribute to get
     * @throws NoSuchAttribute If there is no attribute with rdn
     */
    fh_attribute
    AttributeCollection::getAttribute( const std::string& _rdn )
        throw( NoSuchAttribute )
    {
        std::string rdn = _rdn;

//         cerr << "AttributeCollection::getAttribute() this:" << (void*)this
//              << " rdn:" << rdn
//              << " ti.size:" << getTypeInfos().size()
//              << endl;

        
        if( !rdn.length() || rdn == "." )
        {
            rdn = "content";
        }
        
        if( EA_Atom* atom = getAttributePtr( rdn ) )
        {
            string rdn_exp = expandEAName( rdn, false );
//            cerr << "AttributeCollection::getAttribute() making proxy for rdn:" << rdn << endl;
            return new AttributeProxy( static_cast<Context*>(this), atom, rdn_exp );
        }

        std::stringstream ss;
        ss << "NoSuchAttribute() for attr:" << rdn << endl;
        Throw_NoSuchAttribute( tostr(ss), 0);//this );
    }
    
    
    /**
     * Returns Attribute* if its there or 0.
     *
     * @rdn The rdn to look for an attribute with
     */
    EA_Atom*
    AttributeCollection::getAttributeIfExists( const std::string& rdn )
    {     
        if( rdn != "mtime" )
        {
//            cerr << "getAttributeIfExists() rdn:" << rdn << endl;

            m_RDFCacheAttributes_t& rca = getRDFCacheAttributes();
            m_RDFCacheAttributes_t::iterator iter = rca.find( rdn );
            if( iter != rca.end() )
            {
                time_t mt = getMTime();
                if( mt <= getRDFCacheMTime() )
                {
//                    cerr << "getAttributeIfExists(A) rdn:" << rdn << endl;
                    LG_RDFATTRCACHE_D << "getAttributeIfExists() found rdfcacheAttr for rdn:" << rdn << endl;
                    return iter->second;
                }
                else
                {
//                    cerr << "getAttributeIfExists(B) rdn:" << rdn << endl;
                    // Been modified!
                    bool hadSz = !getRDFCacheAttributes().empty();
                    if( hadSz || m_RDFCacheMTime > 1 )
                    {
                        for( m_RDFCacheAttributes_t::iterator iter = getRDFCacheAttributes().begin();
                             iter != getRDFCacheAttributes().end(); iter++ )
                        {
                            delete iter->second;
                            iter->second = 0;
                        }
                        getRDFCacheAttributes().clear();
                        LG_RDFATTRCACHE_D << "erasing all cached RDF attributes for context"
                                          << " c:" << ((Context*)this)->getURL()
                                          << " mt:" << mt
                                          << " rdf-mt:" << getRDFCacheMTime()
                                          << endl;
                        m_RDFCacheMTime = mt;
                
                
                        // Wipe out the cache in the RDF store.
                        fh_model m = getRDFCacheModel();
                        fh_node n = tryToGetRDFCacheNode( this );
                        if( n )
                        {
                            LG_RDFATTRCACHE_D << "erasing all cached RDF attributes for context c:" << ((Context*)this)->getURL()
                                              << " subject node:" << n->toString()
                                              << endl;
                            m->eraseTransitive( n );
                        }
                    }
                }
            }
        }

        //
        //// 
        // These must be stateless for now.
        ////
        // 
        // LG_RDFATTRCACHE_D << "...0 inside:" << Inside_RDFCacheAttributes_priv_createAttributes << " rdn:" << rdn << endl;
        // if( rdn == "subtitles"
        //     && !Inside_RDFCacheAttributes_priv_createAttributes )
        // {
        //     LG_RDFATTRCACHE_D << "...1" << endl;
        //     if( shouldRDFCacheAttribute( rdn ) )
        //     {
        //         LG_RDFATTRCACHE_D << "...2" << endl;
        //         if( Attributes.end() != Attributes.find( rdn ) )
        //         {
        //             EA_Atom* a = Attributes[ rdn ];

        //             LG_RDFATTRCACHE_D << "...3" << endl;
        //             if( !dynamic_cast<EA_Atom_RDFCacheAttribute*>( a ) )
        //             {
        //                 LG_RDFATTRCACHE_D << "...4" << endl;
        //                 if( getRDFCacheAttributes().end() == getRDFCacheAttributes().find( rdn ) )
        //                 {
        //                     EA_Atom_RDFCacheAttribute* rdfa = new EA_Atom_RDFCacheAttribute( a, true );
        //                     LG_RDFATTRCACHE_D << "adding rdfcache attribute for... rdn:" << rdn
        //                                       << " value reading delayed." << endl;
        //                     getRDFCacheAttributes().insert( make_pair( rdn, rdfa ) );
        //                     m_RDFCacheMTime = getMTime();
        //                     return rdfa;
        //                 }
        //             }
        //         }
        //     }
        // }

        {
            SLAttributes_t* sl = getStateLessAttrs();
            SLAttributes_t::iterator iter = sl->find( rdn );
//             cerr << "AttributeCollection::getAttributeIfExists() rdn:" << rdn
//                  << " deep:" << getDeepestTypeInfo().name()
//                  << " found:" << ( sl->end() != iter ) << endl;
            
            if( sl->end() != iter )
            {
//                return iter->second;

                EA_Atom* a = iter->second;

                LG_RDFATTRCACHE_D << "getAttributeIfExists() should rdf cache the value of the given attribute."
                                  << " rdn:" << rdn
                                  << endl;
                
#ifndef RDFCACHEATTRS_DISABLED
                if( !Inside_RDFCacheAttributes_priv_createAttributes
                    && !dynamic_cast<EA_Atom_RDFCacheAttribute*>( a ) )
                {
                    if( shouldRDFCacheAttribute( rdn ) )
                    {
                        LG_RDFATTRCACHE_D << "getAttributeIfExists() should rdf cache the value of the given attribute."
                                          << " rdn:" << rdn
                                          << endl;
                        if( getRDFCacheAttributes().end() == getRDFCacheAttributes().find( rdn ) )
                        {
                            EA_Atom_RDFCacheAttribute* rdfa = new EA_Atom_RDFCacheAttribute( a, true );
                            LG_RDFATTRCACHE_D << "adding rdfcache attribute for... rdn:" << rdn
                                              << " value reading delayed." << endl;
                            getRDFCacheAttributes().insert( make_pair( rdn, rdfa ) );
                            m_RDFCacheMTime = getMTime();
                            return rdfa;
                        }
                    }
                }
#endif
                return a;
            }
        }

        
        if( Attributes.end() != Attributes.find( rdn ) )
        {
            return Attributes[ rdn ];
        }

//         cerr << "getAttributeIfExists() NOT THERE! url:" << ((dynamic_cast<Context*>(this))->getURL())
//              << " rdn:" << rdn << endl;

        return 0;
    }
    

//     static fh_node& RDFCachePredNode()
//     {
//         static fh_node ret = 0;
//         if( !ret )
//             ret = Node::CreateURI( getPredicateURI( "rdf-attribute-cache" ) );
//         return ret;
//     }

    
    
    EA_Atom*
    AttributeCollection::getAttributePtr( const std::string& raw_rdn )
        throw( NoSuchAttribute )
    {
        string rdn = expandEAName( raw_rdn, false );

        LG_ATTR_D << "Attribute::getAttributePtr(enter) raw_eaname:" << raw_rdn
                  << " namespace-expanded-eaname:" << rdn
                  << endl;
//         cerr << "getAttributePtr(enter) raw_eaname:" << raw_rdn
//              << " namespace-expanded-eaname:" << rdn
//              << endl;

        
        if( EA_Atom* a = getAttributeIfExists( rdn ))
        {
            LG_ATTR_D << "Attribute::getAttributePtr(2) raw_eaname:" << raw_rdn
                      << " namespace-expanded-eaname:" << rdn
                      << endl;
            return a;
        }
        if( starts_with( rdn, "schema:" ) )
        {
            if( Context* c = (dynamic_cast<Context*>(this)))
            {
                if( c->isParentBound() )
                {
                    return c->getParent()->getAttributePtr( "subtree" + rdn );
                }
            }
        }

        callEnsureAttributesAreCreatedMarshalEToNSA( rdn );

        if( EA_Atom* a = getAttributeIfExists( rdn ))
        {
            LG_ATTR_D << "Attribute::getAttributePtr(3) raw_eaname:" << raw_rdn
                      << " namespace-expanded-eaname:" << rdn
                      << endl;
            return a;
        }

        if( LG_ATTR_ACTIVE )
        {
            LG_ATTR_D << "Attribute::getAttributePtr() no such attr:" << rdn
                      << " size:" << Attributes.size()
                      << " path:" << ((dynamic_cast<Context*>(this))->getDirPath())
                      << endl;
            LG_ATTR_D << "No attr rdn:" << rdn 
                      << " path:" << ((dynamic_cast<Context*>(this))->getDirPath())
                      << " deepest:" << getDeepestTypeInfo().name()
                      << " SL.size:" << getStateLessAttrs()->size()
                      << " A.size:" << Attributes.size()
                      << endl;
            for( SLAttributes_t::iterator iter = getStateLessAttrs()->begin();
                 iter != getStateLessAttrs()->end(); ++iter )
            {
                LG_ATTR_D << " iter:" << iter->first << endl;
            }
            
            for( Attributes_t::iterator iter = Attributes.begin();
                 iter != Attributes.end(); ++iter )
            {
                LG_ATTR_D << " iter:" << iter->first << endl;
            }
        }
        
        
        std::stringstream ss;
        ss << "NoSuchAttribute() for:" << rdn
           << " path:" << (dynamic_cast<Context*>(this))->getDirPath()
           << endl;
//        cerr << ss.str();
//        BackTrace();
        Throw_NoSuchAttribute( tostr(ss), 0);//this );
    }
    
    void
    AttributeCollection::dumpAttributeNames()
    {
//         cerr << "========================================" << endl;
//         cerr << "     AttributeCollection::dumpAttributeNames()"
//              << " path:" << ((dynamic_cast<Context*>(this))->getDirPath())
//              << " deepest:" << getDeepestTypeInfo().name()
//              << endl;
//         for( SLAttributes_t::iterator iter = getStateLessAttrs()->begin();
//              iter != getStateLessAttrs()->end(); iter++ )
//         {
//             cerr << "    sl iter:" << iter->first << endl;
//         }

//         for( Attributes_t::iterator iter = Attributes.begin();
//              iter != Attributes.end(); iter++ )
//         {
//             cerr << "       iter:" << iter->first << endl;
//         }
//         cerr << "========================================" << endl;
    }
    

    AttributeCollection::AttributeNames_t&
    AttributeCollection::mergeAttributeNames( AttributeNames_t& ret,
                                              const AttributeNames_t& t1,
                                              const AttributeNames_t& t2 ) const
    {
        set_union( t1.begin(), t1.end(),
                   t2.begin(), t2.end(),
                   back_inserter( ret ));
        return ret;
    }

    /**
     * Get an STL collection of all the attribute names that this context has.
     *
     * @see    getAttributeCount()
     * @return STL collection of EA names.
     */
    AttributeCollection::AttributeNames_t&
    AttributeCollection::getAttributeNames( AttributeNames_t& ret )
    {
        ensureAttributesAreCreated();

        for( SLAttributes_t::iterator iter = getStateLessAttrs()->begin();
             iter != getStateLessAttrs()->end(); ++iter )
        {
            ret.push_back( iter->first );
        }

        for( Attributes_t::iterator iter = Attributes.begin();
             iter != Attributes.end(); ++iter )
        {
            ret.push_back( iter->first );
        }

        return ret;
    }
    

    /**
     * Get a count of how many attributes this attribute has.
     *
     * @see    priv_getAttributeCount()
     * @return Count of substtributes.
     */
    int
    AttributeCollection::getAttributeCount()
    {
        return Attributes.size() + getStateLessAttrs()->size();
    }

    /**
     * Test if there is an attribute with the given name
     *
     * @see    getAttributeNames()
     * @see    getAttribute()
     * @param  rdn Name of attribute to get
     * @throws NoSuchAttribute If there is a problem creating attributes
     */
    bool
    AttributeCollection::isAttributeBound( const std::string& rdn_raw, bool createIfNotThere )
        throw( NoSuchAttribute )
    {
        string rdn = expandEAName( rdn_raw, false );
        LG_ATTR_D << "AttributeCollection::isAttributeBound(enter) rdn:" << rdn << endl;

        if( getAttributeIfExists( rdn ))
        {
            return true;
        }
        else if( !createIfNotThere )
        {
            return false;
        }

        callEnsureAttributesAreCreatedMarshalEToNSA( rdn );
        try
        {
            return getAttributeIfExists( rdn );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "rdn:" << rdn << " e:" << e.what() << endl;
            Throw_NoSuchAttribute( tostr(ss), 0 );
        }
        
    }

    bool
    AttributeCollection::isStatelessAttributeBound( const std::string& rdn )
        throw( NoSuchAttribute )
    {
        SLAttributes_t* sl = getStateLessAttrs();
        SLAttributes_t::iterator iter = sl->find( rdn );
        if( sl->end() != iter )
        {
            return iter->second;
        }
        return false;
    }
    

    
    /**
     * Clear all the attributes. This method may delete attributes if a reference
     * is not still held to the attributes that are cleared from the collection.
     */
    void
    AttributeCollection::clearAttributes()
    {
//         if( dynamic_cast<Context*>(this) )
//         {
//             cerr << "clearAttributes() c:" << ((dynamic_cast<Context*>(this))->getURL())
//                  << " size:" << Attributes.size()
//                  << " c addr:" << (void*)((dynamic_cast<Context*>(this)))
//                  << endl;
//         }
        
        for( Attributes_t::iterator iter = Attributes.begin();
             iter != Attributes.end(); iter++ )
        {
            delete iter->second;
            iter->second = 0;
        }
        Attributes.clear();
    }


//     void
//     AttributeCollection::tryAddExternalAttribute(
//         const char* rdn,
//         const EA_Atom_ReadOnly::GetIStream_Func_t& f,
//         bool addToREA
//         )
//         throw( CanNotAddExternalAttribute )
//     {
//         fh_stringstream ss;
//         ss << "Can not add a new external attribute rdn:" << rdn << endl;
//         Throw_CanNotAddExternalAttribute( tostr(ss), 0 );
//     }

//     void
//     AttributeCollection::tryAddWritableExternalAttribute(
//         const char* rdn,
//         const EA_Atom_ReadOnly::GetIStream_Func_t& f_i,
//         const EA_Atom_ReadWrite::GetIOStream_Func_t& f_io,
//         const EA_Atom_ReadWrite::IOStreamClosed_Func_t& f_closed,
//         bool addToREA
//         )
//         throw( CanNotAddExternalAttribute )
//     {
//         fh_stringstream ss;
//         ss << "Can not add a new external attribute rdn:" << rdn << endl;
//         Throw_CanNotAddExternalAttribute( tostr(ss), 0 );
//     }
    
    
//     void
//     AttributeCollection::tryAddEA_Atom_Static(
//         const std::string& rdn,
//         const std::string& v,
//         bool addToREA
//         )
//         throw( CanNotAddExternalAttribute )
//     {
//         fh_stringstream ss;
//         ss << "AttributeCollection::Can not add a new fixed string attribute rdn:" << rdn << endl;
//         Throw_CanNotAddExternalAttribute( tostr(ss), 0 );
//     }


//     EA_Atom*
//     AttributeCollection::tryAddHeapAttribute( const std::string& rdn,
//                                               EA_Atom* a,
//                                               bool addToREA = false )
//     {
//         return a;
//     }
    
    
    
    
    bool
    AttributeCollection::addAttribute( const std::string& rdn,
                                       EA_Atom* atom,
                                       XSDBasic_t sct,
                                       bool addToREA )
    {
        bool rc = false;
        try
        {
            rc = setAttribute( rdn, atom, addToREA, sct );
        }
        catch( std::exception& e )
        {
            delete atom;
            throw;
        }
                
        if( !rc )
        {
            delete atom;
        }
        return rc;
    }
    
    

    /**
     * Insert a new subattribute into the collection.
     *
     * @throws  AttributeAlreadyInUse If the rdn of the new attribute is already in use.
     * @param   atx New attribute to add as a subattribute.
     */
    bool
    AttributeCollection::setAttribute( const std::string& rdn,
                                       EA_Atom* atx,
                                       bool addToREA,
                                       XSDBasic_t sct,
                                       bool isStateLess )
        throw( AttributeAlreadyInUse )
    {
        if( !atx )
        {
            LG_ATTR_ER << "Trying to set an unbound attribute! path"
                       << (dynamic_cast<Context*>(this))->getDirPath() << endl;
            cerr << "Trying to set an unbound attribute! path"
                 << (dynamic_cast<Context*>(this))->getDirPath() << endl;
            return false;
        }
        
        if( isAttributeBound( rdn, false ))
        {
//             cerr << "Warning, attempt to add attribute rdn:" << rdn
//                  << " when it is already bound." << endl;
//            BackTrace();
            return false;
        }

        
        if( isStateLess )
        {
//             cerr << "Setting stateless rdn:" << rdn
//                  << " deepest:" << getDeepestTypeInfo().name()
//                  << endl;
            string rdn_exp = expandEAName( rdn, false );
            if( EA_Atom* a = getAttributeIfExists( rdn_exp ))
            {
//                cerr << "setAttribute() AttributeAlreadyInUse:" << rdn << endl;
                
                std::stringstream ss;
                ss << "setAttribute() AttributeAlreadyInUse:" << rdn_exp;
                Throw_AttributeAlreadyInUse( tostr(ss), 0 );
            }

//            cerr << "setAttribute() adding stateless rdn:" << rdn << endl;
            getStateLessAttrs()->insert( make_pair( rdn, atx ) );
        }
        else
        {
            Attributes.insert( make_pair( rdn, atx ) );
        }

        
        if( !isStateLess && !starts_with( rdn, "schema:" ))
        {
            if( Context* c = dynamic_cast<Context*>(this))
            {
                //
                // Native disk EA are handled differently, see libxfsnative.cpp 
                //
                if( sct != FXD_BINARY_NATIVE_EA )
                {
                    attachGenericSchema( c, rdn, sct );
                }
            }
        }

        bumpVersion();
        return true;
    }

    void
    AttributeCollection::unsetAttribute( const std::string& rdn )
    {
        Attributes.erase( Attributes.find( rdn ) );
        bumpVersion();
    }
    
    /****************************************/
    /****************************************/
    /****************************************/

    time_t
    AttributeCollection::getMTime()
    {
        time_t ret = 0;
        ret = toType<time_t>(getStrAttr( this, "mtime", "0" ) );
        return ret;
    }


    AttributeCollection::m_RDFCacheAttributes_t&
    AttributeCollection::getRDFCacheAttributes()
    {
        return m_RDFCacheAttributes;
    }
    

     bool
     AttributeCollection::shouldRDFCacheAttribute( const std::string& s )
     {
//         cerr << "shouldRDFCacheAttribute() s:" << s << endl;
         
         if( !getIsNativeContext() )
             return false;
         
         {
             static FERRIS_STD_HASH_SET< std::string > col;
             if( col.empty() )
             {
                 string d = getEDBString( FDB_GENERAL,
                                          CFG_RDFCACHE_ATTRS_LIST_K,
                                          CFG_RDFCACHE_ATTRS_LIST_DEFAULT );
                 if( !d.empty() )
                 {
                     stringlist_t sl;
                     Util::parseCommaSeperatedList( d, sl );
                     copy( sl.begin(), sl.end(), inserter( col, col.end() ));
                 }
             }
//             cerr << "should cache:" << col.count( s ) << " s:" << s << endl;
             if( col.count( s ) )
                 return true;
         }

         return false;
     }
    
    time_t
    AttributeCollection::getRDFCacheMTime()
    {
        return m_RDFCacheMTime;
    }

    void
    AttributeCollection::RDFCacheAttributes_priv_createAttributes()
    {
#ifdef RDFCACHEATTRS_DISABLED
        return;
#endif

        LG_RDFATTRCACHE_D << "RDFCacheAttrs_createAttrs() top..."
                          << " c:" << ((Context*)this)->getURL()
                          << " c.name:" << ((Context*)this)->getDirName()
                          << endl;
        
        Util::ValueRestorer< bool > x( Inside_RDFCacheAttributes_priv_createAttributes, true );

        if( m_RDFCacheMTime==1 || !getRDFCacheAttributes().empty() )
            return;
        
        LG_RDFATTRCACHE_D << "RDFCacheAttrs_createAttrs() ok..."
                          << " c:" << ((Context*)this)->getURL()
                          << " c.name:" << ((Context*)this)->getDirName()
                          << endl;
//        m_RDFCacheMTime = getMTime();
        
        m_RDFCacheMTime = 1;
        fh_model m = getRDFCacheModel();
        
//         fh_node rdfCacheTimeObject = m->getObject( Node::CreateURI( ((Context*)this)->getURL() ),
//                                                    getRDFCacheMTimePred() );
//         if( !rdfCacheTimeObject )
//             return;
        
        fh_node subj = tryToGetRDFCacheNode( this );
        if( !subj )
        {
            return;
        }
        fh_node rdfCacheTimeObject = m->getObject( subj, getRDFCacheMTimePred() );
        if( !rdfCacheTimeObject )
        {
            return;
        }
        m_RDFCacheMTime = toint( rdfCacheTimeObject->toString() );
        
//         {
//             string rdn = "width";
//             fh_node obj = m->getObject( subj, Node::CreateURI( Semantic::getPredicateURI( rdn ) ));
//             if( obj )
//             {
//                 string cache = obj->toString();
//                 if( EA_Atom* a = getAttributeIfExists( rdn ))
//                 {
//                     LG_RDFATTRCACHE_D << "adding rdfcache attribute for... rdn:" << rdn
//                                       << " value:" << cache << endl; 
//                     EA_Atom* rdfa = new EA_Atom_RDFCacheAttribute( a, cache );
//                     getRDFCacheAttributes().insert( make_pair( rdn, rdfa ) );
//                 }
//             }
//             return;
//         }
        
        NodeIterator end   = NodeIterator();
        NodeIterator piter = m->findObjects( subj, getRDFCacheEANamesListPred() );
        for( ; piter != end; ++piter )
        {
            fh_node obj = m->getObject( subj, *piter );

            if( !obj )
                continue;
            
            string rdn = Semantic::stripPredicateURIPrefix( (*piter)->getURI()->toString() );
            string cache = obj->toString();

            LG_RDFATTRCACHE_D << "read RDFCache attrs... rdn:" << rdn
                              << " value:" << cache << endl;
            
            if( EA_Atom* a = getAttributeIfExists( rdn ))
            {
                LG_RDFATTRCACHE_D << "adding rdfcache attribute for... rdn:" << rdn
                                  << " value:" << cache << endl; 
                EA_Atom* rdfa = new EA_Atom_RDFCacheAttribute( a, cache );
                getRDFCacheAttributes().insert( make_pair( rdn, rdfa ) );
            }
        }
    }
    

    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static const std::string FERRIS_NAMESPACE_BASE   = "http://witme.sf.net/libferris-core/xmlns:";
    static const std::string FERRIS_NAMESPACE_SCHEMA
    = "http://witme.sf.net/libferris-core/ns/xmlschema/";
    static const std::string FERRIS_NAMESPACE_EMBLEM
    = "http://witme.sf.net/libferris-core/ns/emblem/";
    static const std::string FERRIS_NAMESPACE_UNKNOWN
    = "http://witme.sf.net/libferris-core/ns/unknown/";

    /**
     * Expand user defined namespaces to full URI for the given attribute name
     *
     * @param s The name of an EA which may include a namespace prefix to expand
     * @param expandInternalFerrisNamespaces (default true) if false then namespaces
     *                                      like schema: are not expanded.
     * @return The expanded EA name.
     */
    std::string
    AttributeCollection::expandEAName( const std::string& s,
                                       bool expandInternalFerrisNamespaces )
    {
        if( !expandInternalFerrisNamespaces )
            if( starts_with( s, "schema:")
                || starts_with( s, "emblem:")
                || starts_with( s, "fspot:")
                || starts_with( s, "http:")
                || starts_with( s, "tag:")
                || starts_with( s, "exif:") )
                return s;
        
        int colonIndex = s.find( ":" );
        if( colonIndex != string::npos )
        {
            string xmlns = s.substr( 0, colonIndex );
            string rest  = s.substr( colonIndex + 1 );

            string URI = resolveNamespace( xmlns );
            if( URI.empty() )
            {
                if( starts_with( s, FERRIS_NAMESPACE_UNKNOWN ))
                    return s;
                if( starts_with( s, "http://witme.sf.net" ))
                    return s;
                
//                cerr << "expandEAName() adding unknown prefix s:" << s << endl;
                return FERRIS_NAMESPACE_UNKNOWN + s;
            }
            

            
            fh_stringstream ss;
            ss << URI;
            if( URI[ URI.length()-1 ] != '/' )
                ss << "/";
            ss << rest;
//             cerr << "Context::expandEAName() s:" << s
//                  << " xmlns:" << xmlns
//                  << " rest:" << rest
//                  << " return:" << tostr(ss)
//                  << endl;
            
            return tostr(ss);
        }
        return s;
    }

    /**
     * For this context set the namespace for 'prefix' to resolve to 'URI'
     */
    void
    AttributeCollection::setNamespace( const std::string& prefix,
                                       const std::string& URI )
    {
        if( Context* c = dynamic_cast<Context*>(this))
        {
            setStrAttr( c, FERRIS_NAMESPACE_BASE + prefix, URI, true, true );
//            Shell::createEA( c, FERRIS_NAMESPACE_BASE + prefix, URI );
        }
    }

    /**
     * resolve the prefix to its set URI
     * @return the URI for prefix or an empty string if not found
     */
    std::string
    AttributeCollection::resolveNamespace( const std::string& prefix )
    {
        string ret = resolveFerrisXMLNamespace( prefix );

        if( ret == FERRIS_NAMESPACE_UNKNOWN )
            return "";

//        cerr << "resolveNamespace() prefix:" << prefix << " ret:" << ret << endl;
        return ret;
    }

    /**
     * Remove the binding for the prefix given.
     */
    void
    AttributeCollection::removeNamespace( const std::string& prefix )
    {
        setNamespace( prefix, "" );
    }
    
    stringlist_t
    AttributeCollection::getNamespacePrefixes()
    {
        if( !m_namespaces )
            readNamespaces();

        stringlist_t ret;
        stringmap_t::const_iterator begin = m_namespaces->begin();
        stringmap_t::const_iterator end = m_namespaces->end();
        copy( map_domain_iterator(begin), map_domain_iterator(end), back_inserter( ret ));

        return ret;
    }
    
    
    /**
     * Allocate m_namespaces and marshall all the attribute names that are namespace
     * prefixes into the m_namespaces map. If the namespaces have already been read
     * then they are cleared and reread from the current attributes for this context.
     *
     * When a new attribute has been created then updateNamespacesForNewAttribute()
     * should be called to taint the namespaces cache.
     */
    void
    AttributeCollection::readNamespaces()
    {
        if( !m_namespaces )
            m_namespaces = new stringmap_t;

        m_namespaces->clear();
        AttributeCollection::AttributeNames_t an;
        getAttributeNames( an );
        AttributeCollection::AttributeNames_t::iterator end = an.end();
        int FERRIS_NAMESPACE_BASE_LENGTH = FERRIS_NAMESPACE_BASE.length();
        
        for( AttributeCollection::AttributeNames_t::iterator iter = an.begin();
             iter != end; iter++ )
        {
            if( !starts_with( *iter, FERRIS_NAMESPACE_BASE ))
                continue;

            
            string prefix = iter->substr( FERRIS_NAMESPACE_BASE_LENGTH );
//             cerr << "readNamespaces() iter:" << *iter << endl
//                  << " prefix:" << prefix
//                  << endl;
            try
            {
                //
                // Note that we can't use getStrAttr() etc here because they
                // resolve and expand prefixes for us.
                //
//                string URI    = getStrAttr( this, *iter, "", true, true );
                string URI = "";
                if( EA_Atom* atom = getAttributeIfExists( *iter ))
                {
                    ferris_ios::openmode m = ios::in;
                    fh_istream ss = atom->getIStream( (Context*)this, *iter, m );
                    URI = StreamToString( ss );
                }
                
                m_namespaces->insert( make_pair( prefix, URI ));
//                 cerr << "Setting namespace:" << prefix
//                      << " to URI:" << URI
//                      << endl;
            }
            catch( exception& e )
            {
                cerr << "error setting namespace:" << prefix
                     << " e:" << e.what()
                     << endl;
            }
            

//             int eqIndex = p.find( "=" );
//             if( eqIndex != string::npos )
//             {
//                 string prefix = p.substr( 0, eqIndex );
//                 string URI    = p.substr( eqIndex + 1 );
//                 m_namespaces->insert( make_pair( prefix, URI ));
//             }
        }
    }

    void
    AttributeCollection::freeNamespaces()
    {
        if( m_namespaces )
        {
            m_namespaces->clear();
            delete m_namespaces;
            m_namespaces = 0;
        }
    }
    
    void
    AttributeCollection::updateNamespacesForNewAttribute( const std::string& s )
    {
        freeNamespaces();
    }


    /**
     * Walk up the tree looking for a user defined namespace for 's'.
     * Creates a cache of all set namespaces as the tree is walked upwards.
     *
     * @return The URI for the namespace 's' or the URI for a unknown
     *         prefix which is inside the libferris URI namespace.
     */
    std::string
    AttributeCollection::resolveFerrisXMLNamespace( const std::string& s )
    {
        if( s == "schema" )
            return FERRIS_NAMESPACE_SCHEMA;
        if( s == "emblem" )
            return FERRIS_NAMESPACE_EMBLEM;
        if( s == "http" )
            return FERRIS_NAMESPACE_UNKNOWN;
        
        if( !m_namespaces )
            readNamespaces();

        if( m_namespaces )
        {
            stringmap_t::const_iterator ni = m_namespaces->find( s );
            if( ni != m_namespaces->end() )
            {
//                 cerr << "resolveFerrisXMLNamespace() s:" << s
//                      << " ret:" << ni->second << endl;
                return ni->second;
            }
        }

//         cerr << "resolveFerrisXMLNamespace() s:" << s
//              << " ret==U:" << FERRIS_NAMESPACE_UNKNOWN << endl;
    
        return FERRIS_NAMESPACE_UNKNOWN;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void
    AttributeCollection::ensureAttributesAreCreated( const std::string& eaname )
    {
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    
///////////////////////////////////////////////////////////////////////////////    
///////////////////////////////////////////////////////////////////////////////    
///////////////////////////////////////////////////////////////////////////////    
    
    namespace Factory
    {
        
    
        fh_istream MakePipeEA( const fh_runner& Runner )
            throw( CanNotGetStream )
        {
            try
            {
                fh_istream ret = Runner->RunWithStdoutAsReadablePipe();
                return ret;
                
//                 Runner->Run();
//                 fh_istream filepipe = Runner->getStdOut();
//                 return filepipe;
            }
            catch( ProgramSpawn& e )
            {
                std::stringstream ss;
                ss //<< "ExecAttribute::priv_getIStream() path:" << getDirPath()
                    << " exec/fork was a failure."
                    << " ERR: " << Runner->getErrorString()
                    << endl;
                Throw_CanNotGetStream(tostr(ss), 0); //this);
            }
        }
    };


    const char*
    NonVolitileStringHolder::makeNonVolitileString( const std::string& s )
    {
        typedef std::list< std::string > v_t;
        static v_t v;
        v.push_back( s );
        return v.back().c_str();
    }
    
};
