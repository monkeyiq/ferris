/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2009 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.11 2009/04/18 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <FerrisContextPlugin.hh>
#include <FerrisKDE.hh>
#include <FerrisDOM.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>
#include <FerrisWebServices_private.hh>

#include "libferrisferrisrest_shared.hh"
#include <qjson/parser.h>

using namespace std;

//#define DEBUG cerr
#define DEBUG LG_FERRISREST_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };

    using namespace Ferrisrest;
    using XML::domnode_list_t;
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx = FakeInternalContext >
    class FERRISEXP_CTXPLUGIN FerrisrestContextBase
        :
        public ParentCtx
    {
      protected:
        
        bool   m_haveTriedToRead;
        time_t m_readTime;

        void priv_read();

        //
        // Short cut loading each dir unless absolutely needed.
        //
        fh_context priv_getSubContext( const string& rdn );
        
      public:
        FerrisrestContextBase( Context* parent, const std::string& rdn )
            : ParentCtx( parent, rdn )
            , m_haveTriedToRead( false )
            , m_readTime( 0 )
            {
            }
        virtual fh_ferrisrest getFerrisrest();
        virtual std::string getRemotePath();
        
      private:
        
    };


    /******************************/
    /******************************/
    /******************************/

    /******************************/
    /******************************/
    /******************************/
    
    class FERRISEXP_CTXPLUGIN FerrisrestContext
        :
        public StateLessEAHolder< FerrisrestContext, FerrisrestContextBase<> >
    {
        typedef StateLessEAHolder< FerrisrestContext, FerrisrestContextBase<> > _Base;
        typedef FerrisrestContext                     _Self;

        std::streamsize m_size;
        
        static fh_istream SL_getSize( FerrisrestContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_size;
            return ss;
        }
        
      public:

        FerrisrestContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_size( 0 )
        {
            tryAddStateLessAttribute( "size", SL_getSize, FXD_FILESIZE );
            createStateLessAttributes();
        }

        fh_iostream getIsActive( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << ( getStrAttr( c, "state", "" ) == "OK" );
                return ss;
            }

        std::string priv_getRecommendedEA()
        {
            static string rea = "name,id,active,function,width,height";
            return rea;
        }
        
        void setup( const DOMElement* e )
        {
            fh_ferrisrest zm = getFerrisrest();

            std::list< DOMElement* > keyvals = XML::getAllChildrenElements( (DOMNode*)e, "keyval" );
            for( std::list< DOMElement* >::iterator kviter = keyvals.begin();
                 kviter != keyvals.end(); ++kviter )
            {                        
                string key = ::Ferris::getAttribute( (DOMElement*)*kviter, "key" );
                string val = XML::getChildText( (DOMElement*)*kviter );
                addAttribute( key, val );
                DEBUG << "adding k:" << key << " v:" << val << endl;
                if( key == "size" )
                    m_size = toType<streamsize>(val);
            }
        }

        
      protected:

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    std::ios::in        |
                    ios_base::out       |
                    std::ios::out       |
                    std::ios::trunc     |
                    ios_base::binary    ;
            }

        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ret;
                stringmap_t args;
                args["path"]   = getRemotePath();
                args["method"] = "read";
                args["zz"] = "top";
                fh_ferrisrest zm = getFerrisrest();
                QNetworkReply* reply = zm->post( args );
                ret << tostr(reply->readAll());
                return ret;
            }

        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                ss->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::priv_OnStreamClosed ), m ));
                return ss;
            }

        void priv_OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                AdjustForOpenMode_Closing( ss, m, tellp );
                const string s = StreamToString(ss);

                stringmap_t args;
                args["path"]   = getRemotePath();
                args["method"] = "write";
                args["zz"]     = "top";
                fh_ferrisrest     zm = getFerrisrest();
                QNetworkReply* reply = zm->post( args, toba(s) );

                fh_domdoc dom = zm->toDOM( reply );
                DOMElement* di = dom->getDocumentElement();
                if( di && tostr(di->getNodeName()) == "error" )
                {
                    string e = getStrSubCtx( di, "desc", "" );
                    Throw_CanNotGetStream( e, 0 );
                }
            }
        
        
      private:
        
    };
    
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN FerrisrestServerContext
        :
        public FerrisrestContextBase< FakeInternalContext >
    {
        typedef FerrisrestContextBase< FakeInternalContext >  _Base;
        typedef FerrisrestServerContext                       _Self;
        fh_ferrisrest m_zm;
        userpass_t getUserPass( const std::string& server );
        
        
        
    public:

        FerrisrestServerContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_zm( Factory::getFerrisrest( rdn ) )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
                getFerrisrest()->ensureAuthenticated();
                
            }

        fh_ferrisrest getFerrisrest()
        {
            return m_zm;
        }
        virtual std::string getRemotePath()
        {
            return "/";
        }
        
        
        
//         void priv_read()
//             {
//                 DEBUG << "priv_read() url:" << getURL()
//                       << " m_haveTriedToRead:" << m_haveTriedToRead
//                       << " have read:" << getHaveReadDir()
//                       << " subc.sz:" << getSubContextCount()
//                       << endl;

// //                if( getSubContextCount() )
//                 time_t now = Time::getTime();
//                 if( m_readTime && now - m_readTime < 60 )
//                 {
//                     EnsureStartStopReadingIsFiredRAII _raii1( this );
//                     emitExistsEventForEachItemRAII    _raii2( this );
//                     return;
//                 }

//                 m_readTime = now;
//                 stringmap_t args;
//                 args["method"]    = "ls";
//                 args["path"]      = "/";
//                 args["eanames"]   = "url,name,size,size-human-readable,mtime,atime,ctime,mtime-display,ctime-display,group-owner-number,group-owner-name,user-owner-number,user-owner-name,protection-raw,protection-ls";
//                 args["format"]    = "xml";
//                 args["zz"]        = "xx";
                
//                 fh_ferrisrest zm = getFerrisrest();
//                 EnsureStartStopReadingIsFiredRAII _raii1( this );
//                 QNetworkReply* reply = zm->post( args );
//                 fh_domdoc dom = zm->toDOM( reply );
//                 XML::DOMElementList_t del = XML::evalXPathToElements(  dom, "/context/context" );
//                 DEBUG << "children.sz:" << del.size() << endl;
//                 for( XML::DOMElementList_t::iterator di = del.begin(); di != del.end(); ++di )
//                 {
//                     string rdn;

//                     std::list< DOMElement* > keyvals = XML::getAllChildrenElements( (DOMNode*)*di, "keyval" );
//                     for( std::list< DOMElement* >::iterator kviter = keyvals.begin();
//                          kviter != keyvals.end(); ++kviter )
//                     {                        
//                         string key = ::Ferris::getAttribute( (DOMElement*)*kviter, "key" );
//                         string val = XML::getChildText( (DOMElement*)*kviter );
//                         if( key == "name" )
//                             rdn = val;
//                     }
                    
//                     DEBUG << "name:" << rdn << endl;
//                     FerrisrestContext* c = 0;
//                     c = priv_ensureSubContext( rdn, c );
//                     c->setup( *di );
//                 }
//             }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    
    class FERRISEXP_CTXPLUGIN FerrisrestRootContext
        :
        public networkRootContext< FerrisrestServerContext >
    {
        typedef networkRootContext< FerrisrestServerContext > _Base;

      public:

        FerrisrestRootContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            {
                tryAugmentLocalhostNames();
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }

    };


    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx >
    void
    FerrisrestContextBase< ParentCtx >::priv_read()
            {
                DEBUG << "priv_read() url:" << this->getURL()
                      << " m_haveTriedToRead:" << this->m_haveTriedToRead
                      << " have read:" << this->getHaveReadDir()
                      << " subc.sz:" << this->getSubContextCount()
                      << endl;

//                if( getSubContextCount() )
                time_t now = Time::getTime();
                if( this->m_readTime && now - this->m_readTime < 60 )
                {
                    Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
                    Context::emitExistsEventForEachItemRAII    _raii2( this );
                    return;
                }

                this->m_readTime = now;
                stringmap_t args;
                args["method"]    = "ls";
                args["path"]      = getRemotePath();
                args["eanames"]   = "url,name,size,size-human-readable,mtime,atime,ctime,mtime-display,ctime-display,group-owner-number,group-owner-name,user-owner-number,user-owner-name,protection-raw,protection-ls";
                args["format"]    = "xml";
                args["zz"]        = "xx";
                
                fh_ferrisrest zm = getFerrisrest();
                Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
                QNetworkReply* reply = zm->post( args );
                fh_domdoc dom = zm->toDOM( reply );
                XML::DOMElementList_t del = XML::evalXPathToElements(  dom, "/context/context" );
                DEBUG << "children.sz:" << del.size() << endl;
                for( XML::DOMElementList_t::iterator di = del.begin(); di != del.end(); ++di )
                {
                    string rdn;

                    std::list< DOMElement* > keyvals = XML::getAllChildrenElements( (DOMNode*)*di, "keyval" );
                    for( std::list< DOMElement* >::iterator kviter = keyvals.begin();
                         kviter != keyvals.end(); ++kviter )
                    {                        
                        string key = ::Ferris::getAttribute( (DOMElement*)*kviter, "key" );
                        string val = XML::getChildText( (DOMElement*)*kviter );
                        if( key == "name" )
                            rdn = val;
                    }
                    
                    DEBUG << "name:" << rdn << endl;
                    FerrisrestContext* c = 0;
                    c = this->priv_ensureSubContext( rdn, c );
                    c->setup( *di );
                }
            }

    template < class ParentCtx >
    fh_context
    FerrisrestContextBase< ParentCtx >::priv_getSubContext( const string& rdn )
    {
        try
        {
            DEBUG << "priv_getSubContext() p:" << this->getDirPath()
                  << " rdn:" << rdn
                  << endl;
            Context::Items_t::iterator isSubContextBoundCache;
            if( this->priv_isSubContextBound( rdn, isSubContextBoundCache ) )
            {
                DEBUG << "priv_getSubContext(bound already) p:" << this->getDirPath()
                      << " rdn:" << rdn
                      << endl;
                return *isSubContextBoundCache;
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

            // ok
            stringmap_t args;
            args["method"]    = "stat";
            args["path"]      = getRemotePath() + "/" + rdn;
            args["eanames"]   = "url,name,size,size-human-readable,mtime,atime,ctime,mtime-display,ctime-display,group-owner-number,group-owner-name,user-owner-number,user-owner-name,protection-raw,protection-ls";
            args["format"]    = "xml";
            args["zz"]        = "top";
                
            fh_ferrisrest zm = getFerrisrest();
            Context::EnsureStartStopReadingIsFiredRAII _raii1( this );
            QNetworkReply* reply = zm->post( args );
            fh_domdoc dom = zm->toDOM( reply );
            DOMElement* di = dom->getDocumentElement();
            if( di && tostr(di->getNodeName()) == "result" )
            {
                FerrisrestContext* c = 0;
                c = this->priv_ensureSubContext( rdn, c );
                c->setup( di );
                return c;
            }
        }
        catch( NoSuchSubContext& e )
        {
            throw e;
        }
        catch( exception& e )
        {
            string s = e.what();
//            cerr << "NativeContext::priv_getSubContext() e:" << e.what() << endl;
            Throw_NoSuchSubContext( s, this );
        }
        catch(...)
        {}
        fh_stringstream ss;
        ss << "NoSuchSubContext:" << rdn;
        Throw_NoSuchSubContext( tostr(ss), this );
    }
    
            
            
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx >
    fh_ferrisrest FerrisrestContextBase<ParentCtx>::getFerrisrest()
    {
        FerrisrestServerContext* p = 0;
        p = ParentCtx::getFirstParentOfContextClass( p );
        return p->getFerrisrest();
    }

    template < class ParentCtx >
    std::string FerrisrestContextBase<ParentCtx>::getRemotePath()
        {
            FerrisrestServerContext* p = 0;
            p = ParentCtx::getFirstParentOfContextClass( p );

            string fullearl   = this->getURL();
            string parentearl = p->getURL();
            string ret = fullearl.substr( parentearl.length() );
            DEBUG << "fullearl     :" << fullearl << endl
                  << "   parentearl:" << parentearl << endl
                  << "          ret:" << ret << endl;
            return ret;
        }
    

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                DEBUG << "Brew()" << endl;

                static FerrisrestRootContext* c = 0;
                if( !c )
                {
                    c = new FerrisrestRootContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                    DEBUG << "hi there" << endl;
                }
                fh_context ret = c;

                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
