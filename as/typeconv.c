/* typeconv.c - convert between char arrays and unsigneds */

/*
	c2u2(): 2 byte array to 2 byte unsigned
	c4u4(): 4 byte array to 4 byte unsigned
	cnu2(): n byte array to 2 byte unsigned
	cnu4(): n byte array to 4 byte unsigned
	u2c2(): 2 byte unsigned to 2 byte array
	u2cn(): 2 byte unsigned to n byte array
	u4c4(): 4 byte unsigned to 4 byte array
	u4cn(): 4 byte unsigned to n byte array
	typeconv_init: (re)initialise for given byte order.
		Default is no swapping, but the initialisation should be done
		anyway to provide some validity checks (returns FALSE if error).
	
	Not provided:
		c2u4(), c4u2(), u2c4(), u4c2().
	Each of these is best done by truncating or extending a return value
	or argument to the appropiate fixed-count function.
	c4u2() has too many cases to do in-line conveniently, and the others
	are hardly more efficient when done in-line.
	
	4 byte orderings for both char arrays and unsigneds are supported:
	0123 - little-endian
	3210 - big-endian
	2301 - little-endian with long words big-endian (pdp11)
	1032 - big-endian with long words little_endian (who knows?)

	The unsigned's byte order is that of the machine on which these
	routines are running.
	It is determined at run time initialisation since the compiler/
	preprocessor is too dumb to tell us at compile time.
*/

#include "const.h"
#include "type.h"
#include "globvar.h"

FORWARD void u2c2_00 P((char *buf, u2_pt offset));
FORWARD void u4c4_00 P((char *buf, u4_t offset));
FORWARD void u2c2_ss P((char *buf, u2_pt offset));
FORWARD void u4c4_ss P((char *buf, u4_t offset));
FORWARD void u4c4_s0 P((char *buf, u4_t offset));
FORWARD void u4c4_0s P((char *buf, u4_t offset));

PRIVATE u2_pt c2u2_00 P((char *buf));
PRIVATE u4_pt c4u4_00 P((char *buf));

PRIVATE void (*pu2c2) P((char *buf, u2_pt offset)) = u2c2_00;
PRIVATE void (*pu4c4) P((char *buf, u4_t offset)) = u4c4_00;

PUBLIC void u2c2(buf, offset)
register char *buf;
u2_pt offset;
{
    (*pu2c2) (buf, offset);
}

PUBLIC void u4c4(buf, offset)
register char *buf;
u4_t offset;
{
    (*pu4c4) (buf, offset);
}

PUBLIC void u4cn(buf, offset, count)
register char *buf;
u4_t offset;
unsigned count;
{
    switch (count)
    {
    case 1:
	buf[0] = (char) offset;
	return;
    case 2:
	(*pu2c2) (buf, (u2_pt) (u2_t) offset);
	return;
    case 4:
	(*pu4c4) (buf, offset);
	return;
    }
}

/* === char arrays to unsigneds === */

/* no bytes swapped, longwinded to avoid alignment problems */

PRIVATE u2_pt c2u2_00(buf)
register char *buf;
{
    u2_t offset;

    ((char *) &offset)[0] = buf[0];
    ((char *) &offset)[1] = buf[1];
    return offset;
}

PRIVATE u4_pt c4u4_00(buf)
register char *buf;
{
    u4_t offset;

    ((char *) &offset)[0] = buf[0];
    ((char *) &offset)[1] = buf[1];
    ((char *) &offset)[2] = buf[2];
    ((char *) &offset)[3] = buf[3];
    return offset;
}


/* === unsigneds to char arrays === */

/* no bytes swapped, longwinded to avoid alignment problems */

PRIVATE void u2c2_00(buf, offset)
register char *buf;
u2_pt offset;
{

    buf[0] = ((char *) &offset)[0];
    buf[1] = ((char *) &offset)[1];
}

PRIVATE void u4c4_00(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[0];
    buf[1] = ((char *) &offset)[1];
    buf[2] = ((char *) &offset)[2];
    buf[3] = ((char *) &offset)[3];
}

/* straight swapping for little-endian to big-endian and vice versa */

PRIVATE void u2c2_ss(buf, offset)
register char *buf;
u2_pt offset;
{
    u2_t offset2;

    offset2 = offset;
    buf[0] = ((char *) &offset2)[1];
    buf[1] = ((char *) &offset2)[0];
}

PRIVATE void u4c4_ss(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[3];
    buf[1] = ((char *) &offset)[2];
    buf[2] = ((char *) &offset)[1];
    buf[3] = ((char *) &offset)[0];
}

/* wierd swapping for different-endian u2's, same-endian u4's */

PRIVATE void u4c4_s0(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[1];
    buf[1] = ((char *) &offset)[0];
    buf[2] = ((char *) &offset)[3];
    buf[3] = ((char *) &offset)[2];
}

/* very wierd swapping for same-endian u2's, different-endian u4's */

PRIVATE void u4c4_0s(buf, offset)
register char *buf;
u4_t offset;
{
    buf[0] = ((char *) &offset)[2];
    buf[1] = ((char *) &offset)[3];
    buf[2] = ((char *) &offset)[0];
    buf[3] = ((char *) &offset)[1];
}

/* initialise type conversion, return FALSE if it cannot be handled */

PUBLIC bool_pt typeconv_init()
{
    u2_pt conv2;
    u4_pt conv4;
    char *conv2ptr;
    char *conv4ptr;

    if (sizeof(u2_t) != 2 || sizeof(u4_t) != 4)
	/* dumb preprocessor's don't accept sizeof in #if expressions */
	return FALSE;

    conv2ptr = conv4ptr = "\4\3\2\1";
    conv2 = c2u2_00(conv2ptr);
    conv4 = c4u4_00(conv4ptr);
    if (conv2 == 0x0304)
    {
	pu2c2 = u2c2_00;
	pu4c4 = u4c4_00;
	if (conv4 == (u4_pt) 0x03040102L)
	{
	    pu4c4 = u4c4_0s;
	}
	else if (conv4 != (u4_t) 0x01020304L)
	    return FALSE;
    }
    else if (conv2 == 0x0403)
    {
	pu2c2 = u2c2_ss;
	pu4c4 = u4c4_ss;
	if (conv4 == (u4_pt) 0x02010403L)
	{
	    pu4c4 = u4c4_s0;
	}
	else if (conv4 != (u4_pt) 0x04030201L)
	    return FALSE;
    }
    else
	return FALSE;
    return TRUE;
}
