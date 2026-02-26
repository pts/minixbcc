/* typeconv.c - convert between little-endian char arrays and unsigneds */

/*
	u2c2(): 2 byte unsigned to 2 byte array
	u4c4(): 4 byte unsigned to 4 byte array
	u4cn(): 4 byte unsigned to n byte array
	c2u2(): 2 byte array to 2 byte unsigned
	c4u4(): 4 byte array to 4 byte unsigned
	cnu4(): n byte array to 4 byte unsigned
*/

#include "const.h"
#include "type.h"

PUBLIC void u2c2 P2(REGISTER char *, buf, u2_pt, offset)
{
    buf[1] = (char) ((unsigned) offset >> 8);
    buf[0] = (char) offset;
}

PUBLIC void u4c4 P2(REGISTER char *, buf, u4_pt, offset)
{
    u4cn(buf, offset, 4);
}

PUBLIC void u4cn P3(REGISTER char *, buf, u4_pt, offset, unsigned, count)
{
    switch (count)
    {
    case 4:
	count = (unsigned) (offset >> 16);
	buf[3] = (char) (count >> 8);
	buf[2] = (char) count;
	/* Fallthrough. */
    case 2:
	buf[1] = (char) ((unsigned) offset >> 8);
	/* Fallthrough. */
    case 1:
	buf[0] = (char) offset;
    }
}

PUBLIC u2_pt c2u2 P1(REGISTER _CONST char *, buf)
{
    return (u2_pt) ((unsigned char *) buf)[0] | ((u2_pt) ((unsigned char *) buf)[1]) << 8;
}

PUBLIC u4_pt c4u4 P1(REGISTER _CONST char *, buf)
{
    return c2u2(buf) | ((u4_pt) c2u2(buf + 2)) << 16;
}

PUBLIC u4_pt cnu4 P2(_CONST char *, buf, unsigned, count)
{
    switch (count)
    {
    case 1:
	return ((unsigned char *)buf)[0];
    case 2:
	return c2u2(buf);
    case 4:
	return c2u2(buf) | ((u4_pt) c2u2(buf + 2)) << 16;
    default:
	return 0;
    }
}
