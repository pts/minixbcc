/* byteord.h - byte order dependencies for C compiler, assembler, linker */

/* These are for the targets of everything and for linker source too. */

#ifdef I8086
# define AS_BIG_ENDIAN 0
# define LONG_BIG_ENDIAN 0	/* except longs are back to front for Xenix */
#endif

#ifdef I80386
# define AS_BIG_ENDIAN 0
# define LONG_BIG_ENDIAN 0
#endif
