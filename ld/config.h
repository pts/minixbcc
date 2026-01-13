/* config.h - configuration for linker */

/* these may need to be defined to suit the source processor */

#define S_ALIGNMENT 8		/* source memory alignment, power of 2 */
				/* don't use for 8 bit processors */
				/* don't use even for 80386 - overhead for */
				/* alignment cancels improved access */

/* these should be defined if they are supported by the source compiler */

#undef  PROTO			/* compiler handles prototypes */

/* these must be defined to suit the source libraries */

#define CREAT_PERMS 0666	/* permissions for creat() */
#define EXEC_PERMS  0111	/* extra permissions to set for executable */
