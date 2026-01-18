/* globvar.h - global variables for linker */

EXTERN char hexdigit[];
EXTERN unsigned errcount;		/* count of errors */
EXTERN struct entrylist *entryfirst;	/* first on list of entry symbols */
EXTERN struct modstruct *modfirst;	/* data for 1st module */
EXTERN struct redlist *redfirst;	/* first on list of redefined symbols */

EXTERN char *heapstart;		/* start of heap (catchall table) heapstart <= heapptr <= heapend */
EXTERN char *heapend;		/* end of free space in heap */
EXTERN char *heapptr;		/* next free space in heap */
EXTERN offset_t btextoffset;	/* text base address; default: 0 */
EXTERN offset_t dynam_size;	/* desired dynamic memory size (including heap, stack, argv and environ); the default of 0 means automatic */

