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

    $Id: libferrisxwin_ecore.cpp,v 1.4 2010/09/24 21:31:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "libferrisxwin_ecore.hh"

#include <algorithm>
#include <numeric>
// #include <boost/lambda/lambda.hpp>
// using namespace boost::lambda;

namespace Ferris
{
        struct ecoreSingleton 
        {
            ecoreSingleton()
                {
                    int argc = 1;
                    const char* argv[] = {"libferris", 0};
                    ecore_init();
                    ecore_app_args_set(argc, argv);
                    ecore_x_init( 0 );
                    ecore_x_netwm_init();
                    
                }
            ~ecoreSingleton()
                {
                }
        };

    void ensureECoreSetup()
    {
        static ecoreSingleton _obj;
        return;
    }
    
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    /**
     * wrapper for a single Ecore_X_Window
     */
    class FERRISEXP_CTXPLUGIN xwinContext
        :
        public StateLessEAHolder< xwinContext, leafContext >
    {
        typedef xwinContext                                          _Self;
        typedef StateLessEAHolder< xwinContext, leafContext > _Base;

        Ecore_X_Window m_win;
        
    protected:
        virtual std::string priv_getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, "name,xwindow-name,x,y,w,h,pid,hostname,user-time,iconname");
            }

        struct fourPoints
        {
            int x;
            int y;
            int w;
            int h;
            fourPoints( int x=0, int y=0, int w=0, int h=0 )
                :
                x(x), y(y), w(w), h(h)
                {
                }
            inline fourPoints& operator+=( const fourPoints& p )
                {
                    x += p.x;
                    y += p.y;
                    w += p.w;
                    h += p.h;
                    return *this;
                }
            inline fourPoints operator+( const fourPoints& p )
                {
                    return fourPoints( x+p.x, y+p.y, w+p.w, h+p.h );
                }
        };
        typedef list< fourPoints > fourPointsList_t;

        static fourPointsList_t& getChainToParent( fourPointsList_t& ret, Ecore_X_Window win )
            {
                if( !win )
                    return ret;
                
                int x=0,y=0,w=0,h=0;
                ecore_x_window_geometry_get( win, &x, &y, &w, &h );
                ret.push_back( fourPoints( x,y,w,h ) );
                Ecore_X_Window pw = ecore_x_window_parent_get( win );
                return getChainToParent( ret, pw );
            }
        static fourPoints accumulateGeometry( Ecore_X_Window win )
            {
                fourPoints ret;
                fourPointsList_t fpl;
                getChainToParent( fpl, win );
                ret = accumulate( fpl.begin(), fpl.end(), ret );
//                 cerr << "accumulateGeometry(start)" << endl;
//                 for( fourPointsList_t::const_iterator ci = fpl.begin(); ci!=fpl.end(); ++ci )
//                 {
//                     cerr << "accumulateGeometry(x):" << (ci)->x << endl;
//                     ret += *ci;
//                 }
//                cerr << "accumulateGeometry(end)" << endl;
                return ret;
            }
        
        
        static fh_stringstream SL_getX( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fourPoints g = accumulateGeometry( c->m_win );
                fh_stringstream ss;
                ss << g.x;
                return ss;
            }
        static void SL_setX( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                int v = 0;
                if( ss >> v )
                {
                    fourPoints g = accumulateGeometry( c->m_win );
                    ecore_x_window_move( c->m_win, v, g.y );
//                     int x=0,y=0,w=0,h=0;
//                     ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
//                     fourPoints g = accumulateGeometry( c->m_win );
//                     cerr << "setX() v:" << v << " dx:" << (v - g.x) << endl;
//                     ecore_x_window_move( c->m_win, v - g.x, y );
                }
            }
        static fh_stringstream SL_getY( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fourPoints g = accumulateGeometry( c->m_win );
                fh_stringstream ss;
                ss << g.y;
                return ss;
            }
        static void SL_setY( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                int v = 0;
                if( ss >> v )
                {
                    fourPoints g = accumulateGeometry( c->m_win );
                    ecore_x_window_move( c->m_win, g.x, v );
                }
//                 int x=0,y=0,w=0,h=0;
//                 ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
//                 int v = toint(StreamToString(ss));

//                 ecore_x_window_move( c->m_win, x, v );
            }
        static fh_stringstream SL_getW( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                int x=0,y=0,w=0,h=0;
                ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
                fh_stringstream ss;
                ss << w;
                return ss;
            }
        static void SL_setW( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                int x=0,y=0,w=0,h=0;
                ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
                int v = toint(StreamToString(ss));

                ecore_x_window_resize( c->m_win, v, h );
            }
        static fh_stringstream SL_getH( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                int x=0,y=0,w=0,h=0;
                ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
                fh_stringstream ss;
                ss << h;
                return ss;
            }
        static void SL_setH( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                int x=0,y=0,w=0,h=0;
                ecore_x_window_geometry_get( c->m_win, &x, &y, &w, &h );
                int v = toint(StreamToString(ss));

                ecore_x_window_resize( c->m_win, w, v );
            }
        static fh_istream SL_getPID( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                int pid = 0;
                if( ecore_x_netwm_pid_get( c->m_win, &pid ) )
                    ss << pid;
                return ss;
            }
        static fh_istream SL_getDesktop( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                unsigned int desktop = 0;
                if( ecore_x_netwm_desktop_get( c->m_win, &desktop ) )
                    ss << desktop;
                return ss;
            }
        static fh_istream SL_getIconname( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                string icon;
                const char* icon_CSTR  = ecore_x_icccm_icon_name_get( c->m_win );
                if( icon_CSTR ) icon = icon_CSTR;
                fh_stringstream ss;
                ss << icon;
                return ss;
            }
        static fh_stringstream SL_getVisible( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                ss << ecore_x_window_visible_get( c->m_win );
                return ss;
            }
        static void SL_setVisible( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                int v = toint(StreamToString(ss));
                if( v )
                    ecore_x_window_show(c->m_win);
                else
                    ecore_x_window_hide(c->m_win);
                    
            }
        static fh_stringstream SL_getClientMachine( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                char* v = ecore_x_icccm_client_machine_get( c->m_win );
                fh_stringstream ss;
                ss << (v?v:"");
                return ss;
            }
        static fh_stringstream SL_getName( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                char* v = 0;
                if( ecore_x_netwm_name_get( c->m_win, &v ) )
                    ss << (v?v:"");
                return ss;
            }
        static fh_stringstream SL_getVisibleName( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                char* v = 0;
                if( ecore_x_netwm_visible_name_get( c->m_win, &v ) )
                    ss << (v?v:"");
                return ss;
            }
        static fh_stringstream SL_getUserTime( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                unsigned int time = 0;
                if( ecore_x_netwm_user_time_get( c->m_win, &time) )
                    ss << time;
                return ss;
            }
        static fh_stringstream SL_getPing( _Self* c, const std::string& rdn, EA_Atom* atom )
            {
                ecore_x_netwm_ping_send( c->m_win );
                fh_stringstream ss;
                return ss;
            }
        static void SL_setPing( _Self* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                ecore_x_netwm_ping_send( c->m_win );
            }
        
        
        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "x", SL_getX, SL_getX, SL_setX, FXD_PIXELCOUNT );
                    tryAddStateLessAttribute( "y", SL_getY, SL_getY, SL_setY, FXD_PIXELCOUNT );
                    tryAddStateLessAttribute( "w", SL_getW, SL_getW, SL_setW, FXD_PIXELCOUNT );
                    tryAddStateLessAttribute( "h", SL_getH, SL_getH, SL_setH, FXD_PIXELCOUNT );

                    tryAddStateLessAttribute( "pid",       SL_getPID,      FXD_PID );
                    tryAddStateLessAttribute( "desktop",   SL_getDesktop,  XSD_BASIC_INT );
                    tryAddStateLessAttribute( "iconname",  SL_getIconname, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "visible",
                                              SL_getVisible, SL_getVisible,
                                              SL_setVisible, XSD_BASIC_BOOL );

                    tryAddStateLessAttribute( "hostname", SL_getClientMachine, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "xwindow-name", SL_getName, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "visible-name", SL_getVisibleName, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "user-time", SL_getUserTime, FXD_UNIXEPOCH_T );
                    tryAddStateLessAttribute( "ping", SL_getPing, SL_getPing, SL_setPing, XSD_BASIC_BOOL );
                    
                    
                    _Base::createStateLessAttributes( true );
                }
            }
        
    public:

        void setWindow( Ecore_X_Window win )
            {
                m_win = win;
            }
        
        xwinContext( Context* parent, const std::string& rdn, Ecore_X_Window win = 0 )
            :
            _Base( parent, rdn ),
            m_win( win )
            {
//                 Ecore_X_Window_Attributes a;
//                 ecore_x_window_attributes_get( win, &a );

                
                createStateLessAttributes( true );
            }
        
        virtual ~xwinContext()
            {
            }
    };
    FERRIS_SMARTPTR( xwinContext, fh_xwinContext );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    void
    xwinTopDirectoryContext::getWinList( winlist_t& ret, Ecore_X_Window* el, int el_sz )
    {
        for( int i=0; i<el_sz; ++i )
        {
            Ecore_X_Window win = el[i];

            const char* title = ecore_x_icccm_title_get( win );
            int         vis   = ecore_x_window_visible_get( win );
            if( title && vis )
            {
                ret.push_back( win );
            }
        
            int childlist_sz = 0;
            Ecore_X_Window* childlist = ecore_x_window_children_get( win, &childlist_sz );
            getWinList( ret, childlist, childlist_sz );
        }
    }

    void
    xwinTopDirectoryContext::getWinList( winlist_t& ret )
    {
        int el_sz = 0;
        Ecore_X_Window* el = ecore_x_window_root_list( &el_sz );
        getWinList( ret, el, el_sz );
    }
    
    void
    xwinTopDirectoryContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );

//        if( empty() )
        {
            clearContext();
            winlist_t wl;
            getWinList( wl );
        
            for( winlist_t::const_iterator ci = wl.begin(); ci!=wl.end(); ++ci )
            {
                Ecore_X_Window win = *ci;
                const char* title_CSTR = ecore_x_icccm_title_get( win );
                string rdn = title_CSTR ? title_CSTR : "noname";
            
                 xwinContext* cc = 0;
                 cc = priv_ensureSubContext( rdn, cc, true );
                 cc->setWindow( win );

//                 cerr << "original rdn:" << rdn << endl;
//                 rdn = monsterName( rdn );
//                 cerr << "monstered rdn:" << rdn << endl;
//                 fh_xwinContext child = new xwinContext( this, rdn, win );
//                 Insert( GetImpl(child), false, false );
            }
        }
    }
        
    xwinTopDirectoryContext::xwinTopDirectoryContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        ensureECoreSetup();
        createStateLessAttributes();
    }
    xwinTopDirectoryContext::xwinTopDirectoryContext()
    {
        ensureECoreSetup();
        createStateLessAttributes();
        createAttributes();
    }
    
    xwinTopDirectoryContext::~xwinTopDirectoryContext()
    {
    }
    std::string
    xwinTopDirectoryContext::priv_getRecommendedEA()
    {
        return adjustRecommendedEAForDotFiles(this, "name,x,y,w,h,pid,hostname,user-time,iconname");
    }
    std::string
    xwinTopDirectoryContext::getRecommendedEA()
    {
        return adjustRecommendedEAForDotFiles(this, "name,x,y,w,h,pid,hostname,user-time,iconname");
    }
    
    xwinTopDirectoryContext*
    xwinTopDirectoryContext::priv_CreateContext( Context* parent, string rdn )
    {
        xwinTopDirectoryContext* ret = new xwinTopDirectoryContext();
        ret->setContext( parent, rdn );
        return ret;
    }
        
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
};
