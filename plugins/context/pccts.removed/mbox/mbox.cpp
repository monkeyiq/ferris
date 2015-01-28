/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-1999
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR22
 *
 *   antlr -CC -gt -mrhoist off mbox.g
 *
 */

#define ANTLR_VERSION	13322
#include "pcctscfg.h"
#include "pccts_stdio.h"
#include "tokens.h"
#include "ASTBase.h"

#include "AParser.h"
#include "MBoxParser.h"
#include "DLexerBase.h"
#include "ATokPtr.h"
#ifndef PURIFY
#define PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif

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

void
MBoxParser::interp(ASTBase **_root)
{
	zzRULE;
	ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
	ANTLRTokenPtr _t11=NULL,_t12=NULL,_t13=NULL;
	AST *_ast11=NULL,*_ast12=NULL,*_ast13=NULL;
	ANTLRTokenPtr f=NULL, e=NULL, z=NULL;
	AST *f_ast=NULL, *e_ast=NULL, *z_ast=NULL;
	zzmatch(FROM); _t11 = (ANTLRTokenPtr)LT(1);
	
	_ast11 = new AST(_t11);
	_ast11->subchild(_root, &_sibling, &_tail);
	
	f = _t11;
	f_ast = _ast11;

	mytoken( f)->dumpNode();
 consume();
	zzmatch(EMAILADDR); _t12 = (ANTLRTokenPtr)LT(1);
	
	_ast12 = new AST(_t12);
	_ast12->subchild(_root, &_sibling, &_tail);
	
	e = _t12;
	e_ast = _ast12;

	mytoken( e)->dumpNode();
 consume();
	zzmatch(WS); _t13 = (ANTLRTokenPtr)LT(1);
	
	_ast13 = new AST(_t13);
	_ast13->subchild(_root, &_sibling, &_tail);
	
	z = _t13;
	z_ast = _ast13;

	(*_root)->preorder(); printf("\n");
 consume();
	return;
fail:
	syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
	resynch(setwd1, 0x1);
}
