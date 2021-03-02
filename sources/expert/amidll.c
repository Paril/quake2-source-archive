
#ifdef AMIGA

extern int exe_found;

#define NO_REMOVE_HEADER 1

#include "g_local.h"

#undef NO_REMOVE_HEADER

#include "dll.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>

extern game_export_t *GetGameAPI (game_import_t *import);

#ifdef AMIGA
void SetExeName(char *name)
{
	exe_found=1;
}
#endif

void* __saveds dllFindResource(int id, char *pType)
{
    return NULL;
}

void* __saveds dllLoadResource(void *pHandle)
{
    return NULL;
}

void __saveds dllFreeResource(void *pHandle)
{
    return;
}

ULONG SegList;

dll_tExportSymbol DLL_ExportSymbols[]=
{
    {dllFindResource,"dllFindResource"},
    {dllLoadResource,"dllLoadResource"},
    {dllFreeResource,"dllFreeResource"},
   {(void *)GetGameAPI,"GetGameAPI"},
   {(void *)SetExeName,"SetExeName"},
   {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[]=
{
	{0,0,0,0}
};

int __saveds DLL_Init(void)
{
  	struct CommandLineInterface *pCLI = Cli();
    if (!pCLI) return 0;
    SegList = (ULONG)(pCLI->cli_Module);
    return 1L;
}

void __saveds DLL_DeInit(void)
{
}
#endif
