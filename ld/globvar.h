/* globvar.h - global variables for linker */

#ifdef EXTERN
EXTERN char hexdigit[];
#else
#define EXTERN
PUBLIC char hexdigit[] = "0123456789abcdef";
#endif
EXTERN unsigned errcount;		/* count of errors */
EXTERN struct entrylist *entryfirst;	/* first on list of entry symbols */
EXTERN struct modstruct *modfirst;	/* data for 1st module */
EXTERN struct redlist *redfirst;	/* first on list of redefined symbols */

EXTERN char *heapstart;		/* start of heap (catchall table) heapstart <= heapptr <= heapend */
EXTERN char *heapend;		/* end of free space in heap */
EXTERN char *heapptr;		/* next free space in heap */
