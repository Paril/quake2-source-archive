
#include "g_local.h"
#include "g_sogutil.h"

#define GREEN	1
#define WHITE	2
#define REVERSE	3
#define DEFAULT	4

// For menus
#define GEN_DM_VERSION		"SOG\nDM"
#define GEN_COOP_VERSION	"SOG\nCo-op play"
#define GEN_SP_VERSION		"SOG\nSingle Player"
#define GEN_CTF_VERSION		"SOG\nCapture the Flag"
#define GEN_TEAM_VERSION	"SOG\nTeamplay"
#define GEN_CTF_PICK		"SOG\nTeam Menu"
#define GEN_BUILD_VERSION	"       Version 1.2"

//==============================================
// calc and return number of players by class 
// pretty ugly way of doing it, but hey :D
//==============================================

static int NumPlayer(int pclass)
{
	int i,num=0;
	edict_t		*cl_ent;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if(sog_ctf->value)
			if(!game.clients[i].resp.team)
				continue;

		if(game.clients[i].resp.player_class == pclass)
			num++;
	}
	return num;
}

//==============================================
// calc and return number of players team
//==============================================

static int NumTeam(int team)
{
	int i,num=0;
	edict_t		*cl_ent;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if(game.clients[i].resp.team == team)
			num++;
	}
	return num;
}

//==============================================
// Return a string depending on the skill level
//==============================================

static char * SkillLevel(void)
{
	char *s;
	s = malloc(16);
		
	if((int)skill->value == 1)
	{
		strcpy(s,"King Llama");
	}
	else if((int)skill->value == 2)
	{
		strcpy(s,"Unknown Marine");
	}
	else if((int)skill->value == 3)
	{
		strcpy(s,"Joe Quaker");
	}
	else
	{
		strcpy(s,"Doom Machine");
	}
	return s;
}


/*
==============================================
 Menu Chasecam Function
 Activates Personal or Regular Chasecam
 depending on whether client is playing or not
==============================================
*/

int GenCC(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int i;
	edict_t *e;

	if((ent->client->resp.spectator == true) || 
	   (ent->client->resp.team == CTF_NOTEAM && ent->client->resp.player_class == NO_CLASS))
	{
		if (ent->client->chase_target) 
		{
			//ent->svflags &= ~SVF_NOCLIENT;       //added
			ent->client->chase_target = NULL;
		}
		else
		{
			for (i = 1; i <= maxclients->value; i++) 
			{
				e = g_edicts + i;
				if (e->inuse && e->solid != SOLID_NOT) 
				{
					ent->client->chase_target = e;
					ent->client->update_chase = true;
					ent->svflags |= SVF_NOCLIENT;    //added
					break;
				}
			}
		}
	}
	else if(ent->health && ent->client->resp.player_class > NO_CLASS)
	{
		if(sog_ctf->value)
		{
			if(ent->client->resp.team > CTF_NOTEAM)
				Cmd_Chasecam_Toggle (ent);
		}
		else
		{
			Cmd_Chasecam_Toggle (ent);
		}
	}
	return 0;
}


/*
=======================
GenJoinClass 

Called by
GenClass from Both DM and CTF menus
=======================
*/

void GenJoinClass(edict_t *ent, int desired_class)
{
	if (ent->client->resp.player_class == desired_class)  
	{
		safe_cprintf(ent, PRINT_HIGH, "You are already on the %s class.\n",
					GenClassName(ent->client->resp.player_class));
		return;
	}
	
	if((!ent->client->resp.player_class) && 
	   !(sog_ctf->value || (sog_team->value && (int)sogflags->value & SOG_REDBLUE_TEAMS)))
	{
		if (!deathmatch->value) 
			ent->flags &= ~FL_NOTARGET;

		ent->client->resp.state = START_GEN;
		ent->client->resp.player_next_class = desired_class;
		safe_bprintf(PRINT_HIGH, "%s became %s.\n",
		ent->client->pers.netname, GenClassName(desired_class));
		GenPutClientInGame(ent);
	}
	else
	{
		//Process for Teams
		GenClassPicked(ent,desired_class);
	}
}


//==================================
// Menu Callback funcs
// Lots of code, 
// but I avoid strcmp'ing the menu text this way
//==================================

int GenSetSkill(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		if((int)skill->value == 0)
		{
			gi.cvar_forceset("skill", "1");
			strcpy(((menu_item_t *)selected->it)->valuetext,"Unknown Marine");			
			
		}
		else if((int)skill->value == 1)
		{
			gi.cvar_forceset("skill", "2");
			strcpy(((menu_item_t *)selected->it)->valuetext,"Joe Quaker");
		}
		else if((int)skill->value == 2)
		{
			gi.cvar_forceset("skill", "3");
			strcpy(((menu_item_t *)selected->it)->valuetext,"Doom Machine");
		}
		else if((int)skill->value == 3)
		{
			gi.cvar_forceset("skill", "0");
			strcpy(((menu_item_t *)selected->it)->valuetext,"King Llama");
		}
	}
	return 1;
}

int GenQ2Class(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		GenJoinClass(ent, CLASS_Q2);
	}
	else
	{
		if(ent->client->resp.player_next_class == CLASS_Q2)
			ent->client->resp.player_next_class = ent->client->resp.player_class;
	}
	ent->client->random_class = false;
	return 0;
}

int GenQ1Class(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		GenJoinClass(ent, CLASS_Q1);
	}
	else
	{
		if(ent->client->resp.player_next_class == CLASS_Q1)
			ent->client->resp.player_next_class = ent->client->resp.player_class;
	}
	ent->client->random_class = false;
	return 0;
}

int GenDoomClass(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		GenJoinClass(ent, CLASS_DOOM);
	}
	else
	{
		if(ent->client->resp.player_next_class == CLASS_DOOM)
			ent->client->resp.player_next_class = ent->client->resp.player_class;
	}
	ent->client->random_class = false;
	return 0;
}

int GenWolfClass(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		GenJoinClass(ent, CLASS_WOLF);
	}
	else
	{
		if(ent->client->resp.player_next_class == CLASS_WOLF)
			ent->client->resp.player_next_class = ent->client->resp.player_class;
	}
	ent->client->random_class = false;
	return 0;
}

int GenRandClass(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		GenJoinClass(ent,GetRandomClass());
	}
	ent->client->random_class = true;
	return 0;
}


//==================================
// DM Menu
//==================================

void GenOpenJoinMenu(edict_t *ent)
{
	arena_link_t *menudm=0;

	if(deathmatch->value)
	{
		if (sog_team->value)
			menudm=CreateGenMenu(ent,GEN_TEAM_VERSION);
		else
			menudm=CreateGenMenu(ent,GEN_DM_VERSION);
	} 
	else if(coop->value)
		menudm=CreateGenMenu(ent,GEN_COOP_VERSION); 
	else
		menudm=CreateGenMenu(ent,GEN_SP_VERSION); 
	
	AddMenuItem(menudm, "",								NULL, -1,			NULL,		DEFAULT);
	
	if (!((int)sogflags->value & SOG_IDG3_ONLY)
		&& !((int)sogflags->value & SOG_IDG2_ONLY)
		&& !((int)sogflags->value & SOG_IDG1_ONLY)) 	
	{	
		AddMenuItem(menudm, "Blaze      ",					NULL, NumPlayer(CLASS_WOLF),	&GenWolfClass,DEFAULT); 
		AddMenuItem(menudm, "Flynn      ",					NULL, NumPlayer(CLASS_DOOM),	&GenDoomClass,DEFAULT); 
		AddMenuItem(menudm, "Axe        ",					NULL, NumPlayer(CLASS_Q1),		&GenQ1Class,DEFAULT);
		AddMenuItem(menudm, "Sgt. Payne ",					NULL, NumPlayer(CLASS_Q2),		&GenQ2Class,DEFAULT);
		AddMenuItem(menudm, "Random     ",					NULL, -1,			&GenRandClass,	DEFAULT);
		AddMenuItem(menudm, "",								NULL, -1,			NULL,		DEFAULT);
	}
	if(ent->client->chasetoggle || ent->client->chase_target)
		AddMenuItem(menudm, "Leave ChaseCam",				NULL, -1,			&GenCC,		DEFAULT);
	else
		AddMenuItem(menudm, "ChaseCam",						NULL, -1,			&GenCC,		DEFAULT);
	
	if(!(deathmatch->value) && !(coop->value) )
	{
		AddMenuItem(menudm, "Skill Level   ",				SkillLevel(), -1,	&GenSetSkill,	DEFAULT);
	}
	
	AddMenuItem(menudm, "",								NULL, -1,			NULL,		DEFAULT);
	AddMenuItem(menudm, "Use [ and ] to move cursor",	NULL, -1,			NULL,		GREEN);
	AddMenuItem(menudm, "ENTER to select,TAB To close",		NULL, -1,			NULL,		GREEN);
//	AddMenuItem(menudm, "Press TAB to close",			NULL, -1,			NULL,		GREEN);
	AddMenuItem(menudm, "",								NULL, -1,			NULL,		DEFAULT);
	AddMenuItem(menudm, GEN_BUILD_VERSION,				NULL, -1,			NULL,		DEFAULT);
	FinishMenu(ent, menudm);
}

//==================================
// CTF Menu
//==================================

void GenOpenJoinClassMenu(edict_t *ent)
{
	arena_link_t *menuctf;
	
	if(sog_ctf->value)
		menuctf=CreateGenMenu(ent,GEN_CTF_VERSION);
	else
	    menuctf=CreateGenMenu(ent,GEN_CTF_PICK); 

	AddMenuItem(menuctf,  "",							NULL, -1,			NULL,		DEFAULT);
	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		AddMenuItem(menuctf, "Blaze      ",	NULL, NumPlayer(CLASS_WOLF),&GenWolfClass,DEFAULT); 
	}
	else if ((int)sogflags->value & SOG_IDG2_ONLY)
	{
		AddMenuItem(menuctf, "Flynn      ",	NULL, NumPlayer(CLASS_DOOM),&GenDoomClass,DEFAULT); 
	}
	else if ((int)sogflags->value & SOG_IDG3_ONLY)
	{
		AddMenuItem(menuctf, "Axe        ",	NULL, NumPlayer(CLASS_Q1),&GenQ1Class,DEFAULT);
	}
	else
	{
		AddMenuItem(menuctf, "Blaze      ",	NULL, NumPlayer(CLASS_WOLF),&GenWolfClass,DEFAULT); 
		AddMenuItem(menuctf, "Flynn      ",	NULL, NumPlayer(CLASS_DOOM),&GenDoomClass,DEFAULT); 
		AddMenuItem(menuctf, "Axe        ",	NULL, NumPlayer(CLASS_Q1),&GenQ1Class,DEFAULT);
		AddMenuItem(menuctf, "Sgt. Payne ",NULL, NumPlayer(CLASS_Q2),&GenQ2Class,DEFAULT);
		AddMenuItem(menuctf, "Random     ",					NULL, -1,			&GenRandClass,DEFAULT);
	}
	AddMenuItem(menuctf, "",							NULL, -1,			NULL,		DEFAULT);
	
	if(ent->client->chasetoggle || ent->client->chase_target)
		AddMenuItem(menuctf, "Leave ChaseCam",				NULL, -1,		&GenCC,		DEFAULT);
	else	
		AddMenuItem(menuctf, "ChaseCam",					NULL, -1,		&GenCC,		DEFAULT);
	
	AddMenuItem(menuctf, "",							NULL, -1,			NULL,		DEFAULT);
	AddMenuItem(menuctf, "Use [ and ] to move cursor",	NULL, -1,			NULL,		GREEN);
	AddMenuItem(menuctf, "Press ENTER to select",		NULL, -1,			NULL,		GREEN);
	AddMenuItem(menuctf, "Press TAB to close",			NULL, -1,			NULL,		GREEN);
	AddMenuItem(menuctf, "",							NULL, -1,			NULL,		DEFAULT);
	AddMenuItem(menuctf, GEN_BUILD_VERSION,				NULL, -1,			NULL,		DEFAULT);
	FinishMenu(ent, menuctf);
}


//============================
// CTF Spawn
//============================

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	ent->client->resp.team = desired_team;
	ent->client->resp.state = START_GEN;
	safe_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));

    GenPutClientInGame(ent);
}

//==================================
// CTF Team menu Callback funcs
//==================================

int TeamOutPicked(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		if(NumTeam(CTF_TEAM1) < NumTeam(CTF_TEAM2))
			CTFJoinTeam(ent, CTF_TEAM1);
		else if (NumTeam(CTF_TEAM2) < NumTeam(CTF_TEAM1))
			CTFJoinTeam(ent, CTF_TEAM2);
		else
		{
			if(random() > 0.5)
				CTFJoinTeam(ent, CTF_TEAM1);
			else
				CTFJoinTeam(ent, CTF_TEAM2);
		}
	}	
	return 0;
}

int Team1Picked(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		CTFJoinTeam(ent, CTF_TEAM1);
	}
	return 0;
}

int Team2Picked(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if(key)
	{
		CTFJoinTeam(ent, CTF_TEAM2);
	}
	return 0;
}


//==================================
// CTF team menu
//==================================

// Called by GenClassPicked
void CTFOpenJoinMenu(edict_t *ent)
{
	arena_link_t *menu1;
	menu1=CreateGenMenu(ent,GEN_CTF_PICK);

	AddMenuItem(menu1, "",							NULL, -1,				NULL,			DEFAULT);
	AddMenuItem(menu1, "Join Outnumbered Team",		NULL, -1,				&TeamOutPicked, DEFAULT);
	AddMenuItem(menu1, "Join Red Team    ",			NULL, NumTeam(CTF_TEAM1),&Team1Picked,	DEFAULT);
	AddMenuItem(menu1, "Join Blue Team   ",			NULL, NumTeam(CTF_TEAM2),&Team2Picked,	DEFAULT);	AddMenuItem(menu1, "",							NULL, -1,				NULL,			DEFAULT);
	AddMenuItem(menu1, "",							NULL, -1,				NULL,			DEFAULT);
	AddMenuItem(menu1, "Use [ and ] to move cursor",NULL, -1,				NULL,			GREEN);
	AddMenuItem(menu1, "Press ENTER to select",		NULL, -1,				NULL,			GREEN);
	AddMenuItem(menu1, "Press TAB to Exit",			NULL, -1,				NULL,			GREEN);
	FinishMenu(ent, menu1);
}


