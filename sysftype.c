/*
 * sysftype: detect the operating system and the architecture based on the headers in a native executable file
 * by pts@fazekas.hu at Mon Jan 19 22:34:05 CET 2026
 */

#include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#include <fcntl.h>
/* #include <stdlib.h> */  /* We need it for exit(...). Not including it to prevent BCC sc v0 from displaying an error: function returning structure is illegal */
#include <string.h>
#include <unistd.h>

#if __STDC__
#  define CONST const
#  define P(x) x
#else
#  define CONST
#  define P(x) ()
#endif

#ifndef O_RDONLY
#  define O_RDONLY 0
#endif

#ifndef STDOUT_FILENO
#  define STDOUT_FILENO 1
#endif

static void write_err P((CONST char *msg));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void write_err(msg)
CONST char *msg;
{
  if (*msg != '\0') (void)!write(2, msg, strlen(msg));
}

static void fatal_msg P((CONST char *msg1, CONST char *msg2));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
static void fatal_msg(msg1, msg2)
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
}

static char header[32];

int main P((int argc, char **argv));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
int main(argc, argv)
int argc;
char **argv;
{
  int fd;
  register int got;
  char *p;
  char result[2];
  CONST char *filename;

  (void)argc;
  if (!argv[0] || !(filename = argv[1])) {
    fatal_msg("missing command-line argument: the filename of an executable", (CONST char*)0);
   fatal:
    return 2;
  }
  if (argv[2]) { fatal_msg("too many command-line arguments", (CONST char*)0); goto fatal; }
  if ((fd = open(filename, O_RDONLY)) < 0) { fatal_msg("error opening executable file", filename); goto fatal; }
  for (p = header; p < header + sizeof(header); ) {
    if ((got = read(fd, p, header + sizeof(header) - p)) < 0) { fatal_msg("error reading header", filename); goto fatal; }
    if (got == 0) {
      for (p = header; p < header + sizeof(header); *p++ = '\0') {}
      break;
    }
    p += got;
  }
  /* close(afd): */  /* Not needed, the kernel will close it when the process exits. */
  result[0] = '?';  /* Unknown (so far). */
  if (header[0] == 1 && header[1] == 3 &&  /* a.out a_magic. Minix or ELKS executable. */
      ((got = header[2] & (0xff - 3)) == 0x20 || got == 0x10 || got == 0) &&  /* a_flags. 0x10 == A_EXEC (I&D in same address space ; 0x20 == A_SEP (separate I&D) */
      ((got = header[2] & 3) == 0 || got == 3) &&   /* A_UZP (== 1, unmapped zero page) | A_PAL (== 2, page-aligned executable) */
      (header[3] == 4 || header[3] == 0x10) &&  /* a_cpu. Architecture (CPU): 4 for i86 (8086+ in 16-bit mode: real mode on 16-bit protected mode), 0x10 for i386 (32-bit protected mode). */
      header[4] == 0x20 &&  /* a_hdrlen. a.out object files may have this 0x30. */
      header[5] == 0 &&  /* a_unused. */
      header[6] == 0 && header[7] == 0) {  /* a_version. */
    if (header[3] == 4) {
      result[0] = ((header[2] & 3) == 0 && (header[2] & (0xff - 3)) != 0) ? '0' : '?';  /* '0' is Minix i86. '?' is unknown. */
      /* TODO(pts): Distinguish this from ELKS (i86) executable, and detect '4' (ELKS). Only some ELKS executables have a_version == 1. */
    } else {
      /* The ACK ANSI C compiler 1.202 in Minix 1.7.0 i386 sets a_flags to 0 (not even A_EXEC or A_SEP. We allow this here, but not for i86. */
      result[0] = ((header[2] & 3) == 3) ? '5' : '3';  /* '5' is Minix-386vm or Minix-vmx. '3' is Minix i386. */
    }
  }
  result[1] = '\n';
  (void)!write(STDOUT_FILENO, result, 2);
  return 0;
}
