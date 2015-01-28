/*
 * FFilterParser: P a r s e r  H e a d e r 
 *
 * Generated from: ffilter.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-2001
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 */

#ifndef FFilterParser_h
#define FFilterParser_h

#ifndef ANTLR_VERSION
#define ANTLR_VERSION 13333
#endif

class ASTBase;
#include "AParser.h"

class FFilterParser : public ANTLRParser {
public:
	static  const ANTLRChar *tokenName(int tk);
	enum { SET_SIZE = 22 };
protected:
	static const ANTLRChar *_token_tbl[];
private:


public:

/* #include <string> */
/* #include <sstream> */
/* #include <iostream> */

    
void foobar2()
{
  int x = 2;
}

/* std::string  */
/* edecodeToString(SetWordType *a) */
/* { */
  /*     using namespace std; */
  /*     stringstream retss; */
  
/* 	register SetWordType *p = a; */
  /* 	register SetWordType *endp = &(p[bsetsize]); */
  /* 	register unsigned e = 0; */
  
/* 	if ( set_deg(a)>1 ) retss << " {"; */
  /* 	do { */
    /* 		register SetWordType t = *p; */
    /* 		register SetWordType *b = &(bitmask[0]); */
    /* 		do { */
      /* 			if ( t & *b ) retss << " " << token_tbl[e]; */
      /* 			e++; */
      /* 		} while (++b < &(bitmask[sizeof(SetWordType)*8])); */
    /* 	} while (++p < endp); */
  /* 	if ( set_deg(a)>1 ) retss << " }"; */
  /*     return retss.str(); */
  /* } */

/* std::string getSyntaxErrorString( ANTLRChar *egroup, SetWordType *eset, ANTLRTokenType etok, int k) */
/* { */
  /*     using namespace std; */
  /*     stringstream retss; */
  /* 	int line; */
  
/* 	line = LT(1)->getLine(); */
  
/*     syntaxErrCount++;                                   /\* MR11 *\/ */
  
/*     /\* MR23  If the token is not an EOF token, then use the ->getText() value. */
  
/*              If the token is the EOF token the text returned by ->getText()  */
  /*              may be garbage.  If the text from the token table is "@" use */
  /*              "<eof>" instead, because end-users don't know what "@" means. */
  /*              If the text is not "@" then use that text, which must have been */
  /*              supplied by the grammar writer. */
  /*      *\/ */
  /* 	const char * errorAt = LT(1)->getText(); */
  /* 	if (LA(1) == eofToken) { */
  /*   	  errorAt = parserTokenName(LA(1)); */
  /*   	  if (errorAt[0] == '@') errorAt = "<eof>"; */
  /* 	} */
  /*     retss << "line:" << line << " syntax error at \"" << errorAt << "\""; */
  /* 	if ( !etok && !eset ) */
  /*     { */
  /*         retss << endl; */
  /*         return retss.str(); */
  /*     } */
  /* 	if ( k==1 ) */
  /*         retss << " missing"; */
  /* 	else */
  /* 	{ */
  /* 		retss << "; \"" << LT(k)->getText() << "\" not"; */
  /* 		if ( set_deg(eset)>1 ) retss << " in"; */
  /* 	} */
  /* 	if ( set_deg(eset)>0 ) retss << edecodeToString(eset); */
  /* 	else retss << " " << token_tbl[etok]; */
  /* 	if ( strlen(egroup) > 0 ) retss << " in " << egroup; */
  /* 	retss << endl; */
  /*     return retss.str(); */
  /* } */
  
/* void syn(_ANTLRTokenPtr, ANTLRChar *egroup, SetWordType *eset, */
  /* 	ANTLRTokenType 	etok, int k) */
  /* { */
  /*     using namespace std; */
  /*     string errorString = getSyntaxErrorString( egroup, eset, etok, k ); */
  /*     cerr << errorString << endl; */
  /*     syntaxErrors.push_back( errorString ); */
  /* } */
  

protected:
	static SetWordType err1[4];
	static SetWordType COMPARISON_set[4];
	static SetWordType COMPARISON_errset[4];
	static SetWordType WIDEEAVALUE_set[4];
	static SetWordType WIDEEAVALUE_errset[4];
	static SetWordType setwd1[22];
private:
	void zzdflthandlers( int _signal, int *_retsignal );

public:
	FFilterParser(ANTLRTokenBuffer *input);
	void interp(ASTBase **_root);
	void filter(ASTBase **_root);
	void eaterm(ASTBase **_root);
	void eamatch(ASTBase **_root);
	void eakey(ASTBase **_root);
	void eavalue(ASTBase **_root);
};

#endif /* FFilterParser_h */
