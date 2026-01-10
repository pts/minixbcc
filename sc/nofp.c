#include "const.h"
#include "types.h"

#ifdef NOFP  /* The error messages below indicate lack of floating point support in the C compiler. */
  void no_atof()                { fatalerror("no floating-point (literal)"); }
  void no_1double_to_float()    { fatalerror("no floating-point (double-to-float)"); }
  void no_2double_to_intvalue() { fatalerror("no floating-point (double-to-intvalue)"); }
  void no_intvalue_to_double()  { fatalerror("no floating-point (intvalue-to-double)"); }
  void no_3double_op()          { fatalerror("no floating-point (constant-folding)"); }
#endif
