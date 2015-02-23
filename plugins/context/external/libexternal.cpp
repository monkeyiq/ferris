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

    $Id: libexternal.cpp,v 1.9 2010/09/24 21:31:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <unistd.h>

#include <Ferris_private.hh>
#include <FerrisDOM.hh>

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <Runner.hh>

#include <mapping.hh>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;
#define X(str) XStr(str).unicodeForm()


namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };
    
    
    typedef map< string,string > RecordData_t;

    class externalContext;
    FERRIS_SMARTPTR( externalContext, fh_excontext );

    static string SCRIPTDIR = "/usr/local/ferris/extfs_modules/";


    class FERRISEXP_CTXPLUGIN externalContext
        :
        public StateLessEAHolding_Recommending_ParentPointingTree_Context< externalContext >
    {
        typedef externalContext _Self;
        typedef StateLessEAHolding_Recommending_ParentPointingTree_Context< externalContext > _Base;
    
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        string ScriptName;
        bool m_isDir;
        streamsize m_size;

        
    protected:

        virtual bool isDir()
            {
                return m_isDir || _Base::isDir() || getSubContextCount();
            }

        externalContext* priv_CreateContext( Context* parent, string rdn )
            {
                externalContext* ret = new externalContext();
                ret->setContext( parent, rdn );
                return ret;
            }

        virtual bool VetoEA()
            {
                return true;
            }
    

        string getScriptName();

        void writeStream( fh_istream& ss, std::streamsize tellp );
        virtual fh_context SubCreate_file( fh_context c, fh_context md );


        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                m["file"] = SubContextCreator(
                    SL_SubCreate_file,
                    "	<elementType name=\"file\">\n"
                    "		<elementType name=\"name\" default=\"new file\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }
    


        fh_runner getRunner( string Command, bool SplitArchiveName );

    
        virtual fh_istream real_getIOStream()
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception);

    
        virtual void priv_read();
    

//     externalContext* create( const string& xdn );
        fh_context ensureCreated( const string& xdn );
        void addContext( const RecordData_t& rd );
        void addContext( DOMNode* n );

        static fh_istream
        SL_getSizeIStream( externalContext* c, const std::string& rdn, EA_Atom* atom )
            {
//                LG_EXTFS_D << "SL_getSizeIStream() url:" << c->getURL() << " c->m_size:" << c->m_size << endl;
                fh_stringstream ret;
                ret << c->m_size;
                return ret;
            }
        
        
        void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() || force )
                {
                    tryAddStateLessAttribute( "size", SL_getSizeIStream, FXD_FILESIZE );
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        
        
    public:

        externalContext( Context* parent = 0, const std::string& rdn = "" );
        virtual ~externalContext();

        externalContext* makeContext( Context* parent, std::string rdn )
            {
                return priv_CreateContext( parent, rdn );
            }

        virtual void read( bool force = 0 );
    
        virtual fh_istream getIStream( ferris_ios::openmode m = ios::in )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception);
    
        virtual fh_iostream getIOStream( ferris_ios::openmode m = ios::in|ios::out )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   exception);

        void setScriptName( const string& v );
    
    
    };





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


externalContext::externalContext( Context* parent, const std::string& rdn )
    :
    _Base( parent, rdn ),
    m_isDir( false ),
    m_size( 0 )
{
    setContext( parent, rdn );
    LG_EXTFS_D << "externalContext() rdn:" << rdn << " path:" << getDirPath() << endl;
    createStateLessAttributes();
}

externalContext::~externalContext()
{
}

void
externalContext::setScriptName( const string& v )
{
    ScriptName = v;

    if( ScriptName=="ssh" )
    {
        addAttribute( EAN_IS_REMOTE, "1", XSD_BASIC_BOOL );
    }
    else
    {
        addAttribute( EAN_IS_REMOTE, "0", XSD_BASIC_BOOL );
    }
}


string
externalContext::getScriptName()
{
    if( !ScriptName.length() )
    {
        ScriptName = emap::getScriptName( getBaseContext() );
//         cerr << "externalContext::getScriptName() url:" << getBaseContext()->getURL()
//              << " found sc:" << ScriptName
//              << endl;
    }

    LG_EXTFS_D << "--- getScriptName() path:" << getDirPath()
               << " scName: " << ScriptName
               << endl;
    
    if( !ScriptName.length() )
    {
        fh_stringstream ss;
        ss << "no script name could be found for path:" << getDirPath()
           << " url:" << getURL()
           << endl;
        Throw_FerrisCanNotGetScriptNameError( tostr(ss), this );
    }
    
    
    return ScriptName;
}


fh_runner
externalContext::getRunner( string Command, bool SplitArchiveName )
{
    string ArchiveName = getBaseContext()->getDirPath();
    string FileToGet = "";
    if( getDirPath().length() >= ArchiveName.length()+1 )
        FileToGet = getDirPath().substr( ArchiveName.length()+1 );

    fh_runner r = new Runner();
    string s;

    s  = SCRIPTDIR;
    s += getScriptName();

    LG_EXTFS_D << "externalContext::getRunner() "
               << " script:" << s 
               << " Command:" << Command
               << " ArchiveName:" << ArchiveName
               << " FileToGet:" << FileToGet
               << " SplitArchiveName:" << SplitArchiveName
               << " path:" << getDirPath()
               << endl;
    
    r->getArgv().push_back( s );
    r->getArgv().push_back( Command );
    if( SplitArchiveName )
    {
        r->getArgv().push_back( ArchiveName );
        if( FileToGet.length() ) 
            r->getArgv().push_back( FileToGet );
    }
    else
    {
        r->getArgv().push_back( getDirPath() );
    }
    
    r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags() | G_SPAWN_STDERR_TO_DEV_NULL) );

    return r;
}


fh_istream
externalContext::real_getIOStream()
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           exception)
{
    fh_iostream ss;

    LG_EXTFS_D << "externalContext::real_getIOStream() " << endl;
    string Command     = "copyout";

    bool splitup = true;
    if( ScriptName=="ssh" )
    {
        splitup = false;
    }
    
    fh_runner r = getRunner( Command, splitup );
//     Ferris::Runner r;
//     string s;
//     s  = SCRIPTDIR;
//     s += getScriptName();
//     r->getArgv().push_back( s );
//     r->getArgv().push_back( Command );
//     r->getArgv().push_back( getDirPath() );

    try
    {
        fh_istream ret = r->RunWithStdoutAsReadablePipe();
        
//         r->setSpawnFlags(
//             GSpawnFlags(
//                 r->getSpawnFlags() & (~G_SPAWN_DO_NOT_REAP_CHILD) ));
//         r->Run();
        
        /* ok */
        LG_EXTFS_D << "externalContext::real_getIOStream() path:" << getDirPath()
                   << " exec/fork was ok."
                   << endl;

        return ret;
//         fh_istream filepipe = r->getStdOut();
//         return filepipe;
        
//         fh_stringstream tmpss;
//         copy( istreambuf_iterator<char>(filepipe),
//               istreambuf_iterator<char>(),
//               ostreambuf_iterator<char>(tmpss));

//         int rc = r->getExitStatus();
//         if( rc )
//         {
//             fh_stringstream ss;
//             ss << "Exit code:" << rc
//                << " script:" << getScriptName()
//                << " for url:" << getURL()
//                << endl;
//             LG_EXTFS_D << tostr(ss) << endl;
//             Throw_CanNotGetStream(tostr(ss),this);
//         }
        
// //         {
// //             string s;
// //             LG_EXTFS_D << "------------------------------------------------------------" << endl;
// //             while( getline( filepipe, s ) )
// //                 LG_EXTFS_D << s << endl;
// //             LG_EXTFS_D << "------------------------------------------------------------" << endl;
// //             exit(0);
// //         }

//         LG_EXTFS_D << "externalContext::real_getIOStream() path:" << getDirPath()
//                    << " returning file pipe to user."
//                    << endl;
        
// //        return filepipe;
//         return tmpss;
    }
    catch( exception& e )
    {
        /* fail */
        LG_EXTFS_D << "externalContext::real_getIOStream() path:" << getDirPath()
                   << " exec/fork was a failure."
                   << endl;
        
        fh_stringstream ss;
        ss << "getIOStream failed e:" << e.what()
           << " r:" << r->getErrorString();
        Throw_CanNotGetStream(tostr(ss),this);
    }
}



fh_istream
externalContext::getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    return real_getIOStream();
}

fh_context
externalContext::SubCreate_file( fh_context c, fh_context md )
{
    string rdn = getStrSubCtx( md, "name", "" );

    LG_EXTFS_D << "externalContext::SubCreate_file() rdn:" << rdn << endl;

    if( !rdn.length() )
    {
        fh_stringstream ss;
        ss << "Attempt to create file with no name" << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), this );
    }
    
    const string filename = rdn;
    const string fqfn = appendToPath(getDirPath(),filename);

    LG_EXTFS_D << "externalContext::createSubContext() rdn:" << rdn << endl;
    LG_EXTFS_D << "externalContext::createSubContext() filename:" << filename << endl;
    LG_EXTFS_D << "externalContext::createSubContext() fqfn:" << fqfn << endl;

    /* Make sure we are not overriding an existing file */
    if( !getHaveReadDir() )
    {
        read( false );
    }
    if( isSubContextBound( rdn ) )
    {
        fh_stringstream ss;
        ss << "Attempt to create a file when one already exists with that name!"
           << " rdn:" << rdn
           << " url:" << getURL()
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), this );
    }
    
    externalContext* child = new externalContext( this, rdn );
    child->setScriptName( getScriptName() );
    addNewChild( child );

    /* Write 0 bytes to the new file */
    try
    {
        fh_stringstream ss;
//        ss << "new file contents\n";
        child->writeStream(ss, 0);
    }
    catch( exception& e )
    {
        Remove( child );
        fh_stringstream ss;
        ss << "SL_SubCreate_file() can not make new file"
           << " fully qualified name:" << fqfn
           << " e:" << e.what()
           << endl;
        Throw_FerrisCreateSubContextFailed( tostr(ss), this );
    }
    
    return child;
}




void
externalContext::writeStream( fh_istream& ss, std::streamsize tellp )
{
    LG_EXTFS_D << "externalContext::writeStream()" << endl;
    
    ss->clear();
    ss->seekg(0);
    ss->clear();

    string Command     = "copyin";
    fh_runner r = getRunner( Command, false );
    r->setConnectStdIn( true );
    r->setSpawnFlags( GSpawnFlags(r->getSpawnFlags() | G_SPAWN_STDOUT_TO_DEV_NULL) );

    try
    {
        r->Run();
        
        /* ok */
        LG_EXTFS_D << "externalContext::writeStream() path:" << getDirPath()
                   << " exec/fork was ok."
                   << endl;
        {
            fh_ostream oss = r->getStdIn();
            copy( istreambuf_iterator<char>(ss), istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(oss));
            oss << flush;

//             cerr << "-------------------writeStream------------------------------" << endl;
//             ss->clear();
//             ss->seekg(0);
//             copy( istreambuf_iterator<char>(ss), istreambuf_iterator<char>(),
//                   ostreambuf_iterator<char>(cerr));
//             cerr << "------------------------------------------------------------" << endl;

            if( !oss )
            {
                fh_stringstream ss;
                ss << "error during copyin function for url:" << getURL() << endl;
                Throw_getIOStreamCloseUpdateFailed(tostr(ss),this);
            }
        }

//        LG_EXTFS_D << "externalContext::writeStream() path:" << getDirPath()
        LG_EXTFS_D << "externalContext::writeStream() path:" << getDirPath()
                   << " getting exit status" << endl;
        
        int rc = r->getExitStatus();
        if( !rc )
        {
            return;
        }
        else
        {
            fh_stringstream ss;
            ss << "Exit code:" << rc
               << " script:" << getScriptName()
               << " for url:" << getURL()
               << endl;
            LG_EXTFS_D << tostr(ss) << endl;
            Throw_getIOStreamCloseUpdateFailed(tostr(ss),this);
        }
    }
    catch( exception& e )
    {
        /* fail */
        LG_EXTFS_D << "externalContext::writeStream() path:" << getDirPath()
                   << " exec/fork was a failure."
                   << endl;
        
        fh_stringstream ss;
        ss << "fork/exec failed..."
           << r->getErrorString();
        Throw_getIOStreamCloseUpdateFailed(tostr(ss),this);
    }
}


fh_iostream
externalContext::getIOStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           CanNotGetStream,
           exception)
{
    fh_istream filepipe = real_getIOStream();
    fh_stringstream ss;

    copy( istreambuf_iterator<char>(filepipe), istreambuf_iterator<char>(),
          ostreambuf_iterator<char>(ss));

    ss.getCloseSig().connect(sigc::mem_fun( *this, &externalContext::writeStream )); 
    return ss;
}


// externalContext*
// externalContext::create( const string& xdn )
// {
//     externalContext* ret = priv_CreateContext( this, monsterName( xdn ));
//     Insert( ret, false );
//     return ret;
// }



fh_context
externalContext::ensureCreated( const string& xdn )
{
    return ensureContextCreated( xdn );
    
//     LG_EXTFS_D << "ensureCreated xdn:" << xdn << endl;
    
//     if( canSplitPathAtStart( xdn ) )
//     {
//         pair<string,string> p = splitPathAtStart( xdn );
//         LG_EXTFS_D << "ensureCreated first :" << p.first  
//                    << "ensureCreated second:" << p.second << endl;

//         if( !isSubContextBound( p.first ) )
//         {
//             LG_EXTFS_D << "path:" << getDirPath()
//                        << " making fake dir for:" << p.first
//                        << endl;
            
//             create( p.first );
//         }
        
//         externalContext* c = (externalContext*)GetImpl(getSubContext( p.first ));
//         return c->ensureCreated(p.second);
//     }
//     else if( !isSubContextBound( xdn ) )
//     {
//         LG_EXTFS_D << "ensureCreated path:" << getDirPath()
//                    << " making fake file for xdn:" << xdn
//                    << endl;

//         return create( xdn );
//     }
    
//     LG_EXTFS_D << "ensureCreated using getSubContext() xdn:" << xdn << endl;
//     return getSubContext( xdn );
}


static bool isDirMode( std::string& s )
{
    bool ret = false;
    int len = s.length();
    if( len > 4 )
    {
        string::reverse_iterator si = s.rbegin();
        advance( si, 4 );
        return *si == '4';
    }
    return ret;
}
    

void
externalContext::addContext( DOMNode* theNode )
{
    DOMNamedNodeMap* nattrs = theNode->getAttributes();
    DOMNode*             na = nattrs->getNamedItem( X("name") );
    
    if( na == 0 )
    {
        LG_EXTFS_W << "addContext() found a node with no name, skipping it." << endl;
        return;
        
//         cerr << "externalContext::addContext( DOM_Node theNode ) na==0" << endl;
//         fh_stringstream ss;
//         ss << "Entry found with no name!" << endl;
//         Throw_CanNotReadContext( tostr(ss), 0 );
    }

    string dn = tostr( na->getNodeValue() );

    
    PrefixTrimmer pretrimmer;
    pretrimmer.push_back( "/" );
    dn = pretrimmer( dn );

    PostfixTrimmer posttrimmer;
    posttrimmer.push_back( "/" );
    dn = posttrimmer( dn );

    LG_EXTFS_D << "addContext() dn -->:" << dn << ":<--" << endl;

    
    fh_excontext ctx = this;
    if( dn.length() )
    {
        ctx = dynamic_cast<externalContext*>(GetImpl(ensureCreated( dn )));
    }
    LG_EXTFS_D << "addContext() dn2.this -->:" << getURL() << ":<--" << endl;
    LG_EXTFS_D << "addContext() dn2.ctx  -->:" << ctx->getURL() << ":<--" << endl;
    LG_EXTFS_D << "addContext() dn2.ctxp  -->:" << ctx->getParent()->getURL() << ":<--" << endl;

    for( int i=0; i<nattrs->getLength(); ++i )
    {
        DOMNode* current = nattrs->item( i );
        if( current != na )
        {
            string k = tostr( current->getNodeName() );
            string v = tostr( current->getNodeValue() );
            LG_EXTFS_D << "addContext() k:" << k << " v:" << v << endl;

            if( k == "size" )
            {
                stringstream ss;
                ss << v;
                ss >> ctx->m_size;
//                LG_EXTFS_D << "addContext() SIZE k:" << k << " v:" << v << " m_size:" << m_size << endl;
            }
            
            if( !isAttributeBound( k ) )
            {
                ctx->addAttribute( k, v, XSD_BASIC_STRING, true );
            }

            if( k == "protection-ls" )
            {
                LG_EXTFS_D << "addContext() ctx:" << ctx->getURL()
                           << " k:" << k << " v:" << v << endl;
                if( !v.empty() )
                {
                    if( v[0] == 'd' )
                        ctx->m_isDir = true;
                }
            }
            if( k == "protection" && !v.empty() )
            {
                LG_EXTFS_D << "Found protection isdir:" << isDirMode( v )
                           << " ctx:" << ctx->getURL() << endl;
                ctx->m_isDir = isDirMode( v );
            }
        }
    }
}



void
externalContext::addContext( const RecordData_t& rd )
{
    
    if( rd.end() == rd.find("name") )
    {
        fh_stringstream ss;
        ss << "Entry found with no name!" << endl;
        Throw_CanNotReadContext( tostr(ss), 0 );
    }

    string dn = rd.find("name")->second;

    PrefixTrimmer pretrimmer;
    pretrimmer.push_back( "/" );
    dn = pretrimmer( dn );

    PostfixTrimmer posttrimmer;
    posttrimmer.push_back( "/" );
    dn = posttrimmer( dn );

    LG_EXTFS_D << "addContext(rd) dn -->:" << dn << ":<--" << endl;

    fh_context ctx = this;
    if( dn.length() )
    {
        ctx = ensureCreated( dn );
    }

    for( RecordData_t::const_iterator iter = rd.begin();
         iter != rd.end(); ++iter )
    {
        if( iter->first != "name" )
        {
            const string& rdn = iter->first;
            
            if( !isAttributeBound( rdn ) )
            {
                ctx->addAttribute( rdn, iter->second, XSD_BASIC_STRING );
            }
        }
    }
}


void
externalContext::read( bool force )
{
//    force = true;
    _Base::read( force );
}

    


void
externalContext::priv_read()
{
    Factory::ensureXMLPlatformInitialized();

    LG_EXTFS_D << "externalContext::priv_read() path:" << getDirPath()
         << " name:" << getDirName()
         << " hassub:" << hasSubContexts()
         << " script:" << ScriptName
         << endl;

    if( !getDirName().length() )
    {
        LG_EXTFS_D << "reading root context" << endl;

        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }

    LG_EXTFS_D << "externalContext::priv_read() path:" << getDirPath() << endl;
    LG_EXTFS_D << "externalContext::priv_read() "
               << " this->path:" << getDirPath()
               << " mainExternal->path:" << getBaseContext()->getDirPath()
               << " fake:" << ( getBaseContext() != this )
               << endl;

    if( ScriptName=="ssh" )
    {
        // We are root or the user@machine node //
        if( !isParentBound() ) // || !getParent()->isParentBound() )
        {
            LG_EXTFS_D << "faking it for ssh" << endl;

            emitExistsEventForEachItemRAII _raii1( this );
            return;
        }
    }
    else if( getBaseContext() != this )
    {
        LG_EXTFS_D << "externalContext::priv_read() reading a fake child."
                   << " path:" << getDirPath()
                   << " isDir:" << isDir()
                   << endl;
        if( getSortedItems().begin() == getSortedItems().end() )
        {
            stringstream ss;
            ss << "external/FerrisNotReadableAsContext for path:" << getDirPath();
//            cerr << ss.str() << endl;
            Throw_FerrisNotReadableAsContext( tostr(ss), this );
        }

        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }

    LG_EXTFS_D << "reading for real path:" << getDirPath() << endl;

    EnsureStartStopReadingIsFiredRAII _raii1( this );
    AlreadyEmittedCacheRAII _raiiec( this );
    LG_EXTFS_D << "externalContext::priv_read(1) path:" << getDirPath() << endl;
    LG_EXTFS_D << "externalContext::priv_read(1) name:" << getDirName() << endl;

    fh_runner r = getRunner( "list", false );

    fh_stringstream xdocss;
    try
    {
        r->Run();
        fh_istream ss = r->getStdOut();
        
        /* ok */
        LG_EXTFS_D << "externalContext::priv_read() path:" << getDirPath()
                   << " read was ok."
                   << endl;

//         for( string s; getline( ss, s );  )
//             xdocss << s << endl;
        copy( istreambuf_iterator<char>(ss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(xdocss));

        
        LG_EXTFS_D << "externalContext::priv_read() path:" << getDirPath()
                   << " xdocss:" << tostr(xdocss)
                   << endl;
        
        fh_domdoc      doc = Factory::StreamToDOM( xdocss );
        DOMNodeList* nodes = doc->getElementsByTagName( X("context") );
        LG_EXTFS_D << "have node count:" << nodes->getLength() << endl;
        for( int i=0; i < nodes->getLength(); ++i )
        {
            DOMNode* n = nodes->item( i );
            addContext( n );
        }

        if( int rc = r->getExitStatus() )
        {
            fh_stringstream ss;
            ss << "Exit code:" << rc
               << " script:" << getScriptName()
               << " for url:" << getURL()
               << endl;
            LG_EXTFS_D << tostr(ss) << endl;
            Throw_CanNotReadContext(tostr(ss), 0);
        }


        LG_EXTFS_D << "READ IS DONE" << endl;
//        dumpOutItems();

        bool failed = true;
        int rc=0;
        
        try
        {
            int rc = r->getExitStatus();
            LG_EXTFS_D << "getExitStatus() rc:" << rc << endl;

            if( 0 != WEXITSTATUS(rc) )
            {
                fh_stringstream ss;
                ss << "Child execution failed with return code:" << WEXITSTATUS(rc)
                   << " script was:" << getScriptName()
                   << endl;
                Throw_CanNotReadContext(tostr(ss),0);
            }
            
        }
        catch( FerrisWaitTimedOut& e )
        {
            fh_stringstream ss;
            ss << "Child execution failed, wait too long"
               << " Script was:" << getScriptName()
               << endl;
            Throw_CanNotReadContext(tostr(ss),0);
        }
    }
    catch( exception& e )
    {
        /* fail */
        LG_EXTFS_D << "externalContext::priv_read() path:" << getDirPath()
                   << " read was a failure."
                   << endl;

        BackTrace();
        fh_stringstream ss;
        ss << "read() failed url:" << getURL()
           << " e:" << e.what()
           << endl
           << endl
           << " doc:" << tostr( xdocss ) << endl;
        Throw_CanNotGetStream( tostr(ss), this );
    }

    LG_EXTFS_D << "externalContext::priv_read(done) path:" << getDirPath() << endl;
}

    






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        try
        {
            static externalContext c(0,"/");
            string root = rf->getInfo( RootContextFactory::ROOT );
            string path = rf->getInfo( RootContextFactory::PATH );
            string ScriptName  = "";

            c.AddRef();
            c.AddRef();
            c.AddRef();
        

            LG_EXTFS_D << "libexternal brew() path:" << path << " root:" << root
                       << endl;

            if( string::npos != root.find(":") )
            {
                ScriptName = root.substr(0,root.find(":"));
            }

            if( !ScriptName.length() && string::npos != path.find(":") )
            {
                ScriptName = path.substr(0,path.find(":"));
            }

            if( rf->getContextClass() == "ssh" )
            {
                ScriptName = "ssh";
                LG_EXTFS_D << "have ssh mount. path:" << path << endl;

                if( root != "/" )
                {
                    fh_stringstream ss;
                    ss << root << "/" << path;
                    rf->AddInfo( RootContextFactory::PATH, tostr(ss) );
                    path = tostr(ss);
                }


                PrefixTrimmer pretrimmer;
                pretrimmer.push_back( "/" );
                path = pretrimmer( path );
            
                LG_EXTFS_D << "path:" << path << endl;
                LG_EXTFS_D << "----------------1-------------------" << endl;
            
                /*
                 * a hack to make the requested path exist without checking
                 * if it does on the server
                 */
                {
                    fh_stringstream ss;
                    string s;
                    externalContext* ctx = &c; //dynamic_cast<externalContext*>(c);

                    LG_EXTFS_D << "Brew() root:" << root << endl;
                    ss << path;
                    for( ; getline( ss, s, '/' ); )
                    {
                        LG_EXTFS_D << "for() s:" << s << " ctx:" << ctx->getDirPath() << endl;
                        externalContext* child = new externalContext( ctx, s );
                        child->setScriptName( ScriptName );
                        ctx->addNewChild( child );
                        ctx = child;
                    }

                    LG_EXTFS_D << "break out, ctx:" << ctx->getDirPath() << endl;
//                 ctx->read();
//                 ctx->dumpOutItems();
                }

                LG_EXTFS_D << "----------------2-------------------" << endl;

//             fh_excontext ret = c.makeContext( 0, root );
//             ret->setScriptName( ScriptName );
//             ret->read();
//             return GetImpl(ret);

                fh_excontext ret = &c;
                return GetImpl(ret);
            }

            if( ScriptName.empty() )
            {
                fh_context r = Resolve( root );
                ScriptName = emap::getScriptName( r );
                //ScriptName = getScriptName();
            }
            
            LG_EXTFS_D << "libexternal brew() path:" << path << " root:" << root
                       << " ScriptName:" << ScriptName
                       << endl;

            
            if( ScriptName.length() )
            {
                fh_excontext ret = c.makeContext( 0, root );
                ret->setScriptName( ScriptName );
                ret->read();
                return GetImpl(ret);
            }
            else
            {
                fh_stringstream ss;
                ss << "libexternal: no script found that can handle this datatype."
                   << " path:" << path
                   << endl;
                Throw_RootContextCreationFailed( tostr(ss), 0 );
            }
        
        
            fh_context ret = c.CreateContext( 0, root );
            return ret;
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << ""
               << e.what()
               << endl;
            Throw_RootContextCreationFailed( tostr(ss), 0 );
        }
    }
}








 
};
