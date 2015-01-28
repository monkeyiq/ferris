
#include "libferrisvimeo_shared.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>

using namespace Ferris;
using namespace Ferris::Vimeo;

using namespace std;

#include <QCoreApplication>


string arg1;
string arg2;


string trim( const std::string& s, int limit = 80 )
{
    return s.substr( 0, 80 );
}






int main( int argc, char** argv )
{
//    return simpletest( argc, argv );
    
    arg1 = argv[1];
    arg2 = argv[2];
 

    fh_vimeo v = Factory::getVimeo();
    std::string s;
    
    // std::string frob = v->getFrob();    
    // cerr << "frob:" << frob << endl;
    // s = v->createFrobAuthLink( frob );
    // cerr << "s:" << s << endl;

    s = v->testLogin();
    cerr << "result:" << s << endl;

    fh_Upload u = v->createUpload();
    {
        fh_iostream z = u->createStreamingUpload("video/mp4");
        ifstream iss( "/tmp/small-video-test.mp4" );

        int bufsz = 4096;
        char buf[bufsz+1];
        while( true )
        {
            iss.read( buf, bufsz );
            int count = iss.gcount();
            cerr << "copying byte count:" << count << endl;
            if( count < 0 || iss.eof() )
                break;
            z.write( buf, count );
        }
    
        // char ch;
        // while( iss >> noskipws >> ch )
        //     z << ch;
        z << flush;
    }
    u->streamingUploadComplete();
    
    return 0;
}
