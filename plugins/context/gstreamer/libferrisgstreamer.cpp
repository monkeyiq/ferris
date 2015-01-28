/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2009 Ben Martin

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

    $Id: ut_close_signal.cpp,v 1.3 2008/05/24 21:31:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "libferrisgstreamer_shared.hh"

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>


using namespace std;

//#define DEBUG LG_GSTREAMER_D
#define DEBUG cerr

namespace Ferris
{
    using namespace GStreamer;
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN GStreamerCaptureContext
        :
        public StateLessEAHolder< GStreamerCaptureContext, leafContext >
    {
        typedef StateLessEAHolder< GStreamerCaptureContext, leafContext > _Base;
        typedef GStreamerCaptureContext _Self;

        string m_spec;

        static fh_istream
            SL_getSizeFromContentIStream( Context* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << 1;
            return ss;
        }
    
    public:

        GStreamerCaptureContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_spec("")
            {
                tryAddStateLessAttribute( "size",
                                          SL_getSizeFromContentIStream,
                                          FXD_FILESIZE );
                createStateLessAttributes();
            }
        virtual ~GStreamerCaptureContext()
        {
        }
        void constructObject( const string& spec )
        {
            m_spec = spec;
        }
        virtual bool isDir()
        {
            return true;
        }
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                DEBUG << "GStreamerCaptureContext::priv_FillCreateSubContextSchemaParts" << endl;
                
                m["file"] = SubContextCreator(SL_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");
                m["dir"]  = SubContextCreator(SL_SubCreate_dir,
                                              "	<elementType name=\"dir\">\n"
                                              "		<elementType name=\"name\" default=\"new directory\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");
            }
        virtual fh_context SubCreate_dir( fh_context ctx, fh_context md )
        {
            return SubCreate_file( ctx, md );
        }
        
        virtual fh_context SubCreate_file( fh_context ctx, fh_context md )
            {
                DEBUG << "GStreamerCaptureContext::SubCreate_file" << endl;
                
                string rdn = getStrSubCtx( md, "name", "" );
                DEBUG << "create_file for rdn:" << rdn << endl;
                
                GStreamerCaptureContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->constructObject( m_spec );
                fh_context child = getSubContext( rdn );
                
//                fh_context child = 0;
                // child = new GStreamerCaptureContext( this, rdn );
                // constructObject( m_spec );
                // Insert( GetImpl(child), false, true );

                DEBUG << "create_file OK for rdn:" << rdn << endl;
                return child;
            }
        

        virtual std::string getRecommendedEA()
            {
                return adjustRecommendedEAForDotFiles(this, "name");
            }

        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_gstreamer_readFrom_stream ss( m_spec );
                return ss;
            }

        fh_iostream
        priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
            {
                fh_gstreamer_readFrom_stream ss( m_spec );
                return ss;
            }
        

        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }
    };
    
    
    class FERRISEXP_CTXPLUGIN GStreamerTopLevelDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef GStreamerTopLevelDirectoryContext _Self;

        std::string m_topLevelXMLElement;
        
    public:

        GStreamerTopLevelDirectoryContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_topLevelXMLElement("")
            {
            }
        void constructObject( const string& n )
        {
            m_topLevelXMLElement = n;
        }
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    string domxml = getStrSubCtx( "~/.ferris/gstreamer.xml", "", true, false );
                    if( !domxml.empty() )
                    {
                        fh_domdoc dom = Factory::StringToDOM( domxml );
                        DEBUG << "have dom!" << endl;
                        DOMElement* root = dom->getDocumentElement();
                        if( DOMElement* xelement = XML::firstChild( root, m_topLevelXMLElement ) )
                        {
                            std::list< DOMElement* > el =
                                XML::getAllChildrenElements( xelement, "file", true );

                            for( std::list< DOMElement* >::iterator ei = el.begin(); ei!=el.end(); ++ei )
                            {
                                string name = ::Ferris::getAttribute( *ei, "name" );
                                string spec = getStrSubCtx( *ei, "source" );
                                DEBUG << "name:" << name << endl;
                                DEBUG << "spec:" << spec << endl;

                                GStreamerCaptureContext* c = 0;
                                c = priv_ensureSubContext( name, c );
                                c->constructObject( spec );
                            }
                        }
                    }
                }
            }
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN GStreamerRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
    public:

        GStreamerRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                LG_FACEBOOK_D << "ctor, have read:" << getHaveReadDir() << endl;
            }

        
        void priv_read()
            {
                LG_FACEBOOK_D << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    fh_context c;
                    {

                        string domxml = getStrSubCtx( "~/.ferris/gstreamer.xml", "", true, false );
                        if( !domxml.empty() )
                        {
                            fh_domdoc dom = Factory::StringToDOM( domxml );
                            DEBUG << "have dom!" << endl;
                            DOMElement* root = dom->getDocumentElement();
                        
                            domnode_list_t nl;
                            XML::getChildren( nl, root );
                            for( std::list< DOMNode* >::iterator ei = nl.begin(); ei!=nl.end(); ++ei )
                            {
                                if( DOMElement* e = dynamic_cast<DOMElement*>(*ei))
                                {
                                    // e->getNodeName()
                                    string name = ::Ferris::getAttribute( e, "name" );
                                    if( name.empty() )
                                        name = tostr(e->getNodeName());
                                    
                                    DEBUG << "x1 name:" << name << endl;

                                    if( !name.empty() )
                                    {
                                        GStreamerTopLevelDirectoryContext* c = 0;
                                        c = priv_ensureSubContext( name, c );
                                        c->constructObject(name);
                                    }
                                }
                            }
                        }
                    }
                }
            }
    };

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                int argc = 0;
                char* argv[] = { (char*)"/usr/local/bin/libferris", 0 };
                gst_init (&argc, (char***)&argv);
                LG_FACEBOOK_D << "Brew()" << endl;

                static GStreamerRootContext* c = 0;
                if( !c )
                {
                    c = new GStreamerRootContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
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
