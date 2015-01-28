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

    $Id: FerrisGPGContext.cpp,v 1.5 2010/09/24 21:30:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisGPGContext_private.hh>
#include <BranchContext_private.hh>
#include <FerrisGPG_private.hh>
#include <Resolver_private.hh>
#include <General.hh>
#include <SchemaSupport.hh>
#include <Ferris.hh>
#include <Ferris/Context_private.hh> // VirtualSoftlinkContext

using namespace std;

namespace Ferris
{
#define DUBCORE_DESCRIPTION "dc:description"

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    static std::string ValidityToString( gpgme_validity_t v )
    {
        switch( v )
        {
        case GPGME_VALIDITY_UNKNOWN   : return( "unknown" );
        case GPGME_VALIDITY_UNDEFINED : return( "undefined" );
        case GPGME_VALIDITY_NEVER     : return( "never" );
        case GPGME_VALIDITY_MARGINAL  : return( "marginal" );
        case GPGME_VALIDITY_FULL      : return( "full" );
        case GPGME_VALIDITY_ULTIMATE  : return( "ultimate" );
        }
        return( "unknown-gpgme" );
    }
                                                    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    gpgme_ctx_t
    FerrisGPGRootContext::getGPGMEContext()
    {
        static GPGMEContextHolder obj;
        return obj;
    }
    
    void
    FerrisGPGRootContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }


    void
    FerrisGPGRootContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            fh_gpgallkeysContext allkeysc = 0;
            fh_context child = 0;

            allkeysc = new FerrisGPGAllKeysDirectoryContext( this, "keys" );
            addNewChild( allkeysc );

            child = new FerrisGPGUserIDToKeyContext( this, "by-uid", allkeysc );
            addNewChild( child );

            child = new FerrisGPGUserIDToKeyContext( this, "by-name", allkeysc );
            addNewChild( child );

            child = new FerrisGPGUserIDToKeyContext( this, "by-email", allkeysc );
            addNewChild( child );

            child = new FerrisGPGUserIDToKeyContext( this, "by-comment", allkeysc );
            addNewChild( child );

        }
    }
    
    FerrisGPGRootContext::FerrisGPGRootContext()
        :
        _Base( 0, "/" )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGRootContext::~FerrisGPGRootContext()
    {
    }
    
    void
    FerrisGPGRootContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    class FERRISEXP_DLLLOCAL FerrisGPGRootContext_RootContextDropper
        :
        public RootContextDropper
    {
    public:
        FerrisGPGRootContext_RootContextDropper()
            {
                RootContextFactory::Register( "gpg", this );
            }

        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
            {
                static fh_context c = 0;
                if( !isBound(c) )
                {
                    c = new FerrisGPGRootContext();
                }
                return c;
            }
    };
    static FerrisGPGRootContext_RootContextDropper ___FerrisGPGRootContext_static_init;

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    gpgme_ctx_t
    FerrisGPGAllKeysDirectoryContext::getGPGMEContext()
    {
        if( FerrisGPGRootContext* cc = dynamic_cast<FerrisGPGRootContext*>( getParent() ))
        {
            return cc->getGPGMEContext();
        }
        return 0;
    }
    
    
    void
    FerrisGPGAllKeysDirectoryContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }

    FerrisGPGAllKeysDirectoryContext::FerrisGPGAllKeysDirectoryContext( Context* parent,
                                                                  const std::string& rdn )
        :
        _Base( parent, rdn )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGAllKeysDirectoryContext::~FerrisGPGAllKeysDirectoryContext()
    {
    }

    fh_context
    FerrisGPGAllKeysDirectoryContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "gpg://keys need to implement key creation method! FIXME" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    

    void
    FerrisGPGAllKeysDirectoryContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    
    void
    FerrisGPGAllKeysDirectoryContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            fh_context    child = 0;

            const char* PATTERN = 0;
            int SECRET_ONLY = 0;
            gpgme_error_t  e = GPG_ERR_NO_ERROR;
            gpgme_ctx_t gctx = getGPGMEContext();
            e = gpgme_op_keylist_start( gctx, PATTERN, SECRET_ONLY );

            if( e != GPG_ERR_NO_ERROR )
            {
                fh_stringstream ss;
                ss << "Error starting to read the keylist";
                ss << "GPGME error:" << gpgme_strerror (e) << endl;
                Throw_KeylistException( tostr(ss), 0 );
            }

            gpgme_key_t key;
            while (!e)
            {
                e = gpgme_op_keylist_next (gctx, &key);
                if (e)
                    break;
                const char* rdn = key->subkeys->keyid;
//                const char* rdn = key->uids->email;
//                cerr << "FerrisGPGRootContext(list) rdn:" << rdn << endl;
             
                fh_context child = new FerrisGPGKeyContext( this, rdn, key );
                addNewChild( child );
//                cerr << "FerrisGPGRootContext(list) keyid:" << getStrAttr( child, "keyid", "none" ) << endl;
            }

            if( gpgme_err_code(e) != GPG_ERR_EOF )
            {
                fh_stringstream ss;
                ss << "Error reading the keylist" << endl;
                ss << "GPGME e:" << e << " GPG_ERR_EOF:" << GPG_ERR_EOF << " error:" << gpgme_strerror (e) << endl;
                Throw_KeylistException( tostr(ss), 0 );
            }
        }
    }

    FerrisGPGAllKeysDirectoryContext*
    FerrisGPGAllKeysDirectoryContext::priv_CreateContext( Context* parent, std::string rdn )
    {
        return 0;
    }
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    gpgme_ctx_t
    FerrisGPGUserIDToKeyContext::getGPGMEContext()
    {
        if( FerrisGPGRootContext* cc = dynamic_cast<FerrisGPGRootContext*>( getParent() ))
        {
            return cc->getGPGMEContext();
        }
        return 0;
    }
    
    
    void
    FerrisGPGUserIDToKeyContext::priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    {
    }

    FerrisGPGUserIDToKeyContext::FerrisGPGUserIDToKeyContext( Context* parent,
                                                                        const std::string& rdn,
                                                                        fh_gpgallkeysContext allkc )
        :
        _Base( parent, rdn ),
        m_allkc( allkc )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGUserIDToKeyContext::~FerrisGPGUserIDToKeyContext()
    {
    }

    fh_context
    FerrisGPGUserIDToKeyContext::createSubContext( const std::string& rdn, fh_context md )
        throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported )
    {
        fh_stringstream ss;
        ss << "gpg://keys need to implement key creation method! FIXME" << endl;
        Throw_FerrisCreateSubContextNotSupported( tostr(ss), this );
    }
    

    void
    FerrisGPGUserIDToKeyContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    
    void
    FerrisGPGUserIDToKeyContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            m_allkc->read( false );
            for( Context::iterator ci = m_allkc->begin(); ci != m_allkc->end(); ++ci )
            {
                if( FerrisGPGKeyContext* cc = dynamic_cast< FerrisGPGKeyContext* >( GetImpl( *ci )))
                {
                    gpgme_key_t        k = cc->getKey();
                    gpgme_user_id_t  uid = k->uids;
                    while( uid )
                    {
                        string rdn = uid->uid;

                        if( getDirName() == "by-uid" && uid->uid )
                        {
                            rdn = uid->uid;
                        }
                        else if( getDirName() == "by-name" && uid->name )
                        {
                            rdn = uid->name;
                        }
                        else if( getDirName() == "by-email" && uid->email )
                        {
                            rdn = uid->email;
                        }
                        else if( getDirName() == "by-comment" && uid->comment )
                        {
                            rdn = uid->comment;
                        }

                        fh_context target = *ci;

                        LG_CTX_D << "FerrisGPGUserIDToKeyContext::priv_read(1) "
                                 << " target:" << target->getURL()
                                 << " rdn:" << rdn
                                 << endl;
                        
                        fh_context child = new VirtualSoftlinkContext( this, target, rdn );

                        LG_CTX_D << "FerrisGPGUserIDToKeyContext::priv_read(2) "
                                 << " target:" << target->getURL()
                                 << " rdn:" << rdn << endl;
                        LG_CTX_D << " child.rdn:" << child->getDirName()
                                 << " child.url:" << child->getURL()
                                 << endl;
                        
                        addNewChild( child );

                        uid = uid->next;
                    }
                }
            }

            for( Context::iterator ci = begin(); ci != end(); ++ci )
            {
                fh_context c = *ci;

                LG_CTX_D << "FerrisGPGUserIDToKeyContext::priv_read(dump) c.rdn:" << c->getDirName()
                         << " EA(rdn):" << getStrAttr( c, "name", "n/a" )
                         << " c.url:" << c->getURL()
                         << endl;
                
            }
            
        }
    }

    FerrisGPGUserIDToKeyContext*
    FerrisGPGUserIDToKeyContext::priv_CreateContext( Context* parent, std::string rdn )
    {
        return 0;
    }
    
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    FerrisGPGKeyContext::FerrisGPGKeyContext( Context* parent,
                                              const char* rdn,
                                              gpgme_key_t key )
        :
        _Base( parent, rdn ),
        m_key( key )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGKeyContext::~FerrisGPGKeyContext()
    {
        if( m_key )
            gpgme_key_release( m_key );
    }

    std::string
    FerrisGPGKeyContext::priv_getRecommendedEA()
    {
        static string rea = "email,keyid,person-name,creation-time-display,expire-time-display";
        return rea;
    }
    
    fh_stringstream
    FerrisGPGKeyContext::SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "A GPG public or private key";
        return ss;
    }

    fh_stringstream
    FerrisGPGKeyContext::SL_getKeyID( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            LG_CTX_D << "FerrisGPGKeyContext::SL_getKeyID() cc:" << toVoid( cc )
                     << " m_key:" << toVoid( cc->m_key )
                     << endl;
            ss << cc->getSubKey()->keyid;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getKey()->uids->name;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getKey()->uids->email;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getFingerPrint( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            if( cc->getSubKey()->fpr )
                ss << cc->getSubKey()->fpr;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getAlgorithm( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << gpgme_pubkey_algo_name( cc->getSubKey()->pubkey_algo );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getKeyLength( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->length;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getKeyCreationTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->timestamp;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getKeyExpireTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->expires;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getComment( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getKey()->uids->comment;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getUserIDRevoked( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getKey()->uids->revoked;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getUserIDInvalid( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getKey()->uids->invalid;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getIsSecretKey( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << ( !!cc->getSubKey()->secret );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getIsPublicKey( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << ( !cc->getSubKey()->secret );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getRevoked( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->revoked;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getInvalid( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->invalid;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getExpired( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->expired;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getDisabled( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->disabled;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getCanEncrypt( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->can_encrypt;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getCanSign( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->can_sign;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGKeyContext::SL_getCanCertify( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>(c))
        {
            ss << cc->getSubKey()->can_certify;
        }
        return ss;
    }

    gpgme_key_t
    FerrisGPGKeyContext::getKey()
    {
        return m_key;
    }
    
    gpgme_subkey_t
    FerrisGPGKeyContext::getSubKey()
    {
        return m_key->subkeys;
    }
    

    void
    FerrisGPGKeyContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA tryAddStateLessAttribute
            SLEA( DUBCORE_DESCRIPTION, &_Self::SL_getDesc,   XSD_BASIC_STRING );
            SLEA( "keyid",         &_Self::SL_getKeyID,        XSD_BASIC_STRING );
            SLEA( "person-name",   &_Self::SL_getPersonName,   XSD_BASIC_STRING );
            SLEA( "email",         &_Self::SL_getEmailAddress, XSD_BASIC_STRING );
            SLEA( "fingerprint",   &_Self::SL_getFingerPrint,  XSD_BASIC_STRING );
            SLEA( "algorithm",     &_Self::SL_getAlgorithm,    XSD_BASIC_STRING );
            SLEA( "key-length",    &_Self::SL_getKeyLength,    XSD_BASIC_INT );
            SLEA( "ctime",         &_Self::SL_getKeyCreationTime, FXD_UNIXEPOCH_T );
            SLEA( "creation-time", &_Self::SL_getKeyCreationTime, FXD_UNIXEPOCH_T );
            SLEA( "expire-time",   &_Self::SL_getKeyExpireTime,   FXD_UNIXEPOCH_T );
            SLEA( "comment",       &_Self::SL_getComment,         XSD_BASIC_STRING );
            SLEA( "userid-revoked",&_Self::SL_getUserIDRevoked,   XSD_BASIC_BOOL );
            SLEA( "userid-invalid",&_Self::SL_getUserIDInvalid,   XSD_BASIC_BOOL );
            SLEA( "is-secret",     &_Self::SL_getIsSecretKey,     XSD_BASIC_BOOL );
            SLEA( "is-public",     &_Self::SL_getIsPublicKey,     XSD_BASIC_BOOL );
            SLEA( "revoked",       &_Self::SL_getRevoked,         XSD_BASIC_BOOL );
            SLEA( "invalid",       &_Self::SL_getInvalid,         XSD_BASIC_BOOL );
            SLEA( "expired",       &_Self::SL_getExpired,         XSD_BASIC_BOOL );
            SLEA( "disabled",      &_Self::SL_getDisabled,        XSD_BASIC_BOOL );
            SLEA( "can-encrypt",   &_Self::SL_getCanEncrypt,      XSD_BASIC_BOOL );
            SLEA( "can-sign",      &_Self::SL_getCanSign,         XSD_BASIC_BOOL );
            SLEA( "can-certify",   &_Self::SL_getCanCertify,      XSD_BASIC_BOOL );
#undef SLEA
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    void
    FerrisGPGKeyContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            gpgme_subkey_t skey = m_key->subkeys;
//             if( skey )
//                 skey = skey->next;

            while( skey )
            {
                const char* rdn = skey->keyid;
             
                fh_context child = new FerrisGPGSubKeyContext( this, rdn, skey );
                addNewChild( child );
                
                skey = skey->next;
            }

            gpgme_user_id_t uid = m_key->uids;
            while( uid )
            {
                const char* rdn = uid->uid;

                fh_fcontext userid = new FakeInternalContext( this, "userids" );
                addNewChild( userid );
                
                fh_context child = new FerrisGPGUserIDContext( this, rdn, getKey(), uid );
                userid->addNewChild( child );
                
                uid = uid->next;
            }
        }
    }
            
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    gpgme_key_t
    FerrisGPGSubKeyContext::getKey()
    {
        if( FerrisGPGKeyContext* cc = dynamic_cast<FerrisGPGKeyContext*>( getParent() ))
        {
            return cc->getKey();
        }
        
        return 0;
    }
    
    gpgme_subkey_t
    FerrisGPGSubKeyContext::getSubKey()
    {
        return m_subkey;
    }
    
    void
    FerrisGPGSubKeyContext::priv_read()
    {
        emitExistsEventForEachItemRAII _raii1( this );
    }
    
    std::string
    FerrisGPGSubKeyContext::priv_getMimeType( bool fromContent )
    {
        return "application/gpgkey";
    }

    FerrisGPGSubKeyContext::FerrisGPGSubKeyContext( Context* parent,
                                                    const char* rdn,
                                                    gpgme_subkey_t subkey )
        :
        _Base( parent, rdn, 0 ),
        m_subkey( subkey )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGSubKeyContext::~FerrisGPGSubKeyContext()
    {
    }
    
        
    void
    FerrisGPGSubKeyContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }

    /****************************************/
    /****************************************/
    /****************************************/
    /****************************************/

    
    FerrisGPGUserIDContext::FerrisGPGUserIDContext( Context* parent, const char* rdn,
                                                    gpgme_key_t key,
                                                    gpgme_user_id_t uid )
        :
        _Base( parent, rdn ),
        m_key( key ),
        m_uid( uid )
    {
        createStateLessAttributes();
    }
    
    FerrisGPGUserIDContext::~FerrisGPGUserIDContext()
    {
    }
    

    fh_stringstream
    FerrisGPGUserIDContext::SL_getUID( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->uid;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGUserIDContext::SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->name;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGUserIDContext::SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->email;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGUserIDContext::SL_getComment( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->comment;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGUserIDContext::SL_getValidity( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            gpgme_validity_t v = cc->m_uid->validity;
            ss << ValidityToString( v );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGUserIDContext::SL_getValidityNumeric( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->validity;
        }
        return ss;
    }

    fh_stringstream
    FerrisGPGUserIDContext::SL_getRevoked( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->revoked;
        }
        return ss;
    }

    fh_stringstream
    FerrisGPGUserIDContext::SL_getInvalid( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGUserIDContext* cc = dynamic_cast<FerrisGPGUserIDContext*>(c))
        {
            ss << cc->m_uid->invalid;
        }
        return ss;
    }

    std::string
    FerrisGPGUserIDContext::priv_getRecommendedEA()
    {
        return "person-name,comment,email,revoked,invalid,validity";
    }
    
    
    void
    FerrisGPGUserIDContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA tryAddStateLessAttribute

            SLEA( "comment",       &_Self::SL_getUID,          XSD_BASIC_STRING );
            SLEA( "person-name",   &_Self::SL_getPersonName,   XSD_BASIC_STRING );
            SLEA( "email",         &_Self::SL_getEmailAddress, XSD_BASIC_STRING );
            SLEA( "comment",       &_Self::SL_getComment,      XSD_BASIC_STRING );

            SLEA( "validity",         &_Self::SL_getValidity,         XSD_BASIC_STRING );
            SLEA( "validity-numeric", &_Self::SL_getValidityNumeric,  XSD_BASIC_INT );
            SLEA( "revoked",          &_Self::SL_getRevoked,          XSD_BASIC_BOOL );
            SLEA( "invalid",          &_Self::SL_getInvalid,          XSD_BASIC_BOOL );

#undef SLEA
            
            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


    class FERRISEXP_DLLLOCAL FerrisGPGSignaturesInternalContext
        :
        public FerrisBranchInternalContext
    {
        typedef FerrisGPGSignaturesInternalContext _Self;
        typedef FerrisBranchInternalContext        _Base;

    protected:

        virtual void priv_read_leaf()
            {
                EnsureStartStopReadingIsFiredRAII _raii( this );
                
                LG_CTX_D << "FerrisGPGSignaturesInternalContext::read() is-file url:"
                         << getURL()
                         << " this:" << toVoid( dynamic_cast<Context*>(this) )
                         << " delegate:" << toVoid( GetImpl(Delegate) )
                         << endl;

                GPGMEContextHolder gctx;
                fh_istream dss = Delegate->getIStream( ios::in | ferris_ios::o_nouncrypt );
                GPGMEDataHolder inputdata( getGPGData( &dss ) );
                GPGMEDataHolder d;
                GPGMEDataHolder plaintext;

                LG_CTX_D << "FerrisGPGSignaturesInternalContext::read()2 is-file url:"
                         << getURL() << endl;
            
                gpgme_error_t e = gpgme_op_verify( gctx, inputdata, 0, plaintext );

                LG_CTX_D << "FerrisGPGSignaturesInternalContext::read()3 is-file url:"
                         << getURL() << endl;
            
                if( e == GPG_ERR_NO_ERROR )
                {
                    EnsureStartStopReadingIsFiredRAII _raii1( this );
                    
                    gpgme_verify_result_t result = gpgme_op_verify_result( gctx );
                    LG_CTX_D << "FerrisGPGSignaturesInternalContext::read()4 is-file url:"
                             << getURL() << endl;
                
                    gpgme_signature_t sig = result->signatures;

                    while( sig )
                    {
                        string rdn = sig->fpr;

                        FerrisGPGSignatureContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( sig );
                            
                        sig = sig->next;
                    }
                }
            }
        
    public:

        FerrisGPGSignaturesInternalContext( Context* theParent,
                                            const fh_context& theDelegate,
                                            const std::string& rdn )
            :
            _Base( theParent, theDelegate, rdn )
            {
                createStateLessAttributes();
            }
        
        virtual ~FerrisGPGSignaturesInternalContext()
            {
            }
    };

    FerrisBranchInternalContext*
    FerrisGPGSignaturesInternalContext_Creator( Context* ctx,
                                                const fh_context& theDelegate,
                                                const std::string& rdn )
    {
        return new FerrisGPGSignaturesInternalContext( ctx, theDelegate, rdn );
    }
    
    static bool FerrisGPGSignaturesInternalContext_Dropper =
    FerrisBranchRootContext_Register( "branchfs-gpg-signatures",
                                      BranchInternalContextCreatorFunctor_t(
                                          FerrisGPGSignaturesInternalContext_Creator ) );


    
    
    /************************************************************/
    /************************************************************/
    /************************************************************/

    void
    FerrisGPGSignatureContext::constructObject( gpgme_signature_t sig )
    {
//                         LG_CTX_D << " GPGME_SIGSUM_VALID:" << (sig->summary & GPGME_SIGSUM_VALID) << endl
//                              << " GPGME_SIGSUM_GREEN:" << (sig->summary & GPGME_SIGSUM_GREEN) << endl
//                              << " GPGME_SIGSUM_RED:"   << (sig->summary & GPGME_SIGSUM_RED)   << endl
//                              << " GPGME_SIGSUM_KEY_REVOKED:" << (sig->summary & GPGME_SIGSUM_KEY_REVOKED) << endl
//                              << " GPGME_SIGSUM_KEY_EXPIRED:" << (sig->summary & GPGME_SIGSUM_KEY_EXPIRED) << endl
//                              << " GPGME_SIGSUM_SIG_EXPIRED:" << (sig->summary & GPGME_SIGSUM_SIG_EXPIRED) << endl
//                              << " GPGME_SIGSUM_KEY_MISSING:" << (sig->summary & GPGME_SIGSUM_KEY_MISSING) << endl
//                              << " GPGME_SIGSUM_CRL_MISSING:" << (sig->summary & GPGME_SIGSUM_CRL_MISSING) << endl
//                              << " GPGME_SIGSUM_CRL_TOO_OLD:" << (sig->summary & GPGME_SIGSUM_CRL_TOO_OLD) << endl
//                              << " GPGME_SIGSUM_BAD_POLICY:"  << (sig->summary & GPGME_SIGSUM_BAD_POLICY) << endl
//                              << " GPGME_SIGSUM_SYS_ERROR:" << (sig->summary & GPGME_SIGSUM_SYS_ERROR) << endl
//                              << " fingerprint:" << (sig->fpr ? sig->fpr : "")          << endl
//                              << " status:" << sig->status                              << endl
//                              << " timestamp:" << sig->timestamp                        << endl
//                              << " timestamp:" << Time::toTimeString( sig->timestamp )  << endl
//                              << " exp_timestamp:" << sig->exp_timestamp                << endl
//                              << " exp_timestamp:" << Time::toTimeString( sig->exp_timestamp )  << endl
//                              << " wrong_key_usage" << sig->wrong_key_usage                     << endl
//                              << " validity:" << sig->validity                                  << endl
//                              << " validity-is-full:" << (sig->validity == GPGME_VALIDITY_FULL) << endl
//                              << " validity_reason:" << gpgme_strerror( sig->validity_reason )  << endl
//                              << endl;
        
        summary = sig->summary ;
        fpr = sig->fpr ? sig->fpr : "" ;
        status = sig->status ;
        timestamp = sig->timestamp ;
        exp_timestamp = sig->exp_timestamp ;
        wrong_key_usage = sig->wrong_key_usage ;
        validity = sig->validity ;
        validity_reason = sig->validity_reason ;
    }
    
    
    FerrisGPGSignatureContext::FerrisGPGSignatureContext(
        Context* parent, const std::string& rdn )
        :
        _Base( parent, rdn ),
//        summary( 0 ),
        fpr( 0 ),
        status( 0 ),
        timestamp( 0 ),
        exp_timestamp( 0 ),
        wrong_key_usage( 0 ),
//        validity( 0 ),
        validity_reason( 0 )
    {
        createStateLessAttributes();
    }
        
    FerrisGPGSignatureContext::~FerrisGPGSignatureContext()
    {
    }
    
    std::string
    FerrisGPGSignatureContext::priv_getRecommendedEA()
    {
        return "email,expire-time-display,is-valid,is-green,ctime-display,validity";
    }

    fh_stringstream
    FerrisGPGSignatureContext::SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        ss << "An individual digital signature from a collection of such "
           << "signatures made about the data in a file";
        return ss;
    }

    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsValid( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_VALID > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsGreen( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_GREEN > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsRed( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_RED > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsKeyRevoked( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_KEY_REVOKED > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsKeyExpired( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_KEY_EXPIRED > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsSigExpired( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_SIG_EXPIRED > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsKeyMissing( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_KEY_MISSING > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsCRLMissing( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_CRL_MISSING > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsCRLToOld( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_CRL_TOO_OLD > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getIsBadPolicy( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << ( cc->summary & GPGME_SIGSUM_BAD_POLICY > 0 );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getFingerPrint( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << cc->fpr;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getStatus( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << gpgme_strerror( cc->status );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getSigCreationTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << cc->timestamp;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getSigExpireTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << cc->exp_timestamp;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getSigHasExpireTime( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << (cc->exp_timestamp != 0);
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getWrongKeyUsage( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            ss << cc->wrong_key_usage;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getValidity( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            gpgme_validity_t v = cc->validity;
            ss << ValidityToString( v );
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getValidityNumeric( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            gpgme_validity_t v = cc->validity;
            ss << v;
        }
        return ss;
    }

    std::pair< gpgme_key_t, fh_context > 
    FerrisGPGSignatureContext::getKey()
    {
        fh_context c = Resolve( "gpg://keys" );
        for( Context::iterator ci = c->begin(); ci != c->end(); ++ci )
        {
            string civ = getStrAttr( *ci, "fingerprint", "" );
            if( civ == fpr )
            {
                FerrisGPGKeyContext* kc = dynamic_cast<FerrisGPGKeyContext*>( GetImpl( *ci ) );
                return make_pair( kc->getKey(), *ci );
            }
        }
        return make_pair( (gpgme_key_t)0, (Context*)0 );
        
//         gpgme_error_t  e = GPG_ERR_NO_ERROR;
//         const char* PATTERN = 0;
//         int SECRET_ONLY = 0;
//         GPGMEContextHolder gctx;
//         e = gpgme_op_keylist_start( gctx, PATTERN, SECRET_ONLY );

//         if( e != GPG_ERR_NO_ERROR )
//         {
//             fh_stringstream ss;
//             ss << "Error starting to read the keylist";
//             ss << "GPGME error:" << gpgme_strerror (e) << endl;
//             Throw_KeylistException( tostr(ss), 0 );
//         }

//         gpgme_key_t key;
//         while (!e)
//         {
//             e = gpgme_op_keylist_next (gctx, &key);
//             if (e)
//                 break;

//             gpgme_subkey_t sk = key->subkeys;
//             while( sk )
//             {
//                 if( sk->keyid == SoughtKeyID.c_str() )
//                 {
//                     return key;
//                 }
//                 sk = sk->next;
//             }
            
//         }
        
//         if( gpgme_err_code(e) != GPG_ERR_EOF )
//         {
//             fh_stringstream ss;
//             ss << "Error reading the keylist" << endl;
//             ss << "GPGME e:" << e << " GPG_ERR_EOF:" << GPG_ERR_EOF << " error:" << gpgme_strerror (e) << endl;
//             Throw_KeylistException( tostr(ss), 0 );
//         }
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            std::pair< gpgme_key_t, fh_context > p = cc->getKey();
            if( p.first )
                ss << p.first->uids->name;
        }
        return ss;
    }
    fh_stringstream
    FerrisGPGSignatureContext::SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom )
    {
        fh_stringstream ss;
        if( FerrisGPGSignatureContext* cc = dynamic_cast<FerrisGPGSignatureContext*>(c))
        {
            std::pair< gpgme_key_t, fh_context > p = cc->getKey();
            if( p.first )
                ss << p.first->uids->email;
        }
        return ss;
    }
    
    void
    FerrisGPGSignatureContext::createStateLessAttributes( bool force )
    {
        if( force || isStateLessEAVirgin() )
        {
#define SLEA tryAddStateLessAttribute
            SLEA( DUBCORE_DESCRIPTION,       &_Self::SL_getDesc,     XSD_BASIC_STRING );
            SLEA( "is-valid",                &_Self::SL_getIsValid,  XSD_BASIC_BOOL );
            SLEA( "is-green",                &_Self::SL_getIsGreen,  XSD_BASIC_BOOL );
            SLEA( "is-red",                  &_Self::SL_getIsRed,    XSD_BASIC_BOOL );
            SLEA( "key-revoked",             &_Self::SL_getIsKeyRevoked,  XSD_BASIC_BOOL );
            SLEA( "key-expired",             &_Self::SL_getIsKeyExpired,  XSD_BASIC_BOOL );
            SLEA( "sig-expired",             &_Self::SL_getIsSigExpired,  XSD_BASIC_BOOL );
            SLEA( "key-missing",             &_Self::SL_getIsKeyMissing,  XSD_BASIC_BOOL );
            SLEA( "crl-missing",             &_Self::SL_getIsCRLMissing,  XSD_BASIC_BOOL );
            SLEA( "revocation-list-missing", &_Self::SL_getIsCRLMissing, XSD_BASIC_BOOL );
            SLEA( "crl-too-old",             &_Self::SL_getIsCRLToOld,   XSD_BASIC_BOOL );
            SLEA( "revocation-list-too-old", &_Self::SL_getIsCRLToOld,   XSD_BASIC_BOOL );
            SLEA( "bad-policy",              &_Self::SL_getIsBadPolicy,  XSD_BASIC_BOOL );

            SLEA( "fingerprint",             &_Self::SL_getFingerPrint,  XSD_BASIC_STRING );
            SLEA( "status",                  &_Self::SL_getStatus,       XSD_BASIC_STRING );

            SLEA( "ctime",                   &_Self::SL_getSigCreationTime,  FXD_UNIXEPOCH_T );
            SLEA( "creation-time",           &_Self::SL_getSigCreationTime,  FXD_UNIXEPOCH_T );
            SLEA( "expire-time",             &_Self::SL_getSigExpireTime,    FXD_UNIXEPOCH_T );
            SLEA( "has-expire-time",         &_Self::SL_getSigHasExpireTime, XSD_BASIC_BOOL );

            SLEA( "wrong-key-usage",         &_Self::SL_getWrongKeyUsage,   XSD_BASIC_BOOL );
            SLEA( "validity",                &_Self::SL_getValidity,         XSD_BASIC_STRING );
            SLEA( "validity-numeric",        &_Self::SL_getValidityNumeric,  XSD_BASIC_INT );

            SLEA( "person-name",             &_Self::SL_getPersonName,   XSD_BASIC_STRING );
            SLEA( "email",                   &_Self::SL_getEmailAddress, XSD_BASIC_STRING );
            
#undef SLEA

            _Base::createStateLessAttributes( true );
            supplementStateLessAttributes( true );
        }
    }
    
    void
    FerrisGPGSignatureContext::priv_read()
    {
        staticDirContentsRAII _raii1( this );

        if( empty() )
        {
            std::pair< gpgme_key_t, fh_context > p = getKey();
            if( p.first && p.second )
            {
                fh_context child = new VirtualSoftlinkContext( this, p.second );
                addNewChild( child );
            }
        }
    }
    
    


    /************************************************************/
    /************************************************************/
    /************************************************************/

};
