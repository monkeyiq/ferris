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

    $Id: libferriscurl.cpp,v 1.10 2009/09/21 10:20:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
//
//
// Note that RFC 0959 is the FTP one.
//
//
#include <config.h>

#include <Ferris_private.hh>
#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <General.hh>

#include <SmartPtr.h>
#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <string>
#include <map>
#include <vector>

#include <sys/utsname.h>
#include <time.h>

#include <sigc++/sigc++.h>
#include <sigc++/slot.h>
#include <sigc++/bind.h>
#include <sigc++/object.h>
#include <sigc++/object_slot.h>

#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QFtp>
#include <QFile>
#include <QBuffer>
#undef emit


using namespace std;

#define DEBUG LG_QIO_D


namespace Ferris
{
    static string CreateObjectType_k      = "CreateObjectType";
    static string CreateObjectType_v_Dir  = "Dir";
    static string CreateObjectType_v_File = "File";
    static string CreateObjectType_k_FileForceCreate = "FileForceCreate";
    
    class qtFtpContext;
    class qtFtpServerContext;
    class qtFtpRootContext;
    
    class FERRISEXP_CTXPLUGIN qtFtpContext
        :
        public QObject,
        public StateLessEAHolder< qtFtpContext, FakeInternalContext >
    {
        Q_OBJECT;
        typedef StateLessEAHolder< qtFtpContext, FakeInternalContext > _Base;

        QUrlInfo m_info;
        QBuffer* m_readBuffer;
        fh_StreamToQIODevice qio;
        bool m_waiting;
                      
    public slots:

        void OnListInfo ( const QUrlInfo & i );
        void OnListStarted ( int id );
        void OnListFinished ( int id, bool error );
        void OnGetFinished ( int id, bool error );
        void OnGeneralFinished ( int id, bool error );
        
    public:

        qtFtpContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_readBuffer( 0 )
        {
            createStateLessAttributes();
            addAttribute( EAN_IS_REMOTE, "1", XSD_BASIC_BOOL );
           
        }
        void constructObject( const QUrlInfo & i )
        {
            m_info = i;
        }
        virtual ~qtFtpContext()
        {
        }

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception);
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
        virtual fh_context SubCreate_file( fh_context c, fh_context md );
        virtual fh_context SubCreate_dir( fh_context c, fh_context md );
        

        static fh_istream SL_getMTimeRawStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            QDateTime dt = c->m_info.lastModified();
            ss << dt.toTime_t();
            return ss;
        }

        static fh_istream SL_getATimeRawStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            QDateTime dt = c->m_info.lastRead();
            ss << dt.toTime_t();
            return ss;
        }
        
        static fh_istream SL_getSizeStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_info.size();
            return ss;
        }
            
        static fh_istream SL_getUserOwnerNumberStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << tostr(c->m_info.owner());
            return ss;
        }
            
        static fh_istream SL_getGroupOwnerNumberStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << tostr(c->m_info.group());
            return ss;
        }

        static fh_istream SL_getPermissionsStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_info.permissions();
            return ss;
        }

        static fh_istream SL_getPermissionsLsStream( qtFtpContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream oss;
            QUrlInfo i = c->m_info;
            int p = i.permissions();

            if     ( i.isSymLink() )          oss << "l";
            else if( i.isDir() )              oss << "d";
            else                              oss << "-";

            if( p & QUrlInfo::ReadOwner )     oss << "r";
            else                              oss << "-";
            if( p & QUrlInfo::WriteOwner )    oss << "w";
            else                              oss << "-";
            if( p & QUrlInfo::ExeOwner )      oss << "x";
            else                              oss << "-";
    
            if( p & QUrlInfo::ReadGroup )     oss << "r";
            else                              oss << "-";
            if( p & QUrlInfo::WriteGroup )    oss << "w";
            else                              oss << "-";
            if( p & QUrlInfo::ExeGroup )      oss << "x";
            else                              oss << "-";
    
            if( p & QUrlInfo::ReadOther )     oss << "r";
            else                              oss << "-";
            if( p & QUrlInfo::WriteOther )    oss << "w";
            else                              oss << "-";
            if( p & QUrlInfo::ExeOther )      oss << "x";
            else                              oss << "-";
            
            return oss;
        }
        
        
        void
            createStateLessAttributes( bool force = false )
        {
            static Util::SingleShot virgin;
            if( virgin() )
            {
                LG_CURL_D << "qtFtpContext::createStateLessAttributes()" << endl;
                tryAddStateLessAttribute( "mtime", SL_getMTimeRawStream, FXD_UNIXEPOCH_T );
                tryAddStateLessAttribute( "atime", SL_getATimeRawStream, FXD_UNIXEPOCH_T );
                tryAddStateLessAttribute( "size",  SL_getSizeStream,     FXD_FILESIZE );
                tryAddStateLessAttribute( "user-owner-number",
                                          SL_getUserOwnerNumberStream,
                                          FXD_UID_T );
                tryAddStateLessAttribute( "group-owner-number",
                                          SL_getGroupOwnerNumberStream,
                                          FXD_GID_T );
                tryAddStateLessAttribute( "protection-raw", SL_getPermissionsStream, FXD_MODE_T );
                tryAddStateLessAttribute( "protection-ls",  SL_getPermissionsLsStream, FXD_MODE_STRING_T );
                
                _Base::createStateLessAttributes( true );
                supplementStateLessAttributes( true );
            }
        }
        
        
        QFtp& getQFTP();
        void priv_read();

        virtual qtFtpServerContext* getServer();
        string getFTPPath();
    };
    

    class FERRISEXP_CTXPLUGIN qtFtpServerContext
        :
        public qtFtpContext
    {
        Q_OBJECT;
        typedef qtFtpContext _Base;
        
        QFtp m_qftp;
        bool m_connecting;
                         

    public slots:
        void OnConnectFinished ( int id, bool error )
        {
            m_connecting = false;
        }
        
    public:

        qtFtpServerContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_qftp( 0 )
        {
            m_connecting = true;
            int rc = m_qftp.connectToHost( rdn.c_str() );
            connect( &m_qftp,
                     SIGNAL( commandFinished(int,bool) ),
                     SLOT( OnConnectFinished(int,bool) ) );
            while( m_connecting )
            {
                Main::processAllPendingEvents();
            }
            m_qftp.login();
        }

        virtual ~qtFtpServerContext()
        {
        }

        QFtp& getQFTP()
        {
            return m_qftp;
        }
        virtual qtFtpServerContext* getServer()
        {
            return this;
        }
        
        
    };
    
    
    /*
     * Allow network hosts to be discovered at run time.
     */
    class FERRISEXP_CTXPLUGIN qtFtpRootContext
        :
        public networkRootContext<qtFtpServerContext>
    {
        typedef qtFtpRootContext                        _Self;
        typedef networkRootContext<qtFtpServerContext>  _Base;
    public:
        
        qtFtpRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn, true )
            {
            }
        
        virtual ~qtFtpRootContext()
            {
            }
    };
    
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    qtFtpServerContext*
    qtFtpContext::getServer()
    {
        qtFtpServerContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p;
    }

    QFtp&
    qtFtpContext::getQFTP()
    {
        return getServer()->getQFTP();
    }

    string
    qtFtpContext::getFTPPath()
    {
        string spath = getServer()->getDirPath();
        string lpath = getDirPath();
        string ret = lpath.substr( spath.length() );
        if( ret.empty() )
            ret = "/";
        return ret;
    }

    void
    qtFtpContext::OnListStarted ( int id )
    {
        EnsureStartReadingIsFired();
    }

    void
    qtFtpContext::OnListFinished ( int id, bool error )
    {
        EnsureStopReadingIsFired();
        updateMetaData();
    }
    
    void
    qtFtpContext::OnListInfo ( const QUrlInfo & i )
    {
        string rdn = tostr(i.name());
        
        if( rdn=="." || rdn==".." )
            return;
        
        // DEBUG << "OnListInfo() name:" << i.name() << " size:" << i.size() << endl;
        qtFtpContext* c = 0;
        c = priv_ensureSubContext( rdn.c_str(), c );
        c->constructObject( i );
    }
    
    
    void
    qtFtpContext::priv_read()
    {
        LG_CURL_D << "qtFtpContext::priv_read() path:" << getDirPath() << endl;
        
//        EnsureStartStopReadingIsFiredRAII _raii1( this );
        clearContext();

        QFtp& qf = getQFTP();
        string ftpPath = getFTPPath();
//        DEBUG << "ftpPath:" << ftpPath << endl;
        
        connect( &getQFTP(),
                 SIGNAL( listInfo ( const QUrlInfo& ) ),
                 SLOT( OnListInfo ( const QUrlInfo& ) ) );
        connect( &getQFTP(),
                 SIGNAL( commandStarted ( int )),
                 SLOT( OnListStarted ( int ) ));
        connect( &getQFTP(),
                 SIGNAL( commandFinished ( int,bool )),
                 SLOT( OnListFinished ( int,bool )));
        
        qf.list( ftpPath.c_str() );
    }

    void
    qtFtpContext::OnGeneralFinished ( int id, bool error )
    {
        m_waiting = false;
        DEBUG << "OnGeneralFinished() id:" << id << " error:" << error << endl;
    }
    
    void
    qtFtpContext::OnGetFinished ( int id, bool error )
    {
        DEBUG << "OnGetFinished()" << endl;
//        m_readBuffer->open(QBuffer::ReadOnly);
//        m_readBuffer->close();
        m_readBuffer->seek(0);
        DEBUG << "OnGetFinished(end)" << endl;
    }
    
    
    fh_istream
    qtFtpContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception)
    {
        QFtp& qf = getQFTP();
        string ftpPath = getFTPPath();

        m_readBuffer = new QBuffer( this );
        m_readBuffer->open(QBuffer::ReadWrite);
        int rc = qf.get( ftpPath.c_str(), m_readBuffer );
        DEBUG << "priv_getIStream() rc:" << rc << " ftpPath:" << ftpPath << endl;

        connect( &qf,
                 SIGNAL( commandFinished(int,bool) ),
                 SLOT( OnGetFinished(int,bool) ) );
        
        fh_istream ret = Factory::createIStreamFromQIODevice( m_readBuffer, true );
        return ret;
    }
    
    fh_iostream
    qtFtpContext::priv_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
    {
        return _Base::priv_getIOStream( m );
        // FIXME:
        QFtp& qf = getQFTP();
        string ftpPath = getFTPPath();

        fh_StreamToQIODevice qio = Factory::createStreamToQIODevice();
        
        int rc = qf.put( GetImpl(qio), ftpPath.c_str() );
        DEBUG << "priv_getIStream() rc:" << rc << " ftpPath:" << ftpPath << endl;

        fh_iostream oss = qio->getStream();
        return oss;
    }


    void
    qtFtpContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
        m["dir"] = SubContextCreator(
            SL_SubCreate_dir,
            "	<elementType name=\"dir\">\n"
            "		<elementType name=\"name\" default=\"new directory\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");
        m["file"] = SubContextCreator(
            SL_SubCreate_file,
            "	<elementType name=\"dir\">\n"
            "		<elementType name=\"name\" default=\"new directory\">\n"
            "			<dataTypeRef name=\"string\"/>\n"
            "		</elementType>\n"
            "	</elementType>\n");
    }
    
    fh_context
    qtFtpContext::SubCreate_file( fh_context c, fh_context md )
    {
        return _Base::SubCreate_file( c, md );
        // FIXME:
        string rdn = getStrSubCtx( md, "name", "" );
        return priv_readSubContext( rdn, true );
    }
    
    fh_context
    qtFtpContext::SubCreate_dir( fh_context c, fh_context md )
    {
        string rdn = getStrSubCtx( md, "name", "" );
        QFtp& qf = getQFTP();
        string ftpPath = getFTPPath();
        string fqfn = ftpPath + "/" + rdn;
        DEBUG << "fqfn:" << fqfn << endl;
        m_waiting = true;
        qf.mkdir( fqfn.c_str() );
        connect( &qf,
                 SIGNAL( commandFinished(int,bool) ),
                 SLOT( OnGeneralFinished(int,bool) ) );
        while( m_waiting )
            Main::processAllPendingEvents();
        DEBUG << "done fqfn:" << fqfn << endl;

        return priv_readSubContext( rdn, true );
    }
    
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
                Main::processAllPendingEvents();
                KDE::ensureKDEApplication();
                
                static qtFtpRootContext* c = 0;

                if( !c )
                {
                    LG_CURL_D << "Making FakeInternalContext(1) " << endl;
                    c = new qtFtpRootContext(0, "/");
            
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                    LG_CURL_D << "Making FakeInternalContext(3) " << endl;
                }

                fh_context ret = c;
                LG_CURL_D << "curl.brew() ret:" << ret->getURL() << endl;
                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_CURL_D << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
};

#include "libferrisqtftp_moc_impl.cpp"
