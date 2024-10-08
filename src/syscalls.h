/* Callable system calls */

#include "keyedarray.h"
#include "rplist.h"

#include <exec/types.h>

struct SysCall
{
	const char *library;
	int offset;
	const char *arguments;
	const char *results;
	int minLibVersion;
};

extern struct KeyedPair<SysCall> SysCalls[];
extern struct KeyedPair<const char*> BaseNames[];

bool GenerateSysCall(const char *name, Function *f);


class LibraryToOpen : public RpNode<LibraryToOpen>
{
	const char *baseName;
	int minVersion;

	public:
	
	const char *name;
	
	LibraryToOpen(const char* libname, int minver);
	void bumpVersion(int minver) { if (minver > minVersion) minVersion = minver; }
	const char* getBase() { return baseName; }
	int getVersion() { return minVersion; }
};
