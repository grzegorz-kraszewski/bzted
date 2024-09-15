//-------------------
// source code token
//-------------------

#ifndef BZTED_TOKEN_H
#define BZTED_TOKEN_H

#include "function.h"

#define TT_INT MAKE_ID3('i','n','t')
#define TT_STR MAKE_ID3('s','t','r')
#define TT_OPR MAKE_ID3('o','p','r')
#define TT_FNC MAKE_ID3('f','n','c')
#define TT_SYS MAKE_ID3('s','y','s')
#define TT_DEF MAKE_ID3('d','e','f')
#define TT_IDN MAKE_ID3('i','d','n')

class Token : public RpNode<Token>
{
	public:

	char *text;
	int textSize;
	int type;
	int lineNum;
	int intValue;            /* valid for <int> */

	Token()
	{
		text = NULL;
		type = 0;
		lineNum = 0;
		textSize = 0;
	}

	bool append(char *buf, LONG len);
	bool parseDecNumber();
	bool parseHexNumber();
	bool parseBinNumber();
	bool parseString();
	bool parseIdentifier();
	bool parseDefinition();
	bool translate(Function *f);
	void print();
};

#endif  /* BZTED_TOKEN_H */