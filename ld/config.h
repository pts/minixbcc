/* config.h - configuration for linker */

/* these may need to be defined to suit the source processor */

#ifndef LD_ALIGNMENT
#  define LD_ALIGNMENT ALIGNBYTES  /* source memory alignment, power of 2 */
#endif

/* these should be defined if they are supported by the source compiler */

/* these must be defined to suit the source libraries */

#define CREAT_PERMS 0666	/* permissions for creat() */
#define EXEC_PERMS  0111	/* extra permissions to set for executable */
