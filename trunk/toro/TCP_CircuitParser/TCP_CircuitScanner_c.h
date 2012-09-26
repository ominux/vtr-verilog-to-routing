#ifndef TCP_CircuitScanner_c_h
#define TCP_CircuitScanner_c_h
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

class TCP_CircuitScanner_c : public DLGLexerBase {
public:
public:
	static const int MAX_MODE;
	static const int DfaStates;
	static const int START;
	static const int QUOTED_VALUE;
	typedef unsigned short DfaState;

	TCP_CircuitScanner_c(DLGInputStream *in,
		unsigned bufsize=2000)
		: DLGLexerBase(in, bufsize, 0)
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
	ANTLRTokenType act20();
	ANTLRTokenType act21();
	ANTLRTokenType act22();
	ANTLRTokenType act23();
	ANTLRTokenType act24();
	ANTLRTokenType act25();
	ANTLRTokenType act26();
	ANTLRTokenType act27();
	ANTLRTokenType act28();
	ANTLRTokenType act29();
	ANTLRTokenType act30();
	ANTLRTokenType act31();
	ANTLRTokenType act32();
	ANTLRTokenType act33();
	ANTLRTokenType act34();
	ANTLRTokenType act35();
	ANTLRTokenType act36();
	ANTLRTokenType act37();
	ANTLRTokenType act38();
	ANTLRTokenType act39();
	ANTLRTokenType act40();
	ANTLRTokenType act41();
	ANTLRTokenType act42();
	ANTLRTokenType act43();
	ANTLRTokenType act44();
	ANTLRTokenType act45();
	ANTLRTokenType act46();
	ANTLRTokenType act47();
	ANTLRTokenType act48();
	ANTLRTokenType act49();
	ANTLRTokenType act50();
	ANTLRTokenType act51();
	ANTLRTokenType act52();
	ANTLRTokenType act53();
	ANTLRTokenType act54();
	ANTLRTokenType act55();
	ANTLRTokenType act56();
	ANTLRTokenType act57();
	ANTLRTokenType act58();
	ANTLRTokenType act59();
	ANTLRTokenType act60();
	ANTLRTokenType act61();
	static DfaState st0[258];
	static DfaState st1[258];
	static DfaState st2[258];
	static DfaState st3[258];
	static DfaState st4[258];
	static DfaState st5[258];
	static DfaState st6[258];
	static DfaState st7[258];
	static DfaState st8[258];
	static DfaState st9[258];
	static DfaState st10[258];
	static DfaState st11[258];
	static DfaState st12[258];
	static DfaState st13[258];
	static DfaState st14[258];
	static DfaState st15[258];
	static DfaState st16[258];
	static DfaState st17[258];
	static DfaState st18[258];
	static DfaState st19[258];
	static DfaState st20[258];
	static DfaState st21[258];
	static DfaState st22[258];
	static DfaState st23[258];
	static DfaState st24[258];
	static DfaState st25[258];
	static DfaState st26[258];
	static DfaState st27[258];
	static DfaState st28[258];
	static DfaState st29[258];
	static DfaState st30[258];
	static DfaState st31[258];
	static DfaState st32[258];
	static DfaState st33[258];
	static DfaState st34[258];
	static DfaState st35[258];
	static DfaState st36[258];
	static DfaState st37[258];
	static DfaState st38[258];
	static DfaState st39[258];
	static DfaState st40[258];
	static DfaState st41[258];
	static DfaState st42[258];
	static DfaState st43[258];
	static DfaState st44[258];
	static DfaState st45[258];
	static DfaState st46[258];
	static DfaState st47[258];
	static DfaState st48[258];
	static DfaState st49[258];
	static DfaState st50[258];
	static DfaState st51[258];
	static DfaState st52[258];
	static DfaState st53[258];
	static DfaState st54[258];
	static DfaState st55[258];
	static DfaState st56[258];
	static DfaState st57[258];
	static DfaState st58[258];
	static DfaState st59[258];
	static DfaState st60[258];
	static DfaState st61[258];
	static DfaState st62[258];
	static DfaState st63[258];
	static DfaState st64[258];
	static DfaState st65[258];
	static DfaState st66[258];
	static DfaState st67[258];
	static DfaState st68[258];
	static DfaState st69[258];
	static DfaState st70[258];
	static DfaState st71[258];
	static DfaState st72[258];
	static DfaState st73[258];
	static DfaState st74[258];
	static DfaState st75[258];
	static DfaState st76[258];
	static DfaState st77[258];
	static DfaState st78[258];
	static DfaState st79[258];
	static DfaState st80[258];
	static DfaState st81[258];
	static DfaState st82[258];
	static DfaState st83[258];
	static DfaState st84[258];
	static DfaState st85[258];
	static DfaState st86[258];
	static DfaState st87[258];
	static DfaState st88[258];
	static DfaState st89[258];
	static DfaState st90[258];
	static DfaState st91[258];
	static DfaState st92[258];
	static DfaState st93[258];
	static DfaState st94[258];
	static DfaState st95[258];
	static DfaState st96[258];
	static DfaState st97[258];
	static DfaState st98[258];
	static DfaState st99[258];
	static DfaState st100[258];
	static DfaState st101[258];
	static DfaState st102[258];
	static DfaState st103[258];
	static DfaState st104[258];
	static DfaState st105[258];
	static DfaState st106[258];
	static DfaState st107[258];
	static DfaState st108[258];
	static DfaState st109[258];
	static DfaState st110[258];
	static DfaState st111[258];
	static DfaState st112[258];
	static DfaState st113[258];
	static DfaState st114[258];
	static DfaState st115[258];
	static DfaState st116[258];
	static DfaState st117[258];
	static DfaState st118[258];
	static DfaState st119[258];
	static DfaState st120[258];
	static DfaState st121[258];
	static DfaState st122[258];
	static DfaState st123[258];
	static DfaState st124[258];
	static DfaState st125[258];
	static DfaState st126[258];
	static DfaState st127[258];
	static DfaState st128[258];
	static DfaState st129[258];
	static DfaState st130[258];
	static DfaState st131[258];
	static DfaState st132[258];
	static DfaState st133[258];
	static DfaState st134[258];
	static DfaState st135[258];
	static DfaState st136[258];
	static DfaState st137[258];
	static DfaState st138[258];
	static DfaState st139[258];
	static DfaState st140[258];
	static DfaState st141[258];
	static DfaState st142[258];
	static DfaState st143[258];
	static DfaState st144[258];
	static DfaState st145[258];
	static DfaState st146[258];
	static DfaState st147[258];
	static DfaState st148[258];
	static DfaState st149[258];
	static DfaState st150[258];
	static DfaState st151[258];
	static DfaState st152[258];
	static DfaState st153[258];
	static DfaState st154[258];
	static DfaState st155[258];
	static DfaState st156[258];
	static DfaState st157[258];
	static DfaState st158[258];
	static DfaState st159[258];
	static DfaState st160[258];
	static DfaState st161[258];
	static DfaState st162[258];
	static DfaState st163[258];
	static DfaState st164[258];
	static DfaState st165[258];
	static DfaState st166[258];
	static DfaState st167[258];
	static DfaState st168[258];
	static DfaState st169[258];
	static DfaState st170[258];
	static DfaState st171[258];
	static DfaState st172[258];
	static DfaState st173[258];
	static DfaState st174[258];
	static DfaState st175[258];
	static DfaState st176[258];
	static DfaState st177[258];
	static DfaState st178[258];
	static DfaState st179[258];
	static DfaState st180[258];
	static DfaState st181[258];
	static DfaState st182[258];
	static DfaState st183[258];
	static DfaState st184[258];
	static DfaState st185[258];
	static DfaState st186[258];
	static DfaState st187[258];
	static DfaState st188[258];
	static DfaState st189[258];
	static DfaState st190[258];
	static DfaState st191[258];
	static DfaState st192[258];
	static DfaState st193[258];
	static DfaState st194[258];
	static DfaState st195[258];
	static DfaState st196[258];
	static DfaState st197[258];
	static DfaState st198[258];
	static DfaState st199[258];
	static DfaState st200[258];
	static DfaState st201[258];
	static DfaState st202[258];
	static DfaState st203[258];
	static DfaState st204[258];
	static DfaState st205[258];
	static DfaState st206[258];
	static DfaState st207[258];
	static DfaState st208[258];
	static DfaState st209[258];
	static DfaState st210[258];
	static DfaState st211[258];
	static DfaState st212[258];
	static DfaState st213[258];
	static DfaState st214[258];
	static DfaState st215[258];
	static DfaState st216[258];
	static DfaState st217[258];
	static DfaState st218[258];
	static DfaState st219[258];
	static DfaState st220[258];
	static DfaState st221[258];
	static DfaState st222[258];
	static DfaState st223[258];
	static DfaState st224[258];
	static DfaState st225[258];
	static DfaState st226[258];
	static DfaState *dfa[227];
	static DfaState dfa_base[];
	static unsigned char *b_class_no[];
	static DfaState accepts[228];
	static DLGChar alternatives[228];
	static ANTLRTokenType (TCP_CircuitScanner_c::*actions[62])();
	static unsigned char shift0[257];
	static unsigned char shift1[257];
	int ZZSHIFT(int c) { return 1+c; }
//
// 133MR1 Deprecated feature to allow inclusion of user-defined code in DLG class header
//
#ifdef DLGLexerIncludeFile
#include DLGLexerIncludeFile
#endif
};
typedef ANTLRTokenType (TCP_CircuitScanner_c::*PtrTCP_CircuitScanner_cMemberFunc)();
#endif
