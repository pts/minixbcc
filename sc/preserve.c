/* preserve.c - preserve opererands or registers in use for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "const.h"
#include "types.h"
#include "gencode.h"
#include "reg.h"
#include "type.h"

/* change stack ptr without changing condition codes */

PUBLIC void changesp P2(offset_t, newsp, bool_pt, absflag)
{
    if (newsp != sp || ((bool_t) absflag && switchnow != (struct switchstruct*) 0))
    {
#ifdef FRAMEPOINTER
	if (newsp != framep || (!(bool_t) absflag && switchnow != (struct switchstruct*) 0))
	{
	    outleasp();
	    if (!(bool_t) absflag && switchnow != (struct switchstruct*) 0)
		outswoffset(newsp);
	    else
		outoffset(newsp - framep);
	    outindframereg();
	    outnl();
	}
	else
	    regtransfer(FRAMEREG, STACKREG);
	sp = newsp;
	if (framep == 0)
	    bugerror("no frame pointer");
#else
	outleasp();
	outoffset(newsp - sp);
	outindstackreg();
	outnl();
#endif /* FRAMEPOINTER */
    }
}

/* load source to any while preserving target */

PUBLIC void loadpres P2(struct symstruct *, source, struct symstruct *, target)
{
    store_t regmark;

    if (target->storage & ALLDATREGS)
    {
	if (source->type->scalar & CHAR)
	{
	    push(target);
	    load(source, DREG);
	}
	else
	    load(source, getindexreg());
    }
    else
    {
	regmark = reguse;
	reguse |= target->storage;
	loadany(source);
	reguse = regmark;
    }
}

/* change stack ptr */

PUBLIC void modstk P1(offset_t, newsp)
{
    if (newsp != sp)
    {
#ifdef FRAMEPOINTER
	if (newsp != framep || framep == 0 || switchnow != (struct switchstruct*) 0)
	    addconst(newsp - sp, STACKREG);
	else
	    regtransfer(FRAMEREG, STACKREG);
#else
	addconst(newsp - sp, STACKREG);
#endif
	sp = newsp;
    }
}

/* preserve target without changing source */

PUBLIC void pres2 P2(struct symstruct *, source, struct symstruct *, target)
{
    if (target->storage & allregs)
    {
	if (target->storage & (allregs - allindregs) /* XXX */ ||
	    (target->indcount == 0 && target->type->scalar & (DLONG | RSCALAR)))
	    push(target);	/* XXX - perhaps not float */
	else if (((target->storage | reguse) & allindregs) == allindregs)
	{
	    loadpres(target, source);
	    push(target);
	}
	else
	    reguse |= target->storage;
    }
}

/* preserve source */

PUBLIC void preserve P1(struct symstruct *, source)
{
    if (source->storage & allregs)
    {
	if (source->storage & (allregs - allindregs) /* XXX */ ||
	    ((source->storage | reguse) & allindregs) == allindregs)
	    push(source);
	else
	    reguse |= source->storage;
    }
}

/* preserve lvalue target without changing source or target */

PUBLIC store_pt preslval P2(struct symstruct *, source, struct symstruct *, target)
{
    store_pt regpushed;

    if (target->indcount == 0)
	reguse &= ~target->storage;
    else
	reguse = (target->storage | reguse) & allindregs;
    if (!((source->type->scalar | target->type->scalar) & (DLONG | RSCALAR))
	|| reguse != allindregs)
	return 0;		/* XXX - perhaps not float */
    reguse = source->storage | target->storage;	/* free one other than s/t */
    pushreg(regpushed = getindexreg());
    reguse = ~(store_t) regpushed & allindregs;
    return regpushed;
}

PUBLIC void recovlist P1(store_pt, reglist)
{
    poplist(reglist);
    reguse |= (store_t) reglist;
}

PRIVATE smalin_t regoffset[] = {0, 0, 0, 1, 2, 3, 0, 0, 0, 4, 5};
 /* CONSTANT, BREG, ax = DREG, bx = INDREG0, si = INDREG1, di = INDREG2 */
 /* LOCAL, GLOBAL, STACKREG, cx = DATREG1, dx = DATREG2 */

PUBLIC void savereturn P2(store_pt, savelist, offset_t, saveoffset)
{
    store_t reg;
    smalin_t *regoffptr;
    offset_t spoffset;

    if (savelist == 0)
	return;
    for (reg = 1, regoffptr = regoffset; reg != 0; ++regoffptr, reg <<= 1)
	if (reg & savelist)
	{
	    outstore();
	    spoffset = saveoffset + *regoffptr * maxregsize;
#ifdef FRAMEPOINTER
	    if (switchnow != (struct switchstruct*) 0)
		outswoffset(spoffset);
	    else
		outoffset(spoffset - framep);
	    outindframereg();
#else
	    outoffset(spoffset - sp);
	    outindstackreg();
#endif
	    outncregname(reg);
	}
}
