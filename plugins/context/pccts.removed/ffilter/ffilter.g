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

    $Id: ffilter.g,v 1.3 2008/12/12 21:30:11 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#lexmember <<
  protected:
    int          isWS;
    int          begline;
    virtual void tabAdjust() {}
>>

<<

#include "Common.hh"
#include "ferrisPcctsContext.hh"
#include "DLGLexer.h"

//#include <iostream>

using namespace std;
using namespace Ferris;


void dump( AST* node, const string& s )
{
	if( !node )
		return;

	cout << "1... s:" << s << "   " << node->getName() << endl;

	dump( (AST*)node->down(), s + node->getName());

	for( AST* n = node;  n = (AST*)n->right(); )
	{
		cout << "2... s:" << s << "   " << n->getName() << endl;
		dump( (AST*)n->down(), s + n->getName());
	}


}

int main()
{
	fh_ifstream ss("input");

	MyParserBlackBox<DLGLexer, FFilterParser, ANTLRToken> p(ss);
	ASTBase* root = NULL;
	p.parser()->interp( &root );


	cout << root << endl;

	AST* node = (AST*)root;
	string s = node->getName();

	dump( node, "" );

	return 0;
}

>>

#lexclass START

#token Eof       "@"
#token "[\ \t]+"	<<skip();>>
#token "\n"		<<skip(); newline();>>

#token KOCTETSTR          "[\ a-z\.A-Z0-9\-\_\*\$\^\]\[\\\/\~\:]+"
//#token VOCTETSTR          "[a-zA-Z0-9\-\_\*\$\^\]\[\\\/\~\:]+"
//#token VOCTETSTR          "[\ a-zA-Z0-9\-\_\*\$\^\]\[\\\/\~\.\?\+]+"


#token EQREGEX "\=\~" << mode(EAVALUECLASS); >>
#token EQLT    "<="   << mode(EAVALUECLASS); >>
#token EQGT    ">="   << mode(EAVALUECLASS); >>
#token EQ      "=="   << mode(EAVALUECLASS); >>
#token EQLTSC  "<\?=" << mode(EAVALUECLASS); >>
#token EQGTSC  ">\?=" << mode(EAVALUECLASS); >>
#token EQSC    "=\?=" << mode(EAVALUECLASS); >>
#tokclass COMPARISON {EQREGEX EQLT EQGT EQ EQLTSC EQGTSC SQSC} 

/******************************/
/******************************/
/******************************/

#lexclass EAVALUECLASS
#token EAVALUETERMINATOR "\)"    <<
                             replchar('\0');
                             mode(START);
                         >>
#token           "~[]" <<more();>>
#tokclass WIDEEAVALUE {EAVALUETERMINATOR}

/******************************/
/******************************/
/******************************/

#lexclass START

<<
// justGG
>>
                             
class FFilterParser 
{

<<

public:

void foobar2()
{
int x = 2;
}



>>


interp
	:	a:filter Eof!
	;

filter
	:	eaterm   <<{ /*#0->preorder(); printf("\n"); */ }>>
	;


eaterm
	:
		"\("! 
		 (
		     eamatch
		   | ( "\!"^ eaterm  "\)"! )
		   | ( "\|"^ (eaterm)+ "\)"! )
		   | ( "\&"^ (eaterm)+ "\)"! )
		 )
		
		
	;

// eavalue eats the terminating ')' char
eamatch : 	eakey COMPARISON^ eavalue;
eakey  	:	KOCTETSTR ;
eavalue :	WIDEEAVALUE ;

}

