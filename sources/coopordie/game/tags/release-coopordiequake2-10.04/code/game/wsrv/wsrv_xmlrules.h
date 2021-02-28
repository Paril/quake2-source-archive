/*
Copyright (C) 2001-2009 Pat AfterMoon (www.aftermoon.net)

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

// xmlrules.h -- primary header for XMLRULES management

#include "domc.h"

#ifdef DEBUG_XMLRULES

#define qboolean            int
#define false               0
#define true                1
#define SIZE_TAB( Tab )  ( sizeof(Tab) / sizeof(Tab[0]) )

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

#endif

#define rnd(vmax)	((float)(((((float)rand()) / ((float)RAND_MAX))*(vmax))))

// *** CONSTANT ***

#define ENTITY_MAX_KEY  16

// *** TYPE ***

typedef enum { XMLRULES_Exec, XMLRULES_Continue, XMLRULES_Break, XMLRULES_Exit } t_XMLRULES_Status ; 

typedef struct 
{
	char    szTagName[16];
	t_XMLRULES_Status (*pfunc_Begin)(DOM_Node *pNode, void *pParam) ;
	t_XMLRULES_Status (*pfunc_End)(DOM_Node *pNode, void *pParam) ;
	qboolean    bBreak;
} t_XMLRULES_TabFunc;

typedef struct 
{ 
	int     nbkey ;
	char    name[ENTITY_MAX_KEY][256]; 
	char    value[ENTITY_MAX_KEY][256]; 
} t_key ;

// *** FUNCTION ***

#ifdef DEBUG_XMLRULES
extern char *ED_ParseEdict (char *data, t_key *pkey);
typedef     void        edict_t ;
#define     MAX_OSPATH  256
#endif

extern qboolean XMLRULES_CreateDOM();
extern qboolean XMLRULES_DestroyDOM();
extern qboolean XMLRULES_entity_change(t_key *pkey);
extern qboolean XMLRULES_entity_add(edict_t **ent, int *inhibit);


