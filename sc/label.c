/* label.c - label handling routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "const.h"
#include "types.h"
#include "condcode.h"
#include "gencode.h"
#include "label.h"
#include "output.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

#define outlbranch() outop3str( "b")
#define outsbranch() outop2str( "j")

#define MAXVISLAB 32

struct labdatstruct
{
    label_t labnum;		/* 0 if not active */
    offset_t lablc;		/* location counter for branch or label */
    char *labpatch;		/* buffer ptr for branch, (char*) 0 for label */
    ccode_t labcond;		/* condition code for branch */
};

PRIVATE _CONST char lcondnames[][2] =	/* names of long condition codes */
{
    { 'e', 'q', }, { 'n', 'e', }, { 'r', ' ', }, { 'r', 'n', },
    { 'l', 't', }, { 'g', 'e', }, { 'l', 'e', }, { 'g', 't', },
    { 'l', 'o', }, { 'h', 'i', }, { 'l', 'o', }, { 'h', 'i' },
};
PRIVATE _CONST char scondnames[][2] =	/* names of short condition codes */
{
    { 'e', ' ', }, { 'n', 'e', }, { ' ', ' ', }, { 'n', 0, },
    { 'l', ' ', }, { 'g', 'e', }, { 'l', 'e', }, { 'g', ' ', },
    { 'b', ' ', }, { 'a', 'e', }, { 'b', 'e', }, { 'a', ' ', }, 
};

/* The `& ~(label_t) 0' pacifies the ACK 3.1 warning on Minix 1.5.10 i86: overflow in unsigned constant expression */
/* !! Is it really OK to overflow lasthighlab to 0 here if sizeof(unsigned) == 2? */
PRIVATE label_t lasthighlab = (label_t) ((0xFFFFL + 1L) & ~(label_t) 0);  /* temp & temp init so labels fixed */
				/* lint */
PRIVATE label_t lastlab;	/* bss init to 0 */
PRIVATE offset_t lc;		/* bss init to 0 */

PRIVATE struct labdatstruct vislab[MAXVISLAB];	/* bss, all labnum's init 0 */
PRIVATE smalin_t nextvislab;	/* bss init to 0 */
PRIVATE struct symstruct *namedfirst;	/* bss init to (struct symstruct*) 0 */
PRIVATE struct symstruct *named2last;	/* bss init to (struct symstruct*) 0 */

FORWARD void addlabel P((ccode_pt cond, label_t label, char *patch));
FORWARD struct labdatstruct *findlabel P((label_t label));

/* add label to circular list */

PRIVATE void addlabel P3(ccode_pt, cond, label_t, label, char *, patch)
{
    REGISTER struct labdatstruct *labptr;

    labptr = &vislab[(int) nextvislab];
    labptr->labcond = cond;
    labptr->labnum = label;
    labptr->lablc = lc;
    labptr->labpatch = patch;
    if (++nextvislab == MAXVISLAB)
	nextvislab = 0;
}

/* bump location counter */

PUBLIC void bumplc P0()
{
    ++lc;
}

/* bump location counter by 2 */

PUBLIC void bumplc2 P0()
{
    lc += 2;
}

/* bump location counter by 3 */

PUBLIC void bumplc3 P0()
{
    lc += 3;
}

/* clear out labels in function */

PUBLIC void clearfunclabels P0()
{
    REGISTER struct symstruct *symptr;
    REGISTER struct symstruct *tmp;

    for (symptr = namedfirst; symptr != (struct symstruct*) 0;)
    {
	if (symptr->indcount == 2)
	    error("undefined label");
	symptr->indcount = 0;
	tmp = symptr;
	symptr = (struct symstruct *) symptr->type;
	tmp->type = (struct typestruct*) 0;
    }
    named2last = namedfirst = (struct symstruct*) 0;
}

/* clear out labels no longer in buffer */

PUBLIC void clearlabels P2(char *, patchbuf, char *, patchtop)
{
    REGISTER struct labdatstruct *labptr;
    struct labdatstruct *labtop;
    REGISTER char *labpatch;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB];
	 labptr < labtop; ++labptr)
	if ((labpatch = labptr->labpatch) >= patchbuf && labpatch < patchtop)
	    labptr->labnum = 0;
}

/* clear out labels in switch statement */

PUBLIC void clearswitchlabels P0()
{
    REGISTER struct symstruct *symptr;

    for (symptr = namedfirst; symptr != (struct symstruct*) 0;
	 symptr = (struct symstruct *) symptr->type)
	if (symptr->indcount == 3)
	{
	    equlab(symptr->offset.offlabel, lowsp);
	    symptr->indcount = 4;
	}
}

/* return location counter */

PUBLIC uoffset_t getlc P0()
{
    return (uoffset_t) lc;
}

/* define location of label and backpatch references to it */

PUBLIC void deflabel P1(label_t, label)
{
    _CONST char *cnameptr;
    struct labdatstruct *labmin;
    struct labdatstruct *labmax;
    struct labdatstruct *labmid;
    struct labdatstruct *labptrsave;
    offset_t nlonger;

    outnlabel(label);
    {
	REGISTER struct labdatstruct *labptr;
	REGISTER char *labpatch;

	labmin = &vislab[0];
	labmax = &vislab[MAXVISLAB];
	labptr = labmid = &vislab[(int) nextvislab];
	if (!watchlc)
	    do
	    {
		if (labptr == labmin)
		    labptr = &vislab[MAXVISLAB];
		--labptr;
		if (labptr->labnum == label)
		{
		    if ((labpatch = labptr->labpatch) != (char*) 0 &&
			isshortbranch(lc - labptr->lablc))
		    {
			/* patch "bcc(c) to j(c)(c)( ) */
			*labpatch = 'j';
			*(labpatch + 1) =
			    *(cnameptr = scondnames[(int) labptr->labcond]);
			*(labpatch + 2) = *(cnameptr + 1);
			*(labpatch + 3) = ' ';
			nlonger = jcclonger;
			if (labptr->labcond == RA)
			    nlonger = jmplonger;
			lc -= nlonger;
			labptrsave = labptr;
		    incr1:
			++labptr;
		    test1:
			if (labptr == labmid) goto done1;
			if (labptr == labmax)
			{
			    labptr = vislab;
			    goto test1;
			}
			labptr->lablc -= nlonger;
			goto incr1;
		    done1:
			labptr = labptrsave;
		    }
		}
	    }
	    while (labptr != labmid);
    }
    addlabel((ccode_pt) 0, label, (char *) 0);
}

PRIVATE struct labdatstruct *findlabel P1(label_t, label)
{
    REGISTER struct labdatstruct *labptr;
    struct labdatstruct *labtop;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB];
	 labptr < labtop; ++labptr)
	if (labptr->labnum == label)
	{
	    if (labptr->labpatch != 0)
		break;
	    return labptr;
	}
    return (struct labdatstruct *) 0;
}

/* reserve a new label, from top down to temp avoid renumbering low labels */

PUBLIC label_t gethighlabel P0()
{
    return --lasthighlab;
}

/* reserve a new label */

PUBLIC label_t getlabel P0()
{
    return ++lastlab;
}

/* jump to label */

PUBLIC void jump P1(label_t, label)
{
    lbranch(RA, label);
}

/* long branch on condition to label */

PUBLIC void lbranch P2(ccode_pt, cond, label_t, label)
{
    _CONST char *cnameptr;

    struct labdatstruct *labptr;
    char *oldoutptr;

    if ((ccode_t) cond == RN)
	return;
    if ((labptr = findlabel(label)) != (struct labdatstruct*) 0 &&
	isshortbranch(lc - labptr->lablc + 2))
    {
	sbranch(cond, label);
	return;
    }
    oldoutptr = outbufptr;
    if (cond == RA)
	outjumpstring();
    else
    {
	outlbranch();
	outbyte(*(cnameptr = lcondnames[cond]));
	outbyte(*(cnameptr + 1));
	if ((ccode_t) cond == LS || (ccode_t) cond == HS)
	    outbyte('s');	/* "blos" or "bhis" */
	else
	    outbyte(' ');
	outtab();
	bumplc2();
	if (i386_32)
	    bumplc();
    }
    outlabel(label);
    outnl();
    if (labptr == (struct labdatstruct*) 0 && oldoutptr < outbufptr)	/* no wrap-around */
	addlabel(cond, label, oldoutptr);
}

/* look up the name gsname in label space, install it if new */

PUBLIC struct symstruct *namedlabel P0()
{
    struct symstruct *symptr;

    gs2name[1] = (char) 0xFF;
    if ((symptr = findlorg(gs2name + 1)) == (struct symstruct*) 0)
    {
	symptr = addglb(gs2name + 1, vtype);
	symptr->flags = LABELLED;
    }
    if (symptr->indcount < 2)
    {
	symptr->indcount = 2;
	symptr->offset.offlabel = gethighlabel();
	if (namedfirst == (struct symstruct*) 0)
	    namedfirst = symptr;
	else
	    named2last->type = (struct typestruct *) symptr;
	named2last = symptr;
	symptr->type = (struct typestruct*) 0;
    }
    return symptr;
}

/* print label */

PUBLIC void outlabel P1(label_t, label)
{
    outbyte(LABELSTARTCHAR);
    outhexdigs((uoffset_t) label);
}

/* print label and newline */

PUBLIC void outnlabel P1(label_t, label)
{
    outlabel(label);
#ifdef LABELENDCHAR
    outnbyte(LABELENDCHAR);
#else
    outnl();
#endif
}

/* short branch on condition to label */

PUBLIC void sbranch P2(ccode_pt, cond, label_t, label)
{
    _CONST char *cnameptr;

    if ((ccode_t) cond != RN)
    {
	outsbranch();
	outbyte(*(cnameptr = scondnames[cond]));
	outbyte(*(cnameptr + 1));
	outtab();
	outlabel(label);
	outnl();
    }
}

/* reverse bump location counter */

PUBLIC void unbumplc P0()  /* !! Inline these functions as macros? Does it make the sc-generated code shorter? */
{
    --lc;
}
