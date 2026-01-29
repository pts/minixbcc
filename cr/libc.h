/* This is a short header file containing all libc functionality used by sc.
 * 
 * The Minix 1.5.10 i86 ACK 3.1 C compiler fails with ``fatal error -- out
 * of memory'' for some files (input.c, state.c and table.c) if there are
 * too many macros and/or declarations in the libc headers. Our workaround
 * is using `#include "libc.h"' instead, which declares the bare minimum.
 */

#if __STDC__
#  define _LIBCP(x) x
#else
#  define _LIBCP(x) ()
#endif

#define O_RDONLY 0

typedef long off_t;    /* offsets within a file */

/* <fcntl.h> */
int creat _LIBCP((const char *_path, int _mode));
int open _LIBCP((const char *_path, int _oflag, ...));

/* <stdlib.h> */
void exit _LIBCP((int _status));

/* <string.h> */
unsigned strlen _LIBCP((const char *_s));

/* <unistd.h> */
int read  _LIBCP((int _fd, char *_buf, unsigned _nbytes));
int write _LIBCP((int _fd, const char *_buf, unsigned _nbytes));
off_t lseek _LIBCP((int _fd, off_t _offset, int _whence));
int close _LIBCP((int _fd));

#undef _LIBCP
