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

    $Id: mbox.g,v 1.1 2005/07/12 04:12:11 ben Exp $

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
#include <Common.hh>
#include <ferrisPcctsContext.hh>
#include <DLGLexer.h>
#include <Support.hh>
#include <iomanip.h>

void dump( AST* node, const string& s )
{
	if( !node )
		return;

	cout    << "dump name:" << setw(30) << node->getName()  
		<< " s:" << s <<  endl;

	dump( (AST*)node->down(), s + node->getName());

	for( AST* n = node;  n = (AST*)n->right(); )
	{
//		cout << " s:" << s << "   " << n->getName() << endl;
		dump( (AST*)n, s + n->getName());
	}


}

main()
{
	fh_ifstream ss = new f_ifstream("input");

	MyParserBlackBox<MyDLGLexer, MBoxParser, ANTLRToken> p(ss);
//	MyParserBlackBox<DLGLexer, MBoxParser, ANTLRToken> p(ss);

	ASTBase* root = NULL;
	p.parser()->interp( &root );


	cout << root << endl;
	if( root )
	{
		AST* node = (AST*)root;
		string s = node->getName();

		dump( node, "" );
	}

}
>>

#token FROM             "From"		<< mode(LC_Email); >>
#token Eof		"@"
#token DIGITS		"[0-9]+" 	
#token WS		"[\ \t\n]+"  	<< isWS=1; tabAdjust(); >>


#lexclass LC_Email
#token EMAILADDR 	"[a-zA-Z]+{\@[a-zA-Z0-9\.]+}"   << mode( START ); >>
#token WS		"[\ \t\n]+" 			<< isWS=1; tabAdjust(); >>



#lexclass START
class MBoxParser 
{


interp 
	:
	f:FROM 		<<mytoken($f)->dumpNode();>> 
//	w:WS 		<<mytoken($w)->dumpNode();>>
	e:EMAILADDR  	<<mytoken($e)->dumpNode();>>
	z:WS
	<<#0->preorder(); printf("\n");>>
	;

//emailaddr
//	:       OCTETSTR { EMAILHOSTSEP^ OCTETSTR ( EMAILDOMAINSEP OCTETSTR )* }
//      	;	

}


