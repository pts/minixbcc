/* file.h - global variables involving files for assembler */

extern fd_t binfil;		/* binary output file */
EXTERN fd_t lstfil;		/* list output file (0 = standard) */
extern fd_t objfil;		/* object output file */
extern fd_t symfil;		/* symbol table output file */
EXTERN char *filnamptr;		/* file name pointer */
EXTERN fd_t infil0;		/* initial input file */
EXTERN fd_t infil;		/* current input file (stacked, 0 = memory) */
EXTERN unsigned char infiln;	/* outfd when file was opened */
EXTERN fd_t outfd;		/* current output file writec(...), writes(...) etc. write to */
EXTERN char *truefilename;	/* in case actual source name is a tmpname */
