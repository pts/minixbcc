/*
 * cr.c: tool to create a Minix archive (.a) file
 * by pts@fazekas.hu at Fri Jan 16 22:10:08 CET 2026
 */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <sys/types.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifndef _AUTOC_H
#  include "autoc.h"  /* For INT32T. */
#endif

#if __STDC__
#  define CONST const
#  define P(x) x
#else
#  define CONST
#  define P(x) ()
#endif

#ifdef OPEN00
  extern int open00 P((_CONST char *pathname));  /* flags and mode are both 0. */
#else
#  define open00(pathname) open(pathname, 0  /* O_RDONLY */)
#endif

#if __STDC__
#  define cast_lseek_offset(offset) (offset)
#  define const_lseek_offset(v) ((INT32T) (v))  /* Without the cast, it would be broken with `cc -m' on Minix 2.0.4 i86, which runs `irrel -m', which strips arguments from function prototypes, thus it would push only 16 bits. */
#else
#  define cast_lseek_offset(offset) ((off_t) (offset))  /* We must pass the offset of the correct size, because the K&R C compiler doesn't know the argument type of lseek(...). Compile with -Doff_t=long if needed. */
#  define const_lseek_offset(v) ((off_t) (v))
#endif

/* Parses an unsigned decimal, hexadecimal or octal number. It doesn't check
 * for overflow. It allows leading ' ' and '\t'. It doesn't allow trailing
 * garbage. Returns nonzero on success. It produces a twos complement number
 * if the input has a minus sign.
 */
static int parse_u_arg P((CONST char *s, unsigned INT32T *output));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static int parse_u_arg(s, output)
CONST char *s;
unsigned INT32T *output;
{
  register char c;
  unsigned base;
  int sign;

  sign = 0;
  *output = 0;
  for (; (c = *s) == ' ' || c == '\t'; ++s) {}
  if (*s == '-') { ++s; sign = 1; }
  if (*s == '0' && ((c = s[1]) == 'x' || c == 'X')) {
    s += 2;
    base = 16;
  } else if (*s == '0') {
    ++s;
    base = 8;
  } else {
    base = 10;
  }
  if ((c = *s) == '\0') return 0;  /* Number missing. */
  while ((c = *s++) != '\0') {
    if (c >= '0' && c <= '9' && base == 10) {  /* This doesn't work in EBCDIC, it's probably ASCII-only. */
      c -= '0';
      *output *= 10;
    } else if (c >= '0' && c <= '7' && base == 8) {
      c -= '0';
      *output <<= 3;
    } else if (c >= '0' && c <= '9' && base == 10) {  /* This doesn't work in EBCDIC, it's probably ASCII-only. */
      c -= '0';
      *output <<= 4;
    } else if (c >= 'a' && c <= 'f' && base == 16) {
      c -= 'a' - 10;
      *output <<= 4;
    } else if (c >= 'A' && c <= 'F' && base == 16) {
      c -= 'A' - 10;
      *output <<= 4;
    } else {
      return 0;  /* Bad digit in c. */
    }
    *output += c;
  }
  if (sign) *output = ~*output + 1;  /* Twos complement. */
  return 1;
}

static void write_err P((CONST char *msg));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void write_err(msg)
CONST char *msg;
{
  if (*msg != '\0') (void)!write(2, msg, strlen(msg));
}

static void fatal2 P((CONST char *msg1, CONST char *msg2));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void fatal2(msg1, msg2)
CONST char *msg1;
CONST char *msg2;
{
  write_err("fatal: ");
  write_err(msg1);
  if (msg2) {
    write_err(": ");
    write_err(msg2);
  }
  write_err("\n");
  exit(2);
}

static char *dump16le P((unsigned v, char *p));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static char *dump16le(v, p)  /* Appends 16 bits in little-endian byte order. */
unsigned v;
char *p;
{
  *p++ = v;
  *p++ = v >> 8;
  return p;
}

static char *dump32pdp11 P((unsigned INT32T v, char *p));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static char *dump32pdp11(v, p)  /* Appends 16 bits in PDP-11 middle-endian byte order. */
unsigned INT32T v;
char *p;
{
  /* K&R C is much more unsafe than ANSI C (C89): The caller must remember to add these `(unsigned)' casts. */
  return dump16le((unsigned) v, dump16le((unsigned) (v >> 16), p));
}

static char iobuf[8192];

int main P((int argc, char **argv));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
int main(argc, argv)
int argc;
char **argv;
{
  CONST char *arg;
  CONST char *p;
  char c;
  CONST char *afn;
  unsigned INT32T mtime;
  unsigned INT32T uid;
  unsigned INT32T gid;
  unsigned INT32T mode;
  char hdrbuf[28];
  char *hdrp;
  char *hdrp0;
  char *hdrp14;
  int afd;
  int mfd;
  int got;
  INT32T size;

  afn = (CONST char*)0;
  mtime = 1;
  uid = gid = 2;
  mode = 0444;
  (void)argc;
  for (++argv; (arg = *argv) != (CONST char*)0; ) {
    if (arg[0] != '-' || arg[1] == '\0') {
      break;
    } else if (arg[2] != '\0') {
      fatal2("flag too long", arg);
    } else if (++argv, (c = arg[1]) == '-') {
      break;
    } else if ((arg = *argv++) == (CONST char*)0) {
      fatal2("missing flag argument", argv[-1]);
    } else if (c == 'o') {
      afn = arg;
    } else if (c == 't') {
      /* This is poor man's strptime(3). It only supports Unix timestamps. */
      if (arg[0] != '@' || !parse_u_arg(arg + 1, &mtime)) fatal2("bad mtime (-t)", arg);
    } else if (c == 'u') {  /* User ID. */
      if (!parse_u_arg(arg, &uid)) fatal2("bad UID (-u)", arg);
    } else if (c == 'g') {  /* Group ID. */
      if (!parse_u_arg(arg, &gid)) fatal2("bad GID (-g)", arg);
    } else if (c == 'm') {
      if (!parse_u_arg(arg, &mode)) fatal2("bad mode (-m)", arg);
    } else {
      fatal2("unknown command-line flag", arg);
    }
  }
  if (!afn) fatal2("missing archive filename (-o)", (CONST char*)0);
  if ((afd = creat(afn, 0666)) < 0) fatal2("error creating archive file", afn);
  hdrbuf[0] = '\145';  /* '\x65'. Low  byte of MINIXARMAG. */
  hdrbuf[1] = '\377';  /* '\xff'. High byte of MINIXARMAG. */
  hdrp0 = hdrbuf + 2;

  mfd = 0;   /* Pacify useless GCC 4.5 warning -Wmaybe-uninitialized. */
  size = 0;  /* Pacify useless GCC 4.5 warning -Wmaybe-uninitialized. */
  while ((arg = *argv++) != (char*)0) {
    if ((mfd = open00(arg)) < 0) fatal2("error opening member file", arg);
    /* K&R C is much more unsafe than ANSI C (C89): The caller must remember to add cast_lseek_offset(...). */
    if ((size = lseek(mfd, const_lseek_offset(0), 2)) < 0) fatal2("error getting member file size", arg);
    if ((INT32T) size < 0 || (size >> 15 >> 15 >> 1) != 0) fatal2("member file too large", arg);
    /* K&R C is much more unsafe than ANSI C (C89): The caller must remember to add cast_lseek_offset(...). */
    if (lseek(mfd, const_lseek_offset(0), 0) != 0) fatal2("error rewinding member file", arg);
    for (p = arg + strlen(arg); p != arg && p[-1] != '/'; --p) {}
    if (*p == '\0') fatal2("empty member basename", arg);
    for (hdrp14 = (hdrp = hdrp0) + 14; *p != '\0' && hdrp != hdrp14; *hdrp++ = *p++) {}
    for (; hdrp != hdrp14; *hdrp++ = '\0') {}  /* Pad basename to 14 bytes with NULs. */
    hdrp = dump32pdp11(mtime, hdrp);
    *hdrp++ = uid;
    *hdrp++ = gid;
    /* K&R C is much more unsafe than ANSI C (C89): The caller must remember to add this `(unsigned)' cast. */
    hdrp = dump16le((unsigned) mode, hdrp);
    hdrp = dump32pdp11(size, hdrp);
    if (write(afd, hdrbuf, hdrp - hdrbuf) != hdrp - hdrbuf) { fatal_hdrwrite:
      fatal2("error writing header", afn);
    }
    hdrp0 = hdrbuf;
    if ((unsigned) size & 1) *hdrp0++ = '\0';  /* Append a NUL byte after the file. */
    for (got = 0; size > 0 && (got = read(mfd, iobuf, sizeof(iobuf))) > 0; size -= got) {
      if (write(afd, iobuf, got) != got) fatal2("error writing member data", afn);
    }
    if (got < 0) fatal2("error reading member file", arg);
    if (size != 0) fatal2("member file became longer in the meantime", arg);
    close(mfd);
  }
  /* If needed, write 2-byte header of empty archive, or write the padding NUL byte. */
  if ((got = hdrp0 - hdrbuf) != 0 && write(afd, hdrbuf, got) != got) goto fatal_hdrwrite;

  /* close(afd): */  /* Not needed, the kernel will close it when the process exits. */

  return 0;  /* EXIT_SUCCESS. */
}
