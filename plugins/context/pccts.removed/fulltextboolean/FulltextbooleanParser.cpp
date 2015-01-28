/*
 * FulltextbooleanParser: P a r s e r  S u p p o r t
 *
 * Generated from: fulltextboolean.g
 *
 * Terence Parr, Russell Quong, Will Cohen, and Hank Dietz: 1989-2001
 * Parr Research Corporation
 * with Purdue University Electrical Engineering
 * with AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"
#define ANTLR_SUPPORT_CODE
#include "tokens.h"
#include "FulltextbooleanParser.h"

const ANTLRChar *FulltextbooleanParser::tokenName(int tok)   { return _token_tbl[tok]; }

const ANTLRChar *FulltextbooleanParser::_token_tbl[]={
	/* 00 */	"Invalid",
	/* 01 */	"Eof",
	/* 02 */	"AND",
	/* 03 */	"OR",
	/* 04 */	"NOT",
	/* 05 */	"MINUS",
	/* 06 */	"WS",
	/* 07 */	"\\n",
	/* 08 */	"OCTETSTR"
};

FulltextbooleanParser::FulltextbooleanParser(ANTLRTokenBuffer *input) : ANTLRParser(input,1,0,0,4)
{
	token_tbl = _token_tbl;
	traceOptionValueDefault=0;		// MR10 turn trace OFF
}

SetWordType FulltextbooleanParser::err1[4] = {0x3c,0x0,0x0,0x0};
SetWordType FulltextbooleanParser::setwd1[9] = {0x0,0xd,0xa,0xa,0xa,0xa,0x0,
	0x0,0x0};
