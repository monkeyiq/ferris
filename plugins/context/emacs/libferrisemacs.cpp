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

    $Id: libferrisemacs.cpp,v 1.8 2010/09/24 21:31:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <FerrisLoki/loki/Functor.h>

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>
#include <Ferris/Runner.hh>

#include <algorithm>
#include <numeric>

#include <config.h>

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

#include <boost/spirit.hpp>
#include <boost/spirit/home/classic/utility/regex.hpp>
using namespace boost::spirit;

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;
#define _1 ::boost::lambda::_1
#define _2 ::boost::lambda::_2

using namespace std;


namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct FERRISEXP_DLLLOCAL Tramp
    {
        typedef Loki::Functor<
            void, LOKI_TYPELIST_2( const char*, const char* ) >
        SAction_t;
        mutable SAction_t SAction;
        typedef const char* IteratorT;
    
        Tramp( const SAction_t& SAction )
            :
            SAction( SAction )
            {
            }
        template <typename PointerToObj, typename PointerToMemFn>
        Tramp( const PointerToObj& pObj, PointerToMemFn pMemFn )
            :
            SAction( SAction_t( pObj, pMemFn ) )
            {
            }

        void operator()( IteratorT first, IteratorT last) const
            {
                SAction( first, last );
            }
    };

    class FERRISEXP_DLLLOCAL EmacsBufferListParser;
    class FERRISEXP_DLLLOCAL EmacsBuffer
    {
        friend class EmacsBufferListParser;

        string m_bufferName;
        long m_size;
        string m_mode;
        string m_bufferPath;
        bool m_modified;
        bool m_readonly;

    public:
    
        EmacsBuffer()
            :
            m_size( 0 ),
            m_modified( 0 ),
            m_readonly( 0 )
            {
            }
    
    
        string getBufferName() const
            {
                return m_bufferName;
            }
        long getSize() const
            {
                return m_size;
            }
        string getMode() const
            {
                return m_mode;
            }
        string getBufferPath() const
            {
                return m_bufferPath;
            }
        bool isModified() const
            {
                return m_modified;
            }
        bool isReadOnly() const
            {
                return m_readonly;
            }
    
    };


    class FERRISEXP_DLLLOCAL EmacsBufferListParser
    {
        typedef EmacsBufferListParser _Self;

    public:
        typedef list< EmacsBuffer > m_EmacsBufferList_t;
        m_EmacsBufferList_t getBufferList()
            {
                return m_EmacsBufferList;
            }
    
    private:
        m_EmacsBufferList_t m_EmacsBufferList;
        EmacsBuffer m_tb;
    
        typedef Tramp F_t;
        template < typename PointerToMemFn >
        F_t F( PointerToMemFn pMemFun )
            {
                return F_t( this, pMemFun );
            }
    
    public:

        EmacsBufferListParser()
            {
            }

        void start_new_buffer_f( const char* beg, const char* end )
            {
                m_EmacsBufferList.push_back( m_tb );
                m_tb = EmacsBuffer();
            }
    

        void get_modified_f( const char* beg, const char* end )
            {
                string v( beg, end );
                m_tb.m_modified = isTrue( v );
            }
        void get_size_f( const char* beg, const char* end )
            {
                string v( beg, end );
                m_tb.m_size = toint( v );
            }
        void get_mode_f( const char* beg, const char* end )
            {
                string v( beg, end );
                m_tb.m_mode = v;
            }
        void get_buffer_name_f( const char* beg, const char* end )
            {
                string v( beg, end );
                m_tb.m_bufferName = v;
            }
        void get_buffer_path_f( const char* beg, const char* end )
            {
                string v( beg, end );
                m_tb.m_bufferPath = v;
            }
    
        bool parse( fh_istream iss )
            {
                string tmp;
                getline( iss, tmp );
                getline( iss, tmp );
                string s = StreamToString( iss );
                typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
                typedef rule< scanners > R;

                R space_p = str_p(" ");
                R ws_p = space_p | regex_p("\t");
                R space_seperated_token_p = regex_p("[^ \n]+");
                R modified_p = str_p(".")[ F( &_Self::get_modified_f ) ];
                R special_p  = str_p("*");
                R readonly_p  = str_p("%");
                R size_p      = regex_p("[0-9]+")[ F( &_Self::get_size_f ) ];
                R mode_p      = regex_p("[^\t\n]+")[ F( &_Self::get_mode_f ) ];
                R buffer_path_p = regex_p("[^\n]+")[ F( &_Self::get_buffer_path_f ) ];

                R quoted_filename_p = regex_p("[^\"]+")[ F( &_Self::get_buffer_name_f ) ];
                R raw_filename_p = regex_p("[^ \t\n]+")[ F( &_Self::get_buffer_name_f ) ];
                R possibly_quoted_buffer_name_p =
                    ( raw_filename_p )
                    | ( str_p("\"") >> quoted_filename_p >> str_p("\"") );
            
                R main_line_p = (*(modified_p | special_p | readonly_p | space_p)
                                 >> possibly_quoted_buffer_name_p >> *ws_p
                                 >> *( size_p >> *ws_p
                                       >> mode_p >> *ws_p )
                                 >> *buffer_path_p >> str_p("\n") )[ F( &_Self::start_new_buffer_f ) ];
                R main_p = +( main_line_p );
                
                parse_info<> info = ::boost::spirit::parse( s.c_str(), main_p ); // , space_p );
                if (info.full)
                {
                    return true;
                }
                else
                {
                    stringstream ss;
                    ss << "Parsing failed" << endl
                       << "s:" << s << endl
                       << "stopped at:" << info.stop << " " << endl
                       << "char offset:" << ( info.stop - s.c_str() ) << endl;
                    LG_EMACS_W << tostr(ss);
                    cerr << tostr(ss);
                    return false;
                }
            }
    
    
    
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN emacsBufferContext
        :
        public StateLessEAHolder< emacsBufferContext, leafContext >
    {
        typedef StateLessEAHolder< emacsBufferContext, leafContext > _Base;
        typedef emacsBufferContext _Self;

        long m_size;
        string m_mode;
        string m_path;
        bool m_createdFromFerris;
        
    protected:

        virtual std::string priv_getRecommendedEA()
            {
                return "size,name,mode,buffer-path";
            }
        virtual std::string getRecommendedEA()
            {
                return "size,name,mode,buffer-path";
            }
        
        static fh_istream SL_getSize( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_size;
                return ss;
            }
        static fh_istream SL_getMode( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_mode;
                return ss;
            }
        static fh_istream SL_getPath( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << c->m_path;
                return ss;
            }
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "size", SL_getSize, XSD_BASIC_INT );
                    tryAddStateLessAttribute( "mode", SL_getMode, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "buffer-path", SL_getPath, XSD_BASIC_STRING );
                    _Base::createStateLessAttributes( true );
                }
            }

        ferris_ios::openmode getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    ios_base::trunc     |
                    ios_base::binary    ;
            }

        string getBufferName()
            {
                return getDirName();
            }
        
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {

                fh_runner r = new Runner();
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                               | G_SPAWN_STDERR_TO_DEV_NULL
                                               | G_SPAWN_SEARCH_PATH) );
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() | (G_SPAWN_DO_NOT_REAP_CHILD)));
                stringstream cmdss;
                cmdss << "ferris-emacsclient-function '(libferris-export-buffer \"" << getBufferName() << "\" )'";
                LG_EMACS_D << "cmd:" << tostr(cmdss) << endl;
                r->setCommandLine( cmdss.str() );
                r->Run();
                gint e = r->getExitStatus();

                fh_ifstream ret( "~/.ferris/tmp/libferris-xemacs-export" );
                return ret;
            }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception)
            {
                if( !m_createdFromFerris )
                {
                    stringstream ss;
                    ss << "Current support is only write once for files created by libferris. patches accepted!" << endl;
                    Throw_CanNotGetStream( ss.str(), this );
                }

                fh_stringstream ret;
                if( !(m & ios_base::trunc) )
                {
//                  ret << data;
                }
                ret->getCloseSig().connect( bind( sigc::mem_fun( *this, &_Self::OnStreamClosed ), m ));
                return ret;
            }

        virtual void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const std::string s = StreamToString(ss);
                {
                    fh_ofstream ofss( "/home/ben/.ferris/tmp/libferris-xemacs-import" );
                    ofss << s;
                }
                LG_EMACS_D << "OnStreamClosed() s:" << s << endl;

                fh_runner r = new Runner();
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                               | G_SPAWN_STDERR_TO_DEV_NULL
                                               | G_SPAWN_SEARCH_PATH) );
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() | (G_SPAWN_DO_NOT_REAP_CHILD)));
                stringstream cmdss;
                cmdss << "ferris-emacsclient-function "
                      << " ' (libferris-import-buffer \"" << getBufferName() << "\" \"\" )' ";
                LG_EMACS_D << "cmd:" << cmdss.str() << endl;
                
                r->setCommandLine( cmdss.str() );
                r->Run();
                gint e = r->getExitStatus();
            }
        
        
    public:

        void constructObject( long size,
                    const std::string& mode,
                    const std::string& path,
                    bool createdFromFerris = false )
            {
                m_size = size;
                m_mode = mode;
                m_path = path;
                m_createdFromFerris = createdFromFerris;
            }
        
        emacsBufferContext( Context* parent,
                            const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
    };
    FERRIS_CTX_SMARTPTR( emacsBufferContext, fh_emacsBufferContext );
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_CTXPLUGIN emacsInstanceContext
        :
        public StateLessEAHolder< emacsInstanceContext, FakeInternalContext >
    {
        typedef emacsInstanceContext _Self;
        typedef StateLessEAHolder< emacsInstanceContext, FakeInternalContext > _Base;

    protected:

        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    _Base::createStateLessAttributes( true );
                }
            }

        void priv_read_parse_bufferlist()
            {
                    fh_ifstream iss( "~/.ferris/tmp/libferris-xemacs-list" );

                    EmacsBufferListParser p;

                    p.parse( iss );

                    EmacsBufferListParser::m_EmacsBufferList_t bl = p.getBufferList();
                    LG_EMACS_D << "Buffer.count:" << bl.size() << endl;

                    for( EmacsBufferListParser::m_EmacsBufferList_t::const_iterator bi = bl.begin();
                         bi != bl.end(); ++bi )
                    {
                        string rdn = bi->getBufferName();
                        LG_EMACS_D << "Adding child rdn:" << rdn << endl;

                        emacsBufferContext* cc = 0;
                        cc = priv_ensureSubContext( rdn, cc );
//                         fh_context cc = priv_readSubContext( rdn, false );
//                         fh_emacsBufferContext child =
//                             dynamic_cast<emacsBufferContext*>(GetImpl(cc));
//                        cerr << "cc:" << toVoid(cc) << " child:" << toVoid(child) << endl;
                        cc->constructObject( bi->getSize(), bi->getMode(), bi->getBufferPath() );
                    }
            }

        virtual FakeInternalContext* priv_CreateContext( Context* parent, std::string rdn )
            {
                Context* ret = new emacsBufferContext( parent, rdn );
                return (FakeInternalContext*)ret;
            }

        void
        priv_read()
            {
                LG_EMACS_D << "priv_read() url:" << getURL() << endl;

                EnsureStartStopReadingIsFiredRAII _raii1( this );
                clearContext();

                fh_runner r = new Runner();
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                               | G_SPAWN_STDERR_TO_DEV_NULL
                                               | G_SPAWN_SEARCH_PATH) );
                r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() | (G_SPAWN_DO_NOT_REAP_CHILD)));
                stringstream cmdss;
                cmdss << "ferris-emacsclient-function '(libferris-list-buffers)'";
                r->setCommandLine( cmdss.str() );
                r->Run();
                gint e = r->getExitStatus();
                priv_read_parse_bufferlist();
                LG_EMACS_D << "Instance() priv_read. child count:" << getSubContextCount() << endl;
            }

        //
        // Short cut loading each dir unless absolutely needed.
        // Also tell emacs to create a new buffer if the user tries to getSubContext() for it.
        //
        fh_context
        priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
            {
                try
                {
                    LG_EMACS_D << "priv_getSubContext() p:" << getDirPath()
                               << " rdn:" << rdn
                               << endl;

                    if( priv_isSubContextBound( rdn ) )
                    {
                        LG_EMACS_D << "priv_getSubContext(bound already) p:" << getDirPath()
                                   << " rdn:" << rdn
                                   << endl;
                        return _Base::priv_getSubContext( rdn );
                    }

                    if( rdn.empty() )
                    {
                        fh_stringstream ss;
                        ss << "NoSuchSubContext no rdn given";
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }
                    else if( rdn[0] == '/' )
                    {
                        fh_stringstream ss;
                        ss << "NoSuchSubContext no files start with unescaped '/' as filename";
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }

                    string bufferName = rdn;
                    string bufferPath = bufferName;
                    LG_EMACS_D << "Telling emacs to create bufferName:" << bufferName
                               << " bufferPath:" << bufferPath << endl;

                    bool created = false;
                    bool createdFromFerris = true;
                    fh_emacsBufferContext child = new emacsBufferContext( this, rdn );
                    child->constructObject( 0, "Fundamental", "", createdFromFerris );
                    Insert( GetImpl( child ), created );
                    return child;
                }
                catch( NoSuchSubContext& e )
                {
                    throw e;
                }
                catch( exception& e )
                {
                    string s = e.what();
                    Throw_NoSuchSubContext( s, this );
                }
                catch(...)
                {}
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }
        
    public:

        emacsInstanceContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes( true );
            }
        
    };
    
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    class FERRISEXP_CTXPLUGIN emacsRootContext
        :
        public FakeInternalContext
    {
        typedef emacsRootContext _Self;
        typedef FakeInternalContext _Base;

    protected:

        void
        priv_read()
            {
                staticDirContentsRAII _raii1( this );
                
                if( empty() )
                {
                    FakeInternalContext* loc = 0;
                    loc = priv_ensureSubContext( "localhost", loc );
//                      fh_fcontext loc = new FakeInternalContext( this, "localhost" );
//                      Insert( GetImpl(loc) );
                    
                    string un = Shell::getUserName( getuid() );
                    if( !un.empty() )
                    {
                        emacsInstanceContext* cc = 0;
                        cc = loc->priv_ensureSubContext( un, cc );
                    }
                }
            }

    public:

        emacsRootContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        virtual ~emacsRootContext()
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
        emacsRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                emacsRootContext* ret = new emacsRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
        
    };
    
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
                static emacsRootContext c;
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
