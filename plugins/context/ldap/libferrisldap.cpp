/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris ldap
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

    $Id: libferrisldap.cpp,v 1.4 2010/09/24 21:31:39 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

#include <FerrisContextPlugin.hh>
#include <Trimming.hh>
#include <ValueRestorer.hh>
#include <Configuration_private.hh>
#include <libferrisldapshared.hh>
#include <FerrisDOM.hh>

#include <ldap.h>
#include <ldap_schema.h>
#include <errno.h>


using namespace std;
namespace Ferris
{
    using namespace LDAPAuth;
    
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct ConnectionDetails 
    {
        string serv;
        int port;
        string path;

        ConnectionDetails( string serv="localhost", int port=LDAP_PORT, string path="/" )
            :
            serv( serv ),
            port( port ),
            path( path )
            {
            }

        void reset()
            {
                serv=""; path = "";
                port = LDAP_PORT;
            }

        string getServPort()
            {
                fh_stringstream ss;
                ss << serv << ":" << port;
                return tostr(ss);
            }
    };
    
    

    /*
     * /ldap/localhost:3445/pub/somewhere/anotherdir
     *  |        |       |   |-------+------|
     *  x      serv    port       path
     */
    ConnectionDetails getConnectionDetails( fh_context c )
    {
        static ConnectionDetails d;
        typedef list< fh_context > flattree_t;
        flattree_t ftree;

        d.reset();

        fh_context pc = c;
        while( pc->isParentBound() )
        {
            ftree.push_front( pc );
            pc = pc->getParent();
        }

        flattree_t::iterator iter = ftree.begin();
        
        if( iter != ftree.end() )
        {
            d.serv = (*iter)->getDirName();

            d.path = "/";
            while( ++iter != ftree.end() )
            {
                d.path += "/";
                d.path += (*iter)->getDirName();
            }
        }
        else
        {
            fh_stringstream ss;
            ss << "Can not determine the server name for context:"
               << c->getDirPath();
            Throw_FerrisCurlServerNameNotFound( tostr(ss), 0 );
        }
        
        LG_LDAP_D << " c:" << c->getDirPath()
                  << " serv:" << d.serv
                  << " path:" << d.path
                  << endl;
        
        return d;
    }

    
    LDAP* getLDAP( fh_context c )
    {
        typedef map< string, LDAP* > ldap_connections_t;
        ldap_connections_t cons;
        LDAP* ret = 0;
        
        ConnectionDetails d = getConnectionDetails( c );
        ldap_connections_t::iterator iter = cons.find( d.getServPort() );
        if( iter != cons.end() )
            return iter->second;

        LG_LDAP_D << "getLDAP() setting up connection serv:" << d.serv
                  << " port:" << d.port
                  << endl;
        ret = ldap_init( d.serv.c_str(), d.port );

        LG_LDAP_D << "getLDAP() setting up connection ret:" << toVoid(ret) << endl;
        if( !ret )
        {
            string es = errnum_to_string( "", errno );
            fh_stringstream ss;
            ss << "Cant open connection to server:" << d.serv
               << " port:" << d.port
               << " reason:" << es
               << endl;
            cerr << tostr(ss) << endl;
            Throw_LDAPException( tostr(ss), GetImpl(c) );
        }

        LDAPAuthInfo authd = getUserPass( d.serv );

        int rc = ldap_simple_bind_s( ret,
                                     authd.username.c_str(),
                                     authd.password.c_str() );
        if( rc != LDAP_SUCCESS )
        {
            char* es = ldap_err2string(rc);
            fh_stringstream ss;
            ss << "Cant bind connection to server:" << d.serv
               << " port:" << d.port
               << " reason:" << es
               << endl;
            cerr << tostr(ss) << endl;
            Throw_LDAPException( tostr(ss), GetImpl(c) );
        }
        
        cons[ d.getServPort() ] = ret;
        return ret;
    }

    string
    getA( LDAP* ld, LDAPMessage* m, const std::string& eaname )
    {
        string ret;
        
        struct berval** bv = ldap_get_values_len( ld, m, eaname.c_str() );
        if( bv && ldap_count_values_len(bv) )
        {
            fh_stringstream ss;
            ss.write( bv[0]->bv_val, bv[0]->bv_len );
            ret = tostr(ss);
        }

        if( bv ) 
            ldap_value_free_len( bv );
        
        return ret;
    }

    pair< string, string >
    getRdn( LDAP* ld, LDAPMessage* m )
    {
        if( !ld || !m )
            LG_LDAP_I << "getRdn() bad args! ld:" << toVoid(ld)
                      << " m:" << toVoid(m) << endl;

        pair< string, string > ret;
        
        char*  l_dn  = ldap_get_dn(ld, m);
        char** l_exp = ldap_explode_dn( l_dn, 0 );
        if( *l_exp )
        {
            string s  = l_exp[0];
            int    eq = s.find('=');

            ret.first  = s.substr( 0, eq   );
            ret.second = s.substr( eq+1 );
        }
        ldap_value_free( l_exp );

        ldap_memfree( l_dn );

        LG_LDAP_D << "getRdn() rdn.first:" << ret.first << " sec:" << ret.second << endl;

        return ret;
    }
    


    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_istream
    SL_getIsDirStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->hasSubContexts();
        return ss;
    }


    fh_istream
    SL_getHasSubContextsGuessStream( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << c->hasSubContexts();
        return ss;
    }
    
    class FERRISEXP_CTXPLUGIN ldapContext
        :
        public StateLessEAHolding_Recommending_ParentPointingTree_Context< ldapContext >
    {
        typedef ldapContext                                _Self;
        typedef StateLessEAHolding_Recommending_ParentPointingTree_Context< ldapContext> _Base;

        /******************************************************************************/
        /******************************************************************************/
        /******************************************************************************/

        /**
         * get the context that is the server for this connection
         * eg for /localhost/com/mydomain/me
         * would get /localhost context
         */
        ldapContext* getServerContext()
            {
                Context* c = this;
                Context* cache = c;
                
                for( ; c->isParentBound(); c = c->getParent()  )
                {
                    cache = c;
                }

                ldapContext* ret = dynamic_cast<ldapContext*>(cache);
                LG_LDAP_D << "getServerContext() this:" << getURL()
                          << " ret:" << cache->getURL()
                          << " dyncast:" << toVoid( ret )
                          << endl;
                
                return ret;
            }

        LDAP* getLDAP()
            {
                fh_context p = getServerContext();
                return ::Ferris::getLDAP( p );
            }

        
        /**
         * The operational attributes for the server
         */
        struct LDAPServerData
        {
            string rootdn;
            map< string, string > ea;
        };

        /**
         * get all the operational attributes for the LDAP server itself.
         */
        LDAPServerData getLDAPServerData()
            {
                ldapContext* serv = getServerContext();
                LDAPServerData d;
                typedef map< fh_context, LDAPServerData > cache_t;
                cache_t cache;

                cache_t::iterator ci = cache.find( serv );
                if( ci != cache.end() )
                    return ci->second;

                LDAPMessage* result = 0;
                LDAP* ld = getLDAP();

                char* attrs[ 2 ] = { "", 0 };
                attrs[0] = (char*)LDAP_ALL_OPERATIONAL_ATTRIBUTES;
                    
                int rc = ldap_search_s( ld, "",
                                        LDAP_SCOPE_BASE,
                                        "(objectClass=*)",
                                        attrs, 0,
                                        &result );
                LG_LDAP_D << "ldapContext::getRootDN() rc:" << rc << endl;
                if( rc != LDAP_SUCCESS )
                {
                    char* es = ldap_err2string(rc);
                    if( result ) ldap_msgfree( result );
                    fh_stringstream ss;
                    ss << "Can not get base dn for URL:" << getURL()
                       << " e:" << es;
                    cerr << tostr(ss) << endl;
                    Throw_LDAPException( tostr(ss), this );
                }
                else
                {
                    LG_LDAP_D << "ldapContext::getRootDN() rc:" << rc << endl;
                    for( LDAPMessage* m = ldap_first_entry( ld, result );
                         m; m = ldap_next_entry( ld, m ) )
                    {
                        pair< string, string > rootdn = getRdn( ld, m );
                        LG_LDAP_D << "Found root dn f:" << rootdn.first << " s:" << rootdn.second << endl;
                            
                        BerElement* berptr = 0;
                        for( char* ea = ldap_first_attribute( ld, m, &berptr );
                             ea; ea = ldap_next_attribute( ld, m, berptr ) )
                        {
                            string k = ea;
                            string v = getA( ld, m, ea );

                            LG_LDAP_D << " base has ea:" << ea
                                      << " v:" << v
                                      << endl;

                            if( k == "namingContexts" )
                            {
                                LG_LDAP_D << "Found root DN:" << v << endl;
                                d.rootdn = v;
                            }
                            
                            d.ea[ k ] = v;
                        }
                        ber_free( berptr, 0 );
                    }
                }

                cache_t::iterator n = cache.insert( make_pair(serv, d) ).first;
                return n->second;
            }
                

        
        
        string getRootDN()
            {
                ldapContext* serv = getServerContext();
                if( !serv )
                {
                    fh_stringstream ss;
                    ss << "getRootDN() called on a non server context."
                       << " for URL:" << getURL();
                    cerr << tostr(ss) << endl;
                    Throw_LDAPException( tostr(ss), this );
                }
                
                LDAPAuthInfo d = getUserPass( serv->getDirName() );

                LG_LDAP_D << "getRootDN() url:" << getURL()
                          << " lookup:" << d.lookup_basedn
                          << " basedn:" << d.basedn
                          << endl;

                if( d.lookup_basedn )
                {
                    LDAPServerData d = getLDAPServerData();
                    LG_LDAP_D << "getRootDN() autoRootDN of:" << d.rootdn << endl;
                    return d.rootdn;
                }
                else
                {
                    return d.basedn;
                }
                
                
                    
//                     LDAPMessage* result = 0;
//                     LDAP* ld = getLDAP();

//                     char* attrs[ 2 ] = { "", 0 };
//                     attrs[0] = (char*)LDAP_ALL_OPERATIONAL_ATTRIBUTES;
                    
//                     int rc = ldap_search_s( ld, "",
//                                             LDAP_SCOPE_BASE,
//                                             "(objectClass=*)",
//                                             attrs, 0,
//                                             &result );
//                     LG_LDAP_D << "ldapContext::getRootDN() rc:" << rc << endl;
//                     if( rc != LDAP_SUCCESS )
//                     {
//                         char* es = ldap_err2string(rc);
//                         if( result ) ldap_msgfree( result );
//                         fh_stringstream ss;
//                         ss << "Can not get base dn for URL:" << getURL()
//                            << " e:" << es;
//                         cerr << tostr(ss) << endl;
// //                    Throw_LDAPException( tostr(ss), this );
//                     }
//                     else
//                     {
//                         LG_LDAP_D << "ldapContext::getRootDN() rc:" << rc << endl;
//                         for( LDAPMessage* m = ldap_first_entry( ld, result );
//                              m; m = ldap_next_entry( ld, m ) )
//                         {
//                             pair< string, string > rootdn = getRdn( ld, m );
//                             LG_LDAP_D << "Found root dn f:" << rootdn.first << " s:" << rootdn.second << endl;
                            
//                             BerElement* berptr = 0;
//                             for( char* ea = ldap_first_attribute( ld, m, &berptr );
//                                  ea; ea = ldap_next_attribute( ld, m, berptr ) )
//                             {
//                                 LG_LDAP_D << " base has ea:" << ea
//                                           << " v:" << getA( ld, m, ea )
//                                           << endl;
//                             }
//                             ber_free( berptr, 0 );
//                         }
                    
//                     }
//                 }
                
                
                return "dc=my-domain,dc=com";
            }

        bool callingXMLStreamingFunctions;
        pair< string, string > ldapRDN;

        /**
         * Each context needs to know what its dn = value RDN info is.
         */
        void setRdn( const pair< string, string >& p )
            {
                ldapRDN = p;
            }

        void getDN_rec( fh_stringstream ss, bool starting, string rootdn )
            {
//                cerr << "getDN_rec() url:" << getURL() << " ss:" << tostr(ss) << endl;
                
                if( !ldapRDN.first.empty() )
                {
                    if( !starting )
                        ss << ",";
                    ss << ldapRDN.first << "=" << ldapRDN.second;
                    starting = false;
                }

                if( isParentBound() )
                {
                    Context* p = getParent();
                    if( ldapContext* pp = dynamic_cast<ldapContext*>( p ))
                    {
                        pp->getDN_rec( ss, starting, rootdn );
                    }
                }
                else
                {
                    if( !starting )
                        ss << ",";
                    ss << rootdn;
                }
            }

        /**
         * Use the info from setRdn() calls and all parents to convert a path of
         * /server/where/ever
         * to the LDAP dn
         * cn=ever, dn=where, dc=mydomain, dc=com
         */
        string getDN()
            {
                string ret = getRootDN();
                LG_LDAP_D << "getDN(top) url:" << getURL() << " ret:" << ret << endl;
                
                ConnectionDetails d = getConnectionDetails( this );
                LG_LDAP_D << "getDN(2) url:" << getURL() << " d.path:" << d.path << endl;
                if( d.path == "/" )
                {
                    LG_LDAP_D << "getDN(3) url:" << getURL() << " d.path:" << d.path << endl;
                    ret = getRootDN();
                }
                else
                {
                    LG_LDAP_D << "getDN(4) url:" << getURL() << " d.path:" << d.path << endl;
                    fh_stringstream ss;
                    getDN_rec( ss, true, getRootDN() );
                    ret = tostr(ss);
                }
                
                LG_LDAP_D << "getDN(bottom) url:" << getURL() << " ret:" << ret << endl;
                return ret;
            }
        
        inline string getValue( const std::string& eaname )
            {
                string ret;
                LDAP* ld = getLDAP();
                
//                cerr << "getValue(top) url:" << getURL() << endl;

                try
                {
                    LDAPMessage* result = 0;

                    char* attrs[ 2 ] = { "", 0 };
                    attrs[0] = (char*)eaname.c_str();

//                     cerr << "getValue() url:" << getURL()
//                          << " dn:" << getDN()
//                          << " ea:" << eaname
//                          << endl;
                    
                    int rc = ldap_search_s( getLDAP(),
                                            getDN().c_str(),
                                            LDAP_SCOPE_BASE,
                                            0,
                                            attrs, 0,
                                            &result );
                    if( rc != LDAP_SUCCESS )
                    {
                        char* es = ldap_err2string(rc);
                        if( result ) ldap_msgfree( result );
                        fh_stringstream ss;
                        ss << "Can not get ea:" << eaname
                           << " for URL:" << getURL()
                           << " e:" << es;
                        cerr << tostr(ss) << endl;
                        Throw_LDAPException( tostr(ss), this );
                    }

                    for( LDAPMessage* m = ldap_first_entry( ld, result );
                         m && ret.empty(); m = ldap_next_entry( ld, m ) )
                    {
                        BerElement* berptr = 0;
                        for( char* ea = ldap_first_attribute( ld, m, &berptr );
                             ea; ea = ldap_next_attribute( ld, m, berptr ) )
                        {
                            if( eaname == ea )
                            {
                                ret = getA( ld, m, eaname );
                                break;
                            }
                        }
                        
                        ber_free( berptr, 0 );
                    }
                    
                    
                    if( result ) ldap_msgfree( result );
                    return ret;
                }
                catch( exception& e )
                {
                    fh_stringstream ss;
                    ss << "Can not get value for URL:" << getURL()
                       << " e:" << e.what();
                    Throw_LDAPException( tostr(ss), this );
                }
            }

        /**
         * perform a ldap_modify() with the given operation/values marshaling
         * an error to an exception.
         *
         * @param opcode is add/replace/delete
         * @param eaname is attribute to operate on
         * @param values are the new values for the operation
         */
        void ldapModify( int opcode, const string& eaname, stringlist_t values )
            {
                LDAP* ld = getLDAP();
                struct berval** bvals = (struct berval**)calloc( values.size()+1,
                                                                 sizeof(struct berval*));
                int i=0;
                struct berval* bval = bvals[0];
                for( stringlist_t::iterator si = values.begin(); si != values.end(); ++si, ++i )
                {
                    bvals[ i ] = (struct berval*)calloc( 1, sizeof(struct berval));
                    bvals[ i ]->bv_len = si->length();
                    bvals[ i ]->bv_val = (char*)si->data();
                }

                LDAPMod* mods[2];
                LDAPMod mod;
                mod.mod_op    = opcode | LDAP_MOD_BVALUES;
                mod.mod_type  = (char*)eaname.c_str();
                mod.mod_vals.modv_strvals = 0;
                mod.mod_vals.modv_bvals   = bvals;
                
                mods[0] = &mod;
                mods[1] = 0;
                
                int rc = ldap_modify_s( ld, getDN().c_str(), mods );

                for( int i=0; i<values.size(); ++i )
                    free( bvals[ i ] );
                free( bvals );
                
                if( rc != LDAP_SUCCESS )
                {
                    char* es = ldap_err2string(rc);
                    fh_stringstream ss;
                    ss << "Can't perform modification on:" << getURL()
                       << " eaname:" << eaname
                       << " reason:" << es
                       << endl;
                    Throw_LDAPException( tostr(ss), this );
                }
            }

        /**
         * same as ldapModify() but with only a single value
         */
        void ldapModify( int opcode, const string& eaname, string v )
            {
                stringlist_t listv;
                listv.push_back( v );
                ldapModify( opcode, eaname, listv );
            }

        void ldapAdd( const string& rdn )
            {
                LDAP* ld = getLDAP();
                LDAPMod *attrs[] = { 0 };
                
                int rc = ldap_add(ld, rdn.c_str(), attrs );
                
                if( rc != LDAP_SUCCESS )
                {
                    char* es = ldap_err2string(rc);
                    fh_stringstream ss;
                    ss << "Can't add new context on:" << getURL()
                       << " rdn:" << rdn
                       << " reason:" << es
                       << endl;
                    Throw_LDAPException( tostr(ss), this );
                }
                
            }
        
        
        inline void setValue( const std::string& k, const std::string& v )
            {
                ldapModify( LDAP_MOD_REPLACE, k, v );
            }
    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

        friend fh_context SL_ldap_SubCreate_context( fh_context c, fh_context md );
        friend fh_context SL_ldap_SubCreate_ea(      fh_context c, fh_context md );
        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
    
        ldapContext* priv_CreateContext( Context* parent, string rdn )
            {
                ldapContext* ret = new ldapContext();
                ret->setContext( parent, rdn );
                return ret;
            }

protected:


        void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m );

        virtual fh_stringstream real_getIOStream()
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   exception);
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   std::exception);
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   AttributeNotWritable,
                   CanNotGetStream,
                   std::exception);
        
        void
        createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute
                    SLEA( "is-dir",                SL_getIsDirStream, XSD_BASIC_BOOL );
                    SLEA( "has-subcontexts-guess",
                          SL_getHasSubContextsGuessStream,
                          XSD_BASIC_BOOL );
#undef SLEA
                    _Base::createStateLessAttributes( true );
                }
            }
    

    
        virtual void priv_read();

        virtual fh_context SubCreate_file( fh_context c, fh_context md );

public:

        ldapContext( Context* parent = 0, const std::string& rdn = "" )
            :
            _Base( parent, rdn ),
            callingXMLStreamingFunctions( false )
            {
                createStateLessAttributes();
            }
        
        
        virtual ldapContext::~ldapContext()
            {
            }
        

        fh_iostream getEAStream( Context* c, const std::string& rdn, EA_Atom* atom );
        void        setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream );

        
        /**
         * Put the given LDAP OID attributes into an XSD style attribute list in ss
         */
        void
        private_addIODsToXSDStream( char** oids, fh_stringstream& ss )
            {
                LDAPAttributeType* a = 0;
                int ec;
                const char* es;
                const int flags = LDAP_SCHEMA_ALLOW_ALL;

                for( char** iter = oids; *iter; ++iter )
                {
                    a = ldap_str2attributetype( *iter, &ec, &es, flags );
                    string name = a->at_names[0];
                    string description = a->at_desc;
                    int minLen = a->at_syntax_len;
                    
                    ss << "	  <elementType name=\"" << name << "\" default=\"\" "
                       << "      description=\"" << description << "\" "
                       << "      min-length=\"" << minLen << "\" "
                       << "      >\n"
                       << "	    <dataTypeRef name=\"string\"/>\n"
                       << "	  </elementType>\n";
                }
            }
        
        void
        priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
            {
                LDAP* ld = getLDAP();
                int ec = 0;
                const char* es = "";
                const int flags = LDAP_SCHEMA_ALLOW_ALL;
                fh_stringstream requiredAttributes;
                fh_stringstream optionalAttributes;
                
                string objectclass = getValue( "objectClass" );
                LDAPObjectClass* oc = ldap_str2objectclass( objectclass.c_str(),
                                                            &ec, &es, flags );

                if( !oc )
                {
                    LG_LDAP_W << "WARNING, can not find the object class"
                              << " for url:" << getURL() << endl
                              << " objectclass:" << objectclass
                              << " reason:" << es
                              << " code:" << ec
                              << endl;
                }
                else
                {
                    private_addIODsToXSDStream( oc->oc_at_oids_must, requiredAttributes );
                    private_addIODsToXSDStream( oc->oc_at_oids_may,  optionalAttributes );
                }

                LG_LDAP_W << "REQ:" << tostr(requiredAttributes) << endl;
                LG_LDAP_W << "OPT:" << tostr(optionalAttributes) << endl;
                
                {
                    fh_stringstream ss;
                    ss << "	<elementType name=\"dir\">\n"
                       << "		<elementType name=\"name\" default=\"new directory\">\n"
                       << "			<dataTypeRef name=\"string\"/>\n"
                       << "		</elementType>\n"
                       << tostr(requiredAttributes)
                       << tostr(optionalAttributes)
                       << "	</elementType>\n";
                    m["dir"] = SubContextCreator(SL_ldap_SubCreate_context, tostr(ss) );
                }
                {
                    fh_stringstream ss;
                    ss << "	<elementType name=\"file\">\n"
                       << "		<elementType name=\"name\" default=\"new file\">\n"
                       << "			<dataTypeRef name=\"string\"/>\n"
                       << "		</elementType>\n"
                       << tostr(requiredAttributes)
                       << tostr(optionalAttributes)
                       << "	</elementType>\n";
                    m["file"] = SubContextCreator(SL_ldap_SubCreate_context, tostr(ss) );
                }
                

                /* Note that EA created for the root of the db4 file will not show up
                 *  until after the db4 has been read()
                 */
                m["ea"] = SubContextCreator(
                    SL_ldap_SubCreate_ea,
                    "	<elementType name=\"ea\">\n"
                    "		<elementType name=\"name\" default=\"new ea\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "		<elementType name=\"value\" default=\"\">\n"
                    "			<dataTypeRef name=\"string\"/>\n"
                    "		</elementType>\n"
                    "	</elementType>\n");
            }
    };

/********************************************************************************/    
/********************************************************************************/    
/********************************************************************************/    

fh_context SL_ldap_SubCreate_context( fh_context c, fh_context md )
{
    return c->SubCreate_file( c, md );
}

fh_context SL_ldap_SubCreate_ea( fh_context c, fh_context md )
{
    LG_LDAP_D << "SL_ldap_SubCreate_ea() c:" << c->getURL() << endl;

    if( ldapContext* ldapc = dynamic_cast<ldapContext*>(GetImpl(c)))
    {
        string rdn      = getStrSubCtx( md, "name", "" );
        string v        = getStrSubCtx( md, "value", "" );

        ldapc->ldapModify( LDAP_MOD_REPLACE, rdn, v );
    }

    return c;
}


fh_context
ldapContext::SubCreate_file( fh_context c, fh_context md )
{
    string rdn      = getStrSubCtx( md, "name", "" );

    LG_LDAP_D << "ldapContext::SubCreate_file rdn:" << rdn
             << " c:" << c->getURL()
             << endl;
    
    if( ldapContext* ldapc = dynamic_cast<ldapContext*>(GetImpl(c)))
    {
        ldapc->ldapAdd( rdn );
        fh_context childc = ldapc->ensureContextCreated( rdn, true );
        return childc;
    }

    fh_stringstream ss;
    ss << "Attempt to create a subcontext on non LDAP child."
       << " url:" << c->getURL()
       << endl;
    Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
}

/********************************************************************************/    
/********************************************************************************/    
/********************************************************************************/    


fh_stringstream
ldapContext::real_getIOStream()
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           exception)
{
    fh_stringstream ss;

    if( callingXMLStreamingFunctions )
        return ss;

    Util::ValueRestorer<bool> dummy( callingXMLStreamingFunctions, true );
    
    fh_context thisc = this;
    ss << XML::contextToXML( thisc );
    
    return ss;
}

void
ldapContext::OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
{
    Factory::ensureXMLPlatformInitialized();

    LG_LDAP_D << "OnStreamClosed()" << endl;

    AdjustForOpenMode_Closing( ss, m, tellp );

    const string s = StreamToString(ss);
    fh_context thisc = this;
    XML::updateFromXML( thisc, s );
    
    LG_LDAP_D << "OnStreamClosed() len:" << s.length() << " s:" << s << endl;
}

fh_istream
ldapContext::priv_getIStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           CanNotGetStream,
           exception)
{
    return real_getIOStream();
}

fh_iostream
ldapContext::priv_getIOStream( ferris_ios::openmode m )
    throw (FerrisParentNotSetError,
           AttributeNotWritable,
           CanNotGetStream,
           exception)
{
    fh_stringstream ret = real_getIOStream();
    ret->getCloseSig().connect( bind( SigC::slot( *this, &ldapContext::OnStreamClosed ), m )); 

    LG_LDAP_D << "priv_getIOStream() url:" << getURL()
              << " tellp:" << ret->tellp()
              << " tellg:"  << ret->tellg()
              << endl;
    
    return ret;
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/



void
ldapContext::priv_read()
{
    LG_LDAP_D << "ldapContext::priv_read() url:" << getURL()
              << " is-parent-bound:" << isParentBound()
              << endl;
    if( !isParentBound() )
    {
        emitExistsEventForEachItemRAII _raii1( this );
        return;
    }

    EnsureStartStopReadingIsFiredRAII _raii1( this );
    AlreadyEmittedCacheRAII _raiiec( this );
    LG_LDAP_D << "ldapContext::priv_read(1)  url:" << getURL() << endl;
    LG_LDAP_D << "ldapContext::priv_read(1) name:" << getDirName() << endl;

    LDAP* ld = getLDAP();
    LG_LDAP_D << "ldapContext::priv_read(2)  dn:" << getDN() << endl;
    LG_LDAP_D << "ldapContext::priv_read(2) rootdn:" << getRootDN() << endl;
    LDAPMessage* result = 0;

    int rc = ldap_search_s( ld, getDN().c_str(),
                            LDAP_SCOPE_ONELEVEL,
                            0,
                            0, 1,
                            &result );
    LG_LDAP_D << "ldapContext::priv_read(3) rc:" << rc << endl;
    if( rc != LDAP_SUCCESS )
    {
        char* es = ldap_err2string(rc);
        if( result ) ldap_msgfree( result );
        fh_stringstream ss;
        ss << "Can not list URL:" << getURL()
           << " e:" << es;
        cerr << tostr(ss) << endl;
        Throw_LDAPException( tostr(ss), this );
    }

    LG_LDAP_D << "ldapContext::priv_read() count of found entries:"
              << ldap_count_entries( ld, result ) << endl;

    for( LDAPMessage* m = ldap_first_entry( ld, result ); m ; m = ldap_next_entry( ld, m ) )
    {
        LG_LDAP_D << "ldapContext::priv_read(4)" << endl;
        pair< string, string > rdninfo = getRdn( ld, m );
        string rdn = rdninfo.second;
        LG_LDAP_D << "ldapContext::priv_read() creating child:" << rdn << endl;
        
        fh_childc child = ensureContextCreated( rdn, false );
        child->setRdn( rdninfo );
        LG_LDAP_D << "ldapContext::priv_read() made child f:" << rdninfo.first << " rdn:" << rdn << endl;
        
        BerElement* berptr = 0;
        for( char* ea = ldap_first_attribute( ld, m, &berptr );
             ea; ea = ldap_next_attribute( ld, m, berptr ) )
        {
            LG_LDAP_D << "ldapContext::priv_read() ea:" << ea << endl;
            child->ensureEACreated( ea, false );
//             addAttribute(
//                 rdn,
//                 this, &_Self::getEAStream,
//                 this, &_Self::getEAStream,
//                 this, &_Self::setEAStream,
//                 FXD_BINARY,
//                 true );
        }
        ber_free( berptr, 0 );
    }
    
    if( result ) ldap_msgfree( result );

    LG_LDAP_D << "ldapContext::priv_read(done) path:" << getDirPath() << endl;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

fh_iostream
ldapContext::getEAStream( Context* c, const std::string& rdn, EA_Atom* atom )
{
    LG_LDAP_D << "ldapContext::getEAStream(X) c:" << c->getURL()
              << " this:" << getURL()
              << " rdn:" << rdn << endl;

    fh_stringstream ss;
    ss << getValue( rdn );
    return ss;
}

void
ldapContext::setEAStream( Context* c, const std::string& rdn, EA_Atom* atom, fh_istream zz )
{
    string v = StreamToString( zz );
    ldapModify( LDAP_MOD_REPLACE, rdn, v );
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

extern "C"
{
    fh_context Brew( RootContextFactory* rf )
        throw( RootContextCreationFailed )
    {
//         static ldapContext c;
//         fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
//        return ret;
        
        try {
            const string& serv = rf->getInfo( RootContextFactory::SERVERNAME );
            const string& root = rf->getInfo( RootContextFactory::ROOT );
            const string& path = rf->getInfo( RootContextFactory::PATH );

//            cerr << "LDAP serv:" << serv << " root:" << root << " path:" << path << endl;
            
            
            static ldapContext* c = 0;
            if( !c )
            {
                c = new ldapContext(0, "/");
                // Bump ref count.
                static fh_context keeper = c;
                static fh_context keeper2 = keeper;
            }
        
            fh_context ret = c;
            if( root != "/" )
            {
                fh_stringstream ss;
                ss << root << "/" << path;
                rf->AddInfo( RootContextFactory::PATH, tostr(ss) );
            }

            

            /*
             * a hack to make the requested server path exist without checking
             * if the server really exists
             */
            fh_stringstream ss;
            string s;
            ldapContext* ctx = dynamic_cast<ldapContext*>(c);

            ss << root << "/" << path;
            while( getline( ss, s, '/' ))
            {
                if( s.empty() )
                    continue;

                LG_LDAP_D << "Brew() server path:" << s << endl;
                ldapContext* child = new ldapContext( ctx, s );
                ctx->addNewChild( child );
                break;
            }
            if( ctx->empty() )
            {
                string s = "localhost";
                
                ldapContext* child = new ldapContext( ctx, s );
                ctx->addNewChild( child );
            }
            

            LG_LDAP_D << "ldap.brew() ret:" << ret->getURL() << endl;
            return ret;
        }
        catch( exception& e )
        {
            LG_LDAP_D << "Brew() e:" << e.what() << endl;
            Throw_RootContextCreationFailed( e.what(), 0 );
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
