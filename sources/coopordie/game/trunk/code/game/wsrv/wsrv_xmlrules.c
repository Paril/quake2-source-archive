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
// g_xmlrules.c -- XMLRULES management


#include "../g_local.h"
void printf_null(const char *s, ...) {} ;

#define TRACE_printf printf_null
// Enable this line for debug
//#define TRACE_printf Com_Printf

#include "wsrv_xmlrules.h"
#include "wsrv.h"
#include "wsrv_rpc.h"

#include <locale.h>

extern qboolean ParseEntities(char *mapname, char **pentities, edict_t **pent, int *inhibit);

char *strnspace(int i) 
{ 
	static char sTmp[128]; 
	int         ii;
	for ( ii=0 ; ii<i ; ii++ )
		sTmp[ii*2] = sTmp[1+ii*2] = ' ' ;
	sTmp[ii*2] = 0;
	return sTmp ;
}

int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

//-----------------------------------------------------------------------------
static DOM_Document *pXMLRULES_doc           = NULL;
static DOM_NodeList *pXMLRULES_entity_change = NULL;
static DOM_NodeList *pXMLRULES_entity_add    = NULL;
static edict_t      *pXMLRULES_ent ;
static int          pXMLRULES_inhibit ;
//-----------------------------------------------------------------------------
#define MAX_LEVEL_SWITCH    8
static int iLevel_switch = 0 ;
typedef struct 
{
	DOM_Node    *pParam1;
	float       fVal;
} t_param_switch ;
static t_param_switch ParamSwitch[MAX_LEVEL_SWITCH];
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Check a condition with a keyname and a keyvalue.
// return XMLRULES_Exec if the condition result is true, else return XMLRULES_Continue
//-----------------------------------------------------------------------------
static t_XMLRULES_Status Check1Condition(const char *keyname, const char *keyvalue, DOM_Node *pOp, t_key *pKey)
{
	t_XMLRULES_Status Status = XMLRULES_Continue;
	int i ;

	// Try all key for keyname
	for ( i = 0 ; i < pKey->nbkey ; i++ )
	{
		if ( strcmp(pKey->name[i], keyname)==0 )
		{
			// if keyname is found, test the keyvalue with the right operand
			if ( !pOp || strcmp(pOp->nodeValue, "equ")==0 )
			{
				if ( strcmp(pKey->value[i], keyvalue)==0 )
				{
					Status = XMLRULES_Exec ;
				}
			}
			else if ( strcmp(pOp->nodeValue, "noequ")==0 )
			{
				if ( strcmp(pKey->value[i], keyvalue)!=0 )
				{
					Status = XMLRULES_Exec ;
				}
			}
			else if ( strcmp(pOp->nodeValue, "strstr")==0 )
			{
				if ( strstr(pKey->value[i], keyvalue) )
				{
					Status = XMLRULES_Exec ;
				}
			}
			else if ( strcmp(pOp->nodeValue, "nostrstr")==0 )
			{
				if ( !strstr(pKey->value[i], keyvalue) )
				{
					Status = XMLRULES_Exec ;
				}
			}
			break;
		}
	}

	// If the key name wasn't found, check the condition "if key isn't defined".
	// ex: <if keyname="item" keyvalue="">
	if (   i >= pKey->nbkey 
		&& strcmp(keyvalue, "")==0 
		&& (!pOp || strcmp(pOp->nodeValue, "equ")==0) )
	{
		Status = XMLRULES_Exec ;
	}

	return Status;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status CheckCondition(DOM_Node *pParam1, DOM_Node *pParam2, DOM_Node *pOp, t_key *pKey)
{
	int                 i;
	char                szValue[256] ;
	char                *pszValue ;
	t_XMLRULES_Status   Status ;

	TRACE_printf( " %s=\"%s\" %s=\"%s\"", 
			pParam1->nodeName, pParam1->nodeValue, 
			pParam2->nodeName, pParam2->nodeValue ) ;
	if ( pOp )
		TRACE_printf(" %s=\"%s\"", pOp->nodeName, pOp->nodeValue);

	// Key condition : equ, strstr
	if ( strcmp(pParam1->nodeName, "keyname")==0 )
	{
		if ( strcmp(pParam2->nodeName, "keyvalue")==0 )
		{
			return Check1Condition(pParam1->nodeValue, pParam2->nodeValue, pOp, pKey) ;
		}
		else if ( strcmp(pParam2->nodeName, "keyvalues")==0 )
		{
			pszValue = pParam2->nodeValue ;
			i = 0 ;
			while ( *pszValue )
			{
				szValue[i++] = *(pszValue++) ;
				if ( i >= sizeof(szValue) )
				{
					Com_Printf("XMLRULES : CheckCondition too long\n");
					break ;
				}

				if ( (*pszValue == ',') || (*pszValue == '\0') )
				{
					szValue[i] = '\0' ;
					i = 0 ;
					Status = Check1Condition(pParam1->nodeValue, szValue, pOp, pKey) ;
					if ( Status == XMLRULES_Exec || !*pszValue )
						return Status ;
					pszValue++ ;
				}
			}
		}
	}
	// Random condition
	if ( strcmp(pParam1->nodeName, "random")==0 )
	{
		float   fVal = (float)atof(pParam2->nodeValue) ;
		float   fRnd = ParamSwitch[iLevel_switch].fVal ;

		ParamSwitch[iLevel_switch].fVal -= fVal ;
		
		if ( (0 <= fRnd) && (fRnd < fVal) )
			return XMLRULES_Exec ;
		else
			return XMLRULES_Continue ;
	}

	return XMLRULES_Continue ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_switch(DOM_Node *pNode, void *pParam)
{
	NodeEntry   *pNodeEntry;

	iLevel_switch++ ;
	ParamSwitch[iLevel_switch].pParam1  = NULL ;

	if ( pNode->attributes )
	{
		pNodeEntry = pNode->attributes->first ;
		if ( pNodeEntry && pNodeEntry->node )
		{
			ParamSwitch[iLevel_switch].pParam1 = pNodeEntry->node ;
			if ( pNodeEntry->next )
				Com_Printf("XMLRULES : switch with more than 1 param\n");

			if ( strcmp(pNodeEntry->node->nodeName, "random")==0 )
			{
				ParamSwitch[iLevel_switch].fVal = rnd(atof(pNodeEntry->node->nodeValue)) ;
				TRACE_printf(" rnd(%0.2f)=%0.2f", atof(pNodeEntry->node->nodeValue), ParamSwitch[iLevel_switch].fVal);
			}
		}
	}

	return XMLRULES_Exec;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_switch_End(DOM_Node *pNode, void *pParam)
{
	iLevel_switch-- ;
	return XMLRULES_Continue;

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_if(DOM_Node *pNode, void *pParam)
{
	NodeEntry   *pNodeEntry;
	DOM_Node    *pParam1, *pParam2, *pOp ;

	if ( pNode->attributes )
	{
		pNodeEntry = pNode->attributes->first ;

		if ( pNodeEntry )
		{
			pParam1 = pNodeEntry->node ;
			pNodeEntry = pNodeEntry->next ;
		}
		else
			pParam1 = NULL ;

		if ( pNodeEntry )
		{
			pParam2 = pNodeEntry->node ;
			pNodeEntry = pNodeEntry->next ;
		}
		else
			pParam2 = NULL ;

		if ( pNodeEntry )
		{
			pOp = pNodeEntry->node ;
		}
		else
			pOp = NULL ;

		
		if ( pParam1 && pParam2 )
			return CheckCondition(pParam1, pParam2, pOp, (t_key*)pParam) ;
	}

	return XMLRULES_Exec;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_case(DOM_Node *pNode, void *pParam)
{
	DOM_Node    *pParam2;
	DOM_Node    *pOp;

	if ( ParamSwitch[iLevel_switch].pParam1 && pNode->attributes && pNode->attributes->first )
	{
		pParam2 = pNode->attributes->first->node ;
		if ( pParam2 )
		{
			if ( pNode->attributes->first->next )
				pOp = pNode->attributes->first->next->node ;
			else
				pOp = NULL ;
		
			return CheckCondition(ParamSwitch[iLevel_switch].pParam1, pParam2, pOp, (t_key*)pParam) ;
		}
	}

	return XMLRULES_Exec ;
}

//-----------------------------------------------------------------------------
// Change a key if it exist or add a new key
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_set_key(DOM_Node *pNode, void *pParam)
{
	NodeEntry   *pNodeEntry;
	t_key       *pKey = (t_key*)pParam;
	char        *pTag_keyname = NULL ;
	char        *pTag_value   = NULL ;
	int         i;

	if ( pNode->attributes )
	{
		pNodeEntry = pNode->attributes->first ;
		while ( pNodeEntry )
		{
			if ( strcmp(pNodeEntry->node->nodeName, "keyname")==0 )
				pTag_keyname = pNodeEntry->node->nodeValue ;
			else if ( strcmp(pNodeEntry->node->nodeName, "value")==0 )
				pTag_value = pNodeEntry->node->nodeValue ;

			pNodeEntry = pNodeEntry->next ;
		}
		if ( pTag_keyname && pTag_value )
		{
			for ( i=0 ; i<pKey->nbkey ; i++ )
				if ( strcmp((char*)pKey->name[i], pTag_keyname)==0 )
					break ;

			TRACE_printf(" [%s]=\"%s\"", pTag_keyname, pTag_value);
			// if key not found : key added
			if ( i==pKey->nbkey )
			{
				TRACE_printf(" (key added)");
				strcpy(pKey->name[i], pTag_keyname);
				pKey->nbkey++;
			}
			strcpy(pKey->value[i], pTag_value);
		}
		else
			Com_Printf("XMLRULES : set_key with wrong param\n");
	}
	else
		Com_Printf("XMLRULES : set_key without param\n");

	return XMLRULES_Continue ;
}

//-----------------------------------------------------------------------------
// Delete a key if it exist
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_del_key(DOM_Node *pNode, void *pParam)
{
	t_key       *pKey = (t_key*)pParam;
	DOM_Node    *pAtt ;
	int         i;

	if (    pNode->attributes 
		 && pNode->attributes->first 
		 && pNode->attributes->first->node )
	{
		pAtt = pNode->attributes->first->node ;
		if ( strcmp(pAtt->nodeName, "keyname")==0 )
		{
			for ( i=0 ; i<pKey->nbkey ; i++ )
				if ( strcmp((char*)pKey->name[i], pAtt->nodeValue)==0 )
					break ;

			if ( i < pKey->nbkey )
			{
				// if the key is found : delete it
				TRACE_printf(" %s=\"%s\"", pAtt->nodeName, pAtt->nodeValue);
				TRACE_printf(" (key deleted)");
				pKey->nbkey--;
				for ( ; i<pKey->nbkey ; i++ )
				{
					strcpy(pKey->name[i],  pKey->name[i+1]) ;
					strcpy(pKey->value[i], pKey->value[i+1]) ;
				}
			}
		}
		else
			Com_Printf("XMLRULES : del_key with wrong param\n");
	}
	else
		Com_Printf("XMLRULES : del_key without param\n");

	return XMLRULES_Continue ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_exit(DOM_Node *pNode, void *pParam)
{
	return XMLRULES_Exit;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_break(DOM_Node *pNode, void *pParam)
{
	return XMLRULES_Break;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_del_entity(DOM_Node *pNode, void *pParam)
{
	t_key   *pKey = (t_key*)pParam;

	pKey->nbkey = 0 ;
	return XMLRULES_Exit;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_entity(DOM_Node *pNode, void *pParam)
{
	DOM_NodeList    *pMemo = pXMLRULES_entity_change ;
	DOM_Node        *pAtt ;
	char            *pszEntitie ;

	if (    pNode->attributes 
		 && pNode->attributes->first 
		 && pNode->attributes->first->node )
	{
		pAtt = pNode->attributes->first->node ;
		if (    strcmp(pAtt->nodeName, "entity_change")==0 
			 && (    strcmp(pAtt->nodeValue, "true")==0
				  || strcmp(pAtt->nodeValue, "1")==0 ) )
		{
			pMemo = NULL ;
		}
		else
			Com_Printf(" : entity error on param %s=\"%s\"\n", pAtt->nodeName, pAtt->nodeValue);
	}

	if ( pMemo )
		pXMLRULES_entity_change = NULL ;

	if ( pNode->firstChild && strcmp(pNode->firstChild->nodeName, "#text")==0 )
	{
		pszEntitie = pNode->firstChild->nodeValue ;
		ParseEntities("", &pszEntitie, &pXMLRULES_ent, &pXMLRULES_inhibit) ;
	}
	
	if ( pMemo )
		pXMLRULES_entity_change = pMemo ;

	return XMLRULES_Continue;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_exec(DOM_Node *pNode, void *pParam)
{
	return XMLRULES_Exec ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_continue(DOM_Node *pNode, void *pParam)
{
	return XMLRULES_Continue ;
}

//-----------------------------------------------------------------------------
// List of tags
//-----------------------------------------------------------------------------
static t_XMLRULES_TabFunc XMLRULES_Tags[] = 
{
	{"switch",    XMLRULES_switch,    XMLRULES_switch_End,  true},
	{"if",        XMLRULES_if,        XMLRULES_continue,    false},
	{"case",      XMLRULES_case,      XMLRULES_break,       false},
	{"set_key",   XMLRULES_set_key,   XMLRULES_continue,    false},
	{"del_key",   XMLRULES_del_key,   XMLRULES_continue,    false},
	{"exit",      XMLRULES_exit,      XMLRULES_continue,    false},
	{"break",     XMLRULES_break,     XMLRULES_continue,    false},
	{"del_entity",XMLRULES_del_entity,XMLRULES_continue,    false},

	{"entity",    XMLRULES_entity,    XMLRULES_break,       false},

	{"entity_change",XMLRULES_exec,   XMLRULES_break,       false},
	{"entity_add",   XMLRULES_exec,   XMLRULES_break,       false},
};

//-----------------------------------------------------------------------------
// Generic function for parsing the xmlrules document (list of tag behind)
//-----------------------------------------------------------------------------
static t_XMLRULES_Status XMLRULES_Generic(DOM_Node *pNode, void *pParam)
{
	t_XMLRULES_TabFunc  *pXMLRULES_Tag;
	int                 i;
	t_XMLRULES_Status   eStatus = XMLRULES_Continue ;
	static int          iLevel = -1 ;

	iLevel++;

	do
	{
		pXMLRULES_Tag = NULL ;
		for ( i=0 ; !pXMLRULES_Tag && i<SIZE_TAB(XMLRULES_Tags) ; i++ )
			if ( strcmp(pNode->nodeName, XMLRULES_Tags[i].szTagName)==0 )
				pXMLRULES_Tag = &XMLRULES_Tags[i];

		// Begin branch
		if ( pXMLRULES_Tag )
		{
			TRACE_printf("\n%s<%s>", strnspace(iLevel), pNode->nodeName);
			eStatus = pXMLRULES_Tag->pfunc_Begin(pNode, pParam);
		}
		else
			eStatus = XMLRULES_Exec ;

		// Explore branch
		if ( eStatus==XMLRULES_Exec )
		{
			if ( pNode->firstChild )
				eStatus = XMLRULES_Generic(pNode->firstChild, pParam);
			else
				eStatus = XMLRULES_Continue ;

			// "Break" inhibit all command until we are out of the current block
			if ( pXMLRULES_Tag )
			{
				if ( eStatus==XMLRULES_Break && pXMLRULES_Tag->bBreak )
					eStatus = XMLRULES_Continue ;
		
				TRACE_printf("\n%s</%s>", strnspace(iLevel), pNode->nodeName);

				// End branch
				if ( eStatus == XMLRULES_Continue )
					eStatus = pXMLRULES_Tag->pfunc_End(pNode, pParam);
				else
					pXMLRULES_Tag->pfunc_End(pNode, pParam);
			}
		}

		pNode = pNode->nextSibling ;
	}
	while ( pNode && eStatus==XMLRULES_Continue ) ;
	
	iLevel--;

	return eStatus;
}

//-----------------------------------------------------------------------------
// return true if XMLRULES are allowed to the current level
//-----------------------------------------------------------------------------
qboolean XMLRULES_Allowed()
{
	char            szSaveGame[MAX_OSPATH];
	FILE		    *file;
	int             iLen;
	cvar_t	        *cvar_game;
	qboolean        bResult = false ;

	if (    !pXMLRULES_doc 
		 && !strstr(level.mapname, ".cin") 
		 && !strstr(level.mapname, ".dm2")
		 && !strstr(level.mapname, ".pcx") )
	{
		// Check for saved game
		cvar_game = gi.cvar ("game", "", 0);
		if ( cvar_game && strlen(cvar_game->string) )
			Com_sprintf ( szSaveGame, sizeof(szSaveGame), "%s", cvar_game->string ) ;
		else
			strcpy(szSaveGame, GAMEVERSION);

		iLen = strlen(szSaveGame);
		Com_sprintf ( szSaveGame+iLen, sizeof(szSaveGame)-iLen, 
					  "/save/current/%s.sav", level.mapname);
		file = fopen (szSaveGame, "rb");
		// If a saved game exist : don't apply XMLRULES (overwriten by ReadLevel)
		if ( file )
			fclose(file);
		else
			bResult = true ;
	}
	return bResult;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static qboolean XMLRULES_CreateDocument(char *pszXML)
{
	int iResult ;

	// Init
	if (!setlocale(LC_CTYPE, "")) 
	{
		Com_Printf("XMLRULES : Can't set the specified locale! Check LANG, LC_CTYPE, LC_ALL.\n");
		return false;
	}

	DOM_Exception = 0;
	pXMLRULES_doc = DOM_Implementation_createDocument(NULL, NULL, NULL);

	iResult = DOM_DocumentLS_readmem(pXMLRULES_doc, pszXML) ;
	if ( iResult == 0 )
	{
		pXMLRULES_doc = NULL;
		Com_Printf("XMLRULES : DOM_DocumentLS_readmem() == %d\n", iResult);
		// Save XML file for debug
		{
			FILE    *file;

			file = fopen("xmlrules_debug.xml", "w") ;
			if ( file )
			{
				fwrite(pszXML, 1, strlen(pszXML), file) ;
				fclose(file) ;
			}
		}
		return false ;
	}

	pXMLRULES_entity_change = DOM_Document_getElementsByTagName(pXMLRULES_doc, "entity_change");
	pXMLRULES_entity_add    = DOM_Document_getElementsByTagName(pXMLRULES_doc, "entity_add");

	// Initialize pseudorandom for never generate the same thing
	srand(clock());

	return true;
}

//-----------------------------------------------------------------------------
// If allowed, create the XMLRULES document from the WorldServer if connected, 
// or from a local file if the xmlrules cvar is defined
//-----------------------------------------------------------------------------
qboolean XMLRULES_CreateDOM()
{
	volatile rpc_data_t  RpcData ;
	char     szFileName[MAX_OSPATH];
	FILE	 *file;
	int      iLen;
	cvar_t	 *cvar_game;
	char     *pBuffer;
	qboolean bResult = false ;

	if ( XMLRULES_Allowed() )
	{
		if ( worldserver->value )
		{
			RpcData.iResultSize = RPC_XMLRULES_RESULT_SIZE;
			RpcData.pszResult = gi.TagMalloc(RpcData.iResultSize, 0);
			if ( RpcData.pszResult )
			{
				WSrv_MapGetXmlRules(&RpcData) ;

				// Blocking the server during the request to the WorldServer is not nice :(
				while ( RpcData.byRpcStatus == RPC_STATUS_IN_PROGRESS )
					RPC_Sleep(50) ;

				if ( WSrv_MapGetXmlRules_OK(&RpcData) )
				{
					bResult = XMLRULES_CreateDocument(RpcData.pszResult) ;
					if ( !bResult )
						Com_Printf("XMLRULES_CreateDOM : XMLRULES_CreateDocument() == FALSE\n");
				}
				else
					Com_Printf("XMLRULES_CreateDOM : RpcStatus = %d\n", RpcData.byRpcStatus);

				gi.TagFree(RpcData.pszResult);
			}
		}
		else if ( strlen(xmlrules->string) )
		{
			cvar_game = gi.cvar ("game", "", 0);
			if ( cvar_game && strlen(cvar_game->string) )
				Com_sprintf ( szFileName, sizeof(szFileName), "%s", cvar_game->string ) ;
			else
				strcpy(szFileName, GAMEVERSION);

			iLen = strlen(szFileName);
			Com_sprintf ( szFileName+iLen, sizeof(szFileName)-iLen, 
						  "/%s", xmlrules->string);
			file = fopen (szFileName, "rb");

			if ( file )
			{
				iLen = FS_filelength(file) ;

				pBuffer = gi.TagMalloc(iLen+1, 0);
				if ( pBuffer )
				{
					fread (pBuffer, 1, iLen, file);
					pBuffer[iLen] = '\0' ;
					bResult = XMLRULES_CreateDocument(pBuffer) ;
					gi.TagFree(pBuffer);
				}
				fclose(file);
			}
		}
	}

	return bResult ;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qboolean XMLRULES_DestroyDOM()
{
	if ( pXMLRULES_doc )
	{
/*
		// Need to read the DOMC documentation for make clean code
		if ( pXMLRULES_entity_change )
			DOM_Document_destroyNodeList( pXMLRULES_doc, 
										  pXMLRULES_entity_change, 
										  pXMLRULES_entity_change->length);
		if ( pXMLRULES_entity_add )
			DOM_Document_destroyNodeList( pXMLRULES_doc, 
										  pXMLRULES_entity_add, 
										  pXMLRULES_entity_add->length);
*/
		DOM_Document_destroyNode(pXMLRULES_doc, pXMLRULES_doc);
		pXMLRULES_doc           = NULL;
		pXMLRULES_entity_change = NULL;
		pXMLRULES_entity_add    = NULL;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Manage the <entity_change> section
//-----------------------------------------------------------------------------
qboolean XMLRULES_entity_change(t_key *pkey)
{
	NodeEntry   *pNodeEntry;

	if ( pXMLRULES_entity_change )
	{
		pNodeEntry = pXMLRULES_entity_change->first ;

		while ( pNodeEntry )
		{
			if ( pNodeEntry->node )
				if ( XMLRULES_Generic(pNodeEntry->node, pkey) == XMLRULES_Exit )
					break ;
			pNodeEntry = pNodeEntry->next ;
		}
	}

	return true ;
}

//-----------------------------------------------------------------------------
// Manage the <entity_add> section
//-----------------------------------------------------------------------------
qboolean XMLRULES_entity_add(edict_t **ent, int *inhibit)
{
	NodeEntry   *pNodeEntry;
	int         iZero = 0;

	if ( pXMLRULES_entity_add )
	{
		pXMLRULES_ent = *ent ;
		pXMLRULES_inhibit = 0 ;

		pNodeEntry = pXMLRULES_entity_add->first ;

		while ( pNodeEntry )
		{
			if ( pNodeEntry->node )
				if ( XMLRULES_Generic(pNodeEntry->node, (t_key*)&iZero) == XMLRULES_Exit )
					break ;
			pNodeEntry = pNodeEntry->next ;
		}

		*ent = pXMLRULES_ent ;
		*inhibit += pXMLRULES_inhibit ;
	}


	return true ;
}
