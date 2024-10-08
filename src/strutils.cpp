/* Untangle: string utilities */

#include <proto/exec.h>
#include <exec/memory.h>

#include "strutils.h"

extern Library *SysBase;

//---------------------------------------------------------------------------------------------

static void ProcPutChar()
{
	asm("move.b d0,(a3)+");
}

//---------------------------------------------------------------------------------------------

static void ProcCountChars()
{
	asm("addq.l #1,(a3)");
}

//---------------------------------------------------------------------------------------------

int VFmtLen(const char *fmt, int *args)
{
	int len = 0;

	RawDoFmt(fmt, args, ProcCountChars, &len);
	return len;
}

//---------------------------------------------------------------------------------------------

void VFmtPut(char *dest, const char *fmt, int *args)
{
	RawDoFmt(fmt, args, ProcPutChar, dest);
}

//---------------------------------------------------------------------------------------------

void FmtPut(char *dest, const char *fmt, ...)
{
	int *_args = (int*)&fmt + 1;
	VFmtPut(dest, fmt, _args);
}

//---------------------------------------------------------------------------------------------

char* VFmtNew(const char *fmt, int *args)
{
	int len;
	char *dest;

	len = VFmtLen(fmt, args) + 1;

	if (dest = new char[len]) VFmtPut(dest, fmt, args);
	return dest;
}

//---------------------------------------------------------------------------------------------

char* FmtNew(const char *fmt, ...)
{
	int *_args = (int*)&fmt + 1;
	return VFmtNew(fmt, _args);
}

//---------------------------------------------------------------------------------------------
	
int StrLen(const char *s)
{
	const char *v = s;

	while (*v) v++;
	return (int)(v - s);
}

//---------------------------------------------------------------------------------------------

char* StrClone(const char *s)
{
	char *d;

	if (d = new char[StrLen(s) + 1]) StrCopy(s, d);
	return d;
}

//---------------------------------------------------------------------------------------------

char* StrCopy(const char *s, char *d)
{
	while (*d++ = *s++);
	return (--d);
}

//---------------------------------------------------------------------------------------------

int StrCmp(const char *a, const char *b)
{
	while(*a && (*a == *b)) { a++; b++; }
	return (*(const unsigned char*)a - *(const unsigned char*)b);
}

//---------------------------------------------------------------------------------------------

int IsSpace(char c)
{
	return ((c == 0x20) || (c == 0x09) || (c == 0x0A) || (c == 0x0D));
}

//---------------------------------------------------------------------------------------------

int IsDigit(char c)
{
	return ((c >= '0') && (c <= '9'));	
}

//---------------------------------------------------------------------------------------------

bool IsInString(char c, char *s)
{
	char x;

	while (x = *s++) if (x == c) break;
	return x;
}

//---------------------------------------------------------------------------------------------
