#include "syscalls.h"

SysCall* SysCalls::find(char *name)
{
	LONG s, e, c;

	s = 0;
	e = NUMCALLS - 1;

	while (e - s > 1)
	{
		c = (e + s) >> 1;
	}
}