/* string utilities */

#include <proto/exec.h>
#include <exec/memory.h>

int VFmtLen(const char *fmt, int *args);
void VFmtPut(char *dest, const char *fmt, int *args);
void FmtPut(char *dest, const char *fmt, ...);
char* VFmtNew(const char *fmt, int *args);
char* FmtNew(const char *fmt, ...);
int StrLen(const char *s);
char* StrClone(const char *s);
char* StrCopy(const char *s, char *d);
int StrCmp(const char *a, const char *b);
int IsSpace(char c);
int IsDigit(char c);
bool IsInString(char c, char *s);
