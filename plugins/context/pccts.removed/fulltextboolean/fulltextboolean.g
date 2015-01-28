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

    $Id: fulltextboolean.g,v 1.1 2005/07/12 04:13:59 ben Exp $

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

using namespace std;
using namespace Ferris;


void dump( AST* node, const string& s )
{
	if( !node )
		return;

	cout << " s:" << s << "   " << node->getName() << endl;

	dump( (AST*)node->down(), s + node->getName());

	for( AST* n = node;  n = (AST*)n->right(); )
	{
		cout << " s:" << s << "   " << n->getName() << endl;
		dump( (AST*)n->down(), s + n->getName());
	}


}

int main()
{
	fh_ifstream ss("input");

	MyParserBlackBox<PCCTSLEXERCLASSNAME, FulltextbooleanParser, ANTLRToken> p(ss);
	ASTBase* root = NULL;
	p.parser()->interp( &root );


	cout << root << endl;

	AST* node = (AST*)root;
	string s = node->getName();

	dump( node, "" );

	return 0;
}

>>

#token AND "\&"
#token OR  "\|"
#token NOT "\!"
#token MINUS "\-"

#token Eof       "@"
#token WS        "[\ \t]+"	<<skip();>>
#token           "\n"		<<skip(); newline();>>
#token OCTETSTR  "[a-zA-Z0-9]+"

class FulltextbooleanParser 
{

interp
	:	a:query Eof! <<{ /*#0->preorder(); printf("\n"); */ }>>
	;

query
	:       
                term ( ( AND^ | OR^ | NOT^ | MINUS^ ) term)*
	;

term    :       OCTETSTR;

}





