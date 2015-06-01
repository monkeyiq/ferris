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

    $Id: ftestshellquote.cpp,v 1.1 2006/12/07 07:03:31 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include <Ferris.hh>
#include <Ferrisls.hh>

using namespace std;
using namespace Ferris;

int main( int argc, char** argv )
{
    try
    {
        string s;
        
        s = "AhelloE";
//         s = "Ahello ThereE";
//         s = "Ahello\"s ThereE";
//        s = "Ahello's ThereE";
        s = "dirname\\/filepart";
        
        StringQuote sq( StringQuote::SHELL_QUOTING );

        cerr << "original -->" << s     << "<--" << endl;
        cerr << "quoted   -->" << sq(s) << "<--" << endl;
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return 0;
}
