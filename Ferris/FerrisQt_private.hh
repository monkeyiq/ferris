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

    $Id: FerrisQt_private.hh,v 1.6 2011/05/01 21:30:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_QT_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_QT_PRIV_H_


#include <HiddenSymbolSupport.hh>
#include <TypeDecl.hh>
#include <FerrisStreams/Streams.hh>
#include <FerrisDOM.hh>

#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QtCore/QVariant>

#include <QNetworkAccessManager>
#include <QUrl>

namespace Ferris
{
    FERRISEXP_API std::string tostr( const QString& q );
    FERRISEXP_API std::string tostr( const QVariant& q );
    FERRISEXP_API std::string tostr( const QByteArray& q );
    FERRISEXP_API QByteArray toba( const std::string& s );
};

namespace std
{
    template < class ostream >
    ostream& operator<<( ostream& ss, QByteArray& ba )
    {
        ss.write( ba.data(), ba.size() );
        return ss;
    }
    template < class ostream >
    ostream& operator<<( ostream& ss, QString& s )
    {
        ss << ::Ferris::tostr(s);
        return ss;
    }
    // // pesky warnings about ambigu
    // template < class ostream >
    // ostream& operator<<( ostream& ss, const char* s )
    // {
    //     std::operator<<( ss, s );
    //     return ss;
    // }
};


namespace Ferris
{
//    FERRISEXP_API fh_stringstream& operator<<( fh_stringstream& S, const QString& q );

    FERRISEXP_API std::string URLencode( const std::string& d );

    FERRISEXP_API stringmap_t to_std_map( const QList<QPair<QString, QString> >& qm );
    FERRISEXP_API QUrl& addQueryItems( QUrl& u, const stringmap_t& m );

    // WARNING: XMLToJSON is partially implemented.
    FERRISEXP_API std::string XMLToJSON( fh_domdoc doc );
    FERRISEXP_API std::string stringmapToJSON( const stringmap_t& sm );
    FERRISEXP_API std::string contextToJSON( fh_context c,
                                             int recurse = 0,
                                             bool includeContent = true );
    FERRISEXP_API std::string contextToJSON( fh_context c,
                                             stringlist_t eatoinclude,
                                             int recurse = 0,
                                             bool includeContent = true );
    FERRISEXP_API std::string contextlistToJSON( fh_contextlist cl,
                                                 stringlist_t eatoinclude,
                                                 bool includeContent );
    FERRISEXP_API std::string contextlistToJSON( fh_contextlist cl,
                                                 bool includeContent );
    FERRISEXP_API void contextToJSONProcessContext( fh_context c,
                                                    QVariantMap& dm,
                                                    stringlist_t& an,
                                                    bool includeContent );
    FERRISEXP_API int httpResponse( QNetworkReply* reply );
    FERRISEXP_API QNetworkRequest setArgs( QNetworkRequest req, stringmap_t args );

    FERRISEXP_API stringmap_t JSONToStringMap( const std::string& jsontext );
    FERRISEXP_API QVariantMap JSONToQVMap( const std::string& jsontext );
    FERRISEXP_API stringmap_t JSONToStringMap( QVariantMap qm );
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    FERRISEXP_API void installQTMsgHandler();
    FERRISEXP_API void ensureQApplication();
    FERRISEXP_API QNetworkAccessManager* getQManager( QNetworkCookieJar* cjar = 0 );
    FERRISEXP_API QNetworkAccessManager* getQNonCachingManager();

    class FERRISEXP_API QNetworkResponseWaiter
    {
        GMainLoop* m_loop;
        long m_refCount;
        std::set< QNetworkReply* > m_alreadyUnblocked;
    public:
        QNetworkResponseWaiter();
        void block( QNetworkReply* r );
        void unblock( QNetworkReply* r );
    };

    void wait( QNetworkReply* r );


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    class StreamToQIODevice;
    FERRIS_SMARTPTR( StreamToQIODevice, fh_StreamToQIODevice );

    namespace Factory
    {
        FERRISEXP_API fh_StreamToQIODevice createStreamToQIODevice( std::streamsize totalPostSize = 0 );

        /**
         * Allow read access to a QIODevice through a std::iostream interface.
         */
        FERRISEXP_API fh_istream createIStreamFromQIODevice( QIODevice* qio, bool maintainPos = false  );
    };

    /**
     * Abstraction bringing together a writable fh_ostream for libferris
     * to use and a QIODevice for Qt to use to stream a POST over the network as
     * data is written to the fh_ostream.
     *
     * Do not use getStream() until after you use post() to start the network IO.
     *
     *
     * QNetworkAccessManager* qm = new QNetworkAccessManager(0);
     * QUrl u( getURL() );
     * u.addQueryItem("v", "2.0");
     * QNetworkRequest req(u);
     *
     * int ContentLength = 21;
     * req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(ContentLength).c_str() );
     * x = Factory::createStreamToQIODevice();
     * stringstream blobss;
     * blobss << "name=\"photo\"; ...;
     * 
     * QNetworkReply* reply = x->post( qm, req, blobss.str() );
     * connect( reply, SIGNAL( finished() ), SLOT( handleFinished() ) );
     *
     * fh_ostream oss = x->getStream();
     * oss << photo-jpeg-bytes;
     * x->writingComplete();
     */
    class StreamToQIODevice
        :
        public QIODevice,
        public Handlable
    {
        QNetworkResponseWaiter m_waiter;
        
        
    protected:
        bool m_userHasStream;
        
        // friend FERRISEXP_API fh_StreamToQIODevice
        // Factory::createStreamToQIODevice( std::streamsize totalPostSize = 0 );

        StreamToQIODevice( std::streamsize totalPostSize = 0 )
            :
            m_userHasStream( false )
            {
            }

        
        
    ////////////////////////    
    // Ferris side interface
    public:

        bool getUserHasStream()
            {
                return m_userHasStream;
            }
        
        QNetworkResponseWaiter& getWaiter()
            {
                std::cerr << "getWaiter() obj:" << (&m_waiter) << std::endl;
                return m_waiter;
            }
                
        /**
         * This can be called multiple times, every time the
         * return value drops scope, the class will try to write
         * anything new to the network
         */
        virtual fh_iostream getStream() = 0;
        
        /**
         * When you are fully done, possibly after calling getStream()
         * multiple times, call this to signal that the request is complete.
         */
        virtual void writingComplete() = 0;
        typedef sigc::signal1< void,
                               fh_StreamToQIODevice > WritingComplete_Sig_t;
        WritingComplete_Sig_t& getWritingCompleteSig()
            {
                return WritingComplete_Sig;
            }
        
        
        
        /**
         * Move all the Query parameters from request.url() into the post body
         * and setup a single streaming parameter in the post body using coreBlobDesc
         * as the header for that blob. You can stream to the POST using getStream()
         * and writingComplete() when done.
         */
        virtual QNetworkReply* post( QNetworkAccessManager* qm,
                                     QNetworkRequest& request,
                                     const std::string& coreBlobDesc ) = 0;

        virtual QNetworkReply* put( QNetworkAccessManager* qm,
                                    QNetworkRequest& request ) = 0;
        
        /**
         * Once you have written all your data, the write stream closed,
         * call here to read the reply.
         */
        virtual QByteArray readResponse() = 0;
        
        /**
         * You don't have to call this method, getStream() will call
         * it for you when the stream drops out of scope.
         */
        virtual void dataWritten() = 0;

        /**
         * Add an extra complete data chunk to send to the server
         * this is wrapped in the mime border and should be everything
         * you want between two mime borders, not including the border
         */
        virtual void addExtraDataChunk( const std::string& s ) = 0;
        virtual void setContentType( const std::string& s ) = 0;

        
    private:
        WritingComplete_Sig_t WritingComplete_Sig;
        
    };
    

    
    
};

#endif
