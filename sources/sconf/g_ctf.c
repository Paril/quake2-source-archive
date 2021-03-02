#include "g_local.h"
#include "s_readconfig.h" 

// sconfig 2.8
void Cmd_Inven_f (edict_t *ent);
// end sconfig 2.8

void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

/*-----------------------------------------------------------------------*/

void CTFJoinGame(edict_t *ent)
{
	char *s;
	int j;

	PMenu_Close(ent);

	ent->client->hudcloak_state == false; // not cloaked by default... (optional)
	if (ent->client->wasingame == 1)
		j=1;
	else j=0;

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_TEAM1;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	
	
	PutClientInServer (ent);	
	
	
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	// Sconfig 2.3, start sound when Entering game from menu
   
	// sconfig 2.8
	if (j==0)
		gi.sound(ent, CHAN_NO_PHS_ADD, gi.soundindex("misc/bigtele.wav"), 1, ATTN_IDLE, 0);
	// end sconfig 2.8

	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	
	// sconfig 2.8
	if (j==0)	
		gi.bprintf(PRINT_HIGH, "%s joined the game.\n",	ent->client->pers.netname); // ORIGINAL
	else	
		gi.bprintf(PRINT_HIGH, "%s rejoined the game.\n",	ent->client->pers.netname);
	// end sconfig 2.8
	
	
	// sconfig 2.8
 	ent->client->menu_state = 2; // 1 = not in game; 2 = in game
	// end sconfig 2.8

	// SCONFIG START
	// Version 2.6b5
	// can't be here
	ent->client->resp.helphud=false;
	

//	spectator=0; // Set spectator mode 0. now in game.		
	// SCONFIG STOP
}

void CTFJoinGame1(edict_t *ent, pmenu_t *p)
{
	CTFJoinGame(ent);
}


// Activate Chase Cam mode, now supports Chase Cam selection from within game (2.8)
void CTFChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	// sconfig 2.8		
	if (ent->client->menu_state == 2) {
		
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		
//		}
		
		//else
		gi.linkentity (ent);

		if (chasekeepscore==0)
			ent->client->resp.score=0; // (reset's score if set to zero value)
		// sconfig 2.8
		ent->client->menu_state = 1; // 1 = not in game; 2 = in game, 3=inventory
		ent->client->wasingame = 1;
		// end sconfig 2.8
		ent->client->resp.helphud=true;


		PMenu_Close(ent);
		}


	// Check to see if player is dead.  And adjust if they are entering "chase cam mode"
	if (ent->health <= 0 && (deathmatch->value && !((int)dmflags->value & DF_FORCE_RESPAWN))){

		//ent->deadflag = DEAD_DEAD;
		ent->deadflag = DEAD_NO;		
		//respawn(ent);	
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT; // |=
		//ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		ent->client->menu_state = 1; // 1 = not in game; 2 = in game, 3=inventory
		ent->client->wasingame = 1;
		//ent->client->latched_buttons = 0;
		ent->client->resp.helphud=true;
		//ent->client->chase_target = NULL; // jumped off always
		//gi.linkentity (ent);
		PMenu_Close(ent);
	}


	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

/*
void CTFChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;


	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}
*/
// sconfig 2.8
// if playing local, do not exit this way... reconnect instead...
//void CTFDisconnect(edict_t *ent, pmenu_t *p)
//{
	//ClientDisconnect (ent);
//}
// end sconfig 2.8
		   
void GetInventory(edict_t *ent, pmenu_t *p)
{
   	ent->client->menu_state = 3; // 1 = not in game; 2 = in game, 3=inventory
	Cmd_Inven_f (ent);

}

void CTFReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

// sconfig 2.8
pmenu_t MOTDmenu2[] = {
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "Inventory",              PMENU_ALIGN_RIGHT, NULL, GetInventory },
	{ "Chase Cam",				PMENU_ALIGN_RIGHT, NULL, CTFChaseCam }
};
// end sconfig 2.8

pmenu_t MOTDmenu[] = {
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "",						PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "Join Game",				PMENU_ALIGN_RIGHT, NULL, CTFJoinGame1 },
	//{ "High Scores",            PMENU_ALIGN_RIGHT, NULL, GetHighs }
	{ "Chase Cam",				PMENU_ALIGN_RIGHT, NULL, CTFChaseCam }
};

// Used to say Continue where Join Game resides...

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	if (ent->client->chase_target)
		team = 8;
	else
		team = 4;

		MOTDmenu[0].text = somevar0;
		MOTDmenu[1].text = somevar1;
		MOTDmenu[2].text = somevar2;
		MOTDmenu[3].text = somevar3;
		MOTDmenu[4].text = somevar4;
		MOTDmenu[5].text = somevar5;
		MOTDmenu[6].text = somevar6;
		MOTDmenu[7].text = somevar7;
		MOTDmenu[8].text = somevar8;
		MOTDmenu[9].text = somevar9;
		MOTDmenu[10].text = somevar10;
		MOTDmenu[11].text = somevar11;
		MOTDmenu[12].text = somevar12;
		MOTDmenu[13].text = somevar13;
		MOTDmenu[14].text = somevar14;
		
		PMenu_Open(ent, MOTDmenu, team, sizeof(MOTDmenu) / sizeof(pmenu_t));
}

// sconfig 2.8
void CTFOpenJoinMenu2(edict_t *ent)
{
	int team;

	if (ent->client->chase_target)
		team = 8;
	else
		team = 4;

		MOTDmenu2[0].text = somevar0;
		MOTDmenu2[1].text = somevar1;
		MOTDmenu2[2].text = somevar2;
		MOTDmenu2[3].text = somevar3;
		MOTDmenu2[4].text = somevar4;
		MOTDmenu2[5].text = somevar5;
		MOTDmenu2[6].text = somevar6;
		MOTDmenu2[7].text = somevar7;
		MOTDmenu2[8].text = somevar8;
		MOTDmenu2[9].text = somevar9;
		MOTDmenu2[10].text = somevar10;
		MOTDmenu2[11].text = somevar11;
		MOTDmenu2[12].text = somevar12;
		MOTDmenu2[13].text = somevar13;
		MOTDmenu2[14].text = somevar14; 
		
		PMenu_Open(ent, MOTDmenu2, team, sizeof(MOTDmenu2) / sizeof(pmenu_t));
}
// end sconfig 2.8


qboolean CTFStartClient(edict_t *ent)
{
	if (ent->client->resp.ctf_team != CTF_NOTEAM) // Place info in here if you want players to see it
		return false;						      // after they join.
	

	//if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		
		// sconfig 2.8
		ent->client->menu_state = 1; // 1 = not in game; 2 = in game, 3=inventory
		// end sconfig 2.8
		ent->client->resp.helphud=true;
		CTFOpenJoinMenu(ent);
		return true;
	//}
	//return false;
}


