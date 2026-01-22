/* function.c - function call protocol for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#include "const.h"
#include "types.h"
#include "align.h"
#include "gencode.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "table.h"
#include "type.h"

#define ADJUSTLONGRETURN
#define CANHANDLENOFRAME
#undef  CANHANDLENOFRAME
#define STUPIDFRAME

FORWARD void out_callstring P((void));

/* call a named (assembly interface) procedure, don't print newline after */

PUBLIC void call(name)
char *name;
{
    out_callstring();
    outstr(name);
}

PUBLIC void function(source)
struct symstruct *source;
{
    if (source->indcount == 0 && source->storage == GLOBAL &&
	!(source->flags & LABELLED) && *source->name.namep != 0)
    {
	out_callstring();
	outnccname(source->name.namep);
    }
    else
    {
#ifdef XENIX_AS
	if (source->indcount == 0)	/* fix call fixed address */
	    out_callstring();
	else
#endif
	    outcalladr();
	outadr(source);
    }
    source->type = source->type->nexttype;
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    if (source->type->scalar & DLONG)
    {
# ifdef ADJUSTLONGRETURN
	regtransfer(DXREG, LONGRETURNREGS & ~LONGREG2);
# endif
	source->storage = LONGRETURNREGS & ~LONGREG2;
    }
    else
#endif
    if (source->type->scalar & CHAR)
    {
#if RETURNREG != DREG
	transfer(source, DREG);
#endif
	source->storage = BREG;
    }
    else if (source->type->scalar & DOUBLE)
	source->storage = doublreturnregs & ~DREG;
#if 0
    else if (source->type->scalar & FLOAT)
	source->storage = floatreturnregs /* XXX? & ~DREG */;
#endif
    else
	source->storage = RETURNREG;
    source->offset.offi = source->indcount = 0;
    if (source->level == OFFKLUDGELEVEL)
	source->level = EXPRLEVEL;
    if (source->type->constructor & STRUCTU)
    {
	transfer(source, getindexreg());	/* so it can be indirected
					 * and/or preserved in blockmove() */
	source->indcount = 1;
	source->flags = TEMP;	/* kludge so blockpush can be avoided */
    }
}

PUBLIC void ldregargs()
{
    register struct symstruct *symptr;
    store_pt targreg;
    struct symstruct temptarg;

    for (symptr = &locsyms[0]; symptr < locptr && symptr->level == ARGLEVEL;
	 symptr = (struct symstruct *)
		  align(&symptr->name.namea[strlen(symptr->name.namea) + 1]))
    {
	if ((store_t) (targreg = symptr->storage) & allregs)
	{

	    /* load() is designed to work on expression symbols, so don't
	     * trust it on reg variables although it almost works.
	     */
	    temptarg = *symptr;
	    if (arg1inreg && symptr == &locsyms[0])
	    {
		temptarg.storage = ARGREG;
		temptarg.offset.offi = 0;
	    }
	    else
	    {
		temptarg.storage = LOCAL;
		temptarg.indcount = 1;
	    }
	    load(&temptarg, targreg);
	    symptr->offset.offi = 0;
	}
    }
    regarg = FALSE;
}

PUBLIC void loadretexpression()
{
    if (returntype->constructor & STRUCTU)
    {
	struct nodestruct *etmark;
	struct nodestruct *exp;
	struct symstruct *exprmark;
	struct symstruct *structarg;

	etmark = etptr;
	exprmark = exprptr;
	exp = expression();
	makeleaf(exp);
	structarg = constsym((value_t) 0);
	structarg->type = pointype(returntype);
	onstack(structarg);
	indirec(structarg);
	structarg->flags = 0;	/* assign() doesn't like TEMP even for indir */
	structarg->offset.offi = returnadrsize;
	assign(exp->left.symptr, structarg);
	etptr = etmark;
	exprptr = exprmark;
    }
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    else if (returntype->scalar & DLONG)
    {
	loadexpression(LONGRETURNREGS & ~LONGREG2, returntype);
# ifdef ADJUSTLONGRETURN
	regtransfer(LONGRETURNREGS & ~LONGREG2, DXREG);
# endif
    }
    else
#endif
    if (returntype->scalar & DOUBLE)
	loadexpression(doublreturnregs & ~DREG, returntype);
#if 0
    else if (returntype->scalar & FLOAT)
	loadexpression(floatreturnregs /* XXX? & ~DREG */, returntype);
#endif
    else
	loadexpression(RETURNREG, returntype);
}

PUBLIC void listo(target, lastargsp)
struct symstruct *target;
offset_t lastargsp;
{
    extend(target);
    push(target);
    if (lastargsp != 0 && (uoffset_t) sp != lastargsp - target->type->typesize)
    {
	loadany(target);
	modstk(lastargsp);
	push(target);
	if ((uoffset_t) sp != lastargsp - target->type->typesize)
	{
	    bugerror("botched push of arg");
#ifdef DEBUG
	    outstr("arg type is ");
	    dbtype(target->type);
	    outnl();
#endif
	}
    }
}

PUBLIC void listroot(target)
struct symstruct *target;
{
    extend(target);
    /* necessary regs are free since they were saved for function */
    if (target->type->scalar & DLONG)
	load(target, LONGARGREGS & ~LONGREG2);
    else
	load(target, ARGREG);
}

PRIVATE void out_callstring()
{
    outop3str(callstring);
    if (i386_32)
	bumplc2();
}

#ifdef FRAMEPOINTER

PUBLIC void popframe()
{
    poplist(frame1list);
}

#endif

/* reserve storage for locals if necessary */
/* also push 1st function arg and load register args if necessary */

PUBLIC void reslocals()
{
#ifdef FRAMEPOINTER
# ifndef STUPIDFRAME
    bool_t loadframe = FALSE;

# endif
#endif

    if (switchnow != (struct switchstruct*) 0)
    {
#ifdef FRAMEPOINTER
	if (framep == 0 && softsp != sp)
	    bugerror("local variables in switch statement messed up, sorry");
#else
	if (sp != softsp)
	    bugerror("local variables in switch statement don't work, sorry");
#endif
	if (lowsp > softsp)
	    lowsp = softsp;
	sp = softsp;
	return;
    }
#ifdef FRAMEPOINTER
    if (framep == 0)
    {
# ifdef STUPIDFRAME
	pushreg(FRAMEREG);
	regtransfer(STACKREG, FRAMEREG);
	framep = sp;
	pushlist(callee1mask);
# else /* not STUPIDFRAME */
#  ifdef CANHANDLENOFRAME
	if (stackarg || softsp != -frameregsize)	/* args or locals */
#  endif
	{
	    pushlist(frame1list);
	    loadframe = TRUE;
	}
# endif /* not STUPIDFRAME */
    }
#else
    if (sp == 0)
	pushlist(callee1mask);
#endif /* FRAMEPOINTER */
    if (arg1size)
    {
	switch ((fastin_t) arg1size)
	{
	case 8:
	    pushlist(doubleargregs);
	    break;
	case 4:
	    if (!i386_32)
	    {
		pushlist(LONGARGREGS);
		break;
	    }
	    /* Fallthrough. */
	case 2:
	    pushlist(ARGREG);
	}
	arg1size = 0;		/* show 1st arg allocated */
    }
#ifdef FRAMEPOINTER
# ifndef STUPIDFRAME /* else this moved above for compat with Xenix cc frame */
    if (loadframe || softsp != -frameregsize)
	modstk(softsp);
    /* else avoid modstk() because softsp holds space for frame pointer only) */
    /* but pointer has not been pushed (must keep softsp for later levels) */
    if (loadframe)
    {
	regtransfer(STACKREG, FRAMEREG);
	framep = sp;
    }
# else /* STUPIDFRAME */
    modstk(softsp);
# endif /* STUPIDFRAME */
#else /* no FRAMEPOINTER */
    modstk(softsp);
#endif /* FRAMEPOINTER */
    if (regarg)
	ldregargs();
}

/* clean up stack and return from a function */

PUBLIC void ret()
{
#ifdef FRAMEPOINTER
    offset_t newsp;

    if (framep != 0)
    {
	newsp = -(offset_t) func1saveregsize;
	if (switchnow != (struct switchstruct*) 0 || newsp - sp >= 0x80)
	    changesp(newsp, TRUE);
	else
	    modstk(newsp);
	popframe();
    }
    outreturn();
#else /* no FRAMEPOINTER */
    if (sp != 0)
    {
	modstk(-(offset_t) func1saveregsize);
	poplist(callee1mask);
    }
    outreturn();
#endif /* no FRAMEPOINTER */
}
