/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: logtest.cpp,v 1.1 2006/12/07 07:02:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>

using namespace std;
using namespace Ferris;

int debugMode = 1;

#define LOG_SWITCH(v)           \
     ((v==1)              \
         ? getRealSS()         \
         : getSS())




static NullStream<> getSS_NULL;
inline fh_ostream getSS()
{
//    cerr << "getSS()" << endl;

//    return fh_ostream( getSS_NULL.rdbuf(), getSS_NULL.rdbuf() );

    return getSS_NULL;
    
}

// inline fh_ostream getZZ()
// {
//     Ferris_commonstream<char>* c = &getSS_NULL;
//     return fh_ostream( c->rdbuf() );
// }



inline fh_ostream& getRealSS()
{
    static fh_stringstream ss;
    return ss;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class nullstream
{
public:
    inline nullstream() {}
    inline ~nullstream() {}
    inline nullstream &operator<<(int )  { return *this; }
    inline nullstream &operator<<(const char *) { return *this; }
    inline nullstream &operator<<(const string&) { return *this; }
};


nullstream getNS()
{
    if( debugMode ) 
    return nullstream();
}


#define GETNS(v) ( (v) ? getRealSS() : getNS() )




inline fhl_ostream& getos( int fac, int dmod )
{
    static basic_stringbuf<char> strbuf;
    static null_streambuf<char>  nullbuf;
    static fhl_ostream ss( &nullbuf );
    static int msk = 0x111;

    if( dmod ^ msk )
    {
        if( dmod )
        {
            /* Debug mode */
            cerr <<"switch to slow\n";
            ss.rdbuf( &strbuf );
            ss.clear();
        }
        else
        {
            /* fast null mode */
            cerr <<"switch to fast\n";
//          ss.rdbuf( &nullbuf );
            ss.clear( ios_base::failbit );
        }
    }
    msk = dmod;
    return ss;
}



int main( int argc, char** argv )
{
    NullStream<> ns;
    fh_ostringstream rs;
    int sixnine = 69;

    string s1 = "somedir/somelib.so";
    string s2 = "somedir/somelib_factory.so";
    string facend = "_factory.so";
    
    cerr << "s1:" << s1 << " ends:" << ends_with( s1, facend ) << endl;
    cerr << "s2:" << s2 << " ends:" << ends_with( s2, facend ) << endl;
    
    if( argc > 1 )
    {
        cerr << "no debug" << endl;
        debugMode = 0;
    }

    cerr << "debugMode:" << debugMode << endl;
    

    
    rs << "Hello there " << " more " << sixnine << "\n";
    cerr << tostr(rs);

    
    {
        NullStream< f_ostream > ns;
    }
    
    string samplestr1 = "hi this is a small smaple std::string for testing";

    fh_ostream genericss = getSS();
    int faculty = 10;
    
    cerr << "start " << endl;
    for( int i=0; i<1000000; ++i )
    {
//        ns << "Hello there " << " more " << samplestr1 << endl;
//        genericss <<  "Hello there " << " more " << samplestr1 << endl;
        
//        getSS() <<  "Hello there " << " more " << samplestr1 << endl;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//         getNS()
//             <<  "Hello there "
//             << " more "
//             << samplestr1
//             << "\n";

//         GETNS(debugMode) <<  "Hello there "
//             << " more "
//             << samplestr1
//             << "\n";

        getos( faculty, debugMode )
            <<  "Hello there "
            << " more "
            << samplestr1
            << "\n";

//         if( i == 500000 )
//             debugMode = 0;
        
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
        
//        LOG_SWITCH(debugMode) <<  "Hello there " << " more " << samplestr1 << endl;
//        logswitch(debugMode) <<  "Hello there " << " more " << samplestr1 << endl;
//        getZZ() <<  "Hello there " << " more " << samplestr1 << endl;
        
    }
    cerr << "end " << endl;

//     ns << "Something " << endl;
//     Logging::LG_STRF::Instance().unset( Timber::PRI_WARNING );
//     LG_STRF_W << "Warning!" << endl;


    
    
    
}









