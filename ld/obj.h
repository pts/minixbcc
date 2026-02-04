/* obj.h - constants for Introl object modules */

#define OBJ_H

#define OMAGIC 0x86A3

#if 0
#  define OMAGIC_MC6809 0x5331  /* Unused, because MC6809 support is not implemented. */
#  define OMAGIC_I386 0x86A3
#  define OMAGIC_I86 0x86A0  /* This is currently unused. The linker expects OMAGIC_I386 for i86 object files as well. */
#endif

/* In Linux libc 4.5.21, there is a `#define roundup(x, y)' macro in <sys/types.h>, so we rename this. */
#define ldroundup( num, boundary, type ) \
	(((num) + ((boundary) - 1)) & (type) ~((boundary) - 1))

#define MAX_OFFSET_SIZE 4
#define NSEG 16

/* flag values |SZ|LXXXX|N|E|I|R|A|SEGM|, X not used */

#define A_MASK 0x0010		/* absolute */
#define C_MASK 0x0020		/* common (internal only) */
#define E_MASK 0x0080		/* exported */
#define I_MASK 0x0040		/* imported */
#define N_MASK 0x0100		/* entry point */
#define R_MASK 0x0020		/* relative (in text only) */
#define SEGM_MASK 0x000F	/* segment (if not absolute) */
#define SA_MASK 0x2000		/* offset is storage allocation */
#define SZ_MASK 0xC000		/* size descriptor for value */
#define SZ_SHIFT 14
