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

    $Id: ostream_modifiers.cpp,v 1.3 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

//#include <Ferris.hh>
#include <SignalStreams.hh>

#include <iostream>
#include <fstream>
#include <iterator>

#include <string.h>

using namespace std;
using namespace Ferris;


void test_ifstream()
{
    cerr << "--- test_ifstream() --- " << endl;
    string tmp;
    fh_ifstream ss;

//    ss.open("input1");
    ss->open("input1");

    while( getline(ss,tmp) )
    {
        cerr << "Read:" << tmp << endl;
    }
    
}

void test_ofstream()
{
    vector< string > v;

    v.push_back( "line 1" );
    v.push_back( "number 2 is this line" );
    v.push_back( "third and final" );
    
    fh_ofstream ss("test_ofstream");

    for( vector< string >::const_iterator iter = v.begin();
         iter != v.end();
         ++iter )
    {
        cerr << "write:" << *iter << endl;
        ss << *iter << endl;
    }
}


void test_fstream()
{
    vector< string > v;

    v.push_back( "line 1 (IO)" );
    v.push_back( "number 2 is this line" );
    v.push_back( "third and final" );
    
    fh_fstream ss("test_fstream", ios_base::out | ios_base::in | ios_base::trunc);

    copy( v.begin(), v.end(), ostream_iterator<string>( cerr, "\n" ));
    copy( v.begin(), v.end(), ostream_iterator<string>( ss, "\n" ));

    cerr << "--- read in data again ---" << endl;
    ss.seekg(0);

    string tmp;
    while( getline(ss,tmp) )
    {
        cerr << "Read:" << tmp << endl;
    }

//    ss.unsetf( ios::skipws );
//     copy( istream_iterator<string>( ss ), istream_iterator<string>(),
//            ostream_iterator<string>( cerr ));
    
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void test_signal_stream_cb( fh_istream& ss, std::streamsize tellp )
{
    cerr << "test_signal_stream_cb() TOP" << endl;
    
    ss->clear();
    cerr << "test_signal_stream_cb() 1" << endl;
    ss->seekg(0);
    cerr << "test_signal_stream_cb() 2" << endl;
    string tmp;
    getline( ss, tmp );

    cerr << "test_signal_stream_cb() tmp:" << tmp << endl;
}



void test_signal_stream()
{
    
    {
        fh_stringstream ss_outer;
        ss_outer << "hello, I am the written line...";
//        getCloseSig( ss_outer ).connect( slot( &test_signal_stream_cb ) );
        {
            fh_stringstream ss = ss_outer;
            ss.getCloseSig().connect( sigc::ptr_fun( &test_signal_stream_cb ) );
            cerr << "About to let 'ss' go out of scope." << endl;
        }
        
        cerr << "About to let 'ss_outer' go out of scope." << endl;
    }
    
    cerr << "All scopes are out." << endl;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// int func2( fh_ostream ss )
// {
//     string s = "hi there";
//     ss << s << " this is more string from func2() " << 69 << " dude" << endl << nlf;
//     return 0;
// }



void printstate( const fh_stringstream& ss )
{
    if( !ss.good() )
    {
        cerr << " rdstate:" << ss.rdstate() << endl;
        cerr << " good state:" << ss.good() << endl;
        cerr << " eof  state:" << ss.eof() << endl;
        cerr << " fail state:" << ss.fail() << endl;
        cerr << " bad  state:" << ss.bad() << endl;
    }
}

void printstate_io( const fh_iostream& ss )
{
    if( !ss.good() )
    {
        cerr << " rdstate:" << ss.rdstate() << endl;
        cerr << " good state:" << ss.good() << endl;
        cerr << " eof  state:" << ss.eof() << endl;
        cerr << " fail state:" << ss.fail() << endl;
        cerr << " bad  state:" << ss.bad() << endl;
    }
}

void printstate_io( const fh_istream& ss )
{
    if( !ss.good() )
    {
        cerr << " rdstate:" << ss.rdstate() << endl;
        cerr << " good state:" << ss.good() << endl;
        cerr << " eof  state:" << ss.eof() << endl;
        cerr << " fail state:" << ss.fail() << endl;
        cerr << " bad  state:" << ss.bad() << endl;
    }
}


fh_iostream test_io2( fh_iostream x )
{
    string tmp = "";
    
    x.clear();
    x.seekp(0);
    x.seekg(2);
    tmp = "";
    getline( x, tmp );
    cerr << "test_io2. Read back string:" << tmp << endl;

    return x;
}


void test_generic()
{
    string tmp;
    fh_stringstream ss;

    ss << "hi";
    ss.seekg(0);
    ss->seekg(0);
    tmp = "";
    ss >> tmp;
    cerr << "1. Read back string:" << tmp << endl;

    printstate( ss );
    ss.clear(); // eof
    
    ss.seekp(0);
    ss.seekg(0);
    ss << "there" << endl;
    tmp = "";
    getline( ss, tmp );
    cerr << "2. Read back string:" << tmp << endl;
    

    fh_stringstream ss2 = ss;
    ss2.seekg(0);
    tmp = "";
    ss2 >> tmp;
    cerr << "3. Read back string:" << tmp << endl;

    ss2.clear(); // eof
    ss2.seekp(0);
    ss2.seekg(0);
    ss2 << "there for ss2" << endl;
    tmp = "";
    getline( ss2, tmp );
    cerr << "4. Read back string:" << tmp << endl;

    cerr << "--------- before ----------" << endl;
    printstate( ss );
    fh_iostream ios1 = ss;
    printstate( ss );
    cerr << "--------- after ----------" << endl;

    ios1.clear();
    ss.clear();
    if( !ios1.good() )
    {
        cerr << " rdstate:" << ios1.rdstate() << endl;
        cerr << " good state:" << ios1.good() << endl;
        cerr << " eof  state:" << ios1.eof() << endl;
        cerr << " fail state:" << ios1.fail() << endl;
        cerr << " bad  state:" << ios1.bad() << endl;
    }
    printstate( ss );

    
    ios1.clear();
    ios1.seekg(0);
    ios1.seekp(0);
    tmp = "";
    ios1 >> tmp;
    cerr << "5. Read back string:" << tmp << endl;

    ios1.clear(); // eof
    ios1.seekp(0);
    ios1.seekg(0);
    ios1 << "there for ios1." << endl;
    tmp = "";
    getline( ios1, tmp );
    cerr << "6. Read back string:" << tmp << endl;



    ss.clear(); // eof
    ss.seekg(0);
    tmp = "";
    getline(ss, tmp);
    cerr << "6.ss Read back string:" << tmp << endl;


    fh_iostream x = test_io2( ss );
    x.clear();
    x.seekp(0);
    x << "do me, do me dry" << flush;

    fh_iostream x2 = test_io2( x );


    ss.clear(); // eof
    ss.seekg(0);
    tmp = "";
    getline(ss, tmp);
    cerr << "7.ss Read back string:" << tmp << endl;
    

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    fh_istream iss = ss;

//    iss.clear();
    if( !iss.good() )
    {
        cerr << " rdstate:"    << iss.rdstate() << endl;
        cerr << " good state:" << iss.good() << endl;
        cerr << " eof  state:" << iss.eof() << endl;
        cerr << " fail state:" << iss.fail() << endl;
        cerr << " bad  state:" << iss.bad() << endl;
    }
    printstate( ss );

    
    iss.clear();
    iss.seekg(0);
    tmp = "";
    getline( iss, tmp );
    cerr << "10.iss Read back string:" << tmp << endl;

    
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /*
     * Make sure that the base stream can go out of scope.
     */
    {
        {
            fh_stringstream ss;
            ss << "test123" << flush;
            ss.clear();
            ss.seekg(0);
            
            fh_istream iss = ss;
        }
        
        tmp = "";
        iss.clear();
        iss.seekg(0);
        getline( iss, tmp );
        cerr << "11.iss Read back string:" << tmp << endl;
        
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void test_datastream_cb( fh_istream& ss, std::streamsize tellp )
{
    cerr << "test_datastream_cb() enter." << endl;

    printstate_io( ss );
    ss.clear();
    ss.seekg(0);

    cerr << "test_datastream_cb() start reading." << endl;

    printstate_io( ss );
    copy( istreambuf_iterator<char>( ss ), istreambuf_iterator<char>(),
          ostreambuf_iterator<char>( cerr ));
    cerr << endl;
    printstate_io( ss );
    
    cerr << "test_datastream_cb() done." << endl;
}


fh_iostream make_datastream()
{
    static char* d = "hello\0\0\0";

    cerr << " =========== make_datastream() starting." << endl;
    fh_iostream ret;

    cerr << " =========== make_datastream() create ds." << endl;
    fh_iostream ds = Factory::MakeMemoryIOStream( d, strlen(d)+1 );
    ds.getCloseSig().connect( sigc::ptr_fun( &test_datastream_cb ) );

    
    ds.clear();
    ds.seekg(0);
    copy( istreambuf_iterator<char>( ds ), istreambuf_iterator<char>(),
          ostreambuf_iterator<char>( cerr ));
    cerr << " =========== make_datastream() dump done." << endl;
    ds.clear();
    ds.seekg(0);

    
    cerr << " =========== make_datastream() create ret=ds." << endl;
    ret = ds;

    cerr << " =========== make_datastream() returning." << endl;
    return ret;
}

void test_datastream()
{
    cerr << " =========== test_datastream() starting." << endl;
    {
        fh_iostream ss = make_datastream();
        ss.clear();
        ss.seekg(0);
        cerr << " =========== test_datastream() dropping ss scope." << endl;
    }
    cerr << " =========== test_datastream() done." << endl;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void test_idatastream()
{
    static char* data = "hi there\0";

    fh_istream ds = Factory::MakeMemoryIStream( data, strlen(data)+1 );

    cerr << "Reading idatastream. Start." << endl;
    copy( istreambuf_iterator<char>( ds ), istreambuf_iterator<char>(),
          ostreambuf_iterator<char>( cerr ));
    cerr << endl << "Reading idatastream. Done." << endl;
    cerr << "Sould have printed the string: " << data << endl;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
    
//     test_generic();
//     test_ifstream();
//     test_ofstream();
//     test_fstream();
//    test_signal_stream();
    test_datastream();
//    test_idatastream();
}









