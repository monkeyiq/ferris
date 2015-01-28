#ifndef DLGLexerFulltextBoolean_h
#define DLGLexerFulltextBoolean_h
/*
 * D L G L e x e r  C l a s s  D e f i n i t i o n
 *
 * Generated from: parser.dlg
 *
 * 1989-2001 by  Will Cohen, Terence Parr, and Hank Dietz
 * Purdue University Electrical Engineering
 * DLG Version 1.33MR33
 */


#include "DLexerBase.h"

class DLGLexerFulltextBoolean : public DLGLexerBase {
public:


protected:
int          isWS;
int          begline;
virtual void tabAdjust() {}
public:
	static const int MAX_MODE;
	static const int DfaStates;
	static const int START;
	typedef unsigned char DfaState;

	DLGLexerFulltextBoolean(DLGInputStream *in,
		unsigned bufsize=2000)
		: DLGLexerBase(in, bufsize, 1)
	{
	;
	}
	void	  mode(int);
	ANTLRTokenType nextTokenType(void);
	void     advance(void);
protected:
	ANTLRTokenType act1();
	ANTLRTokenType act2();
	ANTLRTokenType act3();
	ANTLRTokenType act4();
	ANTLRTokenType act5();
	ANTLRTokenType act6();
	ANTLRTokenType act7();
	ANTLRTokenType act8();
	static DfaState st0[9];
	static DfaState st1[9];
	static DfaState st2[9];
	static DfaState st3[9];
	static DfaState st4[9];
	static DfaState st5[9];
	static DfaState st6[9];
	static DfaState st7[9];
	static DfaState st8[9];
	static DfaState *dfa[9];
	static DfaState dfa_base[];
	static unsigned char *b_class_no[];
	static DfaState accepts[10];
	static DLGChar alternatives[10];
	static ANTLRTokenType (DLGLexerFulltextBoolean::*actions[9])();
	static unsigned char shift0[257];
	int ZZSHIFT(int c) { return b_class_no[automaton][1+c]; }
//
// 133MR1 Deprecated feature to allow inclusion of user-defined code in DLG class header
//
#ifdef DLGLexerIncludeFile
#include DLGLexerIncludeFile
#endif
};
typedef ANTLRTokenType (DLGLexerFulltextBoolean::*PtrDLGLexerFulltextBooleanMemberFunc)();
#endif
