/* Callable system calls */

#include <exec/types.h>

#define VR_D0 1
#define VR_A0 2
#define VR_D1 3
#define VR_A1 4

struct SysCall
{
	const char *name;
	const char *library;
	LONG offset;
	char args[12];
};

#define NUMCALLS 5
	
class SysCalls
{
	static const SysCall calls[NUMCALLS] = {
		{ "Close", "dos.library", -36, { VR_D1, 0 } },
		{ "Open", "dos.library", -30, { VR_D1, VR_D2, 0 } },
		{ "PutStr", "dos.library", -948, { VR_D1, 0 } },
		{ "Read", "dos.library", -42, { VR_D1, VR_D2, VR_D3, 0 } },
		{ "Write", "dos.library", -48, { VR_D1, VR_D2, VR_D3, 0 }
	};

	public:

	SysCall* find(char *name);
};
  	