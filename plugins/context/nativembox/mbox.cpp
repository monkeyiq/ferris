/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001 Ben Martin

    libferris is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libferris is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libferris.  If not, see <http://www.gnu.org/licenses/>.

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: mbox.cpp,v 1.3 2010/09/24 21:31:40 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <iostream>
using namespace std;
#include <stdlib.h>

#ifdef GCC_HASCLASSVISIBILITY
int main( int argc, char** argv )
{
    cerr << "mbox client disabled for libferris builds that have hidden symbols" << endl;
    exit(1);
}
#else

#include <libnativembox.cpp>
using namespace Ferris;

int main( int argc, char** argv )
{
    try
    {
        string mbox = "input";
        if( argc > 1 )
        {
            mbox = argv[1];
        }

        const string& rdn = mbox;
        fh_ifstream ss( rdn );

        fh_context omc = Resolve( mbox );
    
        fh_mbcontext ctx = new MBoxContext( omc, rdn, ss );

        cout << "About to read() " << endl;
        ctx->read();

        cout << "About to get the subcontext names!" << endl;
        Context::SubContextNames_t names = ctx->getSubContextNames();
        cout << "got the subcontext names!" << endl;

        for(Context::SubContextNames_t::const_iterator iter = names.begin();
            iter != names.end();
            iter++ )
        {
            fh_context sub = ctx->getSubContext( *iter );
            fh_istream ss  = sub->getIStream();

            cout << "=======" << sub->getDirPath() << "========" << endl;
            while( ss->good() )
            {
                string s;
                getline( ss, s );
                cout << s << endl;
            }
        }
        
    }
    catch( exception& e )
    {
        cerr << "mbox test client cought e:" << e.what() << endl;
    }
    
    return 0;
}
#endif






