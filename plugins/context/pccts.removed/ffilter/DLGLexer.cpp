
/* parser.dlg -- DLG Description of scanner
 *
 * Generated from: ffilter.g
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
#include "DLGLexer.h"

ANTLRTokenType DLGLexer::act1()
{ 
		return Eof;
	}


ANTLRTokenType DLGLexer::act2()
{ 
    skip();  
		return (ANTLRTokenType)2;
	}


ANTLRTokenType DLGLexer::act3()
{ 
    skip(); newline();  
		return (ANTLRTokenType)3;
	}


ANTLRTokenType DLGLexer::act4()
{ 
		return KOCTETSTR;
	}


ANTLRTokenType DLGLexer::act5()
{ 
    mode(EAVALUECLASS);   
		return EQREGEX;
	}


ANTLRTokenType DLGLexer::act6()
{ 
    mode(EAVALUECLASS);   
		return EQLT;
	}


ANTLRTokenType DLGLexer::act7()
{ 
    mode(EAVALUECLASS);   
		return EQGT;
	}


ANTLRTokenType DLGLexer::act8()
{ 
    mode(EAVALUECLASS);   
		return EQ;
	}


ANTLRTokenType DLGLexer::act9()
{ 
    mode(EAVALUECLASS);   
		return EQLTSC;
	}


ANTLRTokenType DLGLexer::act10()
{ 
    mode(EAVALUECLASS);   
		return EQGTSC;
	}


ANTLRTokenType DLGLexer::act11()
{ 
    mode(EAVALUECLASS);   
		return EQSC;
	}


ANTLRTokenType DLGLexer::act12()
{ 
		return (ANTLRTokenType)17;
	}


ANTLRTokenType DLGLexer::act13()
{ 
		return (ANTLRTokenType)18;
	}


ANTLRTokenType DLGLexer::act14()
{ 
		return (ANTLRTokenType)19;
	}


ANTLRTokenType DLGLexer::act15()
{ 
		return (ANTLRTokenType)20;
	}


ANTLRTokenType DLGLexer::act16()
{ 
		return (ANTLRTokenType)21;
	}

 unsigned char DLGLexer::shift0[257] = {
  0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  2, 3, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 1, 11, 15, 15, 5, 15, 14, 
  15, 10, 12, 5, 15, 15, 5, 5, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  15, 7, 6, 8, 9, 15, 5, 5, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 15, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 5, 5, 15, 13, 15, 4, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
  15, 15, 15, 15, 15, 15, 15
};


ANTLRTokenType DLGLexer::act17()
{ 
		return Eof;
	}


ANTLRTokenType DLGLexer::act18()
{ 
    
    replchar('\0');
    mode(START);
		return EAVALUETERMINATOR;
	}


ANTLRTokenType DLGLexer::act19()
{ 
    more();  
		return (ANTLRTokenType)15;
	}

 unsigned char DLGLexer::shift1[257] = {
  0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
  2, 2, 2, 2, 2, 2, 2
};


const int DLGLexer::MAX_MODE=2;
const int DLGLexer::DfaStates=28;
const int DLGLexer::START=0;
const int DLGLexer::EAVALUECLASS=1;

DLGLexer::DfaState DLGLexer::st0[16] = {
  1, 2, 3, 4, 5, 5, 6, 7, 8, 28, 
  9, 10, 11, 12, 13, 28
};

DLGLexer::DfaState DLGLexer::st1[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st2[16] = {
  28, 2, 3, 28, 5, 5, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st3[16] = {
  28, 3, 3, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st4[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st5[16] = {
  28, 5, 28, 28, 5, 5, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st6[16] = {
  28, 28, 28, 28, 14, 28, 15, 28, 28, 16, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st7[16] = {
  28, 28, 28, 28, 28, 28, 17, 28, 28, 18, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st8[16] = {
  28, 28, 28, 28, 28, 28, 19, 28, 28, 20, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st9[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st10[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st11[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st12[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st13[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st14[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st15[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st16[16] = {
  28, 28, 28, 28, 28, 28, 21, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st17[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st18[16] = {
  28, 28, 28, 28, 28, 28, 22, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st19[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st20[16] = {
  28, 28, 28, 28, 28, 28, 23, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st21[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st22[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st23[16] = {
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 
  28, 28, 28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st24[4] = {
  25, 26, 27, 28
};

DLGLexer::DfaState DLGLexer::st25[4] = {
  28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st26[4] = {
  28, 28, 28, 28
};

DLGLexer::DfaState DLGLexer::st27[4] = {
  28, 28, 28, 28
};


DLGLexer::DfaState *DLGLexer::dfa[28] = {
	st0,
	st1,
	st2,
	st3,
	st4,
	st5,
	st6,
	st7,
	st8,
	st9,
	st10,
	st11,
	st12,
	st13,
	st14,
	st15,
	st16,
	st17,
	st18,
	st19,
	st20,
	st21,
	st22,
	st23,
	st24,
	st25,
	st26,
	st27
};


DLGLexer::DfaState DLGLexer::accepts[29] = {
  0, 1, 2, 2, 3, 4, 0, 0, 0, 12, 
  13, 14, 15, 16, 5, 8, 0, 6, 0, 7, 
  0, 11, 9, 10, 0, 17, 18, 19, 0
};

PtrDLGLexerMemberFunc DLGLexer::actions[20] = {
	&DLGLexer::erraction,
	&DLGLexer::act1,
	&DLGLexer::act2,
	&DLGLexer::act3,
	&DLGLexer::act4,
	&DLGLexer::act5,
	&DLGLexer::act6,
	&DLGLexer::act7,
	&DLGLexer::act8,
	&DLGLexer::act9,
	&DLGLexer::act10,
	&DLGLexer::act11,
	&DLGLexer::act12,
	&DLGLexer::act13,
	&DLGLexer::act14,
	&DLGLexer::act15,
	&DLGLexer::act16,
	&DLGLexer::act17,
	&DLGLexer::act18,
	&DLGLexer::act19
};

DLGLexer::DfaState DLGLexer::dfa_base[] = {
	0,
	24
};

 unsigned char *DLGLexer::b_class_no[] = {
	shift0,
	shift1
};

DLGChar DLGLexer::alternatives[29] = {
	1,
	0,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
/* must have 0 for zzalternatives[DfaStates] */
	0
};

#include "DLexer.h"
