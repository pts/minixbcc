/* The <ar.h> header gives the layout of archives. */

#ifndef _MINIXAR_H
#define _MINIXAR_H

#define	MINIXARMAG	0177545
#define MINIXARNAMEMAX    14

struct minixar_hdr {
  char  ar_name[MINIXARNAMEMAX];
  char  ar_date[4];		/* long in byte order 2 3 1 0 */
  char  ar_uid;
  char  ar_gid;
  char  ar_mode[2];		/* short in byte order 0 1 */
  char  ar_size[4];		/* long in byte order 2 3 1 0 */
};

#endif /* _MINIXAR_H */
