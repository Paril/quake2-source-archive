/*==============================================================================
The Weapons Factory - 
Referee Functions
Original code by Gregg Reno
==============================================================================*/
#include "g_local.h"

void WFEndDMLevel (char *mapname);
void WFMapVote(edict_t *ent);

//Util to find a player
edict_t *refFindPlayer(char *name)
{
	edict_t *e;
	int i;

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//match on netname?
		if (Q_stricmp(e->client->pers.netname, name) == 0)
		{
			return e;
		}
	}
	return NULL;
}


void Cmd_Ref_Password (edict_t *ent)
{
	char    *string;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, there already is a REF.\n");
		return;
	}

	string=gi.args();

	if (wf_game.ref_password == NULL || wf_game.ref_password[0] == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "The REF has been disabled on this server\n");
		return;
	}


	if (Q_stricmp ( string, wf_game.ref_password) == 0)
	{
		wf_game.ref_ent = ent;
		my_bprintf (PRINT_MEDIUM, "%s is now the REF.\n", wf_game.ref_ent->client->pers.netname);
//		safe_cprintf (ent, PRINT_HIGH, "You are now the REF.\n");
	}
	else	//If no "on" or "off", toggle state
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, password is not valid\n");
		wf_game.ref_ent = NULL;
	}
}

void Cmd_Ref_Show (edict_t *ent)
{
	if (ent->bot_client) return;
	if (!ent->client) return;


	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent && wf_game.ref_ent->client)
	{
		safe_cprintf (ent, PRINT_HIGH, "The REF is %s.\n",
			wf_game.ref_ent->client->pers.netname);
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "Right now, there is no REF.\n");
	}
}

void Cmd_Ref_Skin_On (edict_t *ent)
{
	char	 *s;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
		wf_game.show_ref_skin = 1;
		s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
		CTFAssignSkin(ent, s);
		safe_cprintf(ent, PRINT_HIGH, "Ref Skin ON.\n");
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

void Cmd_Ref_Skin_Off (edict_t *ent)
{
	char	 *s;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
		wf_game.show_ref_skin = 0;
		s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
		CTFAssignSkin(ent, s);
		safe_cprintf(ent, PRINT_HIGH, "Ref Skin OFF.\n");
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}


void Cmd_Ref_NextMap (edict_t *ent)
{
	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
         if (maplist.nummaps > 0)  // does a maplist exist? 
            EndDMLevel(); 
         else 
		 {
			safe_cprintf (ent, PRINT_HIGH, "Can't do next - No maps in current list\n");
		 }
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

void Cmd_Ref_PickMap (edict_t *ent)
{
//	char    *string;
	
	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	/*
	string=gi.args();

	if (string[0] == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "You have to specify a map name.\n");
		return;
	}
	*/

	if (wf_game.ref_ent == ent)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Changing to map '%s'.\n", string);
		//WFEndDMLevel(string); 
		
		//Bring up vote menu instead of using a typed map name
		wf_game.ref_picked_map = 1;
		WFMapVote(ent);
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
	


}

void Cmd_Ref_Kick (edict_t *ent)
{
	char    *string;
	edict_t *e;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	string=gi.args();

	if (wf_game.ref_ent == ent)
	{
		//Find the player with the given name
		e = refFindPlayer(string);
		if (e)
		{
			safe_cprintf(e, PRINT_HIGH, "You have been kicked by the REF.\n");
			stuffcmd(e, "disconnect\n"); 
			my_bprintf (PRINT_MEDIUM, "Player '%s' was kicked by the REF.\n", string);
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Player '%s' not found.\n", string);
		}
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

void Cmd_Ref_Start (edict_t *ent)
{
	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
		my_bprintf (PRINT_MEDIUM, "REF HAS STARTED THE GAME\n");
		wf_game.game_halted = 0;
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

void Cmd_Ref_Stop (edict_t *ent)
{
	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
		my_bprintf (PRINT_MEDIUM, "REF HAS STOPPED THE GAME\n");
		wf_game.game_halted = 1;
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

//Don't allow a player to talk
void Cmd_Ref_NoTalk (edict_t *ent)
{
	char    *string;
	edict_t *e;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	string=gi.args();

	if (wf_game.ref_ent == ent)
	{
		//Find the player with the given name
		e = refFindPlayer(string);
		if (e)
		{
			safe_cprintf(e, PRINT_HIGH, "The REF has silenced you!\n");
			my_bprintf(PRINT_HIGH, "The REF has silenced %s!\n",e->client->pers.netname);
			e->client->silenced = 1;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Player '%s' not found.\n", string);
		}
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

//Allow a player to talk again
void Cmd_Ref_Talk (edict_t *ent)
{
	char    *string;
	edict_t *e;

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	string=gi.args();

	if (wf_game.ref_ent == ent)
	{
		//Find the player with the given name
		e = refFindPlayer(string);
		if (e)
		{
			safe_cprintf(e, PRINT_HIGH, "The REF says you to talk again.\n");
			e->client->silenced = 0;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Player '%s' not found.\n", string);
		}
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}

//Lets ref stop being the ref
void Cmd_Ref_Leave (edict_t *ent)
{

	if (ent->bot_client) return;
	if (!ent->client) return;

	if (wf_game.ref_ent && !wf_game.ref_ent->inuse)
	{
		wf_game.ref_ent = NULL;
	}

	if (wf_game.ref_ent == ent)
	{
		if (wf_game.show_ref_skin)
			Cmd_Ref_Skin_Off(ent);
		wf_game.ref_ent = NULL;

		safe_cprintf (ent, PRINT_HIGH, "You are no longer the ref.\n");
	}
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You are not the REF.\n");
	}
}
