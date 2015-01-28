/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrisxmp.cpp,v 1.3 2009/10/02 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <Cache.hh>
#include <libferrisredlandeashared.hh>

using namespace std;

namespace Ferris
{
    using namespace RDFCore;


#define EA_XMPINDEX_PREFIX "http://witme.sf.net/libferris-core/xmp-0.1/"
    static const char* EA_XMPINDEX_MTIME   = EA_XMPINDEX_PREFIX "index-mtime";
    static const char* EA_XMPINDEX_OFFSETS = EA_XMPINDEX_PREFIX "index-offsets";


    class FERRISEXP_DLLLOCAL FerrisXMPHandler
        :
        public CacheHandlable
    {
        static const char* packet_header_utf8;
        static const int   packet_header_utf8_length;
        static const char* packet_header_utf8_end;

        fh_context m_ctx;
        fh_model   m_model;

        bool findString( fh_istream& iss, const string& s, const std::set<char>& terminalChars );
        bool findString( fh_istream& iss, const string& s );
        bool moveToEndOfXMPPacket( fh_istream& iss, const streamsize start_of_packet );
        void copyTo( fh_istream& iss_raw, streamsize begin, streamsize end, fh_ostream& oss );

        void loadXMPFromIndex();
        void reindexXMP();
    
    public:

        FerrisXMPHandler( fh_context c );
        virtual ~FerrisXMPHandler();
        virtual void sync();
        virtual void dumpRDF();
        fh_model getModel();
    };
    FERRIS_SMARTPTR( FerrisXMPHandler, fh_xmpHandler );

    const char* FerrisXMPHandler::packet_header_utf8 = 
    "\x3C" "\x3F" "\x78" "\x70" "\x61" "\x63" "\x6B"
    "\x65" "\x74" "\x20" "\x62" "\x65" "\x67" "\x69" "\x6E" "\x3D";
    const int FerrisXMPHandler::packet_header_utf8_length = 16;
    const char* FerrisXMPHandler::packet_header_utf8_end =
    FerrisXMPHandler::packet_header_utf8
    + FerrisXMPHandler::packet_header_utf8_length;



    FerrisXMPHandler::FerrisXMPHandler( fh_context c )
        :
        m_ctx( c )
    {
        time_t idxmtime = toType< time_t >( getStrAttr( c, EA_XMPINDEX_MTIME, "0" ));
        if( idxmtime >= toType< time_t >(getStrAttr( c, "mtime", "1" )))
        {
            LG_RDF_D << "file:" << c->getURL() << endl
                     << " not modified since indexing, reusing index offsets to RDF chunks." << endl;
            loadXMPFromIndex();
        }
        else
        {
            try
            {
                LG_RDF_D << "file:" << c->getURL() << endl
                         << " modified since indexing, rescanning entire file again for XMP." << endl;
                reindexXMP();
            }
            catch( exception& e )
            {
                setStrAttr( m_ctx, EA_XMPINDEX_OFFSETS, "", true );
                throw;
            }
        }
    }

    FerrisXMPHandler::~FerrisXMPHandler()
    {
    }

    void
    FerrisXMPHandler::loadXMPFromIndex()
    {
        fh_uri base_uri = new URI( m_ctx->getURL() );
        m_model = Model::MemoryOnlyModel();

        string offsetsStr = getStrAttr( m_ctx, EA_XMPINDEX_OFFSETS, "" );
        if( offsetsStr.empty() )
            return;
        stringlist_t sl = Util::parseCommaSeperatedList( offsetsStr );

        LG_RDF_D << "loadXMPFromIndex() offsets:" << offsetsStr << endl;
    
        if( sl.size() % 2 != 0 )
        {
        }

        if( sl.empty() )
            return;
        
        fh_istream iss = m_ctx->getIStream( ios::in );
    
        for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
        {
            streamsize packet_start = toType< streamsize >( *si );
            ++si;
            streamsize packet_end   = toType< streamsize >( *si );

            LG_RDF_D << " packet_start:" << packet_start
                     << " packet_end:" << packet_end
                     << endl;
        
            iss.seekg( packet_start );
            fh_stringstream ss;
            copyTo( iss, packet_start, packet_end, ss );
            m_model->MergeRDFXML( tostr( ss ), base_uri );
        }
    }

    void
    FerrisXMPHandler::reindexXMP()
    {
        stringlist_t rdfOffsets;
    
        fh_uri base_uri = new URI( m_ctx->getURL() );
        m_model = Model::MemoryOnlyModel();

//  fh_istream iss = m_ctx->getIStream( ferris_ios::o_mmap | ferris_ios::o_mseq | ios::in );
        fh_istream iss = m_ctx->getIStream( ios::in );

        while( findString( iss, packet_header_utf8 ))
        {
            LG_RDF_D << "findString() has found the start of a packet at:" << iss.tellg() << endl;
        
            streamsize packet_start = iss.tellg();
            packet_start -= packet_header_utf8_length;
            bool readOnly = moveToEndOfXMPPacket( iss, packet_start );
            streamsize packet_end = iss.tellg();
            rdfOffsets.push_back( tostr( packet_start ));
            rdfOffsets.push_back( tostr( packet_end ));
            LG_RDF_D << "end of that packet is at:" << iss.tellg() << endl;

            fh_stringstream ss;
            copyTo( iss, packet_start, iss.tellg(), ss );
            m_model->MergeRDFXML( tostr( ss ), base_uri );

//         fh_ostream oss = Factory::fcout();
//         copyTo( iss, packet_start, iss.tellg(), oss );
        }
        
        setStrAttr( m_ctx, EA_XMPINDEX_MTIME,   tostr( Time::getTime() ), true );
        if( rdfOffsets.empty() )
            setStrAttr( m_ctx, EA_XMPINDEX_OFFSETS, "", true );
        else
            setStrAttr( m_ctx,
                        EA_XMPINDEX_OFFSETS,
                        Util::createCommaSeperatedList( rdfOffsets ),
                        true );
    }


    void FerrisXMPHandler::sync()
    {
    }
    
    void
    FerrisXMPHandler::dumpRDF()
    {
        fh_ostream oss = Factory::fcout();
        m_model->write( oss );
    }

    fh_model
    FerrisXMPHandler::getModel()
    {
        return m_model;
    }



    bool
    FerrisXMPHandler::findString( fh_istream& iss, const string& s, const std::set<char>& terminalChars )
    {
        const char* b = s.c_str();
        const char* p = b;
        const char* e = p + s.length();
    
        for( char c = 0; iss >> noskipws >> c; )
        {
            if( terminalChars.count( c ) )
            {
                return false;
            }
        
            if( *p == c ) ++p;
            else            p = b;
        
            if( p == e )
                return true;
        }
        return false;
    }
    bool
    FerrisXMPHandler::findString( fh_istream& iss, const string& s )
    {
        const char* b = s.c_str();
        const char* p = b;
        const char* e = p + s.length();
    
        for( char c = 0; iss >> noskipws >> c; )
        {
            if( *p == c ) ++p;
            else            p = b;
        
            if( p == e )
                return true;
        }
        return false;
    }


    bool
    FerrisXMPHandler::moveToEndOfXMPPacket( fh_istream& iss, const streamsize start_of_packet )
    {
        bool readOnly = true;

        std::set<char> bytes_attribute_terminal_chars;
        bytes_attribute_terminal_chars.insert( '>' );
    
        if( findString( iss, "bytes=", bytes_attribute_terminal_chars ))
        {
            char c = 0;
            iss >> c;
            c = ' ';
        
            fh_stringstream byteCountSS;
            while( iss >> c )
            {
                if( c == '\'' || c == '"' )
                    break;
                byteCountSS << c;
            }
            string byteCountString = tostr( byteCountSS );
            if( byteCountString.empty() )
            {
                fh_stringstream ss;
                ss << "Looking for XMP in file:" << m_ctx->getURL() << endl
                   << "error: packet header bytes='' has incorrect format"
                   << " offset is at:" << iss.tellg() << endl;
                Throw_XMPPacketScanFailed( tostr(ss), GetImpl( m_ctx ) );
            }
            int packetSize = toint( byteCountString );
            LG_RDF_D << " packetSize:" << packetSize
                     << " byteCountSS:" << tostr( byteCountSS )
                     << endl;
            iss.seekg( start_of_packet + packetSize, ios::beg );
            LG_RDF_D << " stream pos:" << iss.tellg() << endl;
        
        }
        else
        {
            // have to find the end the slow way
            const char* trailer = "<?xpacket end=";
            streamsize search_start_offset = iss.tellg();

            LG_RDF_D << "looking for end marker the slow way because there is no bytes='' attribute"
                     << " start_of_search is at offset:" << search_start_offset
                     << endl;
        
            if( !findString( iss, trailer ) )
            {
                fh_stringstream ss;
                ss << "Looking for XMP in file:" << m_ctx->getURL() << endl
                   << "error: no packet trailer found!"
                   << " started looking from offset:" << search_start_offset
                   << endl;
                Throw_XMPPacketScanFailed( tostr(ss), GetImpl( m_ctx ) );
            }
            search_start_offset = iss.tellg();
            if( !findString( iss, "?>" ) )
            {
                fh_stringstream ss;
                ss << "Looking for XMP in file:" << m_ctx->getURL() << endl
                   << "error: no packet trailer termination found!"
                   << " started looking from offset:" << search_start_offset
                   << endl;
                Throw_XMPPacketScanFailed( tostr(ss), GetImpl( m_ctx ) );
            }
        }
    
        // validate that ret seems correct
        //    <?xpacket end='r'?>
        {
            streamsize endOfPacket = iss.tellg();
        
            char c = 0;
            while( c != '<' )
            {
                iss.seekg( -2, ios::cur );
                iss >> noskipws >> c;
                if( !iss )
                {
                    fh_stringstream ss;
                    ss << "Looking for XMP in file:" << m_ctx->getURL() << endl
                       << "error: packet trailer has incorrect format"
                       << " trailer offset is at:" << iss.tellg()
                       << endl;                        
                    Throw_XMPPacketScanFailed( tostr(ss), GetImpl( m_ctx ) );
                }
            }

            streamsize startOfTrailerOffset = iss.tellg();
            const char* trailer = "?xpacket end=";
            if( !findString( iss, trailer ))
            {
                fh_stringstream ss;
                ss << "Looking for XMP in file:" << m_ctx->getURL() << endl
                   << "error: packet trailer has incorrect internal format"
                   << " trailer offset is at:" << startOfTrailerOffset
                   << endl;
                Throw_XMPPacketScanFailed( tostr(ss), GetImpl( m_ctx ) );
            }
            
            iss >> c;
            iss >> c;
            if( c == 'w' )
                readOnly = false;

            iss.seekg( endOfPacket );
        }

        return readOnly;
    }


    void
    FerrisXMPHandler::copyTo( fh_istream& iss_raw,
                              streamsize begin, streamsize end,
                              fh_ostream& oss )
    {
        LG_RDF_D << "copyTo() begin:" << begin
                 << " end:" << end
                 << endl;
    
        streamsize final_tellg = iss_raw.tellg();
    
        fh_istream iss = Factory::MakeLimitingIStream( iss_raw, begin, end );

        copy( istreambuf_iterator<char>(iss),
              istreambuf_iterator<char>(),
              ostreambuf_iterator<char>(oss));
        oss << flush;
    
        iss_raw.seekg( final_tellg );
    }

/************************************************************/
/************************************************************/
/************************************************************/

    
    class FERRISEXP_DLLLOCAL EAGenerator_XMP
        :
        public MatchedEAGeneratorFactory
    {
    protected:

        virtual void Brew( const fh_context& a );

        void generateAttribute( const fh_context& a,
                                const std::string& rdn,
                                stringlist_t& eanlist,
                                bool forceCreate = false );
        
    public:

        EAGenerator_XMP();

        fh_attribute CreateAttr(
            const fh_context& a,
            const string& rdn,
            fh_context md = 0 )
            throw(
                FerrisCreateAttributeFailed,
                FerrisCreateAttributeNotSupported
                );
        
        virtual bool isDynamic()
            {
                return false;
            }
    
        bool
        supportsCreateForContext( fh_context c )
            {
                return false;
            }

        RDFCore::fh_model getModel( Context* c );


        stringset_t& getDateAttributes();
        
        virtual void augmentRecommendedEA( const fh_context& a, fh_stringstream& ss )
        {
            stringset_t& dateAttributes = getDateAttributes();
            for( stringset_t::const_iterator ci = dateAttributes.begin();
                 ci != dateAttributes.end(); ++ci )
            {
                string rdn = *ci;
                
                if( a->isAttributeBound( rdn, false ))
                    ss << "," << rdn;
            }
        }
        
    };

    class FERRISEXP_DLLLOCAL XMPRedlandByteArrayAttribute
        :
        public RedlandByteArrayAttribute
    {
        EAGenerator_XMP* m_xmp;
        
    protected:
        virtual RDFCore::fh_model getModel( Context* c )
            {
                return m_xmp->getModel( c );
            }
        std::string getPredicateURI( const std::string& rdn )
            {
                return rdn;
            }
    public:
        XMPRedlandByteArrayAttribute(
            EAGenerator_XMP* m_xmp,
            const fh_context& parent,
            const string& rdn,
            const string& uri,
            bool forceCreate = false
            )
            :
            m_xmp( m_xmp ),
            RedlandByteArrayAttribute( parent, rdn, uri, forceCreate )
            {
            }
        virtual ~XMPRedlandByteArrayAttribute()
            {
            }
        static XMPRedlandByteArrayAttribute* Create( EAGenerator_XMP* m_xmp,
                                                     const fh_context& parent,
                                                     const string& rdn,
                                                     bool forceCreate = false )
            {
                return new XMPRedlandByteArrayAttribute( m_xmp,
                                                         parent,
                                                         rdn,
                                                         rdn,
                                                         forceCreate );
            }
        virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_iostream ret;
                
                try
                {
                    fh_model   m = getModel( c );
                    fh_node subj = Node::CreateURI( c->getURL() );
                    fh_node pred = Node::CreateURI( getPredicateURI( rdn ) );
                    
                    ret = priv_getStream( c, rdn, m, subj, pred, atom );
                }
                catch( exception& e )
                {
                    LG_RDF_D << "XMP::getStream() c:" << c->getURL()
                             << " rdn:" << rdn
                             << " e:" << e.what()
                             << endl;
                    throw;
                }
                
                return ret;
            }
        virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream  )
            {
                fh_stringstream ss;
                ss << "Writing EA which are to be embedded in XMP metadata is not"
                   << "supported at current. Please consider sending patches or donations."
                   << " context:" << c->getURL()
                   << " rdn:" << rdn
                   << endl;
                Throw_getIOStreamCloseUpdateFailed( tostr(ss), 0 );
            }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    EAGenerator_XMP::EAGenerator_XMP()
        :
        MatchedEAGeneratorFactory()
    {
    }

    RDFCore::fh_model
    EAGenerator_XMP::getModel( Context* c )
    {
        static Cache< Context*, fh_xmpHandler > cache;
        cache.setTimerInterval( 3000 );
        
        if( fh_xmpHandler d = cache.get( c ) )
        {
            return d->getModel();
        }

        try
        {
            fh_xmpHandler d = new FerrisXMPHandler( c );
            cache.put( c, d );
            return d->getModel();
        }
        catch( exception& e )
        {
            // There is no RDF model for this context.
            fh_model model = Model::MemoryOnlyModel();
            return model;
        }
    }

    static const string AS_TIMET_POSTFIX = "-as-timet";
    
    static fh_stringstream getTimeT( Context* c, const std::string& rdntt, EA_Atom* )
    {
        try
        {
            string rdn = getBaseEAName( AS_TIMET_POSTFIX, c, rdntt );
            string dateval = getStrAttr( c, rdn, "" );
            
            fh_stringstream ret;
            struct tm tm = Time::ParseTimeString( dateval );
            time_t tt = mktime( &tm );
            ret << tt;
//             cerr << "getTimeT() c:" << c->getURL() << " rdntt:" << rdntt << endl;
//             cerr << "   dateval:" << dateval << " tt:" << tt << endl;
            return ret;
        }
        catch( exception& e )
        {
            cerr << "getTimeT(e) c:" << c->getURL()
                 << " e:" << e.what()
                 << endl;
            throw;
        }
        
    }


    stringset_t&
    EAGenerator_XMP::getDateAttributes()
    {
        static stringset_t dateAttributes;
        if( dateAttributes.empty() )
        {
            dateAttributes.insert( "http://ns.adobe.com/pdf/1.3/CreationDate" );
            dateAttributes.insert( "http://ns.adobe.com/pdf/1.3/ModDate" );
            dateAttributes.insert( "http://ns.adobe.com/xap/1.0/CreateDate" );
            dateAttributes.insert( "http://ns.adobe.com/xap/1.0/ModifyDate" );
            dateAttributes.insert( "http://ns.adobe.com/xap/1.0/MetadataDate" );
        }
        return dateAttributes;
    }
    
    void
    EAGenerator_XMP::generateAttribute( const fh_context& a,
                                        const std::string& rdn,
                                        stringlist_t& eanlist,
                                        bool forceCreate )
    {
        XMPRedlandByteArrayAttribute* rba = XMPRedlandByteArrayAttribute::Create(
            this, a, rdn, forceCreate );
        
        bool rc = a->addAttribute( rdn,
                                   (EA_Atom*)rba,
                                   FXD_BINARY_NATIVE_EA );
        eanlist.push_back( rdn );

        LG_RDF_D << "generateAttribute() rdn:" << rdn
                 << " rc:" << rc
                 << endl;
        LG_RDF_D << "generateAttribute() rdn:" << rdn
                 << " read value:" << getStrAttr( a, rdn, "none" )
                 << endl;

        stringset_t& dateAttributes = getDateAttributes();

        // Add some translation for known dates to time_t values.
        if( dateAttributes.count( rdn ) )
        {
            string rdntt = rdn + AS_TIMET_POSTFIX;
            
            a->addAttribute( rdntt,
                             EA_Atom_ReadOnly::GetIStream_Func_t( getTimeT ),
                             FXD_UNIXEPOCH_T );
            eanlist.push_back( rdntt );
            a->supplementStateLessAttributes_timet( rdntt );
        }
        

        
        //
        // Attach schema
        //
    }

    void
    EAGenerator_XMP::Brew( const fh_context& a )
    {
        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );
    
        try
        {
//            a->setHasDynamicAttributes( true );
            stringlist_t xmp_ea_names;

            string              earl = a->getURL();
            RDFCore::fh_model          m = getModel( GetImpl( a ) );
            RDFCore::NodeIterator iter = m->getArcsOut( Node::CreateURI( earl ) );
            for( ; iter != NodeIterator(); ++iter )
            {
                LG_RDF_D << "Found another arc out for earl:" << earl << endl;
                string rdn = (*iter)->getURI()->toString();
            
                generateAttribute( a, rdn, xmp_ea_names );
            }
        
            /*
             * Add a new EA showing the names of all the RDF EA
             */
            a->addAttribute( "xmp-ea-names",
                             Util::createCommaSeperatedList( xmp_ea_names ),
                             FXD_EANAMES,
                             true );
        }
        catch( exception& e )
        {
            LG_RDF_D << "Failed to load XMP EA, error:" << e.what() << endl;
        }
    }
    
    fh_attribute
    EAGenerator_XMP::CreateAttr( const fh_context& a,
                                 const string& rdn,
                                 fh_context md )
        throw(
            FerrisCreateAttributeFailed,
            FerrisCreateAttributeNotSupported
            )
    {
        fh_stringstream ss;
        ss << "Creating EA which are to be embedded in XMP metadata is not"
           << "supported at current. Please consider sending patches or donations."
           << " context:" << a->getURL()
           << " rdn:" << rdn
           << endl;
        Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            return new EAGenerator_XMP();
        }
    };
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
//     class FERRISEXP_DLLLOCAL CachedXMPData
//         :
//         public CacheHandlable
//     {
        
//         MetaXAP* m_data;
        
//     public:

//         CachedXMPData( MetaXAP* d )
//             :
//             m_data( d )
//             {
//             }
        
//         virtual ~CachedXMPData()
//             {
//                 if( m_data )
//                     delete m_data;
//             }

//         virtual void sync()
//             {
//             }

// //         typedef MetaXAP* DataPtr;
// //         operator DataPtr()
// //             {
// //                 return m_data;
// //             }
        
//     };
//     FERRIS_SMARTPTR( CachedXMPData, fh_cachedXMPData );

//     static fh_cachedXMPData getXMPData( const fh_context& ctx )
//     {
//         static Cache< Context*, fh_cachedXMPData > cache;
//         cache.setTimerInterval( 3000 );
        
//         Context* c = GetImpl(ctx);
        
//         if( fh_cachedXMPData d = cache.get( c ) )
//         {
//             return d;
//         }

//         PacketScanner* scanner = 0;
//         string contents = getStrAttr( ctx, "contents", "" );
        
//         if( contents.empty() )
//             return 0;

//         streamsize sz = contents.size();
// 		scanner = new PacketScanner ( sz );
// 		scanner->Scan ( contents.data(), 0, sz );
// 		int snipCount = scanner->GetSnipCount();
//         if ( snipCount < 2 )
//         {
//             return 0;
//         }

//         fh_uri base_uri    = new URI( ctx->getURL() );
//         fh_storage storage = Storage::CreateStorage( "memory", "", "" );
//         fh_model   model   = storage->CreateModel();
        
//         PacketScanner::SnipInfoVector* snips = new PacketScanner::SnipInfoVector( snipCount );
//         for ( int s = 0; s < snipCount; s++ )
//         {
//             if ( (*snips)[s].fState != PacketScanner::eValidPacketSnip )
//                 continue;

//             model->MergeRDFXML( contents.substr(
//                                     (*snips)[s].fOffset, (*snips)[s].fLength ),
//                                 base_uri );
//         }
        
//         MetaXAP* rawd = FIXME;
//         if( !rawd )
//         {
//             cerr << "WARNING: getXMPData() no data for ctx:" << ctx->getURL() << endl;
//             return 0;
//         }
        
//         fh_cachedXMPData d = new CachedXMPData( rawd );
//         cache.put( c, d );
//         return d;
//     }

//     static void
//     updateEntry( fh_context c, fh_cachedXMPData edholder, ExifEntry* entry, fh_istream iss )
//     {
//         MetaXAP* ed = *edholder;

//         fh_stringstream ss;
//         ss << "Update support not implemented yet." << endl;
//         Throw_getIOStreamCloseUpdateFailed( tostr(ss), GetImpl(c) );
//     }

//     static void
//     updateEntry( fh_context c, fh_cachedXMPData edholder, ExifEntry* entry, const std::string& s )
//     {
//         fh_stringstream ss;
//         ss << s;
//         updateEntry( c, edholder, entry, ss );
//     }
    
//     static void
//     SaveXMP( fh_context c, fh_cachedXMPData edholder )
//     {
//         MetaXAP* ed = *edholder;
        
//         {
//             fh_stringstream ss;
//             ss << "write support for XMP not implemented yet!"
//                << " file untouched at:" << c->getURL() << endl;
//             Throw_FerrisImageSaveFailed( tostr(ss), GetImpl(c) );
//         }
//     }
    
    

    
// /********************************************************************************/
// /********************************************************************************/
// /********************************************************************************/
// /********************************************************************************/
// /********************************************************************************/
// /********************************************************************************/
    
//     class FERRISEXP_DLLLOCAL EAGenerator_XMP : public MatchedEAGeneratorFactory
//     {
//         typedef EAGenerator_XMP _Self;
        
//     protected:

//         virtual void Brew( const fh_context& a );

//     public:

//         EAGenerator_XMP();
//         virtual ~EAGenerator_XMP();

//         fh_attribute CreateAttr(
//             const fh_context& a,
//             const string& rdn,
//             fh_context md = 0 )
//             throw(
//                 FerrisCreateAttributeFailed,
//                 FerrisCreateAttributeNotSupported
//                 );

//         virtual bool isDynamic()
//             {
//                 return true;
//             }
    
    
//         bool
//         supportsCreateForContext( fh_context c )
//             {
//                 return true;
//             }

//         void generateAttribute( ExifEntry* entry,
//                                 const fh_context& a,
//                                 const std::string& rdn,
//                                 bool forceCreate = false );

//         /********************************************************************************/
//         /********************************************************************************/

//     };


//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/


    
//     class FERRISEXP_DLLLOCAL ExifByteArrayAttribute
//         :
//         public EA_Atom_ReadWrite
//     {

//     protected:

//         ferris_ios::openmode getSupportedOpenModes()
//             {
//                 return ios::in | ios::out | ios::trunc | ios::binary | ios::ate;
//             }

//         void createOnDiskAttribute( const fh_context& parent, const std::string& uri )
//             {
// //                 string v = "";

// //                 RDF::fh_model  m = getDefaultFerrisModel();

// //                 try
// //                 {
// //                     m->insert( Node::CreateURI( parent->getURL() ),
// //                                Node::CreateURI( uri ),
// //                                Node::CreateLiteral( v ) );
// //                 }
// //                 catch( exception& e )
// //                 {
// //                     fh_stringstream ss;
// //                     ss << "Failed to create RDF EA for c:" << parent->getURL()
// //                        << " attribute name:" << uri
// //                        << endl;
// //                     ThrowFromErrno( 0, tostr(ss), 0 );
// //                 }
//             }
    
    
//     public:

//         virtual fh_iostream getStream( Context* c, const std::string& rdn, EA_Atom* atom )
//             {
//                 fh_stringstream ss;

//                 string    tagstr = stripPrefix( rdn );
//                 fh_cachedXMPData ed = getXMPData( c );

//                 ExifEntry* entry = findTagIfExists( ed, tagstr );
//                 if( entry )
//                 {
//                     ss << exif_entry_get_value( entry );
//                 }
//                 else
//                 {
//                     fh_stringstream ss;
//                     ss << "reading attribute:" << rdn;
//                     Throw_NoSuchAttribute( tostr(ss), c );
//                 }
//                 return ss;
//             }

//         virtual void setStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
//             {
//                 fh_cachedXMPData ed = getXMPData( c );
//                 string     tagstr = stripPrefix( rdn );
//                 ExifEntry*  entry = findTagIfExists( ed, tagstr );
//                 updateEntry( c, ed, entry, ss );
                
//  				SaveXMP( c, ed );
//             }

    
    
//         ExifByteArrayAttribute(
//             const fh_context& parent,
//             const string& rdn,
//             const string& uri,
//             bool forceCreate = false
//             )
//             :
//             EA_Atom_ReadWrite( this, &ExifByteArrayAttribute::getStream,
//                                this, &ExifByteArrayAttribute::getStream,
//                                this, &ExifByteArrayAttribute::setStream )
//             {
//                 if( forceCreate )
//                 {
//                     cerr << "ExifByteArrayAttribute() making EA for earl:" << parent->getURL()
//                          << " predicate::" << uri
//                          << endl;
//                     createOnDiskAttribute( parent, uri );
//                 }
//             }

//         ~ExifByteArrayAttribute()
//             {
//             }

//         static ExifByteArrayAttribute* Create( const fh_context& parent,
//                                                const string& rdn,
//                                                bool forceCreate = false )
//             {
//                 return new ExifByteArrayAttribute( parent,
//                                                    rdn,
//                                                    getPredicateURI( rdn ),
//                                                    forceCreate );
//             }
//     };


//     static fh_stringstream getSchema( Context* c, const std::string& rdn, EA_Atom* )
//     {
//         fh_cachedXMPData   edholder = getXMPData( c );
//         MetaXAP*                 ed = *edholder;
        
//         string          tagstr = stripPrefix( rdn );
// 		ExifTag            tag = exif_tag_from_string( tagstr.c_str() );

//         Factory::xsdtypemap_t tmap;
//         Factory::makeBasicTypeMap( tmap );
//         fh_context schema = tmap[ XSD_UNKNOWN ];
        
//         for( int ifd = EXIF_IFD_0; ifd < EXIF_IFD_COUNT; ++ifd )
//         {
//             ExifEntry* e = exif_content_get_entry (ed->ifd[ifd], tag);
//             if (e)
//             {

//                 switch (e->format)
//                 {
//                 case EXIF_FORMAT_LONG:
//                 case EXIF_FORMAT_SLONG:
//                     schema = tmap[ FXD_INT32 ];
//                     break;
                    
//                 case EXIF_FORMAT_SHORT:
//                     schema = tmap[ XSD_BASIC_INT ];
//                     break;

//                 case EXIF_FORMAT_RATIONAL:
//                 case EXIF_FORMAT_SRATIONAL:
//                     schema = tmap[ XSD_BASIC_FLOAT ];
//                     break;

//                 case EXIF_FORMAT_BYTE:
//                 case EXIF_FORMAT_UNDEFINED:
//                     schema = tmap[ XSD_UNKNOWN ];
//                     break;

//                 case EXIF_FORMAT_ASCII:
//                     schema = tmap[ XSD_BASIC_STRING ];
//                     break;
//                 }
//             }
//         }
        
//         fh_stringstream ss;
//         ss << schema->getURL();
//         return ss;
//     }
    
    

//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/

//     EAGenerator_XMP::EAGenerator_XMP()
//     :
//         MatchedEAGeneratorFactory()
// #ifdef HAVE_EPEG
//         ,m_thumbW( 0 ),
//         m_thumbH( 0 ),
//         m_nothumb( false )
// #endif
//     {
//     }

//     EAGenerator_XMP::~EAGenerator_XMP()
//     {
//     }
    
    
//     void
//     EAGenerator_XMP::generateAttribute( ExifEntry* entry,
//                                          const fh_context& a,
//                                          const std::string& rdn,
//                                          bool forceCreate )
//     {
//         cerr << "EAGenerator_XMP::generateAttribute() ctx:" << a->getURL()
//              << " entry:" << toVoid( entry )
//              << " rdn:" << rdn
//              << endl;

//         if( a->isAttributeBound( rdn, false ))
//         {
//             cerr << "EAGenerator_XMP::generateAttribute(already bound) ctx:" << a->getURL()
//                  << " entry:" << toVoid( entry )
//                  << " rdn:" << rdn
//                  << endl;
//             return;
//         }

//         XSDBasic_t sct = FXD_BINARY_NATIVE_EA;

//         if( entry )
//         {
//             switch (entry->tag) {
//             case EXIF_TAG_EXIF_VERSION:
//             case EXIF_TAG_FLASH_PIX_VERSION:
//                 sct = FXD_EXIF_VERSION;
//                 break;
                
//             case EXIF_TAG_USER_COMMENT:
//                 sct = FXD_EXIF_USER_COMMENT;
//                 break;
//             case EXIF_TAG_COPYRIGHT:
//                 sct = FXD_EXIF_COPYRIGHT;
//                 break;
//             case EXIF_TAG_FLASH:
//                 sct = FXD_EXIF_FLASH;
//                 break;
//             case EXIF_TAG_EXPOSURE_PROGRAM:
//                 sct = FXD_EXIF_EXPOSURE_PROGRAM;
//                 break;
//             case EXIF_TAG_SENSING_METHOD: 
//                 sct = FXD_EXIF_SENSING_METHOD;
//                 break;
//             case EXIF_TAG_ORIENTATION: 
//                 sct = FXD_EXIF_ORIENTATION;
//                 break;
//             case EXIF_TAG_METERING_MODE: 
//                 sct = FXD_EXIF_METERING_MODE;
//                 break;
//             case EXIF_TAG_YCBCR_POSITIONING:
//                 sct = FXD_EXIF_YCBCR_POSITIONING;
//                 break;
//             case EXIF_TAG_COMPRESSION: 
//                 sct = FXD_EXIF_COMPRESSION;
//                 break;
//             case EXIF_TAG_LIGHT_SOURCE:
//                 sct = FXD_EXIF_LIGHT_SOURCE;
//                 break;
//             case EXIF_TAG_RESOLUTION_UNIT:
//             case EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT:
//                 sct = FXD_UNIT_NAME_LENGTH;
//                 break;
//             case EXIF_TAG_X_RESOLUTION:
//             case EXIF_TAG_Y_RESOLUTION:
//             case EXIF_TAG_FOCAL_PLANE_X_RESOLUTION:
//             case EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION:
//                 sct = FXD_BINARY_NATIVE_EA;
//                 break;
//             case EXIF_TAG_MAKE:
//             case EXIF_TAG_MODEL:
//             case EXIF_TAG_IMAGE_DESCRIPTION:
//             case EXIF_TAG_SOFTWARE:
//             case EXIF_TAG_ARTIST:
//                 sct = XSD_BASIC_STRING;
//                 break;
//             case EXIF_TAG_DATE_TIME:
//             case EXIF_TAG_DATE_TIME_ORIGINAL:
//             case EXIF_TAG_DATE_TIME_DIGITIZED:
//                 sct = FXD_EXIF_DATETIME_STRING;
//                 break;
//             default:
//                 switch (entry->format) {
//                 case EXIF_FORMAT_RATIONAL:
//                 case EXIF_FORMAT_SRATIONAL:
//                     sct = XSD_BASIC_DOUBLE;
//                     break;
//                 case EXIF_FORMAT_BYTE:
//                 case EXIF_FORMAT_SHORT:
//                 case EXIF_FORMAT_LONG:
//                 case EXIF_FORMAT_SLONG:
//                     sct = XSD_BASIC_INT;
//                     break;
//                 default:
//                     sct = FXD_BINARY_NATIVE_EA;
//                     break;
//                 }
//                 break;
//             }
//         }
        
//         a->addAttribute( rdn,
//                          (EA_Atom*)ExifByteArrayAttribute::Create( this, a, rdn, forceCreate ),
//                          sct
//             );

//         //
//         // Attach schema
//         //
//         if( !starts_with( rdn, "schema:" ))
//         {
//             string schema_name      = "schema:" + rdn;

//             if( ! a->addAttribute(
//                     schema_name,
//                     EA_Atom_ReadOnly::GetIStream_Func_t( getSchema ),
//                     XSD_SCHEMA ))
//             {
//                 ostringstream ss;
//                 ss << "Can't create schema attribute for rdn:" << rdn
//                    << " schema_name:" << schema_name
//                    << " forceCreate:" << forceCreate;
//                 cerr << tostr(ss) << endl;
//                 Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
//             }
//         }
//     }

// /********************************************************************************/
// /********************************************************************************/

//     static void generateAttributeForAll( MetaXAP* ed,
//                                          EAGenerator_XMP* selfp,
//                                          fh_context ctx,
//                                          stringlist_t& xmp_ea_names )
//     {

//         for( unsigned int edi = 0; edi < EXIF_IFD_COUNT; edi++)
//         {
//             ExifContent *content = ed->ifd[ edi ];

//             for( unsigned int ci = 0; ci < content->count; ci++)
//             {
//                 ExifEntry* entry = content->entries[ci];

//                 string rdn = "exif:";
//                 rdn += Util::convertCapsToDashLower( exif_tag_get_name( entry->tag ) );

//                 exif->generateAttribute( entry, ctx, rdn );
//                 xmp_ea_names.push_back( rdn );
//             }
//         }
//     }

// /********************************************************************************/
// /********************************************************************************/

//     void
//     EAGenerator_XMP::Brew( const fh_context& a )
//     {
//         static bool brewing = false;
//         if( brewing )
//             return;
//         Util::ValueRestorer< bool > dummy1( brewing, true );
    
//         try
//         {
//             cerr << "EAGenerator_XMP::Brew() ctx:" << a->getURL() << endl;

            
// //            a->setHasDynamicAttributes( true );
//             stringlist_t xmp_ea_names;

//             if( fh_cachedXMPData edholder = getXMPData( a ) )
//             {
//                 MetaXAP* ed = *edholder;
//                 generateAttributeForAll( ed, this, a, xmp_ea_names );

//                 xmp_ea_names.sort();
            
//                 /*
//                  * Add a new EA showing the names of all the exif EA
//                  */
//                 a->addAttribute( "xmp-ea-names",
//                                  Util::createCommaSeperatedList( xmp_ea_names ),
//                                  FXD_EANAMES,
//                                  true );
//             }
//         }
//         catch( exception& e )
//         {
//             LG_EXIF_ER << "Failed to load xfs EA, error:" << e.what() << endl;
//         }
//     }

//     fh_attribute
//     EAGenerator_XMP::CreateAttr( const fh_context& a,
//                                   const string& rdn,
//                                   fh_context md )
//         throw(
//             FerrisCreateAttributeFailed,
//             FerrisCreateAttributeNotSupported
//             )
//     {
//         fh_stringstream ss;
//         ss << "Creation of attributes in XMP metadata chunks is currently\n"
//            << " not implemented. Please donate to the witme project on sf.net\n"
//            << " to show your support." << endl;
//         Throw_FerrisCreateAttributeFailed( tostr(ss), 0 );
//     }


//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/

//     extern "C"
//     {
//         FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
//         {
//             return new EAGenerator_XMP();
//         }
//     };



 
};
