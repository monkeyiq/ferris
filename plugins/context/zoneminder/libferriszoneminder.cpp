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

#include "libferriszoneminder_shared.hh"
#include <qjson/parser.h>

using namespace std;

//#define DEBUG cerr
#define DEBUG LG_ZONEMINDER_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    using namespace Zoneminder;

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx >
    class FERRISEXP_CTXPLUGIN ZoneminderContextBase
        :
        public ParentCtx
    {
      public:
        ZoneminderContextBase( Context* parent, const std::string& rdn )
            :
            ParentCtx( parent, rdn )
            {
            }
        fh_zoneminder getZoneminder();

      protected:

      private:
        
    };


    /******************************/
    /******************************/
    /******************************/

    class FERRISEXP_CTXPLUGIN ZoneminderEventFrameContext
        :
        public ZoneminderContextBase< leafContext >
    {
        typedef ZoneminderContextBase< leafContext > _Base;
        typedef ZoneminderEventFrameContext          _Self;
        int m_frame;
        
      public:
        ZoneminderEventFrameContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_frame(0)
            {
            }
        void setup( int frame )
        {
            fh_zoneminder zm = getZoneminder();
            m_frame = frame;
        }

        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                stringmap_t args;
                args["skin"]   = "xml";
                args["view"]   = "actions";
                args["action"] = "vframe";
                args["eid"]    = getStrAttr( getParent(), "id", "" );
                args["frame"]  = tostr(m_frame);
                args["foo"]    = "bar";
                
                fh_zoneminder     zm = getZoneminder();
                QNetworkReply* reply = zm->post( args );

                QUrl possibleRedirectUrl =
                    reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                
                if( !tostr(possibleRedirectUrl.toString()).empty() )
                {
                    std::string earl = tostr(possibleRedirectUrl.toString());
                    DEBUG << "1 redirected to:" << earl << endl;
                    if( starts_with( earl, "./" ))
                    {
                        earl = zm->getBaseURI() + earl;
                    }
                    DEBUG << "2 redirected to:" << earl << endl;
                    
                    QNetworkRequest request;
                    request.setUrl( QUrl( earl.c_str() ));
                    reply = zm->getQManager()->post( request, QByteArray() );
                    wait( reply );
                }
                
                fh_stringstream ret;
                ret << tostr( reply->readAll() );
                return ret;
            }
        
      protected:

        
      private:
        
    };

    /******************************/
    /******************************/
    /******************************/
    
    class FERRISEXP_CTXPLUGIN ZoneminderEventContext
        :
        public ZoneminderContextBase< FakeInternalContext >
    {
        typedef ZoneminderContextBase< FakeInternalContext > _Base;
        typedef ZoneminderEventContext                       _Self;
        
      public:
        ZoneminderEventContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
        void setup( const DOMElement* e )
        {
            fh_zoneminder zm = getZoneminder();

            addAttribute( "id",            getStrSubCtx( e, "ID" ) );
            addAttribute( "event-name",    getStrSubCtx( e, "NAME" ) );
            addAttribute( "duration",      getStrSubCtx( e, "DURATION" ) );
            addAttribute( "frames",        getStrSubCtx( e, "FRAMES" ) );
            addAttribute( "fps",           getStrSubCtx( e, "FPS" ) );
            addAttribute( "score-total",   getStrSubCtx( e, "TOTSCORE" ) );
            addAttribute( "score-average", getStrSubCtx( e, "AVGSCORE" ) );
            addAttribute( "score-max",     getStrSubCtx( e, "MAXSCORE" ) );

            int totalFrames = toint( getStrSubCtx( e, "FRAMES" ) );
            int fps         = toint( getStrSubCtx( e, "FPS" ) );
            DEBUG << "totalFrames:" << totalFrames << endl;
            DEBUG << "fps:" << fps << endl;
            if( !fps ) fps = 1;
            
            int skip = totalFrames / fps;
            for( int i = 0; i < totalFrames; i+=skip )
            {
                stringstream ss;
                ss << "frame-" << i << ".jpg";
                string rdn = ss.str();
                
                ZoneminderEventFrameContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->setup( i );
            }
        }
        
      protected:

        
      private:
        
    };



    /******************************/
    /******************************/
    /******************************/

    class FERRISEXP_CTXPLUGIN ZoneminderMonitorContextBase
        :
        public ZoneminderContextBase< FakeInternalContext >
    {
        typedef ZoneminderContextBase< FakeInternalContext > _Base;
        typedef ZoneminderMonitorContextBase                 _Self;
        
      public:

        ZoneminderMonitorContextBase( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
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
        std::string getID()
        {
            string ret = getStrAttr( this, "id", "" );
            if( ret.empty() && isParentBound() )
                ret = getStrAttr( getParent(), "id", "" );
            return ret;
        }
        
      protected:

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    std::ios::in        |
                    ios_base::binary    ;
            }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                stringmap_t args;
                args["skin"] = "xml";
                args["view"] = "actions";
                args["action"] = "feed";
                args["monitor"] = getID();
                args["vcodec"] = "mjpeg";
                args["fps"] = "0";
                args["foo"] = "bar";
                
                fh_zoneminder zm = getZoneminder();
                QNetworkReply* reply = zm->post( args );
                string img = tostr( reply->readAll() );
                DEBUG << "image url 1:" << img << endl;
                img = regex_match_single( img, ".*liveStream\" src=\"([^\"]*)\".*" );
                DEBUG << "image url 2:" << img << endl;
                img = Util::replace_all( img, "&amp;", "&" );

                DEBUG << "image url e:" << img << endl;
                fh_context z = Resolve( img );
                return z->getIStream();
            }
    };
    

    
    class FERRISEXP_CTXPLUGIN ZoneminderMonitorContext
        :
        public ZoneminderMonitorContextBase
    {
        typedef ZoneminderMonitorContextBase  _Base;
        typedef ZoneminderMonitorContext      _Self;
        
      public:

        ZoneminderMonitorContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
        {
        }

        void setup( const DOMElement* e )
        {
            fh_zoneminder zm = getZoneminder();

            addAttribute( "id",       getStrSubCtx( e, "ID" ) );
            addAttribute( "enabled",  getStrSubCtx( e, "ENABLED" ) );
            addAttribute( "function", getStrSubCtx( e, "FUNCTION" ) );
            addAttribute( "state",    getStrSubCtx( e, "STATE" ) );
            addAttribute( "width",    getStrSubCtx( e, "WIDTH" ) );
            addAttribute( "height",   getStrSubCtx( e, "HEIGHT" ) );
            addAttribute( "active",   this, &_Self::getIsActive );

            clearContext();
            FakeInternalContext* fic = 0;
            fic = priv_ensureSubContext( "events", fic );
            std::list< DOMElement* > del = XML::getAllChildrenElements( (DOMNode*)e, "EVENT" );
            DEBUG << "events.sz:" << del.size() << endl;
            for( std::list< DOMElement* >::iterator di = del.begin(); di != del.end(); ++di )
            {
                string rdn = getStrSubCtx( *di, "TIME" );
                string id  = getStrSubCtx( *di, "ID" );

                rdn = Util::replace_all(rdn, "/","-");
                DEBUG << "adding event:" << rdn << endl;

                ZoneminderEventContext* c = 0;
                c = fic->priv_ensureSubContext( rdn, c );
                c->setup( *di );
            }

            ZoneminderMonitorContextBase* liveCtx = 0;
            liveCtx = priv_ensureSubContext( "live.jpg", liveCtx );
            
        }
        
      private:
        
    };
    
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN ZoneminderServerContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext   _Base;
        typedef ZoneminderServerContext _Self;
        bool m_haveTriedToRead;
        fh_zoneminder m_zm;
        userpass_t getUserPass( const std::string& server );
        time_t m_readTime;
        
        
        
    public:

        ZoneminderServerContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_zm( Factory::getZoneminder( rdn ) )
            , m_haveTriedToRead( false )
            , m_readTime( 0 )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
                getZoneminder()->ensureAuthenticated();
                
            }

        fh_zoneminder getZoneminder()
        {
            return m_zm;
        }
        
        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " m_haveTriedToRead:" << m_haveTriedToRead
                      << " have read:" << getHaveReadDir()
                      << " subc.sz:" << getSubContextCount()
                      << endl;

//                if( getSubContextCount() )
                time_t now = Time::getTime();
                if( m_readTime && now - m_readTime < 60 )
                {
                    EnsureStartStopReadingIsFiredRAII _raii1( this );
                    emitExistsEventForEachItemRAII    _raii2( this );
                    return;
                }

                m_readTime = now;
                stringmap_t args;
                args["skin"]      = "xml";
                args["view"]      = "console";
                args["numEvents"] = "50";
                
                fh_zoneminder zm = getZoneminder();
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                QNetworkReply* reply = zm->post( args );
                fh_domdoc dom = zm->toDOM( reply );
                XML::DOMElementList_t del = XML::evalXPathToElements(  dom, "/ZM_XML/MONITOR_LIST/MONITOR" );
                DEBUG << "monitors.sz:" << del.size() << endl;
                for( XML::DOMElementList_t::iterator di = del.begin(); di != del.end(); ++di )
                {
                    string rdn = getStrSubCtx( *di, "NAME" );
                    DEBUG << "monitor.id:" << rdn << endl;

                    ZoneminderMonitorContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setup( *di );
                }
                
                // {
                //     ZoneminderStatusContext* c = 0;
                //     c = priv_ensureSubContext( "status", c );
                // }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    
    class FERRISEXP_CTXPLUGIN ZoneminderRootContext
        :
        public networkRootContext< ZoneminderServerContext >
    {
        typedef networkRootContext< ZoneminderServerContext > _Base;

      public:

        ZoneminderRootContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            {
                tryAugmentLocalhostNames();
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }

    };


    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    template < class ParentCtx >
    fh_zoneminder ZoneminderContextBase<ParentCtx>::getZoneminder()
    {
        ZoneminderServerContext* p = 0;
        p = ParentCtx::getFirstParentOfContextClass( p );
        return p->getZoneminder();
    }
    

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew()" << endl;

                static ZoneminderRootContext* c = 0;
                if( !c )
                {
                    c = new ZoneminderRootContext(0, "/" );
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
