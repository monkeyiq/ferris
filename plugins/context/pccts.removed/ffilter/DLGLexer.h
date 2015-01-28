#ifndef DLGLexer_h
#define DLGLexer_h
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

class DLGLexer : public DLGLexerBase {
public:


protected:
int          isWS;
int          begline;
virtual void tabAdjust() {}
public:
	static const int MAX_MODE;
	static const int DfaStates;
	static const int START;
	static const int EAVALUECLASS;
	typedef unsigned char DfaState;

	DLGLexer(DLGInputStream *in,
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
	ANTLRTokenType act9();
	ANTLRTokenType act10();
	ANTLRTokenType act11();
	ANTLRTokenType act12();
	ANTLRTokenType act13();
	ANTLRTokenType act14();
	ANTLRTokenType act15();
	ANTLRTokenType act16();
	ANTLRTokenType act17();
	ANTLRTokenType act18();
	ANTLRTokenType act19();
	static DfaState st0[16];
	static DfaState st1[16];
	static DfaState st2[16];
	static DfaState st3[16];
	static DfaState st4[16];
	static DfaState st5[16];
	static DfaState st6[16];
	static DfaState st7[16];
	static DfaState st8[16];
	static DfaState st9[16];
	static DfaState st10[16];
	static DfaState st11[16];
	static DfaState st12[16];
	static DfaState st13[16];
	static DfaState st14[16];
	static DfaState st15[16];
	static DfaState st16[16];
	static DfaState st17[16];
	static DfaState st18[16];
	static DfaState st19[16];
	static DfaState st20[16];
	static DfaState st21[16];
	static DfaState st22[16];
	static DfaState st23[16];
	static DfaState st24[4];
	static DfaState st25[4];
	static DfaState st26[4];
	static DfaState st27[4];
	static DfaState *dfa[28];
	static DfaState dfa_base[];
	static unsigned char *b_class_no[];
	static DfaState accepts[29];
	static DLGChar alternatives[29];
	static ANTLRTokenType (DLGLexer::*actions[20])();
	static unsigned char shift0[257];
	static unsigned char shift1[257];
	int ZZSHIFT(int c) { return b_class_no[automaton][1+c]; }
//
// 133MR1 Deprecated feature to allow inclusion of user-defined code in DLG class header
//
#ifdef DLGLexerIncludeFile
#include DLGLexerIncludeFile
#endif
};
typedef ANTLRTokenType (DLGLexer::*PtrDLGLexerMemberFunc)();
#endif
