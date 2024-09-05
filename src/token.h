#include "function.h"

#define TT_INT MAKE_ID3('i','n','t')
#define TT_STR MAKE_ID3('s','t','r')
#define TT_OPR MAKE_ID3('o','p','r')
#define TT_FNC MAKE_ID3('f','n','c')
#define TT_SYS MAKE_ID3('s','y','s')
#define TT_DEF MAKE_ID3('d','e','f')
#define TT_IDN MAKE_ID3('i','d','n')

#define FAIL(m) { error(m); return FALSE; }

class Token
{
	public:

	Token *succ;
	Token *pred;
	char *text;
	LONG type;
	LONG linenum;
	LONG textsize;
	LONG intval;            /* valid for <int> */

	Token()
	{
		succ = NULL;
		pred = NULL;
		text = NULL;
		type = 0;
		linenum = 0;
		textsize = 0;
	}

	BOOL append(char *buf, LONG len);
	BOOL parseDecNumber();
	BOOL parseHexNumber();
	BOOL parseBinNumber();
	BOOL parseString();
	BOOL parseIdentifier();
	BOOL parseDefinition();
	BOOL compile(Function *function);
	void error(char *msg);
	Token* next() {	return succ->succ ? succ : NULL; }

};
