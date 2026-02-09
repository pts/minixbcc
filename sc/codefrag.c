/* codefrag.c - code fragments for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "const.h"
#include "types.h"
#include "condcode.h"
#include "gencode.h"
#include "label.h"
#include "output.h"
#include "reg.h"
#include "scan.h"
#include "sizes.h"

#define DEFSTR_BYTEMAX 10
#define DEFSTR_DELIMITER '"'
#define DEFSTR_STRINGMAX 40
#define EOS_TEXT '0'
#define MAXPRINTCHAR '~'
#define MINPRINTCHAR ' '

/* segment numbers */

#define CSEG 0
#define outcseg() outop0str(".text\n")
#define DSEG 1
#define outdseg() outop0str(".data\n")
#define BSSSEG 2
#define outbssseg() outop0str(".bss\n")

FORWARD void adjcarry P((void));
FORWARD void clr P((store_pt reg));
FORWARD bool_pt lowregisDreg P((void));
FORWARD void outand P((void));
FORWARD void outequate P((void));
FORWARD void outmovsx P((void));
FORWARD void outmovzx P((void));
FORWARD void tfrhilo P((void));
FORWARD void tfrlohi P((void));
FORWARD void outaccum P((void));
FORWARD void outstackreg P((void));
FORWARD void opregadr P((void));

/* operator and miscellaneous strings */

#define ACCHISTR "ah"
#define ANDSTRING "and\t"
#define DEFSTR_QUOTER '\\'
#define EORSTRING "xor\t"
#define MAX_INLINE_SHIFT 2	/* better 3 for 88, 1 for 186 and above */
#define ORSTRING "or\t"
#define TARGET_FIRST
#define addfactor(reg) (outadd(), outregname(reg), outncregname(DXREG))
#define defstorage() outop0str(".blkb\t")
#define extBnegD() (ctoi(), negDreg())
#define finishfactor()		/* save/add/subfactor() ended already */
#define outadc() outop3str("adc\t")
#define outandac() (outand(), outaccum(), bumplc())
#define outandlo() (outand(), outstr(acclostr))
#define outbimmed() outbyte('*')
#define outcommon() outop0str(".comm\t")
#define outcwd() outnop1str("cwd")
#define outdefstr() outop0str(".ascii\t\"")
#define outexchange() outop1str("xchg\t")
#define outglobl() outop0str(".globl\t")
#define outexport() outop0str("export\t")
#define outimport() outop0str("import\t")
#define outj1switch() outop3str("seg\tcs\nbr\t");
#define outj2switch() \
	(outindleft(), outstr(ireg0str), outindright(), bumplc2(), outnl())
#define outlcommon() outop0str("\tlcomm\t")
#define outlswitch() (outload(), outstr(ireg0str), outncregname(DREG))
#define outnc1() outnstr(",*1")
#define outsbc() outop3str("sbb\t")
#define outset() outstr ("\tset\t")
#define outsl() outop2str("shl\t")
#define outsr() outop2str("sar\t")
#define outtransfer() outload()
#define outusr() outop2str("shr\t")
#define outxor() outop2str(EORSTRING)
#define reclaimfactor()	/* factor in DXREG, DXREG now junk */
#define savefactor(reg) regtransfer((reg), DXREG)
#define smiDreg() (outcwd(), regexchange(DREG, DXREG))
#define sr1() (outsr(), outaccum(), outnc1())
#define subfactor(reg) (outsub(), outregname(reg), outncregname(DXREG))
#define usr1() (outusr(), outaccum(), outnc1())
PRIVATE void adjcarry P0()
{
    outop3str("rcl\t");
    outregname(DXREG);
    outncimmadr((offset_t) 9);
    outand();
    bumplc2();
    bumplc2();
    outregname(DXREG);
    outncimmadr((offset_t) 0x100);
}
PUBLIC void clrBreg P0()
{
    outxor();
    outstr(acclostr);
    outncregname(BREG);
}
PUBLIC void comment P0()
{
    outstr("! ");
}
PUBLIC void ctoi P0()
{
    if (i386_32)
    {
	outmovzx();
	outaccum();
	outncregname(BREG);
    }
    else
    {
	outxor();
	outhiaccum();
	outcomma();
	outhiaccum();
	outnl();
    }
}
PUBLIC void defbyte P0()
{
    outop0str(".byte\t");
}
PUBLIC void defword P0()
{
    outop0str(".word\t");
}
PUBLIC void defdword P0()
{
    outop0str("dd\t");
}
PUBLIC void even P0()
{
    outop0str(".even\n");
}
PUBLIC void negDreg P0()
{
    outop2str("neg\t");
    outnregname(DREG);
}
PUBLIC void comDreg P0()
{
    outop2str("not\t");
    outnregname(DREG);
}
PUBLIC void outadd P0()
{
    outop2str("add\t");
}
PUBLIC void outaddsp P0()
{
    outadd();
    outstackreg();
    outcomma();
    outimmed();
    bumplc2();
}
PRIVATE void outand P0()
{
    outop2str(ANDSTRING);
}
PUBLIC void outcalladr P0()
{
    outop2str("call\t");
}
PUBLIC void outcmp P0()
{
    outop2str("cmp\t");
}
PUBLIC void outdec P0()
{
    outop1str("dec\t");
}
PUBLIC void outdword P0()
{
    outstr("dword ");
}
PRIVATE void outequate P0()
{
    outop0str("\t=\t");
}
PUBLIC void outfail P0()
{
    outop0str(".fail\t");
}
PUBLIC void outinc P0()
{
    outop1str("inc\t");
}
PUBLIC void outindleft P0()
{
    outbyte('[');
}
PUBLIC void outindright P0()
{
    outbyte(']');
}
#ifndef FRAMEPOINTER
PUBLIC void outindstackreg P0()
{
    outindleft();
    outregname(STACKREG);
    outindright();
}
#endif
PUBLIC void outldaccum P0()
{
    outload();
    outaccum();
    outcomma();
}
PUBLIC void outldmulreg P0()
{
    outload();
    outregname(MULREG);
    outcomma();
}
PUBLIC void outlea P0()
{
    outop2str("lea\t");
}
PUBLIC void outleasp P0()
{
    outlea();
    outstackreg();
    outcomma();
}
PUBLIC void outload P0()
{
    outop2str("mov\t");
}
PRIVATE void outmovsx P0()
{
    outop3str("movsx\t");
}
PRIVATE void outmovzx P0()
{
    outop3str("movzx\t");
}
PUBLIC void outmulmulreg P0()
{
    outop2str("mul\t");
    outnregname(MULREG);
}
PUBLIC void outopsep P0()
{
    outcomma();
}
PUBLIC void outpshs P0()
{
    outop1str("push");
}
PUBLIC void outpuls P0()
{
    outop1str("pop");
}
PUBLIC void outreturn P0()
{
    outnop1str("ret");
}
PUBLIC void outstore P0()
{
    outload();
}
PUBLIC void outsub P0()
{
    outop2str("sub\t");
}
PUBLIC void outtest P0()
{
    outop2str("test\t");
}
PUBLIC void outword P0()
{
    outstr("word ");
}
PUBLIC void sctoi P0()
{
    if (i386_32)
    {
	outmovsx();
	outncregname(BREG);
    }
    else
	outnop1str("cbw");
}
PUBLIC void stoi P0()
{
    outnop1str("cwde");
}
PRIVATE void tfrhilo P0()
{
    outload();
    outstr(acclostr);
    outcomma();
    outhiaccum();
    outnl();
}
PRIVATE void tfrlohi P0()
{
    outload();
    outhiaccum();
    outncregname(BREG);
}
PUBLIC void ustoi P0()
{
    outmovzx();
    outaccum();
    outcomma();
    outshortregname(DREG);
    outnl();
}

#ifdef FRAMEREG
PUBLIC void outindframereg P0()
{
    outindleft();
    outregname(FRAMEREG);
    outindright();
}
#endif

typedef fastin_t seg_t;		/* range 0..3 */

PRIVATE seg_t segment;		/* current seg, depends on init to CSEG = 0 */

/* add carry resulting from char addition */

PUBLIC void adc0 P0()
{
    if (i386_32)
    {
	adjcarry();
	outadd();
	outaccum();
	outncregname(DXREG);
    }
    else
    {
	outadc();
	outhiaccum();
	outncimmadr((offset_t) 0);
    }
}

/* add constant to register */

PUBLIC void addconst P2(offset_t, offset, store_pt, reg)
{
    /* !! Optimize this condition for code size. */
    if ((i386_32 && (uoffset_t) offset + 1 <= 2)	/* do -1 to 1 by dec/inc */
	|| (!i386_32 && (uoffset_t) offset + 2 <= 4))	/* do -2 to 2  */
    {
	if (reg == ALREG)
	    reg = AXREG;	/* shorter and faster */
	do
	{
	    if (offset < 0)
	    {
		outdec();
		++offset;
	    }
	    else		/* if offset == 0, do inc + dec */
	    {
		outinc();
		--offset;	/* shouldn't happen and harmless */
	    }
	    outnregname(reg);
	}
	while (offset);
    }
    else
    {
	outadd();
	outimadj(offset, reg);
    }
}

/* adjust lc for signed offset */

PUBLIC void adjlc P2(offset_t, offset, store_pt, reg)
{
    if (!(reg & CHARREGS))
    {
	bumplc();
	if (!isbyteoffset(offset))
	{
	    if ((store_t) reg != AXREG)
		bumplc();
	    if (i386_32)
		bumplc2();
	}
    }
}

/* adjust stack ptr by adding a labelled constant less current sp */

PUBLIC void adjsp P1(label_t, label)
{
    outaddsp();
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    if (switchnow != (struct switchstruct*) 0)
    {
	outminus();
	outswstacklab();
    }
    else
    {
	outplus();
	outhex((uoffset_t) - sp);
    }
    if (i386_32)
	bumplc2();
    outnl();
}

/* and accumulator with constant; this only touches AL if the constat is small */

PUBLIC void andconst P1(offset_t, offset)
{
    char_t botbits;
    uoffset_t topbits;

    if ((topbits = offset & ~(uoffset_t) CHMASKTO & intmaskto) != 0 &&
	topbits != (~(uoffset_t) CHMASKTO & intmaskto))
	/* if topbits == 0, callers reduce the type */
    {
#ifdef OP1
	outandhi();
	outncimmadr((offset_t) (topbits >> (INT16BITSTO - CHBITSTO)));
#else
	outandac();
	if (i386_32)
	    bumplc2();
	outncimmadr(offset);
	return;
#endif
    }
    if ((botbits = (char_t) offset & CHMASKTO) == 0)
	clrBreg();
    else if (botbits != CHMASKTO)
    {
	outandlo();
	outncimmadr((offset_t) botbits);
    }
}

/* set bss segment */

PUBLIC void bssseg P0()
{
    if (segment != BSSSEG)
    {
	segment = BSSSEG;
	outbssseg();
    }
}

/* jump to case of switch */

PUBLIC label_t casejump P0()
{
    label_t jtablelab;

    outlswitch();
    outj1switch();
    outlabel(jtablelab = getlabel());
    outj2switch();
    if (i386_32)
	bumplc2();
    return jtablelab;
}

/* clear register to 0 */

PRIVATE void clr P1(store_pt, reg)
{
    loadconst((offset_t) 0, reg);
}

/* define common storage */

PUBLIC void common P1(_CONST char *, name)
{
    outcommon();
    outccname(name);
    outcomma();
}

/* set code segment */

PUBLIC void cseg P0()
{
    if (segment != CSEG)
    {
	segment = CSEG;
	outcseg();
    }
}

/* define long */

PUBLIC void deflong P1(uoffset_t, value)
{
    uoffset_t longhigh;
    uoffset_t longlow;

    longlow = value & (uoffset_t) intmaskto;
    if (i386_32)
	defdword();
    else
    {
	longhigh = (value >> INT16BITSTO) & (uoffset_t) intmaskto;
	defword();
	{
	    outnhex(longlow);
	    longlow = longhigh;
	}
	defword();
    }
    outnhex(longlow);
}

/* define null storage */

PUBLIC void defnulls P1(uoffset_t, nullcount)
{
    if (nullcount != 0)
    {
	defstorage();
	outnhex(nullcount);
    }
}

/* define string */

PUBLIC label_t defstr P3(char *, sptr, char *, stop, bool_pt, dataflag)
{
    int byte;			/* promoted char for output */
    label_t strlab;
    seg_t oldsegment;
    fastin_t count;		/* range 0..max(DEFSTR_BYTEMAX,DEFSTR_STRMAX) */

#ifdef HOLDSTRINGS
    if (!(bool_t) dataflag)
	return holdstr(sptr, stop);
#endif
    oldsegment = segment;
    dseg();
    outnlabel(strlab = getlabel());
    byte = (unsigned char) *sptr++;
    while (sptr <= stop)
    {
	if ((unsigned char) byte >= MINPRINTCHAR
	    && (unsigned char) byte <= MAXPRINTCHAR)
	{
	    outdefstr();
	    count = DEFSTR_STRINGMAX;
	    while (count-- > 0 && (unsigned char) byte >= MINPRINTCHAR
		   && (unsigned char) byte <= MAXPRINTCHAR && sptr <= stop)
	    {
#if DEFSTR_DELIMITER - DEFSTR_QUOTER
		if ((unsigned char) byte == DEFSTR_DELIMITER
		    || (unsigned char) byte == DEFSTR_QUOTER)
#else
		if ((unsigned char) byte == DEFSTR_DELIMITER)
#endif
		    outbyte(DEFSTR_QUOTER);
		outbyte(byte);
		byte = (unsigned char) *sptr++;
	    }
	    outnbyte(DEFSTR_DELIMITER);
	}
	else
	{
	    defbyte();
	    count = DEFSTR_BYTEMAX;
	    while (count-- > 0 && ((unsigned char) byte < MINPRINTCHAR
		   || (unsigned char) byte > MAXPRINTCHAR) && sptr <= stop)
	    {
		if (count < DEFSTR_BYTEMAX - 1)
		    outcomma();	/* byte separator */
		outhex((uoffset_t) byte);
		byte = (unsigned char) *sptr++;
	    }
	    outnl();
	}
    }
    defbyte();
    outnbyte(EOS_TEXT);
    switch (oldsegment)
    {
    case CSEG:
	cseg();
	break;
    case DSEG:
	dseg();
	break;
    case BSSSEG:
	bssseg();
	break;
    }
    return strlab;
}

FORWARD void outsxaccum P((void));

/* Sign-extend the accumulator to DATAREG2. */

PRIVATE void outsxaccum P0()
{
    outnop1str(i386_32 ? "cdq" : "cwd");  /* !! Replace outcwd() with outsxaccum(), even for i386_32. */
}

FORWARD void outaccandreg2n P((void));

PRIVATE void outaccandreg2n P0()
{
	outaccum(); outcomma(); outnstr(dreg2str);  /* This desn't call bumplc(). */
}

/* divide D register by a constant if it is easy to do with shifts */

PUBLIC bool_pt diveasy P2(value_t, divisor, bool_pt, uflag)
{
    int hb;

    if (divisor < 0 && !(bool_t) uflag) return FALSE;  /* There is no way to avoid idiv, no (reasonable) bit operations work for dividing INT_MIN with the negative of a power of 2. */
    if (divisor == 0) { clr(DREG); goto done; }
    if ((divisor & (divisor - 1))) return FALSE;  /* divisor is not a power of 2. */
    if ((hb = highbit((uvalue_t) divisor)) == 31 && !(bool_t) uflag && i386_32)  /* Convert (value_t)-0x80000000L to 1, everything else to 0. */
    {
	outnop2str("rol\teax,*1");
	outnop1str("dec\teax");
	outnop2str("neg\teax");
	outnop2str("sbb\teax,eax");
	outnop1str("inc\teax");
    }
    else if (!(bool_t) uflag && !i386_32 && hb == 15)  /* Convert -32768 to 1, everything else to 0. */
    {
	outnop2str("xor\tah,*$80"); bumplc();
	outnop2str("neg\tax");
	outnop2str("sbb\tax,ax");
	outnop1str("inc\tax");
    }
    else
    {
	if ((bool_t) uflag || hb == 0) goto do_srconst;
	outsxaccum();
	if (hb == 1) { outsub(); outaccandreg2n(); goto do_srconst; }
	if (hb < 8)
	{
	    outand();  /* This calls outop2str(), which calls bumplc2(). */
	    outstr(dreg2str);  /* "edx" or "dx". */
	    outncimmadr((offset_t) --divisor); bumplc();
	    outadd(); outaccandreg2n();
	    if (hb == 7 && !i386_32) {  /* Output code size and speed optimization. It would work without this. */
		outnop2str("add\tax,ax");
		outnop2str("mov\tal,ah");
		outnop2str("sbb\tah,ah");
		goto done;  /* Don't call srconst(...), we are done. */
	    }
	    goto do_srconst;
	}
	if (i386_32)  /* Increase negative EAX before right shift (sar) to do rounding towards 0 (consistent with the i86 and i386 idiv instruction, and required by C99). */
	{
	    outsl();  /* This calls outop2str(), which calls bumplc2(). */
	    outstr(dreg2str);  /* edx. */
	    outncimmadr((offset_t) hb); bumplc();
	    outsbc(); outaccandreg2n();
	    goto do_srconst;
	}
	if (hb == 8)  /* Output code size and speed optimization. It would work without this. */
	{
	    outnop2str("add\tal,dl");
	    outnop2str("mov\tal,ah");
	    outnop2str("adc\tal,*0");
	    outnop1str("cbw");
	    goto done;  /* Don't call srconst(...), we are done. */
	}
	/* Increase negative AX before right shift (sar) to do rounding towards 0 (consistent with the i86 and i386 idiv instruction, and required by C99). */
	outand();  /* This calls outop2str(), which calls bumplc2(). */
	outstr("dh");
	outncimmadr((offset_t) (((unsigned) divisor - 1) >> 8)); bumplc();
	outadd(); outaccandreg2n();
      do_srconst:
	srconst((value_t) hb, uflag);  /* For !uflag, this generates the `sar ..., *1' instruction, which rounds down. We've adjusted the input above so that in total it will round towards 0. */
    }
  done:
    return TRUE;
}

/* set data segment */

PUBLIC void dseg P0()
{
    if (segment != DSEG)
    {
	segment = DSEG;
	outdseg();
    }
}

/* equate a name to an EOL-terminated string */

PUBLIC void equ P2(_CONST char *, name, _CONST char *, string)
{
    outstr(name);
    outequate();
    outline(string);
}

/* equate a local label to a value */

PUBLIC void equlab P2(label_t, label, offset_t, offset)
{
    outbyte(LOCALSTARTCHAR);
    outlabel(label);
    outequate();
    outshex(offset);
    outnl();
}

/* import or export a variable */

PUBLIC void globl P1(_CONST char *, name)
{
    outglobl();
    outnccname(name);
}

/* import a variable */

PUBLIC void import P1(_CONST char *, name)
{
    outimport();
    outnccname(name);
}

/* extend an int to a long */

PUBLIC void itol P1(store_pt, reg)
{
#define TEMP_LABEL_FOR_REGRESSION_TESTS
#ifdef TEMP_LABEL_FOR_REGRESSION_TESTS
    getlabel();
#endif

    if (lowregisDreg())
    {
	outcwd();
	regtransfer(DXREG, reg);
    }
    else
    {
	regtransfer(DREG, reg);
	smiDreg();
    }
}

/* define local common storage */

PUBLIC void lcommlab P1(label_t, label)
{
    outlabel(label);
    outlcommon();
}

PUBLIC void lcommon P1(_CONST char *, name)
{
    outccname(name);
    outlcommon();
}

/* load constant into given register */

PUBLIC void loadconst P2(offset_t, offset, store_pt, reg)
{
    if (offset == 0)
    {
	outxor();
	outregname(reg);
	outncregname(reg);
    }
    else
    {
	outload();
	outregname(reg);
	if (reg != BREG)
	{
	    bumplc();
	    if (i386_32)
		bumplc2();
	}
	outncimmadr(offset);
    }
}

/* convert index half of long reg pair into low half of pair */

PRIVATE bool_pt lowregisDreg P0()  /* !! inline */
{
	return TRUE;
}

/* partially long shift left register by a constant (negative = infinity) */

PUBLIC int lslconst P2(value_t, shift, store_pt, reg)
{
    if ((uvalue_t) shift >= INT16BITSTO)
    {
	slconst(shift - INT16BITSTO, lowregisDreg() ? DREG : reg);
	regexchange(reg, DREG);
	clr(lowregisDreg() ? DREG : reg);
	return 0;
    }
    if (shift >= CHBITSTO)
    {
	outnop2str("mov\tbh,bl");
	outnop2str("mov\tbl,ah");
	tfrlohi();
	clrBreg();
	return (int) shift - CHBITSTO;
    }
    return (int) shift;
}

/* partially long shift right register by a constant (negative = infinity) */

PUBLIC int lsrconst P3(value_t, shift, store_pt, reg, bool_pt, uflag)
{
    if ((uvalue_t) shift >= INT16BITSTO)
    {
	if (lowregisDreg())
	    regexchange(reg, DREG);
	srconst(shift - INT16BITSTO, uflag);
	if ((bool_t) uflag)
	    uitol(reg);
	else
	    itol(reg);
	return 0;
    }
    if (shift >= CHBITSTO)
    {
	tfrhilo();
	outnop2str("mov\tah,bl");
	outnop2str("mov\tbl,bh");
	if ((bool_t) uflag)
	    outnop2str("sub\tbh,bh");
	else
	{
	    regexchange(reg, DREG);
	    sctoi();
	    regexchange(reg, DREG);
	}
	return (int) shift - CHBITSTO;
    }
    return (int) shift;
}

FORWARD void andrconst P((bool_pt is_accum, value_t value));

/* and REG (is_accum) or DATAREG2 with the specified constant. */

PRIVATE void andrconst P2(bool_pt, is_accum, value_t, value)
{
    if (value == 0) { clr(is_accum ? DREG : DATREG2); return; }
    outand();  /* Calls 2 * bumplc(). */
    bumplc();  /* 1 or 2 bytes of value. */
    if (is_accum)
    {
	outaccum();
    }
    else
    {
	if (!i386_32 && ((int) value & 0xff) == 0xff)
	{
	    outstr("dh");
	    outncimmadr((offset_t) ((unsigned) value >> 8));
	    return;
	}
	outstr(dreg2str);
    }
    outncimmadr((offset_t) value);
    if ((uvalue_t) value > MAXUCHTO + 1)
    {
	if (!is_accum) bumplc();  /* 1 byte because of non-accumulator register. */
	if (i386_32) bumplc2();  /* 2 bytes of value. */
    }
}

/* take D register modulo a constant if it is easy to do with a mask */

PUBLIC bool_pt modeasy P2(value_t, divisor, bool_pt, uflag)
{
    bool_pt is_div_by_2;

    if (divisor < 0 && !(bool_t) uflag)  /* For a round-towards-zero modulo calculation, we just take the absolute value of the divisor. */
    {
#ifdef ACKFIX  /* For Minix 1.5.10 i86 ACK 3.1 C compiler, no matter the optimization setting (cc -O). */  /* Fix not needed when compiling this BCC sc by this BCC sc compiled with ACK. */
	divisor = (value_t) (~(uvalue_t) divisor + 1);  /* It works with any C compiler doing 2s complement arithmetic. */
#else
	divisor = (value_t) -(uvalue_t) divisor;  /* The Minix 1.5.10 i86 ACK 3.1 C compiler is buggy: it only negates the low 16 bits of the 32-bit variable here. */
#endif
    }
    if ((uvalue_t) divisor <= 1) { clr(DREG); return TRUE; }  /* original divisor 1 or -1 yields 0 */
    if ((divisor & (divisor - 1))) return FALSE;  /* divisor is not a power of 2. */
    --divisor;
    if ((bool_t) uflag) goto do_and;
    outsxaccum();
    if (!i386_32)
    {
	if (divisor == 256)  /* Output code size and speed optimization. It would work without this. */
	{
	    outnop2str("mov\tdh,*0");
	    outadd(); outaccandreg2n();
	    outnop2str("mov\tah,*0");
	    goto do_sub;
	}
	if ((uvalue_t) divisor == (uvalue_t)1 << 15)  /* Output code size and speed optimization. It would work without this. */
	{
	    outnop2str("mov\tdx,ax");
	    outnop2str("xor\tdh,*$80"); bumplc();
	    outnop2str("neg\tdx");
	    outnop2str("sbb\tdx,dx");
	    outand();
	    goto do_last2args;
	}
    }
    else if ((uvalue_t) divisor == (uvalue_t)1 << 31)  /* Output code size and speed optimization. It would work without this. */
    {
	outnop2str("mov\tedx,eax");
	outnop2str("rol\tedx,*$1");
	outnop1str("dec\tedx");
	outnop2str("neg\tedx");
	outnop2str("sbb\tedx,edx");
	outand();
	goto do_last2args;
    }
    is_div_by_2 = divisor == 1;
    andrconst(is_div_by_2, (value_t) divisor);
    if (is_div_by_2)
    {
	outxor(); outaccandreg2n();
    }
    else
    {
	outadd(); outaccandreg2n();  /* Increase negative EAX to do rounding towards 0 (consistent with the i86 and i386 idiv instruction, and required by C99). */
      do_and:
	andrconst(TRUE, (value_t) divisor);
    }
  do_sub:
    if (!(bool_t) uflag) { outsub(); do_last2args: outaccandreg2n(); }
    return TRUE;
}

/* multiply register by a constant if it is easy to do with shifts */

PUBLIC bool_pt muleasy P2(uvalue_t, factor, store_pt, reg)
{
    int mulstack[MAXINTBITSTO / 2 + 1];	/* must be signed, not a fastin_t */
    fastin_pt count;
    fastin_t single1skip;
    fastin_t lastcount;
    int mulsp;
    int stackentry;		/* signed */

    if (factor == 0)
    {
	clr(reg);
	return TRUE;
    }
    single1skip = 0;
    mulsp = -1;			/* may be unsigned, but bumps to 0 */
    while (factor != 0)
    {
	for (lastcount = single1skip; (factor & 1) == 0; factor >>= 1)
	    ++lastcount;
	mulstack[++mulsp] = lastcount;
	/* first time bumps mulsp to 0 even if an unsigned char */
	for (count = 0; (factor & 1) != 0; factor >>= 1)
	    ++count;
	single1skip = 1;
	if (count == 2 && factor == 0)
	    /* 3 = 2 + 1  better than  3 = 4 - 1 */
	    /* but rest of algorithm messed up unless factor now 0 */
	    mulstack[++mulsp] = 1;
	else if (count > 1)
	{
	    single1skip = 0;
	    if (lastcount == 1 && mulsp != 0)
		mulstack[mulsp] = -1 - count;
	    else
		mulstack[++mulsp] = -count;
	}
    }
    if (mulsp > 3)
	return FALSE;
    if (mulsp != 0)
    {
	savefactor(reg);	/* on stack or in reg as nec */
	do
	{
	    finishfactor();	/* finish save/add/subfactor() if nec */
	    stackentry = mulstack[mulsp--];
	    if (stackentry < 0)
	    {
		if (stackentry == -INT32BITSTO)
		    clr(reg);	/* shifting would do nothing */
		else
		    slconst((value_t) - stackentry, reg);
		subfactor(reg);	/* from wherever put by savefactor() */
	    }
	    else
	    {
		slconst((value_t) stackentry, reg);
		addfactor(reg);	/* from wherever put by savefactor() */
	    }
	}
	while (mulsp != 0);
	reclaimfactor();	/* reclaim storage if nec */
    }
    slconst((value_t) mulstack[0], reg);
    return TRUE;
}

/* negate a register */

PUBLIC void negreg P1(store_pt, reg)
{
    if ((store_t) reg == BREG)
	extBnegD();
    else
	negDreg();
}

/* return string of operator */

PUBLIC _CONST char *opstring P1(op_pt, op)
{
    switch (op)
    {
    case ANDOP:
	return ANDSTRING;
    case EOROP:
	return EORSTRING;
    case OROP:
	return ORSTRING;
    }
    return "badop";
}

/* print DREG (accumulator) */

PRIVATE void outaccum P0()
{
    outstr(accumstr);
}

/* print a c compiler name with leading CCNAMEPREXFIX */

PUBLIC void outccname P1(_CONST char *, name)
{
    outbyte(CCNAMEPREFIX);
    outstr(name);
}

/* print high byte of word accumulator */

PUBLIC void outhiaccum P0()
{
    outstr(ACCHISTR);
}

/* print immediate address */

PUBLIC void outimmadr P1(offset_t, offset)
{
    if (!isbyteoffset(offset))
	outimmed();
    else
	outbimmed();
    outshex(offset);
}

/* print register, comma, immediate address and adjust lc */

PUBLIC void outimadj P2(offset_t, offset, store_pt, targreg)
{
    outregname(targreg);
    adjlc(offset, targreg);
    outncimmadr(offset);
}

/* print immediate address designator */

PUBLIC void outimmed P0()
{
    outbyte('#');
}

PUBLIC void outjumpstring P0()
{
    outop3str(jumpstring);
    if (i386_32)
	bumplc2();
}

/* print cc name, then newline */

PUBLIC void outnccname P1(_CONST char *, name)
{
    outccname(name);
    outnl();
}

/* print separator, immediate address, newline */

PUBLIC void outncimmadr P1(offset_t, offset)
{
    outcomma();
    outimmadr(offset);
    outnl();
}

/* print signed offset and adjust lc */

PUBLIC void outoffset P1(offset_t, offset)
{
    adjlc(offset, INDREG0);
    outshex(offset);
}

/* print stack register */

PRIVATE void outstackreg P0()
{
    outstr(stackregstr);
}

PUBLIC void public_ P1(_CONST char *, name)
{
    outexport();
    outnccname(name);
    outccname(name);
    outnbyte(PUBLICENDCHAR);
}

/* print cc name as a private label */

PUBLIC void private_ P1(_CONST char *, name)
{
#ifdef LABELENDCHAR
    outccname(name);
    outnbyte(LABELENDCHAR);
#else
    outnccname(name);
#endif
}

/* exchange registers */

PUBLIC void regexchange P2(store_pt, sourcereg, store_pt, targreg)
{
    outexchange();
    outregname(sourcereg);
    outncregname(targreg);
    if (!((sourcereg | targreg) & AXREG))
	bumplc();
}

/* transfer a register */

PUBLIC void regtransfer P2(store_pt, sourcereg, store_pt, targreg)
{
    outtransfer();
#ifdef TARGET_FIRST
    outregname(targreg);
    outncregname(sourcereg);
#else
    outregname(sourcereg);
    outncregname(targreg);
#endif
}

/* subtract carry resulting from char addition */

PUBLIC void sbc0 P0()
{
    if (i386_32)
    {
	adjcarry();
	outsub();
	outaccum();
	outncregname(DXREG);
    }
    else
    {
	outsbc();
	outhiaccum();
	outncimmadr((offset_t) 0);
    }
}

/* set a name to a value */

PUBLIC void set P2(_CONST char *, name, offset_t, value)
{
    outccname(funcname);
    outbyte(LOCALSTARTCHAR);
    outstr(name);
    outset();
    outshex(value);
    outnl();
}

/* shift left register by 1 */

PUBLIC void sl1 P1(store_pt, reg)
{
    outsl();
    outregname(reg);
    outnc1();
}

/* shift left register by a constant (negative = infinity) */

PUBLIC void slconst P2(value_t, shift, store_pt, reg)
{
    if (i386_32)
    {
	if ((shift = (uvalue_t) shift % INT32BITSTO) != 0)
	{
	    outsl();
	    if (shift != 1)
		bumplc();
	    outregname(reg);
	    outncimmadr((offset_t) shift);
	}
	return;
    }
    if ((uvalue_t) shift >= INT16BITSTO)
	clr(reg);
    else
    {
	if (shift >= CHBITSTO && reg == DREG)
	{
	    tfrlohi();
	    clrBreg();
	    shift -= CHBITSTO;
	}
#if MAX_INLINE_SHIFT < INT16BITSTO
	if (shift > MAX_INLINE_SHIFT)
	{
	    outload();
	    outregname(SHIFTREG);
	    outcomma();
	    outimmadr((offset_t) shift);
	    outnl();
	    outsl();
	    outregname(reg);
	    outncregname(SHIFTREG);
	}
	else
#endif
	    while (shift--)
		sl1(reg);
    }
}

/* shift right D register by a constant (negative = infinity) */

PUBLIC void srconst P2(value_t, shift, bool_pt, uflag)
{
    if (i386_32)
    {
	if ((shift = (uvalue_t) shift % INT32BITSTO) != 0)
	{
	    if (uflag)
		outusr();
	    else
		outsr();
	    if (shift != 1)
		bumplc();
	    outaccum();
	    outncimmadr((offset_t) shift);
	}
	return;
    }
    if ((uvalue_t) shift >= INT16BITSTO)	/* covers negatives too */
    {
	if ((bool_t) uflag)
	    clr(DREG);
	else			/* make D == 0 if D >= 0, else D == -1 */
	    smiDreg();
    }
    else
    {
	if (shift >= CHBITSTO)
	{
	    tfrhilo();
	    if ((bool_t) uflag)
		ctoi();
	    else
		sctoi();
	    shift -= CHBITSTO;
	}
#if MAX_INLINE_SHIFT < INT16BITSTO
	if (shift > MAX_INLINE_SHIFT)
	{
	    outload();
	    outregname(SHIFTREG);
	    outcomma();
	    outimmadr((offset_t) shift);
	    outnl();
	    if ((bool_t) uflag)
		outusr();
	    else
		outsr();
	    outaccum();
	    outncregname(SHIFTREG);
	}
	else
#endif
	    while (shift--)
	    {
		if ((bool_t) uflag)
		    usr1();
		else
		    sr1();
	    }
    }
}

/* extend an unsigned in DREG to a long */

PUBLIC void uitol P1(store_pt, reg)
{
    if (lowregisDreg())
	clr(reg);
    else
    {
	regexchange(DREG, reg);
	clr(DREG);
    }
}

PRIVATE char opregstr[] = "_opreg";

/*-----------------------------------------------------------------------------
	opregadr()
	outputs address of variable opreg where OPREG is saved
-----------------------------------------------------------------------------*/

PRIVATE void opregadr P0()
{
    outindleft();
    outccname(opregstr);
    outindright();
    bumplc2();
    if (i386_32)
	bumplc2();
}

/*-----------------------------------------------------------------------------
	restoreopreg()
	restores register OPREG from static location >opreg if it is was use
-----------------------------------------------------------------------------*/

PUBLIC void restoreopreg P0()
{
    if (reguse & OPREG)
    {
	outload();
	outregname(OPREG);
	outopsep();
	opregadr();
	outnl();
    }
}

/*-----------------------------------------------------------------------------
	saveopreg()
	saves register OPREG to static location >opreg if it is in use
	this makes the flop routines non-reentrant. It is too messy to
	push it because the flop routines leave results on the stack
-----------------------------------------------------------------------------*/

PUBLIC void saveopreg P0()
{
    if (reguse & OPREG)
    {
	bssseg();
	common(opregstr);
	outnhex(opregsize);
	cseg();
	outstore();
	opregadr();
	outncregname(OPREG);
    }
}
