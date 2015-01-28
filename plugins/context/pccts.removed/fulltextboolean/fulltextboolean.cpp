/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 *
 *   antlr -CC -gt fulltextboolean.g
 *
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"
#include "tokens.h"
#include "ASTBase.h"

#include "AParser.h"
#include "FulltextbooleanParser.h"
#include "DLexerBase.h"
#include "ATokPtr.h"

/* MR23 In order to remove calls to PURIFY use the antlr -nopurify option */

#ifndef PCCTS_PURIFY
#define PCCTS_PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif



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

  

void
FulltextbooleanParser::interp(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t12=NULL;
  AST *_ast11=NULL,*_ast12=NULL;
  ANTLRTokenPtr a=NULL;
  AST *a_ast=NULL;
  _ast = NULL;
  query(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast11 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  a_ast = _ast11;
  zzmatch(Eof); _t12 = (ANTLRTokenPtr)LT(1); 
  { /*#0->preorder(); printf("\n"); */ }
 consume();
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x1);
}

void
FulltextbooleanParser::query(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  AST *_ast11=NULL;
  _ast = NULL;
  term(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast11 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  {
    AST *_ast22=NULL;
    while ( (setwd1[LA(1)]&0x2) ) {
      {
        ANTLRTokenPtr _t31=NULL;
        AST *_ast31=NULL;
        if ( (LA(1)==AND) ) {
          zzmatch(AND); _t31 = (ANTLRTokenPtr)LT(1);
          _ast31 = new AST(_t31);
          _ast31->subroot(_root, &_sibling, &_tail);
           consume();
        }
        else {
          if ( (LA(1)==OR) ) {
            zzmatch(OR); _t31 = (ANTLRTokenPtr)LT(1);
            _ast31 = new AST(_t31);
            _ast31->subroot(_root, &_sibling, &_tail);
             consume();
          }
          else {
            if ( (LA(1)==NOT) ) {
              zzmatch(NOT); _t31 = (ANTLRTokenPtr)LT(1);
              _ast31 = new AST(_t31);
              _ast31->subroot(_root, &_sibling, &_tail);
               consume();
            }
            else {
              if ( (LA(1)==MINUS) ) {
                zzmatch(MINUS); _t31 = (ANTLRTokenPtr)LT(1);
                _ast31 = new AST(_t31);
                _ast31->subroot(_root, &_sibling, &_tail);
                 consume();
              }
              else {FAIL(1,err1,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
            }
          }
        }
      }
      _ast = NULL;
      term(&_ast);
      if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
      _ast22 = (AST *)_ast;
      ASTBase::link(_root, &_sibling, &_tail);
    }
  }
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x4);
}

void
FulltextbooleanParser::term(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t11=NULL;
  AST *_ast11=NULL;
  zzmatch(OCTETSTR); _t11 = (ANTLRTokenPtr)LT(1);
  _ast11 = new AST(_t11);
  _ast11->subchild(_root, &_sibling, &_tail);
   consume();
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x8);
}
