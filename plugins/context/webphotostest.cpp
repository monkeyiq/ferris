
#include "libferriswebphotos_shared.hh"
#include <Ferris/Ferris.hh>

using namespace Ferris;
using namespace std;

#include <QCoreApplication>

string arg1;
string arg2;

int main( int argc, char** argv )
{
//    return simpletest( argc, argv );

    arg1 = argv[1];
    arg2 = argv[2];
    

    cerr << "getting flickr..." << endl;
    fh_webPhotos wf = Factory::getDefaultFlickrWebPhotos();
    cerr << "username:" << wf->getUserName() << endl;
    photolist_t r = wf->getMyRecent();
    cerr << "r.sz:" << r.size() << endl;

    return 0;
}
