/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/amidll.c,v $
 *   $Revision: 1.1 $
 *   $Date: 2002/07/23 21:11:37 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef AMIGA

#define NO_REMOVE_HEADER 1

#include "g_local.h"

#undef NO_REMOVE_HEADER

#include "dll.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>

extern game_export_t *GetGameAPI (game_import_t *import);

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
