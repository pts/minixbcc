/* keywords.c - keywords for assembler */

#include "const.h"
#include "type.h"
#include "opcode.h"

#ifdef KV0  /* Use the keywords as in assembler version 0. It may not work. */
  /* Keyword table extraction command: perl -0777 -wne 'use integer; use strict; my $i = 0; while ($i < length($_)) { my $s = vec($_, $i, 8); die(1) if $s == 0; die(2) if $i + $s + 3 > length($_); my $name = substr($_, $i + 1, $s); $name = join("\x27, \x27", split("", $name)); $i += $s + 3; my $v1 = sprintf("0x%02x", vec($_, $i - 2, 8)); my $v2 = sprintf("0x%02x", vec($_, $i - 1, 8)); print("    $s, \x27$name\x27, $v1, $v2,\n") }' <keywords.bin */
#  undef KV1
#  define W01(wv0, wv1) (wv0)
#  define W01M(wv0, wv1, wv1m) (wv0)
#  define RETFI 99  /* It doesn't work, RETFI is not implemented in version 1. */
#else  /* Default, use keywords in assembler version 1. */
#  undef KV1
#  define KV1 1
#  define W01(wv0, wv1) (wv1)
#  ifdef MINIX_SYNTAX
#    define W01M(wv0, wv1, wv1m) (wv1m)
#  else
#    define W01M(wv0, wv1, wv1m) (wv1)
#  endif
#endif

/* registers */
/* the register code (internal to assembler) is given in 1 byte */
/* the "opcode" field is not used */

PUBLIC char regs[] =
{
    2, 'B', 'P', BPREG, 0,
    2, 'B', 'X', BXREG, 0,
    2, 'D', 'I', DIREG, 0,
    2, 'S', 'I', SIREG, 0,

    3, 'E', 'A', 'X', EAXREG, 0,
    3, 'E', 'B', 'P', EBPREG, 0,
    3, 'E', 'B', 'X', EBXREG, 0,
    3, 'E', 'C', 'X', ECXREG, 0,
    3, 'E', 'D', 'I', EDIREG, 0,
    3, 'E', 'D', 'X', EDXREG, 0,
    3, 'E', 'S', 'I', ESIREG, 0,
    3, 'E', 'S', 'P', ESPREG, 0,

    2, 'A', 'X', AXREG, 0,
    2, 'C', 'X', CXREG, 0,
    2, 'D', 'X', DXREG, 0,
    2, 'S', 'P', SPREG, 0,

    2, 'A', 'H', AHREG, 0,
    2, 'A', 'L', ALREG, 0,
    2, 'B', 'H', BHREG, 0,
    2, 'B', 'L', BLREG, 0,
    2, 'C', 'H', CHREG, 0,
    2, 'C', 'L', CLREG, 0,
    2, 'D', 'H', DHREG, 0,
    2, 'D', 'L', DLREG, 0,

    2, 'C', 'S', CSREG, 0,
    2, 'D', 'S', DSREG, 0,
    2, 'E', 'S', ESREG, 0,
    2, 'F', 'S', FSREG, 0,
    2, 'G', 'S', GSREG, 0,
    2, 'S', 'S', SSREG, 0,

    3, 'C', 'R', '0', CR0REG, 0,
    3, 'C', 'R', '2', CR2REG, 0,
    3, 'C', 'R', '3', CR3REG, 0,
    3, 'D', 'R', '0', DR0REG, 0,
    3, 'D', 'R', '1', DR1REG, 0,
    3, 'D', 'R', '2', DR2REG, 0,
    3, 'D', 'R', '3', DR3REG, 0,
    3, 'D', 'R', '6', DR6REG, 0,
    3, 'D', 'R', '7', DR7REG, 0,
#ifdef KV1
    3, 'T', 'R', '3', TR3REG, 0,
    3, 'T', 'R', '4', TR4REG, 0,
    3, 'T', 'R', '5', TR5REG, 0,
#endif
    3, 'T', 'R', '6', TR6REG, 0,
    3, 'T', 'R', '7', TR7REG, 0,

#ifdef KV1
    2, 'S', 'T', ST0REG, 0,
#endif

    0				/* end of register list */
};

/* type sizes */
/* the "opcode" field gives the type size */

PUBLIC char typesizes[] =
{
    4, 'B', 'Y', 'T', 'E', BYTEOP, 1,
    5, 'D', 'W', 'O', 'R', 'D', DWORDOP, 4,
    5, 'F', 'W', 'O', 'R', 'D', FWORDOP, 6,
    3, 'F', 'A', 'R', FAROP, 0,
    3, 'P', 'T', 'R', PTROP, 0,
    5, 'P', 'W', 'O', 'R', 'D', PWORDOP, W01(4, 6),
    5, 'Q', 'W', 'O', 'R', 'D', QWORDOP, 8,
    5, 'T', 'B', 'Y', 'T', 'E', TBYTEOP, 10,
    4, 'W', 'O', 'R', 'D', WORDOP, 2,
    0				/* end of typesize list */
};

/* ops */
/* the routine number is given in 1 byte */
/* the opcode is given in 1 byte (it is not used for pseudo-ops) */

PUBLIC char ops[] =
{
    /* pseudo-ops. The "opcode" field is unused and padded with a null byte */
    /* conditionals - must be first */
    4, 'E', 'L', 'S', 'E', ELSEOP, 0,
    6, 'E', 'L', 'S', 'E', 'I', 'F', ELSEIFOP, 0,
    7, 'E', 'L', 'S', 'E', 'I', 'F', 'C', ELSEIFCOP, 0,
    5, 'E', 'N', 'D', 'I', 'F', ENDIFOP, 0,
    2, 'I', 'F', IFOP, 0,
    3, 'I', 'F', 'C', IFCOP, 0,

    /* unconditionals */
    /* The code in versions 0 and 1 are identical, but they are in a different order. */
    6, '.', 'A', 'L', 'I', 'G', 'N', ALIGNOP, 0,
    6, '.', 'A', 'S', 'C', 'I', 'I', FCCOP, 0,
    5, '.', 'B', 'L', 'K', 'B', RMBOP, 0,
    5, '.', 'B', 'L', 'K', 'W', BLKWOP, 0,
    5, 'B', 'L', 'O', 'C', 'K', BLOCKOP, 0,
    4, '.', 'B', 'S', 'S', BSSOP, 0,
    5, '.', 'B', 'Y', 'T', 'E', FCBOP, 0,
    4, 'C', 'O', 'M', 'M', COMMOP, 0,
    5, '.', 'C', 'O', 'M', 'M', COMMOP1, 0,
    5, '.', 'D', 'A', 'T', 'A', DATAOP, 0,
#ifdef KV1
    6, '.', 'D', 'A', 'T', 'A', '1', FCBOP, 0,
    6, '.', 'D', 'A', 'T', 'A', '2', FDBOP, 0,
    6, '.', 'D', 'A', 'T', 'A', '4', FQBOP, 0,
#endif
    2, 'D', 'B', FCBOP, 0,
    2, 'D', 'D', FQBOP, 0,
    7, '.', 'D', 'E', 'F', 'I', 'N', 'E', EXPORTOP, 0,
    2, 'D', 'W', FDBOP, 0,
    3, 'E', 'N', 'D', PROCEOFOP, 0,
    4, 'E', 'N', 'D', 'B', ENDBOP, 0,
    5, 'E', 'N', 'T', 'E', 'R', ENTEROP, 0,
    5, 'E', 'N', 'T', 'R', 'Y', ENTRYOP, 0,
    3, 'E', 'Q', 'U', EQUOP, 0,
    5, '.', 'E', 'V', 'E', 'N', EVENOP, 0,
    6, 'E', 'X', 'P', 'O', 'R', 'T', EXPORTOP, 0,
    6, 'E', 'X', 'T', 'E', 'R', 'N', IMPORTOP, 0,
    7, '.', 'E', 'X', 'T', 'E', 'R', 'N', IMPORTOP, 0,
    5, 'E', 'X', 'T', 'R', 'N', IMPORTOP, 0,
    4, 'F', 'A', 'I', 'L', FAILOP, 0,
    5, '.', 'F', 'A', 'I', 'L', FAILOP, 0,
    3, 'F', 'C', 'B', FCBOP, 0,
    3, 'F', 'C', 'C', FCCOP, 0,
    3, 'F', 'D', 'B', FDBOP, 0,
    3, 'G', 'E', 'T', GETOP, 0,
    6, '.', 'G', 'L', 'O', 'B', 'L', W01M(EXPORTOP, GLOBLOP, EXPORTOP), 0,
    5, 'I', 'D', 'E', 'N', 'T', IDENTOP, 0,
    6, 'I', 'M', 'P', 'O', 'R', 'T', IMPORTOP, 0,
    7, 'I', 'N', 'C', 'L', 'U', 'D', 'E', GETOP, 0,
    5, 'L', 'C', 'O', 'M', 'M', LCOMMOP, 0,
    6, '.', 'L', 'C', 'O', 'M', 'M', LCOMMOP1, 0,
    5, '.', 'L', 'I', 'S', 'T', LISTOP, 0,
    3, 'L', 'O', 'C', LOCOP, 0,
    5, '.', 'L', 'O', 'N', 'G', FQBOP, 0,
    8, '.', 'M', 'A', 'C', 'L', 'I', 'S', 'T', MACLISTOP, 0,
    5, 'M', 'A', 'C', 'R', 'O', MACROOP, 0,
    4, '.', 'M', 'A', 'P', MAPOP, 0,
    3, 'O', 'R', 'G', ORGOP, 0,
    4, '.', 'O', 'R', 'G', ORGOP, 0,
    6, 'P', 'U', 'B', 'L', 'I', 'C', EXPORTOP, 0,
    3, 'R', 'M', 'B', RMBOP, 0,
#ifdef KV1
    4, '.', 'R', 'O', 'M', DATAOP, 0,
    5, '.', 'S', 'E', 'C', 'T', SECTOP, 0,
#endif
    3, 'S', 'E', 'T', SETOP, 0,
    5, 'S', 'E', 'T', 'D', 'P', SETDPOP, 0,
    6, '.', 'S', 'H', 'O', 'R', 'T', FDBOP, 0,
    6, '.', 'S', 'P', 'A', 'C', 'E', RMBOP, 0,
    5, '.', 'T', 'E', 'X', 'T', TEXTOP, 0,
    5, 'U', 'S', 'E', '1', '6', USE16OP, 0,
    5, 'U', 'S', 'E', '3', '2', USE32OP, 0,
#ifdef KV1
    5, '.', 'W', 'A', 'R', 'N', WARNOP, 0,
#endif
    5, '.', 'W', 'O', 'R', 'D', FDBOP, 0,
    6, '.', 'Z', 'E', 'R', 'O', 'W', BLKWOP, 0,

    /* hardware ops. The opcode field is now used */
    3, 'A', 'A', 'A', INHER, 0x37,
    3, 'A', 'A', 'D', INHER_A, (char) 0xD5,
    3, 'A', 'A', 'M', INHER_A, (char) 0xD4,
    3, 'A', 'A', 'S', INHER, 0x3F,
    3, 'A', 'D', 'C', GROUP1, 0x10,
    3, 'A', 'D', 'D', GROUP1, 0x00,
    3, 'A', 'N', 'D', GROUP1, 0x20,
    4, 'A', 'R', 'P', 'L', EwGw, 0x63,
    3, 'B', 'C', 'C', BCC, 0x73,
    3, 'B', 'C', 'S', BCC, 0x72,
    3, 'B', 'E', 'Q', BCC, 0x74,
    3, 'B', 'G', 'E', BCC, 0x7D,
    3, 'B', 'G', 'T', BCC, 0x7F,
    3, 'B', 'H', 'I', BCC, 0x77,
    4, 'B', 'H', 'I', 'S', BCC, 0x73,
    3, 'B', 'L', 'E', BCC, 0x7E,
    3, 'B', 'L', 'O', BCC, 0x72,
    4, 'B', 'L', 'O', 'S', BCC, 0x76,
    3, 'B', 'L', 'T', BCC, 0x7C,
    3, 'B', 'M', 'I', BCC, 0x78,
    3, 'B', 'N', 'E', BCC, 0x75,
    5, 'B', 'O', 'U', 'N', 'D', GvMa, 0x62,
    3, 'B', 'P', 'C', BCC, 0x7B,
    3, 'B', 'P', 'L', BCC, 0x79,
    3, 'B', 'P', 'S', BCC, 0x7A,
    2, 'B', 'R', CALL, (char) JMP_OPCODE,
    3, 'B', 'V', 'C', BCC, 0x71,
    3, 'B', 'V', 'S', BCC, 0x70,
    4, 'C', 'A', 'L', 'L', CALL, (char) JSR_OPCODE,
#ifdef KV1
    5, 'C', 'A', 'L', 'L', 'F', CALLI, (char) 0x9A,
#endif
    5, 'C', 'A', 'L', 'L', 'I', CALLI, (char) 0x9A,
    3, 'C', 'B', 'W', INHER16, (char) 0x98,
    3, 'C', 'L', 'C', INHER, (char) 0xF8,
    3, 'C', 'L', 'D', INHER, (char) 0xFC,
    3, 'C', 'L', 'I', INHER, (char) 0xFA,
    3, 'C', 'M', 'C', INHER, (char) 0xF5,
    3, 'C', 'M', 'P', GROUP1, CMP_OPCODE_BASE,
    4, 'C', 'M', 'P', 'S', W01(INHER16, INHER), (char) CMPSW_OPCODE,
    5, 'C', 'M', 'P', 'S', 'B', INHER, (char) CMPSB_OPCODE,
    5, 'C', 'M', 'P', 'S', 'D', INHER32, (char) CMPSW_OPCODE,
    5, 'C', 'M', 'P', 'S', 'W', INHER16, (char) CMPSW_OPCODE,
    4, 'C', 'M', 'P', 'W', INHER16, (char) CMPSW_OPCODE,
#ifdef KV1
    4, 'C', 'S', 'E', 'G', INHER, 0x2E,
#endif
    3, 'C', 'W', 'D', INHER16, (char) 0x99,
    4, 'C', 'W', 'D', 'E', INHER32, (char) 0x98,
    3, 'C', 'D', 'Q', INHER32, (char) 0x99,
    3, 'D', 'A', 'A', INHER, 0x27,
    3, 'D', 'A', 'S', INHER, 0x2F,
#ifdef KV1
    4, 'D', 'S', 'E', 'G', INHER, 0x3E,
#endif
    3, 'D', 'E', 'C', INCDEC, 0x08,
    3, 'D', 'I', 'V', DIVMUL, 0x30,
    5, 'E', 'N', 'T', 'E', 'R', ENTER, (char) 0xC8,
#ifdef KV1
    4, 'E', 'S', 'E', 'G', INHER, 0x26,
    4, 'F', 'S', 'E', 'G', INHER, 0x64,
    4, 'G', 'S', 'E', 'G', INHER, 0x65,
#endif
    3, 'H', 'L', 'T', INHER, (char) 0xF4,
    4, 'I', 'D', 'I', 'V', DIVMUL, 0x38,
    4, 'I', 'M', 'U', 'L', IMUL, 0x28,
    2, 'I', 'N', IN, (char) 0xEC,
#ifdef KV1
#else
    3, 'I', 'N', 'B', INHER, (char) 0xEC,
#endif
    3, 'I', 'N', 'C', INCDEC, 0x00,
    3, 'I', 'N', 'S', W01(INHER16, INHER), 0x6D,
    4, 'I', 'N', 'S', 'B', INHER, 0x6C,
    4, 'I', 'N', 'S', 'D', INHER32, 0x6D,
    4, 'I', 'N', 'S', 'W', INHER16, 0x6D,
    3, 'I', 'N', 'T', INT, (char) 0xCD,
    4, 'I', 'N', 'T', 'O', INHER, (char) 0xCE,
    3, 'I', 'N', 'W', W01(INHER16, IN), (char) 0xED,
    4, 'I', 'R', 'E', 'T', INHER16, (char) 0xCF,
    5, 'I', 'R', 'E', 'T', 'D', INHER32, (char) 0xCF,
    1, 'J', W01(JCC, CALL), (char) JMP_SHORT_OPCODE,
    2, 'J', 'A', JCC, 0x77,
    3, 'J', 'A', 'E', JCC, 0x73,
    2, 'J', 'B', JCC, 0x72,
    3, 'J', 'B', 'E', JCC, 0x76,
    2, 'J', 'C', JCC, 0x72,
    4, 'J', 'C', 'X', 'E', JCXZ, 0x02,
    4, 'J', 'C', 'X', 'Z', JCXZ, 0x02,
    5, 'J', 'E', 'C', 'X', 'E', JCXZ, 0x04,
    5, 'J', 'E', 'C', 'X', 'Z', JCXZ, 0x04,
    2, 'J', 'E', JCC, 0x74,
    2, 'J', 'G', JCC, 0x7F,
    3, 'J', 'G', 'E', JCC, 0x7D,
    2, 'J', 'L', JCC, 0x7C,
    3, 'J', 'L', 'E', JCC, 0x7E,
    3, 'J', 'M', 'P', CALL, (char) W01M(JMP_OPCODE, JMP_SHORT_OPCODE, JMP_OPCODE),
#ifdef KV1
    4, 'J', 'M', 'P', 'F', CALLI, (char) 0xEA,
#endif
    4, 'J', 'M', 'P', 'I', CALLI, (char) 0xEA,
    3, 'J', 'N', 'A', JCC, 0x76,
    4, 'J', 'N', 'A', 'E', JCC, 0x72,
    3, 'J', 'N', 'B', JCC, 0x73,
    4, 'J', 'N', 'B', 'E', JCC, 0x77,
    3, 'J', 'N', 'C', JCC, 0x73,
    3, 'J', 'N', 'E', JCC, 0x75,
    3, 'J', 'N', 'G', JCC, 0x7E,
    4, 'J', 'N', 'G', 'E', JCC, 0x7C,
    3, 'J', 'N', 'L', JCC, 0x7D,
    4, 'J', 'N', 'L', 'E', JCC, 0x7F,
    3, 'J', 'N', 'O', JCC, 0x71,
    3, 'J', 'N', 'P', JCC, 0x7B,
    3, 'J', 'N', 'S', JCC, 0x79,
    3, 'J', 'N', 'Z', JCC, 0x75,
    2, 'J', 'O', JCC, 0x70,
    2, 'J', 'P', JCC, 0x7A,
    3, 'J', 'P', 'E', JCC, 0x7A,
    3, 'J', 'P', 'O', JCC, 0x7B,
    2, 'J', 'S', JCC, 0x78,
    2, 'J', 'Z', JCC, 0x74,
    4, 'L', 'A', 'H', 'F', INHER, (char) 0x9F,
    3, 'L', 'D', 'S', GvMp, (char) 0xC5,
    3, 'L', 'E', 'A', LEA, (char) 0x8D,
    5, 'L', 'E', 'A', 'V', 'E', INHER, (char) 0xC9,
    3, 'L', 'E', 'S', GvMp, (char) 0xC4,
    4, 'L', 'O', 'C', 'K', INHER, (char) 0xF0,
    4, 'L', 'O', 'D', 'B', INHER, (char) 0xAC,
    4, 'L', 'O', 'D', 'S', W01(INHER16, INHER), (char) 0xAD,
    5, 'L', 'O', 'D', 'S', 'B', INHER, (char) 0xAC,
    5, 'L', 'O', 'D', 'S', 'D', INHER32, (char) 0xAD,
    5, 'L', 'O', 'D', 'S', 'W', INHER16, (char) 0xAD,
    4, 'L', 'O', 'D', 'W', INHER16, (char) 0xAD,
    4, 'L', 'O', 'O', 'P', JCC, (char) 0xE2,
    5, 'L', 'O', 'O', 'P', 'E', JCC, (char) 0xE1,
    6, 'L', 'O', 'O', 'P', 'N', 'E', JCC, (char) 0xE0,
    6, 'L', 'O', 'O', 'P', 'N', 'Z', JCC, (char) 0xE0,
    5, 'L', 'O', 'O', 'P', 'Z', JCC, (char) 0xE1,
    3, 'M', 'O', 'V', MOV, (char) 0x88,
    4, 'M', 'O', 'V', 'S', W01(INHER16, INHER), (char) MOVSW_OPCODE,
    5, 'M', 'O', 'V', 'S', 'B', INHER, (char) MOVSB_OPCODE,
    5, 'M', 'O', 'V', 'S', 'D', INHER32, (char) MOVSW_OPCODE,
    5, 'M', 'O', 'V', 'S', 'W', INHER16, (char) MOVSW_OPCODE,
    4, 'M', 'O', 'V', 'W', INHER16, (char) MOVSW_OPCODE,
    3, 'M', 'U', 'L', DIVMUL, 0x20,
    3, 'N', 'E', 'G', NEGNOT, 0x18,
    3, 'N', 'O', 'P', INHER, (char) 0x90,
    3, 'N', 'O', 'T', NEGNOT, 0x10,
    2, 'O', 'R', GROUP1, 0x08,
    3, 'O', 'U', 'T', OUT, (char) 0xEE,
#ifdef KV1
#else
    4, 'O', 'U', 'T', 'B', INHER, (char) 0xEE,
#endif
    4, 'O', 'U', 'T', 'S', W01(INHER16, INHER), 0x6F,
    5, 'O', 'U', 'T', 'S', 'B', INHER, 0x6E,
    5, 'O', 'U', 'T', 'S', 'D', INHER32, 0x6F,
    5, 'O', 'U', 'T', 'S', 'W', INHER16, 0x6F,
    4, 'O', 'U', 'T', 'W', W01(INHER16, OUT), (char) 0xEF,
    3, 'P', 'O', 'P', PUSHPOP, (char) POP_OPCODE,
    4, 'P', 'O', 'P', 'A', INHER16, 0x61,
    5, 'P', 'O', 'P', 'A', 'D', INHER32, 0x61,
    4, 'P', 'O', 'P', 'F', INHER16, (char) 0x9D,
    5, 'P', 'O', 'P', 'F', 'D', INHER32, (char) 0x9D,
    4, 'P', 'U', 'S', 'H', PUSHPOP, (char) PUSH_OPCODE,
    5, 'P', 'U', 'S', 'H', 'A', INHER16, 0x60,
    6, 'P', 'U', 'S', 'H', 'A', 'D', INHER32, 0x60,
    5, 'P', 'U', 'S', 'H', 'F', INHER16, (char) 0x9C,
    6, 'P', 'U', 'S', 'H', 'F', 'D', INHER32, (char) 0x9C,
    3, 'R', 'C', 'L', GROUP2, 0x10,
    3, 'R', 'C', 'R', GROUP2, 0x18,
    3, 'R', 'O', 'L', GROUP2, 0x00,
    3, 'R', 'O', 'R', GROUP2, 0x08,
    3, 'R', 'E', 'P', INHER, (char) 0xF3,
    4, 'R', 'E', 'P', 'E', INHER, (char) 0xF3,
    5, 'R', 'E', 'P', 'N', 'E', INHER, (char) 0xF2,
    5, 'R', 'E', 'P', 'N', 'Z', INHER, (char) 0xF2,
    4, 'R', 'E', 'P', 'Z', INHER, (char) 0xF3,
    3, 'R', 'E', 'T', RET, (char) 0xC3,
    4, 'R', 'E', 'T', 'F', W01(RETFI, RET), (char) 0xCB,
    4, 'R', 'E', 'T', 'I', W01(RETFI, RET), (char) 0xCB,
    4, 'S', 'A', 'H', 'F', INHER, (char) 0x9E,
    3, 'S', 'A', 'L', GROUP2, 0x20,
    3, 'S', 'A', 'R', GROUP2, 0x38,
    3, 'S', 'B', 'B', GROUP1, 0x18,
    4, 'S', 'C', 'A', 'B', INHER, (char) 0xAE,
    4, 'S', 'C', 'A', 'S', W01(INHER16, INHER), (char) 0xAF,
    5, 'S', 'C', 'A', 'S', 'B', INHER, (char) 0xAE,
    5, 'S', 'C', 'A', 'S', 'D', INHER32, (char) 0xAF,
    5, 'S', 'C', 'A', 'S', 'W', INHER16, (char) 0xAF,
    4, 'S', 'C', 'A', 'W', INHER16, (char) 0xAF,
    3, 'S', 'E', 'G', SEG, 0x06,
    3, 'S', 'H', 'L', GROUP2, 0x20,
    3, 'S', 'H', 'R', GROUP2, 0x28,
#ifdef KV1
    4, 'S', 'S', 'E', 'G', INHER, 0x36,
#endif
    3, 'S', 'T', 'C', INHER, (char) 0xF9,
    3, 'S', 'T', 'D', INHER, (char) 0xFD,
    3, 'S', 'T', 'I', INHER, (char) 0xFB,
    4, 'S', 'T', 'O', 'B', INHER, (char) 0xAA,
    4, 'S', 'T', 'O', 'S', W01(INHER16, INHER), (char) 0xAB,
    5, 'S', 'T', 'O', 'S', 'B', INHER, (char) 0xAA,
    5, 'S', 'T', 'O', 'S', 'D', INHER32, (char) 0xAB,
    5, 'S', 'T', 'O', 'S', 'W', INHER16, (char) 0xAB,
    4, 'S', 'T', 'O', 'W', INHER16, (char) 0xAB,
    3, 'S', 'U', 'B', GROUP1, 0x28,
    4, 'T', 'E', 'S', 'T', TEST, (char) 0x84,
    4, 'W', 'A', 'I', 'T', INHER, (char) WAIT_OPCODE,
    4, 'X', 'C', 'H', 'G', XCHG, (char) 0x86,
    4, 'X', 'L', 'A', 'T', INHER, (char) 0xD7,
    5, 'X', 'L', 'A', 'T', 'B', INHER, (char) 0xD7,
    3, 'X', 'O', 'R', GROUP1, 0x30,

    /* floating point */
#ifdef KV1
    5, 'F', '2', 'X', 'M', '1', F_INHER, 0x70,
    4, 'F', 'A', 'B', 'S', F_INHER, 0x61,
    4, 'F', 'A', 'D', 'D', F_M4_M8_STST, 0x00,
    5, 'F', 'A', 'D', 'D', 'P', F_STST, 0x60,
    4, 'F', 'B', 'L', 'D', F_M10, 0x74,
    5, 'F', 'B', 'S', 'T', 'P', F_M10, 0x76,
    4, 'F', 'C', 'H', 'S', F_INHER, 0x60,
    5, 'F', 'C', 'L', 'E', 'X', F_W_INHER, (char) 0xE2,
    4, 'F', 'C', 'O', 'M', F_M4_M8_OPTST, 0x02,
    5, 'F', 'C', 'O', 'M', 'P', F_M4_M8_OPTST, 0x03,
    6, 'F', 'C', 'O', 'M', 'P', 'P', F_INHER, 0x19,
    4, 'F', 'C', 'O', 'S', F_INHER, 0x7F,
    7, 'F', 'D', 'E', 'C', 'S', 'T', 'P', F_INHER, 0x76,
    5, 'F', 'D', 'I', 'S', 'I', F_W_INHER, (char) 0xE1,
    4, 'F', 'D', 'I', 'V', F_M4_M8_STST, 0x06,
    5, 'F', 'D', 'I', 'V', 'P', F_STST, 0x67,
    5, 'F', 'D', 'I', 'V', 'R', F_M4_M8_STST, 0x07,
    6, 'F', 'D', 'I', 'V', 'R', 'P', F_STST, 0x66,
    4, 'F', 'E', 'N', 'I', F_W_INHER, (char) 0xE0,
    5, 'F', 'F', 'R', 'E', 'E', F_ST, 0x50,
    5, 'F', 'I', 'A', 'D', 'D', F_M2_M4, 0x20,
    5, 'F', 'I', 'C', 'O', 'M', F_M2_M4, 0x22,
    6, 'F', 'I', 'C', 'O', 'M', 'P', F_M2_M4, 0x23,
    5, 'F', 'I', 'D', 'I', 'V', F_M2_M4, 0x26,
    6, 'F', 'I', 'D', 'I', 'V', 'R', F_M2_M4, 0x27,
    4, 'F', 'I', 'L', 'D', F_M2_M4_M8, 0x30,
    5, 'F', 'I', 'M', 'U', 'L', F_M2_M4, 0x21,
    7, 'F', 'I', 'N', 'C', 'S', 'T', 'P', F_INHER, 0x77,
    5, 'F', 'I', 'N', 'I', 'T', F_W_INHER, (char) 0xE3,
    4, 'F', 'I', 'S', 'T', F_M2_M4, 0x32,
    5, 'F', 'I', 'S', 'T', 'P', F_M2_M4_M8, 0x33,
    5, 'F', 'I', 'S', 'U', 'B', F_M2_M4, 0x24,
    6, 'F', 'I', 'S', 'U', 'B', 'R', F_M2_M4, 0x25,
    3, 'F', 'L', 'D', F_M4_M8_M10_ST, 0x10,
    4, 'F', 'L', 'D', '1', F_INHER, 0x68,
    6, 'F', 'L', 'D', 'L', '2', 'E', F_INHER, 0x6A,
    6, 'F', 'L', 'D', 'L', '2', 'T', F_INHER, 0x69,
    5, 'F', 'L', 'D', 'C', 'W', F_M2, 0x15,
    6, 'F', 'L', 'D', 'E', 'N', 'V', F_M, 0x14,
    6, 'F', 'L', 'D', 'L', 'G', '2', F_INHER, 0x6C,
    6, 'F', 'L', 'D', 'L', 'N', '2', F_INHER, 0x6D,
    5, 'F', 'L', 'D', 'P', 'I', F_INHER, 0x6B,
    4, 'F', 'L', 'D', 'Z', F_INHER, 0x6E,
    4, 'F', 'M', 'U', 'L', F_M4_M8_STST, 0x01,
    5, 'F', 'M', 'U', 'L', 'P', F_STST, 0x61,
    6, 'F', 'N', 'C', 'L', 'E', 'X', F_INHER, (char) 0xE2,
    6, 'F', 'N', 'D', 'I', 'S', 'I', F_INHER, (char) 0xE1,
    5, 'F', 'N', 'E', 'N', 'I', F_INHER, (char) 0xE0,
    6, 'F', 'N', 'I', 'N', 'I', 'T', F_INHER, (char) 0xE3,
    4, 'F', 'N', 'O', 'P', F_INHER, 0x50,
    6, 'F', 'N', 'S', 'A', 'V', 'E', F_M, 0x56,
    6, 'F', 'N', 'S', 'T', 'C', 'W', F_M2, 0x17,
    7, 'F', 'N', 'S', 'T', 'E', 'N', 'V', F_M, 0x16,
    6, 'F', 'N', 'S', 'T', 'S', 'W', F_M2_AX, 0x57,
    6, 'F', 'P', 'A', 'T', 'A', 'N', F_INHER, 0x73,
    5, 'F', 'P', 'R', 'E', 'M', F_INHER, 0x78,
    6, 'F', 'P', 'R', 'E', 'M', '1', F_INHER, 0x75,
    5, 'F', 'P', 'T', 'A', 'N', F_INHER, 0x72,
    7, 'F', 'R', 'N', 'D', 'I', 'N', 'T', F_INHER, 0x7C,
    6, 'F', 'R', 'S', 'T', 'O', 'R', F_M, 0x54,
    5, 'F', 'S', 'A', 'V', 'E', F_W_M, 0x56,
    6, 'F', 'S', 'C', 'A', 'L', 'E', F_INHER, 0x7D,
    6, 'F', 'S', 'E', 'T', 'P', 'M', F_INHER, (char) 0xE4,
    4, 'F', 'S', 'I', 'N', F_INHER, 0x7E,
    7, 'F', 'S', 'I', 'N', 'C', 'O', 'S', F_INHER, 0x7B,
    5, 'F', 'S', 'Q', 'R', 'T', F_INHER, 0x7A,
    3, 'F', 'S', 'T', F_M4_M8_ST, FST_ENCODED,
    5, 'F', 'S', 'T', 'C', 'W', F_W_M2, 0x17,
    6, 'F', 'S', 'T', 'E', 'N', 'V', F_W_M, 0x16,
    4, 'F', 'S', 'T', 'P', F_M4_M8_M10_ST, FSTP_ENCODED,
    5, 'F', 'S', 'T', 'S', 'W', F_W_M2_AX, 0x57,
    4, 'F', 'S', 'U', 'B', F_M4_M8_STST, 0x04,
    5, 'F', 'S', 'U', 'B', 'P', F_STST, 0x65,
    5, 'F', 'S', 'U', 'B', 'R', F_M4_M8_STST, 0x05,
    6, 'F', 'S', 'U', 'B', 'R', 'P', F_STST, 0x64,
    4, 'F', 'T', 'S', 'T', F_INHER, 0x64,
    5, 'F', 'U', 'C', 'O', 'M', F_OPTST, 0x54,
    6, 'F', 'U', 'C', 'O', 'M', 'P', F_OPTST, 0x55,
    7, 'F', 'U', 'C', 'O', 'M', 'P', 'P', F_INHER, (char) 0xA9,
    5, 'F', 'W', 'A', 'I', 'T', INHER, (char) WAIT_OPCODE,
    4, 'F', 'X', 'A', 'M', F_INHER, 0x65,
    4, 'F', 'X', 'C', 'H', F_OPTST, 0x11,
    7, 'F', 'X', 'T', 'R', 'A', 'C', 'T', F_INHER, 0x74,
    5, 'F', 'Y', 'L', '2', 'X', F_INHER, 0x71,
    7, 'F', 'Y', 'L', '2', 'X', 'P', '1', F_INHER, 0x79,
#endif

    0				/* end of ops */
};

PUBLIC char page1ops[] =
{
    3, 'B', 'S', 'F', GvEv, (char) 0xBC,
    3, 'B', 'S', 'R', GvEv, (char) 0xBD,
#ifdef KV1
    5, 'B', 'S', 'W', 'A', 'P', BSWAP, (char) 0xC8,
#endif
    2, 'B', 'T', GROUP8, 0x20,
    3, 'B', 'T', 'C', GROUP8, 0x38,
    3, 'B', 'T', 'R', GROUP8, 0x30,
    3, 'B', 'T', 'S', GROUP8, 0x28,
    4, 'C', 'L', 'T', 'S', INHER, 0x06,
#ifdef KV1
    7, 'C', 'M', 'P', 'X', 'C', 'H', 'G', ExGx, (char) 0xA6,
    4, 'I', 'N', 'V', 'D', INHER, 0x08,
    6, 'I', 'N', 'V', 'L', 'P', 'G', GROUP7, 0x38,
#endif
    3, 'L', 'A', 'R', GvEv, 0x02,
    3, 'L', 'F', 'S', GvMp, (char) 0xB4,
    4, 'L', 'G', 'D', 'T', GROUP7, 0x10,
    3, 'L', 'G', 'S', GvMp, (char) 0xB5,
    4, 'L', 'I', 'D', 'T', GROUP7, 0x18,
    4, 'L', 'L', 'D', 'T', GROUP6, 0x10,
    4, 'L', 'M', 'S', 'W', GROUP7, 0x30,
    3, 'L', 'S', 'L', GvEv, 0x03,
    3, 'L', 'S', 'S', GvMp, (char) 0xB2,
    3, 'L', 'T', 'R', GROUP6, 0x18,
    5, 'M', 'O', 'V', 'S', 'X', MOVX, (char) 0xBE,
    5, 'M', 'O', 'V', 'Z', 'X', MOVX, (char) 0xB6,
    4, 'S', 'E', 'T', 'A', SETCC, (char) 0x97,
    5, 'S', 'E', 'T', 'A', 'E', SETCC, (char) 0x93,
    4, 'S', 'E', 'T', 'B', SETCC, (char) 0x92,
    5, 'S', 'E', 'T', 'B', 'E', SETCC, (char) 0x96,
    4, 'S', 'E', 'T', 'C', SETCC, (char) 0x92,
    4, 'S', 'E', 'T', 'E', SETCC, (char) 0x94,
    4, 'S', 'E', 'T', 'G', SETCC, (char) 0x9F,
    5, 'S', 'E', 'T', 'G', 'E', SETCC, (char) 0x9D,
    4, 'S', 'E', 'T', 'L', SETCC, (char) 0x9C,
    5, 'S', 'E', 'T', 'L', 'E', SETCC, (char) 0x9E,
    5, 'S', 'E', 'T', 'N', 'A', SETCC, (char) 0x96,
    6, 'S', 'E', 'T', 'N', 'A', 'E', SETCC, (char) 0x92,
    5, 'S', 'E', 'T', 'N', 'B', SETCC, (char) 0x93,
    6, 'S', 'E', 'T', 'N', 'B', 'E', SETCC, (char) 0x97,
    5, 'S', 'E', 'T', 'N', 'C', SETCC, (char) 0x93,
    5, 'S', 'E', 'T', 'N', 'E', SETCC, (char) 0x95,
    5, 'S', 'E', 'T', 'N', 'G', SETCC, (char) 0x9E,
    6, 'S', 'E', 'T', 'N', 'G', 'E', SETCC, (char) 0x9C,
    5, 'S', 'E', 'T', 'N', 'L', SETCC, (char) 0x9D,
    6, 'S', 'E', 'T', 'N', 'L', 'E', SETCC, (char) 0x9F,
    5, 'S', 'E', 'T', 'N', 'O', SETCC, (char) 0x91,
    5, 'S', 'E', 'T', 'N', 'P', SETCC, (char) 0x9B,
    5, 'S', 'E', 'T', 'N', 'S', SETCC, (char) 0x99,
    5, 'S', 'E', 'T', 'N', 'Z', SETCC, (char) 0x95,
    4, 'S', 'E', 'T', 'O', SETCC, (char) 0x90,
    4, 'S', 'E', 'T', 'P', SETCC, (char) 0x9A,
    5, 'S', 'E', 'T', 'P', 'E', SETCC, (char) 0x9A,
    5, 'S', 'E', 'T', 'P', 'O', SETCC, (char) 0x9B,
    4, 'S', 'E', 'T', 'S', SETCC, (char) 0x98,
    4, 'S', 'E', 'T', 'Z', SETCC, (char) 0x94,
    4, 'S', 'G', 'D', 'T', GROUP7, 0x00,
    4, 'S', 'I', 'D', 'T', GROUP7, 0x08,
    4, 'S', 'H', 'L', 'D', SH_DOUBLE, (char) 0xA4,
    4, 'S', 'H', 'R', 'D', SH_DOUBLE, (char) 0xAC,
    4, 'S', 'L', 'D', 'T', GROUP6, 0x00,
    4, 'S', 'M', 'S', 'W', GROUP7, 0x20,
    3, 'S', 'T', 'R', GROUP6, 0x08,
    4, 'V', 'E', 'R', 'R', GROUP6, 0x20,
    4, 'V', 'E', 'R', 'W', GROUP6, 0x28,
#ifdef KV1
    6, 'W', 'B', 'I', 'N', 'V', 'D', INHER, 0x09,
    4, 'X', 'A', 'D', 'D', ExGx, (char) 0xC0,
#endif

    0				/* end of page 1 ops */
};

PUBLIC char page2ops[] =  /* !! Remove this constant. */
{

    0				/* end of page 2 ops */
};

#ifdef MNSIZE
PUBLIC char bytesizeops[] =
{
    4, 'A', 'D', 'C', 'B', GROUP1, 0x10,
    4, 'A', 'D', 'D', 'B', GROUP1, 0x00,
    4, 'A', 'N', 'D', 'B', GROUP1, 0x20,
    4, 'C', 'M', 'P', 'B', GROUP1, CMP_OPCODE_BASE,
    4, 'D', 'E', 'C', 'b', INCDEC, 0x08,
    4, 'D', 'I', 'V', 'B', DIVMUL, 0x30,
    5, 'I', 'D', 'I', 'V', 'B', DIVMUL, 0x38,
    5, 'I', 'M', 'U', 'L', 'B', IMUL, 0x28,
#ifdef KV1
    3, 'I', 'N', 'B', IN, (char) 0xEC,
#endif
    4, 'I', 'N', 'C', 'B', INCDEC, 0x00,
    4, 'M', 'O', 'V', 'B', MOV, (char) 0x88,
    4, 'M', 'U', 'L', 'B', DIVMUL, 0x20,
    4, 'N', 'E', 'G', 'B', NEGNOT, 0x18,
    4, 'N', 'O', 'T', 'B', NEGNOT, 0x10,
    3, 'O', 'R', 'B', GROUP1, 0x08,
#ifdef KV1
    4, 'O', 'U', 'T', 'B', OUT, (char) 0xEE,
#endif
    4, 'R', 'C', 'L', 'B', GROUP2, 0x10,
    4, 'R', 'C', 'R', 'B', GROUP2, 0x18,
    4, 'R', 'O', 'L', 'B', GROUP2, 0x00,
    4, 'R', 'O', 'R', 'B', GROUP2, 0x08,
    4, 'S', 'A', 'L', 'B', GROUP2, 0x20,
    4, 'S', 'A', 'R', 'B', GROUP2, 0x38,
    4, 'S', 'H', 'L', 'B', GROUP2, 0x20,
    4, 'S', 'H', 'R', 'B', GROUP2, 0x28,
    4, 'S', 'B', 'B', 'B', GROUP1, 0x18,
    4, 'S', 'U', 'B', 'B', GROUP1, 0x28,
    5, 'T', 'E', 'S', 'T', 'B', TEST, (char) 0x84,
    5, 'X', 'C', 'H', 'G', 'B', XCHG, (char) 0x86,
    4, 'X', 'O', 'R', 'B', GROUP1, 0x30,
    0				/* end of byte size ops */
};
#endif /* MNSIZE */
