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
#include <FerrisKDE.hh>
#include <FerrisDOM.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>
#include <FerrisWebServices_private.hh>

#include "libferriskio_shared.hh"

using namespace std;

#define DEBUG cerr
//#define DEBUG   LG_KIO_D
#define WARNING LG_KIO_W

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
    
    
    KioBaseContext::KioBaseContext( Context* parent, const std::string& rdn )
        : _Base( parent, rdn )
        , m_isDir(true)
        , m_sz(0)
    {
        DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
        createStateLessAttributes();
    }
    void
    KioBaseContext::setup( const KIO::UDSEntry& e )
    {
        m_isDir = e.isDir();
        m_sz = e.numberValue( KIO::UDSEntry::UDS_SIZE, -1 );
    }

    fh_istream
    KioBaseContext::SL_getSizeIStream( KioBaseContext* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->m_sz;
        return ss;
    }
    

    void
    KioBaseContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            DEBUG << "setting up stateless ea" << endl;
            
#define SLEA tryAddStateLessAttribute         

            tryAddStateLessAttribute( "size",
                                      SL_getSizeIStream,
                                      FXD_FILESIZE );
            
#undef SLEA
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );

        }
    }
    
    void
    KioBaseContext::enter_loop()
    {
        QEventLoop eventLoop;
        connect(this, SIGNAL(leaveModality()),
                &eventLoop, SLOT(quit()));
        cerr << "enter loop" << endl;
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
        cerr << "exit loop" << endl;
    }
        
    void
    KioBaseContext::slotResult( KJob* job )
    {
        if(job->error()) 
        {
            DEBUG << "x result:" << tostr(job->errorString()) << endl;
        }

        emit leaveModality();
        cerr << "emitted." << endl;
    }


    void
    KioBaseContext::data_fromBuffer(KIO::Job *job, const QByteArray &data)
    {
        m_fromBufferTarget << tostr(data);
    }
    
    
    fh_istream
    KioBaseContext::priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               std::exception)
            {
                DEBUG << "priv_getIStream()" << endl;
                fh_stringstream ss;
                m_fromBufferTarget = ss;
                std::string path = getKIOPath();
                DEBUG << "priv_getIStream() path:" << path << endl;
                TransferJob* job = KIO::get( QUrl(path.c_str()));
                connect( job, SIGNAL( data(KIO::Job *, const QByteArray &) ),
                         this, SLOT( data_fromBuffer(KIO::Job *, const QByteArray &) ) );
                connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
                job->start();
                enter_loop();
                return ss;
            }
    
        
    void
    KioBaseContext::entries(KIO::Job *job, const KIO::UDSEntryList& entries)
    {
        cerr << "entries()" << endl;
        KIO::UDSEntryList::ConstIterator it = entries.begin();
        const KIO::UDSEntryList::ConstIterator end = entries.end();
        for (; it != end; ++it)
        {
            const KIO::UDSEntry& entry = *it;
            QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );

            DEBUG << "f: " << tostr(name) << endl;
            KioBaseContext* cc = 0;
            std::string rdn = tostr(name);
            cc = priv_ensureSubContext( rdn, cc );
            cc->setup( entry );
        }
    }

    std::string
    KioBaseContext::getKIOPath()
    {
        std::string path = getDirPath();
        path = path.substr( 1 );
        path = Util::replace_first( path, ":", ":/" );
        path = Util::replace_first( path, "://", ":/" );
        return path;
    }
    
    
    void
    KioBaseContext::priv_read()
    {
        if( !m_isDir )
        {
            stringstream ss;
            ss << "Can not read file as directory through KIO path:" << getKIOPath();
            Throw_FerrisNotReadableAsContext( tostr(ss), this );
            
        }
        
        bool includeHidden = false;
        std::string path = getKIOPath();
        ListJob* job = KIO::listDir( KUrl(path.c_str()) ); // ::KIO::DefaultFlags, includeHidden );
        DEBUG << "priv_read(b) m_isDir:" << m_isDir << " path:" << path << " url:" << getURL() << endl;
        connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
                 this, SLOT( entries( KIO::Job*, const KIO::UDSEntryList& ) ) );
        connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
        job->start();
        enter_loop();

        _Base::priv_read();
        
    }
    
    

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    KioRootContext::KioRootContext( Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn )
    {
    }

    void
    KioRootContext::priv_read()
    {
        DEBUG << "priv_read(r) path:" << getDirPath() << endl;
        Context::priv_read();
        
    }
    
    fh_context
    KioRootContext::priv_getSubContext( const string& rdn )
        throw( NoSuchSubContext )
    {
        DEBUG << "priv_getSubContext() rdn:" << rdn << endl;
//        if( rdn == "file:" )
        {
            KioBaseContext* cc = 0;
            cc = priv_ensureSubContext( rdn, cc );
            return cc;
        }
        
        return _Base::priv_getSubContext( rdn );
    }
    
            
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

                static KioBaseContext* c = 0;
                if( !c )
                {
                    KDE::ensureKDEApplication();

                    c = new KioRootContext(0, "/" );
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
                WARNING << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
