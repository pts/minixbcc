/* by pts, to be compiled with /local/bin/sc.pts */

#define atof no_atof
#include "const.h"
#include "types.h"
#include "proto.h"
#undef atof

void atof()   { fatalerror("no atof"); }
void Fdi()    { fatalerror("no dtoi"); }
void Fmp()    { fatalerror("no fcmp"); }
void Fmpd()   { fatalerror("no fcmpd"); }
void Fulld()  { fatalerror("no fpulld"); }
void Fuld()   { fatalerror("no fmuld"); }
void Fegd()   { fatalerror("no fnegd"); }
void Fushui() { fatalerror("no fpushui"); }
void Fdd()    { fatalerror("no fadd"); }
void Fddd()   { fatalerror("no faddd"); }
void Fub()    { fatalerror("no fsub"); }
void Fubd()   { fatalerror("no fsubd"); }
void Fstd()   { fatalerror("no ftstd"); }
void Fushc()  { fatalerror("no fpushc"); }
void Fushd()  { fatalerror("no fpushd"); }
void Fushi()  { fatalerror("no fpushi"); }
void Fivd()   { fatalerror("no fdivd"); }
void Fullf()  { fatalerror("no fpullf"); }
