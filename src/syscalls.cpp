#include "strutils.h"
#include "compiler.h"


//------------------------
// Must be sorted by key!
//------------------------

struct KeyedPair<SysCall> SysCalls[23] = {
	{"Close", {"dos.library", -36, "d1", "d0", 34}},
	{"CreateDir", {"dos.library", -120, "d1", "d0", 34}},
	{"CurrentDir", {"dos.library", -126, "d1", "d0", 34}},
	{"DeleteFile", {"dos.library", -72, "d1", "d0", 34}},
	{"DupLock", {"dos.library", -96, "d1", "d0", 34}},
	{"Examine", {"dos.library", -102, "d1d2", "d0", 34}},
	{"ExNext", {"dos.library", -108, "d1d2", "d0", 34}},
	{"Info", {"dos.library", -114, "d1d2", "d0", 34}},
	{"Input", {"dos.library", -54, "", "d0", 34}},
	{"IoErr", {"dos.library", -132, "", "d0", 34}},
	{"Lock", {"dos.library", -84, "d1d2", "d0", 34}},
	{"Open", {"dos.library", -30, "d1d2", "d0", 34}},
	{"Output", {"dos.library", -60, "", "d0", 34}},
	{"PrintFault", {"dos.library", -474, "d1d2", "d0", 36}},
	{"Printf", {"dos.library", -954, "d1d2", "d0", 36}},
	{"PutStr", {"dos.library", -948, "d1", "d0", 36}}, 
	{"Read", {"dos.library", -42, "d1d2d3", "d0", 34}},
	{"Rename", {"dos.library", -78, "d1d2", "d0", 34}},
	{"SDivMod32", {"utility.library", -150, "d0d1", "d0d1", 36}},
	{"SMult32", {"utility.library", -138, "d0d1", "d0", 36}},
	{"Seek", {"dos.library", -66, "d1d2d3", "d0", 34}},
	{"UnLock", {"dos.library", -90, "d1", "", 34}},
	{"Write", {"dos.library", -48, "d1d2d3", "d0", 34}}
};
  

//------------------------
// Must be sorted by key!
//------------------------

struct KeyedPair<const char*> BaseNames[3] = {
	{ "dos.library", "DOSBase" },
	{ "exec.library", "SysBase" },
	{ "utility.library", "UtilityBase" }
};

//---------------------------------------------------------------------------------------------

bool GenerateSysCall(const char *name, Function *f)
{
	if (SysCall *sc = Comp->findSysCall(name))
	{
		if (Comp->addSysLibrary(sc->library, sc->minLibVersion))
		{
			const char *baseName = Comp->getLibraryBaseName(sc->library);	
			Operand bname(IIOP_LABEL, (int)baseName);
			Operand rega6(IIOP_ADDRREG, 6);
			Operand jump(IIOP_SYSJUMP, (int)name);
			InterInstruction *ii0 = new InterInstruction(II_MOVE, bname, rega6);
			InterInstruction *ii1 = new InterInstruction(II_JSBR, jump);

			if (ii0 && ii1)
			{
				f->addCode(ii0);
				f->addCode(ii1);
				Comp->useSysCall(name, sc->library, sc->offset);
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//---------------------------------------------------------------------------------------------

LibraryToOpen::LibraryToOpen(const char *libname, int minver)
{
	name = libname;
	minVersion = minver;
	baseName = Comp->getLibraryBaseName(libname);
}
