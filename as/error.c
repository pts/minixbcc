/* error.c - error routines for assembler */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#include "const.h"
#include "type.h"

PRIVATE char *errormessage[] =
{
  /* Syntax errors. */
    /* COMEXP */  "comma expected",
    /* DELEXP */  "delimiter expected",
    /* FACEXP */  "factor expected",
    /* IREGEXP */ "index register expected",
    /* LABEXP */  "label expected",
    /* LPEXP */   "left parentheses expected",
    /* OPEXP */   "opcode expected",
    /* RBEXP */   "right bracket expected",
    /* REGEXP */  "register expected",
    /* RPEXP */   "right parentheses expected",
    /* SPEXP */   "space expected",

  /* Expression errors. */
    /* ABSREQ */     "absolute expression required",
    /* NONIMPREQ */  "non-imported expression required",
    /* RELBAD */     "relocation impossible",

  /* Label errors. */
    /* ILLAB */   "illegal label",
    /* MACUID */  "MACRO used as identifier",
    /* MISLAB */  "missing label",
    /* MNUID */   "opcode used as identifier",
    /* REGUID */  "register used as identifier",
    /* RELAB */   "redefined label",
    /* UNBLAB */  "unbound label",
    /* UNLAB */   "undefined label",
    /* VARLAB */  "variable used as label",

  /* Addressing errors. */
    /* ABOUNDS */  "address out of bounds",
    /* DBOUNDS */  "data out of bounds",
    /* ILLMOD */   "illegal address mode",
    /* ILLREG */   "illegal register",

  /* Control structure errors. */
    /* ELSEBAD */  /* ELSEIFBAD */  /* ENDIFBAD */  "no matching IF",
    /* ENDBBAD */  "no matching BLOCK",
    /* EOFBLOCK */ "end of file in BLOCK",
    /* EOFIF */    "end of file in IF",
    /* EOFLC */    "location counter was undefined at end",
    /* EOFMAC */   "end of file in MACRO",
    /* FAILERR */  "user-generated error",

  /* Overflow errors. */
    /* BLOCKOV */  "BLOCK stack overflow",
    /* BWRAP */    "binary file wrap-around",
    /* COUNTOV */  "counter overflow",
    /* COUNTUN */  "counter underflow",
    /* GETOV */    "GET stack overflow",  /* GET == INCLUDE */
    /* IFOV */     "IF stack overflow",

    /* LINLONG */   "line too long",
    /* MACOV */     "MACRO stack overflow",
    /* OBJSYMOV */  "object symbol table overflow",
    /* OWRITE */    "",  /* "program overwrite", -- never emitted, kept for compatibility only. */
    /* PAROV */     "parameter table overflow",
    /* SYMOV */     "symbol table overflow",
    /* SYMOUTOV */  "output symbol table overflow",

  /* I/O errors. */
    /* OBJOUT */  "error writing object file",

  /* Miscellaneous errors. */
    /* AL_AX_EAX_EXP */    "al, ax or eax expected",
    /* CTLINS */           "control character in string",
    /* FURTHER */          "futher errors suppressed",
    /* ILL_IMM_MODE */     "illegal immediate mode",
    /* ILL_IND_TO_IND */   "illegal indirect to indirect",
    /* ILL_IND */          "illegal indirection",
    /* ILL_IND_PTR */      "illegal indirection from previous 'ptr'",
    /* ILL_SCALE */        "illegal scale",
    /* ILL_SECTION */      "illegal section",
    /* ILL_SEG_REG */      "illegal segment register",
    /* ILL_SOURCE_EA */    "illegal source effective address",
    /* ILL_SIZE */         "illegal size",
    /* IMM_REQ */          "immediate expression expected",
    /* INDEX_REG_EXP */    "index register expected",
    /* IND_REQ */          "indirect expression required",
    /* MISMATCHED_SIZE */  "mismatched size",
    /* NOIMPORT */         "no imports with binary file output",
    /* REENTER */          "multiple ENTER pseudo-ops",
    /* REL_REQ */          "relative expression required",
    /* REPEATED_DISPL */   "repeated displacement",
    /* SEGREL */           "segment or relocatability redefined",
    /* SEG_REG_REQ */      "segment register required",
    /* SIZE_UNK */         "size unknown",

    /* FP_REG_REQ */           "FP register required",
    /* FP_REG_NOT_ALLOWED */   "FP register not allowed",
    /* ILL_FP_REG */           "illegal FP register",
    /* ILL_FP_REG_PAIR */      "illegal FP register pair",
    /* JUNK_AFTER_OPERANDS */  "junk after operands",

    /* ALREADY */"already defined",

  /* Warnings. */  /* MINWARN */
    /* SHORTB */  "short branch would do",

    "unknown error",
};

/* build null-terminated error message for given error at given spot */

PUBLIC char *build_error_message(errnum, buf)
unsigned errnum;
char *buf;
{
    if (errnum >= sizeof errormessage / sizeof errormessage[0])
	errnum = sizeof errormessage / sizeof errormessage[0] - 1;
    return strcpy(buf, errormessage[errnum]);
}
