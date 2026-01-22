/* os.h - source/target operating system dependencies for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* defaults */

#define CREATPERMS 0666		/* permissions for creat */
#define EOL 10			/* source newline */
#define EOLTO 10		/* target newline */
#define DIRCHAR '/'
#define DIRSTRING "/"
#define isabspath(fnameptr, tempcptr) \
	((*(tempcptr) = *(fnameptr)) == DIRCHAR)
