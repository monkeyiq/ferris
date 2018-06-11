
#include "libferrisgstreamer_shared.hh"

using namespace Ferris;
using namespace Ferris::GStreamer;
using namespace std;


string arg1;


int main( int argc, char** argv )
{
//    return simpletest( argc, argv );
    
    arg1 = argv[1];


    gst_init (&argc, &argv);

    string metadataURL = "/Develop/play/ferris/gstreamerin-pipe.txt";
    string pipeDesc = getStrSubCtx( metadataURL, "", true, true );
    fh_gstreamer_readFrom_stream ss( pipeDesc );


    ofstream dumpss("/tmp/dumpss");
    int gc=0;
    int bufsz = 4096;
    char buf[ bufsz + 1];
    while( ss.good() )
    {
        ss.read( buf, bufsz );
        
        gc = ss.gcount();
        dumpss.write( buf, gc );
        dumpss << flush;
          
          // char c;
          // ss >> noskipws >> c;
          // dumpss << c;
          cerr << "iteration...gc:" << gc << endl;
          g_main_iteration( false );
      }
     
    return 0;
}
