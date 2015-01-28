/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 *
 *   antlr -CC -gt ffilter.g
 *
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"
#include "tokens.h"
#include "ASTBase.h"

#include "AParser.h"
#include "FFilterParser.h"
#include "DLexerBase.h"
#include "ATokPtr.h"

/* MR23 In order to remove calls to PURIFY use the antlr -nopurify option */

#ifndef PCCTS_PURIFY
#define PCCTS_PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif



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

  

// justGG

void
FFilterParser::interp(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t12=NULL;
  AST *_ast11=NULL,*_ast12=NULL;
  ANTLRTokenPtr a=NULL;
  AST *a_ast=NULL;
  _ast = NULL;
  filter(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast11 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  a_ast = _ast11;
  zzmatch(Eof); _t12 = (ANTLRTokenPtr)LT(1);  consume();
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x1);
}

void
FFilterParser::filter(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  AST *_ast11=NULL;
  _ast = NULL;
  eaterm(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast11 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  { /*#0->preorder(); printf("\n"); */ }
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x2);
}

void
FFilterParser::eaterm(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t11=NULL;
  AST *_ast11=NULL;
  zzmatch(17); _t11 = (ANTLRTokenPtr)LT(1);  consume();
  {
    AST *_ast21=NULL;
    if ( (LA(1)==KOCTETSTR) ) {
      _ast = NULL;
      eamatch(&_ast);
      if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
      _ast21 = (AST *)_ast;
      ASTBase::link(_root, &_sibling, &_tail);
    }
    else {
      if ( (LA(1)==18) ) {
        {
          ANTLRTokenPtr _t31=NULL,_t33=NULL;
          AST *_ast31=NULL,*_ast32=NULL,*_ast33=NULL;
          zzmatch(18); _t31 = (ANTLRTokenPtr)LT(1);
          _ast31 = new AST(_t31);
          _ast31->subroot(_root, &_sibling, &_tail);
           consume();
          _ast = NULL;
          eaterm(&_ast);
          if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
          _ast32 = (AST *)_ast;
          ASTBase::link(_root, &_sibling, &_tail);
          zzmatch(19); _t33 = (ANTLRTokenPtr)LT(1);  consume();
        }
      }
      else {
        if ( (LA(1)==20) ) {
          {
            ANTLRTokenPtr _t31=NULL,_t33=NULL;
            AST *_ast31=NULL,*_ast33=NULL;
            zzmatch(20); _t31 = (ANTLRTokenPtr)LT(1);
            _ast31 = new AST(_t31);
            _ast31->subroot(_root, &_sibling, &_tail);
             consume();
            {
              AST *_ast41=NULL;
              int zzcnt=1;
              do {
                _ast = NULL;
                eaterm(&_ast);
                if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
                _ast41 = (AST *)_ast;
                ASTBase::link(_root, &_sibling, &_tail);
              } while ( (LA(1)==17) );
            }
            zzmatch(19); _t33 = (ANTLRTokenPtr)LT(1);  consume();
          }
        }
        else {
          if ( (LA(1)==21) ) {
            {
              ANTLRTokenPtr _t31=NULL,_t33=NULL;
              AST *_ast31=NULL,*_ast33=NULL;
              zzmatch(21); _t31 = (ANTLRTokenPtr)LT(1);
              _ast31 = new AST(_t31);
              _ast31->subroot(_root, &_sibling, &_tail);
               consume();
              {
                AST *_ast41=NULL;
                int zzcnt=1;
                do {
                  _ast = NULL;
                  eaterm(&_ast);
                  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
                  _ast41 = (AST *)_ast;
                  ASTBase::link(_root, &_sibling, &_tail);
                } while ( (LA(1)==17) );
              }
              zzmatch(19); _t33 = (ANTLRTokenPtr)LT(1);  consume();
            }
          }
          else {FAIL(1,err1,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x4);
}

void
FFilterParser::eamatch(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t12=NULL;
  AST *_ast11=NULL,*_ast12=NULL,*_ast13=NULL;
  _ast = NULL;
  eakey(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast11 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  zzsetmatch(COMPARISON_set, COMPARISON_errset); _t12 = (ANTLRTokenPtr)LT(1);
  _ast12 = new AST(_t12);
  _ast12->subroot(_root, &_sibling, &_tail);
   consume();
  _ast = NULL;
  eavalue(&_ast);
  if ( _tail==NULL ) _sibling = _ast; else _tail->setRight(_ast);
  _ast13 = (AST *)_ast;
  ASTBase::link(_root, &_sibling, &_tail);
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x8);
}

void
FFilterParser::eakey(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t11=NULL;
  AST *_ast11=NULL;
  zzmatch(KOCTETSTR); _t11 = (ANTLRTokenPtr)LT(1);
  _ast11 = new AST(_t11);
  _ast11->subchild(_root, &_sibling, &_tail);
   consume();
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x10);
}

void
FFilterParser::eavalue(ASTBase **_root)
{
  zzRULE;
  ASTBase *_ast = NULL, *_sibling = NULL, *_tail = NULL;
  ANTLRTokenPtr _t11=NULL;
  AST *_ast11=NULL;
  zzsetmatch(WIDEEAVALUE_set, WIDEEAVALUE_errset); _t11 = (ANTLRTokenPtr)LT(1);
  _ast11 = new AST(_t11);
  _ast11->subchild(_root, &_sibling, &_tail);
   consume();
  return;
fail:
  syn(zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk);
  resynch(setwd1, 0x20);
}
