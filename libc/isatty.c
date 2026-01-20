#include <lib.h>
#include <sgtty.h>
#include <minix/com.h>

int isatty(fd)
int fd;
{
  char dummy[sizeof(int) == 2 ? 32 : 36];  /* struct termios dummy; */  /* For compatibility with Minix 1.7.4--2.0.4--3.2.0. */

  _M.TTY_REQUEST = 0x7408;  /* TIOCGETP; */
  _M.TTY_LINE = fd;
  if (callx(FS, IOCTL) >= 0) goto found_tty;  /* Minix 1.5--1.7.2. */
  _M.TTY_REQUEST = (unsigned) (0x80245408L & ~(unsigned) 0);  /* TCGETS. */
  _M.TTY_LINE = fd;
  _M.ADDRESS = dummy;
  if (callx(FS, IOCTL) < 0) return 0;  /* Minix 1.7.4--2.0.4--3.2.0. */
 found_tty:
  return(1);
}
