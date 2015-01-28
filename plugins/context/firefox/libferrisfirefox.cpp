/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisfirefox.cpp,v 1.7 2010/09/24 21:31:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>

#include <algorithm>
#include <numeric>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <config.h>

using namespace std;

namespace Ferris
{
    using namespace XML;
    typedef list< pair< string, string > > strstrlist_t;
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void sockwrite( int fd, const std::string& s )
    {
        LG_FIREFOX_D << "sockwrite(begin) fd:" << fd << " data:" << s << endl;

        write( fd, s.c_str(), s.length() );
        fsync( fd );
        
        LG_FIREFOX_D << "sockwrite(end) fd:" << fd << " data:" << s << endl;
    }
    string sockread( int fd )
    {
        LG_FIREFOX_D << "sockread(begin) fd:" << fd << endl;

        char buf[1024];
        buf[0] = '\0';
        int nread = ::read( fd, buf, sizeof(buf)-1 );
        if( nread )
            buf[nread] = '\0';

        LG_FIREFOX_D << "sockread(end) read:" << nread << " fd:" << fd << endl;
        return buf;
    }

    struct SocketHolder
    {
        int m_fd;
        SocketHolder( int fd )
            :
            m_fd( fd )
            {
            }
        ~SocketHolder()
            {
                if( m_fd )
                    close( m_fd );
            }
        operator int ()
            {
                return m_fd;
            }
    };
    
    int makeClientSocket( string& errormsg )
    {
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(7111);
        sa.sin_addr.s_addr = inet_addr("127.0.0.1");

        int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
        if( sockfd < 0 )
        {
            stringstream ss;
            ss << "Can not connect with running firefox process!" << endl;
            cerr << ss.str();
            return 0;
        }
        
        LG_FIREFOX_D << "makeClientSocket() calling connect..." << endl;
        int rc = connect( sockfd, (const sockaddr*)&sa, sizeof(sa) );
        if( rc < 0 )
        {
            stringstream ss;
            ss << "Can not connect with running firefox process!" << endl;
            perror( "reason" );
            cerr << ss.str();
            close( sockfd );
            return 0;
        }
        
        LG_FIREFOX_D << "makeClientSocket() have socket! " << endl;
        return sockfd;
    }
    
    string g_makeAbsoluteURL( string& base, string& earl )
    {
        if( starts_with( earl, "/" ) )
            return earl;
        
        string le = tolowerstr()( base );
        string prefix = base;
        
        int e = le.rfind('/');
        if( e != string::npos )
        {
            prefix = prefix.substr( 0, e );
        }
                
        return prefix + "/" + earl;
    }
    
    class FERRISEXP_CTXPLUGIN firefoxDOMContext
        :
        public StateLessEAHolder< firefoxDOMContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< firefoxDOMContext, FakeInternalContext > _Base;
        typedef firefoxDOMContext _Self;

        fh_context m_delegate;
        
        string getTitle();

        void
        priv_read()
            {
                LG_FIREFOX_D << "priv_read() url:" << getURL() << endl;

                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    string errormsg = "";
                    SocketHolder sockfd = makeClientSocket( errormsg );

                    if( !sockfd )
                        Throw_CanNotReadContext( errormsg, this );

                    LG_FIREFOX_D << "priv_read() 3" << endl;
                    stringstream cmdss;
                    cmdss << "write-dom " << getTitle() << endl;
                    LG_FIREFOX_D << "priv_read() 4" << endl;
                    sockwrite( sockfd, cmdss.str() );
                    LG_FIREFOX_D << "priv_read() 5" << endl;
                    if( sockread( sockfd ) == "OK" )
                    {
                        LG_FIREFOX_D << "priv_read() 6" << endl;
                        
                        fh_ifstream iss("~/.ferris/tmp/mount-firefox/libferris-moz-reply-export-dom");

                        //
                        // Xerces-C seems very picky if there is a doctype PI.
                        // strip it out.
                        //
                        string xml = StreamToString( iss );
                        int doctypeoffset = xml.find("<!DOCTYPE");
                        if( doctypeoffset == string::npos )
                            doctypeoffset = xml.find("<!doctype");
                        if( doctypeoffset != string::npos )
                        {
                            stringstream ss;
                            ss << xml.substr( 0, doctypeoffset );
                            int quoted=0;
                            while( true )
                            {
                                if( xml[ doctypeoffset ] != '>' )
                                    ++doctypeoffset;
                                if( xml[ doctypeoffset ] == '"' )
                                    quoted = !quoted;
                                if( !quoted && xml[ doctypeoffset ] == '>' )
                                    break;
                            }
                            ss << xml.substr( doctypeoffset+1 );
                            xml = ss.str();
                        }
                        
//                        fh_domdoc dom = Factory::StreamToDOM( iss );
                        fh_domdoc dom = Factory::StringToDOM( xml );
                        string forcedPrefix = getURL();
                        fh_context delegate = Factory::mountDOM( dom, forcedPrefix );

                        m_delegate = delegate;

                        for( Context::iterator iter = m_delegate->begin();
                             iter != m_delegate->end(); ++iter )
                        {
                            fh_VirtualSoftlinkContext child = new VirtualSoftlinkContext( this, *iter );
                            addNewChild( child );
                        }
                    }
                   LG_FIREFOX_D << "priv_read() closing" << endl;
                }
            }
        
        
    public:

        firefoxDOMContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_delegate( 0 )
            {
                createStateLessAttributes( true );
            }
        
    };
    FERRIS_SMARTPTR( firefoxDOMContext, fh_firefoxDOMContext );


    class FERRISEXP_CTXPLUGIN firefoxImageContext
        :
        public StateLessEAHolder< firefoxImageContext, leafContext >
    {
        typedef StateLessEAHolder< firefoxImageContext, leafContext > _Base;
        typedef firefoxImageContext _Self;

        string m_earl;

        virtual std::string priv_getRecommendedEA()
            {
                return "name,earl";
            }
        virtual std::string getRecommendedEA()
            {
                return "name,earl";
            }
        
        
        static fh_istream SL_getEarl( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_earl;
                return ss;
            }
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "earl",  SL_getEarl,  XSD_BASIC_STRING );
                    _Base::createStateLessAttributes( true );
                }
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                LG_FIREFOX_D << "priv_getIStream() URL:" << m_earl << endl;
                
                fh_context c = Resolve( m_earl );
                return c->getIStream();
            }
        
    public:

        void setEarl( const std::string& earl )
            {
                m_earl = earl;
            }
        
        firefoxImageContext( Context* parent, const std::string& rdn, const std::string& earl = "" )
            :
            _Base( parent, rdn ),
            m_earl( earl )
            {
                createStateLessAttributes( true );
            }
        
    };
    FERRIS_CTX_SMARTPTR( firefoxImageContext, fh_firefoxImageContext );
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class firefoxBufferContext;
    FERRIS_CTX_SMARTPTR( firefoxBufferContext, fh_firefoxBufferContext );
    
    class FERRISEXP_CTXPLUGIN firefoxBufferContext
        :
        public StateLessEAHolder< firefoxBufferContext,
                                  ParentPointingTreeContext< firefoxBufferContext,
                                                             leafContext > >
    {
        typedef StateLessEAHolder< firefoxBufferContext,
                                   ParentPointingTreeContext< firefoxBufferContext,
                                                              leafContext > > _Base;
        typedef firefoxBufferContext _Self;

        string m_earl;
        string m_title;
        
    protected:

        virtual std::string priv_getRecommendedEA()
            {
                return "name,title,earl";
            }
        virtual std::string getRecommendedEA()
            {
                return "name,title,earl";
            }
        
        static fh_istream SL_getTitle( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_title;
                return ss;
            }
        static fh_istream SL_getEarl( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_earl;
                return ss;
            }
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "title", SL_getTitle, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "earl",  SL_getEarl,  XSD_BASIC_STRING );
                    _Base::createStateLessAttributes( true );
                }
            }

        ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::binary    ;
            }

        string getBufferName()
            {
                return m_title;
            }
        
        
//         virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
//             throw (FerrisParentNotSetError,
//                    CanNotGetStream,
//                    exception)
//             {

//                 fh_runner r = new Runner();
//                 r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
//                                                | G_SPAWN_STDERR_TO_DEV_NULL
//                                                | G_SPAWN_SEARCH_PATH) );
//                 r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() | (G_SPAWN_DO_NOT_REAP_CHILD)));
//                 stringstream cmdss;
//                 cmdss << "gnuclient -batch -eval '(libferris-export-buffer \"" << getBufferName() << "\" )'";
//                 LG_FIREFOX_D << "cmd:" << tostr(cmdss) << endl;
//                 r->setCommandLine( cmdss.str() );
//                 r->Run();
//                 gint e = r->getExitStatus();

//                 fh_ifstream ret( "~/.ferris/tmp/libferris-xemacs-export" );
//                 return ret;
//             }

        string makeAbsoluteURL( string& earl )
            {
                return g_makeAbsoluteURL( m_earl, earl );
            }

        virtual void priv_read();

    public:

        void clearContext()
            {
                _Base::clearContext();
            }
        
        
        fh_firefoxBufferContext ensureContextCreated( const std::string& xdn, bool created = false )
            {
                return _Base::ensureContextCreated( xdn, created );
            }
        
        void constructObject( const std::string& earl,
                              const std::string& title,
                              stringset_t& images,
                              strstrlist_t& links )
            {
                m_earl = earl;
                m_title = title;

                firefoxDOMContext* dc = 0;
                dc = priv_ensureSubContext( "dom", dc );

                //
                // A context for each image on the page.
                //
                firefoxDOMContext* imagec = 0;
                imagec = priv_ensureSubContext( "images", imagec );
                
                for( stringset_t::const_iterator si = images.begin(); si!=images.end(); ++si )
                {
                    string earl = *si;
                    string rdn = earl;
                    int rdne = earl.rfind('/');
                    if( rdne != string::npos )
                    {
                        ++rdne;
                        rdn = earl.substr( rdne );
                    }
                    LG_FIREFOX_D << "rdn:" << rdn << " image:" << earl << endl;

                    earl = makeAbsoluteURL( earl );

                    firefoxImageContext* cc = 0;
                    cc = imagec->priv_ensureSubContext( rdn, cc );
                    cc->setEarl( earl );
//                     fh_firefoxImageContext dc = new firefoxImageContext( GetImpl(imagec), rdn, earl );
//                     imagec->addNewChild( dc );
                }

                //
                // A new context for each link.
                //
                firefoxDOMContext* linksc = 0;
                linksc = priv_ensureSubContext( "links", linksc );
                
                LG_FIREFOX_D << "links.size:" << links.size() << endl;
                for( strstrlist_t::const_iterator mi = links.begin(); mi!=links.end(); ++mi )
                {
                    string title = mi->first;
                    if( title.empty() )
                        title = "unknown";
                    title = linksc->monsterName( title );
                    string earl  = mi->second;

                    earl = makeAbsoluteURL( earl );
                    LG_FIREFOX_D << "Resolve() title:" << title << " earl:" << earl << endl;

                    firefoxImageContext* cc = 0;
                    cc = linksc->priv_ensureSubContext( title, cc );
                    cc->setEarl( earl );
                    
//                     fh_firefoxImageContext dc = new firefoxImageContext( GetImpl(linksc), title, earl );
//                     linksc->addNewChild( dc );
                }
            }
        
                

        firefoxBufferContext( Context* parent = 0,
                              const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }

        string getTitle() const
            {
                return m_title;
            }
        
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN firefoxInstanceContext
        :
        public StateLessEAHolder< firefoxInstanceContext, FakeInternalContext >
    {
        typedef firefoxInstanceContext _Self;
        typedef StateLessEAHolder< firefoxInstanceContext, FakeInternalContext > _Base;

        fh_firefoxBufferContext m_byTitle;
        fh_firefoxBufferContext m_byURL;
        int m_readCallCount;
        
    protected:

        ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                fh_stringstream ret;
                ret->getCloseSig().connect( bind( sigc::mem_fun( *this, &_Self::OnStreamClosed ), m ));
                return ret;
            }

        virtual void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);

                char templatestr[100] = "/tmp/libferris-firefox-tmp-XXXXXX";
                {
                    int fd = mkstemp( templatestr );
                    if( fd == -1 )
                    {
                        stringstream ss;
                        ss << "temporary file creation failed";
                        Throw_getIOStreamCloseUpdateFailed( ss.str(), this );
                    }
                    int len = s.length();
                    
                    if( len != write( fd, s.c_str(), len )
                        || -1 == close( fd ) )
                    {
                        stringstream ss;
                        ss << "temporary file setup failed";
                        Throw_getIOStreamCloseUpdateFailed( ss.str(), this );
                    }
                }
                string pageURL = templatestr;
                
                
                
                string errormsg = "";
                SocketHolder sockfd = makeClientSocket( errormsg );
                
                if( !sockfd )
                    Throw_getIOStreamCloseUpdateFailed( errormsg, this );

                LG_FIREFOX_D << "OnStreamClosed() 1" << endl;
                stringstream cmdss;
                cmdss << "load-virtual-page " << pageURL << endl;
                sockwrite( sockfd, cmdss.str() );
                string reply = sockread( sockfd );
                if( reply == "OK" )
                {
                    LG_FIREFOX_D << "OnStreamClosed() ok" << endl;
                }
                else
                {
                    stringstream ss;
                    ss << "Failed to pass new page to firefox! reply:" << reply << endl;
                    Throw_getIOStreamCloseUpdateFailed( ss.str(), this );
                }
            }

        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                }
            }

        void
        priv_read()
            {
                LG_FIREFOX_D << "priv_read() url:" << getURL() << endl;

                if( m_readCallCount )
                    return;
                Util::ValueRestorer<int> _vr1( m_readCallCount, 1 );
                
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();

                typedef std::list< DOMElement* > nl_t;
                fh_ifstream iss( "~/.ferris/tmp/mount-firefox/libferris-firefox-metadata" );
                fh_domdoc   dom = Factory::StreamToDOM( iss );
                DOMNode* node = dom->getDocumentElement();
                nl_t nl = XML::getAllChildrenElements( node, "tab", true );
                LG_FIREFOX_D << "nl.size:" << nl.size() << endl;

                {
                    firefoxBufferContext* child = 0;
                    child = priv_ensureSubContext( "by-title", child );
                    m_byTitle = child;
                    child->clearContext();
                }
                {
                    firefoxBufferContext* child = 0;
                    child = priv_ensureSubContext( "by-url", child );
                    m_byURL = child;
                    child->clearContext();
                }
                LG_FIREFOX_D << "priv_read(2) url:" << getURL() << endl;
                
                for( nl_t::const_iterator ni = nl.begin(); ni != nl.end(); ++ni )
                {
                    DOMNode* n = *ni;
                    string earl  = ::Ferris::getAttribute( (DOMElement*)n, "href" );
                    string title = getChildText( getChildElement( n, "title" ) );

                    LG_FIREFOX_D << "-----------------------------------------" << endl;
                    LG_FIREFOX_D << "earl:" << earl << endl;
                    LG_FIREFOX_D << "title:" << title << endl;

                    stringset_t images;
                    {
                        DOMNode* inode = getChildElement( n, "images" );
                        nl_t il = XML::getAllChildrenElements( inode, "image", true );
                        for( nl_t::const_iterator ii = il.begin(); ii != il.end(); ++ii )
                        {
                            DOMNode* in = *ii;
                            string earl  = ::Ferris::getAttribute( (DOMElement*)in, "src" );
                            images.insert( earl );
                        }
                    }
                    
                    
                    strstrlist_t links;
                    {
                        DOMNode* inode = getChildElement( n, "links" );
                        nl_t il = XML::getAllChildrenElements( inode, "a", true );
                        for( nl_t::const_iterator ii = il.begin(); ii != il.end(); ++ii )
                        {
                            DOMNode* in = *ii;
                            string earl  = ::Ferris::getAttribute( (DOMElement*)in, "href" );
                            string title = getChildText( (DOMElement*)in );
                            links.push_back( make_pair( title, earl ) );

                            LG_FIREFOX_D << "link.title:" << title << " earl:" << earl << endl;
                        }
                    }

                    LG_FIREFOX_D << "priv_read(make by-title) url:" << getURL() << endl;
                    // by title
                    {
                        string rdn = Util::replace_all( title, '/', ' ' );
                        PostfixTrimmer ptrimmer;
                        ptrimmer.push_back( " " );
                        rdn = ptrimmer( rdn );
                        LG_FIREFOX_D << "priv_read(make by-title) rdn:" << rdn << endl;
                        fh_firefoxBufferContext newc = m_byTitle->ensureContextCreated( rdn );
                        newc->constructObject( earl, title, images, links );
                    }

                    LG_FIREFOX_D << "priv_read(make by-url) url:" << getURL() << endl;
                    // by URL
                    {
                        string rdn = earl;
                        LG_FIREFOX_D << "priv_read(make by-url) rdn:" << rdn << endl;
                        fh_firefoxBufferContext newc = m_byURL->ensureContextCreated( rdn );
                        newc->constructObject( earl, title, images, links );
                    }
                }
            }

        
    public:

        firefoxInstanceContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_byTitle( 0 ),
            m_byURL( 0 ),
            m_readCallCount( 0 )
            {
                createStateLessAttributes( true );
            }
        
    };
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN firefoxRootContext
        :
        public FakeInternalContext
    {
        typedef firefoxRootContext _Self;
        typedef FakeInternalContext _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_fcontext loc = new FakeInternalContext( this, "localhost" );
                    Insert( GetImpl(loc) );

                    string un = Shell::getUserName( getuid() );
                    if( !un.empty() )
                    {
                        fh_fcontext c = new firefoxInstanceContext( GetImpl(loc), un );
                        loc->addNewChild( c );
                    }
                }
            }

    public:

        firefoxRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        virtual ~firefoxRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }
        

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        firefoxRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                firefoxRootContext* ret = new firefoxRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
        
    };

    string firefoxDOMContext::getTitle()
    {
        firefoxBufferContext* p = dynamic_cast<firefoxBufferContext*>(getParent());
        return p->getTitle();
    }

    void
    firefoxBufferContext::priv_read()
    {
        if( isParentBound() )
        {
            if( dynamic_cast<firefoxInstanceContext*>( getParent() ) )
            {
                emitExistsEventForEachItemRAII _raii1( this );

//                cerr << "firefoxBufferContext::priv_read() calling parent->read()" << endl;
                getParent()->read();
                return;
            }
        }
        _Base::priv_read();
    }
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static firefoxRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_ANNODEX_ER << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
