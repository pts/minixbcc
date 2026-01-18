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

/* _oflag for open. */
#define O_RDONLY 0

/* Values used for whence in lseek(fd, offset, whence). */
#define SEEK_SET 0  /* offset is absolute  */
#define SEEK_CUR 1  /* offset is relative to current position */
#define SEEK_END 2  /* offset is relative to end of file */

#define STDIN_FILENO   0  /* file descriptor for stdin */
#define STDOUT_FILENO  1  /* file descriptor for stdout */
#define STDERR_FILENO  2  /* file descriptor for stderr */

#ifdef LIBCHMINIX  /* Minix-specific <time.h> and <sys/stat.h>. */
  typedef long time_t;
  typedef unsigned short  dev_t;    /* holds (major|minor) device pair */
  typedef unsigned char   gid_t;    /* group id */
  typedef unsigned short  ino_t;    /* i-node number */
  typedef unsigned short  mode_t;   /* mode number within an i-node */
  typedef unsigned char   nlink_t;  /* number-of-links field within an i-node */
  typedef long            off_t;    /* offsets within a file */
  typedef unsigned short  uid_t;    /* user id */
  struct stat {
    dev_t st_dev;		/* major/minor device number */
    ino_t st_ino;		/* i-node number */
    mode_t st_mode;		/* file mode, protection bits, etc. */
    short int st_nlink;		/* # links; TEMPORARY HACK: should be nlink_t*/
    uid_t st_uid;		/* uid of the file's owner */
    short int st_gid;		/* gid; TEMPORARY HACK: should be gid_t */
    dev_t st_rdev;
    off_t st_size;		/* file size */
    time_t st_atime;		/* time of last access */
    time_t st_mtime;		/* time of last data modification */
    time_t st_ctime;		/* time of last file status change */
  };
#endif

/* <sys/stat.h> */
/* mode_t */ int umask _LIBCP((int _cmask));
int chmod _LIBCP((const char *_path, int _mode));

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

#undef _LIBCP
