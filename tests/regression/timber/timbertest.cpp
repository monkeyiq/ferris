/******************************************************************************
*******************************************************************************
*******************************************************************************

    A test for timber logging

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

    $Id: timbertest.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <timber.hh>
#include <popt.h>

using namespace std;
using namespace Ferris;


const char* PROGRAM_NAME = "timbertest";


void stringstream_test()
{
    fh_stringstream ss; 

    ss << 'h' << 'i';
//    ss.flush();
    ss.seekg(0);
    string s;
    getline( ss, s );
    cerr << "s:" << s << endl;
    cerr << "tostr() :" << tostr(ss) << endl;
    

    ss.clear();
    ss.seekp(0);

    ss << 'h' << '2';
//    ss.flush();
    ss.seekg(0);
    getline( ss, s );
    cerr << "s:" << s << endl;
    cerr << "tostr() :" << tostr(ss) << endl;
}



int main( int argc, const char** argv )
{
    cerr << "Starting..." << endl;
//     Timber t( PROGRAM_NAME,
//               Timber::_SBufT::OPT_PID,
//               Timber::_SBufT::FAC_L6,
//               Timber::_SBufT::PRI_ALERT);

//     cerr << "Performing..." << endl;
//     t << "Timber test (TM) starting" << endl;

//     for( int i=0; i<argc; ++i )
//         t << "argv[" << i << "] is:" << argv[i] << endl;

    cerr << "done with explicit timber..." << endl;

    LG_STRF_W << "strf warning. " << endl;

    cerr << "Exiting..." << endl;
    return 0;
}
