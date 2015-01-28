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

    $Id: libeaindexldap.cpp,v 1.1 2006/12/07 06:49:42 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include "libeaindexldap.hh"

#include "EAIndexerMetaInterface.hh"
#include "EAIndexer.hh"
#include "EAQuery.hh"
#include "General.hh"

// guessComparisonOperatorFromData
#include "FilteredContext_private.hh"

#include <ldap.h>
#include <ldap_schema.h>
#include <errno.h>

using namespace std;


namespace Ferris
{
    namespace EAIndex 
    {
        
//         static const char* CFG_LDAPIDX_SERVERNAME_K       = "ldapidx-servername";
//         static const char* CFG_LDAPIDX_SERVERNAME_DEFAULT = "localhost";
//         static const char* CFG_LDAPIDX_PORT_K             = "ldapidx-port";
//         static const char* CFG_LDAPIDX_PORT_DEFAULT       = "389";
//         static const char* CFG_LDAPIDX_USERNAME_K         = "ldapidx-user";
//         static const char* CFG_LDAPIDX_USERNAME_DEFAULT   = "";
//         static const char* CFG_LDAPIDX_PASSWORD_K         = "ldapidx-password";
//         static const char* CFG_LDAPIDX_PASSWORD_DEFAULT   = "";

        static const char* LDAP_DOCID_ATTR = "documentIdentifier";
        static const char* LDAP_URL_ATTR   = "ferris-url-string";


        static const char* LIBFERRIS_OID_PREFIX = "2.5.18.1.2.5.18.1.2.5.18.1.1.1.";
        static const char* CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_K = "ldapidx-ferriscontext-attrlist";
        static const char* CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_DEFAULT = "";
        
        static const char* LDAP_STRING_SYNTAX   = "1.3.6.1.4.1.1466.115.121.1.15";
        static const char* LDAP_STRING_EQUALITY = "2.5.13.2";
        static const char* LDAP_STRING_ORDERING = "2.5.13.3";
        static const char* LDAP_STRING_SUBSTR   = "2.5.13.4";
        static const char* LDAP_STRING_LENGTH   = "200";
        
        static const char* LDAP_INT_SYNTAX   = "1.3.6.1.4.1.1466.115.121.1.27";
        static const char* LDAP_INT_EQUALITY = "2.5.13.14";
        static const char* LDAP_INT_LENGTH   = "11";

        static string
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

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/

        typedef Loki::SmartPtr< LDAPMod, 
                                Loki::RefLinked, 
                                Loki::AllowConversion, 
                                FerrisLoki::FerrisExSmartPointerChecker, 
                                Loki::DefaultSPStorage >  fh_ldapmod;
        typedef Loki::SmartPtr< LDAPMod*, 
                                Loki::RefLinked, 
                                Loki::AllowConversion, 
                                FerrisLoki::FerrisExSmartPointerChecker, 
                                Loki::DefaultSPStorage >  fh_ldapmods;
    
        class FERRISEXP_DLLLOCAL FerrisLDAPMod
        {
            typedef list< fh_ldapmod > m_mod_t;
            m_mod_t      m_mod;
            int          m_opcode;
            stringlist_t m_strcache;
            typedef      list< struct berval** > m_berval_t;
            m_berval_t   m_berval;
            list< stringlist_t >    m_slcache;
    
            struct berval** MakeBerVal( const stringlist_t& sl )
                {
                    struct berval** bvals = (struct berval**)calloc( sl.size()+1,
                                                                     sizeof(struct berval*));
                    int i=0;
                    struct berval* bval = bvals[0];
                    for( list< string >::const_iterator si = sl.begin(); si != sl.end(); ++si, ++i )
                    {
                        bvals[ i ] = (struct berval*)calloc( 1, sizeof(struct berval));
                        bvals[ i ]->bv_len = si->length();
                        bvals[ i ]->bv_val = (char*)si->data();
                    }
                    return bvals;
                }

            fh_ldapmods getNativeLDAPMod();
    
        public:
            FerrisLDAPMod( int opcode );
            ~FerrisLDAPMod();
            void add( const std::string& k, const std::string& v, int opcode = 0 );
            void add( const std::string& k, const stringlist_t& sl, int opcode = 0 );

            int getOperationCount();
            
            int ldap_add_s( LDAP* ld, const std::string& dn );
            int ldap_modify_s( LDAP* ld, const std::string& dn );
            };

    FerrisLDAPMod::FerrisLDAPMod( int opcode )
        :
        m_opcode( opcode )
    {
    }

    FerrisLDAPMod::~FerrisLDAPMod()
    {
        m_mod.clear();
            
        m_berval_t   m_berval;
        for( m_berval_t::iterator bi = m_berval.begin(); bi != m_berval.end(); )
        {
            m_berval_t::iterator c = bi;
            ++bi;

            struct berval** bvals = *c;
            for( int i=0; bvals[ i ]; ++i )
                free( bvals[ i ] );
            free( bvals );
        }
    }

    void
    FerrisLDAPMod::add( const std::string& k, const std::string& v, int opcode )
    {
        stringlist_t sl;
        sl.push_back( v );
        add( k, sl, opcode );
    }

    void
    FerrisLDAPMod::add( const std::string& k, const stringlist_t& sl, int opcode )
    {
        m_mod.push_back( new LDAPMod );
//            m_mod[ m_modidx ] = (LDAPMod*)calloc( 1, sizeof( LDAPMod ) );
        LDAPMod* m = m_mod.back();
            
        if( opcode )
            m->mod_op    = opcode | LDAP_MOD_BVALUES;
        else
            m->mod_op    = m_opcode | LDAP_MOD_BVALUES;

        m_slcache.push_back( sl );
        struct berval** bv = MakeBerVal( m_slcache.back() );
        m_berval.push_back( bv );
    
        m_strcache.push_back( k );
        m->mod_type  = (char*)m_strcache.back().c_str();
        m->mod_vals.modv_strvals = 0;
        m->mod_vals.modv_bvals   = bv;
    }

    int
    FerrisLDAPMod::getOperationCount()
    {
        return m_mod.size();
    }
        
    fh_ldapmods
    FerrisLDAPMod::getNativeLDAPMod()
    {
        int sz = m_mod.size();
        typedef LDAPMod* LDAPMod_ptr;
        LDAPMod** ret = new LDAPMod_ptr[ sz+1 ];
        ret[ sz ] = 0;

//        LG_EAIDX_D << "FerrisLDAPMod::getNativeLDAPMod() sz:" << sz << endl;
        
        m_mod_t::iterator mi = m_mod.begin();
        for( int i=0; i < sz; ++i )
        {
            ret[ i ] = *mi;
//            LG_EAIDX_D << "FerrisLDAPMod::getNativeLDAPMod() attr:" << ret[i]->mod_type << endl;
            ++mi;
        }
        return ret;
    }
    
    int
    FerrisLDAPMod::ldap_add_s( LDAP* ld, const std::string& dn )
    {
//    m_mod[ m_modidx ] = 0;

//         LDAPMod** m = m_mod;
//         for( int i=0; ; ++i )
//         {
// //                cerr << "...i:" << i << " m:" << *m << endl;
//             ++m;
//             if( !*m )
//                 break;
//         }

        fh_ldapmods m = getNativeLDAPMod();
        return ::ldap_add_s( ld, dn.c_str(), m );
    }

    int
    FerrisLDAPMod::ldap_modify_s( LDAP* ld, const std::string& dn )
    {
        fh_ldapmods m = getNativeLDAPMod();
        return ::ldap_modify_s( ld, dn.c_str(), m );
    }
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL EAIndexerLDAP
            :
            public MetaEAIndexerInterface
        {
            LDAP* m_ldap;
            int   m_currentMaxDocID; // for generating new docuemnt IDs
            int   m_newOID;          // for generating new postfixes for schema attr OIDs
            map< int, string > m_docIDHash;
            stringset_t        m_ferrisContextAttributes;

            // These are mainly used in addToIndex() and its helper
            // methods.
            struct AttrInfo 
            {
                string v;
                string cop;
            };
            typedef map< string, AttrInfo > AttributeValues_t;
            
        protected:
            
            virtual void Setup();
            virtual void CreateIndex( fh_context c,
                                      fh_context md );
            virtual void CommonConstruction();

            
        public:

            EAIndexerLDAP();

            LDAP*  getLDAP();
            string getServerName();
            int    getServerPort();
            string getUserName();
            string getPassword();
            string getBaseDN();
            
            string dnFromURL( const std::string& url );
            string toLDAPAttributeName( const std::string& ea,
                                        const std::string& cop );
            
            virtual ~EAIndexerLDAP();

            void closeConnection();
            void openConnection();

            void readIndexableAttrbiutes( fh_context c,
                                          fh_docindexer di,
                                          const std::string& dn,
                                          AttributeValues_t& AttrubuteValues );
            void attemptBulkLDAPWrite( fh_context c,
                                       fh_docindexer di,
                                       const std::string& dn,
                                       AttributeValues_t& AttrubuteValues );
            void modifySchemaForAttributes( fh_context c,
                                            fh_docindexer di,
                                            const std::string& dn,
                                            AttributeValues_t& AttrubuteValues );
            void singleWriteUpdateForAttributes( fh_context c,
                                                 fh_docindexer di,
                                                 const std::string& dn,
                                                 AttributeValues_t& AttrubuteValues );
            

            virtual void sync();

            virtual void reindexingDocument( fh_context c,
                                             const std::string& dn,
                                             docid_t docid );
            docid_t obtainDocumentID( fh_context c );
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );
            void
            BuildQuery( fh_context q,
                        docNumSet_t& output,
                        fh_eaquery qobj,
                        fh_stringstream& ldap_query );
            virtual docNumSet_t& ExecuteQuery( fh_context q,
                                               docNumSet_t& output,
                                               fh_eaquery qobj,
                                               int limit = 0 );


            virtual std::string resolveDocumentID( docid_t );
            virtual void cleanDocumentIDCache();

            /**************************************************/
            /**************************************************/
            /**************************************************/
            /**************************************************/

            string
            getLDAPValue( const std::string& dn, const std::string& attrName )
                {
                    string ret;
                    LDAP* ld = getLDAP();
                
                    try
                    {
                        LDAPMessage* result = 0;
                        
                        char* attrs[ 2 ] = { "", 0 };
                        attrs[0] = (char*)attrName.c_str();

                    
                        int rc = ldap_search_s( ld,
                                                dn.c_str(),
                                                LDAP_SCOPE_BASE,
                                                0,
                                                attrs, 0,
                                                &result );
                        if( rc != LDAP_SUCCESS )
                        {
                            char* es = ldap_err2string(rc);
                            if( result ) ldap_msgfree( result );
                            fh_stringstream ss;
                            ss << "Can not get attribute:" << attrName
                               << " for dn:" << dn
                               << " e:" << es;
                            cerr << tostr(ss) << endl;
                            Throw_EAIndexException( tostr(ss), 0 );
                        }

                        for( LDAPMessage* m = ldap_first_entry( ld, result );
                             m && ret.empty(); m = ldap_next_entry( ld, m ) )
                        {
                            BerElement* berptr = 0;
                            for( char* ea = ldap_first_attribute( ld, m, &berptr );
                                 ea; ea = ldap_next_attribute( ld, m, berptr ) )
                            {
                                if( attrName == ea )
                                {
                                    ret = getA( ld, m, attrName );
                                    break;
                                }
                                ldap_memfree( ea ); 
                            }
                        
                            ber_free( berptr, 0 );
                        }
                    
                        if( result ) ldap_msgfree( result );
                        return ret;
                    }
                    catch( exception& e )
                    {
                        fh_stringstream ss;
                        ss << "Can not get attribute:" << attrName
                           << " for dn:" << dn
                           << " e:" << e.what();
                        Throw_EAIndexException( tostr(ss), 0 );
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
            void ldapModify( const std::string& dn,
                             int opcode,
                             const string& eaname, stringlist_t values )
                {
                    int values_size = values.size();
                    LDAP* ld = getLDAP();
                    struct berval** bvals = (struct berval**)calloc( values_size+1,
                                                                     sizeof(struct berval*));
//                    cerr << "ldapModify() eaname:" << eaname << " values.sz:" << values_size << endl;
                    int i=0;
                    struct berval* bval = bvals[0];
                    for( stringlist_t::iterator si = values.begin(); i<values_size; ++si, ++i )
                    {
//                         cerr << "ldapModify() val i:" << i
//                              << " value.len:" << si->length()
//                              << " dn:" << dn
//                              << " eaname:" << eaname
//                              << endl;
                        bvals[ i ] = (struct berval*)calloc( 1, sizeof(struct berval));
                        bvals[ i ]->bv_len = si->length();
                        bvals[ i ]->bv_val = (char*)si->data();
                    }

//                     LDAPMod* mods[3];
//                     LDAPMod mod1,mod2;
//                     mod1.mod_op    = LDAP_MOD_DELETE;
//                     mod1.mod_type  = (char*)eaname.c_str();
//                     mod1.mod_vals.modv_strvals = 0;
//                     mod1.mod_vals.modv_bvals   = 0;
//                     mod2.mod_op    = opcode | LDAP_MOD_BVALUES;
//                     mod2.mod_type  = (char*)eaname.c_str();
//                     mod2.mod_vals.modv_strvals = 0;
//                     mod2.mod_vals.modv_bvals   = bvals;
                
//                     mods[0] = &mod1;
//                     mods[1] = &mod2;
//                     mods[2] = 0;

                    
                    LDAPMod* mods[2];
                    LDAPMod mod;
                    mod.mod_op    = opcode | LDAP_MOD_BVALUES;
                    mod.mod_type  = (char*)eaname.c_str();
                    mod.mod_vals.modv_strvals = 0;
                    mod.mod_vals.modv_bvals   = bvals;
                
                    mods[0] = &mod;
                    mods[1] = 0;

                    
                    int rc = ldap_modify_s( ld, dn.c_str(), mods );

                    for( int i=0; i<values.size(); ++i )
                        free( bvals[ i ] );
                    free( bvals );
                
                    if( rc != LDAP_SUCCESS )
                    {
                        char* es = ldap_err2string(rc);
                        fh_stringstream ss;
                        ss << "ERROR: Can't perform modification on dn:" << dn
                           << " eaname:" << eaname
                           << " rc:" << rc
                           << " rch:" << hex << rc
                           << " reason:" << es
                           << endl;
                        Throw_EAIndexException( tostr(ss), 0 );
                    }
                }
            void ldapModify( const std::string& dn,
                             int opcode,
                             const string& eaname, string v )
                {
                    stringlist_t listv;
                    listv.push_back( v );
                    ldapModify( dn, opcode, eaname, listv );
                }

            
            void ldapAdd( const string& dn )
                {
                    LDAP* ld = getLDAP();
                    LDAPMod *attrs[] = { 0 };
                
                    int rc = ldap_add(ld, dn.c_str(), attrs );
                
                    if( rc != LDAP_SUCCESS )
                    {
                        char* es = ldap_err2string(rc);
                        fh_stringstream ss;
                        ss << "ERROR: Can't add new context on dn:" << dn
                           << " reason:" << es
                           << endl;
                        Throw_LDAPException( tostr(ss), 0 );
                    }
                
                }
            
            void setLDAPValue( const std::string& dn,
                               const std::string& k,
                               const std::string& v )
                {
                    ldapModify( dn, LDAP_MOD_REPLACE, k, v );
                }
            void addLDAPValue( const std::string& dn,
                               const std::string& k,
                               const std::string& v )
                {
                    ldapModify( dn, LDAP_MOD_ADD, k, v );
                }
            
        };

        /************************************************************/
        /************************************************************/
        /************************************************************/

        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        EAIndexerLDAP::EAIndexerLDAP()
            :
            m_ldap( 0 ),
            m_currentMaxDocID( toint( CFG_LDAPIDX_CURRENTMAXDOCID_DEFAULT ) ),
            m_newOID( toint( CFG_LDAPIDX_NEWOID_DEFAULT ) )
        {
        }

        

        EAIndexerLDAP::~EAIndexerLDAP()
        {
//            cerr << "EAIndexerLDAP::~EAIndexerLDAP()" << endl;
            
            sync();
            
            if( m_ldap )
            {
//                cerr << "Closing LDAP connection." << endl;
                closeConnection();
            }
        }

        void
        EAIndexerLDAP::closeConnection()
        {
            int rc = ldap_unbind_s( m_ldap );
            m_ldap = 0;

            if( rc != LDAP_SUCCESS )
            {
                char* es = ldap_err2string(rc);
                fh_stringstream ss;
                ss << "ERROR: Cant unbind connection to server:" << getServerName()
                   << " port:" << getServerPort()
                   << " reason:" << es
                   << endl;
                cerr << tostr(ss) << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }
        }

        void
        EAIndexerLDAP::openConnection()
        {
            m_ldap = ldap_init( getServerName().c_str(),
                                getServerPort() );
            LG_EAIDX_D << "CommonConstruction() setting up connection:"
                       << toVoid(m_ldap) << endl;

            if( !m_ldap )
            {
                string es = errnum_to_string( "", errno );
                fh_stringstream ss;
                ss << "ERROR: Cant open connection to server:" << getServerName()
                   << " port:" << getServerPort()
                   << " reason:" << es
                   << endl;
                cerr << tostr(ss) << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }

            int rc = ldap_simple_bind_s( m_ldap,
                                         getUserName().c_str(),
                                         getPassword().c_str() );
            if( rc != LDAP_SUCCESS )
            {
                char* es = ldap_err2string(rc);
                fh_stringstream ss;
                ss << "ERROR: Cant bind connection to server:" << getServerName()
                   << " port:" << getServerPort()
                   << " reason:" << es
                   << endl;
                cerr << tostr(ss) << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }
        }
        
        

        LDAP*
        EAIndexerLDAP::getLDAP()
        {
            return m_ldap;
        }
        
        string
        EAIndexerLDAP::getServerName()
        {
            return getConfig( CFG_LDAPIDX_SERVERNAME_K,
                              CFG_LDAPIDX_SERVERNAME_DEFAULT );
        }

        int
        EAIndexerLDAP::getServerPort()
        {
            return toint( getConfig( CFG_LDAPIDX_PORT_K,
                                     CFG_LDAPIDX_PORT_DEFAULT ) );
        }
            
        string
        EAIndexerLDAP::getUserName()
        {
            return getConfig( CFG_LDAPIDX_USERNAME_K,
                              CFG_LDAPIDX_USERNAME_DEFAULT );
        }
        
        string
        EAIndexerLDAP::getPassword()
        {
            return getConfig( CFG_LDAPIDX_PASSWORD_K,
                              CFG_LDAPIDX_PASSWORD_DEFAULT );
        }

        string
        EAIndexerLDAP::getBaseDN()
        {
            return getConfig( CFG_LDAPIDX_BASEDN_K,
                              CFG_LDAPIDX_BASEDN_DEFAULT );
        }

        string
        EAIndexerLDAP::toLDAPAttributeName( const std::string& ea,
                                            const std::string& cop )
        {
            string ret = (string)"ferris-" + ea + "-string";
            
            if( cop == "int" || cop == "long" )
                ret = (string)"ferris-" + ea + "-int";

            
            ret = Util::replace_all( ret, "-ns-", "-ns--ns-" );
            ret = Util::replace_all( ret, ':', "-ns-" );

            ret = Util::replace_all( ret, '_', "__" );
            ret = Util::replace_all( ret, ' ', '_' );
            
//             cerr << "toLDAPAttributeName ea:" << ea
//                  << " cop:" << cop << " ret:" << ret << endl;

            return ret;
        }
        
        string
        EAIndexerLDAP::dnFromURL( const std::string& url )
        {
            fh_stringstream iss;
            fh_stringstream oss;
            vector< string > tmp;
            string s;

            iss << url;
            while( getline( iss, s, '/' ))
                tmp.push_back( s );

            for( vector< string >::reverse_iterator ti = tmp.rbegin(); ti!=tmp.rend(); ++ti )
            {
                if( !ti->empty() )
                {
                    string s = Util::replace_all( *ti, ':', '_' );
                    oss << "cn=" << s << ",";
                }
            }
        
            oss << getBaseDN();
            
//             cerr << "dnFromURL() url:" << url << endl
//                  << " ret:" << tostr(oss) << endl;
            return tostr(oss);
        }
        
        void
        EAIndexerLDAP::Setup()
        {
        }
        
        void
        EAIndexerLDAP::CreateIndex( fh_context c,
                                    fh_context md )
        {
            string serv = getStrSubCtx( md, CFG_LDAPIDX_SERVERNAME_K, "" );
            setConfig( CFG_LDAPIDX_SERVERNAME_K, serv );

            string port = getStrSubCtx( md, CFG_LDAPIDX_PORT_K, "" );
            setConfig( CFG_LDAPIDX_PORT_K, port );
            
            string userName = getStrSubCtx( md, CFG_LDAPIDX_USERNAME_K, "" );
            setConfig( CFG_LDAPIDX_USERNAME_K, userName );

            string password = getStrSubCtx( md, CFG_LDAPIDX_PASSWORD_K, "" );
            setConfig( CFG_LDAPIDX_PASSWORD_K, password );

            string basedn = getStrSubCtx( md, CFG_LDAPIDX_BASEDN_K, "" );
            setConfig( CFG_LDAPIDX_BASEDN_K, basedn );
            
            m_currentMaxDocID = toint( CFG_LDAPIDX_CURRENTMAXDOCID_DEFAULT );
            setConfig( CFG_LDAPIDX_CURRENTMAXDOCID_K, tostr( m_currentMaxDocID ));

            m_newOID = toint( CFG_LDAPIDX_NEWOID_DEFAULT );
            setConfig( CFG_LDAPIDX_NEWOID_K, tostr( m_newOID ));
            
            {
                string s =  Util::createCommaSeperatedList( m_ferrisContextAttributes );
                setConfig( CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_K, s );
            }
        }
        
        void
        EAIndexerLDAP::CommonConstruction()
        {
            openConnection();
            
            m_currentMaxDocID = toint(
                getConfig( CFG_LDAPIDX_CURRENTMAXDOCID_K,
                           CFG_LDAPIDX_CURRENTMAXDOCID_DEFAULT ));

            m_newOID = toint(             
                getConfig( CFG_LDAPIDX_NEWOID_K,
                           CFG_LDAPIDX_NEWOID_DEFAULT ));

            {
                string s = getConfig( CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_K,
                                      CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_DEFAULT );
                Util::parseCommaSeperatedList( s, m_ferrisContextAttributes );
                LG_EAIDX_D << "loaded m_ferrisContextAttributes.size:"
                           << m_ferrisContextAttributes.size()
                           << endl;
//                m_ferrisContextAttributes.sort();
//                 stringlist_t::iterator newend = unique( m_ferrisContextAttributes.begin(),
//                                                         m_ferrisContextAttributes.end() );
//                m_ferrisContextAttributes.erase( newend, m_ferrisContextAttributes.end() );
            }
            
        }
        
        
        void
        EAIndexerLDAP::sync()
        {
            setConfig( CFG_LDAPIDX_CURRENTMAXDOCID_K, tostr( m_currentMaxDocID ));
            setConfig( CFG_LDAPIDX_NEWOID_K, tostr( m_newOID ));
            {
                string s =  Util::createCommaSeperatedList( m_ferrisContextAttributes );
                setConfig( CFG_LDAPIDX_FERRISCONTEXT_ATTRLIST_K, s );
            }
        }


        /************************************************************/
        /************************************************************/
        /************************************************************/

        void
        createDocument_rec( LDAP* ld,
                            stringlist_t::const_iterator dn_begin,
                            stringlist_t::const_iterator dn_end,
                            int docid )
        {
    
            string dn;
            {
                bool v = true;
                for( stringlist_t::const_iterator ci = dn_begin; ci!=dn_end; ++ci )
                {
                    if( v ) v = false;
                    else    dn += ",";
                    dn += *ci;
                }
            }

            string cn = *dn_begin;
            FerrisLDAPMod m( LDAP_MOD_ADD );
            stringlist_t sl;
            sl.push_back("document");
            sl.push_back("ferris-context");
            sl.push_back("container");
            sl.push_back("top");
            m.add( "objectclass", sl );
            m.add( "documentidentifier", tostr( docid ) );
            m.add( "cn", cn );

//            cerr << "Attempting to add DN:" << dn << endl;
    
            int rc = m.ldap_add_s( ld, dn );

            int DEBUG = 1;
            if( DEBUG )
            {
                fh_stringstream ss;
                ss << nl
                   << "dn: " << dn << nl;
                for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                    ss << "objectClass:" << *si << nl;
                ss << "cn: " << cn << endl;
                ss << "documentidentifier: " << docid << endl;
                LG_EAIDX_D << tostr(ss) << endl;
            }
            LG_EAIDX_D << "Attempting to add a new document to index. "
                       << " dn:" << dn
                       << " cn:" << *dn_begin
                       << " documentID:" << tostr( docid )
                       << " rc:" << rc
                       << " reason:" << ldap_err2string(rc)
//                       << " objectclasses: doc, f-ctx, container, top"
                       << endl;
            
            if( rc == LDAP_NO_SUCH_OBJECT || rc == LDAP_OBJECT_CLASS_VIOLATION )
            {
                stringlist_t::const_iterator be = dn_begin;
                ++be;
                createDocument_rec( ld, be, dn_end, docid );
                rc = m.ldap_add_s( ld, dn );
            }
            if( rc != LDAP_SUCCESS )
            {
                const char* es = ldap_err2string(rc);
                fh_stringstream ss;
                ss << "ERROR: Can't add new context on dn:" << dn
                   << " reason:" << es
                   << endl;
                cerr << ss.str() << endl;
                LG_EAIDX_I << ss.str() << endl;
                Throw_LDAPException( tostr(ss), 0 );
            }
        }


        void
        createDocument( LDAP* ld,
                        const std::string& dn,
                        int docid )
        {
            stringlist_t dnl;
            stringstream iss;
            string s;
            iss << dn;
            while( getline( iss, s, ',' ) )
                dnl.push_back( s );
    
            createDocument_rec( ld, dnl.begin(), dnl.end(), docid );
        }
        
        /************************************************************/
        /************************************************************/
        /************************************************************/

        void
        EAIndexerLDAP::reindexingDocument( fh_context c,
                                           const std::string& dn,
                                           docid_t docid )
        {
//             int rc = ldap_delete_s( m_ldap, dn.c_str() );
//             cerr << "removal of dn:" << dn
//                  << " rc:" << rc
//                  << endl;
    
//             createDocument( m_ldap, dn, docid );
        }

        docid_t
        EAIndexerLDAP::obtainDocumentID( fh_context c )
        {
            const std::string& earl = c->getURL();
            const std::string dn = dnFromURL( earl );
            
            try
            {
                docid_t docid = toint( getLDAPValue( dn, LDAP_DOCID_ATTR ) );
                LG_EAIDX_D << "obtainDocumentID() c:" << earl
                           << " found pre existing docid:" << docid << endl;
                reindexingDocument( c, dn, docid );
                return docid;
            }
            catch( exception& e )
            {
                LG_EAIDX_D << "obtainDocumentID( adding new ID ) c:" << c->getURL() << endl;
                
                docid_t docid = m_currentMaxDocID;
                ++m_currentMaxDocID;

//                ldapAdd( dn );
                createDocument( m_ldap, dn, docid );
//                setLDAPValue( dn, LDAP_DOCID_ATTR, tostr( docid ) );
                return docid;
            }
        }

        static std::string toupper(const std::string& x)
        {
            std::string ret = x;
                
            for( std::string::iterator p = ret.begin(); p != ret.end(); ++p )
            {
                *p = ::toupper( *p );
            }
                
            return ret;
        }

        /**
         * Read the values for each EA that should be added to the index.
         * The comparison operator that should be used is also cached
         * in the AttrubuteValues parameter.
         */
        void
        EAIndexerLDAP::readIndexableAttrbiutes( fh_context c,
                                                fh_docindexer di,
                                                const std::string& dn,
                                                AttributeValues_t& AttrubuteValues )
        {
            Time::Benchmark bm( "reading acceptable attributes" );
            bm.start();

            typedef AttributeCollection::AttributeNames_t ant;
            ant an;
            c->getAttributeNames( an );
            
            for( ant::const_iterator ai = an.begin(); ai!=an.end(); ++ai )
            {
                try
                {
                    string attributeName = *ai;

                    LG_EAIDX_D << "addContextToIndex(building attrs) c:" << c->getURL() << endl
                               << " attributeName:" << attributeName << endl;
                    
                    if( !shouldIndex( c, di, attributeName ))
                    {
                        LG_EAIDX_D << "addContextToIndex(shouldn't index)"
                                   << " c:" << c->getURL() << endl
                                   << " attributeName:" << attributeName << endl;
                        continue;
                    }
                    
                    string k = attributeName;
                    string v = "";
                    if( !obtainValueIfShouldIndex( c, di,
                                                   attributeName, v ))
                    {
                        LG_EAIDX_D << "addContextToIndex(value not indexable)"
                                   << " c:" << c->getURL() << endl
                                   << " attributeName:" << attributeName << endl;
                        continue;
                    }

                    fh_context sc = c->getSchema( k );
                    string cop = getSchemaDefaultSort( sc );
                    string ldapk = toLDAPAttributeName( k, cop );
                    
                    AttrubuteValues[ ldapk ].v   = v;
                    AttrubuteValues[ ldapk ].cop = cop;
                }
                catch( exception& e )
                {
                    LG_EAIDX_W << "reading attribute values... EXCEPTION e:" << e.what() << endl;
                }
            }

            AttrubuteValues[ LDAP_URL_ATTR ].v   = c->getURL();
            AttrubuteValues[ LDAP_URL_ATTR ].cop = "string";
        }

        /**
         * For each attribute that is already known to the LDAP server
         * add an LDAP MODIFY entry to a single LDAP modify_s() structure.
         * This should make updating the index much quicker because there
         * is less API calls and the LDAP server can perform more updates
         * with the same reference to the DN.
         */
        void
        EAIndexerLDAP::attemptBulkLDAPWrite( fh_context c,
                                             fh_docindexer di,
                                             const std::string& dn,
                                             AttributeValues_t& AttrubuteValues )
        {
            FerrisLDAPMod m( LDAP_MOD_REPLACE );
            typedef list< AttributeValues_t::iterator > bulkkeys_t;
            bulkkeys_t bulkkeys;
                
            for( AttributeValues_t::iterator ai = AttrubuteValues.begin();
                 ai != AttrubuteValues.end(); ++ai )
            {
                string attributeName = ai->first;
                string ldapk = ai->first;
                string     v = ai->second.v;

                LG_EAIDX_D << "bulk, checking for key:" << ldapk
                           << " count:" << m_ferrisContextAttributes.count( ldapk ) << endl;
                    
                if( m_ferrisContextAttributes.count( ldapk ) )
                {
                    m.add( ldapk, v, LDAP_MOD_REPLACE );
                    bulkkeys.push_back( ai );
                    LG_EAIDX_D << "bulk, adding k:" << ldapk << " v:" << v << endl;
                }
            }
                
            {
                Time::Benchmark bm( "bulk modify" );
                bm.start();
                    
                LG_EAIDX_D << "bulk, ldap_modify()ing for a total of:" << m.getOperationCount()
                           << " update operations." << endl;
                
                int rc = m.ldap_modify_s( m_ldap, dn );
                if( rc == LDAP_SUCCESS )
                {
                    LG_EAIDX_D << "bulk update success for a total of:" << m.getOperationCount()
                               << " update operations." << endl;
                    
                    for( bulkkeys_t::const_iterator bi = bulkkeys.begin();
                         bi != bulkkeys.end(); ++bi )
                    {
                        AttrubuteValues.erase( *bi );
                    }
                }
                else
                {
                    char* es = ldap_err2string(rc);
                    LG_EAIDX_W << "ERROR: bulk, ldap_modify() failed for keys which already"
                               << " have a known schema in LDAP. rc:" << rc
                               << " error string:" << es
                               << endl;
                }
            }
        }
        

        void
        EAIndexerLDAP::modifySchemaForAttributes( fh_context c,
                                                  fh_docindexer di,
                                                  const std::string& dn,
                                                  AttributeValues_t& AttrubuteValues )
        {
            for( AttributeValues_t::const_iterator ai = AttrubuteValues.begin();
                 ai != AttrubuteValues.end(); ++ai )
            {
                string attributeName = ai->first;
                string ldapk = ai->first;
                string     v = ai->second.v;
                string   cop = ai->second.cop;
                
                try
                {
                    LG_EAIDX_D << "addContextToIndex(single+sc) c:" << c->getURL() << endl
                               << " ldapk:" << ldapk << endl
                               << " v:" << v
                               << " cop:" << cop
                               << endl;
                    
                    string oid = LIBFERRIS_OID_PREFIX + tostr( m_newOID );
                    ++m_newOID;

                    string syntax   = LDAP_STRING_SYNTAX;
                    string equality = LDAP_STRING_EQUALITY;
                    string length   = LDAP_STRING_LENGTH;
                    string dbname   = (string)"FRS" + toupper( tostr( m_newOID-1 ) );
                        
                    if( cop == "int" )
                    {
                        syntax   = LDAP_INT_SYNTAX;
                        equality = LDAP_INT_EQUALITY;
                        length   = LDAP_INT_LENGTH;
                    }

                    fh_stringstream val1ss;
                    val1ss << "( " << oid << nl
                           << " NAME '" << ldapk + "' " << nl
                           << " DESC 'DO NOT EDIT: AUTO UPDATED by indexing engine.' " << nl
                           << " EQUALITY " << equality << nl;
                    if( cop != "int" && cop != "double" )
                    {
                        val1ss << " ORDERING " << LDAP_STRING_ORDERING << " "; // "2.5.13.3 ";
                        val1ss << " SUBSTR "   << LDAP_STRING_SUBSTR   << " ";
                    }
                    val1ss << " SYNTAX " << syntax << "{" << length << "} " << nl
                           << " SINGLE-VALUE )";
                    string vals1str = tostr(val1ss);
                    char* vals1[] = { (char*)vals1str.c_str(), 0 };

                    fh_stringstream vals2ss;
                    vals2ss << "( " << oid
                            << " DBNAME ( '" + dbname + "'  '" + dbname + "' ) "
                            << " LENGTH " << length
                            << " EQUALITY ORDERING ";
                    if( cop != "int" && cop != "double" )
                        vals2ss << " SUBSTR APPROX ";
                    vals2ss << " )";
                    string vals2str = tostr( vals2ss );
                    char* vals2[] = { (char*)vals2str.c_str(), 0 };
                        
                    LDAPMod attr1;
                    LDAPMod attr2;
                    LDAPMod* attrs[] = { &attr1, &attr2, 0 };
                    attr1.mod_op = LDAP_MOD_ADD;
                    attr1.mod_type = "attributeTypes";
                    attr1.mod_values = vals1;
                    attr2.mod_op = LDAP_MOD_ADD;
                    attr2.mod_type = "IBMattributeTypes";
                    attr2.mod_values = vals2;
                    int rc = ldap_modify_s( m_ldap, "cn=schema", attrs );

                    if( rc == LDAP_TYPE_OR_VALUE_EXISTS )
                    {
                        LG_EAIDX_D << "Warning adding new attributeType:" << ldapk << nl
                                   << " vals1str:" << vals1str   << nl
                                   << " vals2str:" << vals2str   << nl
                                   << " LDAP_TYPE_OR_VALUE_EXISTS" << endl;
                    }
                    else if( rc != LDAP_SUCCESS )
                    {
                        char* es = ldap_err2string(rc);

                        LG_EAIDX_D << "ERROR: adding new attributeType:" << ldapk << nl
                                   << " vals1str:" << vals1str   << nl
                                   << " vals2str:" << vals2str   << nl
                                   << " error value:" << rc << " error string:" << es << endl;

                        fh_stringstream ss;
                        ss << "Error adding new attributeType:" << ldapk << nl
                           << " error value:" << rc << " error string:" << es;
                        Throw_EAIndexException( tostr(ss), 0 );
                    }

                    // Add the new attribute to the objectclass we are using.
                    {
                        string ferrisContextOID = "2.5.18.1.2.5.18.1.2.5.18.100.99.99";
                        fh_stringstream vals1ss;
                        vals1ss << "( " << ferrisContextOID  << nl
                                << " NAME 'ferris-context' " << nl
                                << " DESC 'DO NOT EDIT: AUTO UPDATED by indexing engine.' " << nl
                                << " SUP ( container $ document ) "             << nl
                                << " AUXILIARY "             
                                << " MUST objectClass "      << nl
                                << " MAY ( "                 << nl
                                << " ferris-ea-dbl $ ferris-ea-int $ ferris-ea-string ";
                            
                        m_ferrisContextAttributes.insert( ldapk );
                        for( stringset_t::const_iterator ci = m_ferrisContextAttributes.begin();
                             ci != m_ferrisContextAttributes.end(); ++ci )
                        {
                            vals1ss << " $ " << *ci;
                        }
                        vals1ss << " ) )";
                            
                        LG_EAIDX_D << "ObjectClass update:" << tostr( vals1ss ) << endl;
                            
                        string vals1str = tostr( vals1ss );
                        char* vals1[] = { (char*)vals1str.c_str(), 0 };
                        LDAPMod attr1;
                        LDAPMod* attrs[] = { &attr1, 0 };
                        attr1.mod_op = LDAP_MOD_REPLACE;
                        attr1.mod_type = "objectClasses";
                        attr1.mod_values = vals1;
                        rc = ldap_modify_s( m_ldap, "cn=schema", attrs );

                        if( rc == LDAP_SUCCESS )
                        {
                            LG_EAIDX_D << "Objectclass update success." << endl << endl;
                        }
                        if( rc != LDAP_SUCCESS )
                        {
                            char* es = ldap_err2string(rc);

                            fh_stringstream ss;
                            ss << "ERROR: updating the 'ferris-context' ObjectClass" << nl
                               << " attempting to add new attribute to MAYBE clause." << nl
                               << " attribute:" << ldapk
                               << " error value:" << rc << " error string:" << es;
                            LG_EAIDX_W << tostr(ss) << endl;
                            Throw_EAIndexException( tostr(ss), 0 );
                        }
                            
                    }
                }
                catch( exception& e )
                {
                    LG_EAIDX_D << "Adding LDAP schema EXCEPTION e:" << e.what() << endl;
                }
            }
        }
        
        
        void
        EAIndexerLDAP::singleWriteUpdateForAttributes( fh_context c,
                                                       fh_docindexer di,
                                                       const std::string& dn,
                                                       AttributeValues_t& AttrubuteValues )
        {
            for( AttributeValues_t::const_iterator ai = AttrubuteValues.begin();
                 ai != AttrubuteValues.end(); ++ai )
            {
                string attributeName = ai->first;
                string ldapk = ai->first;
                string     v = ai->second.v;
                string   cop = ai->second.cop;

                try
                {
                    LG_EAIDX_D << "addContextToIndex(single setting) c:" << c->getURL() << endl
                               << " ldapk:" << ldapk << endl
                               << " v:" << v
                               << " cop:" << cop
                               << endl;

                    setLDAPValue( dn, ldapk, v );

                    LG_EAIDX_D << "addContextToIndex(single DONE setting) k:" << ldapk << " to v:" << v << endl;
                }
                catch( exception& e )
                {
                    LG_EAIDX_W << "Updating single attribute k:" << ldapk
                               << " EXCEPTION e:" << e.what() << endl;
                }
            }
            
        }
        
        
        void
        EAIndexerLDAP::addToIndex( fh_context c, fh_docindexer di )
        {
            string       earl  = c->getURL();
            const string dn    = dnFromURL( earl );
            
            int attributesDone = 0;
            int signalWindow   = 0;

            Time::Benchmark bm( "earl:" + earl );
            bm.start();
            
            LG_EAIDX_D << "EAIndexerLDAP::addToIndex() earl:" << earl << endl;
            long docid  = obtainDocumentID( c );
            LG_EAIDX_D << "EAIndexerLDAP::addToIndex() docid:" << docid << endl;

            /**
             * Read all the attribute values and the comparison operator
             * for the attribute into a cache.
             */
            AttributeValues_t AttrubuteValues;
            readIndexableAttrbiutes( c, di, dn, AttrubuteValues );
            int totalAttributes = AttrubuteValues.size();
            
            
            LG_EAIDX_D << "EAIndexerLDAP::addToIndex() earl:" << earl
                       << " Attributes.size:" << AttrubuteValues.size()
                       << endl;

            /**
             * Now that we have read all the attributes we can pick out
             * the ones that already have a LDAP schema and add them all
             * in one ldap_modify_s() call. The attributes which don't
             * have an LDAP schema will be added one at a time after this
             * bulk add.
             */
            attemptBulkLDAPWrite( c, di, dn, AttrubuteValues );

            /**
             * For the remaining attributes, make sure there is a schema
             * in the LDAP server for them and then add them singularly.
             */
            modifySchemaForAttributes( c, di, dn, AttrubuteValues );
            singleWriteUpdateForAttributes( c, di, dn, AttrubuteValues );
        }


        void
        EAIndexerLDAP::BuildQuery( fh_context q,
                                   docNumSet_t& output,
                                   fh_eaquery qobj,
                                   fh_stringstream& ldap_query )
        {
//            ldap_query << "(objectclass=ferris-context)";

            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase( token );
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();

            LG_EAIDX_D << "ExecuteQuery() token:" << tokenfc << endl;
            LG_EAIDX_D << " q.child count:" << q->SubContextCount() << endl;

            string s;
            getline( orderedtls, s );
            LG_EAIDX_D << "ExecuteQuery() lc:" << s << endl;
            fh_context lc = q->getSubContext( s );

            if( tokenfc == "!" )
            {
                ldap_query << "(!";
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BuildQuery( *ci, output, qobj, ldap_query );
                }
                ldap_query << ")";
                return;
            }

            getline( orderedtls, s );
            LG_EAIDX_D << "ExecuteQuery() rc:" << s << endl;
            fh_context rc = q->getSubContext( s );            

            if( tokenfc == "&" )
            {
                ldap_query << "(&";
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BuildQuery( *ci, output, qobj, ldap_query );
                }
                ldap_query << ")";
                return;
            }
            else if( tokenfc == "|" )
            {
                ldap_query << "(|";
                for( Context::iterator ci = q->begin(); ci != q->end(); ++ci )
                {
                    BuildQuery( *ci, output, qobj, ldap_query );
                }
                ldap_query << ")";
                return;
            }

            string k      = getStrAttr( lc, "token", "" );
            string v      = getStrAttr( rc, "token", "" );
            string cop    = guessComparisonOperatorFromData( v );
            string ldapk  = toLDAPAttributeName( k, cop );

            stringlist_t numOPCodes;
            numOPCodes.push_back( "int" );
            numOPCodes.push_back( "double" );
            stringlist_t strOPCodes;
            strOPCodes.push_back( "string" );
            typedef stringlist_t::const_iterator opcodeiter_t;
            
            if( tokenfc == "==" )
            {
                ldap_query << "(" << ldapk << "=" << v << ")";
            }
            else if( tokenfc == "=~" )
            {
                ldap_query << "(" << ldapk << "~=" << v << ")";
            }
            else if( tokenfc == "=?=" )
            {
                for( opcodeiter_t si = numOPCodes.begin(); si!=numOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si )    << "="
                               << convertStringToInteger(v) << ")";
                
                for( opcodeiter_t si = strOPCodes.begin(); si!=strOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si )    << "=" << v << ")";
            }
            else if( tokenfc == "<=" || tokenfc == ">=" )
            {
                ldap_query << "(" << ldapk << tokenfc << v << ")";
            }
            else if( tokenfc == "<?=" )
            {
                for( opcodeiter_t si = numOPCodes.begin(); si!=numOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si ) << "<="
                               << convertStringToInteger(v) << ")";
                
                for( opcodeiter_t si = strOPCodes.begin(); si!=strOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si ) << "<=" << v << ")";
            }
            else if( tokenfc == ">?=" )
            {
                for( opcodeiter_t si = numOPCodes.begin(); si!=numOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si ) << ">="
                               << convertStringToInteger(v) << ")";
                for( opcodeiter_t si = strOPCodes.begin(); si!=strOPCodes.end(); ++si )
                    ldap_query << "(" << toLDAPAttributeName( k, *si ) << ">=" << v << ")";
            }
            else
            {
                LG_EAIDX_W << "WARNING: LDAP index can not resolve operation:" << tokenfc << endl;
            }
        }
        
        docNumSet_t&
        EAIndexerLDAP::ExecuteQuery( fh_context q,
                                     docNumSet_t& output,
                                     fh_eaquery qobj,
                                     int limit )
        {
            if( limit )
            {
                stringstream ss;
                ss << "EAIndexerLDAP::ExecuteQuery() limit not currently supported for LDAP backend" << endl;
                cerr << tostr(ss);
                LG_EAIDX_I << tostr(ss);
            }
            
            fh_stringstream ldap_query;

            BuildQuery( q, output, qobj, ldap_query );
            
            LDAPMessage* result = 0;
            LDAP* ld = getLDAP();

            char* attrs[ 3 ] = { "", "", 0 };
            attrs[0] = (char*)LDAP_DOCID_ATTR;
            attrs[1] = (char*)LDAP_URL_ATTR;

            LG_EAIDX_D << "ExecuteQuery() base:" << getBaseDN()
                 << " query:" << tostr( ldap_query )
                 << endl;
            int rc = ldap_search_s( ld,
                                    getBaseDN().c_str(),
                                    LDAP_SCOPE_SUBTREE,
                                    tostr( ldap_query ).c_str(),
                                    attrs,
                                    0,
                                    &result );

            if( rc != LDAP_SUCCESS )
            {
                char* es = ldap_err2string(rc);
                if( result ) ldap_msgfree( result );
                fh_stringstream ss;
                ss << "Can not query LDAP database for matching files." << endl
                   << " basedn:" << getBaseDN()
                   << " e:" << es;
                cerr << tostr(ss) << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }


            for( LDAPMessage* m = ldap_first_entry( ld, result );
                 m; m = ldap_next_entry( ld, m ) )
            {
                string mdn = ldap_get_dn( ld, m );

                LG_EAIDX_D << "got ldap entry..." << endl;
                
                BerElement* berptr = 0;

                string docid = "";
                string url   = "";
                
                for( char* ea = ldap_first_attribute( ld, m, &berptr );
                     ea; ea = ldap_next_attribute( ld, m, berptr ) )
                {
                    LG_EAIDX_D << "got ldap entry...ea:" << ea << endl;

                    if( !strcmp( ea, LDAP_DOCID_ATTR ) )
                        docid = getA( ld, m, ea );
                    if( !strcmp( ea, LDAP_URL_ATTR ) )
                        url = getA( ld, m, ea );
                        
                    ldap_memfree( ea ); 
                }
                ber_free( berptr, 0 );

                if( !url.empty() && !docid.empty() )
                {
                    LG_EAIDX_D << " found matching document id:" << docid
                               << " url:" << url
                               << endl;

                    int id = toint( docid );
                    m_docIDHash[ id ] = url;
                    addDocID( output, id );
                }
                
            }
            if( result ) ldap_msgfree( result );
                
            return output;
        }
        
        std::string
        EAIndexerLDAP::resolveDocumentID( docid_t id )
        {
            return m_docIDHash[ id ];
        }

        
        void
        EAIndexerLDAP::cleanDocumentIDCache()
        {
            m_docIDHash.clear();
        }
        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::EAIndex::MetaEAIndexerInterface* Create()
    {
        return new Ferris::EAIndex::EAIndexerLDAP();
    }
};


