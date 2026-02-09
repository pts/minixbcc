/* bcc-cc1.c - "pass 1" for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "const.h"
#include "types.h"

PUBLIC int main P2(int, argc, char **, argv)
{
    growheap(0);		/* init order is important */
    syminit();
    etreeinit();
    ifinit();
    predefine();
    openio(argc, argv);
    codeinit();
    typeinit();
    program();
    finishup();

    /* NOTREACHED */
    return 0;
}

#ifdef MXMALLOC
#  include "mxmalloc.h"  /* Implementation. */
#endif
