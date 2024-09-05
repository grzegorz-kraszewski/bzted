#include "main.h"
#include "function.h"

/* identifiers of types */

#define BTYPE_INT      MAKE_ID3('i', '3', '2')
#define BTYPE_WORD     MAKE_ID3('i', '1', '6')    /* arrays only */
#define BTYPE_BYTE     MAKE_ID3('i', '8', 0x00)   /* arrays only */
#define BTYPE_UINT     MAKE_ID3('u', '3', '2')    /* arrays only */
#define BTYPE_UWORD    MAKE_ID3('u', '1', '6')    /* arrays only */
#define BTYPE_UBYTE    MAKE_ID3('u', '8', 0x00)   /* arrays only */
#define BTYPE_FLOAT    MAKE_ID3('f', '3', '2')    /* future */
#define BTYPE_DOUBLE   MAKE_ID3('f', '6', '4')    /* future */
#define BTYPE_STR      MAKE_ID3('s', 't', 'r')    /* nullterminated [UTF-8] string */

/* identifiers of registers for M68k */

#define BREG_D0        0
#define BREG_D1        1
#define BREG_D2        2
#define BREG_D3        3
#define BREG_D4        4
#define BREG_D5        5
#define BREG_D6        6
#define BREG_D7        7
#define BREG_A0        8
#define BREG_A1        9
#define BREG_A2       10
#define BREG_A3       11
#define BREG_A4       12
#define BREG_A5       13
#define BREG_A6       14
#define BREG_A7       15
#define BREG_Dn       16   /* any data register */
#define BREG_An       17   /* any address register */

/* flags for operators and functions */

#define BFLAG_COMMUTATIVE  /* inputs may be swapped around if more than one */

/* function input and output descriptor */

struct FuncPin
{
	LONG type;
	LONG reg;
};

/* built-in function descriptor */

struct Operator
{
	LONG numInputs;
	LONG numOutputs;
	LONG flags;
	const FuncPin *inputs;
	const FuncPin *outputs;
	BOOL (*generator)(Function *function);
};
