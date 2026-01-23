/* sizes.h - target scalar type sizes for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  the compiler is not very portable in this area
  it only directly supports Intel i86 (8086) and i386
  it assumes
      sizeof(source INT32T) >= sizeof(target long)
      usual register size = int
      long = 2 int sizes
*/

#define CHBITSTO	8	/* bits in a character */
#define CHMASKTO	0xFF	/* mask to reduce SOURCE int to TARGET uchar */
#define INT16BITSTO	16	/* not accessed in non-16 bit case */
#define INT32BITSTO	32	/* not accessed in non-32 bit case */
#define MAXINTBITSTO	32	/* max bits in an integer (var processors) */
#define MAXSCHTO	127	/* maximum signed character */
#define MAXUCHTO	255	/* maximum unsigned character */
#define MINSCHTO	(-128)	/* minimum signed character */

#define isbyteoffset(n)	((uoffset_t) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)
#define ischarconst(n)	((uvalue_t) (n) <= MAXUCHTO)
#define isnegbyteoffset(n) ((uvalue_t) (n) + MAXSCHTO <= MAXSCHTO - MINSCHTO)
#define isshortbranch(n)   ((uoffset_t) (n) - MINSCHTO <= MAXSCHTO - MINSCHTO)

extern uvalue_t intmaskto;	/* mask for ints */
extern uvalue_t maxintto;	/* maximum int */
extern uvalue_t maxlongto;	/* maximum long */
extern uvalue_t maxoffsetto;	/* maximum offset */
extern uvalue_t maxshortto;	/* maximum short */
extern uvalue_t maxuintto;	/* maximum unsigned */
extern uvalue_t maxushortto;	/* maximum unsigned short */
extern uvalue_t shortmaskto;	/* mask for shorts */
