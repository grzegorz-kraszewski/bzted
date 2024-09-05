#include "compiler.h"

#include <proto/exec.h>
#include <proto/dos.h>

extern Compiler *Comp;


const char LastChar(const char *s)
{
	if (!*s) return 0x00;
	while (*s) s++;
	return *(--s);
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::append(char *buf, LONG len)
{
	int newsize = textsize + len;
	char *newtext = new char[newsize + 1];

	if (newtext)
	{
		if (text)
		{
			CopyMem(text, newtext, textsize);
			delete text;
		}

		CopyMem(buf, newtext + textsize, len);
		newtext[textsize + len] = 0x00;
		textsize = newsize;
		text = newtext;
		return TRUE;
	}

	return FALSE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseDecNumber()
{
	LONG val = 0;
	BOOL leadzero = TRUE;
	BOOL positive = TRUE;
	char c, *p = text;

	if (*p == '+') p++;                          // ignore leading '+'
	if (*p == '-') { positive = FALSE; p++; }

	while (c = *p++)
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		if ((c >= '0') && (c <= '9')) c -= '0';
		else FAIL("Unexpected character in decimal number");

		if (positive)
		{
			if ((val < 214748364) || ((val == 214748364) && (c < 8)))
			{
				val *= 10;
				val += c;
			}
			else FAIL("Decimal number out of 32-bit range");
		}
		else
		{
			if ((val > -214748364) || ((val == -214748364) && (c < 9)))
			{
				val *= 10;
				val -= c;
			}
			else FAIL("Decimal number out of 32-bit range");
		}
	}

	intval = val;
	type = MAKE_ID3('i','n','t');
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseHexNumber()
{
	LONG val = 0;
	WORD digit = 0;
	BOOL leadzero = TRUE;
	char c, *p = text + 1;      // skip '$' prefix

	if (!*p) FAIL("No hexadecimal digits after '$' prefix");

	while ((c = *p++) && (digit < 8))
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		val <<= 4;
		if ((c >= '0') && (c <= '9')) c -= '0';
		else if ((c >= 'A') && (c <= 'F')) c -= 55;
		else if ((c >= 'a') && (c <= 'f')) c -= 87;
		else FAIL("Unexpected character in hexadecimal number");
		val += c;
		digit++;			
	};

	if (c) FAIL("Hexadecimal number too big");

	intval = val;
	type = MAKE_ID3('i','n','t');
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseBinNumber()
{
	LONG val = 0;
	WORD digit = 0;
	BOOL leadzero = TRUE;
	char c, *p = text + 1;      // skip '%' prefix

	if (!*p) FAIL("No binary digits after '%' prefix");

	while ((c = *p++) && (digit < 32))
	{
		if (leadzero && (c == '0')) continue;
		else leadzero = FALSE;
		val <<= 1;
		if (c == '1') val |= 1;
		else if (c != '0') FAIL("Unexpected character in binary number");
		digit++;			
	};

	if (c) FAIL("Binary number too big");

	intval = val;
	type = MAKE_ID3('i','n','t');
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseString()
{
	char *p = text;
	char delimiter = *p;
	char *m = p + 1;

	while (*m != delimiter) *p++ = *m++;
	*p = 0x00;
	textsize -= 2;

	type = MAKE_ID3('s','t','r');
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseIdentifier()
{
	Operator *op;
	Function *fu;

	// user functions can redefine operators
	//--------------------------------------

    if (fu = Comp->functions.find(text))
	{
		type = TT_FNC;	
	}

	// Operators before definitions, so operator ':' 
	// is not taken as definition with empty name.
	//-----------------------------------------------

	if (op = Comp->operators.find(text))
	{
		type = TT_OPR;
		return TRUE;
	}

	if (LastChar(text) == ':') return parseDefinition();

	type = TT_IDN;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::parseDefinition()
{
	char *p = text;

	while (*p) p++;
	*--p = 0x00;
	textsize -= 1;
	type = TT_DEF;
	return TRUE;
}

/*-------------------------------------------------------------------------------------------*/

BOOL Token::compile(Function *function)
{
	BOOL success = TRUE;
	switch (type)
	{
		case TT_INT:
		{
			Operand op1 = { IIOP_IMMEDIATE, intval };
			Operand op2 = { IIOP_REGISTER, II_D + 0 };
			InterInstruction *ii0 = new InterInstruction(II_MOVE, op1, op2);
			InterInstruction *ii1 = new InterInstruction(II_DROP, op2);

			if (ii0 && ii1)
			{
				function->addCode(ii0);
				function->addCode(ii1);
			}
			else success = FALSE;
		}
		break;

		case TT_OPR:
			success = Comp->operators.find(text)->generator(function);
		break;

		case TT_FNC:
		{
			InterInstruction *ii = new InterInstruction(II_JSBR, text);
			if (ii) function->addCode(ii);
			else success = FALSE;
		}
		break;

		default:
			success = FALSE;		
	}

	return success;
}

/*-------------------------------------------------------------------------------------------*/

void Token::error(char *msg)
{
	Printf("%s in line %ld.\n", msg, linenum);
}