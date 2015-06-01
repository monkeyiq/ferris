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

    $Id: streamtimes.cpp,v 1.1 2006/12/07 06:57:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"

//#include <Ferris.hh>
#include <SignalStreams.hh>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <time.h>

using namespace std;
using namespace Ferris;

int loopmax = 100000;

template < class T >
void runtest()
{
    for( int i=0; i<loopmax; ++i )
    {
        T ss;
        ss << "something";
//        ss << "something" << "and more" << "again" << 6;
//         ss << "something" << "and more" << "again" << 6
//            << "something" << "and more" << "again" << 6;
//        cerr << tostr(ss) << endl;
    }
}


int main( int argc, char** argv )
{
    int ow = 10;
    
    clock_t rawbeg = clock();
    runtest<stringstream>();
    clock_t rawend = clock();

    clock_t ferbeg = clock();
    runtest<fh_stringstream>();
    clock_t ferend = clock();

    clock_t rawt = rawend - rawbeg;
    clock_t fert = ferend - ferbeg;
    cerr << " rawbeg:" << setw(ow) << rawbeg
         << " rawend:" << setw(ow) << rawend
         << " rawt:"   << setw(ow) << rawt << endl;
    cerr << " ferbeg:" << setw(ow) << ferbeg
         << " ferend:" << setw(ow) << ferend
         << " fert:"   << setw(ow) << fert << endl;
    
    cerr << " ratio:" << setw(ow) << (1.0 * rawt / fert ) << endl;
    return 0;
}









