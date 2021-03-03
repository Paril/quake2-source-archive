/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_dll.win32.c,v $
 *   $Revision: 1.4 $
 *   $Date: 2002/07/23 22:48:27 $
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

#include "g_local.h"
#include "g_cmds.h"
#include "g_dll.h"
#include "u_findfunc.h"
#include "u_entmgr.h"

// g_dll.win32.c
// D-Day: Normandy DLL interaction code
// WIN32
/*
  int LoadUserDLLs(char *)

  This function takes the name of the ini file that it is to open and read.
  The file contains entries in it that are built the following way -

  filename  {entrypoint_name|entrypoint_index} version [message_digest]
*/

userdll_list_node_t *LoadUserDLLs(edict_t *ent, int team)
{
    //char buffer[512];
    //char libname[128];
    char entryname[256];
    //char MD5Sig[64];
    //char version[32];
    //char *ptr, *tmp;
    int  i;
    userdll_list_node_t *unode;
	qboolean IsDLLName=false;

	// the "team" var is unused since its really a linux-only element

	if(!ent->pathtarget)
	{//if no dll name, use generic usa
		if(!(ent->pathtarget=gi.TagMalloc(sizeof("usa\0"), TAG_GAME))) return NULL;
		strcpy(ent->pathtarget,"usa\0");
		IsDLLName=true;
	}

	if(!(unode = (userdll_list_node_t*)
		gi.TagMalloc(sizeof(userdll_list_node_t), TAG_GAME)))
	{
		gi.dprintf("memory allocation failed for library <%s>\n",ent->pathtarget);
		return NULL;
	}

	strncpy(unode->libname,(GAMEVERSION "/"),sizeof((GAMEVERSION "/")));
    strcat(unode->libname,ent->pathtarget);

#if defined __unix__
	strcat(unode->libname,"i386.so"); //follow suit with gamex86.dll
#else
	strcat(unode->libname,"x86"); //follow suit with gamex86.dll
#endif

    strncpy(entryname,ent->pathtarget,sizeof(ent->pathtarget));
	if(IsDLLName) 
		gi.TagFree(ent->pathtarget);

	for(i=0;i<=strlen(entryname);i++) entryname[i]=toupper(entryname[i]);
	strcat(entryname,"GetAPI");
	strncpy(unode->entryname,entryname,sizeof(entryname));
    strncpy(unode->version,"1",3);
    strncpy(unode->MD5Sig,"1",3);

    unode->next = GlobalUserDLLList;
    GlobalUserDLLList = unode;
		

	unode = GlobalUserDLLList;
//print out the results
#ifdef DEBUG
	gi.dprintf("+= list_node\n");
    gi.dprintf(" |-- library <%s>\n",unode->libname);
    gi.dprintf(" |-- entry   <%s>\n",unode->entryname);
    gi.dprintf(" |-- md5     <%s>\n\n",unode->MD5Sig);
#endif // DEBUG
	//this is the system dependent portion of the code. Currently it is set up
//for windows. This portion would be different for Linux.

#ifdef __unix__ 
	unode->hDLL = dlopen(unode->libname, RTLD_NOW);
#else
	unode->hDLL = LoadLibrary(unode->libname);
#endif

    if(unode->hDLL == NULL)
	{
#ifdef __unix__
		gi.dprintf("Couldn't load library %s, errorcode = %d\n",unode->libname,dlerror());
#else
		gi.dprintf("Couldn't load library %s, errorcode = %d\n",unode->libname,GetLastError());
#endif
		return NULL;
	}
    else
	{
		unode->EntryPoint = (userdll_export_t (*)(userdll_import_t))
			GetProcAddress(unode->hDLL,unode->entryname);
        if(unode->EntryPoint == NULL)
		{
			gi.dprintf("Could not get entry point %s for library %s\n",
				unode->entryname,unode->libname);
			return NULL;
		}
	}
     
	return GlobalUserDLLList;
}



void (*FindGameFunction(char *t));

int InitializeUserDLLs(userdll_list_node_t *unode,int teamindex)
{
        
        int numLibsInit, version;
		userdll_import_t  UserDLLImports;

        numLibsInit = 1;
		if(!unode) return 0;

        //set up the UserDLLImports
        UserDLLImports.game = &game;
        UserDLLImports.level = &level;
        UserDLLImports.gi = &gi;
        UserDLLImports.globals = &globals;

        UserDLLImports.InsertCommands = InsertCmds;
		UserDLLImports.FindFunction = FindGameFunction;
		UserDLLImports.InsertItem = InsertItem;
		UserDLLImports.InsertEntity = InsertEntity;
		UserDLLImports.RemoveEntity = RemoveEntity;
		UserDLLImports.g_edicts=g_edicts;
		UserDLLImports.is_silenced=&is_silenced;
//		UserDLLImports.scope_setting=&scope_setting->value;
		UserDLLImports.team_index=teamindex;

#ifdef DEBUG
        //ok, run through the list of libraries and get their export structures
		gi.dprintf("+= initialize\n");
        gi.dprintf(" |-- library <%s>\n",unode->libname);
        gi.dprintf(" |-- entry   <%s>\n",unode->entryname);
        gi.dprintf(" |-- md5     <%s>\n",unode->MD5Sig);
#endif //DEBUG
		version=atoi(unode->version);

		unode->dll_funcs = (unode->EntryPoint)(UserDLLImports);
#ifdef DEBUG
        gi.dprintf("  |-- [%s]\n",unode->dll_funcs.creator);
        gi.dprintf("  |-- dll_funcs.apiversion = %d\n",unode->dll_funcs.apiversion);
		gi.dprintf("  |--       unode->version = %d\n",version);
#endif //DEBUG
        if(unode->dll_funcs.apiversion != version)
			gi.dprintf("Library %s has invalid version %d\n",
				unode->libname, unode->dll_funcs.apiversion);
        else
		{
			//ok, we have a valid api, call initialization function.
				//gi.dprintf("Library has valid version\n");
				if(!unode->dll_funcs.UserDLLInit)
					gi.dprintf("Could not initialize library %s\n",unode->libname);
                else
				{
					numLibsInit++;
					unode->dll_funcs.UserDLLInit();  //whew, finally initialize the library
					InitMOS_List(team_list[teamindex],unode->dll_funcs.mos_list);

					strcpy (team_list[teamindex]->playermodel, unode->dll_funcs.playermodel);
					strcpy (team_list[teamindex]->teamid, unode->dll_funcs.teamid);
				}
                        
		}

		SetItemNames (); // do this again so we can get the team item strings loaded into CS_ITEMS
        
        return numLibsInit;
}

void ClearUserDLLs()
{
        userdll_list_node_t *unode, *tmp;

        gi.dprintf("Clearing user DLLs\n");
        unode = GlobalUserDLLList;
        while(unode)
        {
                //this should close the reference to the dll
                if(unode->hDLL)
#ifdef __unix__
						dlclose(unode->hDLL);
#else
                        FreeLibrary(unode->hDLL);
#endif

                tmp = unode;
                unode = unode->next;
                gi.TagFree(tmp);
        }
}




void LevelStartUserDLLs(edict_t *ent)
{
        userdll_list_node_t *unode;

        unode = GlobalUserDLLList;
        while(unode)
        {
			if(unode->hDLL) unode->dll_funcs.UserDLLStartLevel(ent);
            unode = unode->next;
        }
}

void LevelExitUserDLLs()
{
     userdll_list_node_t *unode;

     unode = GlobalUserDLLList;
     while(unode)
     {
        unode->dll_funcs.UserDLLLeaveLevel();
        unode = unode->next;
     }

	//ClearUserDLLs();
}


void PlayerSpawnUserDLLs(edict_t *ent)
{
        userdll_list_node_t *unode;

        unode = GlobalUserDLLList;
        while(unode)
        {
        unode->dll_funcs.UserDLLPlayerSpawns(ent);
                unode = unode->next;
        }
}

void PlayerDiesUserDLLs(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        userdll_list_node_t *unode;

        unode = GlobalUserDLLList;
        while(unode)
        {
        unode->dll_funcs.UserDLLPlayerDies (self, inflictor, attacker, damage, point);
        
                unode = unode->next;
        }
}
