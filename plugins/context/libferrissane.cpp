/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2011 Ben Martin

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
#include <Ferris/FerrisQt_private.hh>

#include <libksane/ksane.h>
#include <QApplication>
#include <QMap>
#include <QBuffer>
#include <Ferris/FerrisKDE.hh>
#undef emit

using namespace std;
#define DEBUG LG_SANE_D
//#define DEBUG cerr


using namespace KSaneIface;


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

    void ensureQtApp()
    {
        KDE::ensureKDEApplication();

        // static bool virgin = true;
        // if( virgin )
        // {
        //     virgin = false;
            
        //     static int argc = 1;
        //     static char* argv[] = { (char*)"libferris", 0 };
        //     static QApplication* app = new QApplication( argc, argv );
        // }
        
//                      static QCoreApplication* app = new QCoreApplication(argc, argv);
//                KDE::ensureKDEApplication();
                        //ensureQApplication();
                        
                
                
    }
    
    
    class ConfiguredScannerContext;
    class VirtualImageContext;
    



    class FERRISEXP_CTXPLUGIN ConfiguredScannerContext
        :
        public QObject,
        public StateLessEAHolder< ConfiguredScannerContext, leafContext >
    {
        Q_OBJECT;
        
        typedef StateLessEAHolder< ConfiguredScannerContext, leafContext > _Base;
        typedef ConfiguredScannerContext _Self;

        
        fh_context m_config;
        std::string m_deviceName;
        KSaneIface::KSaneWidget *m_ksane;
        bool                     m_scanComplete;
      protected:
        QImage                   m_img;
        
      public:

        ConfiguredScannerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                m_config = 0;
                m_deviceName = "";
                m_ksane = 0;
                createStateLessAttributes(1);
                addAttribute( "size", "-1", XSD_BASIC_INT );
            }
        void constructObject( fh_context config, std::string deviceName, std::string rdn )
        {
            m_config = config;
            m_deviceName = deviceName;
        }
        void constructObject( ConfiguredScannerContext* other )
        {
            m_config = other->m_config;
            m_deviceName = other->m_deviceName;
        }
        
                


        void startScan()
        {
            
            m_scanComplete = true;
            if( KSaneIface::KSaneWidget* z = getKSane() )
            {
                DEBUG << "XXX startScan() calling scanFinal()" << endl;
//                BackTrace();
                
                m_scanComplete = false;
                z->scanFinal();
            }
        }
        bool scanCompleted()
        {
            return m_scanComplete;
        }

        void grabImage()
        {
            if( KSaneIface::KSaneWidget* z = getKSane() )
            {
                startScan();
                while( !scanCompleted() )
                {
                    Main::processAllPendingEvents();
                    Time::Sleep( 0.001 );
                }
            }
        }

        string getConfig( string k, string v = "" )
        {
            string ret = getStrSubCtx( m_config, k, v );
            return ret;
        }
        
        string getOption( string k, string v = "" )
        {
            if( m_config->isSubContextBound("options"))
            {
                fh_context c = m_config->getSubContext( "options" );
                v = getStrSubCtx( c, k, v );
            }
            return v;
        }
        
        
        KSaneIface::KSaneWidget* getKSane()
        {
            if( !m_ksane )
            {

                m_ksane = new KSaneIface::KSaneWidget( 0 );
                
                if ( !m_ksane->openDevice( m_deviceName.c_str()))
                {
                    DEBUG << "can not open device:" << m_deviceName << endl;
                    m_ksane = 0;
                    stringstream ss;
                    ss << "can not open device:" << m_deviceName << endl;
                    Throw_CanNotReadContext( tostr(ss), this );
                }

                QMap <QString, QString> opts;
                m_ksane->getOptVals( opts );
                for( QMap <QString, QString>::iterator iter = opts.begin();
                     iter != opts.end(); ++iter )
                {
                    DEBUG << "k:" << tostr(iter.key()) << " v:" << tostr(iter.value()) << endl;
                }

                if( m_config->isSubContextBound("options"))
                {
                    fh_context c = m_config->getSubContext( "options" );
                    for( Context::iterator ci = c->begin(); ci != c->end(); ci++ )
                    {
                        string k = (*ci)->getDirName();
                        string v = getStrSubCtx( c, k, "" );
                        DEBUG << " setting option k:" << k << " v:" << v << endl;
                        m_ksane->setOptVal( (const char*)k.c_str(), v.c_str() );
                    }
                }
                m_ksane->setOptVal( (const char*)"wait-for-button", "1" );
                
                
                connect(m_ksane, SIGNAL(imageReady(QByteArray &, int, int, int, int)),
                        this, SLOT(imageReady(QByteArray &, int, int, int, int)));
                connect(m_ksane, SIGNAL(scanDone(int, const QString&)),
                        this, SLOT(scanDone(int, const QString&)));
                connect(m_ksane, SIGNAL(reportSorry(QWidget*, const QString&, const QString& )),
                        this, SLOT(reportSorry(QWidget*, const QString&, const QString& )));
                
            }
            return m_ksane;
        }
      public slots:

        void reportSorry(QWidget* parent, const QString& msg, const QString& )
        {
            DEBUG << "reportSorry():" << tostr(msg) << endl;
        }
        
        void scanDone(int status, const QString &strStatus)
        {
            DEBUG << "scanDone() status:" << status << " str:" << tostr(strStatus) << endl;
            m_scanComplete = true;

            if( strStatus.length() )
            {
                stringstream ss;
                ss << tostr(strStatus);
                Throw_CanNotReadContext( tostr(ss), this );
            }
        }
        void imageReady(QByteArray &data, int w, int h, int bpl, int f)
        {
            DEBUG << "imageReady(top)" << endl;
            m_ksane->scanCancel();
            m_scanComplete = true;
            DEBUG << "imageReady(2)" << endl;
            m_img = m_ksane->toQImage( data, w, h, bpl, (KSaneIface::KSaneWidget::ImageFormat)f);
            DEBUG << "imageReady(3)" << endl;
            


            // double quality = 80; 
            // if (m_img.save("/tmp/testscan.jpg", "jpg", quality))
            // {
            //     DEBUG << "saved image OK" << endl;
            // }
            // else
            // {
            //     DEBUG << "Failed to save image" << endl;
            // }

        }
    

      public:

        void priv_read();
        
        virtual void ensureAttributesAreCreated( const std::string& eaname = "" )
        {
            DEBUG << "ensureAttributesAreCreated(1) p:" << getDirPath() << endl;
            _Base::ensureAttributesAreCreated( eaname );
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
    
    
    class FERRISEXP_CTXPLUGIN VirtualImageContext
        :
        public ConfiguredScannerContext
    {
        Q_OBJECT;
        typedef ConfiguredScannerContext _Base;
        typedef VirtualImageContext      _Self;
        
        ConfiguredScannerContext* m_sc;
        
    public:

        VirtualImageContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                m_sc = 0;
                createStateLessAttributes(1);
                addAttribute( "is-file", "1", XSD_BASIC_BOOL );
            }
        void constructObject( ConfiguredScannerContext* sc )
        {
            m_sc = sc;
            _Base::constructObject( m_sc  );

        }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception);
        void priv_read()
        {
            return leafContext::priv_read();
        }
        virtual void read( bool force )
        {
            stringstream ss;
            ss << "Can not read this file as a directory... path:" << getDirPath();
            Throw_FerrisNotReadableAsContext( tostr(ss), this );
        }
        
        
    };
    
    
    
    class FERRISEXP_CTXPLUGIN ScannerContext
        :
        public StateLessEAHolder< ScannerContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< ScannerContext, FakeInternalContext > _Base;
        typedef ScannerContext _Self;

        fh_context m_config;
        std::string m_deviceName;
        
    public:

        ScannerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                 createStateLessAttributes();
            }
        void constructObject( fh_context config, std::string dn )
        {
            m_config = config;
            m_deviceName = dn;
        }



        void priv_read()
            {
                DEBUG << "ScannerContext::priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                DEBUG << "reading..." << endl;

                ensureQtApp();

                fh_context c = m_config;
                for( Context::iterator ci = c->begin(); ci != c->end(); ci++ )
                {
                    fh_context sc = *ci;
                    
                    if( sc->isDir() )
                    {
                        string rdn = sc->getDirName();
                        DEBUG << "have configuration directory:" << sc->getDirName() << endl;
                        ConfiguredScannerContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( sc, m_deviceName, rdn );
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
    
    class FERRISEXP_CTXPLUGIN SaneRootContext
        :
        public QObject,
        public FakeInternalContext
    {
        Q_OBJECT;
        
        typedef FakeInternalContext _Base;
        typedef SaneRootContext _Self;

        bool m_haveRead;
        bool m_haveSetupConfiguredScanners;
        KSaneIface::KSaneWidget* m_ksane;
        
        KSaneIface::KSaneWidget* getKSane()
        {
            if( !m_ksane )
            {
                m_ksane = new KSaneIface::KSaneWidget( 0 );
            }
            return m_ksane;
        }
        
      public:


        SaneRootContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_haveRead( false )
            , m_haveSetupConfiguredScanners( false )
            , m_ksane( 0 )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;

            }

        void setupConfiguredScanners()
        {
            if( m_haveSetupConfiguredScanners )
                return;
            m_haveSetupConfiguredScanners = true;
            
            fh_context c = Resolve( "~/.ferris/sane" );
                
            DEBUG << "inspecting scanners..." << endl;
            for( Context::iterator ci = c->begin(); ci != c->end(); ci++ )
            {
                DEBUG << "inspecting scanner setup for:" << ci->getDirName() << endl;
                string deviceName = getStrSubCtx( *ci, "device", "" );
                string dirName    = (*ci)->getDirName();
                DEBUG << "                      device:" << deviceName << endl;

                if( dirName == ".discovered" )
                    continue;

                ScannerContext* c = 0;
                c = priv_ensureSubContext( dirName, c );
                c->constructObject( *ci, deviceName );
            }
        }
        
        fh_context priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
        {
            setupConfiguredScanners();
            return _Base::priv_getSubContext( rdn );
        }

        ///////////////////

        bool             m_availableDevicesWaiting;
        QList< QString > m_availableDevices;
        QList< QString > listAvailableDevices( KSaneIface::KSaneWidget* sane )
        {
            if( m_availableDevices.size() )
                return m_availableDevices;

            //void availableDevices( const QList<KSaneWidget::DeviceInfo>& dl );
            
            connect(sane, SIGNAL(availableDevices( const QList<KSaneWidget::DeviceInfo>& )),
                    this, SLOT(availableDevices( const QList<KSaneWidget::DeviceInfo>& )));

//            availableDevices( m_availableDevices );
            m_availableDevicesWaiting = true;
            sane->initGetDeviceList();
            while( m_availableDevicesWaiting )
            {
//                cerr << "list avail... waiting..." << endl;
                Main::processAllPendingEvents();
                Time::Sleep( 0.100 );
            }
            return m_availableDevices;
        }
      public slots:
        void availableDevices( const QList<KSaneWidget::DeviceInfo>& dl )
        {
            cerr << "availableDevices()" << endl;
            m_availableDevicesWaiting = false;
            m_availableDevices.clear();
            for( QList<KSaneIface::KSaneWidget::DeviceInfo>::const_iterator diter = dl.begin(); diter!=dl.end(); ++diter )
            {
                m_availableDevices.push_back( diter->name );
            }
//            m_availableDevices = dl;
        }
      public:
        ///////////////////
        
        void priv_read()
            {
//                ensureQApplication();
                DEBUG << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;

                
                if( m_haveRead )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                m_haveRead = true;
                
                ensureQtApp();
       
                //EnsureStartStopReadingIsFiredRAII _raii1( this );
                setupConfiguredScanners();
                emitExistsEventForEachItemRAII _raii2(this);

                fh_context discoveredScannersConfig = Shell::acquireContext( "~/.ferris/sane/.discovered" );
                
                KSaneIface::KSaneWidget* sane = getKSane();

//                QList< QString > dl = sane->listAvailableDevices();
                QList< QString > dl = listAvailableDevices( sane );
                
                DEBUG << "dl.sz:" << dl.size() << endl;
                for( QList< QString >::iterator diter = dl.begin(); diter != dl.end(); ++diter )
                {
                    DEBUG << "diter:" << tostr(*diter) << endl;
                    string deviceName = tostr(*diter);
                    string dirName    = tostr(*diter);

                    dirName = Util::replace_all( dirName, " ", "_" );
                    ScannerContext* c = 0;
                    c = priv_ensureSubContext( dirName, c );
                    c->constructObject( discoveredScannersConfig, deviceName );
                    
                }
                
            }
    };


    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    void ConfiguredScannerContext::priv_read()
            {
                DEBUG << "ScannerContext::priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                DEBUG << "reading..." << endl;

                string rdn;

                rdn = "scan.jpg";
                VirtualImageContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->constructObject( this );

                // Add a directory of virtual files to grab at the ADF
                {
                    FakeInternalContext* sc = 0;
                    sc = priv_ensureSubContext( "adf", sc );

                    for( int i=0; i<25; ++i )
                    {
                        stringstream rdnss;
                        rdnss << "scan-" << i << ".jpg";
                        VirtualImageContext* c = 0;
                        c = sc->priv_ensureSubContext( rdnss.str(), c );
                        c->constructObject( this );
                    }
                }
            }
    
    
    fh_istream  VirtualImageContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception)
    {
        grabImage();

        // FIXME: don't serialize to RAM and then copy...
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        double quality = toType<double>(getConfig( "quality", "85" ));
        m_img.save( &buffer, "JPG", quality );

        // fh_istream ret = Factory::createIStreamFromQIODevice( &buffer );
        // return ret;
        
        fh_stringstream ss;
        ss << tostr(ba);
//        ss << "test";
        return ss;
    }

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
//                if (!g_thread_supported ()) g_thread_init (NULL);

                static SaneRootContext* c = 0;
                if( !c )
                {
                    c = new SaneRootContext(0, "/" );
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

#include "libferrissane_moc_impl.cpp"
