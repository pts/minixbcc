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

/* <fcntl.h> */
int creat _LIBCP((const char *_path, int _mode));
int open _LIBCP((const char *_path, int _oflag, ...));

/* <stdlib.h> */
void exit _LIBCP((int _status));
#ifndef NOFP
  double atof _LIBCP((const char *_str));
#endif
void *malloc _LIBCP((unsigned _nbytes));
void *realloc _LIBCP((void *_ptr, unsigned _nbytes));
void free _LIBCP((void *_ptr));

/* <string.h> */
void *memcpy _LIBCP((void *_t, const void *_s, unsigned _length));
void *memset _LIBCP((void *_s, int _c, unsigned _nbytes));
char *strcat _LIBCP((char *_target, const char *_source));
int strcmp _LIBCP((const char *_s1, const char *_s2));
char *strcpy _LIBCP((char *_target, const char *_source));
unsigned strlen _LIBCP((const char *_s));

/* <unistd.h> */
int read  _LIBCP((int _fd, char *_buf, unsigned _nbytes));
int write _LIBCP((int _fd, char *_buf, unsigned _nbytes));
long lseek _LIBCP((int _fd, long _offset, int _whence));
int close _LIBCP((int _fd));
int isatty _LIBCP((int _fd));

#undef _LIBCP
