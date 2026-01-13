/* floatop.c - software operations on floats and doubles for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* Incomplete list of floating-point functions the libc (C library) should provide: Faddd Fcomp Fcompd Fdivd Fmuld Fnegd Fpredec Fpreinc Fpostdec Fpostinc Fpulld Fpushd Fpushf Fpushi Fpushl Fpushul Fsub Fsubd Ftstd dtof dtod */

#include "const.h"
#include "types.h"

#ifdef NOFP  /* The error messages below indicate lack of floating point support in the C compiler. */

void no_atof()                { fatalerror("no floating-point (literal)"); }
void no_1double_to_float()    { fatalerror("no floating-point (double-to-float)"); }
void no_3double_op()          { fatalerror("no floating-point (constant-folding)"); }
void no_fp_cast()             { fatalerror("no floating-point (cast)"); }
void no_fp_move()             { fatalerror("no floating-point (move)"); }
void no_fp_op()               { fatalerror("no floating-point (operation)"); }

#else  /* NOFP */

#include "gencode.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "type.h"

/*-----------------------------------------------------------------------------
	f_indirect(target leaf)
	make the float or double target indirect if it is not already
	return nonzero iff the result is a temp double on the base of the stack
-----------------------------------------------------------------------------*/

PUBLIC bool_pt f_indirect(target)
struct symstruct *target;
{
    if (target->indcount == 0)
    {
	if (target->storage == CONSTANT)
	{
	    /* XXX - more for non-386 */
	    if (target->type->scalar & FLOAT)
	    {
#ifdef NOFP
		no_1double_to_float();
#else
		float val;

		val = *target->offset.offd;
		push(constsym(((value_t *) &val)[0]));
#endif
	    }
	    else
	    {
		push(constsym(((value_t *) target->offset.offd)[1]));
		push(constsym(((value_t *) target->offset.offd)[0]));
	    }
	}
	else if (target->type->scalar & FLOAT)
	    pushlist(target->storage);	/* XXX - floatregs */
	else
	    pushlist(doubleregs);
	onstack(target);
    }
    return target->flags == TEMP && target->type->scalar & DOUBLE
	   && target->offset.offi == sp;
}

/*-----------------------------------------------------------------------------
	float1op(operation code, source leaf)
	handles all flop unary operations except inc/dec
	result is double on stack (or in condition codes for EQOP)
-----------------------------------------------------------------------------*/

PUBLIC void float1op(op, source)
op_pt op;
struct symstruct *source;
{
    saveopreg();
    pointat(source);
    if ((op_t) op == NEGOP)
	call("Fneg");
    else			/* op == EQOP */
	call("Ftst");
    outntypechar(source->type);
    if ((op_t) op != EQOP)
	justpushed(source);
    restoreopreg();
}

/*-----------------------------------------------------------------------------
	floatop(operation code, source leaf, target leaf)
	handles all flop binary operations
	result is double on stack (or in condition codes for EQOP)
----------------------------------------------------------------------------*/

PUBLIC void floatop(op, source, target)
op_pt op;
struct symstruct *source;
struct symstruct *target;
{
    store_t regmark;
    bool_t sflag;

    regmark = reguse;
    saveopreg();
    (void) f_indirect(source);
    if (!(reguse & OPREG) && (source->storage == OPREG))
    {
	reguse |= source->storage;
	saveopreg();
    }
    fpush(target);
    sflag = TRUE;
    if (source->flags != TEMP || source->offset.offi != sp + dtypesize)
    {
	sflag = FALSE;
	if (source->storage == OPREG)
	    restoreopreg();
	pointat(source);
    }
    switch ((op_t) op)
    {
    case ADDOP:
	call("Fadd");
	break;
    case DIVOP:
	call("Fdiv");
	break;
    case EQOP:
	call("Fcomp");
	sp += dtypesize;	/* target is popped */
	break;			/* target symbol now invalid but is not used */
    case MULOP:
	call("Fmul");
	break;
    case SUBOP:
	call("Fsub");
	break;
    }
    if (sflag)
    {
	outnl();
	sp += dtypesize;	/* source is popped */
    }
    else
	outntypechar(source->type);
    onstack(target);
    reguse = regmark;		/* early so opreg is not reloaded if source */
    restoreopreg();
}

/*-----------------------------------------------------------------------------
	fpush(source leaf of scalar type)
	converts source to double and pushes it to stack
	OPREG must be free
-----------------------------------------------------------------------------*/

PUBLIC void fpush(source)
struct symstruct *source;
{
    scalar_t scalar;

    if ((scalar = source->type->scalar) & RSCALAR)
    {
	if (f_indirect(source))
	    return;
	pointat(source);
    }
    else if (scalar & DLONG)
	load(source, OPREG);
    else
	load(source, DREG);
    call("Fpush");
    if (scalar & UNSIGNED)
	outbyte('u');
    outntypechar(source->type);
    justpushed(source);
}

/*-----------------------------------------------------------------------------
	justpushed(target leaf)
	records that target has just been pushed to a double on the stack
-----------------------------------------------------------------------------*/

PUBLIC void justpushed(target)
struct symstruct *target;
{
    sp -= dtypesize;
    onstack(target);
    target->type = dtype;
}

#endif  /* #else NOFP */
