#include <config.h>

#include "libferrisfacebook_shared.hh"
#include <Ferris/Ferris.hh>
#include <Ferris/FerrisQt_private.hh>
#include <Ferris/FerrisKDE.hh>

using namespace Ferris;
using namespace Ferris::Facebook;

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
 
    

    fh_facebook fb = Factory::getFacebook();
//    cerr << "status:" << fb->getStatus() << endl;
    
//    fb->setStatus(arg1);

    // PostMap_t posts = fb->getRecentStreamPosts();
    // for( PostMap_t::iterator pi = posts.begin(); pi != posts.end(); ++pi )
    // {
    //     fh_Post p = pi->second;
    //     cerr << "post:" << p->getID() << " message:" << trim(p->getMessage()) << endl;
    
    //     CommentMap_t comments = p->getComments(); 
    //     for( CommentMap_t::iterator ci = comments.begin(); ci != comments.end(); ++ci )
    //     {
    //         fh_Comment c = ci->second;
    //         cerr << "    comment:" << c->getID() << " message:" << trim(c->getMessage()) << endl;
    //     }
    // }

    ContactMap_t friends = fb->getFriends();
    for( ContactMap_t::iterator ci = friends.begin(); ci != friends.end(); ++ci )
    {
        fh_Contact c = ci->second;
        if( c->getID() == "1110121020" )
        {
            cerr << "uid:" << c->getID() << " name:" << c->getName() << endl;
            cerr << c->getVCard() << endl;
        }
    }
    
    
    
    

    return 0;
}
