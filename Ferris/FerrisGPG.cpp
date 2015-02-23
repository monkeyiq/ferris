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

    $Id: FerrisGPG.cpp,v 1.4 2010/09/24 21:30:38 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Ferris/FerrisGPG_private.hh>

using namespace std;


namespace Ferris
{
#ifdef FERRIS_HAVE_GPGME

    GPGMEContextHolder::GPGMEContextHolder( gpgme_ctx_t gctx ) : m_gctx( gctx )
    {
    }
    GPGMEContextHolder::GPGMEContextHolder()
        :
        m_gctx( 0 )
    {
        gpgme_error_t e = gpgme_new( &m_gctx );
        if( e != GPG_ERR_NO_ERROR )
        {
            fh_stringstream ss;
            ss << "Error getting a context handle for GPGME";
            ss << "GPGME error:" << gpgme_strerror (e) << endl;
            Throw_GPGMEInitFailed( tostr(ss), 0 );
        }
    }
    
    GPGMEContextHolder::~GPGMEContextHolder()
    {
//        cerr << "~GPGMEContextHolder()" << endl;
        if( m_gctx )
            gpgme_release( m_gctx );
    }
    
    gpgme_ctx_t
    GPGMEContextHolder::operator*()
    {
        return m_gctx;
    }
    
    gpgme_ctx_t getGPGMEContextSingleton()
    {
        static GPGMEContextHolder obj;
        return obj;
    }

    GPGMEDataHolder::GPGMEDataHolder()
        :
        m_gpt( 0 )
    {
        gpgme_error_t e = gpgme_data_new( &m_gpt );
        if( e != GPG_ERR_NO_ERROR )
        {
            fh_stringstream ss;
            ss << "Error getting a data block handle for GPGME";
            ss << "GPGME error:" << gpgme_strerror (e) << endl;
            Throw_GPGMEInitFailed( tostr(ss), 0 );
        }
    }
    // takes ownership of 'd'
    GPGMEDataHolder::GPGMEDataHolder( gpgme_data_t d )
        :
        m_gpt( d )
    {
    }
    GPGMEDataHolder::~GPGMEDataHolder()
    {
//        cerr << "Freeing m_gpt:" << toVoid( m_gpt ) << endl;
        
        if( m_gpt )
            gpgme_data_release( m_gpt );
        m_gpt = 0;
    }
    void
    GPGMEDataHolder::TakeOwnership( gpgme_data_t newdata )
    {
        if( m_gpt )
            gpgme_data_release( m_gpt );
        m_gpt = 0;
        if( newdata )
            m_gpt = newdata;
    }
    



    /*
     * The following two are an attempt to convert to the new 'cbs' style
     * but they will give you a nasty stack smash instead of working :(
     */
//     static ssize_t getGPGData_readf( void* hook, void* buffer, size_t count )
//     {
//         cerr << "getGPGData_readf() hook:" << toVoid( hook )
//              << " count:" << count
//              << endl;
//         if( !buffer )
//         {
//             return -1;
//         }
    
//         fh_istream* issp = (fh_istream*)(hook);
//         if( issp->eof() )
//         {
//             return 0;
//         }
    
//         (*issp).read( (char*)buffer, count );
//         return (*issp).gcount();
//     }

//     gpgme_data_t getGPGData( fh_istream& iss )
//     {
//         gpgme_data_t ret = 0;
        
// //         gpgme_error_t e = gpgme_data_new_with_read_cb(
// //             &ret, getGPGData_readf, &iss );

//         gpgme_data_cbs cbs;
// //      cbs.read = (gpgme_data_read_cb_t)getGPGData_readf;
//         cbs.read = getGPGData_readf;
//         cbs.write = 0;
//         cbs.seek = 0;
//         cbs.release = 0;
//         gpgme_error_t e = gpgme_data_new_from_cbs( &ret, &cbs, &iss );

//         if( e != GPG_ERR_NO_ERROR )
//         {
//             fh_stringstream ss;
//             ss << "Error setting up an IO handler for GPGME";
//             ss << "GPGME error:" << gpgme_strerror (e) << endl;
//             Throw_GPGMEInitFailed( tostr(ss), 0 );
//         }
        
//         return ret;
//     }


    static int getGPGData_readf( void *hook,
                                 char *buffer, size_t count, size_t *nread )
    {
//         cerr << "getGPGData_readf() hook:" << toVoid( hook )
//              << " count:" << count
//              << endl;
        if( !buffer || !nread )
        {
            return -1;
        }
    
        fh_istream* issp = (fh_istream*)hook;
        if( issp->eof() )
        {
//             cerr << "getGPGData_readf EOF!  hook:" << toVoid( hook )
//                  << " count:" << count << endl;
            *nread = 0;
            return 0;
        }

//         cerr << "getGPGData_readf hook:" << toVoid( hook )
//              << " count:" << count << endl;
        
        (*issp).read( buffer, count );
        *nread = (*issp).gcount();

//         cerr << "getGPGData_readf() hook:" << toVoid( hook )
//              << " count:" << count
//              << " *nread:" << (*nread)
//              << endl;

        return 0;
    }

    gpgme_data_t getGPGData( fh_istream* iss )
    {
        gpgme_data_t ret;
        gpgme_error_t e = gpgme_data_new_with_read_cb( &ret, getGPGData_readf, iss );

        if( e != GPG_ERR_NO_ERROR )
        {
            fh_stringstream ss;
            ss << "Error setting up an IO handler for GPGME";
            ss << "GPGME error:" << gpgme_strerror (e) << endl;
            Throw_GPGMEInitFailed( tostr(ss), 0 );
        }
        
        return ret;
    }
    

    static bool hasValidSignature( gpgme_ctx_t gctx,
                                   fh_context plaintextc,
                                   fh_istream iss,
                                   bool passInPlainText = false )
    {
        bool ret = false;
        gpgme_error_t         e  = GPG_ERR_NO_ERROR;
        GPGMEDataHolder   gd( getGPGData( &iss ) );
        GPGMEDataHolder   gpt;
        GPGMEDataHolder   scratch;
        fh_istream tss;

//         cerr << "hasValidSignature(top) passInPlainText:" << passInPlainText << endl;
//          if( plaintextc )
//              cerr << "hasValidSignature(top) plaintextc:" << plaintextc->getURL() << endl;
            
        if( passInPlainText )
        {
            tss = plaintextc->getIStream( ios::in | ferris_ios::o_nouncrypt );
            gpt.TakeOwnership( getGPGData( &tss ) );
        }

//        cerr << "hasValidSignature(1) passInPlainText:" << passInPlainText << endl;
//         if( plaintextc )
//             cerr << "hasValidSignature(1) plaintextc:" << plaintextc->getURL() << endl;
        
        if( passInPlainText ) e = gpgme_op_verify( gctx, gd, gpt, 0 );
        else                  e = gpgme_op_verify( gctx, gd, 0,   scratch );

//         cerr << "hasValidSignature(2) passInPlainText:" << passInPlainText << endl;
//         if( plaintextc )
//             cerr << "hasValidSignature(2) plaintextc:" << plaintextc->getURL() << endl;
        
        if( e == GPG_ERR_NO_ERROR )
            ret = true;
        if( e != GPG_ERR_NO_ERROR )
        {
            fh_stringstream ss;
            ss << "Error verifying signature";
            if( plaintextc )
                ss << "For url:" << plaintextc->getURL();
            ss << "GPGME error:" << gpgme_strerror (e) << endl;
            Throw_BadSignature( tostr(ss), 0 );
        }
    
        return ret;
    }

    static bool hasValidSignature( gpgme_ctx_t gctx,
                                   fh_context plaintextc,
                                   string detachedSignatureFileName,
                                   bool passInPlainText )
    {
        if( plaintextc->getParent()->isSubContextBound( detachedSignatureFileName ))
        {
            fh_context   sigc = plaintextc->getParent()->getSubContext( detachedSignatureFileName );
            fh_istream sigiss = sigc->getIStream( ios::in | ferris_ios::o_nouncrypt );
            return hasValidSignature( gctx,
                                      plaintextc,
                                      sigiss,
                                      passInPlainText );
        }
        return false;
    }


    bool hasValidSignature( gpgme_ctx_t gctx, fh_context c )
    {
        bool ret = false;
        gpgme_error_t e  = GPG_ERR_NO_ERROR;
        string detachedSignatureFileName = c->getDirName();

        ret |= hasValidSignature( gctx, c, c->getDirName() + ".sig", true );
        if( ret )
        {
            return ret;
        }

        fh_istream  iss = c->getIStream( ios::in | ferris_ios::o_nouncrypt );
        ret |= hasValidSignature( gctx, c, iss, false );
        return ret;
    }

    bool hasValidSignature( fh_context c )
    {
        bool ret = false;
        gpgme_error_t e  = GPG_ERR_NO_ERROR;
        GPGMEContextHolder gctx;
        return hasValidSignature( gctx, c );
    }

#endif
    fh_istream getSignedDocumentStream( fh_context c, fh_istream iss )
    {
        fh_stringstream ret;
#ifdef FERRIS_HAVE_GPGME

        gpgme_error_t         e = GPG_ERR_NO_ERROR;
        GPGMEContextHolder gctx;
        GPGMEDataHolder   gd( getGPGData( &iss ) );
        GPGMEDataHolder   gpt;
    
        e = gpgme_op_verify( gctx, gd, 0, gpt );
        if( e == GPG_ERR_NO_ERROR )
        {
//            cerr << "Sig verified" << endl;
            const int buffersz = 4096;
            char buffer[ buffersz + 1 ];

            size_t nread = 0;
            while( e == GPG_ERR_NO_ERROR )
            {
                ssize_t bread = gpgme_data_read( gpt, buffer, buffersz );
                if( bread < 0 )
                {
                    fh_stringstream ess;
                    ess << "Failed to verify the signature, refusing to give stream." << endl
                        << " for c:" << c->getURL()
                        << endl;
                    cerr << tostr(ess) << endl;
                    Throw_BadSignature( tostr(ess), 0 );
                }
                if( bread == 0 )
                    break;
                
                nread = bread;
                ret.write( buffer, nread );
            }
            return ret;
        }
    

        fh_stringstream ess;
        ess << "Failed to verify the signature, refusing to give stream." << endl
            << " for c:" << c->getURL()
            << endl;
        cerr << tostr(ess) << endl;
        Throw_BadSignature( tostr(ess), 0 );
#endif
        {
            fh_stringstream ess;
            ess << "No support for GPG in this libferris build :(" << endl;
            Throw_BadSignature( tostr(ess), 0 );
        }
    }
};

