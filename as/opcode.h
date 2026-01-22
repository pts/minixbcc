/* opcode.h - routine numbers and special opcodes for assembler */

enum
{
/* Pseudo-op routine numbers.
 * Conditionals are first - this is used to test if op is a conditional.
 */
    ELSEOP,          /* pelse */
    ELSEIFOP,        /* pelseif */
    ELSEIFCOP,       /* pelsifc */
    ENDIFOP,         /* pendif */
    IFOP,            /* pif */
    IFCOP,           /* pifc */

#define MIN_NONCOND	ALIGNOP
    ALIGNOP,         /* palign */
    BLKWOP,          /* pblkw */
    BLOCKOP,         /* pblock */
    BSSOP,           /* pbss */
    COMMOP,          /* pcomm */
    COMMOP1,         /* pcomm1 */
    DATAOP,          /* pdata */
    ENDBOP,          /* pendb */
    ENTEROP,         /* penter */
    ENTRYOP,         /* pentry */
    EQUOP,           /* pequ */
    EVENOP,          /* peven */
    EXPORTOP,        /* pexport */
    FAILOP,          /* pfail */
    FCBOP,           /* pfcb */
    FCCOP,           /* pfcc */
    FDBOP,           /* pfdb */
    FQBOP,           /* pfqb */
    GETOP,           /* pget */
    GLOBLOP,         /* pglobl */
    IDENTOP,         /* pident */
    IMPORTOP,        /* pimport */
    LCOMMOP,         /* plcomm */
    LCOMMOP1,        /* plcomm1 */
    LISTOP,          /* plist */
    LOCOP,           /* ploc */
    MACLISTOP,       /* pmaclist */
    MACROOP,         /* pmacro */
    MAPOP,           /* pmap */
    ORGOP,           /* porg */
    PROCEOFOP,       /* pproceof */
    RMBOP,           /* prmb */
    SECTOP,          /* psect */
    SETOP,           /* pset */
    SETDPOP,         /* psetdp */
    TEXTOP,          /* ptext */
#ifdef I80386
    USE16OP,         /* puse16 */
    USE32OP,         /* puse32 */
#endif
    WARNOP,          /* pwarn */

/* Machine-op routine numbers. */
#ifdef I80386
    BCC,             /* mbcc */
    BSWAP,           /* mbswap */
    CALL,            /* mcall */
    CALLI,           /* mcalli */
    DIVMUL,          /* mdivmul */
    ENTER,           /* menter */
    EwGw,            /* mEwGw */
    ExGx,            /* mExGx */
    F_INHER,         /* mf_inher */
    F_M,             /* mf_m */
    F_M2,            /* mf_m2 */
    F_M2_AX,         /* mf_m2_ax */
    F_M2_M4,         /* mf_m2_m4 */
    F_M2_M4_M8,      /* mf_m2_m4_m8 */
    F_M4_M8_OPTST,   /* mf_m4_m8_optst */
    F_M4_M8_ST,      /* mf_m4_m8_st */
    F_M4_M8_STST,    /* mf_m4_m8_stst */
    F_M4_M8_M10_ST,  /* mf_m4_m8_m10_st */
    F_M10,           /* mf_m10 */
    F_OPTST,         /* mf_optst */
    F_ST,            /* mf_st */
    F_STST,          /* mf_stst */
    F_W_INHER,       /* mf_w_inher */
    F_W_M,           /* mf_w_m */
    F_W_M2,          /* mf_w_m2 */
    F_W_M2_AX,       /* mf_w_m2_ax */
    GROUP1,          /* mgroup1 */
    GROUP2,          /* mgroup2 */
    GROUP6,          /* mgroup6 */
    GROUP7,          /* mgroup7 */
    GROUP8,          /* mgroup8 */
    GvEv,            /* mGvEv */
    GvMa,            /* mGvMa */
    GvMp,            /* mGvMp */
    IMUL,            /* mimul */
    IN,              /* min */
    INCDEC,          /* mincdec */
    INHER,           /* minher */
    INHER16,         /* minher16 */
    INHER32,         /* minher32 */
    INHER_A,         /* minhera */
    INT,             /* mint */
    JCC,             /* mjcc */
    JCXZ,            /* mjcxz */
    LEA,             /* mlea */
    MOV,             /* mmov */
    MOVX,            /* mmovx */
    NEGNOT,          /* mnegnot */
    OUT,             /* mout */
    PUSHPOP,         /* mpushpop */
    RET,             /* mret */
    SEG,             /* mseg */
    SETCC,           /* msetcc */
    SH_DOUBLE,       /* mshdouble */
    TEST,            /* mtest */
    XCHG,            /* mxchg */
#endif /* I80386 */

    LASTENUMVAL2  /* Pacify ACK ANSI C compiler 1.202 warning: unexpected trailing comma in enumerator pack. */
};

/* Special opcodes. */
#ifdef I80386
# define CMP_OPCODE_BASE	0x38
# define CMPSB_OPCODE		0xA6
# define CMPSW_OPCODE		0xA7
# define ESCAPE_OPCODE_BASE	0xD8
# define FST_ENCODED		0x12
# define FSTP_ENCODED		0x13
# define JMP_OPCODE		0xE9
# define JMP_SHORT_OPCODE	0xEB
# define JSR_OPCODE		0xE8
# define MOVSB_OPCODE		0xA4
# define MOVSW_OPCODE		0xA5
# define PAGE1_OPCODE		0x0F
# define POP_OPCODE 		0x8F
# define PUSH_OPCODE 		0xFF
# define WAIT_OPCODE		0x9B
#endif
