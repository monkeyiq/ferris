/*
 * FulltextbooleanParser: P a r s e r  H e a d e r 
 *
 * Generated from: fulltextboolean.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-2001
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 */

#ifndef FulltextbooleanParser_h
#define FulltextbooleanParser_h

#ifndef ANTLR_VERSION
#define ANTLR_VERSION 13333
#endif

class ASTBase;
#include "AParser.h"

class FulltextbooleanParser : public ANTLRParser {
public:
	static  const ANTLRChar *tokenName(int tk);
	enum { SET_SIZE = 9 };
protected:
	static const ANTLRChar *_token_tbl[];
private:
protected:
	static SetWordType err1[4];
	static SetWordType setwd1[9];
private:
	void zzdflthandlers( int _signal, int *_retsignal );

public:
	FulltextbooleanParser(ANTLRTokenBuffer *input);
	void interp(ASTBase **_root);
	void query(ASTBase **_root);
	void term(ASTBase **_root);
};

#endif /* FulltextbooleanParser_h */
