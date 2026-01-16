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

#include <QApplication>
#include <QTextDocument>
#include <QPrinterInfo>
#include <QMap>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#undef emit


using namespace std;
//#define DEBUG LG_PRINTER_D
#define DEBUG cerr





namespace Ferris
{
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };

    std::string getPrintFilePath()
    {
        stringstream ss;
        ss << Shell::getHomeDirPath();
        ss << "/Desktop";
        ss << "/";
        return ss.str();
//        return "/tmp/";
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

    void ensureQtApp()
    {
        static bool virgin = true;
        if( virgin )
        {
            virgin = false;
            
            static int argc = 1;
            static char* argv[] = { (char*)"libferris", 0 };
            static QApplication* app = new QApplication( argc, argv );
        }
        
//                      static QCoreApplication* app = new QCoreApplication(argc, argv);
//                KDE::ensureKDEApplication();
                        //ensureQApplication();
                        
                
                
    }


    

    class FERRISEXP_DLLLOCAL PrintFileContext
        :
        public StateLessEAHolder< PrintFileContext, leafContext >
    {
        typedef StateLessEAHolder< PrintFileContext, leafContext > _Base;
        typedef PrintFileContext _Self;

        string    m_id;
        long long m_ContentLength;
        string    m_explicitUploadFilename;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                DEBUG << "" << endl;

                m["ea"] = SubContextCreator(
                    SL_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }


        fh_iostream
        getNullEAStream( Context*, const std::string&, EA_Atom* attr )
            {
                fh_stringstream ss;
                return ss;
            }
        void
        NullEAStreamClosed( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
            }
        

        void addNullAttr( const std::string& rdn )
            {
                addAttribute( rdn,
                              this, &_Self::getNullEAStream,
                              this, &_Self::getNullEAStream,
                              this, &_Self::NullEAStreamClosed,
                              XSD_BASIC_STRING );
            }
        

        
        fh_context
        SubCreate_ea( fh_context c, fh_context md )
            {
                DEBUG << "SubCreate_ea(1)" << endl;
                string rdn = getStrSubCtx( md, "name", "", true, true );
                addNullAttr( rdn );
                return c;
            }
        
    public:

        PrintFileContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_id(""),
            m_ContentLength( 0 )
            {
                createStateLessAttributes();

                addNullAttr( "user-owner-number" );
                addNullAttr( "group-owner-number" );
                addNullAttr( "ferris-type" );
                addNullAttr( "dontfollow-selinux-context" );
                addNullAttr( "mode" );
                addNullAttr( "mtime" );
                addNullAttr( "atime" );
                addNullAttr( "ctime" );
            }
        
        virtual ~PrintFileContext()
            {}

        void createStateLessAttributes( bool force = false )
            {
                if( force || isStateLessEAVirgin() )
                {
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ss;
                DEBUG << "returning an empty stream" << endl;
                return ss;
            }

        virtual void priv_preCopyAction( fh_context c )
            {
                DEBUG << "preCopyAction() c:" << c->getURL()
                      << " m_id:" << m_id
                      << endl;
            }
        
        virtual void priv_postCopyAction( fh_context c )
            {
                DEBUG << "postCopyAction() c:" << c->getURL()
                      << " m_id:" << m_id
                      << endl;

                if( m_id.empty() )
                {
                    DEBUG << "postCopyAction() c:" << c->getURL()
                          << " no web photo ID! not tagging remote file."
                          << endl;
                    return;
                }
                
                // Save off the photo-id on the source image for later use
                try
                {
//                    setStrAttr( c, "webphoto-photo-id", m_id, true, true );
                }
                catch( exception& e )
                {
                    DEBUG << "Error linking source with photo id:" << e.what() << endl;
                }
            }
        
        
        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }


        fh_iostream
        priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                return ret;
            }
        
        
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );

        

        
        
    };

    
    class FERRISEXP_CTXPLUGIN PrinterContext
        :
        public StateLessEAHolder< PrinterContext, FakeInternalContext >
    {
        typedef StateLessEAHolder< PrinterContext, FakeInternalContext > _Base;
        typedef PrinterContext _Self;

        std::string m_printerName;
        
    public:

        PrinterContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                 createStateLessAttributes();
            }
        void constructObject( const std::string& pname )
        {
            m_printerName = pname;
        }
        std::string getPrinterName()
        {
            return m_printerName;
        }
        
        virtual bool isDir()
        {
            return true;
        }
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                DEBUG << "upload dir. setting file creation schema" << endl;
                m["file"] = SubContextCreator(SL_SubCreate_file,
                                              "	<elementType name=\"file\">\n"
                                              "		<elementType name=\"name\" default=\"new file\">\n"
                                              "			<dataTypeRef name=\"string\"/>\n"
                                              "		</elementType>\n"
                                              "	</elementType>\n");
            }
        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                string rdn = getStrSubCtx( md, "name", "" );
                DEBUG << "create_file for rdn:" << rdn << endl;
                
                fh_context child = 0;
                child = new PrintFileContext( this, rdn );
                Insert( GetImpl(child), false, true );

                
                DEBUG << "create_file OK for rdn:" << rdn << endl;
                return child;
            }
        
        
        void priv_read()
            {
                DEBUG << "PrinterContext::priv_read() url:" << getURL()
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
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN PrinterRootContext
        :
        public QObject,
        public FakeInternalContext
    {
        Q_OBJECT;
        
        typedef FakeInternalContext _Base;
        typedef PrinterRootContext _Self;

        bool m_haveRead;
        
      public:


        PrinterRootContext( Context* parent, const std::string& rdn )
            : _Base( parent, rdn )
            , m_haveRead( false )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;

            }

        void priv_read()
            {
                ensureQtApp();
                DEBUG << "priv_read() url:" << getURL()
                           << " have read:" << getHaveReadDir()
                           << endl;

                if( m_haveRead )
                {
                    emitExistsEventForEachItemRAII _raii2(this);
                    return;
                }
                m_haveRead = true;
                
       
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                QList<QPrinterInfo> pi = QPrinterInfo:: availablePrinters();
                cerr << "pi.sz:" << pi.size() << endl;
                for( QList<QPrinterInfo>::iterator piter = pi.begin(); piter != pi.end(); ++piter )
                {
                    cerr << "printer:" << tostr(piter->printerName()) << endl;
                    std::string dirName = tostr(piter->printerName());
                    dirName = Util::replace_all( dirName, " ", "_" );

                    PrinterContext* c = 0;
                    c = priv_ensureSubContext( dirName, c );
                    c->constructObject( tostr(piter->printerName()) );
                    
                }
            }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    void
    PrintFileContext::OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                DEBUG << "OnStreamClosed()" << endl;
                
                if( !(m & std::ios::out) )
                    return;
                
                AdjustForOpenMode_Closing( ss, m, tellp );
                const string data = StreamToString(ss);
                DEBUG << "OnStreamClosed() data.sz:" << data.size() << endl;

                PrinterContext* pc = 0;
                pc = getFirstParentOfContextClass( pc );
                std::string printerName = pc->getPrinterName();
                DEBUG << "Printer name:" << printerName << endl;
                
                QPrinter p;
                p.setPrinterName( printerName.c_str() );

                std::stringstream fnamess;
                fnamess << getPrintFilePath();
                fnamess << getDirName();
                if( printerName == "Cups-PDF" )
                {
                    if( !ends_with( fnamess.str(), "pdf" ))
                        fnamess << ".pdf";
                }
                DEBUG << "outputfilename:" << fnamess.str() << endl;
                p.setOutputFileName( fnamess.str().c_str() );
//                p.setOutputFileName("/tmp/ferris-print-boo.pdf");
                DEBUG << "printer isvalid:" << p.isValid() << endl;

                QByteArray ba( data.c_str(), data.size() );
                QImage qi = QImage::fromData( ba );
                if( !qi.isNull() )
                {
                    DEBUG << "User prints an image..." << endl;
                    QImage scaled = qi.scaled( QSize( p.width(), p.height() ),
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation );
                        
                    QPainter pablo;
                    pablo.setRenderHint(QPainter::Antialiasing, true);
                    pablo.begin(&p);
                    pablo.drawImage( 0, 0, scaled );
//                    pablo.drawImage( QRect ( 0, 0, p.width(), p.height() ), scaled );
                    pablo.end();
                    
                    return;
                }
                
                QTextDocument document( 0 );
                document.setPlainText( data.c_str() );
                document.print(&p);
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
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                DEBUG << "Brew()" << endl;
//                if (!g_thread_supported ()) g_thread_init (NULL);

                static PrinterRootContext* c = 0;
                if( !c )
                {
                    c = new PrinterRootContext(0, "/" );
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

#include "libferrisprinter_moc.cpp"
