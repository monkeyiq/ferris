/******************************************************************************
*******************************************************************************
*******************************************************************************

    A quick test for the prefix and postfix trimming code.

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

    $Id: stringtrimming.cpp,v 1.1 2006/12/07 06:57:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Trimming.hh>

using namespace std;


int main( int argc, const char** argv )
{
    string ltspaces = " \t hello   th\tere  \t\t  ";
    PrefixTrimmer  pretrimmer;
    PostfixTrimmer posttrimmer;
    PrePostTrimmer pptrimmer;
    
    pretrimmer.push_back( " " );
    pretrimmer.push_back( "\t" );

    posttrimmer.push_back( " " );
    posttrimmer.push_back( "\t" );

    pptrimmer.push_back( " " );
    pptrimmer.push_back( "\t" );

    cout << "ltspaces   :" << ltspaces << ":" << endl;
    cout << "PreTrimed  :" << pretrimmer(ltspaces)  << ":" << endl;
    cout << "PostTrimed :" << posttrimmer(ltspaces) << ":" << endl;
    cout << "PPTrimed   :" << pptrimmer(ltspaces) << ":" << endl;
    
    
    return 0;
}
