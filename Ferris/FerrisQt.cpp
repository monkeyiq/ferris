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

    $Id: FerrisQt.cpp,v 1.6 2011/11/09 21:31:14 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/Ferris.hh>
#include <Ferris/Configuration.hh>

#include "FerrisQt_private.hh"
#include <string>

#include <QNetworkProxy>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QBuffer>

#include <QCoreApplication>

#include <qjson/serializer.h>
#include <qjson/parser.h>

#define DEBUG LG_QIO_D 

using namespace std;

#define DEBUGGING_UPLOAD_STREAMING
#undef  DEBUGGING_UPLOAD_STREAMING

namespace Ferris
{
    std::string URLencode( const std::string& d )
    {
        QByteArray ba = QUrl::toPercentEncoding( d.c_str(), "", " " );
        return tostr(ba);
        // QUrl u(d.c_str());
        // return tostr(u.toEncoded());
    }
    

    std::string tostr( const QString& q )
    {
        return q.toUtf8().data();
    }
    std::string tostr( const QVariant& q )
    {
        return tostr( q.toString() );
    }
    // std::string tostr( const QByteArray& q )
    // {
    //     return (std::string)q;
    // }
    
    std::string tostr( const QByteArray& q )
    {
        std::stringstream ret;
        ret.write( q.data(), q.size() );
        return ret.str();
    }

    QByteArray toba( const std::string& s )
    {
        QByteArray ret( s.c_str(), s.length() );
        return ret;
    }
    
    
    // fh_stringstream& operator<<( fh_stringstream& S, const QString& q )
    // {
    //     S << tostr(q);
    //     return S;
    // }

    stringmap_t to_std_map( const QList<QPair<QString, QString> >& qm )
    {
        map< string, string > ret;
        for( QList<QPair<QString, QString> >::const_iterator qi = qm.begin(); qi!=qm.end(); ++qi )
        {
            ret.insert( make_pair( tostr(qi->first), tostr(qi->second)));
        }
        return ret;
    }

    QUrl& addQueryItems( QUrl& u, const stringmap_t& m )
    {
        for( stringmap_t::const_iterator mi = m.begin(); mi!=m.end(); ++mi )
        {
            u.addQueryItem( mi->first.c_str(), mi->second.c_str() );
        }
        return u;
    }

    std::string toJSONKey( const std::string& s )
    {
        string ret = s;
        ret = Util::replace_all( ret, '-', '_' );
        ret = Util::replace_all( ret, ':', '_' );
        return ret;
    }
    
    void contextToJSONProcessContext( fh_context c,
                                      QVariantMap& dm,
                                      stringlist_t& an,
                                      bool includeContent )
    {
        typedef stringlist_t::iterator I;
        for( I ai = an.begin(); ai != an.end(); ++ai )
        {
            string k = *ai;
            if( !includeContent && k == "content" )
                continue;
            if( k == "as-xml" )
                continue;
            if( k == "as-rdf" )
                continue;
            if( k == "as-json" )
                continue;
            
            try
            {
                string v = getStrAttr( c, k, "", true, true );
                dm[ toJSONKey( k ).c_str() ] = v.c_str();
            }
            catch( exception& e )
            {
            }
        }
    }

    stringlist_t& getDefaultEAToIncludeJSON()
    {
        static stringlist_t eatoinclude;
        if( eatoinclude.empty() )
        {
            eatoinclude.push_back("url");
            eatoinclude.push_back("name");
            eatoinclude.push_back("size");
            eatoinclude.push_back("size-human-readable");
            eatoinclude.push_back("mtime");
            eatoinclude.push_back("atime");
            eatoinclude.push_back("ctime");
            eatoinclude.push_back("mtime-display");
        }
        return eatoinclude;
    }
    
    std::string contextlistToJSON( fh_contextlist cl,
                                   bool includeContent )
    {
        stringlist_t& eatoinclude = getDefaultEAToIncludeJSON();
        return contextlistToJSON( cl, eatoinclude, includeContent );
    }


    static QVariantMap XMLToJSONVisit( fh_domdoc doc, DOMElement* e )
    {
        QVariantMap dm;
        DOMNamedNodeMap* attributes = e->getAttributes();
        int attrCount = attributes->getLength();
        for (int i = 0; i < attrCount; i++)
        {
            DOMNode* attribute = attributes->item(i);
            const string& n = tostr(attribute->getNodeName());
            const string& v = tostr(attribute->getNodeValue());
            dm[ n.c_str() ] = v.c_str();
        }
        return dm;
    }
    
    std::string XMLToJSON( fh_domdoc doc )
    {
        QVariantList dl;
        DOMElement* root = doc->getDocumentElement();
        dl << XMLToJSONVisit( doc, root );

        static int i = 0;
        XML::domnode_list_t nodes = XML::getChildren( root );
        for( XML::domnode_list_t::iterator di = nodes.begin(); di != nodes.end(); ++di )
        {
            DOMElement* e = (DOMElement*)*di;
            dl << XMLToJSONVisit( doc, e );
        }
        QJson::Serializer zz;
        QByteArray ba = zz.serialize( dl );
        string ret = tostr(ba);
        return ret;
    }
    
    
    
    std::string stringmapToJSON( const stringmap_t& sm )
    {
        QVariantMap dm;
        for( stringmap_t::const_iterator si = sm.begin(); si != sm.end(); ++si )
        {
            string k = si->first;
            string v = si->second;
            dm[ k.c_str() ] = v.c_str();
        }

        QJson::Serializer zz;
        QByteArray ba = zz.serialize( dm );
        string ret = tostr(ba);
        return ret;
        
    }
    
    
    std::string contextlistToJSON( fh_contextlist cl,
                                   stringlist_t eatoinclude,
                                   bool includeContent )
    {
        QVariantMap top;
        stringlist_t& an = eatoinclude;
        for( Context::iterator ci = cl->begin(); ci != cl->end(); ++ci )
        {
            QVariantMap dm;
            std::string rdn = (*ci)->getURL();
            contextToJSONProcessContext( *ci, dm, an, includeContent );
            top[ rdn.c_str() ] = dm;
        }
        QJson::Serializer zz;
        QByteArray ba = zz.serialize( top );
        string ret = tostr(ba);
        return ret;
    }
    
    
    std::string contextToJSON( fh_context c,
                               stringlist_t eatoinclude,
                               int recurse,
                               bool includeContent )
    {
        std::string ret;
        QVariantMap top;

        // typedef AttributeCollection::AttributeNames_t::iterator I;
        // AttributeCollection::AttributeNames_t an;
        // c->getAttributeNames( an );
        stringlist_t& an = eatoinclude;

        if( recurse == 1 )
        {
            for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
            {
                QVariantMap dm;
                std::string rdn = (*ci)->getDirName();
                contextToJSONProcessContext( *ci, dm, an, includeContent );
                top[ rdn.c_str() ] = dm;
            }
        }
        else
        {
            QVariantMap dm;
            contextToJSONProcessContext( c, dm, an, includeContent );
            top = dm;
        }
        
        QJson::Serializer zz;
        QByteArray ba = zz.serialize( top );
        ret = tostr(ba);
        return ret;
    }
    

    std::string contextToJSON( fh_context c,
                               int recurse,
                               bool includeContent )
    {
        stringlist_t& eatoinclude = getDefaultEAToIncludeJSON();
        return contextToJSON( c, eatoinclude, recurse, includeContent );
    }
    
    int httpResponse( QNetworkReply* reply )
    {
        return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    }
    QNetworkRequest
    setArgs( QNetworkRequest req, stringmap_t args )
    {
        QUrl u = req.url();
        for( stringmap_t::iterator iter = args.begin(); iter != args.end(); ++iter )
        {
            u.addQueryItem( iter->first.c_str(), iter->second.c_str() );
        }
        req.setUrl( u );
        return req;
    }
    stringmap_t JSONToStringMap( const std::string& jsontext )
    {
        stringmap_t ret;
        QVariantMap qm = JSONToQVMap( jsontext );
        ret = JSONToStringMap( qm );
        return ret;
    }
    QVariantMap JSONToQVMap( const std::string& jsontext )
    {
        QVariantMap ret;
        QJson::Parser parser;
        bool ok;
        ret = parser.parse( jsontext.c_str(), &ok ).toMap();
        if( ok )
        {
        }
        return ret;
    }
    stringmap_t JSONToStringMap( QVariantMap qm )
    {
        stringmap_t ret;
        for(QVariantMap::const_iterator iter = qm.begin(); iter != qm.end(); ++iter)
        {
            ret[ tostr(iter.key()) ] = tostr(iter.value());
        }
        return ret;
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    void OnGenericStreamClosed( FerrisLoki::Handlable* a );

    template<
        typename _CharT,
        typename _Traits = std::char_traits < _CharT >,
        typename _Alloc  = std::allocator   < _CharT >
        >
    class  StreamToQIODevice_streambuf
        :
        public ferris_stringbuf<_CharT, _Traits>
    {
    
        typedef std::basic_stringbuf<_CharT, _Traits, _Alloc> sb;
    
    public:
    
        typedef std::char_traits<_CharT>          traits_type;
        typedef typename traits_type::int_type    int_type;
        typedef typename traits_type::char_type   char_type;
        typedef typename traits_type::pos_type    pos_type;
        typedef typename traits_type::off_type    off_type;
        typedef ferris_stringbuf<_CharT, _Traits>                      _Base;
        typedef StreamToQIODevice_streambuf<_CharT, _Traits, _Alloc>   _Self;
        typedef std::basic_string<_CharT, _Traits, _Alloc>  _String;

        StreamToQIODevice* m_iodev;
        int m_streamAutoClosing;
        
        void setDev( StreamToQIODevice* iodev )
            {
                m_iodev = iodev;
            }
        ofstream debugStream;
        
        explicit
        StreamToQIODevice_streambuf( std::ios_base::openmode m = std::ios_base::in | std::ios_base::out )
            :
            _Base(m)
            , m_iodev( 0 )
#ifdef DEBUGGING_UPLOAD_STREAMING
            , debugStream( "/tmp/debug-stream" )
#endif
            {
                LG_QIO_D << "StreamToQIODevice_streambuf() this :" << (void*)this << endl;
                m_streamAutoClosing = 0;
            }
    
        explicit
        StreamToQIODevice_streambuf( const _String& s,
                                     std::ios_base::openmode m = std::ios_base::in | std::ios_base::out )
            :
            _Base(s,m)
            , m_iodev( 0 )
#ifdef DEBUGGING_UPLOAD_STREAMING
            , debugStream( "/tmp/debug-stream" )
#endif
            {
                LG_QIO_D << "StreamToQIODevice_streambuf() this :" << (void*)this << endl;
                m_streamAutoClosing = 0;
            }
    
        virtual ~StreamToQIODevice_streambuf()
            {
                LG_QIO_D << "~StreamToQIODevice_streambuf() this :" << (void*)this << endl;
#ifdef DEBUGGING_UPLOAD_STREAMING
                debugStream << flush;
                debugStream.close();
#endif
            }
        void handleWrite();
        streamsize xsputn( const char_type* s, std::streamsize n )
            {
                streamsize ret = _Base::xsputn( s, n );

#ifdef DEBUGGING_UPLOAD_STREAMING
                debugStream.write( s, n );
#endif
                LG_QIO_D << "xsputn() n:" << n << endl;
                handleWrite();
                return ret;
            }
        
        int_type overflow( int_type c )
            {
                int_type ret = _Base::overflow( c );

                LG_QIO_D << "overflow(c) c:" << c << " ret:" << ret << endl;
//                handleWrite();
                return ret;
            }

        virtual pos_type seekoff( off_type off,
                                  ios_base::seekdir way,
                                  ios_base::openmode m = ios_base::in | ios_base::out )
            {
                LG_QIO_D << "seekoff() off:" << off << " way:" << way << " m:" << m << endl;
                return _Base::seekoff( off, way, m );
            }
        virtual pos_type seekpos( pos_type pos,
                                  ios_base::openmode m = ios_base::in | ios_base::out )
            {
                pos_type ret = pos;
                
                if( m & ios_base::out && !pos )
                {
                }
                else
                {
                    ret = _Base::seekpos( pos, m );
                }
                LG_QIO_D << "seekpos() pos:" << pos
                         << " ret:" << ret
                         << " m:" << m
                         << " in:" << ios_base::in
                         << " out:" << ios_base::out
                         << endl;
                return ret;
            }
        
        
        
        void emitClosing()
            {
                OnGenericStreamClosed( this );
            }
        
        
        typedef __uint32_t ref_count_t;
        
        virtual ref_count_t AddRef()
            {
//                cerr << "StreamToQIODevice_streambuf::Addref() ref_count:" << this->ref_count << endl;
//                BackTrace();
                return _Base::AddRef();
            }
        virtual ref_count_t Release()
            {
//                cerr << "StreamToQIODevice_streambuf::Release() ref_count:" << this->ref_count << endl;
                if( !m_streamAutoClosing )
                {
                    if( this->ref_count == 3 && m_iodev && m_iodev->getUserHasStream() )
                    {
                        cerr << "STREAM IS CLOSING AUTOMATICALLY!" << endl;
                        LG_QIO_I << "STREAM IS CLOSING AUTOMATICALLY!" << endl;
                        ref_count_t ret = _Base::Release();

                        m_streamAutoClosing = 1;
                    
                        if( m_iodev )
                        {
                            emitClosing();
                            // m_iodev->writingComplete();
                        }
                        return ret;
                    }
                }
                return _Base::Release();
            }
    };

template<
    typename _CharT,
    typename _Traits = std::char_traits < _CharT >,
    typename _Alloc  = std::allocator   < _CharT >
    >
class  StreamToQIODevice_stringstream
    :
        public Ferris_iostream<_CharT, _Traits>,
//        public StreamHandlableSigEmitter< StreamToQIODevice_stringstream<_CharT, _Traits, _Alloc> >,
        public stringstream_methods<_CharT, _Traits, io_ferris_stream_traits< _CharT, _Traits > >
{
    typedef StreamToQIODevice_streambuf<_CharT, _Traits, _Alloc> ss_impl_t;
    typedef Loki::SmartPtr<  ss_impl_t,
                             FerrisLoki::FerrisExRefCounted,
                             Loki::DisallowConversion,
                             FerrisLoki::FerrisExSmartPointerChecker,
                             FerrisLoki::FerrisExSmartPtrStorage > ss_t;
    ss_t ss;
    typedef Ferris_commonstream<_CharT, _Traits> _CS;
    typedef io_ferris_stream_traits< _CharT, _Traits > _FerrisStreamTraits;

public:

    typedef _Traits                           traits_type;
    typedef typename traits_type::int_type    int_type;
    typedef typename traits_type::char_type   char_type;
    typedef typename traits_type::pos_type    pos_type;
    typedef typename traits_type::off_type    off_type;

    typedef stringstream_methods<
        char_type, traits_type,
        io_ferris_stream_traits< char_type, traits_type > > delegating_methods;
    typedef StreamToQIODevice_stringstream< _CharT, _Traits, _Alloc > _Self;


    explicit StreamToQIODevice_stringstream(
        ss_t& _ss )
        :
        ss( _ss )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
            this->seekg( 0 );
            this->seekp( 0 );
        }


    explicit StreamToQIODevice_stringstream(
        std::ios_base::openmode m = _FerrisStreamTraits::DefaultOpenMode )
        :
        ss( new ss_impl_t(m) )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
        }
        
    explicit StreamToQIODevice_stringstream(
        const std::basic_string<_CharT, _Traits>& s,
        std::ios_base::openmode m = _FerrisStreamTraits::DefaultOpenMode )
        :
        ss( new ss_impl_t( s, m ) )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
        }

    StreamToQIODevice_stringstream( const StreamToQIODevice_stringstream& rhs )
        :
        ss( rhs.ss )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
        }

        
    virtual ~StreamToQIODevice_stringstream()
        {
        }


    StreamToQIODevice_stringstream& operator=( const StreamToQIODevice_stringstream& rhs )
        {
//                 LG_QIO_D << "StreamToQIODevice_stringstream& op = " << endl;

            this->setsb( &rhs );
            this->init( _CS::sb );
            this->exceptions( std::ios_base::goodbit );
            this->clear( rhs.rdstate() );
            this->copyfmt( rhs );
            return *this;
        }

    
    _Self* operator->()
        {
            return this;
        }

    
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    
    
    ss_impl_t*
    rdbuf() const
        {
            return GetImpl(ss);
        }

    std::basic_string<_CharT, _Traits> str() const
        {
            return ss->str();
        }

    void str(const std::basic_string<_CharT, _Traits>& s )
        {
            ss->str(s);
        }

    enum
    {
        stream_readable = true,
        stream_writable = true
    };

};
    typedef StreamToQIODevice_stringstream<char>   fh_StreamToQIODevice_stringstream;
    


    template<
        typename _CharT,
        typename _Traits = std::char_traits<_CharT>
    >
    class  StreamToQIODevice_IOStream
        :
        public Ferris_iostream< _CharT, _Traits >
    {
        typedef std::basic_streambuf<_CharT, _Traits> sb_t;
        typedef Ferris_iostream<_CharT, _Traits> _CS;
        typedef Ferris_iostream< _CharT, _Traits > _Base;
        typedef StreamToQIODevice_IOStream<_CharT, _Traits>   _Self;

        StreamToQIODevice* m_iodev;
        
    public:

        typedef std::char_traits<_CharT> traits_type;
        typedef typename traits_type::int_type    int_type;
        typedef typename traits_type::char_type   char_type;
        typedef typename traits_type::pos_type    pos_type;
        typedef typename traits_type::off_type    off_type;
        typedef emptystream_methods< _CharT, _Traits > delegating_methods;

        void setDev( StreamToQIODevice* iodev )
            {
                m_iodev = iodev;
            }
        StreamToQIODevice_IOStream()
            :
            m_iodev( 0 )
            { }

        template< class _Alloc >
        StreamToQIODevice_IOStream(
            ferris_stringbuf< char_type, traits_type, _Alloc >* streambuf )
            :
            m_iodev( 0 )
            {
                this->setsbT( streambuf );
                this->init( _CS::sb );
                this->exceptions( std::ios_base::goodbit );
                this->clear();
            }
        
        StreamToQIODevice_IOStream( const StreamToQIODevice_IOStream& rhs )
            :
            m_iodev( 0 )
            {
                this->setsb( &rhs );
                this->init( _CS::sb );
                this->exceptions( std::ios_base::goodbit );
                this->clear( rhs.rdstate() );
                this->copyfmt( rhs );

                m_iodev = rhs.iodev;
            }

        StreamToQIODevice_IOStream& operator=( const StreamToQIODevice_IOStream& rhs )
            {
                this->setsb( &rhs );
                this->init( _CS::sb );
                this->exceptions( std::ios_base::goodbit );
                this->clear( rhs.rdstate() );
                this->copyfmt( rhs );
                this->m_iodev = rhs.iodev;
                return *this;
            }

        virtual ~StreamToQIODevice_IOStream();
    };


    /****************************************/
    /****************************************/
    /****************************************/
    
    class StreamToQIODeviceImpl
        :
        public StreamToQIODevice
    {
        Q_OBJECT
        typedef StreamToQIODeviceImpl _Self;
        
        mutable fh_StreamToQIODevice_stringstream m_stream;
        bool m_streamClosed;
        streamsize m_endpos;
        streamsize m_totalPostSize;
        streamsize m_uploadProgress;
        QNetworkResponseWaiter m_queueSizeWaiter;
        string m_prefixString;
        string m_postfixString;
        bool m_forceNoBlock;
        stringlist_t m_extraDataChunks;
        string m_contentType;
        QNetworkReply* m_reply;
        bool m_replyFinished;

        ofstream QDebugStream;
        
        ////////////////////////    
        // Ferris side interface
    public:
        StreamToQIODeviceImpl( streamsize totalPostSize = 0 );
        virtual ~StreamToQIODeviceImpl();
        void OnStreamClosed( fh_istream& ss, std::streamsize tellp  );
        virtual fh_iostream getStream();
        virtual void writingComplete();
        virtual QNetworkReply* post( QNetworkAccessManager* qm,
                                     QNetworkRequest& request,
                                     const std::string& coreBlobDesc );
        virtual QNetworkReply* put( QNetworkAccessManager* qm,
                                    QNetworkRequest& request );
        
        virtual QByteArray readResponse();
        virtual void dataWritten();

        virtual void addExtraDataChunk( const std::string& s )
            {
                m_extraDataChunks.push_back(s);
            }
        virtual void setContentType( const std::string& s )
            {
                m_contentType = s;
            }
        

        
        ////////////////////////    
        // QT side interface
    public:
        virtual bool isSequential () const;
        virtual bool atEnd () const;
        virtual qint64 bytesAvailable () const;
        virtual qint64 size () const;
    
    protected:
        virtual qint64 readData ( char * data, qint64 maxSize );
        virtual qint64 writeData ( const char * data, qint64 maxSize );

                                                                      
    public slots:
        void handleFinished()
            {
                cerr << "handleFinished()" << endl;
                LG_QIO_I << "handleFinished(1)" << endl;
                QNetworkReply* r = dynamic_cast<QNetworkReply*>(sender());
                LG_QIO_I << "handleFinished(1) r:" << r << endl;
                getWaiter().unblock(r);
                LG_QIO_I << "handleFinished(2)" << endl;
                m_replyFinished = true;

            }
        void uploadProgress ( qint64 bytesSent, qint64 bytesTotal );
        
        // void readChannelFinished ();
        // void readyRead ();
    };


    StreamToQIODeviceImpl::StreamToQIODeviceImpl( streamsize totalPostSize )
        :
        m_totalPostSize( totalPostSize ),
        m_streamClosed( false ),
        m_endpos( 0 ),
        m_forceNoBlock( false ),
        m_uploadProgress( 0 ),
        m_contentType( "multipart/form-data" ),
        m_reply( 0 ),
        m_replyFinished( false )
#ifdef DEBUGGING_UPLOAD_STREAMING
        , QDebugStream( "/tmp/debug-stream-qio-data-read" )
#endif
//    , m_stream( 0, 0 )
    {
        open( QIODevice::ReadOnly );

        // int fd = Shell::generateTempFD( "streamqio-" );
        // int closeFD = 1;
        // m_stream.setup( this, fd, closeFD );

        m_stream.rdbuf()->setDev( this );

        m_stream.getCloseSig().connect(
            sigc::mem_fun(*this, &_Self::OnStreamClosed ) ); 
    }

    StreamToQIODeviceImpl::~StreamToQIODeviceImpl()
    {
//        cerr << "~StreamToQIODeviceImpl()" << endl;
//        BackTrace();

#ifdef DEBUGGING_UPLOAD_STREAMING
        QDebugStream << flush;
        QDebugStream.close();
#endif
    }


    void
    StreamToQIODeviceImpl::OnStreamClosed( fh_istream& ss, std::streamsize tellp  )
    {
        LG_QIO_I << "StreamToQIODeviceImpl::OnStreamClosed()" << endl;
        writingComplete();
    }
    
    fh_iostream
    StreamToQIODeviceImpl::getStream()
    {
        m_userHasStream = 1;
        return m_stream;

        // StreamToQIODevice_IOStream<char> ret( m_stream.rdbuf() );
        // ret.setDev( this );
        // return ret;
    }

    void
    StreamToQIODeviceImpl::uploadProgress ( qint64 bytesSent, qint64 bytesTotal )
        {
            LG_QIO_D << "uploadProgress... bytesSent:" << bytesSent << " total:" << bytesTotal << endl;
            m_uploadProgress = bytesSent;
        }

    QByteArray
    StreamToQIODeviceImpl::readResponse()
    {
        LG_QIO_I << "StreamToQIODeviceImpl::readResponse() blocking m_replyFinished:" << m_replyFinished << endl;
        if( !m_replyFinished )
            getWaiter().block( m_reply );
        LG_QIO_I << "StreamToQIODeviceImpl::readResponse() un blocked" << endl;
        QByteArray ba = m_reply->readAll();
        LG_QIO_I << "StreamToQIODeviceImpl::readAll()" << endl;
        LG_QIO_I << "reply->error:" << m_reply->error() << endl;
        LG_QIO_I << "reply->size:" << ba.size() << endl;
        return ba;
    }
    

    QNetworkReply*
    StreamToQIODeviceImpl::put( QNetworkAccessManager* qm,
                                 QNetworkRequest& req )
    {
        m_uploadProgress = 0;
        QUrl u = req.url();
        req.setUrl( u );

        QNetworkReply* reply = qm->put( req, this );
        m_reply = reply;
        m_replyFinished = false;
        connect( (QObject*)reply,
                 SIGNAL( finished() ),
                 SLOT( handleFinished() ) );
        connect( (QObject*)reply,
                 SIGNAL( uploadProgress(qint64,qint64) ),
                 SLOT( uploadProgress(qint64,qint64) ) );

        m_forceNoBlock = false;
        return reply;
    }
    
    
    QNetworkReply*
    StreamToQIODeviceImpl::post( QNetworkAccessManager* qm,
                                 QNetworkRequest& req,
                                 const std::string& coreBlobDesc )
    {
        LG_QIO_D << "StreamToQIODeviceImpl::post(top)" << endl;
        
        m_uploadProgress = 0;
        QUrl u = req.url();

        string linesep = "\n";
        m_prefixString = "";
        m_postfixString = "";
        string boundry = "----------------------------7bccea7821d6";
        m_postfixString = linesep + "--" + boundry + "--\n";
    
        {
            stringstream ss;
            stringmap_t argmap = to_std_map( u.queryItems() );
            for( stringmap_t::iterator si = argmap.begin(); si!=argmap.end(); ++si )
            {
                ss << "--" << boundry << linesep;
                ss << "Content-Disposition: form-data; name=\"" << si->first << "\"" << linesep;
                ss << linesep;
                ss << si->second << linesep;
//                if( si->first != "ticket_id" )
                u.removeQueryItem( si->first.c_str() );
            }

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
            // FIXME:

            // ss << "--" << boundry << linesep;
            // ss << "Content-Disposition: form-data; name=\"" << "extrajunk" << "\"" << linesep;
            // ss << linesep;
            // for( int i=0; i<4096; ++i )
            //     ss << "X";
            // ss << linesep;
            
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

            
            DEBUG << "m_extraDataChunks.sz: " << m_extraDataChunks.size() << endl;
            for( stringlist_t::iterator si = m_extraDataChunks.begin(); si != m_extraDataChunks.end(); ++si )
            {
                ss << "--" << boundry << linesep;
                ss << *si;

                LG_QIO_D << "ADDING CHUNK:" << *si << endl;
            }
            
            ss << "--" << boundry << linesep;
            ss << coreBlobDesc << linesep;
            ss << linesep;
        
            m_prefixString = ss.str();
        }

        if( !req.header(QNetworkRequest::ContentLengthHeader).isNull()  )
        {
            int cl = toint( tostr(req.header(QNetworkRequest::ContentLengthHeader)));
            LG_QIO_D << "old cl:" << cl << endl;
            cl += m_prefixString.length() + m_postfixString.length();
//            cl += coreBlobDesc.length();
            LG_QIO_D << " m_prefixString.sz:" << m_prefixString.length() << endl;
            LG_QIO_D << "m_postfixString.sz:" << m_postfixString.length() << endl;
            LG_QIO_D << "coreBlobDesc.sz:" << coreBlobDesc.length() << endl;
            LG_QIO_D << " new cl:" << cl << endl;
            req.setHeader(QNetworkRequest::ContentLengthHeader, tostr(cl).c_str() );
        }


        
        req.setUrl( u );
        {
            stringstream ss;
            ss << "" << m_contentType << "; boundary=" << boundry;
//            ss << "multipart/form-data; boundary=" << boundry;
            req.setHeader(QNetworkRequest::ContentTypeHeader, ss.str().c_str() );
        }

        
        LG_QIO_D << "StreamToQIODeviceImpl::post(action!)" << endl;
        QNetworkReply* reply = qm->post( req, this );
        m_reply = reply;
        m_replyFinished = false;
        connect( (QObject*)reply,
                 SIGNAL( finished() ),
                 SLOT( handleFinished() ) );
        connect( (QObject*)reply,
                 SIGNAL( uploadProgress(qint64,qint64) ),
                 SLOT( uploadProgress(qint64,qint64) ) );

        LG_QIO_D << "prefix:" << m_prefixString << endl;
        LG_QIO_D << "postfix:" << m_postfixString << endl;

        m_forceNoBlock = true;
        m_stream << m_prefixString << flush;
        m_forceNoBlock = false;
    
        return reply;
    }


    void
    StreamToQIODeviceImpl::writingComplete()
    {
        if( m_streamClosed )
            return;
        
        m_stream << flush;
        cerr << "StreamToQIODeviceImpl::writingComplete() 1" << endl;
        LG_QIO_D << ".................. WRITINGCOMPLETE ................ " << endl;
//        BackTrace();
        
        m_forceNoBlock = true;
        cerr << "StreamToQIODeviceImpl::writingComplete() 1b" << endl;
        cerr << "StreamToQIODeviceImpl::writingComplete() 1c:" << m_postfixString << endl;
        cerr << "StreamToQIODeviceImpl::writingComplete() 1d" << endl;
        m_stream << m_postfixString << flush;
        cerr << "StreamToQIODeviceImpl::writingComplete() 1c" << endl;
        m_forceNoBlock = false;
    
        cerr << "StreamToQIODeviceImpl::writingComplete() 2" << endl;
        dataWritten();
        cerr << "StreamToQIODeviceImpl::writingComplete() 3" << endl;

        streamsize tellg = m_stream->tellg();
        m_stream->seekg(0, ios::end);
        m_endpos = m_stream->tellg();
        m_stream->seekg(tellg, ios::beg);
        m_streamClosed = true;
        cerr << "StreamToQIODeviceImpl::writingComplete() 4" << endl;
        LG_QIO_D << "writingComplete() end." << endl;
//        LG_QIO_D << "writingComplete() m_stream:" << m_stream.str() << endl;

        emit readChannelFinished();
        cerr << "StreamToQIODeviceImpl::writingComplete() 5" << endl;
#undef emit
        getWritingCompleteSig().emit( 0 );
    }

    void
    StreamToQIODeviceImpl::dataWritten()
    {
        LG_QIO_D << "dataWritten(top)" << endl;
        if( m_forceNoBlock )
        {
            LG_QIO_D << "dataWritten(m_forceNoBlock)" << endl;
            return;
        }
        
        // FIXME: Pause here until Qt has slurped out
        // all of the new data.

        streamsize tellp = m_stream->tellp();
        streamsize tellg = m_stream->tellg();
        streamsize bufsz2 = tellp - tellg;

        streamsize maxQueueSize = 16*1024;
        streamsize bufsz = bytesAvailable();
        LG_QIO_D << "dataWritten() maxQueueSize:" << maxQueueSize << endl;
        LG_QIO_D << "    m_uploadProgress:" << m_uploadProgress << endl;
        LG_QIO_D << "    tellp:" << tellp << endl;
        LG_QIO_D << "    tellg:" << tellg << endl;
        LG_QIO_D << "    bufsz:" << bufsz << " bufsz2:" << bufsz2 << endl;

        LG_QIO_D << "+++ Checking if write should stall for a while..." << endl;

        // FIXME: Qt 4.6
        // while( m_uploadProgress + maxQueueSize < tellp )
        // {
        //     LG_QIO_D << "should sleep, emitting a data ready and preparing..." << endl;
        //     Q_EMIT readyRead();
        
        //     LG_QIO_D << "Sleeping until stream is drained...tellp:" << tellp
        //          << " m_uploadProgress:" << m_uploadProgress << endl;
        //     m_queueSizeWaiter.block( 0 );
        // }
        LG_QIO_D << "+++ Write progressing again..." << endl;
        
        // while( bufsz > maxQueueSize )
        // {
        //     LG_QIO_D << "should sleep, emitting a data ready and preparing..." << endl;
        //     emit readyRead();
        
        //     bufsz = bytesAvailable();
        //     LG_QIO_D << "dataWritten() buffer size:" << bufsz << endl;
        //     LG_QIO_D << "Sleeping until stream is drained..." << endl;
        //     m_queueSizeWaiter.block();
        // }
    
        Q_EMIT readyRead();
        LG_QIO_D << "dataWritten(end)" << endl;
    }

    bool
    StreamToQIODeviceImpl::isSequential () const
    {
        LG_QIO_D << "StreamToQIODeviceImpl::isSequential() m_totalPostSize:" << m_totalPostSize << endl;
        if( m_totalPostSize )
            return false;
        return true;
    }

    bool
    StreamToQIODeviceImpl::atEnd () const
    {
        LG_QIO_D << "StreamToQIODeviceImpl::atEnd() ret:" << (!bytesAvailable())
                 << " m_streamClosed:" << m_streamClosed << endl;
        if( !m_streamClosed )
            return false;

        bool ret = !bytesAvailable();
        return ret;
    }

    qint64
    StreamToQIODeviceImpl::bytesAvailable () const
    {
        // LG_QIO_D << "StreamToQIODeviceImpl::bytesAvailable()" << endl;
        // LG_QIO_D << " is eof() :" << m_stream->eof() << endl;
        // LG_QIO_D << " is good():" << m_stream->good() << endl;
        // LG_QIO_D << " is state :" << m_stream->rdstate() << endl;
        // LG_QIO_D << " is state :" << m_stream->rdstate() << endl;

// /// FIXME:        
//         return 4096;
        
        streamsize tellg = m_stream->tellg();
        m_stream->seekg(0, ios::end);
        streamsize endpos = m_stream->tellg();
        m_stream->seekg(tellg, ios::beg);

        qint64 ret = (endpos - tellg);
        LG_QIO_D << "StreamToQIODeviceImpl::bytesAvailable() epos:" << endpos
                 << " tellg:" << tellg
                 << " ret:" << ret << endl;
//    BackTrace();
        return ret + QIODevice::bytesAvailable();
    }

    qint64
    StreamToQIODeviceImpl::size () const
    {
        qint64 ret = QIODevice::size();
        LG_QIO_D << "StreamToQIODeviceImpl::size() ret:" << ret << endl;
        return m_totalPostSize ? m_totalPostSize : ret;
    }

    
    qint64
    StreamToQIODeviceImpl::readData ( char * data, qint64 maxSize )
    {
        LG_QIO_D << "StreamToQIODeviceImpl::readData()" << endl;
        m_queueSizeWaiter.unblock( 0 );
    
        m_stream->read( data, maxSize );
        streamsize bread = m_stream->gcount();
#ifdef DEBUGGING_UPLOAD_STREAMING
        QDebugStream.write( data, bread );
#endif
        LG_QIO_D << "StreamToQIODeviceImpl::readData() maxSize:" << maxSize << " bread:" << bread << endl;
//        LG_QIO_D << "StreamToQIODeviceImpl::readData() m_stream:" << m_stream.str() << endl;
        if( m_stream.eof() )
        {
            if( !bread && atEnd() )
            {
                return -1;
            }
        
            m_stream.clear();
        }
        return bread;
    }


    qint64
    StreamToQIODeviceImpl::writeData ( const char * data, qint64 maxSize )
    {
        return -1;
    }


    

    namespace Factory
    {
        fh_StreamToQIODevice createStreamToQIODevice( streamsize totalPostSize )
        {
            return new StreamToQIODeviceImpl( totalPostSize );
        }
    };

    /****************************************/
    /****************************************/
    /****************************************/

    template< typename _CharT, typename _Traits >
    StreamToQIODevice_IOStream<_CharT,_Traits>::~StreamToQIODevice_IOStream()
    {
        cerr << "~StreamToQIODevice_IOStream() m_iodev:" << m_iodev << endl;
        if( m_iodev )
            m_iodev->dataWritten();
    }


    template<
        typename _CharT,
        typename _Traits,
        typename _Alloc
        >
    void
    StreamToQIODevice_streambuf<_CharT,_Traits,_Alloc>::handleWrite()
    {
        LG_QIO_D << "StreamToQIODevice_streambuf::handleWrite() m_iodev:" << m_iodev << endl;
        if( m_iodev )
        {
            LG_QIO_D << "StreamToQIODevice_streambuf::handleWrite() m_iodev:" << m_iodev
                     << " rc:" << this->ref_count
                     << " user-has-stream:" << m_iodev->getUserHasStream()
                     << endl;
            m_iodev->dataWritten();
        }
    }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


        class FERRISEXP_DLLLOCAL QAppHolder
        {
        public:
            QAppHolder()
                {
                    if( !QCoreApplication::instance() )
                    {
                        static int argc = 1;
                        static char* argv[] = { (char*)"libferris", 0 };
                        static QCoreApplication* app = new QCoreApplication(argc, argv);
                    }
                }
        };
        typedef Loki::SingletonHolder< QAppHolder, Loki::CreateUsingNew, Loki::NoDestroy  > QAppSingleton;



        void myMessageOutput(QtMsgType type, const char *msg)
        {
            switch (type) {
            case QtDebugMsg:
                LG_QIO_D << "Qt-debug:" << msg << endl;
                break;
            case QtWarningMsg:
                LG_QIO_W << "Qt-debug:" << msg << endl;
                fprintf(stderr, "Warning: %s\n", msg);
                break;
            case QtCriticalMsg:
                fprintf(stderr, "Critical: %s\n", msg);
                break;
            case QtFatalMsg:
                fprintf(stderr, "Fatal: %s\n", msg);
                abort();
            }
        }
    
    void installQTMsgHandler()
    {
        qInstallMsgHandler(myMessageOutput);
    }
    
    void ensureQApplication()
    {
        installQTMsgHandler();
        Main::processAllPendingEvents();
        QAppSingleton::Instance();
    }
    
    QNetworkAccessManager*
    getQNonCachingManager()
    {
        static QNetworkAccessManager* m_qmanager = 0;
        
        if( !m_qmanager )
        {
            m_qmanager = new QNetworkAccessManager(0);
        }
        return m_qmanager;
    }
    
    QNetworkAccessManager*
    getQManager( QNetworkCookieJar* cjar )
    {
        static QNetworkAccessManager* m_qmanager = 0;
        QNetworkAccessManager* ret = m_qmanager;
        
        if( !m_qmanager || cjar )
        {
            ret = new QNetworkAccessManager(0);
            if( cjar )
            {
                ret->setCookieJar( cjar );
            }
            else
            {
                m_qmanager = ret;
            }
            
            if( isTrue( getEDBString( FDB_GENERAL, "curl-use-proxy", "" )))
            {
                std::string proxyname = getEDBString( FDB_GENERAL, "curl-use-proxy-name", "" );
                int         proxyport = toint( getEDBString( FDB_GENERAL, "curl-use-proxy-port", "3128" ));
                std::string proxyuserpass = getEDBString( FDB_GENERAL, "curl-use-proxy-userpass", "" );
                stringlist_t t = Util::parseSeperatedList( proxyuserpass, ':' );
                std::string user = "";
                std::string pass = "";
                if( t.size() >= 2 )
                {
                    stringlist_t::iterator ti = t.begin();
                    user = *ti;
                    ++ti;
                    pass = *ti;
                }
                
                QNetworkProxy prox( QNetworkProxy::HttpProxy,
                                    proxyname.c_str(),
                                    proxyport,
                                    user.c_str(), pass.c_str() );
//                LG_QIO_D << "Setting proxy to:" << proxyname << " port:" << proxyport << std::endl;
                // prox.setCapabilities( QNetworkProxy::HostNameLookupCapability
                //                       | QNetworkProxy::TunnelingCapability );
                
                ret->setProxy( prox );
            }
        }
        return ret;
    }

    
    QNetworkResponseWaiter::QNetworkResponseWaiter()
        :
        m_refCount( 0 )
    {
        m_loop = g_main_loop_new( 0, 0 );
    }
    
    
    void
    QNetworkResponseWaiter::block( QNetworkReply* r )
    {
        LG_QIO_I << "block() this:" << this << " r:" << r << " m_refCount:" << m_refCount << endl;
//        cerr << "block() this:" << this << " r:" << r << " m_refCount:" << m_refCount << endl;
//        BackTrace();
        // ++m_refCount;
        // if( m_refCount <= 0 )
        // {
        //     LG_QIO_D << "unblock() was already called!" << endl;
        //     Main::processAllPendingEvents();
        //     return;
        // }

        if( r )
        {
            if( m_alreadyUnblocked.count(r) )
            {
                LG_QIO_D << "QNetworkResponseWaiter::block() replay has already come!" << endl;
                return;
            }
        }
        
        LG_QIO_D << "QNetworkResponseWaiter::block(top)" << endl;
        g_main_loop_run( m_loop );
        LG_QIO_D << "QNetworkResponseWaiter::block(done)" << endl;
    }
    
    void
    QNetworkResponseWaiter::unblock( QNetworkReply* r )
    {
        LG_QIO_I << "unblock() this:" << this << " r:" << r << " m_refCount:" << m_refCount << endl;
//        --m_refCount;
        if( r )
            m_alreadyUnblocked.insert(r);
        g_main_loop_quit( m_loop );
    }
    
    void wait( QNetworkReply* r )
    {
        ensureQApplication();
        
        QNetworkAccessManager* qm = r->manager();
        
        QEventLoop* loop = new QEventLoop(0);
        qm->connect( qm, SIGNAL(finished(QNetworkReply*)), loop, SLOT(quit()) );
        if( r->isFinished() )
            return;
//        cerr << "waiting on reply...." << endl;
        loop->exec();
//            return reply;
        
//        sleep(4);
    }
    

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

    // template<
    //     typename _CharT,
    //     typename _Traits = std::char_traits < _CharT >,
    //     typename _Alloc  = std::allocator   < _CharT >
    //     >
    class StreamFromQIODevice_streambuf
        :
        public QObject,
        public ferris_basic_streambuf< char, std::char_traits<char> >
    {
        Q_OBJECT
        typedef char _CharT;
        typedef std::char_traits<char> _Traits;
        typedef std::allocator   < _CharT > _Alloc;
        typedef std::basic_stringbuf<_CharT, _Traits, _Alloc> sb;
    
    public:
    
        typedef std::char_traits<_CharT>          traits_type;
        typedef traits_type::int_type    int_type;
        typedef traits_type::char_type   char_type;
        typedef traits_type::pos_type    pos_type;
        typedef traits_type::off_type    off_type;
        typedef ferris_basic_streambuf<_CharT, _Traits>                      _Base;
        typedef StreamFromQIODevice_streambuf   _Self;
        typedef std::basic_string<_CharT, _Traits, _Alloc>  _String;

        QIODevice* m_qio;
        qint64 m_gpos;
        bool m_maintainPos;
        
        explicit
        StreamFromQIODevice_streambuf( QIODevice* qio,
                                       bool maintainPos,
                                       std::ios_base::openmode m = std::ios_base::in | std::ios_base::out )
            :
            _Base()
            , m_qio( qio )
            , m_maintainPos( maintainPos )
            , m_gpos( 0 )
            {
                LG_QIO_D << "StreamFromQIODevice_streambuf()" << endl;
                readingComplete = 0;
                connect( (QObject*)qio, SIGNAL( readyRead() ), SLOT( OnReadyRead() ) );
                connect( (QObject*)qio, SIGNAL( readChannelFinished() ), SLOT( OnReadChannelFinished() ) );
                
            }
    
        virtual ~StreamFromQIODevice_streambuf()
            {
                LG_QIO_D << "~StreamFromQIODevice_streambuf()" << endl;
            }

        virtual pos_type seekoff( off_type off,
                                  ios_base::seekdir way,
                                  ios_base::openmode m = ios_base::in | ios_base::out )
            {
                LG_QIO_D << "seekoff() off:" << off << " way:" << way << " m:" << m << endl;
                return _Base::seekoff( off, way, m );
            }
        virtual pos_type seekpos( pos_type pos,
                                  ios_base::openmode m = ios_base::in | ios_base::out )
            {
                LG_QIO_D << "seekpos()" << endl;
                return _Base::seekpos( pos, m );
            }
        int waiting;
        int readingComplete;
                           
    public slots:
        void OnReadyRead()
            {
                LG_QIO_D << "OnReadyRead()" << endl;
                waiting = 0;
            }
        void OnReadChannelFinished()
            {
                LG_QIO_I << "readChannelFinished()" << endl;
                waiting = 0;
                readingComplete = 1;
            }
        
    public:
        virtual int make_new_data_avail( char_type* buffer, std::streamsize maxsz )
            {
                LG_QIO_D << "make_new_data_avail() maxsz:" << maxsz << endl;
                LG_QIO_D << "make_new_data_avail() available:" << m_qio->bytesAvailable() << endl;
                LG_QIO_D << "make_new_data_avail() atEnd:" << m_qio->atEnd() << endl;
                LG_QIO_D << "make_new_data_avail() isOpen:" << m_qio->isOpen() << endl;
                LG_QIO_D << "make_new_data_avail() pos:" << m_qio->pos() << endl;

                if( m_qio->bytesAvailable() < 1 )
                {
                    if( readingComplete )
                        return 0;
                    
                    waiting = 1;
                    while( m_qio->isOpen() && waiting )
                    {
                        Main::processAllPendingEvents();
                    }
                }
                LG_QIO_D << "make_new_data_avail(2) maxsz:" << maxsz << endl;
                LG_QIO_D << "make_new_data_avail(3) available:" << m_qio->bytesAvailable() << endl;
                LG_QIO_D << "make_new_data_avail(4) pos:" << m_qio->pos() << endl;
                LG_QIO_D << "make_new_data_avail(4) gpos:" << m_gpos << endl;

                qint64 rc = 0;
                
                if( m_maintainPos )
                {
                    qint64 pos = m_qio->pos();
                    m_qio->seek( m_gpos );
                    rc = m_qio->read( buffer, maxsz );
                    m_gpos += rc;
                    m_qio->seek( pos );
                }
                else
                {
                    rc = m_qio->read( buffer, maxsz );
                }
                
                
                LG_QIO_D << "make_new_data_avail(end) readingComplete:" << readingComplete
                         << " rc:" << rc << endl;
                return rc;
            }
        int_type underflow()
            {
                int_type ret = _Base::underflow();
                LG_QIO_D << "underflow() ret:" << ret
                         << " this->gptr():" << (void*)this->gptr()
                         << " this->egptr():" << (void*)this->egptr()
                         << endl;
                return ret;
            }
        
        
    };
    

template<
    typename _CharT,
    typename _Traits = std::char_traits < _CharT >,
    typename _Alloc  = std::allocator   < _CharT >
    >
class  StreamFromQIODevice_stringstream
    :
        public Ferris_iostream<_CharT, _Traits>,
//        public StreamHandlableSigEmitter< StreamFromQIODevice_stringstream<_CharT, _Traits, _Alloc> >,
        public stringstream_methods<_CharT, _Traits, io_ferris_stream_traits< _CharT, _Traits > >
{
//    typedef StreamFromQIODevice_streambuf<_CharT, _Traits, _Alloc> ss_impl_t;
    typedef StreamFromQIODevice_streambuf ss_impl_t;
    typedef Loki::SmartPtr<  ss_impl_t,
                             FerrisLoki::FerrisExRefCounted,
                             Loki::DisallowConversion,
                             FerrisLoki::FerrisExSmartPointerChecker,
                             FerrisLoki::FerrisExSmartPtrStorage > ss_t;
    ss_t ss;
    typedef Ferris_commonstream<_CharT, _Traits> _CS;
    typedef io_ferris_stream_traits< _CharT, _Traits > _FerrisStreamTraits;

public:

    typedef _Traits                           traits_type;
    typedef typename traits_type::int_type    int_type;
    typedef typename traits_type::char_type   char_type;
    typedef typename traits_type::pos_type    pos_type;
    typedef typename traits_type::off_type    off_type;

    typedef stringstream_methods<
        char_type, traits_type,
        io_ferris_stream_traits< char_type, traits_type > > delegating_methods;
    typedef StreamFromQIODevice_stringstream< _CharT, _Traits, _Alloc > _Self;


    explicit StreamFromQIODevice_stringstream(
        ss_t& _ss )
        :
        ss( _ss )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
            this->seekg( 0 );
            this->seekp( 0 );
        }
    
    explicit StreamFromQIODevice_stringstream(
        QIODevice* qio,
        bool maintainPos,
        std::ios_base::openmode m = _FerrisStreamTraits::DefaultOpenMode )
        :
        ss( new ss_impl_t( qio, maintainPos, m ) )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
        }

    StreamFromQIODevice_stringstream( const StreamFromQIODevice_stringstream& rhs )
        :
        ss( rhs.ss )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );
        }

        
    virtual ~StreamFromQIODevice_stringstream()
        {
        }


    StreamFromQIODevice_stringstream& operator=( const StreamFromQIODevice_stringstream& rhs )
        {
//                 LG_QIO_D << "StreamFromQIODevice_stringstream& op = " << endl;

            this->setsb( &rhs );
            this->init( _CS::sb );
            this->exceptions( std::ios_base::goodbit );
            this->clear( rhs.rdstate() );
            this->copyfmt( rhs );
            return *this;
        }

    
    _Self* operator->()
        {
            return this;
        }

    
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    
    
    ss_impl_t*
    rdbuf() const
        {
            return GetImpl(ss);
        }


    enum
    {
        stream_readable = true,
        stream_writable = false
    };

};
typedef StreamFromQIODevice_stringstream<char>   fh_StreamFromQIODevice_stringstream;

namespace Factory
{
    fh_istream createIStreamFromQIODevice( QIODevice* qio, bool maintainPos )
    {
        fh_StreamFromQIODevice_stringstream ret( qio, maintainPos );
        return ret;
    }
};

    

};

#include <QObject>
#include "FerrisQt_moc_impl.cpp"
