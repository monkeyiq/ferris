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

    $Id: libsocket.cpp,v 1.4 2010/09/24 21:31:46 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
//#include <SignalStreams.hh>
#include <FerrisCreationPlugin.hh>

#include <socket++/sockinet.h>
#include <socket++/sockstream.h>
#include <socket++/sockunix.h>

#include <netdb.h>

#define FERRIS_SOCKET_SSL

#ifdef FERRIS_SOCKET_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <unistd.h>
#include <errno.h>

#include <FerrisOpenSSL.hh>
#include "Ferris/FerrisStdHashMap.hh"

#include <iomanip>

using namespace std;
namespace Ferris
{
extern "C"
{
    FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed );
};
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    
    fh_istream
    SL_getHostNameStream( std::string ipstr )
    {
        try
        {
            typedef FERRIS_STD_HASH_MAP< std::string, std::string > cache_t;
            static cache_t cache;

            fh_stringstream ss;
            
            if( cache.find(ipstr) == cache.end() )
            {
                sockinetaddr sa(ipstr.c_str());
                cache[ipstr] = sa.gethostname();
            }
            
            ss << cache[ipstr];
            return ss;
        }
        catch( sockerr& se )
        {
            string e = se.errstr();
            LG_SOCKET_W << "ipstr:" << ipstr << " resolve se:" << e << endl;

            stringstream ss;
            ss << "Error getting hostname e:" << e << endl;
            Throw_CanNotGetStream(tostr(ss), 0);
        }
    }
    
    fh_istream
    SL_getLocalHostNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string ip = getStrAttr( c, "local-ip", "" );
        return SL_getHostNameStream( ip );
    }

    fh_istream
    SL_getRemoteHostNameStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        string ip = getStrAttr( c, "remote-ip", "" );
        return SL_getHostNameStream( ip );
    }
    
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    class SocketContext;
    class SocketContextEA;
    FERRIS_SMARTPTR( SocketContext, fh_sockcontext );


/*
 * Using this we can get type safe actions performed when a child is read.
 */
struct SocketStreamAttacher
{
    string SoughtHostIP;
    string SoughtPort;
    string SoughtHostIPAttrName;
    string SoughtPortAttrName;
    bool ServerMode;
    
    iosockinet* ss;
    bool attached;
    fh_context c;
    SSL_CTX* ssl_ctx;
    bool UsingCrypto;

    SocketStreamAttacher()
        {
            reset();
        }
    
    void reset()
        {
            SoughtHostIP = "";
            SoughtPort = "";
            SoughtHostIPAttrName = "";
            SoughtPortAttrName = "";
            ss = 0;
            attached = false;
            c = 0;
            ServerMode = false;
            ssl_ctx = 0;
        }
    
    void operator()( SocketContext* c );
};


class FERRISEXP_CTXPLUGIN SocketListContext
    :
    public Context
{
    typedef Context _Base;
    typedef SocketListContext _Self;
    
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    Context* priv_CreateContext( Context* parent, string rdn );

    string Filename;

    static const string CREATE_REMOTE_PORT;
    static const string CREATE_REMOTE_HOST;
    static const string CREATE_LOCAL_PORT;
    static const string CREATE_LOCAL_HOST;
    static const string CREATE_PROTOCOL;
    static const string CREATE_STREAM;
    static const string CREATE_NUMBER_CONNECTIONS;
    static const string CREATE_USE_SSLVERSION;
    static const string CREATE_USE_CERTFILE;
    static const string CREATE_USE_PRIVKFILE;
    

    SocketStreamAttacher ssAttacher;

protected:

    virtual void priv_read();

    virtual std::string getRecommendedEA()
        {
            if( getDirName() == "unix" )
                return "name,rx_queue,tx_queue";
            
            return "name,rx_queue,tx_queue,local-ip,local-port,remote-ip,remote-port";
        }

    fh_context
    SubCreateSocket( fh_context c, fh_context md );
    
        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            static Util::SingleShot v;
            if( v() )
                {
                    static bool rst = appendExtraGenerateSchemaSimpleTypes(
                        "<simpleType name=\"SocketSSLT\">"  
                        "    <restriction base=\"string\">"  
                        "        <enumeration value=\"None\"/>"  
                        "        <enumeration value=\"SSL2\"/>"  
                        "        <enumeration value=\"SSL3\"/>"  
                        "        <enumeration value=\"TLS\"/>"  
                        "    </restriction>"  
                        "</simpleType>"  
            
                        "<simpleType name=\"SocketSSLListT\">"  
                        "    <list itemType=\"SocketSSLT\"/>"  
                        "    <restriction>"  
                        "        <length value=\"1\"/>"  
                        "    </restriction>"  
                        "</simpleType>"  
                        "<simpleType name=\"TCPIPProtocolT\">"  
                        "    <restriction base=\"string\">"  
                        "        <enumeration value=\"TCP\"/>"  
                        "        <enumeration value=\"UDP\"/>"  
                        "        <enumeration value=\"ICMP\"/>"  
                        "    </restriction>"  
                        "</simpleType>"  
            
                        "<simpleType name=\"TCPIPProtocolListT\">"  
                        "    <list itemType=\"TCPIPProtocolT\"/>"  
                        "    <restriction>"  
                        "        <length value=\"1\"/>"  
                        "    </restriction>"  
                        "</simpleType>"  
                        );
                }
                {
                    fh_stringstream ss;

                ss << "	<elementType name=\"clientsocket\">\n";

                ss << "<elementType name=\"" << CREATE_REMOTE_PORT << "\" ";
                ss << " default=\"" << 1025 << "\" ";
                ss << " minInclusive=\"" << 1 << "\" ";
                ss << " maxInclusive=\"" << 65536 << "\" ";
                ss << " step=\"" << 1 << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "int" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_REMOTE_HOST << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";

//                 ss << "<elementType name=\"" << CREATE_PROTOCOL << "\" ";
//                 ss << " >\n"
//                    << "   <dataTypeRef name=\"" << "TCPIPProtocolListT" << "\"/>\n"
//                    << "</elementType>\n";

#ifdef FERRIS_SOCKET_SSL
                ss << "<elementType name=\"" << CREATE_USE_SSLVERSION << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "SocketSSLListT" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_USE_CERTFILE << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_USE_PRIVKFILE << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";
#endif                

                ss << "	</elementType>\n";

                m["clientsocket"] = SubContextCreator(
                    SubContextCreator::Perform_t( this, &SocketListContext::SubCreateSocket),
                    tostr(ss));
            }
            {
                fh_stringstream ss;
                
                ss << "	<elementType name=\"serversocket\">\n";

                ss << "<elementType name=\"" << CREATE_LOCAL_PORT << "\" ";
                ss << " default=\"" << 0 << "\" ";
                ss << " minInclusive=\"" << 0 << "\" ";
                ss << " maxInclusive=\"" << 65536 << "\" ";
                ss << " step=\"" << 1 << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "int" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_LOCAL_HOST << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";

//                 ss << "<elementType name=\"" << CREATE_PROTOCOL << "\" ";
//                 ss << " >\n"
//                    << "   <dataTypeRef name=\"" << "TCPIPProtocolListT" << "\"/>\n"
//                    << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_NUMBER_CONNECTIONS << "\" ";
                ss << " default=\"" << 16 << "\" ";
                ss << " minInclusive=\"" << 1 << "\" ";
                ss << " maxInclusive=\"" << 32 << "\" ";
                ss << " step=\"" << 1 << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "int" << "\"/>\n"
                   << "</elementType>\n";

#ifdef FERRIS_SOCKET_SSL
                ss << "<elementType name=\"" << CREATE_USE_SSLVERSION << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "SocketSSLListT" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_USE_CERTFILE << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";

                ss << "<elementType name=\"" << CREATE_USE_PRIVKFILE << "\" ";
                ss << " >\n"
                   << "   <dataTypeRef name=\"" << "string" << "\"/>\n"
                   << "</elementType>\n";
#endif                
                ss << "	</elementType>\n";

                m["serversocket"] = SubContextCreator(
                    SubContextCreator::Perform_t( this, &SocketListContext::SubCreateSocket),
                    tostr(ss));
            }
        }
    
    

    const fh_context&
    createSubContext_connect( fh_context md,
                              const string& remote_port,
                              const string& remote_host,
                              string& protocol,
                              string& stream,
                              SSL_METHOD* ssl_meth,
                              SSL_CTX* ssl_ctx,
                              bool UsingCrypto
        )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

    const fh_context&
    createSubContext_bind( fh_context md,
                           const string& local_port,
                           const string& local_host,
                           string& protocol,
                           string& stream,
                           SSL_METHOD* ssl_meth,
                           SSL_CTX* ssl_ctx,
                           bool UsingCrypto
        )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );
    
    
public:

    SocketListContext( fh_context parent, const string& rdn, const string& filename );

//    void OnExists ( NamingEvent_Exists* ev,  string olddn, string newdn );
    
};
FERRIS_SMARTPTR( SocketListContext, fh_socklistcontext );


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_CTXPLUGIN SocketContext
    :
    public RecommendedEACollectingContext
<
    StateLessEAHolder< SocketContext, leafContext >
>
{
    typedef RecommendedEACollectingContext < StateLessEAHolder< SocketContext, leafContext > > _Base;

    fh_socklistcontext socklistctx;
    fh_socklistcontext& getSockList();

public:
    typedef vector<string> hdr_t;
    static hdr_t hdr;

    typedef vector<int> hdr_offsets_t;
    static hdr_offsets_t hdr_offsets;
private:
    iosockinet* SocketStream;

    bool ServerMode;

    friend class SocketStreamAttacher;
    friend class SocketContextEA;

    SSL_CTX* ssl_ctx;
    bool UsingCrypto;
    
protected:
    void createPeerCertificateEA( SSL* ssl );
    void setupCipherEA( SSL* ssl );
    void protectAgainstCrapCrypto( SSL* ssl ) throw (CanNotGetStream);

    void
    createStateLessAttributes( bool force = false )
        {
//            LG_SOCKET_D << "socket createStateLessAttributes()" << endl;
            static Util::SingleShot virgin;
            if( virgin() )
            {
//                LG_SOCKET_D << "socket createStateLessAttributes(add)" << endl;
                tryAddStateLessAttribute( "remote-hostname",
                                          SL_getRemoteHostNameStream,
                                          FXD_MACHINE_NAME );
                tryAddStateLessAttribute( "local-hostname",
                                          SL_getLocalHostNameStream,
                                          FXD_MACHINE_NAME );
                _Base::createStateLessAttributes( true );
            }
        }

    string
    getRecommendedEA()
        {
            if( isParentBound() && getParent()->getDirName() == "unix" )
                return _Base::getRecommendedEA();

            fh_stringstream ss;
            ss << "remote-hostname,local-hostname," << _Base::getRecommendedEA();
//            LG_SOCKET_D << "Socket-rea:" << tostr(ss) << endl;
            return tostr(ss);
        }
    
    virtual fh_iostream  getRealStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception);
    
    virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               exception);
    virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               AttributeNotWritable,
               CanNotGetStream,
               exception);

    ferris_ios::openmode
    getSupportedOpenModes()
        {
            return
                ios::in        |
                ios::out       |
                ios::binary    ;
        }
    

public:
    static PrePostTrimmer& getTrimmer();
    void addCompactEA( fh_context a, const string& k, const string& v );
    static bool isKey( const string& s );

public:
    void constructObject( stringmap_t& ea );
    SocketContext( Context* parent, const string& rdn );
    virtual ~SocketContext();
        
    
    static void parseHeader( const string& s );
    
    void setSocket( iosockinet* ss );
    

    sockbuf* SockBuf();
    fh_iostream getReuseAddrStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setReuseAddrStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getSocketErrorStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setSocketErrorStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getSocketTypeStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setSocketTypeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getKeepAliveStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setKeepAliveStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getDontRouteStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setDontRouteStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getBroadcastStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setBroadcastStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getInlineOutOfBandStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setInlineOutOfBandStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getLingerStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setLingerStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getSendBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setSendBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

    fh_iostream getRecvBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom );
    void        setRecvBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SocketContext::hdr_t         SocketContext::hdr;
SocketContext::hdr_offsets_t SocketContext::hdr_offsets;


sockbuf*
SocketContext::SockBuf()
{
    if( !SocketStream )
    {
        fh_stringstream ss;
        ss << "Attempt to access socket options for a closed socket!"
           << " url:" << getURL()
           << flush;
        Throw_SocketOptionsException( tostr(ss), this );
    }
    
    return SocketStream->rdbuf();
}

fh_iostream
SocketContext::getReuseAddrStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = -1;
    opval = SockBuf()->reuseaddr(opval);
    ss << opval;
    return ss;
}

void
SocketContext::setReuseAddrStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->reuseaddr(opval);
}

fh_iostream
SocketContext::getSocketErrorStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = 0;
    SockBuf()->getopt( sockbuf::so_error, &opval, sizeof(opval) );
    ss << opval;
    return ss;
}

void
SocketContext::setSocketErrorStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    string s;
    ss >> s;
    if( s == "0" || s == "false" )
    {
        SockBuf()->clearerror();
    }
}

fh_iostream
SocketContext::getSocketTypeStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    sockbuf::type ty = SockBuf()->gettype();

    switch( ty )
    {
    case sockbuf::sock_stream:      ss << "stream";      break;
    case sockbuf::sock_dgram:       ss << "dgram";       break;
    case sockbuf::sock_raw:         ss << "raw";         break;
    case sockbuf::sock_rdm:         ss << "rdm";         break;
    case sockbuf::sock_seqpacket:   ss << "seqpacket";   break;
    default:                        ss << "unknown";     break;
    }
    return ss;
}

void
SocketContext::setSocketTypeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream )
{
}

fh_iostream
SocketContext::getKeepAliveStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = -1;
    opval = SockBuf()->keepalive(opval);
    ss << opval;
    return ss;
}

void
SocketContext::setKeepAliveStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->keepalive(opval);
}

fh_iostream
SocketContext::getDontRouteStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = -1;
    opval = SockBuf()->dontroute(opval);
    ss << opval;
    return ss;
}

void
SocketContext::setDontRouteStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->dontroute(opval);
}

fh_iostream
SocketContext::getBroadcastStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = -1;
    opval = SockBuf()->broadcast(opval);
    ss << opval;
    return ss;
}
void
SocketContext::setBroadcastStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->broadcast(opval);
}


fh_iostream
SocketContext::getInlineOutOfBandStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    int opval = -1;
    opval = SockBuf()->oobinline(opval);
    ss << opval;
    return ss;
}
void
SocketContext::setInlineOutOfBandStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->oobinline(opval);
}

fh_iostream
SocketContext::getLingerStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;

    sockbuf::socklinger lin = SockBuf()->linger();
    if( lin.l_onoff )
    {
        LG_SOCKET_D << " SocketContextEA_Linger ret:"
                    << tostr( lin.l_linger ) << endl;
        ss << lin.l_linger;
    }
    else
    {
        ss << "-1";
    }
    return ss;
}
void
SocketContext::setLingerStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    sockbuf::socklinger lin( (opval == -1) ? 0 : 1, opval );
    SockBuf()->linger(lin);
}

fh_iostream
SocketContext::getSendBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    /*
     * Set the value to -1 to get the current value and then set the current
     * value back again.
     */
    int opval = -1;
    opval = SockBuf()->sendbufsz(opval);
    SockBuf()->sendbufsz(opval);
    ss << opval;
    return ss;
}
void
SocketContext::setSendBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->sendbufsz(opval);
}

fh_iostream
SocketContext::getRecvBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    fh_stringstream ss;
    /*
     * Set the value to -1 to get the current value and then set the current
     * value back again.
     */
    int opval = -1;
    opval = SockBuf()->recvbufsz(opval);
    SockBuf()->recvbufsz(opval);
    ss << opval;
    return ss;
}
void
SocketContext::setRecvBufSizeStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
{
    int opval = -1;
    ss >> opval;
    SockBuf()->recvbufsz(opval);
}


void
SocketContext::setSocket( iosockinet* ss )
{
    SocketStream = ss;

    typedef SocketContext _Self;

    addAttribute( "socket-error",
                  this, &_Self::getSocketErrorStream,
                  this, &_Self::getSocketErrorStream,
                  this, &_Self::setSocketErrorStream,
                  XSD_BASIC_INT );

    addAttribute( "socket-type",
                  this, &_Self::getSocketTypeStream,
                  this, &_Self::getSocketTypeStream,
                  this, &_Self::setSocketTypeStream,
                  XSD_BASIC_STRING );

    addAttribute( "socket-keep-alive",
                  this, &_Self::getKeepAliveStream,
                  this, &_Self::getKeepAliveStream,
                  this, &_Self::setKeepAliveStream,
                  XSD_BASIC_INT );
    
    addAttribute( "socket-route-off",
                  this, &_Self::getDontRouteStream,
                  this, &_Self::getDontRouteStream,
                  this, &_Self::setDontRouteStream,
                  XSD_BASIC_BOOL );

    addAttribute( "socket-broadcast",
                  this, &_Self::getBroadcastStream,
                  this, &_Self::getBroadcastStream,
                  this, &_Self::setBroadcastStream,
                  XSD_BASIC_BOOL );

    addAttribute( "socket-socket-inline-out-of-band-data",
                  this, &_Self::getInlineOutOfBandStream,
                  this, &_Self::getInlineOutOfBandStream,
                  this, &_Self::setInlineOutOfBandStream,
                  XSD_BASIC_BOOL );

    addAttribute( "socket-linger",
                  this, &_Self::getLingerStream,
                  this, &_Self::getLingerStream,
                  this, &_Self::setLingerStream,
                  XSD_BASIC_INT );

    addAttribute( "socket-send-buffer-size",
                  this, &_Self::getSendBufSizeStream,
                  this, &_Self::getSendBufSizeStream,
                  this, &_Self::setSendBufSizeStream,
                  XSD_BASIC_INT );
   
    addAttribute( "socket-receive-buffer-size",
                  this, &_Self::getRecvBufSizeStream,
                  this, &_Self::getRecvBufSizeStream,
                  this, &_Self::setRecvBufSizeStream,
                  XSD_BASIC_INT );

    addAttribute( "socket-reuse-address",
                  this, &_Self::getReuseAddrStream,
                  this, &_Self::getReuseAddrStream,
                  this, &_Self::setReuseAddrStream,
                  XSD_BASIC_BOOL );
}

    


fh_socklistcontext&
SocketContext::getSockList()
{
    return socklistctx;
}


PrePostTrimmer&
SocketContext::getTrimmer()
{
    static PrePostTrimmer pptrimmer;
    static bool virgin = true;

    if( virgin )
    {
        virgin = false;

        pptrimmer.push_back( " " );
        pptrimmer.push_back( "\t" );
    }

    return pptrimmer;
}

string IP32ToString( guint32 a )
{
    fh_stringstream ss;
    ss << ((a & 0x000000FF) >>  0) << ".";
    ss << ((a & 0x0000FF00) >>  8) << ".";
    ss << ((a & 0x00FF0000) >> 16) << ".";
    ss << ((a & 0xFF000000) >> 24);
    return tostr(ss);
}

string RemoteIP32ToString( iosockinet* sock_ss )
{
    sockaddr_in* sia = (*sock_ss)->peeraddr().addr_in();
    return IP32ToString( sia->sin_addr.s_addr );
}

string LocalIP32ToString( iosockinet* sock_ss )
{
    sockaddr_in* sia = (*sock_ss)->localaddr().addr_in();
    return IP32ToString( sia->sin_addr.s_addr );
}


string CompactIPToDotIp( const string& compactip )
{
    stringstream ss_spread;

    LG_SOCKET_D << "compactip:" << compactip << endl;

    ss_spread
        << (char)::tolower(compactip[0]) << (char)::tolower(compactip[1]) << " "
        << (char)::tolower(compactip[2]) << (char)::tolower(compactip[3]) << " "
        << (char)::tolower(compactip[4]) << (char)::tolower(compactip[5]) << " "
        << (char)::tolower(compactip[6]) << (char)::tolower(compactip[7]) << " "
        ;

    unsigned int p1=0, p2=0, p3=0, p4=0;
    stringstream ss( tostr(ss_spread) );
    ss >> hex >> p4 >> hex >> p3 >> hex >> p2 >> hex >> p1;
    
    stringstream retss;
    retss << p1 << "." << p2 << "." << p3 << "." << p4;

    LG_SOCKET_D << "ip:" << tostr(retss) << endl;
    return tostr(retss);
}


void
SocketContext::addCompactEA( fh_context a, const string& k, const string& v )
{
    stringstream ss( v );
    string compactip;
    int port;

    getline( ss, compactip, ':' );
    ss >> hex >> port;
    string ip = CompactIPToDotIp(compactip);
            
    LG_SOCKET_D << "addCompactEA() k:" << k << " ip:" << ip << " port:" << port << endl;
            
            
    string an;
    stringstream name_ss( k );
    getline( name_ss, an, '_' );

    if( an == "rem" )
        an = "remote";

    addAttribute( an + "-ip", ip, FXD_STR_IP4ADDR, true );

    stringstream port_ss;
    port_ss << port;
    addAttribute( an + "-port", tostr(port_ss), FXD_IP4PORT, true );

    /*
     * Lazy reverse host lookup
     */
//    delete a->tryAddHeapAttribute( new IPStringAttribute( a, an + "-hostname", ip ),
//              FXD_MACHINE_NAME );
}

bool
SocketContext::isKey( const string& s )
{
    if( s == "sl" )
        return true;
    else if( s == "Num" )
        return true;
    
    return false;
}


    void
    SocketContext::constructObject( stringmap_t& ea )
    {
        for( stringmap_t::const_iterator ei = ea.begin(); ei!=ea.end(); ++ei )
        {
            const string& attrName  = ei->first;
            const string& attrValue = ei->second;

            if( !isAttributeBound( attrName ) )
                addAttribute( attrName, attrValue, FXD_BINARY, true );
            
            if( attrName == "local_address" || attrName == "rem_address" )
            {
                addCompactEA( getParent(), attrName, attrValue );
            }
        }
    }
    
    SocketContext::SocketContext( Context* parent, const string& rdn )
        :
        _Base( parent, rdn ),
        socklistctx( dynamic_cast<SocketListContext*>(parent) ),
        ssl_ctx(0),
        UsingCrypto( false ),
        SocketStream(0)
    {
        createStateLessAttributes();

//     const string& l = procline;
//     int oldidx = 0;
//     int i = 0;
//     string rdn = "unnamed";

//     LG_SOCKET_D << "SocketContext::SocketContext(start)" << endl;


//     for( hdr_offsets_t::iterator iter = hdr_offsets.begin();
//          iter != hdr_offsets.end();
//          ++iter, ++i )
//     {
//         const string attrName = hdr[i];
//         int endidx = *iter;
//         string attrValue;

//         if( oldidx >= l.length() )
//             break;

//         if( endidx != string::npos )
//         {
//             attrValue = string( l.begin() + oldidx, l.begin() + endidx);
//         }
//         else
//         {
//             attrValue = string( l.begin() + oldidx, l.end() );
//         }
//         attrValue = getTrimmer()(attrValue);

            
//         if( isKey(attrName) )
//         {
//             PostfixTrimmer t;
//             t.push_back( ":" );
//             rdn = t(attrValue);
//         }

//         LG_SOCKET_D << "Endidx:" << endidx
//                     << " attrName:" << attrName
//                     << " attrValue:" << attrValue
//                     << endl;
        
//         addAttribute( attrName, attrValue, FXD_BINARY, true );
        
//         if( attrName == "local_address" || attrName == "rem_address" )
//         {
//             addCompactEA( parent, attrName, attrValue );
//         }
        
//         oldidx = *iter + 1;
//     }

//     setContext( parent, monsterName(rdn) );
//     LG_SOCKET_D << "SocketContext::SocketContext(end)" << endl;
}

SocketContext::~SocketContext()
{
#ifdef FERRIS_SOCKET_SSL
    if( UsingCrypto )
    {
        SSL_CTX_free( ssl_ctx );
    }
#endif    
}



static bool isSpace( char c )
{
    return c == ' ';
}


void
SocketContext::parseHeader( const string& hdrstring )
{
    istringstream ss(hdrstring);
    string s;
    bool LastWasSpace = true;
    bool virgin = true;
    char ch = 0;
    int index = 0;

    hdr.clear();
    hdr_offsets.clear();
    LG_SOCKET_D << "SocketContext::parseHeader() hdrstring:" << hdrstring << endl;

    while( ss )
    {
        ss.unsetf( ios::skipws );
        ss >> ch;
        ++index;

//        cout << "got char :" << ch << endl;

        if( ch == '\n' || !ss )
            break;
        
        if( isSpace(ch) )
        {
            LastWasSpace = true;
        }
        else if( !isSpace(ch) && LastWasSpace )
        {
            LastWasSpace = false;

            if( virgin )
            {
                virgin = false;
            }
            else
            {
                LG_SOCKET_D << "adding header: " << s
                            << " index:" << index
                            << endl;
                hdr.push_back( getTrimmer()(s) );
                hdr_offsets.push_back( index-2 );
                s = "";
            }
        }

        if(!( isSpace(ch) && virgin ))
        {
            s += ch;
        }
    }

    LG_SOCKET_D << "cleaning up remains:" << s << endl;
    
    bool addlast = false;
    
    for( string::iterator iter = s.begin(); iter != s.end(); ++iter )
    {
        if( !isSpace( *iter ) )
        {
            addlast = true;
            break;
        }
    }

    if( addlast )
    {
        LG_SOCKET_D << "adding final element:" << s << endl;
        
        hdr.push_back( getTrimmer()(s) );
        hdr_offsets.push_back( string::npos );
    }
    
    
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef FERRIS_SOCKET_SSL

template<
    class _CharT,
    class _Traits = std::char_traits < _CharT >,
    class _Alloc  = std::allocator   < _CharT >,
    class _BufferSizers = ferris_basic_streambuf_sixteenbytes
    >
class ferris_ssl_streambuf
    :
    public ferris_basic_double_buffered_streambuf< _CharT, _Traits, _Alloc, _BufferSizers >
{

    typedef ferris_ssl_streambuf<_CharT, _Traits, _Alloc, _BufferSizers> _Self;
    typedef ios::seek_dir seekd_t;

    typedef std::basic_streambuf<_CharT, _Traits> ss_impl_t;
    typedef Loki::SmartPtr<  ss_impl_t,
                       Loki::RefCounted,
                       Loki::DisallowConversion,
                       Loki::AssertCheck,
                       Loki::DefaultSPStorage > raw_ss_t;
    raw_ss_t raw_ss;
    
public:

    typedef std::char_traits<_CharT>          traits_type;
    typedef typename traits_type::int_type             int_type;
    typedef typename traits_type::char_type            char_type;
    typedef typename traits_type::pos_type             pos_type;
    typedef typename traits_type::off_type             off_type;
    typedef ferris_basic_double_buffered_streambuf< _CharT,
                                                    _Traits,
                                                    _Alloc,
                                                    _BufferSizers > _Base;
    typedef std::basic_iostream< _CharT, _Traits >                           _ProxyStreamBuf;
    typedef std::basic_string<_CharT, _Traits, _Alloc>                       _String;

    explicit
    ferris_ssl_streambuf( _ProxyStreamBuf* _RealStream,
                          bool _TakeOwnerShip,
                          SSL* _ssl
        )
        :
        raw_ss( _RealStream->rdbuf() ),
        ssl( _ssl ),
        RealStream( _RealStream ),
        TakeOwnerShip( _TakeOwnerShip )
        {
        }

    virtual ~ferris_ssl_streambuf()
        {
            if( TakeOwnerShip )
                delete RealStream;

            SSL_shutdown(ssl);  /* send SSL/TLS close_notify */
            SSL_free(ssl);
        }

    
private:
    
    // prohibit copy/assign
    ferris_ssl_streambuf( const ferris_ssl_streambuf& );
    ferris_ssl_streambuf operator = ( const ferris_ssl_streambuf& );

    SSL* ssl;
    std::basic_istream< _CharT, _Traits >* RealStream;
    bool        TakeOwnerShip;
    
protected:
    
    /*
     * This is the only methods that really needs to be here. It gets
     * up to maxsz data into buffer and returns how much data was really
     * read.
     */
    int make_new_data_avail( char_type* buffer, streamsize maxsz )
        {
            
            int err = SSL_read( ssl, buffer, maxsz );

            LG_SOCKET_D << "make_new_data_avail() maxsz:" << maxsz
                        << " err:" << err
                        << endl;
            
            
             if( err < 1 )
             {
                 LG_SOCKET_D << "Failed to read from the connection. "
                             << " Error: " << ERR_error_string(ERR_get_error(),0);
                 
//                 stringstream ss;
//                 ss << "Failed to SSL_accept the connection. "
//                    << " Error: " << ERR_error_string(ERR_get_error());
//                 Throw_FerrisCreateSubContextFailed(tostr(ss),this);
             }

            if( err < 1 )
                return 0;

            return err;
        }

    /**
     * Write out the data starting at buffer of length sz to the "external"
     * device.
     * 
     * return -1 for error or 0 for success
     */
    virtual int write_out_given_data( const char_type* buffer, std::streamsize sz )
        {
            LG_SOCKET_D << "write_out_given_data() sz:" << sz << endl;
            
            int err = SSL_write( ssl, buffer, sz );
            LG_SOCKET_D << "write_out_given_data() err:" << err << endl;

            if( err < 1 )
            {
                return -1;
            }

            return 0;
        }

    /*
     * Cant seek in a socket stream, but, need to keep with the normal iostreams
     * protocol for switching from input to output.
     */
    virtual pos_type
    seekoff(off_type offset, seekd_t d, int m)
        {
            LG_SOCKET_D << "ferris_ssl_streambuf::seekoff() offset:" << offset << endl;
            return this->have_been_seeked( 0 );
        }

    virtual pos_type
    seekpos(pos_type pos, int m)
        {
            LG_SOCKET_D << "ferris_ssl_streambuf::seekpos() pos:" << pos << endl;
            return this->have_been_seeked( pos );
        }
    
};


/*
 */
template<
    typename _CharT,
    typename _Traits = std::char_traits < _CharT >,
    typename _Alloc  = std::allocator   < _CharT >
    >
class FerrisSSLStream
    :
    public Ferris_iostream< _CharT, _Traits >
{
    typedef FerrisSSLStream<_CharT, _Traits, _Alloc>   _Self;

public:
    
    typedef char_traits< _CharT >  traits_type;
    typedef typename traits_type::int_type  int_type;
    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::pos_type  pos_type;
    typedef typename traits_type::off_type  off_type;

    typedef ferris_ssl_streambuf<_CharT, _Traits> ss_impl_t;
    FERRIS_SMARTPTR( ss_impl_t, ss_t );
    ss_t ss;

    
public:

    FerrisSSLStream( iostream* _ss,
                     bool _TakeOwnerShip,
                     SSL* _ssl
        )
        :
        ss( new ss_impl_t( _ss, _TakeOwnerShip, _ssl ))
        {
            init( rdbuf() );
            setsbT( GetImpl(ss) );
        }

    FerrisSSLStream( const FerrisSSLStream& rhs )
        :
        ss( rhs.ss )
        {
            init( rdbuf() );
            setsbT( GetImpl(ss) );
        }

    virtual ~FerrisSSLStream()
        {
        }

    _Self* operator->()
        {
            return this;
        }

    ss_impl_t*
    rdbuf() const
        {
            return GetImpl(ss);
        }

    enum
    {
        stream_readable = true,
        stream_writable = true
    };
};



#endif //#ifdef FERRIS_SOCKET_SSL

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
SocketContext::createPeerCertificateEA( SSL* ssl )
{
    X509* server_cert = SSL_get_peer_certificate( ssl );
    if( server_cert )
    {
        char* subject = X509_NAME_oneline( X509_get_subject_name( server_cert ), 0, 0 );
        char* issuer  = X509_NAME_oneline( X509_get_issuer_name ( server_cert ), 0, 0 );

        if( subject )
        {
            addAttribute( "certificate-subject", subject, FXD_DISTINGUISHED_PERSON, true );
            LG_SOCKET_D << "Create EA certificate-subject" <<  subject << endl;
            free( subject );
        }
        
        if( issuer )
        {
            addAttribute( "certificate-issuer", issuer, FXD_DISTINGUISHED_PERSON, true );
            LG_SOCKET_D << "Create EA certificate-issuer" <<  issuer << endl;
            free( issuer );
        }
        
        X509_free( server_cert );
    }
    
}


void
SocketContext::setupCipherEA( SSL* ssl )
{
    string c_ean         = "cipher";
    string c_bits_ean    = "cipher-bits";
    string c_algo_bits_ean= "cipher-algo-bits";
    string c_list_ean    = "cipher-list";
    string c_name_ean    = "cipher-name";
    string c_version_ean = "cipher-version";
    
    if( !isAttributeBound( c_ean ) )
    {
        addAttribute( c_ean, SSL_get_cipher( ssl ), FXD_BINARY, true );
    }

    if( !isAttributeBound( c_bits_ean ) )
    {
        int alg_bits  = 0;
        int real_bits = SSL_get_cipher_bits( ssl, &alg_bits );

        addAttribute( c_bits_ean,      tostr(real_bits), FXD_CIPHER_BITS, true );
        addAttribute( c_algo_bits_ean, tostr(alg_bits),  FXD_CIPHER_BITS, true );
    }

    if( !isAttributeBound( c_list_ean ) )
    {
        string cl="";
        int n=0;
        const char* cn = 0;
        bool virgin = true;
        
        while( (cn = SSL_get_cipher_list( ssl, n )) )
        {
            if( virgin ) {
                virgin=false;
            } else {
                cl += ",";
            }
            
            cl += cn;
            ++n;
        }
        
        addAttribute( c_list_ean, cl, FXD_CIPHER_NAME_LIST, true );
    }
    
    addAttribute( c_name_ean,    SSL_get_cipher_name( ssl ),    FXD_CIPHER_NAME,    true );
    addAttribute( c_version_ean, SSL_get_cipher_version( ssl ), FXD_CIPHER_VERSION, true );
}


void
SocketContext::protectAgainstCrapCrypto( SSL* ssl )
    throw (CanNotGetStream)
{
    int alg_bits  = 0;
    int real_bits = SSL_get_cipher_bits( ssl, &alg_bits );

    if( real_bits < 128 )
    {
        stringstream ss;
        ss << "Error, crypto that is < 128 bits in length has been negotiated and"
           << " will not be used. Secure communication or none at all! "
           << " algorithm supports:" << alg_bits
           << " bits, only using:" << real_bits << " bits.";
        Throw_CanNotGetStream(tostr(ss),this);
    }
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

fh_iostream
SocketContext::getRealStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    LG_SOCKET_D << "getting real stream...SocketStream:" << SocketStream << endl;

    if( SocketStream )
    {
        LG_SOCKET_D << "getting real stream...1" << endl;
        iostream* retss = SocketStream;
        LG_SOCKET_D << "getting real stream...2" << endl;
        bool handOverOwnerShip = false;
        int the_fd = -1;

        LG_SOCKET_D << "ServerMode:" << ServerMode << endl;
        
        if( ServerMode )
        {
            sockbuf::sockdesc sdesc = (*SocketStream)->accept();
            retss = new iosockinet( sdesc );
            handOverOwnerShip = true;
            the_fd = sdesc.sock;
        }
        else
        {
//            the_fd = SocketStream->rdbuf()->getfd();
            the_fd = SocketStream->rdbuf()->sd();
        }

        LG_SOCKET_D << "the_fd:" << the_fd << endl;
        

#ifdef FERRIS_SOCKET_SSL
        if( UsingCrypto )
        {
            /* -------------------------------------------------- */
            /* TCP connection is ready. Setup SSL/TLS connection  */
            /* -------------------------------------------------- */

            if( !ssl_ctx )
            {
                stringstream ss;
                ss << "Error, crypto is desired and ssl_ctx has not been set, "
                   << "this is a serious internal ferris error.";
                Throw_FerrisCreateSubContextFailed(tostr(ss),this);
            }
            
            SSL* ssl = 0;
            ssl = SSL_new( ssl_ctx );
            if( !ssl )
            {
                stringstream ss;
                ss << "Error attaching a SSL object to the SSL_ctx";
                Throw_FerrisCreateSubContextFailed(tostr(ss),this);
            }

            LG_SOCKET_D << "the_fd:" << the_fd << endl;
            SSL_set_fd ( ssl, the_fd );

            int err = -1;
            string funcName = "SSL_accept()";
            
            if( ServerMode )
            {
                SSL_set_accept_state( ssl );
                err = SSL_accept( ssl );
            }
            else
            {
                SSL_set_connect_state( ssl );
                err = SSL_connect( ssl );
                funcName = "SSL_connect()";
            }
            
            if( err == -1 )
            {
                stringstream ss;
                ss << "Failed to " << funcName << " the connection. "
                   << " the_fd:" << the_fd
                   << " Error: " << ERR_error_string(ERR_get_error(),0);
                Throw_FerrisCreateSubContextFailed(tostr(ss),this);
            }

//             if(SSL_do_handshake( ssl ) < 0)
//             {
//                 stringstream ss;
//                 ss << "Failed to handshake after " << funcName << " the connection. "
//                    << " the_fd:" << the_fd
//                    << " Error: " << ERR_error_string(ERR_get_error(),0);
//                 Throw_FerrisCreateSubContextFailed(tostr(ss),this);
//             }

            /*
             * Make sure that no attempt is made to use a connection
             * protected by a key that is < 128 bits.
             */
            protectAgainstCrapCrypto( ssl );
            

            createPeerCertificateEA( ssl );
            setupCipherEA( ssl );
            
            return FerrisSSLStream<char>( retss, handOverOwnerShip, ssl );
        }
#endif
        
        return Factory::MakeProxyStream( retss, handOverOwnerShip );
    }

    LG_SOCKET_D << "using generic return..." << endl;
    return leafContext::priv_getIOStream( m | ios::out );
}




fh_istream
SocketContext::priv_getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    LG_SOCKET_D << "getting I stream..." << endl;
    return getRealStream( m );
}


fh_iostream
SocketContext::priv_getIOStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           CanNotGetStream,
           exception)
{
    LG_SOCKET_D << "getting IO stream..." << endl;
    return getRealStream( m );
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const string SocketListContext::CREATE_REMOTE_PORT = "remote-port";
const string SocketListContext::CREATE_REMOTE_HOST = "remote-hostname";
const string SocketListContext::CREATE_LOCAL_PORT  = "local-port";
const string SocketListContext::CREATE_LOCAL_HOST  = "local-hostname";
const string SocketListContext::CREATE_PROTOCOL    = "protocol";
const string SocketListContext::CREATE_STREAM      = "stream";
const string SocketListContext::CREATE_NUMBER_CONNECTIONS = "max-connection-count";
const string SocketListContext::CREATE_USE_SSLVERSION = "use-ssl-version";
const string SocketListContext::CREATE_USE_CERTFILE = "use-cert-file";
const string SocketListContext::CREATE_USE_PRIVKFILE = "use-privkey-file";


SocketListContext::SocketListContext(
    fh_context parent,
    const string& rdn,
    const string& filename )
    :
    Filename( filename )
{
    setContext( GetImpl(parent), rdn );
    InitOpenSSL();
}



void
SocketListContext::priv_read()
{
    EnsureStartStopReadingIsFiredRAII _raii1( this );
    
    LG_SOCKET_D << "priv_read(top) path:" << getDirPath()
                << " Filename:" << Filename
                << endl;

    clearContext();
    ifstream iss( Filename.c_str() );
    bool virgin = true;
    while( iss )
    {
        string s;
        getline( iss, s );

        if( virgin )
        {
            virgin = false;
            SocketContext::parseHeader( s );
        }
        else
        {
            LG_SOCKET_D << "read() dir:" << getDirName() << " newsub... s:" << s << endl;

            if( !s.empty() )
            {
                stringmap_t ea;
                const string& l = s;
                int oldidx = 0;
                int i = 0;
                string rdn = "unnamed";

                for( SocketContext::hdr_offsets_t::iterator iter = SocketContext::hdr_offsets.begin();
                     iter != SocketContext::hdr_offsets.end();
                     ++iter, ++i )
                {
                    const string attrName = SocketContext::hdr[i];
                    int endidx = *iter;
                    string attrValue;

                    if( oldidx >= l.length() )
                        break;

                    if( endidx != string::npos )
                    {
                        attrValue = string( l.begin() + oldidx, l.begin() + endidx);
                    }
                    else
                    {
                        attrValue = string( l.begin() + oldidx, l.end() );
                    }
                    attrValue = SocketContext::getTrimmer()(attrValue);

            
                    if( SocketContext::isKey(attrName) )
                    {
                        PostfixTrimmer t;
                        t.push_back( ":" );
                        rdn = t(attrValue);
                    }

                    LG_SOCKET_D << "Endidx:" << endidx
                                << " attrName:" << attrName
                                << " attrValue:" << attrValue
                                << endl;

                    ea[ attrName ] = attrValue;
                    addAttribute( attrName, attrValue, FXD_BINARY, true );
        
//                     if( attrName == "local_address" || attrName == "rem_address" )
//                     {
//                         addCompactEA( parent, attrName, attrValue );
//                     }
        
                    oldidx = *iter + 1;
                }

                
                SocketContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->constructObject( ea );
                ssAttacher( c );
                
//                 SocketContext* c = new SocketContext( this, s );
//                 Insert( c );
//                 ssAttacher( c );
            }
        }
    }
    
    LG_SOCKET_D << "priv_read(done) path:" << getDirPath() << endl;
}


Context*
SocketListContext::priv_CreateContext( Context* parent, string rdn )
{
    return new SocketListContext( parent, rdn, Filename );
}





void
SocketStreamAttacher::operator()( SocketContext* c )
{
    if( attached )
        return;
    
    try
    {
        string portv = getStrAttr( c, SoughtPortAttrName, "" );
        string hostv = "";

        if( SoughtHostIP.length() )
        {
            hostv = getStrAttr( c, SoughtHostIPAttrName, "" );
        }
        
        LG_SOCKET_D << "Attacher::operator() "
             << " SoughtPortAttrName:" << SoughtPortAttrName
             << " SoughtHostAttrName:" << SoughtHostIPAttrName
             << " SoughtPort:" << SoughtPort
             << " SoughtHost:" << SoughtHostIP
             << " portv:" << portv << " hostv:" << hostv
             << endl;
            
        if( SoughtPort == portv )
        {
            if( !SoughtHostIP.length() || SoughtHostIP == hostv )
            {
                /*
                 * Found the target, get and set info.
                 */
                c->setSocket( ss );
                c->ServerMode  = ServerMode;
                c->ssl_ctx     = ssl_ctx;
                c->UsingCrypto = UsingCrypto;
                    
                this->c = c;
                attached = true;
            }
        }
    }
    catch( ... )
    {}
}


const fh_context&
SocketListContext::createSubContext_connect( fh_context md,
                                             const string& remote_port,
                                             const string& remote_host,
                                             string& protocol,
                                             string& stream,
                                             SSL_METHOD* ssl_meth,
                                             SSL_CTX* ssl_ctx,
                                             bool UsingCrypto
    )
    throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
{
    iosockinet* sock_ss = 0;
    string remote_host_rev_lookup;
    try
    {
        /*
         * Create the socket.
         * Reread the dir.
         * Add the SocketContext describing the new socket. (binding it to the socket)
         * Return the sockctx.
         */
        sock_ss = new iosockinet(
            stream == "1"
            ? sockbuf::sock_stream
            : sockbuf::sock_dgram
            );

        LG_SOCKET_D << "remote_host :" << remote_host << endl;
        LG_SOCKET_D << "remote_port :" << remote_port << endl;
        LG_SOCKET_D << "protocol    :" << protocol << endl;
        LG_SOCKET_D << "stream      :" << stream << endl;
        
        try
        {
            (*sock_ss)->connect( remote_host.c_str(), toint(remote_port) );
        }
        catch( sockerr& se )
        {
            LG_SOCKET_W << "se:" << se.errstr() << endl;

            stringstream ss;
            ss << "Error creating socket. " << se.errstr() << endl;
            Throw_FerrisCreateSubContextFailed(tostr(ss),this);
        }
        catch( exception& e )
        {
            throw;
        }
        
        LG_SOCKET_D << "Perform reverse lookup for remote_host:" << remote_host << endl;
//        remote_host_rev_lookup = (*sock_ss)->peerhost();
        string SoughtHostIP = RemoteIP32ToString( sock_ss );
        LG_SOCKET_D << "ipaddr:" << SoughtHostIP << endl;
        
        
        LG_SOCKET_D << "Got reverse of :" << remote_host_rev_lookup << endl;

        /*
         * The attacher will be called for each element, and will attach the
         * open socket for us.
         */
        ssAttacher.reset();
        ssAttacher.SoughtHostIP       = SoughtHostIP;
        ssAttacher.SoughtPort         = remote_port;
        ssAttacher.ss                 = sock_ss;
        ssAttacher.SoughtHostIPAttrName = "remote-ip";
        ssAttacher.SoughtPortAttrName = "remote-port";
        ssAttacher.ssl_ctx            = ssl_ctx;
        ssAttacher.UsingCrypto        = UsingCrypto;
        

        read(true);


        if( ssAttacher.attached )
        {
            return ssAttacher.c;
        }
    }
    catch( exception& e )
    {
        stringstream ss;
        ss << "Error creating socket. " << e.what() << endl;
        Throw_FerrisCreateSubContextFailed(tostr(ss),this);
    }
    
    
    delete sock_ss;

    stringstream ss;
    ss << "Socket lookup failed. The socket was able to be created but failed to"
       << " be found in the socket lookup using /proc/net. "
       << " remote_host_rev_lookup:" << remote_host_rev_lookup
       << " remote_port:" << remote_port
       << endl;

    Throw_FerrisCreateSubContextFailed(tostr(ss),this);
}


const fh_context&
SocketListContext::createSubContext_bind( fh_context md,
                                          const string& local_port,
                                          const string& local_host,
                                          string& protocol,
                                          string& stream,
                                          SSL_METHOD* ssl_meth,
                                          SSL_CTX* ssl_ctx,
                                          bool UsingCrypto
    )
    throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
{
    iosockinet* sock_ss = 0;

    try
    {
        /*
         * Create the socket.
         * Reread the dir.
         * Add the SocketContext describing the new socket. (binding it to the socket)
         * Return the sockctx.
         */
        sock_ss = new iosockinet(
            stream == "1"
            ? sockbuf::sock_stream
            : sockbuf::sock_dgram
            );

        LG_SOCKET_D << "local_host :" << local_host << endl;
        LG_SOCKET_D << "local_port :" << local_port << endl;
        LG_SOCKET_D << "protocol   :" << protocol << endl;
        LG_SOCKET_D << "stream     :" << stream << endl;
        
        try
        {
            (*sock_ss)->bind( local_host.length()
                                 ? local_host.c_str()
                                 : (unsigned long) INADDR_ANY,
                                 toint(local_port) );

            string numConnections = getStrSubCtx( md, CREATE_NUMBER_CONNECTIONS, "1" );
            if( !numConnections.length() )
                numConnections = "1";
            
            (*sock_ss)->listen( toint(numConnections) );
        }
        catch( sockerr& se )
        {
            LG_SOCKET_W << "se:" << se.errstr() << endl;

            stringstream ss;
            ss << "Error creating socket. " << se.errstr() << endl;
            Throw_FerrisCreateSubContextFailed(tostr(ss),this);
        }
        catch( exception& e )
        {
            throw;
        }
        
        LG_SOCKET_D << "bound() localhost:"
                    << (*sock_ss)->localhost() << ' ' << (*sock_ss)->localport() << endl;


        string SoughtHostIP = "";
        if( local_host.length() )
        {
            LG_SOCKET_D << "setting SoughtHostIP" << endl;
            try
            {
                SoughtHostIP = LocalIP32ToString( sock_ss );
            }
            catch( exception& e )
            {
                LG_SOCKET_D << "e" << e.what() << endl;
                throw e;
            }
            catch( sockerr& e )
            {
                LG_SOCKET_D << "sock e" << e.errstr() << endl;
                throw e;
            }
            catch( ... )
            {
                LG_SOCKET_D << "e ..." << endl;
                throw;
            }
            
            LG_SOCKET_D << "ipaddr:" << SoughtHostIP << endl;
        }
        LG_SOCKET_D << "SoughtHostIP:" << SoughtHostIP << endl;
        
        /*
         * The attacher will be called for each element, and will attach the
         * open socket for us.
         */
        ssAttacher.reset();
        ssAttacher.SoughtHostIP       = SoughtHostIP;
        ssAttacher.SoughtPort         = local_port;
        ssAttacher.ss                 = sock_ss;
        ssAttacher.ServerMode         = true;
        ssAttacher.SoughtHostIPAttrName = "local-ip";
        ssAttacher.SoughtPortAttrName = "local-port";
        ssAttacher.ssl_ctx            = ssl_ctx;
        ssAttacher.UsingCrypto        = UsingCrypto;

        LG_SOCKET_D << "About to attach()" << endl;
        read(true);
        LG_SOCKET_D << "About to attach(done)" << endl;


        if( ssAttacher.attached )
        {
            LG_SOCKET_D << "SocketListContext::createSubContext_bind() success" << endl;
            return ssAttacher.c;
        }
    }
    catch( exception& e )
    {
        LG_SOCKET_D << "SocketListContext::createSubContext_bind() e" << endl;
        stringstream ss;
        ss << "Error creating socket. " << e.what() << endl;
        Throw_FerrisCreateSubContextFailed(tostr(ss),this);
    }
    
    LG_SOCKET_D << "SocketListContext::createSubContext_bind() failure cleanup" << endl;
    
    delete sock_ss;

    stringstream ss;
    ss << "Socket lookup failed. The socket was able to be created but failed to"
       << " be found in the socket lookup using /proc/net.";
    Throw_FerrisCreateSubContextFailed(tostr(ss),this);
}

const string SSLVERSION2 = "SSL2";
const string SSLVERSION3 = "SSL3";
const string TLSVERSION1 = "TLS";


fh_context
SocketListContext::SubCreateSocket( fh_context c, fh_context md )
{
    fh_context ret;
    try
    {
        const string& remote_port = getStrSubCtx( md, CREATE_REMOTE_PORT, "" );
        const string& remote_host = getStrSubCtx( md, CREATE_REMOTE_HOST, "" );
        const string& local_port  = getStrSubCtx( md, CREATE_LOCAL_PORT, "" );
        const string& local_host  = getStrSubCtx( md, CREATE_LOCAL_HOST, "" );

#ifdef FERRIS_SOCKET_SSL
        const string& sslversion  = getStrSubCtx( md, CREATE_USE_SSLVERSION, "None" );
        bool UsingCrypto = true;

        SSL_METHOD* ssl_meth;
        if( sslversion == SSLVERSION2 )
        {
            ssl_meth = (SSL_METHOD*)SSLv2_method();
        }
        else if( sslversion == SSLVERSION3 )
        {
            ssl_meth = (SSL_METHOD*)SSLv3_method();
        }
        else if( sslversion == TLSVERSION1 )
        {
            LG_SOCKET_D << "Using TLS level security." << endl;
            ssl_meth = (SSL_METHOD*)TLSv1_method();
        }
        else if( sslversion == "None" )
        {
            UsingCrypto = false;
        }
        else
        {
            stringstream ss;
            ss << "Invalid selection presented for sslversion:" << sslversion;
            Throw_FerrisCreateSubContextFailed(tostr(ss),this);
        }

        SSL_CTX* ssl_ctx = 0; 
    
        if( UsingCrypto )
        {
            LG_SOCKET_D << "Getting new CTX, ssl_meth:" << (void*)ssl_meth << endl;
            ssl_ctx = SSL_CTX_new (ssl_meth);
            if ( !ssl_ctx )
            {
                stringstream ss;
                ss << "Can not create openssl context. Error: "
                   << ERR_error_string(ERR_get_error(),0);
                Throw_FerrisCreateSubContextFailed(tostr(ss),this);
            }

            /*
             * Block method to only allow selected protocols
             *   also,
             * First attempt to drop out ciphers < 128 bits in length
             */
            string ssl_cipher_list = "TLSv1:!LOW:!EXP:!NULL:!eNULL:@STRENGTH";
        
            if( sslversion == SSLVERSION2 )
            {
                SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 );
                ssl_cipher_list = "SSLv2:!LOW:!EXP:!NULL:!eNULL:@STRENGTH";
            }
            else if( sslversion == SSLVERSION3 )
            {
                SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_TLSv1 );
                ssl_cipher_list = "SSLv3:!LOW:!EXP:!NULL:!eNULL:@STRENGTH";
            }
            else if( sslversion == TLSVERSION1 )
            {
                SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 );
                ssl_cipher_list = "TLSv1:!LOW:!EXP:!NULL:!eNULL:@STRENGTH";
            }

            LG_SOCKET_I << " ssl_cipher_list : " << ssl_cipher_list << endl;
            SSL_CTX_set_cipher_list( ssl_ctx, ssl_cipher_list.c_str() );
        }
    
#endif
    
        string protocol = getStrSubCtx( md, CREATE_PROTOCOL, "" );
        string stream   = getStrSubCtx( md, CREATE_STREAM, "" );

        if( protocol.length() == 0 )
        {
            protocol = getDirName();
        }
    
        if(    getDirPath() == "/tcp" && protocol != "tcp"
               || getDirPath() == "/udp" && protocol != "udp"
            )
        {
            stringstream ss;
            ss << "The correct protocol must be chosen for the context that the"
               << " socket is created in";
            Throw_FerrisCreateSubContextFailed(tostr(ss),this);
        }

        if( protocol == "tcp" )
        {
            stream = "1";
        }
        else if( protocol == "udp" )
        {
            stream = "0";
        }
    
        LG_SOCKET_D << "done preconditions" << endl;

    
        bool isClient = remote_port.length() && remote_host.length();
        bool isServer = !isClient && local_port.length();
    
#ifdef FERRIS_SOCKET_SSL
        if( isServer && UsingCrypto )
        {
            if( !isTrue( stream ) )
            {
                stringstream ss;
                ss << "SSL/TLS servers and clients must be stream (tcp) servers.";
                Throw_FerrisCreateSubContextFailed(tostr(ss),this);
            }
        
            /*
             * Check that server is ready.
             */
            const string& certfile  = getStrSubCtx(md, CREATE_USE_CERTFILE, "" );
            const string& privkfile = getStrSubCtx(md, CREATE_USE_PRIVKFILE, "" );

        
            if( certfile.length() )
            {
                if( SSL_CTX_use_certificate_file( ssl_ctx,
                                                  certfile.c_str(),
                                                  SSL_FILETYPE_PEM) <= 0)
                {
                    stringstream ss;
                    ss << "Can not use certificate file:" << certfile
                       << " Error: " << ERR_error_string(ERR_get_error(),0);
                    Throw_FerrisCreateSubContextFailed(tostr(ss),this);
                }
            }
        

            if( privkfile.length() )
            {
                if (SSL_CTX_use_PrivateKey_file( ssl_ctx,
                                                 privkfile.c_str(),
                                                 SSL_FILETYPE_PEM) <= 0)
                {
                    stringstream ss;
                    ss << "Can not use private key file:" << privkfile
                       << " Error: " << ERR_error_string(ERR_get_error(),0);
                    Throw_FerrisCreateSubContextFailed(tostr(ss),this);
                }
    
                if( certfile.length() )
                {
                    if (!SSL_CTX_check_private_key( ssl_ctx ))
                    {
                        stringstream ss;
                        ss << "Failed to check the private key for validity. Key file:" << privkfile
                           << " Error: " << ERR_error_string(ERR_get_error(),0);
                        Throw_FerrisCreateSubContextFailed(tostr(ss),this);
                    }
                }
            }
        
        }
#endif    

        if( isClient )
        {
            return createSubContext_connect( md, remote_port, remote_host,
                                             protocol, stream,
                                             ssl_meth, ssl_ctx, UsingCrypto

                );
        }
        else if( isServer )
        {
            return createSubContext_bind( md, local_port, local_host,
                                          protocol, stream,
                                          ssl_meth, ssl_ctx, UsingCrypto
                );
        }
    
        stringstream ss;
        ss << "Not enough information was found to bind a local or remote socket"
           << " connection. sorry. Please specify "
           << CREATE_REMOTE_PORT << " and " << CREATE_REMOTE_HOST
           << " or " << CREATE_LOCAL_PORT << " optionally with " << CREATE_LOCAL_HOST
           << " and possibly " << CREATE_PROTOCOL << " , " << CREATE_STREAM;

        LG_SOCKET_D << tostr(ss) << endl;
        dumpOutItems();
        SubContextNames_t na = md->getSubContextNames();
        for( SubContextNames_t::iterator iter = na.begin();
             iter != na.end(); ++iter )
        {
            LG_SOCKET_D << *iter << endl;
            try
            {
                fh_context child = md->getSubContext( *iter );
                fh_istream ss = child->getIStream();
                LG_SOCKET_D << " : " << StreamToString(ss) << endl;
            }
            catch( exception& e )
            {
                LG_SOCKET_D << " .. err:" << e.what() << endl;
            }
            
        }
        LG_SOCKET_D << "---------------------------" << endl;
        LG_SOCKET_D << " remote_port:" << remote_port
             << " remote_host:" << remote_host
             << " local_port:" << local_port
             << " test:" << getStrSubCtx( md, "remote-port", "def" )
             << endl;
        LG_SOCKET_D << "---------------------------" << endl;
        
        Throw_FerrisCreateSubContextFailed(tostr(ss),this);
    }
    catch( exception& e )
    {
        Throw_FerrisCreateSubContextFailed( e.what(), 0 );
    }
    return ret;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
        static FakeInternalContext* c = 0;
        const string& root = rf->getInfo( RootContextFactory::ROOT );

        if( !c )
        {
            LG_SOCKET_D << "Making FakeInternalContext(1) " << endl;
            c = new FakeInternalContext(0, "/");
            
            // Bump ref count.
            static fh_context keeper = c;
            static fh_context keeper2 = keeper;

            c->addNewChild( new SocketListContext( c, "tcp",  "/proc/net/tcp" ) );
            c->addNewChild( new SocketListContext( c, "udp",  "/proc/net/udp" ) );
            c->addNewChild( new SocketListContext( c, "raw",  "/proc/net/raw" ) );
            c->addNewChild( new SocketListContext( c, "unix", "/proc/net/unix" ) );

        }

        fh_context ret = c;

        if( root != "/" )
        {
            string path = rf->getInfo( RootContextFactory::PATH );
            string newpath = root;

            newpath += "/";
            newpath += path;

            rf->AddInfo( RootContextFactory::PATH, newpath );
//            ret = ret->getRelativeContext( "./" + root, 0 );
        }
        return ret;
    }
}


 
};
