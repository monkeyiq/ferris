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

    $Id: libnativembox.cpp,v 1.5 2010/09/24 21:31:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Trimming.hh>

#include <iomanip>
#include <errno.h>
#include <Native.hh>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/version.hpp>

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

const string FROM_ = "From ";


/*
 * This is a bit of a hack to avoid seekg()ing for every message.
 *
 * Its basically a "put back token" buffer for a whole line token.
 * This is due to the message body being delimited by a "From " at
 * the start of a new message. 
 */
string PreReadFromLine = "";


/*
 * Main context for the mbox. This contains a bunch of MessageContext
 * subcontexts that have their names mangled.
 *
 * The name of each subcontext is taken from the header of the email
 * itself using the MessageContext::getNamingKeys() partial order.
 */
class FERRISEXP_CTXPLUGIN MBoxContext
    :
        public Context
{
    // library entry point
    friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );

    Context* priv_CreateContext( Context* parent, string rdn );

    // Where to get the mail from.
    fh_istream BoxStream;

protected:

    virtual void priv_read();
    fh_iostream getMetaDataStream( bool create );
    
    
public:


    MBoxContext(
        const fh_context& omc,
        const string& rdn,
        const fh_istream& ss );

    static MBoxContext* Instance(
        const fh_context& omc,
        const string& rdn,
        const fh_istream& ss );

};

FERRIS_SMARTPTR( MBoxContext, fh_mbcontext )


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class FERRISEXP_CTXPLUGIN MessageContext : public leafContext
{
    bool BodyNeedsEscaping;     // true of the msg has >From at the start of a line
    streampos StartOffsetOfHeader;  // Start of msg header
    streampos StartOffsetOfMsgBody; // Start of msg body
    streampos   EndOffsetOfMsgBody; // End of


    void   parseFromLine( const string& FromLine );
    string parseHeaders( fh_istream& ss );
    void   parseBody(  fh_istream& ss );

    /*
     * Get the keys for the email headers to try in order to find the
     * name of the message
     */
    typedef list<string> namers_t;
    const namers_t& getNamingKeys();
    
    bool   checkIfLineNeedsEscape( const string& l );
    string escapeBodyLine( const string& l );

    fh_mbcontext MBoxCTX;
    
protected:

    virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
        throw (FerrisParentNotSetError,
               CanNotGetStream,
               std::exception);

    void setupRDN( MBoxContext* parent, string rdn );

    
    
public:

    MessageContext(
        MBoxContext* parent,
        const string& FromLine,
        fh_istream& ss,
        const fh_mbcontext& mboxctx
        );

    MessageContext(
        MBoxContext* parent,
        guint64 startOfHeader,
        guint64 endOfHeader,
        guint64 startOfBody,
        guint64 endOfBody,
        string headers,
        const fh_mbcontext& mboxctx
        );
    
    
    streampos getStartOffsetOfHeader() const 
        {
            return StartOffsetOfHeader;
        }
    streampos getEndOffsetOfHeader() const
        {
            return StartOffsetOfMsgBody - (streampos)1;
        }
    streampos getStartOffsetOfBody() const
        {
            return StartOffsetOfMsgBody;
        }
    streampos getEndOffsetOfbody() const
        {
            return EndOffsetOfMsgBody;
        }
    
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void
MessageContext::setupRDN( MBoxContext* parent, string rdn )
{
    PrefixTrimmer trimmer;
    trimmer.push_back( " " );
    rdn = trimmer( rdn );
    
    try {
        LG_MBOX_D << "Message Name is:" << rdn << endl;

        const string& monsteredRDN = parent->monsterName(rdn);
        setContext( parent, monsteredRDN );
    }
    catch( NoSuchAttribute& e )
    {
        LG_MBOX_ER << "Can not get Subject header line..." << endl;
        throw;
    }
}


MessageContext::MessageContext(
    MBoxContext* parent,
    const string& FromLine,
    fh_istream& ss,
    const fh_mbcontext& mboxctx
    )
    :
    MBoxCTX( mboxctx ),
    BodyNeedsEscaping( false ),
    StartOffsetOfHeader( -1 ),
    StartOffsetOfMsgBody( -1 ),
    EndOffsetOfMsgBody( -1 )
{
    LG_MBOX_D << "Fromline:" << FromLine << endl;

    StartOffsetOfHeader = ss->tellg();

    parseFromLine( FromLine );
    string rdn = parseHeaders( ss );
    setupRDN( parent, rdn );
    parseBody( ss );

    createStateLessAttributes();
}

MessageContext::MessageContext(
    MBoxContext* parent,
    guint64 startOfHeader,
    guint64 endOfHeader,
    guint64 startOfBody,
    guint64 endOfBody,
    string headers,
    const fh_mbcontext& mboxctx
    )
    :
    MBoxCTX( mboxctx ),
    BodyNeedsEscaping( false ),
    StartOffsetOfHeader( startOfHeader ),
    StartOffsetOfMsgBody( startOfBody ),
    EndOffsetOfMsgBody( endOfBody )
{
    fh_stringstream ss;
    ss << headers;
    string rdn = parseHeaders( ss );
    setupRDN( parent, rdn );

    createStateLessAttributes();
}



/*
 * If the given line needs escaping applied then chomp of a leading '>'
 */
string
MessageContext::escapeBodyLine( const string& l )
{
    if( checkIfLineNeedsEscape(l) )
    {
        return l.substr( 1, l.length()-1 );
    }
    return l;
}



/*
 * Returns true of the line contains ( '>' )* "From " ... NL
 */
bool
MessageContext::checkIfLineNeedsEscape( const string& l )
{
    bool v = false;
    string::size_type fc = l.find_first_not_of( '>' );

//    if( fc != string::npos && !l.compare( FROM_, fc, fc+FROM_.length() ))
    if( fc != string::npos && !l.compare( fc, fc+FROM_.length(), FROM_ ))
    {
        v = true;
    }
    return v;
}




/*
 * If the given stream is seekable then lazy load the file.
 * Otherwise load the body right away.
 *
 * On Entry Stream is at start of body
 * On Exit  Strean is just before the next FROM_ line
 */
void
MessageContext::parseBody( fh_istream& ss )
{
    string s;

    LG_MBOX_D << "MessageContext::parseBody(1)" << endl;
    LG_MBOX_I << endl;

    /*
     * Check of msg body can be lazy loaded.
     */
    StartOffsetOfMsgBody = ss->tellg();
    
    while( ss->good() )
    {
        LG_MBOX_D << "MessageContext::parseBody(while)" << endl;
        getline( ss, s );
        LG_MBOX_D << "MessageContext::parseBody(while) s:" << s << endl;
        if( starts_with( s, FROM_ ))
        {
            LG_MBOX_D << "MessageContext::parseBody(start with from_)" << endl;

            streampos tellg = ss->tellg();
            EndOffsetOfMsgBody =  tellg - (streampos)s.length() - (streampos)1;
            
            // We can either seekg() or just cache the From_ line.
            PreReadFromLine = s;
            //ss->seekg( 0 - s.length() - 1, ios::cur );
            
            break;
        }
        
        BodyNeedsEscaping |= checkIfLineNeedsEscape(s);
    }
    if( EndOffsetOfMsgBody == (streampos)-1 )
    {
        ss->clear();
//        cerr << "TELLG:" << ss->tellg() << endl;
        EndOffsetOfMsgBody = ss->tellg();
    }
    
    LG_MBOX_D << "MessageContext::parseBody(4)" << endl;

    /*
     * Expose a little EA
     */
    addAttribute( "message-body-needs-from-escaping",
                  string(BodyNeedsEscaping ? "1" : "0"),
                  XSD_BASIC_BOOL );
}


/*
 *
 * Parse from stream stopping at NL NL and reading
 * key:value
 * or
 * \t more value
 *
 * On entry the stream is at the start of a key
 * On exit the stream has consumed the NL NL pair and is at the next char
 *
 * Returns the rdn for this message
 */
string
MessageContext::parseHeaders( fh_istream& ss )
{
    string s;
    string k;
    string v;

    typedef map< string, string > kv_t;
    kv_t kv;
    
    
    while( ss->good() )
        {
            getline( ss, s );

            if( !s.length() )
            {
                kv[ k ] = v;
                break;
            }

            if( s[0] == '\t' || s[0] == ' ' )
            {
                v+="\n";
                v+=s;
            }
            else
            {
                kv[ k ] = v;

                istringstream ss( s );
                getline( ss, k, ':' );

                /*
                 * If we have already seen this header before, find it and read
                 * the value into a new line of the header, read the value as
                 * a new string
                 */
                if( kv.end() != kv.find( k ) )
                {
                    string tmp;

                    v = kv[ k ];
                    getline( ss, tmp );
                    v += "\n\n";
                    v += tmp;
                }
                else
                {
                    getline( ss, v );
                }
            }
        }

    /*
     * Create EA for each of the headers.
     */
    for( kv_t::const_iterator iter = kv.begin();
         iter != kv.end();
         iter++ )
    {
        addAttribute( iter->first, iter->second, XSD_BASIC_STRING );
    }


    /*
     * Debug loop puts the entire header into the log :/
     */
    LG_MBOX_D << "========== Printing message header start ================" << endl;
    for( kv_t::const_iterator iter = kv.begin();
         iter != kv.end();
         iter++ )
    {
        LG_MBOX_D << " key:" << iter->first << " value:" << iter->second << endl;
    }
    LG_MBOX_D << "========== Printing message header end ================" << endl;
    


    /*
     * While we have the nice hash table of k/v we can pick the string that is
     * going to be our name
     */
    for( namers_t::const_iterator iter = getNamingKeys().begin();
         iter != getNamingKeys().end();
         iter++ )
    {
        kv_t::const_iterator foundloc = kv.find( *iter );
        if( kv.end() != foundloc )
        {
            return foundloc->second;
        }
    }
    
    return "unnamed";
}

const MessageContext::namers_t&
MessageContext::getNamingKeys()
{
    static namers_t namers;
    static bool virgin = true;

    if( virgin )
    {
        virgin = false;
        
        namers.push_back( "Subject" );
        namers.push_back( "subject" );
        namers.push_back( "SUBJECT" );
        namers.push_back( "From" );
        namers.push_back( "from" );
    }
    
    return namers;
}



void
MessageContext::parseFromLine( const string& FromLine )
{
    istringstream ss(FromLine);
    string s;

    while( ss )
        {
            ss >> s;
        }
}

    
fh_istream
MessageContext::priv_getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    LG_MBOX_I << "getting stream..." << endl;

    fh_istream ss = MBoxCTX->getCoveredContext()->getIStream( m );
    return Factory::MakeLimitingIStream( ss,
                                         StartOffsetOfMsgBody,
                                         EndOffsetOfMsgBody);
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




MBoxContext::MBoxContext(
    const fh_context& omc,
    const string& rdn,
    const fh_istream& ss )
    :
    Context( GetImpl(omc) ),
    BoxStream(ss)
{
    setContext( 0, rdn );
}




    struct MessageIndex_t
    {
        friend class boost::serialization::access;
        guint64 startOfHeader;
        guint64 endOfHeader;
        guint64 startOfBody;
        guint64 endOfBody;
        string headers;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
            {
                ar & startOfHeader;
                ar & endOfHeader;
                ar & startOfBody;
                ar & endOfBody;
                ar & headers;
            }
        
        
    };
    typedef list<MessageIndex_t> MBoxIndex_t;

void
MBoxContext::priv_read()
{
    LG_MBOX_I << endl;

    emitExistsEventForEachItemRAII _raii1( this );

    MBoxIndex_t MBoxIndex;
    
    if( empty() )
    {
        
        clearContext();

        LG_MBOX_I << "MBoxContext::priv_read() path:" << getDirPath() << endl;
        bool manualParse = false;
        bool forceReindex = false;

        if( const gchar* p = g_getenv ("LIBFERRIS_FORCE_REINDEX_MBOX") )
        {
            forceReindex = true;
        }
        
        try
        {
            if( forceReindex )
            {
                manualParse = true;
            }
            LG_MBOX_I << "forceReindex:" << forceReindex << " manualParse:" << manualParse << endl;
            
            if( !forceReindex )
            {
                fh_iostream mdss = getMetaDataStream( false );
//               LG_MBOX_I << "getMetaDataStream(done) DATA.sz:" << StreamToString(mdss).size() << endl;
//               LG_MBOX_I << "getMetaDataStream(done) DATA:" << StreamToString(mdss) << endl;

            
                boost::archive::text_iarchive archive( mdss );
                LG_MBOX_D << "MBoxContext::priv_read(idx=1) using mbox index..." << endl;
                archive >> MBoxIndex;
                MBoxIndex_t::iterator e = MBoxIndex.end();
                LG_MBOX_D << "MBoxContext::priv_read() using mbox index size:" << MBoxIndex.size() << endl;
                for( MBoxIndex_t::iterator iter = MBoxIndex.begin(); iter != e ; ++iter )
                {
                    LG_MBOX_D << "got msg."
                              << " hstart:" << iter->startOfHeader
                              << " hend:" << iter->endOfHeader
                              << " bstart:" << iter->startOfBody
                              << " bend:" << iter->endOfBody
                              << endl;
                }
            
                for( MBoxIndex_t::iterator iter = MBoxIndex.begin(); iter != e ; ++iter )
                {
                    MessageContext* c = new MessageContext(
                        this,
                        iter->startOfHeader,
                        iter->endOfHeader,
                        iter->startOfBody,
                        iter->endOfBody,
                        iter->headers,
                        this );
                    Insert( c );
                }

                streampos lastpos = 0;
                if( !MBoxIndex.empty() )
                {
                    MBoxIndex_t::iterator e = MBoxIndex.end();
                    --e;
                    lastpos = e->endOfBody;
                }

                LG_MBOX_D << "Switching to manual mode to index new messages from offset:" << lastpos << endl;
                // check for any extra messages at end of mailbox
                BoxStream.seekg( lastpos );
                manualParse = true;
            }
        }
        catch( NoSuchObject& e )
        {
            manualParse = true;
        }

        long manuallyParsedMessageCount = 0;
        if( manualParse )
        {
            LG_MBOX_D << "MBoxContext::priv_read(idx=0) index not found or invalid" << endl;
            LG_MBOX_I << "priv_read(top) path:" << getDirPath() << endl;
            BoxStream.clear();
            BoxStream.seekg( 0 );
            BoxStream.clear();
            
            for( string s; BoxStream; )
            {
                if( !PreReadFromLine.length() )
                {
                    getline(BoxStream,s);
                }
                else
                {
                    s = PreReadFromLine;
                    PreReadFromLine = "";
                }

        
                LG_MBOX_D << "got from line :" << s << endl;
        
                if( starts_with( s, FROM_ ))
                {
                    ++manuallyParsedMessageCount;
                    MessageContext* c = new MessageContext( this, s, BoxStream, this );
                    Insert( c );

                    // Here to end of this block is just updating the index
                    {
                        MessageIndex_t msgIdx;
                        msgIdx.startOfHeader = c->getStartOffsetOfHeader();
                        msgIdx.endOfHeader   = c->getEndOffsetOfHeader();
                        msgIdx.startOfBody   = c->getStartOffsetOfBody();
                        msgIdx.endOfBody     = c->getEndOffsetOfbody();
                        string headersString = "";
                        {
                            giStreamPosRAII raii1( BoxStream );
                            BoxStream.seekg( msgIdx.startOfHeader );
                            long sz = msgIdx.endOfHeader - msgIdx.startOfHeader;
                            headersString.resize( sz );
                            BoxStream.read( (char*)headersString.data(), sz  );
                        }
                        msgIdx.headers = headersString;
                        MBoxIndex.push_back( msgIdx );
                    }
                }
            }
        }
        
        // save the index
        if( manuallyParsedMessageCount )
        {
            fh_iostream mdss = getMetaDataStream( true );
            boost::archive::text_oarchive archive( mdss );
            archive << (const MBoxIndex_t&)MBoxIndex;
            LG_MBOX_D << "MBoxIndex.sz:" << MBoxIndex.size() << endl;
        }
    }
    
    LG_MBOX_I << "priv_read(done) path:" << getDirPath() << endl;
}

fh_iostream
MBoxContext::getMetaDataStream( bool create )
{
    fh_iostream ret;
    LG_MBOX_I << "getMetaDataStream(top)" << endl;
    const string attrName = "ferris-mbox-index";
    fh_context cc = getCoveredContext();
    
    if( cc->isAttributeBound( attrName, true ) )
    {
        LG_MBOX_I << "getMetaDataStream(1)" << endl;
        fh_attribute a = cc->getAttribute( attrName );
        ret = a->getIOStream( std::ios::in|std::ios::out );
        LG_MBOX_I << "getMetaDataStream(done) had cache" << endl;
//        LG_MBOX_I << "getMetaDataStream(done) DATA:" << StreamToString(ret) << endl;
        return ret;
    }

    if( !create )
    {
        LG_MBOX_I << "getMetaDataStream(no create)" << endl;
        stringstream ss;
        ss << "No index for mbox" << endl;
        Throw_NoSuchObject( tostr(ss), this );
    }


    LG_MBOX_I << "getMetaDataStream(b1)" << endl;
    Shell::createEA( cc, attrName, "", "rdf" );
    LG_MBOX_I << "getMetaDataStream(b2)" << endl;
    fh_attribute a = cc->getAttribute( attrName );
    LG_MBOX_I << "getMetaDataStream(b3)" << endl;
    ret = a->getIOStream( std::ios::out|std::ios::trunc );
    LG_MBOX_I << "getMetaDataStream(b4)" << endl;
    LG_MBOX_I << "getMetaDataStream(done) created cache" << endl;
    return ret;
}




Context*
MBoxContext::priv_CreateContext( Context* parent, string rdn )
{
    Throw_FerrisCanNotCreateLeafOfLeaf( "", this );
}



MBoxContext*
MBoxContext::Instance(
    const fh_context& omc,
    const string& rdn,
    const fh_istream& ss )
{
    MBoxContext* ret = new MBoxContext( omc, rdn, ss );
    ret->setContext( 0, rdn );
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
        const string& rdn = rf->getInfo( "Root" );
        
        f_ifstream ss( rdn );
        return new MBoxContext( rf->getBaseOverMountContext(), rdn, ss );
    }
}


 
};
