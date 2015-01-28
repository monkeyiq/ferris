
/* parser.dlg -- DLG Description of scanner
 *
 * Generated from: fulltextboolean.g
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 */

#define ANTLR_VERSION	13333
#include "tokens.h"
#include "AToken.h"
#include "ASTBase.h"
/*
 * D L G tables
 *
 * Generated from: parser.dlg
 *
 * 1989-2001 by  Will Cohen, Terence Parr, and Hank Dietz
 * Purdue University Electrical Engineering
 * DLG Version 1.33MR33
 */

#include "pcctscfg.h"
#include "pccts_stdio.h"

#include "AParser.h"
#include "DLexerBase.h"
#include "DLGLexerFulltextBoolean.h"

ANTLRTokenType DLGLexerFulltextBoolean::act1()
{ 
		return Eof;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act2()
{ 
		return AND;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act3()
{ 
		return OR;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act4()
{ 
		return NOT;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act5()
{ 
		return MINUS;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act6()
{ 
    skip();  
		return WS;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act7()
{ 
    skip(); newline();  
		return (ANTLRTokenType)7;
	}


ANTLRTokenType DLGLexerFulltextBoolean::act8()
{ 
		return OCTETSTR;
	}

 unsigned char DLGLexerFulltextBoolean::shift0[257] = {
  0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 5, 3, 8, 8, 8, 8, 1, 
  8, 8, 8, 8, 8, 8, 4, 8, 8, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 
  8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 8, 8, 8, 8, 8, 8, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
  7, 7, 7, 7, 8, 2, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
  8, 8, 8, 8, 8, 8, 8
};


const int DLGLexerFulltextBoolean::MAX_MODE=1;
const int DLGLexerFulltextBoolean::DfaStates=9;
const int DLGLexerFulltextBoolean::START=0;

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st0[9] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st1[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st2[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st3[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st4[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st5[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st6[9] = {
  9, 9, 9, 9, 9, 6, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st7[9] = {
  9, 9, 9, 9, 9, 9, 9, 9, 9
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::st8[9] = {
  9, 9, 9, 9, 9, 9, 9, 8, 9
};


DLGLexerFulltextBoolean::DfaState *DLGLexerFulltextBoolean::dfa[9] = {
	st0,
	st1,
	st2,
	st3,
	st4,
	st5,
	st6,
	st7,
	st8
};


DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::accepts[10] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 0
};

PtrDLGLexerFulltextBooleanMemberFunc DLGLexerFulltextBoolean::actions[9] = {
	&DLGLexerFulltextBoolean::erraction,
	&DLGLexerFulltextBoolean::act1,
	&DLGLexerFulltextBoolean::act2,
	&DLGLexerFulltextBoolean::act3,
	&DLGLexerFulltextBoolean::act4,
	&DLGLexerFulltextBoolean::act5,
	&DLGLexerFulltextBoolean::act6,
	&DLGLexerFulltextBoolean::act7,
	&DLGLexerFulltextBoolean::act8
};

DLGLexerFulltextBoolean::DfaState DLGLexerFulltextBoolean::dfa_base[] = {
	0
};

 unsigned char *DLGLexerFulltextBoolean::b_class_no[] = {
	shift0
};

DLGChar DLGLexerFulltextBoolean::alternatives[10] = {
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	1,
/* must have 0 for zzalternatives[DfaStates] */
	0
};

#define DLGLexer DLGLexerFulltextBoolean
#include "DLexer.h"
