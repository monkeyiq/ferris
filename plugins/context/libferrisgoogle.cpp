/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2009 Ben Martin

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

    $Id: libferrispostgresql.cpp,v 1.11 2009/04/18 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <FerrisKDE.hh>
#include <Trimming.hh>
#include <FerrisDOM.hh>
#include <FerrisCreationPlugin.hh>
#include <FerrisBoost.hh>

#include "libferrisgoogle_shared.hh"

using namespace std;

#define DEBUG LG_GOOGLE_D

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN GoogleWorksheetRowContext
        :
        public StateLessEAHolder< GoogleWorksheetRowContext, leafContext >
    {
        typedef StateLessEAHolder< GoogleWorksheetRowContext, leafContext > _Base;
        typedef GoogleWorksheetRowContext _Self;
        
        bool m_haveTriedToRead;
        fh_GoogleClient m_gc;
        fh_GoogleWorkSheet m_sheet;
        int m_row;
        
    public:

        GoogleWorksheetRowContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false ),
            m_gc( 0 ),
            m_row( 0 )
            {
                createStateLessAttributes( true );
                LG_GOOGLE_D << "rdn:" << rdn << " dirName:" << getDirName()
                            << " get(name):" << getStrAttr(this,"name","") << endl;
            }
        virtual ~GoogleWorksheetRowContext()
        {
            cerr << "~GoogleWorksheetRowContext()" << endl;
        }

        virtual std::string getRecommendedEA()
            {
                stringstream ss;
                ss << "name,a,b,c,d,e,f,g,h,i,j,";
                if( m_sheet )
                {
                    stringlist_t sl = m_sheet->getColumnNames();
                    for( stringlist_t::iterator si = sl.begin(); si != sl.end(); ++si )
                    {
                        if( si->length() > 1 )
                            ss << *si << ",";
                    }
                }
                
                return adjustRecommendedEAForDotFiles(this, ss.str());
            }
        
                

        virtual void createStateLessAttributes( bool force = false )
            {
                static Util::SingleShot virgin;
                if( virgin() )
                {
#define SLEA tryAddStateLessAttribute
                    for( int col = 1; col < 25; ++col )
                    {
                        string k = columnNumberToName(col);
                        LG_GOOGLE_D << "createStateLessAttributes() k:" << k << endl;
                        
                        SLEA( k,
                              &_Self::SL_getValueStream,
                              &_Self::SL_getValueStream,
                              &_Self::SL_setValueStream,
                              XSD_BASIC_STRING );
                    }
#undef SLEA
                }
                _Base::createStateLessAttributes( force );
            }

        

        static fh_iostream SL_getValueStream( GoogleWorksheetRowContext* c, const std::string& rdn, EA_Atom* atom )
            {
                LG_GOOGLE_D << "SL_getValueStream() rdn:" << rdn << " row:" << c->m_row << endl;
                return c->getValueStream( rdn, atom );
            }
        fh_iostream getValueStream( const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                string v = m_sheet->getCell( m_row, rdn )->value();
                LG_GOOGLE_D << "getValueStream() rdn:" << rdn << " row:" << m_row << " ret:" << v << endl;
                ss << v;
                return ss;
            }
        static void SL_setValueStream( GoogleWorksheetRowContext* c, const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                LG_GOOGLE_D << "SL_setValueStream()" << endl;
                c->setValueStream( rdn, atom, ss );
            }
        void setValueStream( const std::string& rdn, EA_Atom* atom, fh_istream ss )
            {
                string s = StreamToString(ss);
                fh_GoogleWorkSheetCell cell = m_sheet->getCell( m_row, rdn );
                cell->value( s );
                m_sheet->sync();
                
            }
                
        void setGoogleClient( fh_GoogleClient gc )
        {
            m_gc = gc;
        }
        void setGoogleSheet( fh_GoogleWorkSheet ss )
        {
            m_sheet = ss;
        }
        void setRow( int r )
        {
            m_row = r;

#define SLEA tryAddStateLessAttribute
            if( r==1 )
            {
                stringlist_t colnames = m_sheet->getColumnNames();
                for( stringlist_t::iterator ci = colnames.begin(); ci != colnames.end(); ++ci )
                {
                    string k = *ci;
                    LG_GOOGLE_D << "createStateLessAttributes(colnames) k:" << k << endl;

                    SLEA( k,
                          &_Self::SL_getValueStream,
                          &_Self::SL_getValueStream,
                          &_Self::SL_setValueStream,
                          XSD_BASIC_STRING );
                }
            }
#undef SLEA
        }
    };

    
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN GoogleWorksheetContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
        fh_GoogleClient m_gc;
        fh_GoogleWorkSheet m_sheet;
        
    public:
        
        GoogleWorksheetContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false ),
            m_gc( 0 )
            {
            }
        
        void setGoogleClient( fh_GoogleClient gc )
        {
            m_gc = gc;
        }
        void setGoogleSheet( fh_GoogleWorkSheet ss )
        {
            m_sheet = ss;
        }


        
        

        
        
        void priv_read()
        {
            LG_GOOGLE_D << "priv_read() url:" << getURL()
                        << " m_haveTriedToRead:" << m_haveTriedToRead
                        << " have read:" << getHaveReadDir()
                        << endl;
            EnsureStartStopReadingIsFiredRAII _raii1( this );
            
            // if( m_haveTriedToRead )
            // {
            //     emitExistsEventForEachItemRAII _raii1( this );
            // }
            // else
            {
                m_haveTriedToRead = true;

                // FIXME: use some etag / ensure code
                // here to only fetch the cells if they have changed
                try 
                {
                    m_sheet->fetchCells();
                }
                catch(...)
                {
                }
                
                int maxr = m_sheet->getLargestRowNumber();
                for( int r = 1; r <= maxr; ++r )
                {
                    string rdn = tostr(r);

                    LG_GOOGLE_D << "Adding worksheet row:" << rdn << endl;

                    GoogleWorksheetRowContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->setGoogleClient( m_gc );
                    c->setGoogleSheet( m_sheet );
                    c->setRow( r );

                    
                    // GoogleWorksheetRowContext* c = new GoogleWorksheetRowContext( this, rdn );
                    // c->setGoogleClient( m_gc );
                    // c->setGoogleSheet( m_sheet );
                    // c->setRow( r );
                    // Insert( c );
                }

                emitExistsEventForEachItemRAII _raii1( this );
            }
            
        }
        
    };

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    
    class FERRISEXP_CTXPLUGIN GoogleSpreadsheetContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
        fh_GoogleClient m_gc;
        fh_GoogleSpreadSheet m_sheet;
        
    public:
        
        GoogleSpreadsheetContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false ),
            m_gc( 0 )
            {
            }
        
        void setGoogleClient( fh_GoogleClient gc )
        {
            m_gc = gc;
        }
        void setGoogleSheet( fh_GoogleSpreadSheet ss )
        {
            m_sheet = ss;
        }
        
        
        void priv_read()
        {
            LG_GOOGLE_D << "priv_read() url:" << getURL()
                        << " m_haveTriedToRead:" << m_haveTriedToRead
                        << " have read:" << getHaveReadDir()
                        << endl;
            EnsureStartStopReadingIsFiredRAII _raii1( this );
                
            if( m_haveTriedToRead )
            {
                emitExistsEventForEachItemRAII _raii1( this );
            }
            else
            {
                m_haveTriedToRead = true;
                GoogleWorkSheets_t ws = m_sheet->listSheets();
                for( GoogleWorkSheets_t::iterator wi = ws.begin(); wi!=ws.end(); ++wi )
                {
                    fh_GoogleWorkSheet ws = *wi;
                    ws->setDelayCellSync( true );

                    LG_GOOGLE_D << "sheet:" << ws->getTitle() << endl;
                    string rdn = ws->getTitle();

                    GoogleWorksheetContext* c = new GoogleWorksheetContext( this, rdn );
                    c->setGoogleClient( m_gc );
                    c->setGoogleSheet( ws );
                    Insert( c );
                }
            }
            
        }
        
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN GoogleSpreadsheetDirectoryContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
        fh_GoogleClient m_gc;
        
    public:
        
        GoogleSpreadsheetDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false ),
            m_gc( 0 )
            {
                LG_GOOGLE_D << "ctor, have read:" << getHaveReadDir() << endl;

//                m_gc = Factory::createGoogleClient();
//                m_gc->Authenticate_ClientLogin();
            }

        void constructObject( fh_GoogleClient gc )
        {
            m_gc = gc;
        }
        
        void priv_read()
            {
                LG_GOOGLE_D << "priv_read() url:" << getURL()
                           << " m_haveTriedToRead:" << m_haveTriedToRead
                           << " have read:" << getHaveReadDir()
                           << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;

                    GoogleSpreadSheets_t sheets = m_gc->listSheets();
                    for( GoogleSpreadSheets_t::iterator si = sheets.begin(); si!=sheets.end(); ++si )
                    {
                        fh_GoogleSpreadSheet ss = *si;
                        LG_GOOGLE_D << "sheet:" << ss->getTitle() << endl;
                        string rdn = ss->getTitle();

                        GoogleSpreadsheetContext* c = new GoogleSpreadsheetContext( this, rdn );
                        c->setGoogleClient( m_gc );
                        c->setGoogleSheet( ss );
                        Insert( c );
                    }
                }
            
            }
        
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class FERRISEXP_CTXPLUGIN GoogleDocContext
        :
        public leafContext
    {
        typedef leafContext _Base;
        typedef GoogleDocContext _Self;
        fh_GoogleClient m_gc;
        fh_GoogleDocument m_doc;

    public:

        GoogleDocContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }

        void constructObject( fh_GoogleClient gc, fh_GoogleDocument doc )
        {
            m_gc = gc;
            m_doc = doc;
        }

        string getFormat()
        {
            return "txt";
        }


        ferris_ios::openmode
            getSupportedOpenModes()
            {
                return
                    ios_base::in        |
                    ios_base::out       |
                    std::ios::trunc     |
                    std::ios::ate       |
                    std::ios::app       |
                    ios_base::binary    ;
            }

        void
        OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
            {
                if( !(m & std::ios::out) )
                    return;

                string format = getFormat();
                AdjustForOpenMode_Closing( ss, m, tellp );

                string data = StreamToString(ss);
                DEBUG << "OnStreamClosed() format:" << format << " data:" << data << endl;
                fh_stringstream zz;
                zz << data;
                m_doc->importFromFormat( zz, "txt" );
            }

        fh_stringstream
        real_getIOStream( ferris_ios::openmode m )
            {
                string format = getFormat();
//                fh_istream ret = m_doc->exportToFormat( format );
//                return ret;
                
                fh_stringstream ss;
                fh_istream iss = m_doc->exportToFormat( format );
                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(ss));
                ss->clear();
                ss->seekg(0, ios::beg);
                ss->seekp(0, ios::beg);
                ss->clear();
                
                return ss;
            }
        
        fh_iostream
        priv_getIOStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
                return ret;
            }
        fh_istream
        priv_getIStream( ferris_ios::openmode m )
            {
                fh_stringstream ret = real_getIOStream( m );
                return ret;
            }
        
        
    };
    
    class FERRISEXP_CTXPLUGIN GoogleDocsFolderContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef GoogleDocsFolderContext _Self;
        bool m_haveTriedToRead;
        fh_GoogleClient m_gc;
        fh_GoogleDocumentFolder m_folder;
        
    public:

        GoogleDocsFolderContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }

        void constructObject( fh_GoogleClient gc, fh_GoogleDocumentFolder f )
        {
            m_gc = gc;
            m_folder = f;
        }

        static fh_context SL_SubCreate_file( fh_context c, fh_context md )
            {
                return c->SubCreate_file( c, md );
            }

        virtual fh_context SubCreate_file( fh_context c, fh_context md )
            {
                std::string rdn      = getStrSubCtx( md, "name", "" );
                std::string v        = "";
                std::string format   = "txt";
                DEBUG << "SubCreate_file() rdn:" << rdn << " v:" << v << endl;

                if( GoogleDocsFolderContext* cc = dynamic_cast<GoogleDocsFolderContext*>(GetImpl(c)))
                {
                    fh_GoogleDocument doc = cc->m_folder->createDocument( v, rdn, format );

                    GoogleDocContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->constructObject( m_gc, doc );
                    return c;
                    // fh_context childc = cc->ensureContextCreated( rdn, true );
                    // return childc;
                }
                stringstream ss;
                ss << "Attempt to create a google subobject on a context that is not a google one!"
                   << " url:" << c->getURL()
                   << endl;
                Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
            }
        
        
        void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            DEBUG << "priv_FillCreateSubContextSchemaParts()" << endl;
            // m["dir"] = SubContextCreator( _Base::SL_commondb_SubCreate_dir,
            //                              "	<elementType name=\"dir\">\n"
            //                              "		<elementType name=\"name\" default=\"new directory\">\n"
            //                              "			<dataTypeRef name=\"string\"/>\n"
            //                              "		</elementType>\n"
            //                              "	</elementType>\n");
            m["file"] = SubContextCreator( _Self::SL_SubCreate_file,
                                         "	<elementType name=\"file\">\n"
                                         "		<elementType name=\"name\" default=\"new file\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");
            
        }

        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " m_haveTriedToRead:" << m_haveTriedToRead
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;

                    GoogleDocuments_t docs = m_folder->getDocuments();
                    for( GoogleDocuments_t::iterator iter = docs.begin(); iter != docs.end(); ++iter )
                    {
                        fh_GoogleDocument d = *iter;

                        string rdn = d->getTitle();
                        DEBUG << "child:" << rdn << endl;
                        GoogleDocContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( m_gc, d );
                        
                        
                    }

                    DEBUG << "Looking for subfolders... this:" << getURL() << endl;
                    GoogleDocumentFolders_t col = m_folder->getSubFolders();
                    for( GoogleDocumentFolders_t::iterator iter = col.begin(); iter != col.end(); ++iter )
                    {
                        fh_GoogleDocumentFolder f = *iter;
                        string rdn = f->getTitle();
                        DEBUG << "child:" << rdn << endl;
                        GoogleDocsFolderContext* c = 0;
                        c = priv_ensureSubContext( rdn, c );
                        c->constructObject( m_gc, f );
                    }
                    
                }
            
            }
        
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    class FERRISEXP_CTXPLUGIN GoogleYoutubeUploadContext
        :
        public WebServicesFileUploadContext< GoogleYoutubeUploadContext >
    {
        typedef WebServicesFileUploadContext< GoogleYoutubeUploadContext > _Base;
        
    public:

        fh_GoogleClient getGoogle();
        virtual fh_WebServicesUpload getWebServicesUpload()
        {
            if( !m_wsUpload )
            {
                fh_GoogleClient  g = getGoogle();
                m_wsUpload = g->createYoutubeUpload();
            }
            return m_wsUpload;
        }
        
        GoogleYoutubeUploadContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        
    };

    
//     class FERRISEXP_CTXPLUGIN GoogleYoutubeUploadContext
//         :
//         public leafContext
//     {
//         typedef leafContext _Base;
//         typedef GoogleYoutubeUploadContext _Self;

//         fh_YoutubeUpload m_YoutubeUpload;
//         string m_filename;
//         string m_title;
//         string m_desc;
//         string m_keywords;
//         string m_ContentType;
//         int    m_ContentLength;
        
//     public:

//         GoogleYoutubeUploadContext( Context* parent, const std::string& rdn )
//             :
//             _Base( parent, rdn ),
//             m_ContentLength( 0 )
//             {
//                 DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
//             }

//         fh_GoogleClient getGoogle();


//         string filenameToContextType( const std::string& s )
//         {
//             return "video/mp4";
//         }

        

//         ferris_ios::openmode
//             getSupportedOpenModes()
//             {
//                 return
//                     ios_base::in        |
//                     ios_base::out       |
//                     std::ios::trunc     |
//                     std::ios::ate       |
//                     std::ios::app       |
//                     ios_base::binary    ;
//             }

//         fh_YoutubeUpload getYoutubeUpload()
//         {
//             if( !m_YoutubeUpload )
//             {
//                 fh_GoogleClient  g = getGoogle();
//                 m_YoutubeUpload = g->createYoutubeUpload();
//             }
//             return m_YoutubeUpload;
//         }

//         void OnStreamClosed( fh_istream& ss, std::streamsize tellp, ferris_ios::openmode m )
//             {
//                 cerr << "OnStreamClosed()" << endl;
//                 DEBUG << "OnStreamClosed()" << endl;
//                 if( !(m & std::ios::out) )
//                     return;
//                 DEBUG << "OnStreamClosed() waiting..." << endl;
//                 m_YoutubeUpload->streamingUploadComplete();

//                 DEBUG << "OnStreamClosed video url:" << m_YoutubeUpload->getURL() << endl;
//                 DEBUG << "OnStreamClosed video  id:" << m_YoutubeUpload->getID() << endl;
//             }


        
//         virtual void priv_preCopyAction( fh_context c )
//             {
//                 DEBUG << "preCopyAction() c:" << c->getURL() << endl;
        
// //                fh_YoutubeUpload u = getYoutubeUpload();

//                 m_filename = c->getDirName();
//                 m_title = getStrAttr( c, "title", "" );
//                 m_title = getStrAttr( c, "description", "" );
//                 m_title = getStrAttr( c, "keywords", "" );
                 
//                 m_ContentType = getStrAttr( c, "mimetype" ,"" );
//                 m_ContentLength = toint( getStrAttr( c, "size", "200" ));
        
//                 DEBUG << "m_ContentLength:" << m_ContentLength << endl;
//             }

//         virtual void priv_postCopyAction( fh_context c )
//             {
//                 cerr  << "postCopyAction() c:" << c->getURL() << endl;
//                 DEBUG << "postCopyAction() c:" << c->getURL() << endl;
//                 // if( m_YoutubeUpload )
//                 // {
//                 //     m_YoutubeUpload->streamingUploadComplete();
//                 // }
//             }
        
        
//         fh_iostream
//         priv_getIOStream( ferris_ios::openmode m )
//             throw (FerrisParentNotSetError,
//                    AttributeNotWritable,
//                    CanNotGetStream,
//                    std::exception)
//             {
//                 fh_GoogleClient  g = getGoogle();
//                 fh_YoutubeUpload u = getYoutubeUpload();
                
//                 string filename = m_filename;
//                 string title = m_title;
//                 string desc = m_desc;
//                 string keywords = m_keywords;

//                 if( filename.empty() )
//                     filename = getDirName();
//                 if( title.empty() )
//                     title = filename;
//                 if( desc.empty() )
//                     desc = title;

//                 int ContentLength = 200;
//                 if( m_ContentLength )
//                     ContentLength = m_ContentLength;

//                 u->setFilename( filename );
//                 u->setTitle( title );
//                 u->setDescription( desc );
//                 u->setKeywords( keywords );
//                 u->setLength( ContentLength );

//                 string ContentType = filenameToContextType( filename );
//                 if( !m_ContentType.empty() )
//                     ContentType = m_ContentType;

//                 DEBUG << "  ContentType:" << ContentType << endl;
//                 DEBUG << "ContentLength:" << ContentLength << endl;
                
//                 fh_iostream ret = m_YoutubeUpload->createStreamingUpload( ContentType );
//                 ret->getCloseSig().connect( sigc::bind( sigc::mem_fun(*this, &_Self::OnStreamClosed ), m )); 
//                 return ret;
//             }
//         fh_istream
//         priv_getIStream( ferris_ios::openmode m )
//             throw (FerrisParentNotSetError,
//                    CanNotGetStream,
//                    std::exception)
//             {
//                 fh_stringstream ret;
//                 return ret;
//             }

//     };
    

    class FERRISEXP_CTXPLUGIN GoogleYoutubeUploadDirectoryContext
        :
        public WebServicesUploadDirectoryContext<
        GoogleYoutubeUploadDirectoryContext, GoogleYoutubeUploadContext >
    {
        typedef WebServicesUploadDirectoryContext<
        GoogleYoutubeUploadDirectoryContext, GoogleYoutubeUploadContext > _Base;
        
    public:
        fh_GoogleClient getGoogle();
        GoogleYoutubeUploadDirectoryContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
            }
    };

    
    // class FERRISEXP_CTXPLUGIN GoogleYoutubeUploadDirectoryContext
    //     :
    //     public FakeInternalContext
    // {
    //     typedef FakeInternalContext _Base;
    //     typedef GoogleYoutubeUploadDirectoryContext _Self;

        
    // public:

    //     GoogleYoutubeUploadDirectoryContext( Context* parent, const std::string& rdn )
    //         :
    //         _Base( parent, rdn )
    //         {
    //             DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
    //         }

    //     fh_GoogleClient getGoogle();

    //     bool isDir()
    //         {
    //             return true;
    //         }

    //     void createStateLessAttributes( bool force = false )
    //         {
    //             if( force || isStateLessEAVirgin() )
    //             {
    //                 _Base::createStateLessAttributes( true );
    //                 supplementStateLessAttributes( true );
    //             }
    //         }

        

    //     static fh_context SL_SubCreate_file( fh_context c, fh_context md )
    //         {
    //             return c->SubCreate_file( c, md );
    //         }

    //     virtual fh_context SubCreate_file( fh_context c, fh_context md )
    //         {
    //             string rdn         = getStrSubCtx( md, "name", "" );
    //             string v           = "";
    //             DEBUG << "SubCreate_file() rdn:" << rdn << " v:" << v << endl;

    //             if( GoogleYoutubeUploadDirectoryContext* cc = dynamic_cast<GoogleYoutubeUploadDirectoryContext*>(GetImpl(c)))
    //             {
    //                 fh_GoogleClient g = cc->getGoogle();
                    
    //                 GoogleYoutubeUploadContext* c = 0;
    //                 c = priv_ensureSubContext( rdn, c );
    //                 return c;
    //             }
    //             fh_stringstream ss;
    //             ss << "Attempt to create a google subobject on a context that is not a google one!"
    //                << " url:" << c->getURL()
    //                << endl;
    //             Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
    //         }
        
    //     void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
    //     {
    //         DEBUG << "priv_FillCreateSubContextSchemaParts()" << endl;
    //         m["file"] = SubContextCreator( _Self::SL_SubCreate_file,
    //                                      "	<elementType name=\"file\">\n"
    //                                      "		<elementType name=\"name\" default=\"new file\">\n"
    //                                      "			<dataTypeRef name=\"string\"/>\n"
    //                                      "		</elementType>\n"
    //                                      "	</elementType>\n");
    //     }


        
        
    //     void priv_read()
    //         {
    //             DEBUG << "priv_read() url:" << getURL()
    //                   << " have read:" << getHaveReadDir()
    //                   << endl;
    //             EnsureStartStopReadingIsFiredRAII _raii1( this );
    //             emitExistsEventForEachItemRAII    _raii2( this );
    //         }
    // };
    

    class FERRISEXP_CTXPLUGIN GoogleYoutubeRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef GoogleYoutubeRootContext _Self;
        fh_GoogleClient m_gc;
        
    public:

        GoogleYoutubeRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
            }
        void constructObject( fh_GoogleClient gc )
        {
            m_gc = gc;
        }
        fh_GoogleClient getGoogle()
        {
            return m_gc;
        }
        

        
        void priv_read()
            {
                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( !empty() )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    GoogleYoutubeUploadDirectoryContext* c = 0;
                    c = priv_ensureSubContext( "upload", c );
                }
            
            }
    };

    fh_GoogleClient
    GoogleYoutubeUploadDirectoryContext::getGoogle()
    {
        GoogleYoutubeRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getGoogle();
    }
    fh_GoogleClient
    GoogleYoutubeUploadContext::getGoogle()
    {
        GoogleYoutubeRootContext* p = 0;
        p = getFirstParentOfContextClass( p );
        return p->getGoogle();
    }

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    class RateLimiter
    {
        int m_secsBetweenCalls;
        time_t m_tt;
    public:
        enum 
        {
            WEB_READ_RATE = 10
        };
        
        RateLimiter( int secsBetweenCalls )
            : m_secsBetweenCalls(secsBetweenCalls)
            , m_tt(0)
        {
        }
        bool tooQuick()
        {
            time_t n = Time::now();

            // first call can be made right now
            if( !m_tt )
            {
                m_tt = n;
                return false;
            }

            // made a call, was it long enough ago?
            if( m_tt + m_secsBetweenCalls > n )
                return true;

            // can make the call
            m_tt = n;
            return false;
        }
    };
    
    
    // typedef fh_istream (*SL_getXStream)( Context* c, const std::string& rdn, EA_Atom* atom );
    // struct MD 
    // {
    //     string name;
    //     SL_getXStream func;
    //     int type;
    //     MD( string name, SL_getXStream func, int type )
    //         : name(name), func(func), type(type)
    //     {
    //     }
    // };
    // typedef list<MD> MDList_t;
        // MDList_t& getMDList() 
        // {
        //     static MDList_t ret;
        //     return ret;
        // }
    
    class FERRISEXP_CTXPLUGIN GoogleDriveContext
        :
        public StateLessEAHolder< GoogleDriveContext, FakeInternalContext >
    {
        typedef GoogleDriveContext _Self;
        typedef StateLessEAHolder< GoogleDriveContext, FakeInternalContext > _Base;

        fh_GDriveFile m_file;

        static fh_istream SL_getSizeStream( GoogleDriveContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;
            ss << c->m_file->m_sz;
            return ss;
        }


        
        // static fh_istream SL_getIDStream( Context* cc, const std::string& rdn, EA_Atom* atom )
        // {
        //     GoogleDriveContext* c = (GoogleDriveContext*)cc;
        //     fh_stringstream ss;
        //     ss << c->m_file->m_id;
        //     return ss;
        // }
#define ACCESSOR( ATTR )                                                 \
        static fh_iostream SL_getStream_##ATTR( GoogleDriveContext* c, const std::string& rdn, EA_Atom* atom ) \
        {                                                               \
            fh_stringstream ss;                                         \
            ss << c->m_file->ATTR;                                      \
            return ss;                                                  \
        }
        
        ACCESSOR( m_id );
        ACCESSOR( m_etag );
        ACCESSOR( m_title );
        ACCESSOR( m_desc );
        ACCESSOR( m_ext );
        ACCESSOR( m_md5 );
        ACCESSOR( m_ctime );
        ACCESSOR( m_mtime );
        ACCESSOR( m_mmtime );
        ACCESSOR( m_mime );

#define SETTER( ATTR, GOOGLEATTR )                               \
    static void SL_setStream_##ATTR( GoogleDriveContext* c,      \
                                     const std::string& rdn,     \
                                     EA_Atom* atom,              \
                                     fh_istream iss )            \
    {                                                            \
      string nv = StreamToString(iss);                             \
                                                                   \
      DEBUG << "setStream() " << #ATTR << " v:" << nv << endl;    \
      try                                                          \
      {                                                            \
          c->m_file->updateMetadata( GOOGLEATTR, nv );             \
          c->m_file->ATTR = nv;                                    \
      }                                                            \
      catch( exception& WebAPIException )                          \
      {                                                            \
          throw;                                                   \
      }                                                            \
    }
    
        SETTER( m_title, "title" );
        SETTER( m_desc,  "description" );
        SETTER( m_mime,  "mimeType" );


        static fh_iostream SL_getStream_Shares( GoogleDriveContext* c, const std::string& rdn, EA_Atom* atom )
        {
            fh_stringstream ss;

            GDrivePermissions_t pl = c->m_file->readPermissions();
            for( GDrivePermissions_t::iterator iter = pl.begin(); iter != pl.end(); ++iter )
            {
                fh_GDrivePermission p = *iter;
                ss << p->getRoleString() << "," << p->m_name << endl;
            }
            
            return ss;
        }
        
        static void SL_setStream_Shares( GoogleDriveContext* c, const std::string& rdn, EA_Atom* atom, fh_istream iss )
        {
            string s = StreamToString(iss);
            s = chomp(s);
            DEBUG << "SL_setStream_Shares() s:" << s << endl;
            stringlist_t sl = Util::parseCommaSeperatedList( s );
            c->m_file->sharesAdd( sl );            
        }
    
        
        virtual std::string priv_getMimeType( bool fromContent = false )
        {
            return m_file->m_mime;
        }
        
        
        void createStateLessAttributes( bool force = false )
        {
            if( force || isStateLessEAVirgin() )
            {
                _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                SLEA( "size",            &_Self::SL_getSizeStream, FXD_FILESIZE );
                SLEA( "dontfollow-size", &_Self::SL_getSizeStream, FXD_FILESIZE );

                SLEA( "id",            &_Self::SL_getStream_m_id, XSD_BASIC_STRING );
                SLEA( "etag",          &_Self::SL_getStream_m_etag, XSD_BASIC_STRING );
                SLEA( "title",
                      &_Self::SL_getStream_m_title, &_Self::SL_getStream_m_title,
                      &_Self::SL_setStream_m_title, XSD_BASIC_STRING );
                SLEA( "description",
                      &_Self::SL_getStream_m_desc, &_Self::SL_getStream_m_desc,
                      &_Self::SL_setStream_m_desc, XSD_BASIC_STRING );
                SLEA( "extension",     &_Self::SL_getStream_m_ext,  XSD_BASIC_STRING );
                SLEA( "md5",           &_Self::SL_getStream_m_md5,  XSD_BASIC_STRING );
                SLEA( "create-time",   &_Self::SL_getStream_m_ctime, FXD_UNIXEPOCH_T );
                SLEA( "mtime",         &_Self::SL_getStream_m_mtime, FXD_UNIXEPOCH_T );
                SLEA( "mtime-me",      &_Self::SL_getStream_m_mmtime, FXD_UNIXEPOCH_T );
                SLEA( "mimetype",
                      &_Self::SL_getStream_m_mime, &_Self::SL_getStream_m_mime,
                      &_Self::SL_setStream_m_mime, XSD_BASIC_STRING );
                SLEA( "mime",
                      &_Self::SL_getStream_m_mime, &_Self::SL_getStream_m_mime,
                      &_Self::SL_setStream_m_mime, XSD_BASIC_STRING );
                SLEA( "width",     &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "height",    &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                
                supplementStateLessAttributes_timet( "create-time" );
                supplementStateLessAttributes_timet( "mtime" );
                supplementStateLessAttributes_timet( "mtime-me" );

                SLEA( "shares",
                      &_Self::SL_getStream_Shares, &_Self::SL_getStream_Shares,
                      &_Self::SL_setStream_Shares,
                      XSD_BASIC_STRING );
                
#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }

        ferris_ios::openmode
        getSupportedOpenModes()
            {
                return ios::in | ios::out | ios::trunc | ios::binary;
            }
        
        virtual fh_istream  priv_getIStream( ferris_ios::openmode m )
        {
            fh_istream iss = m_file->getIStream();
            return iss;
        }
        
        virtual fh_iostream priv_getIOStream( ferris_ios::openmode m )
        {
            fh_iostream oss = m_file->getIOStream();
            return oss;
        }

        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",id,title,mtime-display,mtime-me-display,";
        }

        void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            DEBUG << "priv_FillCreateSubContextSchemaParts()" << endl;
        }
        
    public:

        GoogleDriveContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
                createStateLessAttributes();
            }
        void constructObject( fh_GDriveFile f )
        {
            m_file = f;
        }
        fh_GDriveClient getDrive();
        
        
        // void priv_read()
        //     {
        //         EnsureStartStopReadingIsFiredRAII _raii1( this );
        //     }
    };
    
    
    class FERRISEXP_CTXPLUGIN GoogleDriveRootContext
        :
        public StateLessEAHolder< GoogleDriveRootContext, FakeInternalContext >
    {
        typedef GoogleDriveRootContext _Self;
        typedef StateLessEAHolder< GoogleDriveRootContext, FakeInternalContext > _Base;
        fh_GDriveClient m_drive;

        void createStateLessAttributes( bool force = false )
        {
            if( force || isStateLessEAVirgin() )
            {
                _Base::createStateLessAttributes( true );

#define SLEA  tryAddStateLessAttribute         

                SLEA( "id",            &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "etag",          &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "title",
                      &_Self::SL_getNothingStream, &_Self::SL_getNothingStream,
                      &_Self::SL_setNothingStream, XSD_BASIC_STRING );
                SLEA( "description",
                      &_Self::SL_getNothingStream, &_Self::SL_getNothingStream,
                      &_Self::SL_setNothingStream, XSD_BASIC_STRING );
                SLEA( "extension",     &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "md5",           &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                SLEA( "create-time",   &_Self::SL_getNothingStream, FXD_UNIXEPOCH_T );
                SLEA( "mtime",         &_Self::SL_getNothingStream, FXD_UNIXEPOCH_T );
                SLEA( "mtime-me",      &_Self::SL_getNothingStream, FXD_UNIXEPOCH_T );
                SLEA( "mtime",         &_Self::SL_getNothingStream, FXD_UNIXEPOCH_T );
                SLEA( "mimetype",      &_Self::SL_getNothingStream, XSD_BASIC_STRING );
                
                supplementStateLessAttributes_timet( "create-time" );
                supplementStateLessAttributes_timet( "mtime" );
                supplementStateLessAttributes_timet( "mtime-me" );

#undef SLEA                    
                    
                supplementStateLessAttributes( true );
            }
        }
        
        virtual std::string getRecommendedEA()
        {
            return _Base::getRecommendedEA() + ",id,title,mtime-display,mtime-me-display,";
        }

        static fh_context SL_SubCreate_file( fh_context c, fh_context md )
            {
                return c->SubCreate_file( c, md );
            }

        virtual fh_context SubCreate_file( fh_context c, fh_context md )
        {
            std::string rdn      = getStrSubCtx( md, "name", "" );
            std::string v        = "";
            std::string format   = "txt";
            DEBUG << "SubCreate_file() rdn:" << rdn << " v:" << v << endl;

            if( GoogleDriveRootContext* cc = dynamic_cast<GoogleDriveRootContext*>(GetImpl(c)))
            {
                fh_GDriveFile factory = new GDriveFile( getDrive(), QVariantMap() );
                DEBUG << "creating a file with title:" << rdn << endl;
                fh_GDriveFile f = factory->createFile( rdn );
                
                GoogleDriveContext* c = 0;
                c = priv_ensureSubContext( rdn, c );
                c->constructObject( f );
                return c;
            }
            stringstream ss;
            ss << "Attempt to create a google subobject on a context that is not a google one!"
               << " url:" << c->getURL()
               << endl;
            Throw_FerrisCreateSubContextNotSupported( tostr(ss), GetImpl(c) );
        }
        
        void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m )
        {
            DEBUG << "priv_FillCreateSubContextSchemaParts()" << endl;
            m["file"] = SubContextCreator( _Self::SL_SubCreate_file,
                                         "	<elementType name=\"file\">\n"
                                         "		<elementType name=\"name\" default=\"new file\">\n"
                                         "			<dataTypeRef name=\"string\"/>\n"
                                         "		</elementType>\n"
                                         "	</elementType>\n");
            
        }
        
    public:

        GoogleDriveRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                DEBUG << "ctor, have read:" << getHaveReadDir() << endl;
                createStateLessAttributes();
            }
        void constructObject( fh_GoogleClient gc )
        {
            m_drive = GDriveClient::getGDriveClient();
        }
        fh_GDriveClient getDrive()
        {
            return m_drive;
        }
        
        bool isDir()
        {
            return true;
        }
        
        
        void priv_read()
            {
                fh_GDriveClient drive = getDrive();

                DEBUG << "priv_read() url:" << getURL()
                      << " have read:" << getHaveReadDir()
                      << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );

                static RateLimiter rl( RateLimiter::WEB_READ_RATE );
                if( rl.tooQuick() )
                {
                    DEBUG << "read() called again to quick, not really doing it" << endl;
                    emitExistsEventForEachItemRAII _raii1( this );
                    return;
                }
                
                DEBUG << "really read()ing from google://drive **************" << endl;
                clearContext();
                files_t fl = drive->filesList();
                for( files_t::iterator iter = fl.begin(); iter != fl.end(); ++iter )
                {
                    fh_GDriveFile f = *iter;

                    string rdn = f->m_title;
                    if( rdn.empty() )
                        rdn = f->m_rdn;
                    rdn = monsterName( rdn );
                    DEBUG << "adding child rdn:" << rdn << endl;
                    GoogleDriveContext* c = 0;
                    c = priv_ensureSubContext( rdn, c );
                    c->constructObject( f );
                }
                emitExistsEventForEachItemRAII _raii2( this );
            }
    };
    

    fh_GDriveClient
    GoogleDriveContext::getDrive()
    {
        GoogleDriveRootContext* rc = 0;
        rc = getFirstParentOfContextClass( rc );
        return rc->getDrive();
    }
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    class FERRISEXP_CTXPLUGIN GoogleRootContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        bool m_haveTriedToRead;
    public:

        GoogleRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn ),
            m_haveTriedToRead( false )
            {
                LG_GOOGLE_D << "ctor, have read:" << getHaveReadDir() << endl;
            }

        void priv_read()
            {
                LG_GOOGLE_D << "priv_read() url:" << getURL()
                           << " m_haveTriedToRead:" << m_haveTriedToRead
                           << " have read:" << getHaveReadDir()
                           << endl;
                EnsureStartStopReadingIsFiredRAII _raii1( this );
                
                if( m_haveTriedToRead )
                {
                    emitExistsEventForEachItemRAII _raii1( this );
                }
                else
                {
                    m_haveTriedToRead = true;
                    fh_context c;

                    fh_GoogleClient gc = Factory::createGoogleClient();

                    {
                        GoogleDocsFolderContext* c = 0;
                        c = priv_ensureSubContext( "docs", c );
                        fh_GoogleDocumentFolder f = gc->getRootFolder();
                        c->constructObject( gc, f );
                    }
                    {
                        GoogleSpreadsheetDirectoryContext* c = 0;
                        c = priv_ensureSubContext( "spreadsheets", c );
                        c->constructObject( gc );
                    }
                    {
                        GoogleYoutubeRootContext* c = 0;
                        c = priv_ensureSubContext( "youtube", c );
                        c->constructObject( gc );
                    }
                    {
                        GoogleDriveRootContext* c = 0;
                        c = priv_ensureSubContext( "drive", c );
                        c->constructObject( gc );
                    }
                }
            }
    };

    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
        {
            try
            {
                LG_GOOGLE_D << "Brew()" << endl;

                static GoogleRootContext* c = 0;
                if( !c )
                {
                    c = new GoogleRootContext(0, "/" );
                    // Bump ref count.
                    static fh_context keeper = c;
                    static fh_context keeper2 = keeper;
                }
                fh_context ret = c;

                return ret;
            }
            catch( FerrisSqlServerNameNotFound& e )
            {
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
            catch( exception& e )
            {
                LG_PG_W << "e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }

};
    
