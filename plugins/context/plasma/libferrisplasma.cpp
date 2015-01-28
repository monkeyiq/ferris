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

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisQt_private.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>

#include <Plasma/DataEngine>
#include <Plasma/DataContainer>
#include <Plasma/DataEngineManager>


#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#undef emit

using namespace std;

#define DEBUG LG_PLASMA_D
//#define DEBUG cerr

namespace Ferris
{
    
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

    class PlasmaRootContext;
    class PlasmaEngineContext;




    class FERRISEXP_CTXPLUGIN PlasmaSourceContext
        :
        public QObject,
        public StateLessEAHolder< PlasmaSourceContext, leafContext >
    {
        Q_OBJECT;
        
        typedef StateLessEAHolder< PlasmaSourceContext, leafContext > _Base;
        typedef PlasmaSourceContext _Self;

        Plasma::DataContainer* m_dataContainer;
        Plasma::DataEngine*    m_dataEngine;

        bool m_waitingForInitialUpdate;
        
    public:

        PlasmaSourceContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                m_dataEngine = 0;
                m_dataContainer = 0;
                m_waitingForInitialUpdate = true;
                createStateLessAttributes(1);
            }
        void constructObject( Plasma::DataEngine* de, QString source )
        {
            m_dataEngine = de;
            if( source == "test" )
                return;



            DEBUG << "waiting for data source..." << tostr(source) << endl;
            m_waitingForInitialUpdate = true;
            de->connectSource( source, this );
            while( m_waitingForInitialUpdate )
            {
                Main::processAllPendingEvents();
                usleep(20 * 1000);
            }
            DEBUG << "done waiting for data!" << endl;
            
        }
    public slots:

        std::string toEA( QVariant v, XSDBasic_t& sct )
        {
            sct = XSD_UNKNOWN;

            switch(v.type())
            {
                case QVariant::UInt:
                case QVariant::Int:
                case QVariant::ULongLong:
                case QVariant::LongLong:
                    sct = XSD_BASIC_INT;
                    break;
                case QVariant::Url:
                    sct = FXD_URL;
                    break;
                case QVariant::DateTime:
                    sct = FXD_UNIXEPOCH_T;
                    return tostr(v.toDateTime().toTime_t());
                case QVariant::Date:
                    sct = FXD_UNIXEPOCH_T;
                    return tostr(QDateTime(v.toDate()).toTime_t());
                case QVariant::Bool:
                    sct = XSD_BASIC_BOOL;
                    break;
                case QVariant::Double:
                    sct = XSD_BASIC_DOUBLE;
                    break;
            }
            
            return tostr(v.toString());
        }
        
        
        void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
        {
            m_waitingForInitialUpdate = false;
            DEBUG << "dataUpdated(1) src:" << tostr(sourceName) << " data.sz:" << data.size() << endl;
            Plasma::DataEngine::DataIterator di(data);
            while( di.hasNext() )
            {
                di.next();
                XSDBasic_t sct = XSD_UNKNOWN;
                QString k = di.key();
                std::string v = toEA( di.value(), sct );
                
                DEBUG << "k:" << k << endl;
            
                addAttribute( tostr(k), v, sct );
            }
            DEBUG << "dataUpdated(2) src:" << tostr(sourceName) << " data.sz:" << data.size() << endl;
        }

        virtual void ensureAttributesAreCreated( const std::string& eaname = "" )
        {
            DEBUG << "ensureAttributesAreCreated()" << endl;
            _Base::ensureAttributesAreCreated( eaname );
        }
        
        
        Plasma::DataEngine* getEngine()
        {
            return m_dataEngine;
        }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception)
        {
            fh_stringstream ss;
            ss << "test";
            return ss;
        }
        
    };
    
    
    
    
    
    class FERRISEXP_CTXPLUGIN PlasmaEngineContext
        :
        public StateLessEAHolder< PlasmaEngineContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< PlasmaEngineContext, FakeInternalContext > _Base;
        typedef PlasmaEngineContext _Self;

        Plasma::DataEngine* m_dataEngine;
        
    public:

        PlasmaEngineContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                 createStateLessAttributes();
            }
        void constructObject( Plasma::DataEngine* de )
        {
            m_dataEngine = de;
        }
        Plasma::DataEngine* getEngine()
        {
            return m_dataEngine;
        }

        void priv_read()
            {
                DEBUG << "PlasmaEngineContext::priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                DEBUG << "reading..." << endl;
                QStringList sl = getEngine()->sources();
                sl.removeDuplicates();
                DEBUG << "sources.sz:" << sl.size() << endl;
                foreach( QString s, sl )
                {
                    DEBUG << "source:" << tostr(s) << endl;
                    Plasma::DataContainer* dc = getEngine()->containerForSource( s );
                    PlasmaSourceContext* c = 0;
                    c = priv_ensureSubContext( tostr(s), c );
                    c->constructObject( getEngine(), s );
                }
                
            }

        
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN PlasmaRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef PlasmaRootContext _Self;
        
    public:


        PlasmaRootContext( Context* parent, const std::string& rdn )
            : 
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;

            }

        void priv_read()
            {
                ensureQApplication();
                DEBUG << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;

                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }

                

                EnsureStartStopReadingIsFiredRAII _raii1( this );
                Plasma::DataEngineManager* em = Plasma::DataEngineManager::self();
                DEBUG << "reading..." << endl;
                QStringList sl = Plasma::DataEngineManager::listAllEngines();
                DEBUG << "engines.sz:" << sl.size() << endl;
                foreach( QString s, sl )
                {
                    DEBUG << "engine:" << tostr(s) << endl;
                    if( s == "tasks" || s == "mouse" || s == "keystate" )
                        continue;
                        
                    Plasma::DataEngine* de = em->engine(s);
                    if( !de->isValid() )
                        de = em->loadEngine(s);
                        
                    PlasmaEngineContext* c = 0;
                    c = priv_ensureSubContext( tostr(s), c );
                    c->constructObject( de );
                }
            }

        
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
    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                DEBUG << "Brew()" << endl;
                if (!g_thread_supported ()) g_thread_init (NULL);

                static PlasmaRootContext* c = 0;
                if( !c )
                {
                    c = new PlasmaRootContext(0, "/" );
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

#include "libferrisplasma_moc.cpp"
