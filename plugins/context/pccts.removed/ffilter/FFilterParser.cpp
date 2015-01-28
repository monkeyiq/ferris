/*
 * FFilterParser: P a r s e r  S u p p o r t
 *
 * Generated from: ffilter.g
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
#include "FFilterParser.h"

const ANTLRChar *FFilterParser::tokenName(int tok)   { return _token_tbl[tok]; }

const ANTLRChar *FFilterParser::_token_tbl[]={
	/* 00 */	"Invalid",
	/* 01 */	"Eof",
	/* 02 */	"[\\ \\t]+",
	/* 03 */	"\\n",
	/* 04 */	"KOCTETSTR",
	/* 05 */	"EQREGEX",
	/* 06 */	"EQLT",
	/* 07 */	"EQGT",
	/* 08 */	"EQ",
	/* 09 */	"EQLTSC",
	/* 10 */	"EQGTSC",
	/* 11 */	"EQSC",
	/* 12 */	"COMPARISON",
	/* 13 */	"SQSC",
	/* 14 */	"EAVALUETERMINATOR",
	/* 15 */	"~[]",
	/* 16 */	"WIDEEAVALUE",
	/* 17 */	"\\(",
	/* 18 */	"\\!",
	/* 19 */	"\\)",
	/* 20 */	"\\|",
	/* 21 */	"\\&"
};

FFilterParser::FFilterParser(ANTLRTokenBuffer *input) : ANTLRParser(input,1,0,0,4)
{
	token_tbl = _token_tbl;
	traceOptionValueDefault=0;		// MR10 turn trace OFF
}

SetWordType FFilterParser::err1[4] = {0x10,0x0,0x34,0x0};
SetWordType FFilterParser::COMPARISON_set[4] = {0xe0,0x27,0x0,0x0};
SetWordType FFilterParser::COMPARISON_errset[4] = {0xe0,0x27,0x0,0x0};
SetWordType FFilterParser::WIDEEAVALUE_set[4] = {0x0,0x40,0x0,0x0};
SetWordType FFilterParser::WIDEEAVALUE_errset[4] = {0x0,0x40,0x0,0x0};
SetWordType FFilterParser::setwd1[22] = {0x0,0x2f,0x0,0x0,0x0,0x10,0x10,
	0x10,0x10,0x10,0x10,0x0,0x0,0x10,0x0,
	0x0,0x0,0x2c,0x0,0x2c,0x0,0x0};
