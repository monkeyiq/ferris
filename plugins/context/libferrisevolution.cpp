/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisevolution.cpp,v 1.4 2010/09/24 21:31:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <config.h>
#ifdef EVOLUTION_BUG__EXTRA_HEADERS_TO_PREINCLUDE
#include "/usr/share/gettext/gettext.h"
#endif


#include <FerrisContextPlugin.hh>
#include <Ferris.hh>
#include <TypeDecl.hh>
#include <Trimming.hh>
#include <General.hh>
#include <Cache.hh>

#include <config.h>

extern "C" {
#include <libebook/e-book.h>
//#include <gtk/gtk.h>
#include <libbonobo.h>
//#include <libgnome/gnome-init.h>
#include <camel/camel.h>
#include <camel/camel-object.h>
#include <camel/camel-folder-summary.h>
#include <camel/camel-folder.h>
#include <camel/camel-store.h>
#include <camel/camel-session.h>
#include <camel/camel-provider.h>
};


using namespace std;


namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    static string tostr( CamelException* ex )
    {
        const char* c = camel_exception_get_description( ex );
        if( c )
            return c;
        return "unknown";
    }
    

    
    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/

    class FERRISEXP_CTXPLUGIN evContactContext
        :
        public StateLessEAHolder< evContactContext, leafContext >
    {
        typedef evContactContext  _Self;
        typedef StateLessEAHolder< evContactContext, leafContext >     _Base;

        EContact* m_contact;
        
    protected:
        
        
    public:

        void constructObject( EContact *contact )
            {
                m_contact = contact;
            }

        evContactContext( Context* parent, const std::string& rdn, EContact *contact = 0 )
            :
            _Base( parent, rdn ),
            m_contact( contact )
            {
                LG_EVO_D << "evContactContext() rdn:" << rdn << endl;
                createStateLessAttributes();
            }
        
        virtual ~evContactContext()
            {
                if( m_contact )
                    g_object_unref (m_contact);
            }

        string getData( EContactField f )
            {
                if( !m_contact )
                {
                    cerr << "getData() called without m_contact" << endl;
                    BackTrace();
                }
                
                gpointer d = (gpointer)e_contact_get_const( m_contact, f );
                if( !d )
                    return "";
                return (const char *)d;
            }
        
        static fh_istream SL_getFullNameStream( evContactContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_FULL_NAME );
                return ss;
            }
        static fh_istream SL_getGivenNameStream( evContactContext* c,
                                                 const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_GIVEN_NAME );
                return ss;
            }
        static fh_istream SL_getFamilyNameStream( evContactContext* c,
                                                  const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_FAMILY_NAME );
                return ss;
            }
        static fh_istream SL_getEmailAddrStream( evContactContext* c,
                                                 const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_EMAIL_1 );
                return ss;
            }
        static fh_istream SL_getPhoneStream( evContactContext* c,
                                             const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_PHONE_HOME );
                return ss;
            }
        static fh_istream SL_getPhoneMobileStream( evContactContext* c,
                                                   const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_PHONE_MOBILE );
                return ss;
            }
        static fh_istream SL_getPhoneHomeStream( evContactContext* c,
                                                   const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_PHONE_HOME );
                return ss;
            }
        static fh_istream SL_getPhoneWorkStream( evContactContext* c,
                                                 const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << c->getData( E_CONTACT_PHONE_BUSINESS );
                return ss;
            }

        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "full-name",   SL_getFullNameStream,   XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "given-name",  SL_getGivenNameStream,   XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "family-name", SL_getFamilyNameStream,  XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "email"      , SL_getEmailAddrStream,   XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "phone"      , SL_getPhoneStream,       XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "mobile"     , SL_getPhoneMobileStream, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "phone-home"   , SL_getPhoneHomeStream, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "phone-work"   , SL_getPhoneWorkStream, XSD_BASIC_STRING );
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        std::string getRecommendedEA()
            {
                return _Base::getRecommendedEA() + "," + "full-name,phone,mobile,email";
            }
        
    };

    
    class FERRISEXP_CTXPLUGIN evContactsQueryContext
        :
        public StateLessEAHolder< evContactsQueryContext, FakeInternalContext >
    {
        typedef evContactsQueryContext  _Self;
        typedef StateLessEAHolder< evContactsQueryContext, FakeInternalContext >     _Base;

        EBook*       m_book;
        EBookQuery*  m_query;
        
    protected:
        
        virtual void priv_read()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );

                clearContext();
                GError* error = 0;
                GList* contacts = 0, *c=0;

                LG_EVO_D << "read() starting" << endl;
                    
                if( e_book_get_contacts( m_book, m_query, &contacts, &error ))
                {
                    int i=0;
                    for (c = contacts;  c;  c = c->next, ++i)
                    {
                        EContact *contact = E_CONTACT (c->data);

                        bool rdnFound = false;
                        string rdn = tostr( i );

                        LG_EVO_D << "read() rdn-is-i:" << rdn << endl;
                        if( gpointer d = (gpointer)e_contact_get_const( contact, E_CONTACT_FULL_NAME ) )
                            rdn = (const char*)d;
                        else
                        {
                            if( gpointer d = (gpointer)e_contact_get_const( contact, E_CONTACT_GIVEN_NAME ) )
                            {
                                rdn = (const char*)d;
                                if( !priv_isSubContextBound( rdn ) )
                                {
                                    rdnFound = true;
                                }
                            }
                            if( !rdnFound )
                            {
                                if( gpointer d = (gpointer)e_contact_get_const( contact, E_CONTACT_EMAIL_1 ) )
                                {
                                    rdn = (const char*)d;
                                }
                            }
                        }
                        LG_EVO_D << "read(selected) rdn:" << rdn << endl;
                            
                        evContactContext* c = 0;
                        c = priv_ensureSubContext( rdn, c, true );
                        c->constructObject( contact );
                    }
                }
                    
                g_list_free (contacts);
            }
        
    public:
        
        evContactsQueryContext( Context* parent, const std::string& rdn, EBook* book, EBookQuery* q )
            :
            _Base( parent, rdn ),
            m_book( book ),
            m_query( q )
            {
                createStateLessAttributes();
            }
        
        virtual ~evContactsQueryContext()
            {
                e_book_query_unref (m_query);
                g_object_unref (m_book);
            }

        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
//                    tryAddStateLessAttribute( "size",     SL_getSizeStream,     FXD_FILESIZE );
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        std::string getRecommendedEA()
            {
                return _Base::getRecommendedEA();// + "," + "phone,email";
            }
        
    };
    
    class FERRISEXP_CTXPLUGIN evContactsEBookContext
        :
        public FakeInternalContext
    {
        typedef evContactsEBookContext    _Self;
        typedef FakeInternalContext  _Base;

    protected:
        
        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );
                if( empty() )
                {
                    EBook* book = e_book_new_system_addressbook (NULL);
                    if (!book)
                    {
                        fh_stringstream ss;
                        ss << "Failed to create an addressbook object" << endl;
                        Throw_NoSuchSubContext( tostr(ss), 0 );
                    }
                    
                    int status = e_book_open (book, TRUE, NULL);
                    if (!status)
                    {
                        fh_stringstream ss;
                        ss << "Failed to open system addressbook" << endl;
                        Throw_NoSuchSubContext( tostr(ss), 0 );
                    }

                    EBookQuery* query = e_book_query_any_field_contains ("");
                    evContactsQueryContext* c = new evContactsQueryContext( this, "system", book, query );
                    fh_context tc = c;
                    Insert( c, false, false );
                }
            }
        
    public:
        
        evContactsEBookContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {}
        
        virtual ~evContactsEBookContext()
            {}
    };
    

    /******************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN evServerLocalMailContext;
    /**
     * A common parent for local evolution mail directories.
     * this provides simple access to the session, exception and store.
     */
    class FERRISEXP_CTXPLUGIN evServerLocalGenericMailContext
        :
        public FakeInternalContext
    {
        typedef evServerLocalGenericMailContext  _Self;
        typedef FakeInternalContext              _Base;
    protected:

    public:
        evServerLocalGenericMailContext( Context* parent, const std::string& rdn )
            :
            FakeInternalContext( parent, rdn )
            {}
        
        virtual ~evServerLocalGenericMailContext()
            {}

        virtual evServerLocalMailContext* getBase();
        virtual CamelSession*   session();
        virtual CamelException* ex();
        virtual CamelStore*     store();
        
    };



    class FERRISEXP_CTXPLUGIN evServerLocalAttachmentContext
        :
        public StateLessEAHolder< evServerLocalAttachmentContext, leafContext >
    {
        typedef evServerLocalAttachmentContext  _Self;
        typedef StateLessEAHolder< evServerLocalAttachmentContext, leafContext >  _Base;
    protected:

        CamelMimePart* m_part;

        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                fh_stringstream ret;

                CamelDataWrapper* dw = camel_medium_get_content_object( CAMEL_MEDIUM(m_part) );
                GByteArray *buffer = g_byte_array_new();
                CamelStream* stream = camel_stream_mem_new_with_byte_array( buffer );
                ssize_t sz = camel_data_wrapper_decode_to_stream( dw, stream );
                ret.write( (const char*)buffer->data, buffer->len );

                camel_object_unref(stream);
                camel_object_unref(dw);
                g_byte_array_free( buffer, true );

                return ret;
            }
        
    public:
        evServerLocalAttachmentContext( Context* parent, const std::string& rdn, CamelMimePart *part )
            :
            _Base( parent, rdn ),
            m_part( part )
            {
                createStateLessAttributes();
            }
        
        virtual ~evServerLocalAttachmentContext()
            {}

        static fh_istream SL_getSizeStream( evServerLocalAttachmentContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                // FIXME: this is a very slow way
                fh_istream iss = c->priv_getIStream( std::ios::in );
                streamsize sz = 0;
                char ch;
                while( iss >> ch )
                    ++sz;
                
                ss << sz;
                return ss;
            }

        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "size",     SL_getSizeStream,     FXD_FILESIZE );
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }
        std::string getRecommendedEA()
            {
                return _Base::getRecommendedEA() + "," + "size";
            }
        
    };
    
        
    class FERRISEXP_CTXPLUGIN evServerLocalMailMessageContext
        :
        public StateLessEAHolder< evServerLocalMailMessageContext, leafContext >
    {
        typedef evServerLocalMailMessageContext  _Self;
        typedef StateLessEAHolder< evServerLocalMailMessageContext, leafContext > _Base;

        CamelMessageInfo* m_msginfo;

    protected:

        CamelFolder* folder();
        CamelException* ex();
        
        CamelDataWrapper* narrow_to_msg_body_for_multipart( CamelDataWrapper*  dw )
            {
                if (CAMEL_IS_MULTIPART(dw)) {
                    int parts = camel_multipart_get_number((CamelMultipart *)dw);
                    for (int i = 0; i < parts; i++)
                    {
                        CamelMimePart *part = camel_multipart_get_part((CamelMultipart *)dw, i);
                        cerr << "Content type:" << camel_data_wrapper_get_mime_type( (CamelDataWrapper*)part )
                             << endl;
                        if( !strcmp( camel_data_wrapper_get_mime_type( (CamelDataWrapper*)part ),
                                     "multipart/alternative" ))
                        {
                            CamelMedium*      medium   = CAMEL_MEDIUM (part);
                            CamelDataWrapper* olddw = dw;
                            dw = camel_medium_get_content_object (medium);
                            camel_object_unref(dw);
                            
                            int parts = camel_multipart_get_number((CamelMultipart *)dw);
                            cerr << "parts2:" << parts << endl;
                            for (int i = 0; i < parts; i++)
                            {
                                CamelMimePart *part = camel_multipart_get_part((CamelMultipart *)dw, i);
                                if( !strcmp( camel_data_wrapper_get_mime_type( (CamelDataWrapper*)part ),
                                             "text/plain" ))
                                {
                                    medium   = CAMEL_MEDIUM (part);
                                    olddw = dw;
                                    dw = camel_medium_get_content_object (medium);
                                    camel_object_unref(dw);
                                    break;
                                }
                            }
                            break;
                        }
                    }            
                }
                return dw;
            }

        CamelDataWrapper* getDW()
            {
                const char* uid = (const char*)camel_message_info_uid( m_msginfo );
//                cerr << "priv_getIStream() uid:" << uid << endl;
                CamelMimeMessage* message = camel_folder_get_message( folder(), uid, ex() );
                CamelMimePart*    mimepart = (CamelMimePart*)message;
                CamelMedium*      medium   = CAMEL_MEDIUM (mimepart);
                CamelDataWrapper* dw = camel_medium_get_content_object (medium);
//                cerr << " message:" << message << " meduim:" << medium << " dw:" << dw << endl;

//                camel_object_unref(message);
                return dw;
            }
        
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
            throw (FerrisParentNotSetError,
                   CanNotGetStream,
                   exception)
            {
                fh_stringstream ret;

                
                CamelDataWrapper* dw = getDW();
                dw = narrow_to_msg_body_for_multipart( dw );
                
                GByteArray *buffer = g_byte_array_new();
                CamelStream* stream = camel_stream_mem_new_with_byte_array( buffer );
                ssize_t sz = camel_data_wrapper_decode_to_stream( dw, stream );
                ret.write( (const char*)buffer->data, buffer->len );

                camel_object_unref(stream);
//                g_byte_array_free( buffer, true );
                camel_object_unref(dw);

                return ret;
            }

        bool haveTriedToReadMIMEParts;
        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( !haveTriedToReadMIMEParts )
                {
                    haveTriedToReadMIMEParts = true;

                    CamelDataWrapper* dw = getDW();

                    if (CAMEL_IS_MULTIPART(dw)) {
                        int parts = camel_multipart_get_number((CamelMultipart *)dw);
                        for (int i = 0; i < parts; i++)
                        {
                            CamelMimePart *part = camel_multipart_get_part((CamelMultipart *)dw, i);
                            cerr << "Content type:" << camel_data_wrapper_get_mime_type( (CamelDataWrapper*)part )<< endl;

                            const char* rdn = camel_mime_part_get_filename( part );

                            if( rdn )
                            {
                                evServerLocalAttachmentContext* c
                                    = new evServerLocalAttachmentContext( this, rdn, part );
                                fh_context tc = c;
                                Insert( c, false, false );
                            }
                        }
                    }

                    camel_object_unref(dw);
                }
            }
        
        
    public:

        void constructObject( CamelMessageInfo* msginfo )
            {
                m_msginfo = msginfo;
            }
        
        evServerLocalMailMessageContext( Context* parent, const std::string& rdn,
                                         CamelMessageInfo* msginfo = 0 )
            :
            _Base( parent, rdn ),
            m_msginfo( msginfo ),
            haveTriedToReadMIMEParts( false )
            {
                createStateLessAttributes();
            }
        
        virtual ~evServerLocalMailMessageContext()
            {
                if( m_msginfo )
                    camel_object_unref( m_msginfo );
            }

        static fh_istream SL_getSubjectStream( evServerLocalMailMessageContext* c,
                                               const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_subject( c->m_msginfo );
                return ss;
            }
        static fh_istream SL_getFromStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_from( c->m_msginfo );
                return ss;
            }
        static fh_istream SL_getToStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_to( c->m_msginfo );
                return ss;
            }
        static fh_istream SL_getCCStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_cc( c->m_msginfo );
                return ss;
            }
        static fh_istream SL_getUIDStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_uid( c->m_msginfo );
                return ss;
            }
        static fh_istream SL_getSizeStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_size(c->m_msginfo);
                return ss;
            }
        static fh_istream SL_getSentTimeStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_date_sent(c->m_msginfo);
                return ss;
            }
        static fh_istream SL_getRecvTimeStream( evServerLocalMailMessageContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                ss << camel_message_info_date_received(c->m_msginfo);
                return ss;
            }
        static fh_istream SL_getHasAttachmentStream( evServerLocalMailMessageContext* c,
                                                     const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                CamelDataWrapper* dw = c->getDW();
                bool hasAttachments = false;
                
                if (CAMEL_IS_MULTIPART(dw))
                {
                    int parts = camel_multipart_get_number((CamelMultipart *)dw);
                    for (int i = 0; i < parts; i++)
                    {
                        CamelMimePart *part = camel_multipart_get_part((CamelMultipart *)dw, i);
//                         cerr << "parts:" << parts
//                              << " Content type:" << camel_data_wrapper_get_mime_type( (CamelDataWrapper*)part )
//                              << endl;

                        const char* disp = camel_mime_part_get_disposition( part );
//                         if( disp )
//                             cerr << "disposition:" << disp << endl;
                        
                        if( disp && !strcmp( disp, "attachment" ))
                        {
                            hasAttachments = true;
                            break;
                        }
                    }
                }
                camel_object_unref(dw);

                ss << hasAttachments;
                return ss;
            }


        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "subject", SL_getSubjectStream, XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "from",    SL_getFromStream,    XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "to",      SL_getToStream,      XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "cc",      SL_getCCStream,      XSD_BASIC_STRING );
                    tryAddStateLessAttribute( "uid",     SL_getUIDStream,     XSD_BASIC_STRING );

                    tryAddStateLessAttribute( "has-attachment", SL_getHasAttachmentStream, XSD_BASIC_BOOL );

                    tryAddStateLessAttribute( "size",     SL_getSizeStream,     FXD_FILESIZE );
                    tryAddStateLessAttribute( "sent",     SL_getSentTimeStream, FXD_UNIXEPOCH_T );
                    tryAddStateLessAttribute( "recv",     SL_getRecvTimeStream, FXD_UNIXEPOCH_T );
                    tryAddStateLessAttribute( "received", SL_getRecvTimeStream, FXD_UNIXEPOCH_T );

                    supplementStateLessAttributes_timet( "sent" );
                    supplementStateLessAttributes_timet( "recv" );
                    supplementStateLessAttributes_timet( "received" );
                    
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        std::string getRecommendedEA()
            {
                stringstream ss;
                ss << "name,subject,size,from,to,sent-display,received-display";
                return adjustRecommendedEAForDotFiles(this, tostr(ss));
            }
        
    };
    
    

    class FERRISEXP_CTXPLUGIN evServerLocalMailFolderContext
        :
        public StateLessEAHolder< evServerLocalMailFolderContext, evServerLocalGenericMailContext >
    {
        typedef evServerLocalMailFolderContext  _Self;
        typedef StateLessEAHolder< evServerLocalMailFolderContext, evServerLocalGenericMailContext > _Base;

        CamelFolder* m_folder;
        CamelFolderInfo* m_fi;
        bool haveReadMessages;
        
    protected:

        //
        // Short cut loading each dir unless absolutely needed.
        //
        fh_context priv_getSubContext( const string& rdn )
            throw( NoSuchSubContext )
            {
                try
                {
                    if( priv_isSubContextBound( rdn ) )
                    {
                        LG_NATIVE_D << "NativeContext::priv_getSubContext(bound already) p:" << getDirPath()
                                    << " rdn:" << rdn
                                    << endl;
                        return _Base::priv_getSubContext( rdn );
                    }

                    stringstream ss;
                    ss << m_fi->full_name << "/" << rdn;
                    int flags = 0;
                    CamelFolder* folder = camel_store_get_folder( store(), ss.str().c_str(), flags, ex() );
                    
                    if( !folder )
                    {
                        // failed
                        fh_stringstream ss;
                        ss << "NoSuchSubContext:" << rdn;
                        Throw_NoSuchSubContext( tostr(ss), this );
                    }

                    CamelFolderInfo* fi = camel_store_get_folder_info( store(), ss.str().c_str(), 0, ex() );
                    
                    bool created = false;
                    evServerLocalMailFolderContext* c
                        = new evServerLocalMailFolderContext( this, fi->name, folder, fi );
                    fh_context tc = c;
                    Insert( c, false, false );

                    cerr << "Successful shortcut load of path:" << fi->full_name << endl;
                    return tc;
                }
                catch( NoSuchSubContext& e )
                {
                    throw e;
                }
                catch( exception& e )
                {
                    string s = e.what();
//            cerr << "NativeContext::priv_getSubContext() e:" << e.what() << endl;
                    Throw_NoSuchSubContext( s, this );
                }
                catch(...)
                {}
                fh_stringstream ss;
                ss << "NoSuchSubContext:" << rdn;
                Throw_NoSuchSubContext( tostr(ss), this );
            }

        virtual void priv_read()
            {
                EnsureStartStopReadingIsFiredRAII _raii1( this );

                clearContext();
                
//                if( !haveReadMessages )
                {
//                    haveReadMessages = true;
                    
                    CamelFolderInfo* subfi = camel_store_get_folder_info( store(), m_fi->full_name, 0, ex() );
                    if( subfi->child )
                    {
                        subfi = subfi->child;
                        CamelFolder* folder = camel_store_get_folder( store(), subfi->full_name, 0, ex() );
                        if( folder )
                        {
                            string rdn = subfi->name;
                            
                            evServerLocalMailFolderContext* c = 0;
                            c = priv_ensureSubContext( rdn, c, true );
                            c->constructObject( folder, subfi );
                        }
                        
//                         if( folder && !priv_isSubContextBound( subfi->name ))
//                         {
//                             evServerLocalMailFolderContext* c
//                                 = new evServerLocalMailFolderContext( this, subfi->name, folder, subfi );
//                             fh_context tc = c;
//                             Insert( c, false, false );
//                         }
                    }

                    CamelFolder* folder = this->folder();

                    LG_EVO_D << "----- top --------" << endl;
                    GPtrArray* uids = camel_folder_get_uids( folder );
                    if( uids )
                        for (int j=0;j<uids->len;j++)
                        {
                            const char* uid = (const char*)uids->pdata[j];
                            CamelMessageInfo* msginfo = camel_folder_get_message_info( folder, uid );
            
                            LG_EVO_D << "msg. ID:" << camel_message_info_uid( msginfo )
                                     << " subject:" << camel_message_info_subject( msginfo )
                                     << " from:" << camel_message_info_from( msginfo )
                                     << " to:" << camel_message_info_to( msginfo )
                                     << endl;

                            string rdn = camel_message_info_uid( msginfo );

                            evServerLocalMailMessageContext* c = 0;
                            c = priv_ensureSubContext( rdn, c, true );
                            c->constructObject( msginfo );
                            
//                             evServerLocalMailMessageContext* c
//                                 = new evServerLocalMailMessageContext( this, rdn, msginfo );
//                             fh_context tc = c;
//                             Insert( c, false, false );
                        }
    
                    camel_folder_free_uids(folder, uids);
                    LG_EVO_D << "----- bot --------" << endl;
                }
            }
        
    public:

        void constructObject( CamelFolder* folder, CamelFolderInfo* fi )
            {
                m_folder = folder;
                m_fi = fi;
            }
        
        evServerLocalMailFolderContext( Context* parent, const std::string& rdn,
                                        CamelFolder* folder = 0 , CamelFolderInfo* fi = 0 )
            :
            _Base( parent, rdn ),
            m_folder( folder ),
            m_fi( fi ),
            haveReadMessages( false )
            {
                createStateLessAttributes();
//                m_folder = camel_store_get_folder( store(), m_fi->full_name, 0, ex() );
            }
        
        virtual ~evServerLocalMailFolderContext()
            {
                if( m_fi )
                    camel_object_unref( m_fi );
                if( m_folder )
                    camel_object_unref( m_folder );
            }

        CamelFolder* folder()
            {
                return m_folder;
            }
        
        static fh_istream SL_getMsgCountStream( evServerLocalMailFolderContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                CamelFolder* folder = c->folder();
                ss << camel_folder_get_message_count( folder );
                return ss;
            }
        static fh_istream SL_getUnreadCountStream( evServerLocalMailFolderContext* c,
                                            const std::string& rdn, EA_Atom* atom )
            {
                c->ensureUpdateMetaDataCalled();
                fh_stringstream ss;
                CamelFolder* folder = c->folder();
                ss << camel_folder_get_unread_message_count( folder );
                return ss;
            }

        
        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
                    tryAddStateLessAttribute( "message-count",  SL_getMsgCountStream,    XSD_BASIC_INT );
                    tryAddStateLessAttribute( "unread-count",   SL_getUnreadCountStream, XSD_BASIC_INT );
                    
                    _Base::createStateLessAttributes( true );
                    supplementStateLessAttributes( true );
                }
            }

        std::string getRecommendedEA()
            {
                stringstream ss;
                ss << "name,message-count,unread-count";
                return adjustRecommendedEAForDotFiles(this, tostr(ss));
            }
        
    };
        

    class FERRISEXP_CTXPLUGIN evServerLocalMailContext
        :
        public evServerLocalGenericMailContext
    {
        typedef evServerLocalMailContext  _Self;
        typedef evServerLocalGenericMailContext       _Base;

        CamelSession*   m_session;
        CamelException* m_ex;
        CamelStore*     m_store;
        
    protected:

        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    char* folderNames[] = { "Inbox", "Sent", "Drafts", 0, "Outbox", 0 };

                    for( char** p = folderNames; *p; ++p )
                    {
                        guint32 folder_info_flags = 0; // CAMEL_STORE_FOLDER_INFO_RECURSIVE
                        string topFolderName = *p;
                        int flags = 0;
                        CamelFolderInfo* fi = camel_store_get_folder_info( store(), topFolderName.c_str(),
                                                                           folder_info_flags, ex() );
                        if( camel_exception_is_set(ex()) || !fi)
                        {
                            cerr << "store:" << store() << " topFolder:" << topFolderName << endl;
                        
                            stringstream ss;
                            ss << "Failed to get local evolution mail. reason:" << tostr(ex());
                            LG_EVO_ER << tostr(ss) << endl;
                            Throw_GenericError( tostr(ss), this );
                        }
                    
                        CamelFolder* folder = camel_store_get_folder( store(), topFolderName.c_str(), flags, ex() );
                        if( camel_exception_is_set(ex()) || !folder)
                        {
                            stringstream ss;
                            ss << "Failed to get local evolution mail. reason:" << tostr(ex());
                            LG_EVO_ER << tostr(ss) << endl;
                            Throw_GenericError( tostr(ss), this );
                        }

                        evServerLocalMailFolderContext* c = new evServerLocalMailFolderContext( this, topFolderName,
                                                                                                folder, fi );
                        fh_context tc = c;
                        Insert( c, false, false );
                    }
                }
            }
        
    public:
        
        evServerLocalMailContext( Context* parent, const std::string& rdn )
            :
            evServerLocalGenericMailContext( parent, rdn ),
            m_session( 0 ),
            m_ex( 0 ),
            m_store( 0 )
            {
                int argc = 1;
                char* argv[] = {"libferrisevolution",0};
                
                if (bonobo_init (&argc, argv) == FALSE)
                    g_error ("Could not initialize Bonobo");
                
                const string camelPath = Shell::getHomeDirPath() + "/.evolution";
                camel_init (camelPath.c_str(), FALSE);
                camel_type_init ();
                camel_provider_init();
                
                m_ex = camel_exception_new();

                m_session = CAMEL_SESSION (camel_object_new (camel_session_get_type ()));
                camel_session_construct (m_session, camelPath.c_str() );

                stringstream defaultStorePathSS;
                defaultStorePathSS << "mbox://" << camelPath << "/mail/local";
                string defaultStorePath = defaultStorePathSS.str();
//                cerr << "defaultStorePath:" << defaultStorePath << endl;
                m_store = camel_session_get_store(m_session, defaultStorePath.c_str(), m_ex);

                if( camel_exception_is_set(m_ex) || !m_store)
                {
                    stringstream ss;
                    ss << "Failed to get local evolution mail. reason:" << tostr(m_ex);
                    LG_EVO_ER << tostr(ss) << endl;
                    Throw_GenericError( tostr(ss), parent );
                }
            }
        
        virtual ~evServerLocalMailContext()
            {
                camel_object_unref(m_session);
                camel_exception_free(m_ex);
            }

        virtual CamelSession*   session()
            {
                return m_session;
            }
        virtual CamelException* ex()
            {
                return m_ex;
            }
        virtual CamelStore*     store()
            {
                return m_store;
            }
        
    };

    evServerLocalMailContext*
    evServerLocalGenericMailContext::getBase()
    {
        Context* c = this;
        while( c->isParentBound() )
        {
            c = c->getParent();
            if( evServerLocalMailContext* ret = dynamic_cast<evServerLocalMailContext*>( c ))
                return ret;
        }
    }
    CamelSession*   evServerLocalGenericMailContext::session()
    {
        return getBase()->session();
    }
    CamelException* evServerLocalGenericMailContext::ex()
    {
        return getBase()->ex();
    }
    CamelStore*     evServerLocalGenericMailContext::store()
    {
        return getBase()->store();
    }
    

    
    CamelFolder* evServerLocalMailMessageContext::folder()
    {
        return ((evServerLocalMailFolderContext*)getParent())->folder();
    }
    CamelException* evServerLocalMailMessageContext::ex()
    {
        return ((evServerLocalMailFolderContext*)getParent())->ex();
    }
    

    
    class FERRISEXP_CTXPLUGIN evMailContext
        :
        public FakeInternalContext
    {
        typedef evMailContext        _Self;
        typedef FakeInternalContext  _Base;

    protected:
        
        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    evServerLocalMailContext* c = new evServerLocalMailContext( this, "local" );
                    fh_context tc = c;
                    Insert( c, false, false );
                }
            }
        
    public:
        
        evMailContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {}
        
        virtual ~evMailContext()
            {}
    };
    
    
    
    class FERRISEXP_CTXPLUGIN evServerContext
        :
        public FakeInternalContext
    {
        typedef evServerContext      _Self;
        typedef FakeInternalContext  _Base;

    protected:
        
        virtual void priv_read()
            {
                LG_EVO_D << "priv_read(a)" << endl;

                staticDirContentsRAII _raii1( this );
                if( empty() )
                {
                    LG_EVO_D << "priv_read()" << endl;
                    {
                        evMailContext* c = new evMailContext( this, "mail" );
                        fh_context tc = c;
                        Insert( c, false, false );
                    }
                    {
                        evContactsEBookContext* c = new evContactsEBookContext( this, "contacts" );
                        fh_context tc = c;
                        Insert( c, false, false );
                    }
                    
                }
            }
        
    public:
        
        evServerContext( Context* parent, const std::string& rdn )
            :
            FakeInternalContext( parent, rdn )
            {}
        
        virtual ~evServerContext()
            {}
    };
    

    /*
     * base evolution:// context
     */
    class FERRISEXP_CTXPLUGIN evRootContext
        :
        public networkRootContext<evServerContext>
    {
        typedef evRootContext                        _Self;
        typedef networkRootContext<evServerContext>  _Base;

    public:
        
        evRootContext( Context* parent, const std::string& rdn )
            :
            networkRootContext<evServerContext>( parent, rdn, true )
            {
            }
        
        virtual ~evRootContext()
            {
            }
    };
    

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                const string& root = rf->getInfo( RootContextFactory::ROOT );
                const string& path = rf->getInfo( RootContextFactory::PATH );
//                cerr << " root:" << root << " path:" << path << endl;

                static evRootContext* c = 0;

                if( !c )
                {
                    g_module_open ( "/usr/lib/evolution/2.0/libcamel.so", G_MODULE_BIND_LAZY );
                    g_module_open ( "/usr/lib/evolution/2.0/libeshell.so", G_MODULE_BIND_LAZY );
                    g_module_open ( "/usr/lib/evolution/2.0/camel-providers/libcamellocal.so", G_MODULE_BIND_LAZY );

                    
                    LG_EVO_D << "Making evRootContext(1) " << endl;
                    c = new evRootContext(0, "/");
            
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                    LG_EVO_D << "Making evRootContext(2) " << endl;
                }

                fh_context ret = c;

                if( root != "/" )
                {
                    fh_stringstream ss;
                    ss << root << "/" << path;
                    rf->AddInfo( RootContextFactory::PATH, tostr(ss) );
                }

                
                
                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_EVO_D << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
