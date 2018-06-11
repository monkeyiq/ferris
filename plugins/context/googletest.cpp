#include <config.h>

#include "libferrisgoogle_shared.hh"
#include <Ferris/Ferris.hh>

using namespace Ferris;
using namespace std;


#include <QCoreApplication>

    string newCellV;
    string newCellV2;

int simpletest( int argc, char** argv )
{
    fh_GoogleClient g = Factory::createGoogleClient();
    g->localtest();
//    Main::mainLoop();
    return 0;
}




int simpletestBAK( int argc, char** argv )
{
    QCoreApplication a(argc, argv);
    fh_GoogleClient g = Factory::createGoogleClient();
    g->localtest();
    return a.exec();
}


ostream& spaces( int indent )
{
    for( int i=0; i<indent; ++i )
        cerr << " ";
    return cerr;
}

void explore_tree( fh_GoogleClient g, fh_GoogleDocumentFolder f, int indent = 0 )
{
    spaces(indent) << "title:" << f->getTitle() << endl;
    GoogleDocuments_t docs = f->getDocuments();

    for( GoogleDocuments_t::iterator iter = docs.begin(); iter != docs.end(); ++iter )
    {
        spaces(indent) << "  doc:" << (*iter)->getTitle() << endl;

        // if( (*iter)->getTitle() == "sub3doc" )
        // {
        //     fh_GoogleDocument d = *iter;
        //     fh_istream iss = d->exportToFormat( "txt" );
        //     ofstream oss("/tmp/dumpy.txt");
        //     copy( istreambuf_iterator<char>(iss),
        //           istreambuf_iterator<char>(),
        //           ostreambuf_iterator<char>(oss));

        //     fh_stringstream ss;
        //     ss << newCellV;
        //     d->importFromFormat( ss, "txt" );
        // }
    }

    
    GoogleDocumentFolders_t col = f->getSubFolders();
    for( GoogleDocumentFolders_t::iterator iter = col.begin(); iter != col.end(); ++iter )
    {
        explore_tree( g, *iter, indent+4 );
    }
}

fh_YoutubeUpload u = 0;
void OnStreamClosed( fh_istream& ss, std::streamsize tellp )
{
    cerr << "OnStreamClosed()" << endl;
    if( u )
    {
        cerr << "OnStreamClosed(A) u:" << GetImpl(u) << endl;
        u->streamingUploadComplete();
        cerr << "OnStreamClosed(ok)" << endl;
    }
}

int main( int argc, char** argv )
{
//    return simpletest( argc, argv );

    newCellV  = argv[1];
    newCellV2 = argv[2];
    
    
    
    userpass_t up = getGoogleUserPass();
    cerr << "user:" << up.first << " pass:" << up.second << endl;

    fh_GoogleClient g = Factory::createGoogleClient();
//    g->Authenticate_ClientLogin( up.first, up.second );

    // GoogleDocuments_t docs = g->listDocuments();
    // for( GoogleDocuments_t::iterator si = docs.begin(); si!=docs.end(); ++si )
    // {
    //     fh_GoogleDocument d = *si;
    //     cerr << " doc:" << d->getTitle() << endl;
    //     cerr << "feed:" << d->getEditURL() << endl;
    // }

    if( 1 )
    {
        u = g->createYoutubeUpload();
        u->AddRef();
        
        int ContentLength = 271978;
//        ContentLength = toint(getStrAttr( inputc, "size", "100" ));
        u->setFilename( "test.mp4" );
        u->setTitle( "test.mp4" );
        u->setDescription( "test.mp4" );
        u->setKeywords( "test.mp4" );
        u->setLength( ContentLength );
        
        string ContentType = "video/mp4";
        {
            fh_istream  iss = Factory::fcin();
            fh_iostream oss = u->createStreamingUpload( ContentType );
            cerr << "have oss" << endl;
            oss->getCloseSig().connect( sigc::ptr_fun( &OnStreamClosed ) );
            cerr << "connected oss" << endl;
            copy( istreambuf_iterator<char>(iss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(oss));
            oss  << flush;
            cerr << "finished writing..." << endl;
        }
//         u->streamingUploadComplete();
        cerr << "done." << endl;
        exit(0);
    }
    

    if( 0 )
    {
        fh_GoogleDocumentFolder f = g->getRootFolder();
        fh_GoogleDocument doc = f->createDocument( newCellV, newCellV2, "text/plain" );
        exit(0);
    }

    if( 0 )
    {
        fh_GoogleDocumentFolder f = g->getRootFolder();
        GoogleDocuments_t docs = f->getDocuments();

        for( GoogleDocuments_t::iterator iter = docs.begin(); iter != docs.end(); ++iter )
        {
            fh_GoogleDocument d = *iter;
            cerr << "  doc:" << d->getTitle() << endl;
            
            if( d->getTitle() == newCellV2 )
            {
                cerr << "UPDATING....  doc:" << d->getTitle() << endl;
                fh_stringstream ss;
                ss << newCellV;
                d->importFromFormat( ss, "txt" );
            }
        }
        exit(0);
    }
    
    
    if( 0 )
    {
        fh_GoogleDocumentFolder f = g->getRootFolder();
        explore_tree( g, f );
        exit(0);
    }

    if( 0 )
    {
        fh_GoogleDocumentFolder f = g->getRootFolder();
        GoogleDocuments_t docs = f->getDocuments();

        for( GoogleDocuments_t::iterator iter = docs.begin(); iter != docs.end(); ++iter )
        {
            fh_GoogleDocument d = *iter;
            cerr << "  doc:" << d->getTitle() << endl;
            if( d->getTitle() == "spready.ods" )
            {
                cerr << "uploading update..." << endl;
                fh_ifstream iss("/tmp/dumpy.ods");
                d->importFromFormat( iss, "ods" );
            }
        }
        exit(0);
    }


    if( 1 )
    {
        fh_GoogleDocumentFolder f = g->getRootFolder();
        GoogleDocuments_t docs = f->getDocuments();

        for( GoogleDocuments_t::iterator iter = docs.begin(); iter != docs.end(); ++iter )
        {
            fh_GoogleDocument d = *iter;
            cerr << "  doc:" << d->getTitle() << endl;
            if( d->getTitle() == "foo2.txt" )
            {
                fh_istream iss = d->exportToFormat( "txt" );
                ofstream oss("/tmp/dumpy.txt");
                copy( istreambuf_iterator<char>(iss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(oss));
            }
        }
        exit(0);
    }
    
    
    GoogleSpreadSheets_t sheets = g->listSheets();
    for( GoogleSpreadSheets_t::iterator si = sheets.begin(); si!=sheets.end(); ++si )
    {
        cerr << "sheet:" << (*si)->getTitle() << endl;
        cerr << " feed:" << (*si)->getFeedURL() << endl;

        
        if( (*si)->getTitle() == "foo2.txt" )
        {
            fh_istream iss = (*si)->exportToFormat( "txt" );
            ofstream oss("/tmp/dumpy.txt");
            copy( istreambuf_iterator<char>(iss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(oss));
        }


    }
    exit(0);



    
    for( GoogleSpreadSheets_t::iterator si = sheets.begin(); si!=sheets.end(); ++si )
    {
        cerr << "sheet:" << (*si)->getTitle() << endl;
        cerr << " feed:" << (*si)->getFeedURL() << endl;
        
#if 0
        {
            fh_GoogleSpreadSheet ss = *si;
            ss->createWorkSheet( "junk_number_2" );
            cerr << "Created..." << endl;
            
        }
#endif   
        
        GoogleWorkSheets_t ws = (*si)->listSheets();
        for( GoogleWorkSheets_t::iterator wi = ws.begin(); wi!=ws.end(); ++wi )
        {
            fh_GoogleWorkSheet ws = *wi;
            
            cerr << "    work sheet:" << ws->getTitle() << endl;
            cerr << "          feed:" << ws->getCellFeedURL() << endl;
            // cerr << " --- fetching cells --- " << endl;
            // ws->fetchCells();

#if 0
            cerr << "cell 5.3 value:" << ws->getCell( 5, 3 )->value() << endl;
            cerr << "setting to value:" << newCellV << endl;
            ws->getCell(5,3)->value( newCellV );
            sleep(1);
            cerr << "setting to value2:" << newCellV2 << endl;
            ws->getCell(5,3)->value( newCellV2 );

#endif


#if 0
            cerr << "------------- updating existing... ---------------------" << endl;
            ws->getCell(4,3)->value( newCellV );
            ws->getCell(5,3)->value( newCellV );
            cerr << "------------- updating NEW... ---------------------" << endl;
            ws->getCell(5,2)->value( newCellV2 );
#endif
            
#if 0
            ws->setDelayCellSync( true );
            cerr << "setting maany values in a little chunk... v2:" << newCellV2 << endl;
            ws->getCell(4,3)->value( newCellV );
            ws->getCell(5,3)->value( newCellV );
            ws->getCell(6,3)->value( newCellV );
            ws->getCell(15,6)->value( newCellV2 );
            ws->getCell(15,7)->value( newCellV2 );
            ws->getCell(15,8)->value( newCellV2 );
            cerr << "calling sync()" << endl;
            ws->sync();
#endif

#if 0
            stringlist_t colnames = ws->getColumnNames();
            cerr << "colanmes:" << endl;
            copy( colnames.begin(), colnames.end(), ostream_iterator<string>(cerr,"\n"));

            {
                string v = ws->getCell( 2, "person" )->value();
                cerr << "value of person col for row 2 is:" << v << endl;
            }
            cerr << "value of 2 col for row 2 is:" << ws->getCell( 2, "b" )->value() << endl;
#endif




cerr << "done..." << endl;
        }
    }
    
//    g->getCells();
    
            
    

    return 0;
}
