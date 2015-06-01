/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris 
    Copyright (C) 2002 Ben Martin

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

    $Id: fshmput.cpp,v 1.1 2006/12/07 07:02:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris/Ferris.hh>

using namespace std;
using namespace Ferris;

int perform( int argc, char** argv )
{
    cerr << "resolve url:" << argv[1] << endl;
    fh_context c = Resolve( argv[1] );
    {
        fh_iostream ss = c->getIOStream();

        cerr << "putting data:" << argv[2] << endl;
        ss << argv[2] << flush;
    }
    cerr << "Stream closed." << endl;
    return 0;
}


void test()
{
    const unsigned int data_sz = 1024;
    static char data[ data_sz ];

    char* x = new char[0];
    delete x;
    

    data[0] = 'a';
    data[1] = 'b';
    data[2] = 'c';
    data[3] = 'd';
    data[4] = 'e';
    for( int i=0; i < 5; ++i )
    {
        char* p = (char*)data;
        cerr << " i:" << i
             << " data[" << i << "]:" << hex << (int)p[i]
             << " data[" << i << "]:" << p[i]
             << endl;
    }

    fh_iostream ds = Factory::MakeMemoryIOStream( data, data_sz );
    ds << "hello" << flush;

    for( int i=0; i < 5; ++i )
    {
        char* p = (char*)data;
        cerr << " i:" << i
             << " data[" << i << "]:" << hex << (int)p[i]
             << " data[" << i << "]:" << p[i]
             << endl;
    }

    fh_istream ross = Factory::MakeMemoryIStream( data, data_sz );
    cerr << "state of ross:" << ross.rdstate() << endl;
    cerr << "good? of ross:" << ross.good() << endl;
    
    for( int i=0; i < 5; ++i )
    {
        char ch = 0;
        ross >> ch;
        
        cerr << " i:" << i
             << " data[" << i << "]:" << hex << (int)ch
             << " data[" << i << "]:" << ch
             << endl;
    }
    
}


int main( int argc, char** argv )
{
    test();
    
    if( argc < 3 )
    {
        cerr << "Usage: " << argv[0] << " shm-url data " << endl;
        exit(1);
    }

    return perform( argc, argv );
}
