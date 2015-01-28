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

    $Id: FerrisGPGContext_private.hh,v 1.5 2010/09/24 21:30:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_GPG_CONTEXT_PRIVH_
#define _ALREADY_INCLUDED_FERRIS_GPG_CONTEXT_PRIVH_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisGPG_private.hh>

namespace Ferris
{
    class FerrisGPGAllKeysDirectoryContext;
    FERRIS_CTX_SMARTPTR( FerrisGPGAllKeysDirectoryContext, fh_gpgallkeysContext );
    
    /**
     * Root context for gpg://
     */
    class FERRISEXP_DLLLOCAL FerrisGPGRootContext
        :
        public StateLessEAHolder< FerrisGPGRootContext, FakeInternalContext >
    {
        typedef FerrisGPGRootContext                                           _Self;
        typedef StateLessEAHolder< FerrisGPGRootContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        
    public:

        FerrisGPGRootContext();
        virtual ~FerrisGPGRootContext();

        void createStateLessAttributes( bool force = false );

        gpgme_ctx_t getGPGMEContext();
    };

    /**
     * Context for showing the 'keys' directory
     *
     * Requires
     * 1) able to read the list of keys currently on the keyring on priv_read()
     * 2) able to create a new key for the keyring on createSubContext()
     */
    class FERRISEXP_DLLLOCAL FerrisGPGAllKeysDirectoryContext
        :
        public StateLessEAHolder< FerrisGPGAllKeysDirectoryContext, FakeInternalContext >
    {
        typedef FerrisGPGAllKeysDirectoryContext _Self;
        typedef StateLessEAHolder< FerrisGPGAllKeysDirectoryContext, FakeInternalContext > _Base;

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
    protected:

        virtual void priv_read();
        virtual std::string priv_getMimeType( bool fromContent = false )
            { return MIMETYPE_DIRECTORY; }

    public:

        FerrisGPGAllKeysDirectoryContext( Context* parent, const std::string& rdn );
        virtual ~FerrisGPGAllKeysDirectoryContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );
        _Self* priv_CreateContext( Context* parent, std::string rdn );

        gpgme_ctx_t getGPGMEContext();
        
    };


    /**
     * Context for showing links for each userid property to the key that it belongs to
     *
     */
    class FERRISEXP_DLLLOCAL FerrisGPGUserIDToKeyContext
        :
        public StateLessEAHolder< FerrisGPGUserIDToKeyContext, FakeInternalContext >
    {
        typedef FerrisGPGUserIDToKeyContext _Self;
        typedef StateLessEAHolder< FerrisGPGUserIDToKeyContext, FakeInternalContext > _Base;

        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
    protected:

        virtual void priv_read();
        virtual std::string priv_getMimeType( bool fromContent = false )
            { return MIMETYPE_DIRECTORY; }

        fh_gpgallkeysContext m_allkc;
        
    public:

        FerrisGPGUserIDToKeyContext( Context* parent, const std::string& rdn, fh_gpgallkeysContext allkc );
        virtual ~FerrisGPGUserIDToKeyContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );
        _Self* priv_CreateContext( Context* parent, std::string rdn );

        gpgme_ctx_t getGPGMEContext();
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    /**
     */
    class FERRISEXP_DLLLOCAL FerrisGPGKeyContext
        :
        public StateLessEAHolder< FerrisGPGKeyContext, FakeInternalContext >
    {
        friend class FerrisGPGRootContext;
        
        typedef FerrisGPGKeyContext                                   _Self;
        typedef StateLessEAHolder< FerrisGPGKeyContext, FakeInternalContext > _Base;

        gpgme_key_t m_key;
        
    protected:

        virtual void priv_read();
        virtual std::string priv_getMimeType( bool fromContent = false )
            { return MIMETYPE_DIRECTORY; }
        
    public:

        virtual gpgme_key_t    getKey();
        virtual gpgme_subkey_t getSubKey();
        
        FerrisGPGKeyContext( Context* parent, const char* rdn, gpgme_key_t key );
        virtual ~FerrisGPGKeyContext();

        virtual std::string priv_getRecommendedEA();
        static fh_stringstream SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getKeyID( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getFingerPrint( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getAlgorithm( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getKeyLength( Context* c, const std::string& rdn, EA_Atom* atom );


        static fh_stringstream SL_getKeyCreationTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getKeyExpireTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getComment( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUserIDRevoked( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getUserIDInvalid( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsSecretKey( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsPublicKey( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRevoked( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getInvalid( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getExpired( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getDisabled( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getCanEncrypt( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getCanSign( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getCanCertify( Context* c, const std::string& rdn, EA_Atom* atom );
        
        void createStateLessAttributes( bool force = false );

    };


    class FERRISEXP_DLLLOCAL FerrisGPGSubKeyContext
        :
        public FerrisGPGKeyContext
    {
        typedef FerrisGPGSubKeyContext _Self;
        typedef FerrisGPGKeyContext    _Base;
        
        gpgme_subkey_t m_subkey;
        
    protected:

        virtual void priv_read();
        virtual std::string priv_getMimeType( bool fromContent = false );

    public:

        virtual gpgme_key_t    getKey();
        virtual gpgme_subkey_t getSubKey();
        
        FerrisGPGSubKeyContext( Context* parent, const char* rdn, gpgme_subkey_t subkey );
        virtual ~FerrisGPGSubKeyContext();
        
        void createStateLessAttributes( bool force = false );
    };


    class FERRISEXP_DLLLOCAL FerrisGPGUserIDContext
        :
        public StateLessEAHolder< FerrisGPGUserIDContext, leafContext >
    {
        typedef FerrisGPGUserIDContext _Self;
        typedef StateLessEAHolder< FerrisGPGUserIDContext, leafContext > _Base;

        gpgme_key_t     m_key;
        gpgme_user_id_t m_uid;
        
    protected:

    public:

        
        FerrisGPGUserIDContext( Context* parent, const char* rdn,
                                gpgme_key_t key,
                                gpgme_user_id_t uid );
        virtual ~FerrisGPGUserIDContext();

        virtual std::string priv_getRecommendedEA();
        
        static fh_stringstream SL_getUID( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getComment( Context* c, const std::string& rdn, EA_Atom* atom );
        
        static fh_stringstream SL_getValidity( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getValidityNumeric( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getRevoked( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getInvalid( Context* c, const std::string& rdn, EA_Atom* atom );
        
        void createStateLessAttributes( bool force = false );
    };
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/


//     /**
//      * Root context for gpg-signatures://
//      */
//     class FerrisGPGSignaturesRootContext
//         :
//         public StateLessEAHolder< FerrisGPGSignaturesRootContext, FakeInternalContext >
//     {
//         typedef FerrisGPGSignaturesRootContext                                           _Self;
//         typedef StateLessEAHolder< FerrisGPGSignaturesRootContext, FakeInternalContext > _Base;
        
//         virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

//     protected:

//         virtual void priv_read();
        
//     public:

//         FerrisGPGSignaturesRootContext();
//         virtual ~FerrisGPGSignaturesRootContext();

//         void createStateLessAttributes( bool force = false );

//         gpgme_ctx_t getGPGMEContext();
//     };
    

//     /**
//      * Context to mirror the underlying filesystem and carry the base fh_context pointer
//      * down to the file level in the layer that is sitting above the normal filesystem.
//      *
//      * eg. gpg-signatures://file:///tmp will have a Delegate of /tmp
//      *
//      */
//     class FerrisGPGSignaturesInternalContext
//         :
//         public ChainedViewContext
//     {
//         typedef FerrisGPGSignaturesInternalContext _Self;
//         typedef ChainedViewContext                 _Base;
//         typedef Context                            _DontDelegateBase;

//     protected:

//         virtual void UnPageSubContextsIfNeeded();
//         virtual std::string private_getStrAttr( const std::string& rdn,
//                                                 const std::string& def = "",
//                                                 bool getAllLines = false ,
//                                                 bool throwEx = false );
        
//     public:

//         FerrisGPGSignaturesInternalContext( Context* theParent, const fh_context& theDelegate );
//         FerrisGPGSignaturesInternalContext( Context* theParent, const fh_context& theDelegate,
//                                             const std::string& rdn );
//         virtual ~FerrisGPGSignaturesInternalContext();

//         virtual std::string getDirName() const;

//         stringlist_t& getForceLocalAttributeNames();
        
//         virtual void OnDeleted( NamingEvent_Deleted* ev, std::string olddn, std::string newdn );
//         virtual void OnExists ( NamingEvent_Exists* ev,  std::string olddn, std::string newdn );
//         virtual void OnCreated( NamingEvent_Created* ev, std::string olddn, std::string newdn );
        
//         virtual void read( bool force = 0 );
//         virtual long guessSize() throw();
        

//         void createStateLessAttributes( bool force = false );
//         _Self* priv_CreateContext( Context* parent, std::string rdn );
//         virtual fh_context priv_getSubContext( const std::string& rdn )
//             throw( NoSuchSubContext );

//         gpgme_ctx_t getGPGMEContext();
        
//     };

    /**
     * Context to show an individual signature for the collection of signatures in a signed item
     */
    class FERRISEXP_DLLLOCAL FerrisGPGSignatureContext
        :
        public StateLessEAHolder< FerrisGPGSignatureContext, FakeInternalContext >
    {
        typedef FerrisGPGSignatureContext _Self;
        typedef StateLessEAHolder< FerrisGPGSignatureContext, FakeInternalContext > _Base;

        gpgme_sigsum_t summary;
        std::string fpr;
        gpgme_error_t status;
        time_t timestamp;
        time_t exp_timestamp;
        int wrong_key_usage : 1;
        gpgme_validity_t validity;
        gpgme_error_t validity_reason;
        
    protected:

        virtual void priv_read();

    public:

        void constructObject( gpgme_signature_t sig );
        FerrisGPGSignatureContext( Context* parent, const std::string& rdn );
        virtual ~FerrisGPGSignatureContext();

        virtual std::string priv_getRecommendedEA();

        static fh_stringstream SL_getDesc( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsValid( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsGreen( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsRed( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsKeyRevoked( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsKeyExpired( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsSigExpired( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsKeyMissing( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsCRLMissing( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsCRLToOld( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getIsBadPolicy( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getFingerPrint( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getStatus( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getSigCreationTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getSigExpireTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getSigHasExpireTime( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getWrongKeyUsage( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getValidity( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getValidityNumeric( Context* c, const std::string& rdn, EA_Atom* atom );


        // we return the context of the key in libferris as ->second so that it is not
        // reclaimed by the memory manager and thus ->first is well defined as long
        // as the pair is in scope.
        std::pair< gpgme_key_t, fh_context > getKey();
        static fh_stringstream SL_getPersonName( Context* c, const std::string& rdn, EA_Atom* atom );
        static fh_stringstream SL_getEmailAddress( Context* c, const std::string& rdn, EA_Atom* atom );
        
        
        void createStateLessAttributes( bool force = false );
    };
    
    
};
#endif
