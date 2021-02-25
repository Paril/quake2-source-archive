// CUSTOM SOURCE FILE

//
// g_combat.c was modified to add a teamplay damage function
//

#include "g_local.h"
#include "botsmain.h"
#include "botsutil.h"
#include "botsteam.h"
#include "botsfile.h"
#include "botsqdev.h"
#include "p_light.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);
void M_ReactToDamage (edict_t *targ, edict_t *attacker);
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void G_FreeEdict (edict_t *ent);
void Use_Silencer (edict_t *ent, gitem_t *item);
void Use_Quad (edict_t *ent, gitem_t *item);
void SpawnItem (edict_t *ent, gitem_t *item);
void DoRespawn (edict_t *ent);
void droptofloor (edict_t *ent);
void teamplay_ClassMenu (edict_t *ent, qboolean isauto);
void teamplay_TeamMenu (edict_t *ent, qboolean isauto);
void teamplay_GeneralMenu (edict_t *ent, qboolean isauto);
void teamplay_DropKey (edict_t *ent);
void teamplay_Impeach (edict_t *ent);
void teamplay_KeyStatus (edict_t *ent);
void teamplay_DisplayTeamScores (edict_t *ent);
edict_t *SelectDeathmatchSpawnPoint (void);

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

qboolean teamplay_IsOffense(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (ent->client->pers.playmode == 0)
		return false;
	else
		return true;
}

void teamplay_CaptainMsg (int team, char *msg)
{
	edict_t		*player;
	int			n;

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team != team)
			continue; 

		if (player->client->pers.player_class == 1)
		{
			gsutil_centerprint(player, "%s", msg);
			return; 
		}
	}

	return;
}

void teamplay_ResetImpeach(int team)
{
	int		i;
	edict_t	*player;

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->client)
			continue; 

		if (player->client->pers.team != team)
			continue; 

		player->client->pers.impeachvote = false;
	}

	teams.team_impeachvotes[team-1] = 0;

	for (i=0; i<3; i++)
	{
		if (teams.team_impeachvotes[i] > 0)
			return;
	}

	startimpeach = 0;
}

void teamplay_ExecScript(edict_t *ent)
{
	if (!ent->client)
		return;

	if (ent->client->pers.player_class == 1)
		stuffcmd(ent, "exec captain.cfg\n");
	else if (ent->client->pers.player_class == 2)
		stuffcmd(ent, "exec bodyguard.cfg\n");
	else if (ent->client->pers.player_class == 3)
		stuffcmd(ent, "exec sniper.cfg\n");
	else if (ent->client->pers.player_class == 4)
		stuffcmd(ent, "exec soldier.cfg\n");
	else if (ent->client->pers.player_class == 5)
		stuffcmd(ent, "exec berzerker.cfg\n");
	else if (ent->client->pers.player_class == 6)
		stuffcmd(ent, "exec infiltrator.cfg\n");
	else if (ent->client->pers.player_class == 7)
		stuffcmd(ent, "exec kamikazee.cfg\n");
	else if (ent->client->pers.player_class == 8)
		stuffcmd(ent, "exec nurse.cfg\n");

	stuffcmd(ent, "bind [ \"invprev\"\nbind ] \"invnext\"\n");
	stuffcmd(ent, "bind ENTER \"invuse\"\nbind BACKSPACE \"killmenu\"\n");
}


void teamplay_ClassMenuChoice(edict_t *ent, char *selection)
{
	if (!ent->client)
		return;

	if (ent->client->pers.player_class == 1)
	{
		if (Q_stricmp (selection, "Create items") == 0)
			gsmod_QueueCommand(ent, "create");
		else if (Q_stricmp (selection, "Promote player") == 0)
			gsmod_QueueCommand(ent, "promote");
		else if (Q_stricmp (selection, "Demote player") == 0)
			gsmod_QueueCommand(ent, "demote");
		else if (Q_stricmp (selection, "Scout info") == 0)
			gsmod_QueueCommand(ent, "scout");
		else if (Q_stricmp (selection, "Scout to team") == 0)
			gsmod_QueueCommand(ent, "scoutall");
		else if (Q_stricmp (selection, "Send alarm") == 0)
			gsmod_QueueCommand(ent, "sendalarm");
		else if (Q_stricmp (selection, "Use tracker") == 0)
			gsmod_QueueCommand(ent, "tracker");
		else if (Q_stricmp (selection, "Drop promo") == 0)
			gsmod_QueueCommand(ent, "droppromote");
	}
	else if (ent->client->pers.player_class == 2)
	{
		if (Q_stricmp (selection, "Create a laser") == 0)
			gsmod_QueueCommand(ent, "laser");
		else if (Q_stricmp (selection, "Turn laser on") == 0)
			gsmod_QueueCommand(ent, "laseron");
		else if (Q_stricmp (selection, "Turn laser off") == 0)
			gsmod_QueueCommand(ent, "laseroff");
		else if (Q_stricmp (selection, "Destroy laser") == 0)
			gsmod_QueueCommand(ent, "laserkill");
		else if (Q_stricmp (selection, "Decoy glow") == 0)
			gsmod_QueueCommand(ent, "decoy");
	}
	else if (ent->client->pers.player_class == 3)
	{
		if (Q_stricmp (selection, "Create an alarm") == 0)
			gsmod_QueueCommand(ent, "alarm");
		else if (Q_stricmp (selection, "Destroy alarm") == 0)
			gsmod_QueueCommand(ent, "alarmkill");
		else if (Q_stricmp (selection, "Send alarm") == 0)
			gsmod_QueueCommand(ent, "sendalarm");
		else if (Q_stricmp (selection, "Use laser sight") == 0)
			gsmod_QueueCommand(ent, "sight");
		else if (Q_stricmp (selection, "Use tracker") == 0)
			gsmod_QueueCommand(ent, "tracker");
		else if (Q_stricmp (selection, "Use flare") == 0)
			gsmod_QueueCommand(ent, "flare");
	}
	else if (ent->client->pers.player_class == 4)
	{
		if (Q_stricmp (selection, "Split rockets") == 0)
			gsmod_QueueCommand(ent, "splitmode");
	}
	else if (ent->client->pers.player_class == 5)
	{
		if (Q_stricmp (selection, "Invincible rage") == 0)
			gsmod_QueueCommand(ent, "rage");
	}
	else if (ent->client->pers.player_class == 6)
	{
		if (Q_stricmp (selection, "Start stealing") == 0)
			gsmod_QueueCommand(ent, "steal");
		else if (Q_stricmp (selection, "Use a disguise") == 0)
			gsmod_QueueCommand(ent, "disguise");
	}
	else if (ent->client->pers.player_class == 7)
	{
		if (Q_stricmp (selection, "Commit suicide") == 0)
			gsmod_QueueCommand(ent, "suicide");
		else if (Q_stricmp (selection, "Use detpipes") == 0)
			gsmod_QueueCommand(ent, "detmode");
	}
	else if (ent->client->pers.player_class == 8)
	{
		if (Q_stricmp (selection, "Poison health") == 0)
			gsmod_QueueCommand(ent, "poison");
	}
}

void teamplay_TeamMenuChoice(edict_t *ent, char *selection)
{
	if (!ent->client)
		return;

	if (Q_stricmp (selection, "Disable Auto ID") == 0)
		gsmod_QueueCommand(ent, "autoid");
	else if (Q_stricmp (selection, "Enable Auto ID") == 0)
		gsmod_QueueCommand(ent, "autoid");
	else if (Q_stricmp (selection, "Drop the flag") == 0)
		gsmod_QueueCommand(ent, "dropflag");
	else if (Q_stricmp (selection, "Impeach Captain") == 0)
		gsmod_QueueCommand(ent, "impeach");
	else if (Q_stricmp (selection, "Flag status") == 0)
		gsmod_QueueCommand(ent, "flagstatus");
	else if (Q_stricmp (selection, "Locate flag") == 0)
		gsmod_QueueCommand(ent, "locateflag");
	else if (Q_stricmp (selection, "Locate promo") == 0)
		gsmod_QueueCommand(ent, "locatepromo");
	else if (Q_stricmp (selection, "Team msg") == 0)
		gsmod_QueueCommand(ent, "messagemode2");
	else if (Q_stricmp (selection, "Your status") == 0)
		gsmod_QueueCommand(ent, "mystatus");
	else if (Q_stricmp (selection, "Player details") == 0)
		gsmod_QueueCommand(ent, "playerid");
	else if (Q_stricmp (selection, "Select new team") == 0)
		gsmod_QueueCommand(ent, "rejoin");
	else if (Q_stricmp (selection, "View scores") == 0)
		teamplay_DisplayTeamScores(ent);
	else if (Q_stricmp (selection, "Class menu") == 0)
		gsmod_QueueCommand(ent, "changeclass");
}

void teamplay_GeneralMenuChoice(edict_t *ent, char *selection)
{
	if (!ent->client)
		return;

	if (Q_stricmp (selection, "Voting menu") == 0)
		gsmod_QueueCommand(ent, "vote");
	else if (Q_stricmp (selection, "Kill yourself") == 0)
		gsmod_QueueCommand(ent, "kill");
	else if (Q_stricmp (selection, "Welcome message") == 0)
		gsmod_QueueCommand(ent, "welcome");
	else if (Q_stricmp (selection, "Gender message") == 0)
		gsmod_QueueCommand(ent, "gender");
	else if (Q_stricmp (selection, "Help msgs off") == 0)
		gsmod_QueueCommand(ent, "helpoff");
	else if (Q_stricmp (selection, "Help msgs on") == 0)
		gsmod_QueueCommand(ent, "helpon");
	else if (Q_stricmp (selection, "Mute player") == 0)
		gsmod_QueueCommand(ent, "mute");
	else if (Q_stricmp (selection, "Unmute player") == 0)
		gsmod_QueueCommand(ent, "unmute");
}

void teamplay_MainMenuChoice(edict_t *ent, char *selection)
{
	if (!ent->client)
		return;

	if (Q_stricmp (selection, "Team Commands Menu") == 0)
		gsmod_QueueCommand(ent, "teammenu");
	else if (Q_stricmp (selection, "General Commands Menu") == 0)
		gsmod_QueueCommand(ent, "generalmenu");
	else if (Q_stricmp (selection, "Team Radio Menu") == 0)
		gsmod_QueueCommand(ent, "team");
	else if (Q_stricmp (selection, "Shouts Menu") == 0)
		gsmod_QueueCommand(ent, "shout");
	else if (Q_stricmp (selection, "Execute Class Script") == 0)
		teamplay_ExecScript(ent);
	else if (Q_stricmp (selection, "Cancel") == 0)
		return;
	else 
		gsmod_QueueCommand(ent, "classmenu");
}

void teamplay_ClassListMenuChoice(edict_t *ent, int selection)
{
	if (!ent->client)
		return;

	if (selection == 1)
		gsmod_QueueCommand(ent, "class1");
	else if (selection == 2)
		gsmod_QueueCommand(ent, "class2");
	else if (selection == 3)
		gsmod_QueueCommand(ent, "class3");
	else if (selection == 4)
		gsmod_QueueCommand(ent, "class4");
	else if (selection == 5)
		gsmod_QueueCommand(ent, "class5");
	else if (selection == 6)
		gsmod_QueueCommand(ent, "class6");
	else if (selection == 7)
		gsmod_QueueCommand(ent, "class7");
	else if (selection == 8)
		gsmod_QueueCommand(ent, "class8");
}

int teamplay_MainMenuCallback(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (!ent->client)
		return 0;

	if (ent->client->menutype == 1)			// main menu
		teamplay_MainMenuChoice (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 2)	// class menu
		teamplay_ClassMenuChoice (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 3)	// team menu
		teamplay_TeamMenuChoice (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 4)	// general menu
		teamplay_GeneralMenuChoice (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 5)	// class list menu
		teamplay_ClassListMenuChoice (ent, ((menu_item_t *)selected->it)->itemvalue);
	
	return 0;
}

void teamplay_ClassListMenu (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	gsmod_Killmenu(ent);

	ent->client->menutype = 5;
	menulink = CreateMenu(ent, "Class List Menu");

	if (!((int)(botsclass->value) & BOTS_BODYGUARD))
		AddMenuItem(menulink, "Bodyguard", " - ", 2, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_SNIPER))
		AddMenuItem(menulink, "Sniper", " - ", 3, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_SOLDIER))
		AddMenuItem(menulink, "Soldier", " - ", 4, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_BERZERKER))
		AddMenuItem(menulink, "Berzerker", " - ", 5, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_INFILTRATOR))
		AddMenuItem(menulink, "Infiltrator", " - ", 6, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_KAMIKAZEE))
		AddMenuItem(menulink, "Kamikazee", " - ", 7, &teamplay_MainMenuCallback);
	if (!((int)(botsclass->value) & BOTS_NURSE))
		AddMenuItem(menulink, "Nurse", " - ", 8, &teamplay_MainMenuCallback);

	AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	FinishMenu(ent, menulink);
}


void teamplay_ClassMenu (edict_t *ent, qboolean isauto)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	if (!isauto)
	{
		gsmod_Killmenu(ent);
	}

	ent->client->menutype = 2;

	if (ent->client->pers.player_class == 1)
	{
		menulink = CreateMenu(ent, "Captain Commands Menu");
		AddMenuItem(menulink, "Create items", " - create", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Promote player", " - promote", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Demote player", " - demote", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Scout info", " - scout", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Scout to team", " - scoutall", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Send alarm", " - sendalarm", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Use tracker", " - tracker", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Drop promo", " - droppromote", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 2)
	{
		menulink = CreateMenu(ent, "Bodyguard Commands Menu");
		AddMenuItem(menulink, "Create a laser", " - laser", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Turn laser on", " - laseron", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Turn laser off", " - laseroff", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Destroy laser", " - laserkill", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Decoy glow", " - decoy", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 3)
	{
		menulink = CreateMenu(ent, "Sniper Commands Menu");
		AddMenuItem(menulink, "Create an alarm", " - alarm", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Destroy alarm", " - alarmkill", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Send alarm", " - sendalarm", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Use laser sight", " - sight", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Use tracker", " - tracker", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Use flare", " - flare", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 4)
	{
		menulink = CreateMenu(ent, "Soldier Commands Menu");
		if (ent->client->pers.classlevel > 2)
			AddMenuItem(menulink, "Split rockets", " - splitmode", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 5)
	{
		menulink = CreateMenu(ent, "Berzerker Commands Menu");
		AddMenuItem(menulink, "Invincible rage", " - rage", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 6)
	{
		menulink = CreateMenu(ent, "Infiltrator Commands Menu");
		AddMenuItem(menulink, "Start stealing", " - steal", -1, &teamplay_MainMenuCallback);
		if (ent->client->pers.classlevel > 1)
			AddMenuItem(menulink, "Use a disguise", " - disguise", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 7)
	{
		menulink = CreateMenu(ent, "Kamikazee Commands Menu");
		AddMenuItem(menulink, "Commit suicide", " - suicide", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Use detpipes", " - detmode", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	else if (ent->client->pers.player_class == 8)
	{
		menulink = CreateMenu(ent, "Nurse Commands Menu");
		AddMenuItem(menulink, "Poison health", " - poison", -1, &teamplay_MainMenuCallback);
		AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	}
	FinishMenu(ent, menulink);
}

void teamplay_TeamMenu (edict_t *ent, qboolean isauto)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	if (!isauto)
	{
		gsmod_Killmenu(ent);
	}

	ent->client->menutype = 3;
	menulink = CreateMenu(ent, "Team Commands Menu");
	if (ent->client->pers.autoid)
		AddMenuItem(menulink, "Disable Auto ID", " - autoid", -1, &teamplay_MainMenuCallback);
	else
		AddMenuItem(menulink, "Enable Auto ID", " - autoid", -1, &teamplay_MainMenuCallback);

	AddMenuItem(menulink, "Drop the flag", " - dropflag", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Impeach Captain", " - impeach", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Flag status", " - flagstatus", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Locate flag", " - locateflag", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Locate promo", " - locatepromo", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Team msg", " - messagemode2", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Your status", " - mystatus", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Player details", " - playerid", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Select new team", " - rejoin", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "View scores", " - teamscore", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Class menu", " - changeclass", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	FinishMenu(ent, menulink);
}

void teamplay_GeneralMenu (edict_t *ent, qboolean isauto)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	if (!isauto)
	{
		gsmod_Killmenu(ent);
	}

	ent->client->menutype = 4;
	menulink = CreateMenu(ent, "General Commands Menu");
	AddMenuItem(menulink, "Voting menu", " - vote", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Kill yourself", " - kill", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Welcome message", " - welcome", -1, &teamplay_MainMenuCallback);
	if (ent->client->pers.showhelp)
		AddMenuItem(menulink, "Help msgs off", " - helpoff", -1, &teamplay_MainMenuCallback);
	else
		AddMenuItem(menulink, "Help msgs on", " - helpon", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Gender message", " - gender", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Mute player", " - mute", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Unmute player", " - unmute", -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	FinishMenu(ent, menulink);
}

void teamplay_MainMenu (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	gsmod_Killmenu(ent);

	ent->client->menutype = 1;
	menulink = CreateMenu(ent, "B.o.t.S. Main Menu");
	
	if (ent->client->pers.player_class == 1)
		AddMenuItem(menulink, "Captain Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 2)
		AddMenuItem(menulink, "Bodyguard Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 3)
		AddMenuItem(menulink, "Sniper Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 4)
		AddMenuItem(menulink, "Soldier Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 5)
		AddMenuItem(menulink, "Berzerker Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 6)
		AddMenuItem(menulink, "Infiltrator Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 7)
		AddMenuItem(menulink, "Kamikazee Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	else if (ent->client->pers.player_class == 8)
		AddMenuItem(menulink, "Nurse Commands Menu", NULL, -1, &teamplay_MainMenuCallback);

	AddMenuItem(menulink, "Team Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "General Commands Menu", NULL, -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Team Radio Menu", NULL, -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Shouts Menu", NULL, -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Execute Class Script", NULL, -1, &teamplay_MainMenuCallback);
	AddMenuItem(menulink, "Cancel", NULL, -1, &teamplay_MainMenuCallback);
	FinishMenu(ent, menulink);
}


static void teamplay_FlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}


void teamplay_FlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("teamplay_FlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = teamplay_FlagThink;
}


qboolean teamplay_AddAmmo (edict_t *ent, int ammotag)
{
	if (!ent->client)
		return false;

	if (teamplay == 0)
		return true;
	
	if (ammotag == AMMO_GRENADES)
		return true;

	if ((ammotag == AMMO_SHELLS) && (!IsFemale(ent)))
		return true;

	if (ent->client->pers.player_class == 1)
	{
		if ((ammotag != AMMO_CELLS) && (ammotag != AMMO_ROCKETS))
			return false;
	}
	else if (ent->client->pers.player_class == 2)
	{
		if ((ammotag != AMMO_SHELLS) && (ammotag != AMMO_CELLS) && (ammotag != AMMO_ROCKETS))
			return false;
	}
	else if (ent->client->pers.player_class == 3)
	{
		if ((ammotag != AMMO_CELLS) && (ammotag != AMMO_SLUGS))
			return false;
	}
	else if (ent->client->pers.player_class == 4)
	{
		if ((ammotag != AMMO_ROCKETS) && (ammotag != AMMO_ROCKETS))
			return false;
	}
	else if (ent->client->pers.player_class == 5)
	{
		if ((ammotag != AMMO_BULLETS) && (ammotag != AMMO_ROCKETS))
			return false;
	}
	else if (ent->client->pers.player_class == 6)
	{
		if (ammotag != AMMO_CELLS)
			return false;
	}
	else if (ent->client->pers.player_class == 7)
	{
		if (ammotag != AMMO_CELLS)
			return false;
	}
	else if (ent->client->pers.player_class == 8)
	{
		if ((ammotag != AMMO_CELLS) && (ammotag != AMMO_BULLETS))
			return false;
	}

	return true;
}


void teamplay_ResetSkin (edict_t *ent)
{
	char	str[MAX_INFO_KEY], femalepic_name[12], malepic_name[12];
	char	skincmd[MAX_INFO_VALUE + 7] = "skin ";
	int		playernum;

	if (!ent->client)
		return;

	if (ent->client->pers.player_class == 6)
	{
		ent->client->special = false;
		ent->client->specialtimer = 0;
	}

	if ((teamplay == 1) && (ent->client->pers.player_class) && (ent->client->pers.team))
	{
		playernum = ent-g_edicts-1;
	
		// set the correct skin
		if (ent->client->pers.player_class == 1)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "capt", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "capt", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "athena");
				sprintf(malepic_name, "%s", "major");
			}
		}
		else if (ent->client->pers.player_class == 2)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "body", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "body", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "stiletto");
				sprintf(malepic_name, "%s", "flak");
			}
		}
		else if (ent->client->pers.player_class == 3)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "snip", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "snip", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "jungle");
				sprintf(malepic_name, "%s", "sniper");
			}
		}
		else if (ent->client->pers.player_class == 4)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "sol", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "sol", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "ensign");
				sprintf(malepic_name, "%s", "grunt");
			}
		}
		else if (ent->client->pers.player_class == 5)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "berz", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "berz", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "voodoo");
				sprintf(malepic_name, "%s", "howitzer");
			}
		}
		else if (ent->client->pers.player_class == 6)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "enf", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "enf", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "brianna");
				sprintf(malepic_name, "%s", "recon");
			}
		}
		else if (ent->client->pers.player_class == 7)
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "kami", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "kami", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "jezebel");
				sprintf(malepic_name, "%s", "psycho");
			}
		}
		else
		{
			if((int)(botsflags->value) & BOTS_TEAM_SKINS)
			{
				sprintf(femalepic_name, "f%s%s", "nurs", gsmod_TeamColor(ent->client->pers.team));
				sprintf(malepic_name, "m%s%s", "nurs", gsmod_TeamColor(ent->client->pers.team));
			}
			else
			{
				sprintf(femalepic_name, "%s", "venus");
				sprintf(malepic_name, "%s", "viper");
			}
		}
		
		if (teamplay == 1)
		{
			if (ent->client->pers.team == 1)
			{
				if (ent->health < 35)
					sprintf(str, "female/damage/%s", femalepic_name);
				else
					sprintf(str, "female/%s", femalepic_name);
			}
			else
			{
				if (ent->health < 35)
					sprintf(str, "male/damage/%s", malepic_name);
				else
					sprintf(str, "male/%s", malepic_name);
			}
		}
		else
		{
			if (IsFemale(ent))
				sprintf(str, "female/%s", femalepic_name);
			else
				sprintf(str, "male/%s", malepic_name);
		}

		if (Q_stricmp (str, ent->client->pers.oldskin) == 0)
			return;

		strcpy(ent->client->pers.oldskin, str);
//		if (!ent->stuffskin) 
//		{
//			Info_SetValueForKey (userinfo, "skin", ent->client->pers.oldskin);
			Info_SetValueForKey (ent->client->pers.userinfo, "skin", ent->client->pers.oldskin);
			ent->stuffskin = true; 
//		}
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, str) );
//		strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
	}
}

void teamplay_UseDisguise (edict_t *ent, int player_class)
{
	char	str[MAX_INFO_KEY], femalepic_name[12], malepic_name[12];
	char	skincmd[MAX_INFO_VALUE + 7] = "skin ";
	int		enemyteam, playernum;

	if (player_class == 0)
		return;

	if (!ent->client)
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 25)
	{
		gsutil_centerprint(ent, "%s", "You need at least 25 cells\nto use a disguise!\n");
		return;
	}
	else
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 25;

	if (ent->client->pers.team == 1)
		enemyteam = 2;
	else
		enemyteam = 1;

	playernum = ent-g_edicts-1;

	// set the correct skin
	if (player_class == 1)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "capt", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "capt", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "athena");
			sprintf(malepic_name, "%s", "major");
		}
	}
	else if (player_class == 2)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "body", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "body", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "stiletto");
			sprintf(malepic_name, "%s", "flak");
		}
	}
	else if (player_class == 3)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "snip", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "snip", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "jungle");
			sprintf(malepic_name, "%s", "sniper");
		}
	}
	else if (player_class == 4)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "sol", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "sol", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "ensign");
			sprintf(malepic_name, "%s", "grunt");
		}
	}
	else if (player_class == 5)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "berz", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "berz", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "voodoo");
			sprintf(malepic_name, "%s", "howitzer");
		}
	}
	else if (player_class == 6)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "enf", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "enf", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "brianna");
			sprintf(malepic_name, "%s", "recon");
		}
	}
	else if (player_class == 7)
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "kami", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "kami", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "jezebel");
			sprintf(malepic_name, "%s", "psycho");
		}
	}
	else
	{
		if((int)(botsflags->value) & BOTS_TEAM_SKINS)
		{
			sprintf(femalepic_name, "f%s%s", "nurs", gsmod_TeamColor(enemyteam));
			sprintf(malepic_name, "m%s%s", "nurs", gsmod_TeamColor(enemyteam));
		}
		else
		{
			sprintf(femalepic_name, "%s", "venus");
			sprintf(malepic_name, "%s", "viper");
		}
	}
	
	if (enemyteam == 2)
		sprintf(str, "male/%s", malepic_name);
	else
		sprintf(str, "female/%s", femalepic_name);

	strcpy(ent->client->pers.oldskin, str);
	if (!ent->stuffskin) 
	{
//		Info_SetValueForKey (userinfo, "skin", ent->client->pers.oldskin);
		Info_SetValueForKey (ent->client->pers.userinfo, "skin", ent->client->pers.oldskin);
		ent->stuffskin = true; 
	}
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, str) );
	ent->client->special = true;
	ent->client->specialtimer = level.time + 30.0;
//	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


qboolean teamplay_HasLeader (int team)
{
	edict_t		*player;
	int			n;

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team != team)
			continue; 

		if (player->client->pers.player_class == 1)
			return true; 
	}

	return false;
}


edict_t *teamplay_GetLeader (int team)
{
	edict_t		*player;
	int			n;
	qboolean	lb_return;

	lb_return = false;
	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team != team)
			continue; 
		if (player->client->pers.player_class != 1)
			continue; 

		return player;
	}

	return NULL;
}


void teamplay_CaptainKey(edict_t *ent)
{
	int			index;
	gitem_t		*it;

	if (!ent->client)
		return;
	
	if (teamkeys[ent->client->pers.team - 1].capkeydropped)
		return;
	
	if (ent->client->pers.team == 1)
	{
		it = FindItemByClassname("key_promotion_blue");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key");
			return;
		}
	}
	else if (ent->client->pers.team == 2)
	{
		it = FindItemByClassname("key_promotion_red");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key");
			return;
		}
	}
	else if (ent->client->pers.team == 3)
	{
		it = FindItemByClassname("key_promotion_green");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key");
			return;
		}
	}
	
	index = ITEM_INDEX(it);
	ent->client->pers.inventory[index] = 1;
}


void teamplay_ResetCaptain (int team)
{
	edict_t		*player;
	int			n;

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team != team)
			continue; 
		if (player->client->pers.player_class != 1)
			continue; 

		teamplay_CaptainKey(player);
		return;
	}
}


void teamplay_ResetLifelink(edict_t *ent)
{
	edict_t		*player;
	int			n;

	if (!ent->client)
		return;

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team != ent->client->pers.team)
			continue; 
		if (player->client->pers.player_class != 2)
			continue;
		if (player->client->lifelink)
		{
			teams.hasprotect[ent->client->pers.team-1] = true;
			return;
		}
	}

	teams.hasprotect[ent->client->pers.team-1] = false;
}


void teamplay_LocatePromoKey (edict_t *ent, qboolean isauto)
{
	edict_t *promokey;
	edict_t *leader;
	char	stats[500];
	int		j;
	vec3_t	v;
	float	len;
	gitem_t  *it;

	if (teamplay == 0)
		return;

	if (!ent->client)
		return;

	if (teamplay_HasLeader(ent->client->pers.team) == false)
	{
		if (isauto == false)
			gsutil_centerprint(ent, "%s", "You do not have a captain!\n");
		return;
	}
	
	for (promokey=g_edicts; promokey < &g_edicts[globals.num_edicts]; promokey++)
	{
		if (!promokey->inuse)
			continue;

		if (ent->client->pers.team == 1)
		{
			if (Q_stricmp (promokey->classname, "key_promotion_blue") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, promokey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Blue Promotion Key distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
		else if (ent->client->pers.team == 2)
		{
			if (Q_stricmp (promokey->classname, "key_promotion_red") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, promokey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Red Promotion Key distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
		else if (ent->client->pers.team == 3)
		{
			if (Q_stricmp (promokey->classname, "key_promotion_green") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, promokey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Green Promotion Key distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
	}

	leader = teamplay_GetLeader(ent->client->pers.team);
	if (leader)
	{
		if (ent->client->pers.team == 1)
			it = FindItemByClassname("key_promotion_blue");
		else if (ent->client->pers.team == 2)
			it = FindItemByClassname("key_promotion_red");
		else if (ent->client->pers.team == 3)
			it = FindItemByClassname("key_promotion_green");

		if (leader->client->pers.inventory[ITEM_INDEX(it)] == 1)
		{
			if (isauto == false)
			{
				j = sprintf(stats, "Captain %s is carrying\nyour Promotion Key\n", leader->client->pers.netname);
				gsutil_centerprint(ent, "%s", stats);
			}
			return;
		}
	}

	// Auto create new promo key here
	teamkeys[ent->client->pers.team-1].capkeydropped = false;
	teamplay_ResetCaptain(ent->client->pers.team);

//	if (isauto == false)
//		gsutil_centerprint(ent, "%s", "Application Error!\nCannot locate promotion key!\n");
}


void teamplay_LocateKey (edict_t *ent, qboolean isauto)
{
	edict_t *teamkey;
	edict_t *player;
	char	stats[500];
	int		j, n, keyindex;
	vec3_t	v;
	float	len;
	gitem_t  *it;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;

	keyindex = ent->client->pers.team;

	for (teamkey=g_edicts; teamkey < &g_edicts[globals.num_edicts]; teamkey++)
	{
		if (!teamkey->inuse)
			continue;

		if (ent->client->pers.team == 1)
		{
			if (Q_stricmp (teamkey->classname, "key_blueteam") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, teamkey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Blue Flag distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
		else if (ent->client->pers.team == 2)
		{
			if (Q_stricmp (teamkey->classname, "key_redteam") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, teamkey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Red Flag distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
		else if (ent->client->pers.team == 3)
		{
			if (Q_stricmp (teamkey->classname, "key_greenteam") == 0)
			{
				if (isauto == false)
				{
					VectorSubtract (ent->s.origin, teamkey->s.origin, v);
					len = VectorLength (v);
					j = sprintf(stats, "Green Key distance: %5.0f\n", len);
					gsutil_centerprint(ent, "%s", stats);
				}
				return;
			}
		}
	}

	if (keyindex == 1)
		it = FindItemByClassname("key_blueteam");
	else if (keyindex == 2)
		it = FindItemByClassname("key_redteam");
	else if (keyindex == 3)
		it = FindItemByClassname("key_greenteam");

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player->client->pers.team == keyindex)
			continue; 

		if (player->client->pers.inventory[ITEM_INDEX(it)] == 1)
		{
			if (isauto == false)
			{
				j = sprintf(stats, "ENEMY %s has your flag!\n", player->client->pers.netname);
				gsutil_centerprint(ent, "%s", stats);
			}
			return;
		}
	}

	// Auto create new key here
	teamplay_SpawnKey(ent, keyindex, false, false);
	teamkeys[keyindex-1].istaken = false;
	teamkeys[keyindex-1].isdropped = false;

//	if (isauto == false)
//		gsutil_centerprint(ent, "%s", "Application Error!\nCannot locate key!\n");
}


void teamplay_AppendBar (char *pszLayout, edict_t *viewer, edict_t *ent)
{
	char	szEntry[STRING_CHARS];
	int		horizPosition;

	
	if (!viewer->client)
		return;

	if ((ent == NULL) && (!viewer->client->observer))
		return; 

	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (viewer->client->observer)
		{
			if (ent == NULL)
			{
				horizPosition = 160 - 4 * 9;

				Com_sprintf(szEntry, sizeof(szEntry), "xv %d yb -60 string2 \"Observing\" ", 
					horizPosition);
			}
			else
			{
				horizPosition = 160 - 4 * strlen(ent->client->pers.netname);

				Com_sprintf(szEntry, sizeof(szEntry), "xv %d yb -60 string2 \"Chasing %s\" ", 
					horizPosition, ent->client->pers.netname);
			}
		}
		else if (viewer->client->pers.team != ent->client->pers.team)
		{
			horizPosition = 160 - 4 * strlen(ent->client->pers.netname);

			Com_sprintf(szEntry, sizeof(szEntry), "xv %d yb -60 string2 \"ENEMY %s\" ", 
				horizPosition, ent->client->pers.netname);
		}
		else
		{
			horizPosition = 160 - 4 * strlen(ent->client->pers.netname);

			Com_sprintf(szEntry, sizeof(szEntry), "xv %d yb -60 string2 \"FRIENDLY %s\" ", 
				horizPosition, ent->client->pers.netname);
		}
	}
	else
	{
		horizPosition = 160 - 4 * strlen(ent->client->pers.netname);

		Com_sprintf(szEntry, sizeof(szEntry), "xv %d yb -60 string2 \"%s\" ", 
			horizPosition, ent->client->pers.netname);
	}

	if (strlen(szEntry) + strlen(pszLayout) > LAYOUT_SAFE) return;
	
	strcat (pszLayout, szEntry);
}


void teamplay_FragBoard (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[3][MAX_CLIENTS];
	int		sortedscores[3][MAX_CLIENTS];
	int		score, total[3], totalscore[3];
	int		last[3];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	if (!ent->client)
		return;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if ((!cl_ent->inuse) || (!cl_ent->client))
			continue;
		if ((cl_ent->client->observer) || (!cl_ent->client->pers.player_class))
			continue;
		if (game.clients[i].pers.team == 1)
			team = 0;
		else if (game.clients[i].pers.team == 2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].pers.kills;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}

		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	ent->client->ps.stats[STAT_TEAM1_CAPS] = teams.team_score[0];
	ent->client->ps.stats[STAT_TEAM2_CAPS] = teams.team_score[1];

	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 84 yv 12 num 3 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 244 yv 12 num 3 20 ",
		teams.team_frags[0], total[0],
		teams.team_frags[1], total[1]);

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];
		
			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 0 %d %d %d %d ",
						42 + i * 8,
						sorted[0][i],
						cl->pers.kills,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
						sprintf(entry + strlen(entry), "xv 56 yv %d picn sbctf2 ", 42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[0] = i;
					}
				}
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 160 %d %d %d %d ",
						42 + i * 8,
						sorted[1][i],
						cl->pers.kills,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
						sprintf(entry + strlen(entry), "xv 216 yv %d picn sbctf1 ",	42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[1] = i;
					}
				}
			}
		}
	}

	// put in observers if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			
			if (!cl_ent->inuse || !cl_ent->client ||
				cl_ent->solid != SOLID_NOT ||
				!cl_ent->client->observer)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Observers\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->pers.kills,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void teamplay_CaptureBoard (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[3][MAX_CLIENTS];
	int		sortedscores[3][MAX_CLIENTS];
	int		score, total[3], totalscore[3];
	int		last[3];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	if (!ent->client)
		return;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if ((!cl_ent->inuse) || (!cl_ent->client))
			continue;
		if ((cl_ent->client->observer) || (!cl_ent->client->pers.player_class))
			continue;
		if (game.clients[i].pers.team == 1)
			team = 0;
		else if (game.clients[i].pers.team == 2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].pers.captures;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}

		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	ent->client->ps.stats[STAT_TEAM1_CAPS] = teams.team_score[0];
	ent->client->ps.stats[STAT_TEAM2_CAPS] = teams.team_score[1];

	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 84 yv 12 num 3 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 244 yv 12 num 3 20 ",
		teams.team_frags[0], total[0],
		teams.team_frags[1], total[1]);

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];
		
			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 0 %d %d %d %d ",
						42 + i * 8,
						sorted[0][i],
						cl->pers.captures,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
						sprintf(entry + strlen(entry), "xv 56 yv %d picn sbctf2 ", 42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[0] = i;
					}
				}
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 160 %d %d %d %d ",
						42 + i * 8,
						sorted[1][i],
						cl->pers.captures,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
						sprintf(entry + strlen(entry), "xv 216 yv %d picn sbctf1 ",	42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[1] = i;
					}
				}
			}
		}
	}

	// put in observers if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			
			if (!cl_ent->inuse || !cl_ent->client ||
				cl_ent->solid != SOLID_NOT ||
				!cl_ent->client->observer)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Observers\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->pers.captures,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void teamplay_ScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[3][MAX_CLIENTS];
	int		sortedscores[3][MAX_CLIENTS];
	int		score, total[3], totalscore[3];
	int		last[3];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	if (!ent->client)
		return;

	ent->client->scoreboard = 0;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if ((!cl_ent->inuse) || (!cl_ent->client))
			continue;
		if ((cl_ent->client->observer) || (!cl_ent->client->pers.player_class))
			continue;
		if (game.clients[i].pers.team == 1)
			team = 0;
		else if (game.clients[i].pers.team == 2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}

		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	ent->client->ps.stats[STAT_TEAM1_CAPS] = teams.team_score[0];
	ent->client->ps.stats[STAT_TEAM2_CAPS] = teams.team_score[1];

//	// team one
//	sprintf(string, 
//		"xv 0 yv 0 cstring \"%s (%s)\" "
//		"xv 8 yv 16 string \"%s\" "
//		"xv 40 yv 28 string \"%4d/%-3d\" "
//		"xv 98 yv 12 num 2 18 "
//		"xv 168 yv 16 string \"%s\" "
//		"xv 200 yv 28 string \"%4d/%-3d\" "
//		"xv 256 yv 12 num 2 20 ",
//		level.level_name, level.mapname,
//		team1_desc->string, 
//		teams.team_frags[0], total[0],
//		team2_desc->string,
//		teams.team_frags[1], total[1]);

	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 84 yv 12 num 3 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 244 yv 12 num 3 20 ",
		teams.team_frags[0], total[0],
		teams.team_frags[1], total[1]);

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0]) 
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];
		
			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 0 %d %d %d %d ",
						42 + i * 8,
						sorted[0][i],
						cl->resp.score,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
						sprintf(entry + strlen(entry), "xv 56 yv %d picn sbctf2 ", 42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[0] = i;
					}
				}
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			if ((cl_ent->inuse) && (cl_ent->client))
			{
				if ((!cl_ent->client->observer) && (cl_ent->client->pers.player_class))
				{
					sprintf(entry+strlen(entry),
						"ctf 160 %d %d %d %d ",
						42 + i * 8,
						sorted[1][i],
						cl->resp.score,
						cl->ping > 999 ? 999 : cl->ping);

					if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
						sprintf(entry + strlen(entry), "xv 216 yv %d picn sbctf1 ",	42 + i * 8);

					if (maxsize - len > strlen(entry)) 
					{
						strcat(string, entry);
						len = strlen(string);
						last[1] = i;
					}
				}
			}
		}
	}

	// put in observers if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			
			if (!cl_ent->inuse || !cl_ent->client ||
				cl_ent->solid != SOLID_NOT)
				continue;
			if (!cl_ent->client->observer)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Observers\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}


static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}


static void teamplay_SetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d, len;
	int i;

	if (!ent->client)
		return;

	ent->client->ps.stats[STAT_ID_VIEW] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	
	if (tr.fraction < 1 && tr.ent && tr.ent->client) 
	{
		ent->client->ps.stats[STAT_ID_VIEW] = 
			CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	
	for (i = 1; i <= maxclients->value; i++) 
	{
		who = g_edicts + i;
		if (!who->inuse)
			continue;
		if (who->client)
		{
			if (who->client->invisible)
				continue;
		}

		VectorSubtract(who->s.origin, ent->s.origin, dir);
		len = VectorLength (dir);
		if (len > 200)
			continue;

		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	
	if (bd > 0.90)
	{
		ent->inview = best;
		ent->client->ps.stats[STAT_ID_VIEW] = CS_PLAYERSKINS + (best - g_edicts - 1);
	}
	else
		ent->inview = NULL;
}


void teamplay_SetStats (edict_t *ent)
{
//	int i;
	int p1, p2;
	edict_t *e;

	if (!ent->client)
		return;

	// logo headers for the frag display
	ent->client->ps.stats[STAT_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	ent->client->ps.stats[STAT_TEAM2_HEADER] = gi.imageindex ("ctfsb2");

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (teams.team_score[0] > teams.team_score[1])
			ent->client->ps.stats[STAT_TEAM1_HEADER] = 0;
		else if (teams.team_score[1] > teams.team_score[0])
			ent->client->ps.stats[STAT_TEAM2_HEADER] = 0;
		else if (teams.team_frags[0] > teams.team_frags[1]) // frag tie breaker
			ent->client->ps.stats[STAT_TEAM1_HEADER] = 0;
		else if (teams.team_frags[1] > teams.team_frags[0]) 
			ent->client->ps.stats[STAT_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_TEAM2_HEADER] = 0;
		}
	}

//	// tech icon
//	i = 0;
//	ent->client->ps.stats[STAT_TECH] = 0;
//	while (tnames[i]) {
//		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
//			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
//			ent->client->ps.stats[STAT_TECH] = gi.imageindex(tech->icon);
//			break;
//		}
//		i++;
//	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = gi.imageindex ("i_ctf1");
	e = G_Find(NULL, FOFS(classname), "key_blueteam");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p1 = gi.imageindex ("i_ctf1d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)]) {
					// enemy has it
					p1 = gi.imageindex ("i_ctf1t");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p1 = gi.imageindex ("i_ctf1d"); // must be dropped
	}
	p2 = gi.imageindex ("i_ctf2");
	e = G_Find(NULL, FOFS(classname), "key_redteam");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p2 = gi.imageindex ("i_ctf2d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)]) {
					// enemy has it
					p2 = gi.imageindex ("i_ctf2t");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p2 = gi.imageindex ("i_ctf2d"); // must be dropped
	}


	ent->client->ps.stats[STAT_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_TEAM2_PIC] = p2;

	if (lastcapteam && level.time - lastcaptime < 5) 
	{
		if (lastcapteam == 1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_TEAM1_PIC] = 0;
		else
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_TEAM2_PIC] = 0;
	}

	ent->client->ps.stats[STAT_TEAM1_CAPS] = teams.team_score[0];
	ent->client->ps.stats[STAT_TEAM2_CAPS] = teams.team_score[1];

	ent->client->ps.stats[STAT_FLAG_PIC] = 0;
	if (ent->client->pers.team == 1 &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_FLAG_PIC] = gi.imageindex ("i_ctf2");

	else if (ent->client->pers.team == 2 &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_FLAG_PIC] = gi.imageindex ("i_ctf1");

	ent->client->ps.stats[STAT_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_JOINED_TEAM2_PIC] = 0;
	if (ent->client->pers.team == 1)
		ent->client->ps.stats[STAT_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
	else if (ent->client->pers.team == 2)
		ent->client->ps.stats[STAT_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");

//	if ((ent->client->pers.autoid) && (level.framenum & 8))
	ent->client->ps.stats[STAT_ID_VIEW] = 0;
	if (ent->client->pers.autoid)
		teamplay_SetIDView(ent);
}


edict_t *teamplay_GetSpot(void)
{
	edict_t *spot;
	int		i;
	
	// find an intermission spot
	spot = intermission_spot;
	if (spot)
	{
		if (mapmode == 1)
		{
			if ((rand() & 2) < 1)
				intermission_spot = G_Find (spot, FOFS(classname), teamkeys[0].playerstart);
			else
				intermission_spot = G_Find (spot, FOFS(classname), teamkeys[1].playerstart);
		}
		else if (mapmode == 2)
		{
			if ((rand() & 2) < 1)
				intermission_spot = G_Find (spot, FOFS(classname), teamkeys[0].playerstart2);
			else
				intermission_spot = G_Find (spot, FOFS(classname), teamkeys[1].playerstart2);
		}
	}
	else
	{
		if (mapmode == 1)
		{
			if ((rand() & 2) < 1)
				spot = G_Find (NULL, FOFS(classname), teamkeys[0].playerstart);
			else
				spot = G_Find (NULL, FOFS(classname), teamkeys[1].playerstart);
		}
		else if (mapmode == 2)
		{
			if ((rand() & 2) < 1)
				spot = G_Find (NULL, FOFS(classname), teamkeys[0].playerstart2);
			else
				spot = G_Find (NULL, FOFS(classname), teamkeys[1].playerstart2);
		}
		
		if (spot)
		{
			if (mapmode == 1)
			{
				if ((rand() & 2) < 1)
					intermission_spot = G_Find (spot, FOFS(classname), teamkeys[0].playerstart);
				else
					intermission_spot = G_Find (spot, FOFS(classname), teamkeys[1].playerstart);
			}
			else if (mapmode == 2)
			{
				if ((rand() & 2) < 1)
					intermission_spot = G_Find (spot, FOFS(classname), teamkeys[0].playerstart2);
				else
					intermission_spot = G_Find (spot, FOFS(classname), teamkeys[1].playerstart2);
			}
		}
		else
		{
			spot = G_Find (NULL, FOFS(classname), "info_player_intermission");
			if (!spot)
			{	
				// the map creator forgot to put in an intermission point...
				spot = G_Find (intermission_spot, FOFS(classname), "info_player_start");
				if (!spot)
					spot = G_Find (intermission_spot, FOFS(classname), "info_player_deathmatch");
			}
			else
			{	
				// chose one of four spots
				i = rand() & 3;
				while (i--)
				{
					spot = G_Find (spot, FOFS(classname), "info_player_intermission");
					if (!spot)	// wrap around the list
						spot = G_Find (spot, FOFS(classname), "info_player_intermission");
				}
			}
		}
	}

	return spot;
}

void teamplay_ObserverMode(edict_t *ent)
{
	edict_t *spot;
	
	if (!ent->client)
		return;

	spot = teamplay_GetSpot();

	VectorCopy (spot->s.origin, level.intermission_origin);
	VectorCopy (spot->s.angles, level.intermission_angle);

	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_SPECTATOR;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_NOCLIP;

	ent->client->observer = true;
	ent->client->pers.startpause = level.time + 1.0;
}


qboolean teamplay_MapCheck(edict_t *mapent, edict_t *player)
{
	if (teamplay == 1)
	{
		if (player->client)
		{
			if ((player->client->pers.player_class == 6) && (player->client->pers.classlevel > 2))
			{
				if ((mapent->bots_team == player->client->pers.team) && (mapent->bots_class == 1))
					return false;
				else
					return true;
			}
			
			if (mapent->bots_team) 
			{
				if ((player->client->pers.team != mapent->bots_team) && (mapent->bots_class == 1) && (player->client->pers.player_class == 1) && (player->client->pers.classlevel == 0))
					return true;

				if (player->client->pers.team != mapent->bots_team)
					return false;
				else
				{
					if ((mapent->bots_class) && (player->client->pers.player_class != mapent->bots_class))
						return false;
				}
			}
			else
			{
				if ((mapent->bots_class) && (player->client->pers.player_class != mapent->bots_class))
					return false;
			}
		}
	}

	return true;
}


edict_t *teamplay_SelectFarthestKeySpawnPoint (int keyindex)
{
	edict_t	*bestspot;
	edict_t	*spot;
	vec3_t	v;
	float	bestdistance, bestkeydistance;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if (keyindex == 2)
		{
			VectorSubtract (spot->s.origin, teamkeys[0].spawnspot->s.origin, v);
			bestkeydistance = VectorLength (v);
			if (bestkeydistance < 100)
				continue;
		}
		else if (keyindex == 3)
		{
			VectorSubtract (spot->s.origin, teamkeys[0].spawnspot->s.origin, v);
			bestkeydistance = VectorLength (v);
			if (bestkeydistance < 100)
				continue;

			VectorSubtract (spot->s.origin, teamkeys[1].spawnspot->s.origin, v);
			bestkeydistance = VectorLength (v);
			if (bestkeydistance < 100)
				continue;
		}

		if (bestkeydistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestkeydistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}


void teamplay_DropGeneral (edict_t *ent, gitem_t *item, edict_t *dropped)
{
	int keyindex;
	
	if (!ent->client)
		return;

	if ((teamplay == 0) || (numberteams > 3))
		return;

	if ( (!strcmp (item->pickup_name, "Blue Team Flag")) || (!strcmp (item->pickup_name, "Red Team Flag")) || (!strcmp (item->pickup_name, "Green Team Key")) ||
	     (!strcmp (item->pickup_name, "Promotion Key")) )
	{
//		dropped->s.effects &= ~(EF_COLOR_SHELL);
//		dropped->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
//		dropped->s.effects |= EF_COLOR_SHELL;

		keyindex = ent->client->keyindex - 1;
		if ((!strcmp (item->pickup_name, "Blue Team Flag")) || (!strcmp (item->pickup_name, "Red Team Flag")) || (!strcmp (item->pickup_name, "Green Team Key")) )
		{
			teamkeys[keyindex].istaken = false;
			teamkeys[keyindex].isdropped = true;
			teamkeys[keyindex].keyent = dropped;
			ent->client->keyindex = 0;
		}
	    else
		{
			teamkeys[keyindex].capkeydropped = true;
			keyindex = ent->client->pers.team - 1;
		}

		dropped->flags = keyindex + 1;
//		dropped->s.renderfx |= teamkeys[keyindex].keyeffect;
	}
}

	
void SP_misc_teleporter_dest (edict_t *ent);

void SP_info_player_teamdeathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (newmapentities == false)
		SP_misc_teleporter_dest (self);
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Use_Item (edict_t *ent, edict_t *other, edict_t *activator);
void teamplay_KeyThink (edict_t *ent)
{
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	
	ent->touch = Touch_Item;

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & 2)	// NO_TOUCH
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & 1)	// TRIGGER_SPAWN
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


qboolean teamplay_FairTeams (int playerteam)
{
	int		i, teamcount, pcount[4];
	edict_t *player;

	if ((numberteams > 3) || (numberteams < 2))
		return true;
	
	if((int)(botsflags->value) & BOTS_FAIR_TEAMS)
	{
		// Continue
	}
	else
		return true;
	
	pcount[0] = 0;
	for (teamcount=1 ; teamcount<4 ; teamcount++)
	{
		pcount[teamcount] = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client) 
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->pers.team != teamcount)
				continue; 

			pcount[teamcount]++;
		}
	}

	if (numberteams == 2)
	{
		if (playerteam == 1)
		{
			if (pcount[2] == 0)
				return false;
			else if ((pcount[1] - pcount[2]) > 1)
				return false;
		}
		else
		{
			if (pcount[1] == 0)
				return false;
			else if ((pcount[2] - pcount[1]) > 1)
				return false;
		}
	}
	else
	{
		if (playerteam == 1)
		{
			if (pcount[2] == 0)
				return false;
			else if (pcount[3] == 0)
				return false;
			else if ((pcount[1] - pcount[2]) > 1)
				return false;
			else if ((pcount[1] - pcount[3]) > 1)
				return false;
		}
		else if (playerteam == 2)
		{
			if (pcount[1] == 0)
				return false;
			else if (pcount[3] == 0)
				return false;
			else if ((pcount[2] - pcount[1]) > 1)
				return false;
			else if ((pcount[2] - pcount[3]) > 1)
				return false;
		}
		else
		{
			if (pcount[1] == 0)
				return false;
			else if (pcount[2] == 0)
				return false;
			else if ((pcount[3] - pcount[1]) > 1)
				return false;
			else if ((pcount[3] - pcount[2]) > 1)
				return false;
		}
	}

	return true;
}


void teamplay_TouchGoal (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		i, li_bonus;
	edict_t	*player;
	gitem_t	*it;

	if (!other->client)
		return;

	else if ((other->client->keyindex > 0) && (other->client->keyindex != other->client->pers.team) && (ent->flags == other->client->pers.team))
	{
		if (ent->conquer)
		{
			if (ent->conquer->client)
			{
				if (ent->conquer->client->pers.team != other->client->pers.team)
				{
					if (level.time > fairwarning)
					{
						gsutil_centerprint(other, "This goal has been conquered\nby %s", ent->conquer->client->pers.netname);
						fairwarning = level.time + 2.0;
					}
					return;
				}
			}
		}

		if (!teamplay_FairTeams(other->client->pers.team))
			gi.bprintf (PRINT_MEDIUM,"%s's team has too many players! No points earned!\n", other->client->pers.netname);
		else
		{
			teamkeys[other->client->keyindex-1].lastcapturer = other;

			gi.bprintf (PRINT_MEDIUM,"%s captured the %s!\n", other->client->pers.netname, teamkeys[other->client->keyindex-1].keyname);
			other->client->pers.captures++;

			li_bonus = 1;
			if (teams.hasleader[other->client->pers.team-1] == true)
			{
				if (ent->bots_points)
					teams.team_score[other->client->pers.team-1] += ent->bots_points;
				else
					teams.team_score[other->client->pers.team-1] += 3;

				if ((teamkeys[other->client->pers.team-1].isdropped) || (teamkeys[other->client->pers.team-1].istaken))
				{
					if (ent->bots_promos)
						teams.team_promos[other->client->pers.team-1] += ent->bots_promos;
					else
						teams.team_promos[other->client->pers.team-1]++;

					other->client->resp.score++;
				}
				else
				{
					if (ent->bots_promos)
						teams.team_promos[other->client->pers.team-1] += ent->bots_promos;
					else
						teams.team_promos[other->client->pers.team-1] += 2;

					other->client->resp.score += 2;
					li_bonus = 2;
				}
			}
			else
			{
				teams.team_promos[other->client->pers.team-1] = 0;

				if (ent->bots_points)
					teams.team_score[other->client->pers.team-1] += ent->bots_points;
				else
					teams.team_score[other->client->pers.team-1] += 2;
				other->client->resp.score++;
			}

			if (ent->bots_sound)
				gi.sound(other, CHAN_AUTO, gi.soundindex(ent->bots_sound), 1, ATTN_NONE, 0);
			else
				gi.sound(other, CHAN_AUTO, gi.soundindex("world/xian1.wav"), 1, ATTN_NONE, 0);

			// flash the screen
			other->client->bonus_alpha = 0.25;	
			
			// check for team blowing out another team
			teams.team_capsinarow[other->client->pers.team-1]++;
			for (i=1 ; i<4 ; i++)
			{
				if (i != other->client->pers.team)
					teams.team_capsinarow[i-1] = 0;
			}

			if (teams.team_capsinarow[other->client->pers.team-1] == 3)
			{
				if((int)(botsflags->value) & BOTS_QUAD_RALLY)
				{
					// opposing teams get quad rally
					centerprint_all("Quad Rally!!!!");
					teams.team_capsinarow[other->client->pers.team-1] = 0;

					for_each_player(player,i)
					{
						if (player->client->pers.team != other->client->pers.team)
						{
							it = FindItem("Quad Damage");
							player->client->pers.inventory[ITEM_INDEX(it)]++;
							Use_Quad (player, it);

							if (player->client->pers.classlevel < 2)
								player->client->pers.classlevel++;
						}
					}
				}
			}
		}

		lastcapteam = other->client->pers.team;
		lastcaptime = level.time;

		teamkeys[other->client->keyindex-1].isdropped = true;
		teamkeys[other->client->keyindex-1].istaken = false;
		teamkeys[other->client->keyindex-1].keyent->nextthink = 0;

		if (other->client->keyindex == 1)
			other->client->pers.inventory[ITEM_INDEX(FindItem("Blue Team Flag"))]--;
		else if (other->client->keyindex == 2)
			other->client->pers.inventory[ITEM_INDEX(FindItem("Red Team Flag"))]--;
		else if (other->client->keyindex == 3)
			other->client->pers.inventory[ITEM_INDEX(FindItem("Green Team Key"))]--;

		other->client->keyindex = 0;
	}

	if (other->client->pers.player_class == 4)
	{
		if (other->client->special == true)
		{
			if (ent->flags != other->client->pers.team)
			{
				if (ent->conquer == other)
				{
					// Do nothing
				}
				else if (ent->conquer)
				{
					if (level.time > fairwarning)
					{
						gsutil_centerprint(other, "This goal has been conquered\nby %s", ent->conquer->client->pers.netname);
						fairwarning = level.time + 2.0;
					}
				}
				else
				{
					ent->conquer = other;
					other->conquer = ent;
					gi.bprintf (PRINT_MEDIUM,"%s has conquered an enemy capture pad!\n", other->client->pers.netname);
				}
			}
		}
	}
}


void teamplay_CycleGoal (edict_t *ent)
{
	if (ent->flags == 1) 
		ent->flags = 2;
	else
		ent->flags = 1;
	
	ent->s.effects &= ~(EF_COLOR_SHELL);
	ent->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	ent->s.effects |= EF_COLOR_SHELL;
	ent->s.renderfx |= teamkeys[ent->flags-1].keyeffect;
	ent->touch = teamplay_TouchGoal;
	ent->think = teamplay_CycleGoal;
	ent->nextthink = level.time + 10.0;
}

void ED_CallSpawn (edict_t *ent);
void teamplay_SpawnKey (edict_t *self, int keyindex, qboolean newspawn, qboolean keydropped)
{
//	vec3_t	offset;
	edict_t *ent, *spot, *goal=NULL;
	gitem_t	*it;

	if (keyindex == 1)
		it = FindItem("Blue Team Flag");
	else if (keyindex == 2)
		it = FindItem("Red Team Flag");
	else if (keyindex == 3)
		it = FindItem("Green Team Key");

	if ((keydropped == true) && (self))
		ent = Drop_Item (self, it);
	else
	{
		if (newspawn == true)
		{
			if (goalmode == 1)
				goal = G_Find (NULL, FOFS(classname), teamkeys[keyindex-1].flaggoal);
			else if (goalmode == 2)
				goal = G_Find (NULL, FOFS(classname), "bots_goal_all");
			
			if (keyindex == 1)
			{
				spot = G_Find (NULL, FOFS(classname), teamkeys[keyindex-1].flagstart);
				if (!spot)
				{
					spot = G_Find (NULL, FOFS(classname), teamkeys[keyindex-1].flagstart2);
					if (!spot)
					{
						spot = SelectDeathmatchSpawnPoint();
						mapmode = 3;
					}
					else
						mapmode = 2;
				}
				else
					mapmode = 1;
			}
			else
			{
				spot = G_Find (NULL, FOFS(classname), teamkeys[keyindex-1].flagstart);
				if (!spot)
				{
					spot = G_Find (NULL, FOFS(classname), teamkeys[keyindex-1].flagstart2);
					if (!spot)
						spot = teamplay_SelectFarthestKeySpawnPoint(keyindex);
				}
			}

			if (goalmode == 1)
			{
				if (goal)
				{
					goal->s.effects &= ~(EF_COLOR_SHELL);
					goal->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
					goal->s.effects |= EF_COLOR_SHELL;
					goal->s.renderfx |= teamkeys[keyindex-1].keyeffect;
					goal->touch = teamplay_TouchGoal;
					goal->flags = keyindex;

					while ((goal = G_Find (goal, FOFS(classname), teamkeys[keyindex-1].flaggoal)) != NULL)
					{
						goal->s.effects &= ~(EF_COLOR_SHELL);
						goal->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
						goal->s.effects |= EF_COLOR_SHELL;
						goal->s.renderfx |= teamkeys[keyindex-1].keyeffect;
						goal->touch = teamplay_TouchGoal;
						goal->flags = keyindex;
					}
				}
			}
			else if (goalmode == 2)
			{
				if (goal)
				{
					goal->s.effects &= ~(EF_COLOR_SHELL);
					goal->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
					goal->s.effects |= EF_COLOR_SHELL;
					goal->s.renderfx |= teamkeys[keyindex-1].keyeffect;
					goal->touch = teamplay_TouchGoal;
					goal->think = teamplay_CycleGoal;
					goal->nextthink = level.time + 10.0;
					goal->flags = keyindex;

					while ((goal = G_Find (goal, FOFS(classname), "bots_goal_all")) != NULL)
					{
						goal->s.effects &= ~(EF_COLOR_SHELL);
						goal->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
						goal->s.effects |= EF_COLOR_SHELL;
						goal->s.renderfx |= teamkeys[keyindex-1].keyeffect;
						goal->touch = teamplay_TouchGoal;
						goal->think = teamplay_CycleGoal;
						goal->nextthink = level.time + 10.0;
						goal->flags = keyindex;
					}
				}
			}
			else
			{
				spot->s.effects &= ~(EF_COLOR_SHELL);
				spot->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
				spot->s.effects |= EF_COLOR_SHELL;
				spot->s.renderfx |= teamkeys[keyindex-1].keyeffect;
			}

			teamkeys[keyindex-1].spawnspot = spot;
			teamkeys[keyindex-1].nextrespawn = NULL;
		}
		else
			spot = teamkeys[keyindex-1].spawnspot;

		PrecacheItem (it);

		ent = G_Spawn();
		ent->classname = it->classname;
		ent->flags = keyindex;
		SpawnItem (ent, it);
//		ent->think = teamplay_KeyThink;
		ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
		ent->think = teamplay_FlagSetup;

		VectorCopy (spot->s.origin, ent->s.origin);

		// Move it off the ground so people are sure to see it
//		VectorSet(offset, 0, 0, 20);    
//		VectorAdd(ent->s.origin, offset, ent->s.origin);

//		ent = Drop_Item (ent, it);
//		ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
//		ent->think = droptofloor;
	}

//	ent->s.effects &= ~(EF_COLOR_SHELL);
//	ent->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
//	ent->s.effects |= EF_COLOR_SHELL;

	ent->flags = keyindex;

	teamkeys[keyindex-1].istaken = false;
	teamkeys[keyindex-1].isdropped = keydropped;
	teamkeys[keyindex-1].keyent = ent;

//	ent->s.renderfx |= teamkeys[keyindex-1].keyeffect;
	
	ent->takedamage = 0; //no damage on flags

	teams.hasprotect[keyindex-1] = false;
}

void teamplay_SpawnAll (edict_t *self)
{
	int	i;
	
	if (numberteams < 2)
		numberteams = 2;
	else if (numberteams > 2)
		numberteams = 2;

	sprintf(teamkeys[0].keyname, "Blue Flag");
	sprintf(teamkeys[1].keyname, "Red Flag");
	sprintf(teamkeys[2].keyname, "Green Key");

	sprintf(teamkeys[0].flagstart, "info_flag_blue");
	sprintf(teamkeys[1].flagstart, "info_flag_red");
	sprintf(teamkeys[2].flagstart, "info_flag_green");

	sprintf(teamkeys[0].flaggoal, "bots_goal_blue");
	sprintf(teamkeys[1].flaggoal, "bots_goal_red");
	sprintf(teamkeys[2].flaggoal, "bots_goal_green");

	sprintf(teamkeys[0].playerstart, "info_player_start_blue");
	sprintf(teamkeys[1].playerstart, "info_player_start_red");
	sprintf(teamkeys[2].playerstart, "info_player_start_green");

	sprintf(teamkeys[0].flagstart2, "item_flag_team2");
	sprintf(teamkeys[1].flagstart2, "item_flag_team1");

	sprintf(teamkeys[0].playerstart2, "info_player_team2");
	sprintf(teamkeys[1].playerstart2, "info_player_team1");

	teamkeys[0].keyeffect = RF_SHELL_BLUE;
	teamkeys[1].keyeffect = RF_SHELL_RED;
	teamkeys[2].keyeffect = RF_SHELL_GREEN;

	teamkeys[0].capkeydropped = false;
	teamkeys[1].capkeydropped = false;
	teamkeys[2].capkeydropped = false;

	if (numberteams <= MAX_KEYS)
	{
		teamplay_SpawnKey(self, 1, true, false);
		teamplay_SpawnKey(self, 2, true, false);
		if (numberteams == 3)
			teamplay_SpawnKey(self, 3, true, false);
	}

	for (i=0 ; i<3 ; i++)
	{
		teams.team_score[i] = 0;
		teams.team_frags[i] = 0;
		teams.hasleader[i] = false;
		teams.members[i] = 0;
		teams.team_capsinarow[i] = 0;
		teams.team_impeachvotes[i] = 0;
		teams.team_promos[i] = 0;
	}

	keychecktime = level.time + 10.0;
}


int teamplay_KeyAction (edict_t *ent, edict_t *key)
{	
	int i, keyindex;
	gitem_t  *it;
	edict_t  *player;

	if (!ent->client)
		return 1;

	keyindex = key->flags;

	if (Q_stricmp (key->item->pickup_name, "Promotion Key") == 0)
	{
		if (Q_stricmp (key->item->classname, "key_promotion_blue") == 0)
			keyindex = 1;
		else if (Q_stricmp (key->item->classname, "key_promotion_red") == 0)
			keyindex = 2;
		else 
			keyindex = 3;

		if (keyindex == ent->client->pers.team)
		{
			if (ent->client->pers.player_class == 1)
			{
				teamkeys[keyindex-1].capkeydropped = false;
				return 0;
			}
			else
				return 1;
		}
		else
		{
			if (ent->client->pers.player_class != 6)
			{
				for (i=0 ; i<game.maxclients ; i++)
				{
					player = g_edicts + 1 + i;
					if (!player->inuse || !player->client)
						continue; 
					if (player->client->pers.team != keyindex)
						continue;
					if (player->client->observer)
						continue;

					if (player->client->pers.classlevel > 0)
					{
						gsutil_centerprint(player, "%s", "You lost a promotion level!\n");
						player->client->pers.classlevel--;
					}
				}

				gi.bprintf (PRINT_MEDIUM,"%s destroyed the promotion key!\n", ent->client->pers.netname);
			}
			else
			{
				if (teamplay_HasLeader(ent->client->pers.team))
				{
					teams.team_promos[ent->client->pers.team-1] += teams.team_promos[keyindex-1];

					if (ent->client->pers.classlevel == MAX_LEVEL)
						teams.team_promos[ent->client->pers.team-1] += 2;
				}

				gi.bprintf (PRINT_MEDIUM,"%s has stolen the promotion key!\n", ent->client->pers.netname);
			}

			ent->client->resp.score++;
			teams.team_promos[keyindex-1] = 0;
			teamkeys[keyindex-1].capkeydropped = false;

			if (teamplay_HasLeader(ent->client->pers.team))
			{
				ent->client->pers.classlevel++;
				if (ent->client->pers.classlevel > MAX_LEVEL)
					ent->client->pers.classlevel = MAX_LEVEL;

				if (ent->client->pers.classlevel == 1)
					gsutil_centerprint(ent, "%s", "You have been promoted to Level 1!\n");
				else if (ent->client->pers.classlevel == 2)
					gsutil_centerprint(ent, "%s", "You have been promoted to Level 2!\n");
				else if (ent->client->pers.classlevel == 3)
					gsutil_centerprint(ent, "%s", "You have been promoted to Level 3!\n");
			}

			teamplay_ResetCaptain(keyindex);

			return 2;
		}
	}
	
	if (ent->client->pers.team == keyindex)
	{
		if (teamkeys[keyindex-1].isdropped == true)
		{
			teamkeys[keyindex-1].keyent->nextthink = 0;
			gi.bprintf (PRINT_MEDIUM,"%s returned the %s!\n", ent->client->pers.netname, teamkeys[keyindex-1].keyname);
			teamkeys[keyindex-1].istaken = false;
			ent->client->resp.score++;

			return 2;
		}
		else
		{
			if (ent->client->keyindex == 0)
				return 1;
			else if ((ent->client->keyindex > 0) && (ent->client->keyindex != ent->client->pers.team) && (goalmode == 0))
			{
				teamkeys[ent->client->keyindex-1].isdropped = true;
				teamkeys[ent->client->keyindex-1].istaken = false;
				teamkeys[ent->client->keyindex-1].keyent->nextthink = 0;
				
				if (!teamplay_FairTeams(ent->client->pers.team))
					gi.bprintf (PRINT_MEDIUM,"%s's team has too many players! No points earned!\n", ent->client->pers.netname);
				else
				{
					teamkeys[ent->client->keyindex-1].lastcapturer = ent;

					gi.bprintf (PRINT_MEDIUM,"%s captured the %s!\n", ent->client->pers.netname, teamkeys[ent->client->keyindex-1].keyname);
					ent->client->resp.score++;
					ent->client->pers.captures++;

					if (teams.hasleader[ent->client->pers.team-1] == true)
					{
						teams.team_score[ent->client->pers.team-1] += 3;
						teams.team_promos[ent->client->pers.team-1]++;
					}
					else
					{
						teams.team_promos[ent->client->pers.team-1] = 0;
						teams.team_score[ent->client->pers.team-1] += 2;
					}

					gi.sound(ent, CHAN_AUTO, gi.soundindex("world/xian1.wav"), 1, ATTN_NONE, 0);

					// flash the screen
					ent->client->bonus_alpha = 0.25;	
					
					// check for team blowing out another team
					teams.team_capsinarow[ent->client->pers.team-1]++;
					for (i=1 ; i<4 ; i++)
					{
						if (i != ent->client->pers.team)
							teams.team_capsinarow[i-1] = 0;
					}

					if((int)(botsflags->value) & BOTS_QUAD_RALLY)
					{
						if (teams.team_capsinarow[ent->client->pers.team-1] == 3)
						{
							// opposing teams get quad rally
							centerprint_all("Quad Rally!!!!");
							teams.team_capsinarow[ent->client->pers.team-1] = 0;

							for_each_player(player,i)
							{
								if (player->client->pers.team != ent->client->pers.team)
								{
									it = FindItem("Quad Damage");
									player->client->pers.inventory[ITEM_INDEX(it)]++;
									Use_Quad (player, it);

									if (player->client->pers.classlevel < 2)
										player->client->pers.classlevel++;
								}
							}
						}
					}
				}
				if (ent->client->keyindex == 1)
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Blue Team Flag"))]--;
				else if (ent->client->keyindex == 2)
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Red Team Flag"))]--;
				else if (ent->client->keyindex == 3)
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Green Team Key"))]--;

				ent->client->keyindex = 0;

				lastcapteam = ent->client->pers.team;
				lastcaptime = level.time;
			}
			return 1;
		}
	}
	else
	{
		if (ent->client->keyindex > 0)
		{
			gsutil_centerprint(ent, "%s", "You can only carry one key at a time!");
			return 1;
		}
		else
		{
			if (!teamplay_FairTeams(ent->client->pers.team))
			{
				if (level.time > fairwarning)
				{
					gi.bprintf (PRINT_MEDIUM,"%s's team has too many players! Flag remains at base!\n", ent->client->pers.netname);
					fairwarning = level.time + 2.0;
				}
				return 1;
			}

			if (ent->client->pers.playmode == 0)
			{
				if (level.time > fairwarning)
				{
					gsutil_centerprint(ent, "%s", "You cannot pick up the enemy flag\nwhile on defense. To switch to offense\ntype 'offense' in the console.\n\nYou have prevented the flag from returning\nautomatically for another 30 seconds.");
					fairwarning = level.time + 2.0;
					teamkeys[keyindex-1].keyent->nextthink = level.time + 29;
				}
				return 1;
			}

			if ((ent->client->pers.player_class == 5) && (ent->client->special))
			{
				if (level.time > fairwarning)
				{
					gsutil_centerprint(ent, "%s", "You cannot pick up the enemy flag\nwhile in Berzerker Rage.");
					fairwarning = level.time + 2.0;
				}
				return 1;
			}

			teamplay_ResetSkin(ent);
//			ent->client->invincible_framenum = 0;
//			if (ent->client->pers.player_class == 5)
//				ent->client->special = false;
			gsutil_centerprint(ent, "You've got the %s!", teamkeys[keyindex-1].keyname);
			if (ent->client->pers.player_class != 6)
			{
				gi.bprintf (PRINT_MEDIUM,"%s has taken the %s!\n", ent->client->pers.netname, teamkeys[keyindex-1].keyname);
				gi.sound(ent, CHAN_AUTO, gi.soundindex("world/klaxon2.wav"), 1, ATTN_NONE, 0);
			}

			if (teamkeys[keyindex-1].isdropped == false)
				ent->client->resp.score++;

			if (ent->client->quadcloak)
			{
				ent->client->quadcloak = false;
				gsutil_centerprint(ent, "%s", "You are visible again!");
				ent->s.modelindex = 255;
				ent->client->quadcloak_framenum = 0;
			}

			teamkeys[keyindex-1].isdropped = false;
			teamkeys[keyindex-1].istaken = true;

			// flash the screen
			ent->client->bonus_alpha = 0.25;	

			// show icon and name on status bar
			ent->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(teamkeys[keyindex-1].keyent->item->icon);
			ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(teamkeys[keyindex-1].keyent->item);
			ent->client->pickup_msg_time = level.time + 3.0;
		}
	}

	ent->client->keyindex = keyindex;

	return 0;
}


//Added following code to g_save.c
//
//void InitGame (void)
//{
//	dm_respawn = gi.cvar ("dm_respawn", "2", CVAR_SERVERINFO);
//	nomonsters = gi.cvar ("nomonsters", "0", CVAR_SERVERINFO);
//
//	teamplay_InitGame();	<---
//	
//	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
//	...
//}
//
void teamplay_InitGame (void)
{
	scorelimit = gi.cvar ("scorelimit", "69", CVAR_SERVERINFO);
	botsflags  = gi.cvar ("botsflags", "31", CVAR_SERVERINFO);
	botsclass  = gi.cvar ("botsclass", "0", CVAR_SERVERINFO);
	botdetect  = gi.cvar ("botdetect", "0", CVAR_SERVERINFO);
	teamflags  = gi.cvar ("teamflags", "33", CVAR_SERVERINFO);
	team1_desc = gi.cvar ("team1_desc", "Women", CVAR_SERVERINFO);
	team2_desc = gi.cvar ("team2_desc", "Men", CVAR_SERVERINFO);
	team3_desc = gi.cvar ("team3_desc", "Green", CVAR_SERVERINFO);
	maxplayers = gi.cvar ("maxplayers", "16", CVAR_SERVERINFO);

	if ((int)(maxplayers->value) > (int)(maxclients->value))
		maxplayers = maxclients;
	
	teamplay = 1;
	numberteams = 2;
}


void teamplay_InitKeys(void)
{
	edict_t	*ent=NULL;
	
	teamplay_SpawnAll(ent);
	tp_keys_spawned = true;

	if (!flag1_item)
		flag1_item = FindItemByClassname("key_blueteam");

	if (!flag2_item)
		flag2_item = FindItemByClassname("key_redteam");

	startgame = level.time + 20.0;
	nextwarn  = level.time;
}


void teamplay_DisplayTeamScores (edict_t *ent)
{
	int n, j;
	char team_list[500];
	edict_t	 *player;
	
	return;
	
	if (teamplay == 0)
		return;
	
	j = sprintf(team_list, "Team          (Ppl) [Score/Frags]\n==================================\n");

	for (n=0 ; n<3 ; n++)
		teams.members[n] = 0;

	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;
		if (!player->inuse || !player->client)
			continue; 
		if (!player->client->pers.team)
			continue;
		if (player->client->observer)
			continue;

		teams.members[player->client->pers.team-1]++;
	}
	
	for(n=1; n<=numberteams; n++)
	{
		if(n == 1)
			j += sprintf(team_list + j, "1) %11s (%3i) [%5i/%5i]\n", team1_desc->string, teams.members[0], teams.team_score[0], teams.team_frags[0]);			
		else if(n == 2)
			j += sprintf(team_list + j, "2) %11s (%3i) [%5i/%5i]\n", team2_desc->string, teams.members[1], teams.team_score[1], teams.team_frags[1]);			
		else if(n == 3)
			j += sprintf(team_list + j, "3) %11s (%3i) [%5i/%5i]\n", team3_desc->string, teams.members[2], teams.team_score[2], teams.team_frags[2]);			

		if (j > 450)
			break;
	}
	j = sprintf(team_list + j, "==================================\n");

	gsutil_centerprint(ent, "%s", team_list);
}


void teamplay_ResetPlayer (edict_t *ent, qboolean override)
{
	edict_t *health;

	if (!ent->client)
		return;

	gsmod_SetStats(ent);
	
	if ((ent->client->pers.player_class != 3) || (override))
	{
		// Clear out the alarm if its hanging around
		if (ent->alarm1)
			G_FreeEdict(ent->alarm1);
		if (ent->alarm2)
			G_FreeEdict(ent->alarm2);
		if (ent->alarm3)
			G_FreeEdict(ent->alarm3);

		ent->alarm1 = NULL;
		ent->alarm2 = NULL;
		ent->alarm3 = NULL;
	}

	if ((ent->client->pers.player_class != 8) || (override))
	{
		// Clear out poisoned health if its hanging around
		for (health=g_edicts; health < &g_edicts[globals.num_edicts]; health++)
		{
			if ((health->owner == ent) && (health->item))
			{
				if (Q_stricmp (health->item->pickup_name, "Health") == 0)
				{
					if (health->count < 0)
					{
						health->owner = NULL;
						health->count = (health->count * -1) / 2;
					}
				}
			}
		}
	}

	teamplay_ResetLifelink(ent);
}


void teamplay_ClientPickTeam (edict_t *ent, int tteam, qboolean isauto)
{
	edict_t	*player;
	int		n,j;
	int count[100];

	// check to see if already on a team
	if (!ent->client)
		return;

	if ((ent->client->showmenu) && (isauto))
		gsmod_Killmenu(ent);

	ent->resethud = true;

	if ((ent->client->pers.team == 1) || (ent->client->pers.team == 2))
		return;

	if ((ent->client->observer) && (ent->client->pers.team == -1))
	{
		if (gsmod_NumPlayers() >= (int)(maxplayers->value))
		{
			gsutil_centerprint(ent, "%s", "The number of active players is full.\n");
			return;
		}
	}

	if (tteam == 9)
	{
		teamplay_ObserverMode(ent);
		ent->client->pers.team = -1;
		return;
	}

	if((int)(botsflags->value) & BOTS_AUTO_TEAM)
		tteam = 0;

	if ((tteam < 1) || (tteam > numberteams))
	{
		memset(count,0,sizeof(int)*100); //numberteams);


		count[0] = 0;
		count[1] = 0;
		for (n = 1; n <= maxclients->value; n++)
		{
			player = &g_edicts[n];
								
			if (!player->inuse || !player->client)
				continue;
			if (player->client->observer)
				continue;
			if (!player->client->pers.team)
				continue;
			if (!player->client->pers.player_class)
				continue;
			count[player->client->pers.team-1]++;
		}
	
		if (count[0] == count[1])
		{
			j = rand() & 2;
			if (j == 2)
				ent->client->pers.team = 1;
			else
				ent->client->pers.team = 2;
		}
		else
		{					
			if (count[0] > count[1])
				ent->client->pers.team = 2;
			else
				ent->client->pers.team = 1;
		}
	}
	else	
		ent->client->pers.team = tteam;

	ent->client->pers.startpause = level.time - 4.0;
	ent->client->pers.classlevel = 0;
	ent->client->pers.playmode = 1;
	ent->client->pers.nextplaymode = 1;

	teamplay_ResetPlayer(ent, true);
}


void teamplay_WeaponToggle (edict_t *ent, gitem_t *item)
{
	if (!ent->client)
		return;

	if (Q_stricmp (item->pickup_name, "Grenade Launcher") == 0)
	{
		ent->client->clustermode = false;

		if (ent->client->weapontoggle)
			gsutil_centerprint(ent, "%s", "Detpipes activated");
		else
			gsutil_centerprint(ent, "%s", "Normal grenades activated");
	}
	else if (Q_stricmp (item->pickup_name, "Grenades") == 0)
	{
		if ((ent->client->pers.player_class == 1) || (ent->client->pers.player_class == 2))
		{
			if (ent->client->weapontoggle)
				gsutil_centerprint(ent, "%s", "Proximity grenades activated");
			else
				gsutil_centerprint(ent, "%s", "Normal grenades activated");
		}
		else if (ent->client->pers.player_class == 6)
		{
			if (ent->client->weapontoggle)
				gsutil_centerprint(ent, "%s", "Decoy grenades activated");
			else
				gsutil_centerprint(ent, "%s", "Normal grenades activated");
		}
		else if (ent->client->pers.player_class == 8)
		{
			if (ent->client->weapontoggle)
				gsutil_centerprint(ent, "%s", "Flash grenades activated");
			else
				gsutil_centerprint(ent, "%s", "Normal grenades activated");
		}
	}
	else if (Q_stricmp (item->pickup_name, "Rocket Launcher") == 0)
	{
		if (ent->client->pers.classlevel < 2)
			ent->client->weapontoggle = false;
		else
		{
			if (ent->client->weapontoggle)
			{
				gsutil_centerprint(ent, "%s", "Split rockets activated");
				ent->client->splitmode = 1;
			}
			else
				gsutil_centerprint(ent, "%s", "Normal rockets activated");
		}
	}
	else
		ent->client->weapontoggle = false;
}


void teamplay_Toggle (edict_t *ent, int playerclass)
{
	if (!ent->client)
		return;
	
	if (ent->client->pers.player_class != playerclass)
		return;

	if (ent->client->weapontoggle)
		ent->client->weapontoggle = false;
	else
		ent->client->weapontoggle = true;
		
	if (playerclass == 4)
	{
		if (ent->client->pers.classlevel < 3)
			ent->client->weapontoggle = false;
		else
		{
			if (ent->client->weapontoggle)
				gsutil_centerprint(ent, "%s", "Split rockets activated");
			else
				gsutil_centerprint(ent, "%s", "Normal rockets activated");
		}
	}
	else if (playerclass == 7)
	{
		if (ent->client->weapontoggle)
			gsutil_centerprint(ent, "%s", "Detpipes activated");
		else
			gsutil_centerprint(ent, "%s", "Normal grenades activated");
	}
	else if ((playerclass == 1) || (playerclass == 2))
	{
		if (ent->client->weapontoggle)
			gsutil_centerprint(ent, "%s", "Proximity grenades activated");
		else
			gsutil_centerprint(ent, "%s", "Normal grenades activated");
	}
	else if (playerclass == 6)
	{
		if (ent->client->weapontoggle)
			gsutil_centerprint(ent, "%s", "Decoy grenades activated");
		else
			gsutil_centerprint(ent, "%s", "Normal grenades activated");
	}
	else if (playerclass == 8)
	{
		if (ent->client->weapontoggle)
			gsutil_centerprint(ent, "%s", "Flash grenades activated");
		else
			gsutil_centerprint(ent, "%s", "Normal grenades activated");
	}
	else
		ent->client->weapontoggle = false;
}

	
qboolean teamplay_Pickup_Weapon (edict_t *ent, edict_t *other)
{
	if (!other->client)
		return false;

	if ((!strcmp (ent->item->pickup_name, "Shotgun")) && (other->client->pers.player_class == 2))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Super Shotgun")) && (other->client->pers.player_class == 2))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Chaingun")) && (other->client->pers.player_class == 5))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Machinegun")) && (other->client->pers.player_class == 8))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Grenade Launcher")) && (other->client->pers.player_class == 7))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Rocket Launcher")) && (other->client->pers.player_class == 4))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "BFG10K")) && (other->client->pers.player_class == 1))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "Railgun")) && (other->client->pers.player_class == 3))
		other->client->newweapon = ent->item;
	else if ((!strcmp (ent->item->pickup_name, "HyperBlaster")) && (other->client->pers.player_class == 6))
		other->client->newweapon = ent->item;
	else
	{
		//gsutil_centerprint(other, "Your class does not allow you to pick\nup a %s\n", ent->item->pickup_name);
		return false;
	}

	return true;
}


void teamplay_UpgradeClass (edict_t *ent)
{
	int				playerclass, index;
	gitem_t			*it;
	gitem_armor_t	*newinfo;

	if (!ent->client)
		return;

	gsmod_SetStats(ent);

	playerclass = ent->client->pers.player_class;

	if (ent->client->pers.classlevel > 0)
	{
		if (playerclass == 1)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = ent->client->pers.max_cells;
		else if (playerclass == 2)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] = ent->client->pers.max_shells;
		else if (playerclass == 3)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] = ent->client->pers.max_slugs;
		else if (playerclass == 4)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] = ent->client->pers.max_rockets;
		else if (playerclass == 5)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] = ent->client->pers.max_bullets;
		else if (playerclass == 6)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = ent->client->pers.max_cells;
		else if (playerclass == 7)
		{
			if (ent->client->pers.max_grenades > 45)
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] = 45;
			else
				ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] = ent->client->pers.max_grenades;
		}
		else if (playerclass == 8)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] = ent->client->pers.max_bullets;
		
		if (IsFemale(ent))
			it = FindItem ("Jacket Armor");
		else
			it = FindItem ("Body Armor");

		if (it)
		{
			if (ent->client->pers.inventory[ITEM_INDEX(it)] == 0)
			{
				newinfo = (gitem_armor_t *)it->info;
				ent->client->pers.inventory[ITEM_INDEX(it)] = newinfo->base_count;
				gsmod_Pickup_Armor(ent, it->pickup_name, ITEM_INDEX(it));
			}
		}
	}

	if (ent->client->pers.classlevel > 1)
	{
		if (playerclass == 1)
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Hook"))] = 1;
		}
		else if (playerclass == 2)
		{
			it = FindItem ("Cells");
		}
		else if (playerclass == 3)
		{
//			if (IsFemale(ent))
//				ent->client->pers.max_slugs = 50;
//			else
//				ent->client->pers.max_slugs = 100;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] = ent->client->pers.max_slugs;

			it = FindItem ("Cells");
		}
		else if (playerclass == 4)
		{
//			if (IsFemale(ent))
//			{
//				if (ent->client->pers.playmode == 0)
//					ent->client->pers.max_rockets = 75;
//				else
//					ent->client->pers.max_rockets = 25;
//			}
//			else
//			{
//				if (ent->client->pers.playmode == 0)
//					ent->client->pers.max_rockets = 100;
//				else
//					ent->client->pers.max_rockets = 50;
//			}
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] = ent->client->pers.max_rockets;
		}
		else if (playerclass == 5)
		{
			if (IsFemale(ent))
				it = FindItem ("Jacket Armor");
			else
				it = FindItem ("Body Armor");

			ent->client->pers.inventory[ITEM_INDEX(it)]++;
			newinfo = (gitem_armor_t *)it->info;
			
			if (IsFemale(ent))
			{
				if (ent->client->pers.playmode == 0)
					ent->client->pers.inventory[ITEM_INDEX(it)] = 250;
				else
					ent->client->pers.inventory[ITEM_INDEX(it)] = 200;
			}
			else
			{
				if (ent->client->pers.playmode == 0)
					ent->client->pers.inventory[ITEM_INDEX(it)] = 300;
				else
					ent->client->pers.inventory[ITEM_INDEX(it)] = 250;
			}

			it = NULL;
		}
		else if (playerclass == 7)
		{
//			if (!strcmp (ent->client->pers.sounddir, "player/female"))
//				ent->client->pers.max_grenades = 50;
//			else
//				ent->client->pers.max_grenades = 100;
//			ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] = ent->client->pers.max_grenades;
			ent->client->longrange = true;
		}
		else if (playerclass == 8)
		{
			it = FindItem ("Grenades");
		}

		if (it)
		{
			index = ITEM_INDEX(it);
			ent->client->pers.inventory[index] += it->quantity;
			if (ent->client->pers.inventory[index] > ent->client->pers.max_bullets)
				ent->client->pers.inventory[index] = ent->client->pers.max_bullets;
		}
	}
}

	
qboolean teamplay_WholeTeam (edict_t *ent, int classlevel)
{
	int		i, pcount;
	edict_t *player;

	if (!ent->client)
		return false;

	pcount = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue; 
		if (player->client->observer)
			continue;
		if (!player->client->pers.player_class)
			continue;
		if (player == ent)
			continue;
		if (player->client->pers.classlevel < classlevel)
			return false;

		pcount++;
	}

	if (pcount)
		return true;
	else
		return false;
}


int teamplay_LowestLevel (edict_t *ent)
{
	int		i, pcount, lowlevel, templevel;
	edict_t *player;

	if (!ent->client)
		return false;

	pcount = 0;
	lowlevel = 3;
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue; 
		if (player->client->observer)
			continue;
		if (!player->client->pers.player_class)
			continue;
		if (player == ent)
			continue;
		if (!player->client->resp.score)
			continue;

		pcount++;
		templevel = player->client->pers.classlevel;
		if (templevel < lowlevel)
			lowlevel = templevel;
	}

	if (pcount)
		return lowlevel;
	else
		return 0;
}


void teamplay_InitClass (edict_t *ent, qboolean spawned)
{
	gitem_t	*it, *it2, *it3;
	gitem_armor_t	*newinfo;
	int		index;

	if (!ent->client)
		return;

	if (ent->client->pers.startlevel == false)
		return;
	
	ent->client->silencer_shots = 0;

	teamplay_ResetPlayer(ent, false);

	it3 = 0;
	if (spawned == true)
	{
		if (ent->client->pers.player_class == 1)
		{
			it = FindItem ("BFG10K");
			it2 = FindItem ("Cells");
			teams.hasleader[ent->client->pers.team-1] = true;
			teamplay_CaptainKey(ent);
			ent->client->pers.classlevel = teamplay_LowestLevel(ent);
		}
		else
			teams.hasleader[ent->client->pers.team-1] = teamplay_HasLeader(ent->client->pers.team);

		if (ent->client->pers.player_class == 2)
		{
			it = FindItem ("Super Shotgun");
			it2 = FindItem ("Shells");

			// get info on new armor and use it
			if (IsFemale(ent))
				it3 = FindItem ("Jacket Armor");
			else
				it3 = FindItem ("Body Armor");

			ent->client->pers.inventory[ITEM_INDEX(it3)]++;
			newinfo = (gitem_armor_t *)it3->info;
			ent->client->pers.inventory[ITEM_INDEX(it3)] = newinfo->base_count;

			gsmod_Pickup_Armor(ent, it3->pickup_name, ITEM_INDEX(it3));

//			if ((!ent->client->forcekill) && (!teamplay_IsOffense(ent)))
//			{
//				if (rand()&1)
//				{
//					it3 = FindItem ("Quad Damage");
//					ent->client->pers.inventory[ITEM_INDEX(it3)]++;
//				}
//			}
		}
		else if (ent->client->pers.player_class == 3)
		{
			it = FindItem ("Railgun");
			it2 = FindItem ("Slugs");

			it3 = FindItem ("Silencer");
			ent->client->pers.inventory[ITEM_INDEX(it3)]++;
			ent->client->special2 = true;
		}
		else if (ent->client->pers.player_class == 4)
		{
			it = FindItem ("Rocket Launcher");
			it2 = FindItem ("Rockets");
		}
		else if (ent->client->pers.player_class == 5)
		{
			it = FindItem ("Chaingun");
			it2 = FindItem ("Bullets");
		}
		else if (ent->client->pers.player_class == 6)
		{
			it = FindItem ("HyperBlaster");
			it2 = FindItem ("Cells");

			it3 = FindItem ("Environment Suit");
			ent->client->pers.inventory[ITEM_INDEX(it3)]++;
		}
		else if (ent->client->pers.player_class == 7)
		{
			it = FindItem ("Grenade Launcher");
			it2 = FindItem ("Grenades");
		}
		else if (ent->client->pers.player_class == 8)
		{
			it = FindItem ("Machinegun");
			it2 = FindItem ("Bullets");
		}

		if (it2)
		{
			index = ITEM_INDEX(it2);
			ent->client->pers.inventory[index] += it2->quantity;

			if (it)
			{
				ent->client->pers.inventory[ITEM_INDEX(it)]++;
				ent->client->newweapon = it;
				ChangeWeapon(ent);
			}
		}

		teamplay_UpgradeClass(ent);

		if (ent->client->pers.hasskins == false)
			gsutil_centerprint(ent, "%s", "You do not have the skins installed!\nGo to \"www.quake2.com/spinoza\" and\ndownload the \"client.zip\" file to\ninstall the skins\n");
		else
			teamplay_DisplayTeamScores (ent);

		ent->client->pers.radioon = true;
		ent->client->pers.radiochannel = 1;
		teamplay_ResetSkin(ent);
		ent->client->last_movement = level.time;
		ent->client->invisible = false;
	}
}


void teamplay_ClientPickClass (edict_t *ent, int player_class, qboolean isauto)
{
	char class_name[20];
	
	if (!ent->client)
		return;

	if ((ent->client->showmenu) && (isauto))
		gsmod_Killmenu(ent);

	if ((int)(botsclass->value))
	{
		if (((int)(botsclass->value) & BOTS_CAPTAIN) && (player_class == 1))
		{
			gsutil_centerprint(ent, "%s\n", "The Captain class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_BODYGUARD) && (player_class == 2))
		{
			gsutil_centerprint(ent, "%s\n", "The Bodyguard class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_SNIPER) && (player_class == 3))
		{
			gsutil_centerprint(ent, "%s\n", "The Sniper class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_SOLDIER) && (player_class == 4))
		{
			gsutil_centerprint(ent, "%s\n", "The Soldier class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_BERZERKER) && (player_class == 5))
		{
			gsutil_centerprint(ent, "%s\n", "The Berzerker class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_INFILTRATOR) && (player_class == 6))
		{
			gsutil_centerprint(ent, "%s\n", "The Infiltrator class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_KAMIKAZEE) && (player_class == 7))
		{
			gsutil_centerprint(ent, "%s\n", "The Kamikazee class is not available\non this server.\n");
			return;
		}

		if (((int)(botsclass->value) & BOTS_NURSE) && (player_class == 8))
		{
			gsutil_centerprint(ent, "%s\n", "The Nurse class is not available\non this server.\n");
			return;
		}
	}

	// Make sure there cannot be an invalid team
	if ((ent->client->pers.team != 1) && (ent->client->pers.team != 2))
	{
		gsmod_QueueCommand(ent, "rejoin");
		return;
	}

	if (player_class == 0)
		player_class = 4;
	else if ((player_class == 1) && (teamplay_HasLeader(ent->client->pers.team)))
	{
		gsutil_centerprint(ent, "%s\n", "Your team already has a Captain.\nChanging to Bodyguard.\n");
		player_class = 2;
	}
	else if ((player_class == 1) && (ent->client->pers.impeached))
	{
		gsutil_centerprint(ent, "%s\n", "You were impeached by your team.\nChanging to Bodyguard.\n");
		player_class = 2;
	}

	if (player_class == 1)
		strcpy (class_name, CLASS1);
	else if (player_class == 2)
		strcpy (class_name, CLASS2);
	else if (player_class == 3)
		strcpy (class_name, CLASS3);
	else if (player_class == 4)
		strcpy (class_name, CLASS4);
	else if (player_class == 5)
		strcpy (class_name, CLASS5);
	else if (player_class == 6)
		strcpy (class_name, CLASS6);
	else if (player_class == 7)
		strcpy (class_name, CLASS7);
	else if (player_class == 8)
		strcpy (class_name, CLASS8);

	if(ent->client->pers.startlevel == false)
	{
		ent->client->pers.player_class = player_class;
		ent->client->pers.player_nextclass = player_class;
		ent->client->pers.classlevel = 0;

		if(ent->client->pers.team == 1)
			gi.bprintf (PRINT_MEDIUM, "%s joined the %s as a %s\n", ent->client->pers.netname, team1_desc->string, class_name);
		else if(ent->client->pers.team == 2)
			gi.bprintf (PRINT_MEDIUM, "%s joined the %s as a %s\n", ent->client->pers.netname, team2_desc->string, class_name);
		else if(ent->client->pers.team == 3)
			gi.bprintf (PRINT_MEDIUM, "%s joined the %s as a %s\n", ent->client->pers.netname, team3_desc->string, class_name);
		else if(ent->client->pers.team == 9)
			gi.bprintf (PRINT_MEDIUM, "%s became an observer\n", ent->client->pers.netname);

		ent->client->pers.startpause = 0;
		ent->client->pers.startlevel = true;
		ent->client->pers.speed_modifier = 0;
		ent->client->pers.gravity_modifier = 0;
		teamplay_ExecScript(ent);
		respawn (ent);

	}
	else
	{
		ent->client->pers.player_nextclass = player_class;
		gsutil_centerprint(ent, "The next time you die your class will become\n--<< %s >>--\n", class_name);
	}
}


void teamplay_ClientKill(edict_t *ent, int mod)
{
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = mod;
	player_die (ent, ent, ent, 10000, vec3_origin);
	ent->deadflag = DEAD_DEAD;
}


void teamplay_ClearMute (edict_t *ent)
{
	int		i;
	edict_t *player;
	
	if (!ent->client)
		return;

	ent->mute1 = NULL;
	ent->mute2 = NULL;
	ent->mute3 = NULL;

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player == ent)
			continue;

		if (ent == player->mute1)
			player->mute1 = NULL;
		else if (ent == player->mute2)
			player->mute2 = NULL;
		else if (ent == player->mute3)
			player->mute3 = NULL;
	}
}


void teamplay_ClientBeginDeathmatch (edict_t *ent)
{
//	FILE	*teamskin;	

	if (!ent->client)
		return;

	level.helpmsg = level.time;
	level.checkskin = level.time;
	level.checkpowerup = level.framenum;

	if (gsmod_PlayerReset(ent, false) == false)
		return;

//	teamskin = fopen("baseq2\\players\\female\\fkamib.pcx", "r");
//	if (!teamskin)
//		ent->client->pers.hasskins = false;
//	else
//	{
		ent->client->pers.hasskins = true;
//		fclose(teamskin);
//	}
	
	teamplay_ObserverMode(ent);
		
	if (gsmod_NumPlayers() >= (int)(maxplayers->value))
		teamplay_ClientPickTeam (ent, 9, true);
	else
	{
		if((int)(botsflags->value) & BOTS_AUTO_TEAM)
			teamplay_ClientPickTeam (ent, 0, true);

		ent->client->pers.startpause = level.time - 2.5;
	}

	ent->client->pers.autoid = false;
	ent->client->pers.impeached = false;
	ent->client->pers.impeachvote = false;
	ent->client->pers.castvote = 0;
	ent->client->pers.scoutmsg = true;
	ent->client->pers.alarmmsg = true;

	if (ent->client->ping > 250)
		ent->client->pers.fancymenu = false;
	else
		ent->client->pers.fancymenu = true;

	gsmod_QueueCommand(ent, "exec bots.cfg");
}


void teamplay_HealMember (edict_t *ent, char *playername)
{
	int		i;
	edict_t *player;
	
	if (!ent->client)
		return;

	if (Q_stricmp ("Cancel heal", playername) == 0)
		return;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 50)
	{
		gsutil_centerprint(ent, "%s", "You need at least 50 cells to heal!\n");
		return;
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;
		if (Q_stricmp (player->client->pers.netname, playername))
			continue;

		player->health = player->client->pers.max_health;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 50;

		gsutil_centerprint(player, "You have been healed by\nNurse %s!\n", ent->client->pers.netname);
		gsutil_centerprint(ent, "%s has been healed!\n\nYou must wait 5 seconds\nto heal again.", playername);
	
		ent->client->specialtimer = level.time + 3.0;

		return;
	}

	gsutil_centerprint(ent, "Unable to heal %s!\n", playername);
}


void teamplay_MutePlayer (edict_t *ent, char *playername)
{
	int		i;
	edict_t *player;
	
	if (!ent->client)
		return;

	if (Q_stricmp ("Cancel Mute", playername) == 0)
		return;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player == ent)
			continue;
		if (Q_stricmp (player->client->pers.netname, playername))
			continue;

		if ((player->mute1 != ent) && (player->mute2 != ent) && (player->mute3 != ent))
			gsutil_centerprint(player, "You have been ignored by\n%s!\n", ent->client->pers.netname);

		if (!ent->mute1)
			ent->mute1 = player;
		else if (!ent->mute2)
			ent->mute2 = player;
		else
			ent->mute3 = player;

		return;
	}

	gsutil_centerprint(ent, "Unable to mute %s!\n", playername);
}


void teamplay_UnmutePlayer (edict_t *ent, char *playername)
{
	int		i;
	edict_t *player;
	
	if (!ent->client)
		return;

	if (Q_stricmp ("Cancel Unmute", playername) == 0)
		return;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player == ent)
			continue;
		if (Q_stricmp (player->client->pers.netname, playername))
			continue;

		gsutil_centerprint(player, "You have been unmuted by\n%s!\n", ent->client->pers.netname);

		if (player == ent->mute1)
			ent->mute1 = NULL;
		else if (player == ent->mute2)
			ent->mute2 = NULL;
		else
			ent->mute3 = NULL;

		return;
	}

	gsutil_centerprint(ent, "Unable to mute %s!\n", playername);
}


void teamplay_PromoteMember (edict_t *ent, char *playername)
{
	int		i, captlevel;
	edict_t *player;
	
	if (!ent->client)
		return;

	if (teams.team_promos[ent->client->pers.team-1] <= 0)
	{
		teams.team_promos[ent->client->pers.team-1] = 0;
		return;
	}

	if (Q_stricmp ("Cancel promotion", playername) == 0)
		return;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;
		if (Q_stricmp (player->client->pers.netname, playername))
			continue;

		player->client->pers.classlevel++;

		if (player->client->pers.classlevel == 1)
		{
			gsutil_centerprint(player, "%s", "You have been promoted to Level 1!\n");
			gsutil_centerprint(ent, "%s promoted to Level 1!\n", playername);
			teams.team_promos[ent->client->pers.team-1]--;
		}
		else if (player->client->pers.classlevel == 2)
		{
			gsutil_centerprint(player, "%s", "You have been promoted to Level 2!\n");
			gsutil_centerprint(ent, "%s promoted to Level 2!\n", playername);
			teams.team_promos[ent->client->pers.team-1] -= 2;
		}
		else if (player->client->pers.classlevel == 3)
		{
			gsutil_centerprint(player, "%s", "You have been promoted to Level 3!\n");
			gsutil_centerprint(ent, "%s promoted to Level 3!\n", playername);
			teams.team_promos[ent->client->pers.team-1] -= 3;
		}

		gsmod_SetStats(player);

		captlevel = teamplay_LowestLevel(ent);
		if (ent->client->pers.classlevel < captlevel)
		{
			ent->client->pers.classlevel = captlevel;
			gsmod_SetStats(ent);
		}
		else
			ent->client->pers.classlevel = captlevel;

		return;
	}

	gsutil_centerprint(ent, "Unable to promote %s!\n", playername);
}


void teamplay_DemoteMember (edict_t *ent, char *playername)
{
	int		i, captlevel;
	edict_t *player;
	
	if (!ent->client)
		return;

	if (teams.team_promos[ent->client->pers.team-1] <= 0)
	{
		teams.team_promos[ent->client->pers.team-1] = 0;
		return;
	}

	if (Q_stricmp ("Cancel demotion", playername) == 0)
		return;
	
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client) 
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;
		if (Q_stricmp (player->client->pers.netname, playername))
			continue;

		player->client->pers.classlevel--;

		if (player->client->pers.classlevel == 0)
		{
			gsutil_centerprint(player, "%s", "You have been demoted to Level 0!\n");
			gsutil_centerprint(ent, "%s demoted to Level 0!\n", playername);
			teams.team_promos[ent->client->pers.team-1]++;
		}
		else if (player->client->pers.classlevel == 1)
		{
			gsutil_centerprint(player, "%s", "You have been demoted to Level 1!\n");
			gsutil_centerprint(ent, "%s demoted to Level 1!\n", playername);
			teams.team_promos[ent->client->pers.team-1]++;
		}
		else if (player->client->pers.classlevel == 2)
		{
			gsutil_centerprint(player, "%s", "You have been demoted to Level 2!\n");
			gsutil_centerprint(ent, "%s demoted to Level 2!\n", playername);
			teams.team_promos[ent->client->pers.team-1]++;
		}
		else if (player->client->pers.classlevel == 3)
		{
			gsutil_centerprint(player, "%s", "You have been demoted to Level 3!\n");
			gsutil_centerprint(ent, "%s demoted to Level 3!\n", playername);
			teams.team_promos[ent->client->pers.team-1]++;
		}

		captlevel = teamplay_LowestLevel(ent);
		ent->client->pers.classlevel = captlevel;

		return;
	}

	gsutil_centerprint(ent, "Unable to demote %s!\n", playername);
}


void teamplay_CreateItem (edict_t *ent, char *itemname, int itemcost)
{
	int numcells;
	gitem_t *newitem;
	edict_t *dropped;

	if (!ent->client)
		return;

	if (Q_stricmp (itemname, "Cancel create") == 0)
		return;
	
	numcells = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];

	if (numcells >= itemcost)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= itemcost;

		if ((itemcost > 50) && (ent->client->pers.player_class == 1))
			ent->createpause = level.time + (10.0 * ent->client->pers.classlevel);
		else if (ent->client->pers.player_class == 8)
			ent->createpause = level.time + (40.0 - (ent->client->pers.classlevel * 10));

		newitem = FindItem(itemname);
		if (newitem)
		{
			dropped = Drop_Item(ent, newitem);

			if (Q_stricmp (itemname, "MegaHealth") == 0)
			{
				dropped->count = 100;
				dropped->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
			}
			else if (Q_stricmp (itemname, "LargeHealth") == 0)
				dropped->count = 25;
		}
		else
			gsutil_centerprint(ent, "Unable to create %s!\n", itemname);
	}
	else
		gsutil_centerprint(ent, "Unable to create %s!\n", itemname);
}

	
int teamplay_MenuCallback(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	int selection;
	
	if (!ent->client)
		return 0;

	selection = ((menu_item_t *)selected->it)->itemvalue;

	if (ent->client->menutype == 1)
	{
		if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, team1_desc->string) == 0)
			gsmod_QueueCommand(ent, "join1");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, team2_desc->string) == 0)
			gsmod_QueueCommand(ent, "join2");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, team3_desc->string) == 0)
			gsmod_QueueCommand(ent, "join3");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, "Observer/Chase Cam") == 0)
			gsmod_QueueCommand(ent, "join9");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, "Auto Select a Team") == 0)
			gsmod_QueueCommand(ent, "join0");

//		teamplay_ClientPickTeam (ent, ((menu_item_t *)selected->it)->itemvalue, false);
	}
	else if (ent->client->menutype == 2)
	{
		if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS1) == 0)
			gsmod_QueueCommand(ent, "class1");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS2) == 0)
			gsmod_QueueCommand(ent, "class2");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS3) == 0)
			gsmod_QueueCommand(ent, "class3");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS4) == 0)
			gsmod_QueueCommand(ent, "class4");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS5) == 0)
			gsmod_QueueCommand(ent, "class5");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS6) == 0)
			gsmod_QueueCommand(ent, "class6");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS7) == 0)
			gsmod_QueueCommand(ent, "class7");
		else if (Q_stricmp (((menu_item_t *)selected->it)->itemtext, CLASS8) == 0)
			gsmod_QueueCommand(ent, "class8");
//		teamplay_ClientPickClass (ent, ((menu_item_t *)selected->it)->itemvalue, false);
	}
	else if (ent->client->menutype == 3)
		teamplay_PromoteMember (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 4)
		teamplay_CreateItem (ent, ((menu_item_t *)selected->it)->itemtext, ((menu_item_t *)selected->it)->itemvalue);
	else if (ent->client->menutype == 5)
		teamplay_HealMember (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 6)
		teamplay_UseDisguise (ent, ((menu_item_t *)selected->it)->itemvalue);
	else if (ent->client->menutype == 7)
		teamplay_MutePlayer (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 8)
		teamplay_UnmutePlayer (ent, ((menu_item_t *)selected->it)->itemtext);
	else if (ent->client->menutype == 9)
		teamplay_DemoteMember (ent, ((menu_item_t *)selected->it)->itemtext);
	
	return 0;
}


void teamplay_DisplayTeams (edict_t *ent)
{
	arena_link_t *menulink;
	int n;
	
	if (!ent->client)
		return;

	if((int)(botsflags->value) & BOTS_AUTO_TEAM)
	{
		stuffcmd(ent, "cmd join0\n");
		return;
	}
	
	gsmod_Killmenu(ent);

	// Reset team
	ent->client->pers.team = 0;

	ent->client->menutype = 1;
	menulink = CreateMenu(ent, "Pick a Team");
	for(n=1; n<=numberteams; n++)
	{
		if(n == 1)
			AddMenuItem(menulink, team1_desc->string, NULL, -1, &teamplay_MenuCallback);
		else if(n == 2)
			AddMenuItem(menulink, team2_desc->string, NULL, -1, &teamplay_MenuCallback);
		else if(n == 3)
			AddMenuItem(menulink, team3_desc->string, NULL, -1, &teamplay_MenuCallback);
	}
	AddMenuItem(menulink, "Observer/Chase Cam", NULL, -1, &teamplay_MenuCallback);
	AddMenuItem(menulink, "Auto Select a Team", NULL, -1, &teamplay_MenuCallback);
	FinishMenu(ent, menulink);
}


void teamplay_DisplayClasses (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	gsmod_Killmenu(ent);

	// reset the player class
	ent->client->pers.player_class = 0;

	ent->client->menutype = 2;
	menulink = CreateMenu(ent, "Pick a Player Class");

//	if (!teamplay_HasLeader(ent->client->pers.team) && !ent->client->pers.impeached)
//		AddMenuItem(menulink, CLASS1, NULL, -1, &teamplay_MenuCallback);

	if (!((int)(botsclass->value) & BOTS_BODYGUARD))
		AddMenuItem(menulink, CLASS2, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_SNIPER))
		AddMenuItem(menulink, CLASS3, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_SOLDIER))
		AddMenuItem(menulink, CLASS4, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_BERZERKER))
		AddMenuItem(menulink, CLASS5, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_INFILTRATOR))
		AddMenuItem(menulink, CLASS6, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_KAMIKAZEE))
		AddMenuItem(menulink, CLASS7, NULL, -1, &teamplay_MenuCallback);
	if (!((int)(botsclass->value) & BOTS_NURSE))
		AddMenuItem(menulink, CLASS8, NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}


void teamplay_DisplayDisguises (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 6)
		{
			gsutil_centerprint(ent, "%s", "Only the Infiltrator can disguise!\n");
			return;
		}
	}
	else
		return;

	if (ent->client->pers.classlevel < 2)
	{
		gsutil_centerprint(ent, "%s", "Only level 2 Infiltrator can disguise!\n");
		return;
	}

	if (!teamplay_IsOffense(ent))
	{
		gsutil_centerprint(ent, "%s", "Only an offensive Infiltrator can\nuse a disguise\n");
		return;
	}

	if (ent->client->keyindex)
	{
		gsutil_centerprint(ent, "%s", "You cannot use a disguise while\ncarrying the enemy flag!\n");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 25)
	{
		gsutil_centerprint(ent, "%s", "You need at least 25 cells\nto use a disguise!\n");
		return;
	}

	ent->client->menutype = 6;

	menulink = CreateMenu(ent, "Pick a Disguise");
	AddMenuItem(menulink, CLASS1, " - ", 1, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS2, " - ", 2, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS3, " - ", 3, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS4, " - ", 4, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS5, " - ", 5, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS6, " - ", 6, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS7, " - ", 7, &teamplay_MenuCallback);
	AddMenuItem(menulink, CLASS8, " - ", 8, &teamplay_MenuCallback);
	AddMenuItem(menulink, "Cancel disguise", " - ", 0, &teamplay_MenuCallback);
	FinishMenu(ent, menulink);
}


void teamplay_DisplayStatus (edict_t *ent, edict_t *display)
{
	int			j, n;
	char		mystatus[500];
	edict_t		*player;
	qboolean	hascaptain;
	
	if (!ent->client)
		return;

	if (teamplay == 0)
	{
		gsutil_centerprint(display, "\n\n\n\n%s", ent->client->pers.netname);
		return;
	}
	
	j = sprintf(mystatus, "Player Status               \n============================\n");
	j += sprintf (mystatus + j, "Name  : %20s\n", ent->client->pers.netname);

	if (ent->client->pers.team == 1)
		j += sprintf (mystatus + j, "Team  : [1] %16s\n", team1_desc->string);
	else if(ent->client->pers.team == 2)
		j += sprintf (mystatus + j, "Team  : [2] %16s\n", team2_desc->string);
	else if(ent->client->pers.team == 3)
		j += sprintf (mystatus + j, "Team  : [3] %16s\n", team3_desc->string);

	if (ent->client->pers.player_class == 1)
		j += sprintf (mystatus + j, "Class : [1] %16s\n", CLASS1);
	else if (ent->client->pers.player_class == 2)
		j += sprintf (mystatus + j, "Class : [2] %16s\n", CLASS2);
	else if (ent->client->pers.player_class == 3)
		j += sprintf (mystatus + j, "Class : [3] %16s\n", CLASS3);
	else if (ent->client->pers.player_class == 4)
		j += sprintf (mystatus + j, "Class : [4] %16s\n", CLASS4);
	else if (ent->client->pers.player_class == 5)
		j += sprintf (mystatus + j, "Class : [5] %16s\n", CLASS5);
	else if (ent->client->pers.player_class == 6)
		j += sprintf (mystatus + j, "Class : [6] %16s\n", CLASS6);
	else if (ent->client->pers.player_class == 7)
		j += sprintf (mystatus + j, "Class : [7] %16s\n", CLASS7);
	else if (ent->client->pers.player_class == 8)
		j += sprintf (mystatus + j, "Class : [8] %16s\n", CLASS8);
	else
		j += sprintf (mystatus + j, "Class : Error\n");

	if (numberteams <= MAX_KEYS)
	{
		if (ent->client->keyindex > 0)
			j += sprintf (mystatus + j, "Flag  : %20s\n", teamkeys[ent->client->keyindex-1].keyname);
		else
			j += sprintf (mystatus + j, "Flag  : %20s\n", "None");
	}

	j += sprintf (mystatus + j, "\nLevel : %i  Promo Pts : %i\n", ent->client->pers.classlevel, teams.team_promos[ent->client->pers.team-1]);

	j += sprintf (mystatus + j, "\n");

	if (ent->client->pers.playmode == 0)
		j += sprintf (mystatus + j, "\nYou are on defense");
	else
		j += sprintf (mystatus + j, "\nYou are on offense");


	if (ent->client->pers.player_class == 1)
	{
		if (ent->client->proxdet < 6)
			j += sprintf (mystatus + j, "\nYou have %1i proximity grenade(s) left", 6 - ent->client->proxdet);
		else
			j += sprintf (mystatus + j, "\nAll 6 proximity grenades have been deployed");

		if (ent->createpause > level.time)
			j += sprintf (mystatus + j, "\n%1i seconds until you can create major items", (int)(ent->createpause - level.time));
	}
	else if (ent->client->pers.player_class == 2)
	{
		if (ent->laserbeam)
		{
			if (level.time < ent->laserdelay)
			{
				if (ent->laserbeam2)
					j += sprintf (mystatus + j, "\nYour lasers are cooling off");
				else
					j += sprintf (mystatus + j, "\nYour laser is cooling off");
			}
			else
			{
				if (level.time > ent->laserbeam->delay)
					j += sprintf (mystatus + j, "\nLaser #1 is off");
				else
					j += sprintf (mystatus + j, "\nLaser #1 is on");
				
				if (ent->laserbeam2)
				{
					if (level.time > ent->laserbeam2->delay)
						j += sprintf (mystatus + j, "\nLaser #2 is off");
					else
						j += sprintf (mystatus + j, "\nLaser #2 is on");
				}
				else
				{
					if (ent->client->pers.classlevel > 1)
						j += sprintf (mystatus + j, "\nYou have one more laser you can place");
				}
			}
		}
		else
		{
			if (ent->client->pers.classlevel < 2)
				j += sprintf (mystatus + j, "\nYou have one laser you can place");
			else
				j += sprintf (mystatus + j, "\nYou have two lasers you can place");
		}

		if (ent->client->proxdet < 6)
			j += sprintf (mystatus + j, "\nYou have %1i proximity grenade(s) left", 6 - ent->client->proxdet);
		else
			j += sprintf (mystatus + j, "\nAll 6 proximity grenades have been deployed");
	}
	else if (ent->client->pers.player_class == 3)
	{
		if (ent->client->invisible)
			j += sprintf (mystatus + j, "\nYou are cloaked");
		else
			j += sprintf (mystatus + j, "\nYou are visible");

		if (ent->client->proxdet < 4)
			j += sprintf (mystatus + j, "\nYou have %1i flare(s) left", 4 - ent->client->proxdet);
		else
			j += sprintf (mystatus + j, "\nAll 4 flares have been deployed");

	}
	else if (ent->client->pers.player_class == 4)
	{
		// nothing
	}
	else if (ent->client->pers.player_class == 5)
	{
		if (IsFemale(ent))
			j += sprintf (mystatus + j, "\nQuad berzerk mode in %2.0f seconds", ent->client->pers.startpause + 60.0 - level.time);
		else
			j += sprintf (mystatus + j, "\nQuad berzerk mode in %2.0f seconds", ent->client->pers.startpause + 90.0 - level.time);
	}
	else if (ent->client->pers.player_class == 6)
	{
		if (ent->client->pers.classlevel > 1)
		{
			if (ent->client->special)
				j += sprintf (mystatus + j, "\nYour disguise will wear off in %2.0f seconds", ent->client->specialtimer - level.time);
			else
				j += sprintf (mystatus + j, "\nYou are not disguised");
		}

		if (ent->client->proxdet < 4)
			j += sprintf (mystatus + j, "\nYou have %1i decoy grenade(s) left", 4 - ent->client->proxdet);
		else
			j += sprintf (mystatus + j, "\nAll 4 decoy grenades have been deployed");
	}
	else if (ent->client->pers.player_class == 7)
	{
		if (ent->client->proxdet < 6)
			j += sprintf (mystatus + j, "\nYou have %1i detpipe(s) left", 6 - ent->client->proxdet);
		else
			j += sprintf (mystatus + j, "\nAll 6 detpipes have been deployed");
	}
	else if (ent->client->pers.player_class == 8)
	{
		hascaptain = false;
		for (n=0 ; n<game.maxclients ; n++)
		{
			player = g_edicts + 1 + n;
			if (!player->inuse || !player->client)
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->pers.team != ent->client->pers.team)
				continue; 

			if (player->client->pers.player_class == 1)
			{
				j += sprintf (mystatus + j, "\nCaptain %s has %3i health left", player->client->pers.netname, player->health);
				hascaptain = true;
				break;
			}
		}

		if (!hascaptain)
			j += sprintf (mystatus + j, "\nYour team does not have a Captain!");

		if (ent->createpause > level.time)
			j += sprintf (mystatus + j, "\n%1i seconds until you can create again", (int)(ent->createpause - level.time));
	}

	gsutil_centerprint(display, "\n\n\n\n%s", mystatus);
}


void teamplay_DisplayHelpMsg (edict_t *ent)
{
	int i, j;

	if (!ent->client)
		return;

	i = rand() & 9;
	j = rand() & 2;
	if ((teamplay == 1) && (j == 2))
	{
		if (i == 1)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP1);
		else if (i == 2)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP2);
		else if (i == 3)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP3);
		else if (i == 4)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP4);
		else if (i == 5)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP5);
		else if (i == 6)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP6);
		else if (i == 7)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP7);
		else if (i == 8)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP8);
		else if (i == 9)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", TEAMHELP9);
	}
	else
	{
		if (i == 1)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP1);
		else if (i == 2)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP2);
		else if (i == 3)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP3);
		else if (i == 4)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP4);
		else if (i == 5)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP5);
		else if (i == 6)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP6);
		else if (i == 7)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP7);
		else if (i == 8)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP8);
		else if (i == 9)
			gi.cprintf (ent, PRINT_HIGH, "Help: %s", HELP9);
	}
}



void teamplay_ChaseCam(edict_t *ent)
{
	edict_t	*player;
	int		offset, i;
	
	if (!ent->client)
		return;

	if (ent->client->chasecam)
		offset = ent->client->chasecam - g_edicts;
	else
		offset = 0;

	if (offset >= maxclients->value)
	{
		ent->client->chasecam = NULL;
		return;
	}

	for (i=offset ; i<maxclients->value ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || player == ent)
			continue;
		if (!player->client)
			continue;
		if (player->client->observer)
			continue;

		ent->client->chasecam = player;
		return;
	}

	ent->client->chasecam = NULL;
}



qboolean teamplay_ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	int		i, grenades, r, l, distance;
	float	quadtime, cloaktime, len, lf_pause;
	edict_t	*player;
	gitem_t *it;
	vec3_t	chaseorigin, chaseangle, v;

	if (!ent->client)
		return false;

	if ((ent->client->observer) && (ent->client->pers.team == -1))
	{
		if ((ent->client->pers.startpause) && (level.time > ent->client->pers.startpause))
		{
			if (ent->client->buttons & BUTTON_ATTACK)
			{
				teamplay_ChaseCam(ent);

				if (!ent->client->chasecam)
					teamplay_ObserverMode(ent);

				ent->client->pers.startpause = level.time + 1.0;
			}
		}

		if (ent->client->chasecam)
		{
			VectorCopy (ent->client->chasecam->s.origin, chaseorigin);
			VectorCopy (ent->client->chasecam->s.angles, chaseangle);

			VectorCopy (chaseorigin, ent->s.origin);
			ent->client->ps.pmove.origin[0] = chaseorigin[0]*8;
			ent->client->ps.pmove.origin[1] = chaseorigin[1]*8;
			ent->client->ps.pmove.origin[2] = chaseorigin[2]*8;
			VectorCopy (chaseangle, ent->client->ps.viewangles);
		}

		return true;
	}
	
	if ((ent->client->pers.startlevel == false) && (teamplay == 1))
	{	
		if (ent->client->pers.team)
		{
			if (!ent->client->pers.player_class)
			{
				if ((level.time > ent->client->pers.startpause + 5.0) && (ent->client->pers.startpause > 0))
				{
					if (!ent->client->showmenu)
						teamplay_DisplayClasses(ent);
					ent->client->pers.startpause = level.time;
				}
			}
			return true;
		}
		else
		{
			if ((level.time > ent->client->pers.startpause + 5.0) && (ent->client->pers.startpause > 0))
			{
				if (!ent->client->showmenu)
					teamplay_DisplayTeams(ent);
				ent->client->pers.startpause = level.time;
			}
			return true;
		}

		MenuThink(ent);
	}

	if (startgame) 
	{
		if (level.time > startgame)
		{
			startgame = 0;
			gi.bprintf (PRINT_CHAT,"Let's get it on!\n");
			return false;
		}
		else if (level.time > nextwarn)
		{
			gi.bprintf (PRINT_CHAT,"Game begins in %2.0f seconds\n", startgame - nextwarn);
			nextwarn += 5.0;
			return false;
		}
		else
			return false;
	}

	if (ent->health > 0)
	{
/*		if ((level.checkpowerup) && (level.framenum > level.checkpowerup + 12))
		{
			if (ent->client->quad_framenum > 0)
			{
				if (ent->client->lastquad == ent->client->quad_framenum)
					ent->client->quad_framenum -= 10;
			}
			else
				ent->client->quad_framenum = 0;

			if (ent->client->invincible_framenum > 0)
			{
				if (ent->client->lastinvincible == ent->client->invincible_framenum)
					ent->client->invincible_framenum -= 10;
			}
			else
				ent->client->invincible_framenum = 0;

			if (ent->client->breather_framenum > 0)
			{
				if (ent->client->lastbreather == ent->client->breather_framenum)
					ent->client->breather_framenum -= 10;
			}
			else
				ent->client->breather_framenum = 0;

			if (ent->client->enviro_framenum > 0)
			{
				if (ent->client->lastenviro == ent->client->enviro_framenum)
					ent->client->enviro_framenum -= 10;
			}
			else
				ent->client->enviro_framenum = 0;

			ent->client->lastquad = ent->client->quad_framenum;
			ent->client->lastinvincible = ent->client->invincible_framenum;
			ent->client->lastbreather = ent->client->breather_framenum;
			ent->client->lastenviro = ent->client->enviro_framenum;

			if ((ent->client->quad_framenum) || (ent->client->invincible_framenum) || (ent->client->breather_framenum) || (ent->client->enviro_framenum))
				level.checkpowerup = level.framenum;
			else
				level.checkpowerup = 0;
		}
*/		
		if (ent->client->showmenu)
			MenuThink(ent);
		
		if (ent->client->killtimer)
		{
			if (level.time > ent->client->killtimer)
			{
				ent->client->killtimer = 0;
				ent->client->pers.respawnpause = level.time + 5.0;
				teamplay_ClientKill (ent, MOD_SUICIDE);
			}
		}
		else if (ent->client->pers.respawnpause)
		{
			if (level.time > ent->client->pers.respawnpause)
				ent->client->pers.respawnpause = 0;
			else
				return false;
		}
		
		if (level.time > keychecktime)
		{
			keychecktime = level.time + 15.0;
			teamplay_LocatePromoKey(ent, true);
			teamplay_LocateKey(ent, true);

			if (ent->flashlight)
			{
				if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= 1)
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 1;
				else
				{
					gsutil_centerprint(ent, "%s", "No cells for flashlight.\n");
					FL_make (ent);
				}
			}
		}
		else if (startvote > 0) 
		{
			if (level.time > startvote + 60.0)
			{
				gsmod_CastVote(ent, "$$Automatic$$");
				return false;
			}
			else if (level.time > nextwarn)
			{
				gi.bprintf (PRINT_MEDIUM,"Voting ends in %2.0f seconds\n", startvote + 60.0 - nextwarn);
				nextwarn += 10.0;
			}
		}
		else if ((startimpeach > 0) && (level.time > startimpeach + 240.0))
		{
			teamplay_ResetImpeach(ent->client->pers.team);
			gi.bprintf (PRINT_MEDIUM,"Impeachment did not happen. All votes have been reset.\n");
		}
		else if (ent->client->pers.showhelp == true)
		{
			if (level.time > level.helpmsg + 30.0)
			{
				level.helpmsg = level.time;
				teamplay_DisplayHelpMsg(ent);
			}
		}
	
		if (ent->client->poisontimer)
		{
			if (level.time < ent->client->poisontimer)
			{
				i = rand() & 5;
				if (i == 1)
					ent->client->ps.fov = 90;
				else if (i == 2)
					ent->client->ps.fov = 20;
				else if (i == 3)
					ent->client->ps.fov = 40;
				else if (i == 4)
					ent->client->ps.fov = 10;
				else
					ent->client->ps.fov = 60;
			}
			else
			{
				ent->client->poisontimer = 0;
				ent->client->ps.fov = 90;
				gsutil_centerprint(ent, "%s", "You feel better now\n");
			}
		}

		if (ent->client->delaypoisontimer)
		{
			if (level.time > ent->client->delaypoisontimer)
			{
				ent->client->delaypoisontimer = 0;

				if ((ent->health + ent->client->delaypoison) <= 0)
				{
					if (ent->client->delayowner)
					{
						if (ent->client->delayowner->client)
						{
							gi.bprintf (PRINT_MEDIUM,"%s was fatally poisoned by Nurse %s!\n", ent->client->pers.netname, ent->client->delayowner->client->pers.netname);
							ent->client->delayowner->client->pers.kills++;
						}
						else
							gi.bprintf (PRINT_MEDIUM,"%s was fatally poisoned!\n", ent->client->pers.netname);
					}
					else
						gi.bprintf (PRINT_MEDIUM,"%s was fatally poisoned!\n", ent->client->pers.netname);

					teamplay_ClientKill(ent, MOD_POISON);

					return false;
				}
				else
				{
					gsutil_centerprint(ent, "%s", "You have been poisoned!\n");
					ent->health += ent->client->delaypoison;
					ent->client->poisontimer = level.time + 10.0;

					return false;
				}
			}
		}
	
		if (ent->client->quadcloak)
		{
			if (ent->client->quadcloaktimer < level.time)
			{
				ent->client->quadcloak = false;
				gsutil_centerprint(ent, "%s", "You are visible again!");
				ent->s.modelindex = 255;
				ent->client->quadcloak_framenum = 0;
			}
		}

		if (ent->client->pers.player_class == 1)
		{
			if (level.time > ent->client->pers.startpause + 10.0) 
			{
				int captlevel;

				captlevel = teamplay_LowestLevel(ent);
				if (ent->client->pers.classlevel < captlevel)
				{
					ent->client->pers.classlevel = captlevel;
					teamplay_UpgradeClass(ent);
				}
				else
					ent->client->pers.classlevel = captlevel;

				ent->client->pers.startpause = level.time;

				if ((!teamkeys[ent->client->pers.team-1].capkeydropped) && (ent->client->pers.warning))
					gsutil_centerprint(ent, "%s", "You are carrying\nthe promotion key!\n");
			}
		}
		else if (ent->client->pers.player_class == 8)
		{
			if (level.time > ent->client->pers.startpause + 5.0) 
			{
				if (ent->health < ent->client->pers.max_health)
				{
					ent->health += 20;
					if (ent->health > ent->client->pers.max_health)
						ent->health = ent->client->pers.max_health;
				}

				ent->client->pers.startpause = level.time;
			}
		}
		else if (ent->client->pers.player_class == 5)
		{
			if (IsFemale(ent))
				quadtime = 60.0;
			else
				quadtime = 90.0;

			if (ent->client->special == true)
			{
				if (level.time > ent->client->specialtimer)
				{
					// suicide
					ent->client->pers.kills++;
					teamplay_ClientKill(ent, MOD_RAGE);
					ent->client->special = false;
				}
			}
			else if (level.time > ent->client->pers.startpause + quadtime) 
			{
				gsmod_DropArmor(ent, "Jacket Armor");
				gsmod_DropArmor(ent, "Combat Armor");
				gsmod_DropArmor(ent, "Body Armor");

				it = FindItem("Quad Damage");
				ent->client->pers.inventory[ITEM_INDEX(it)]++;
				Use_Quad (ent, it);

				ent->client->pers.startpause = level.time;
			}
		}
		else if (ent->client->pers.player_class == 7)
		{
			lf_pause = 2.0 - (ent->client->pers.classlevel / 2);

			if (level.time < ent->client->pausetimer + lf_pause) 
			{
				if (ent->client->thinkmsg == true)
				{
					ent->client->thinkmsg  = false;
					for (i=0 ; i<game.maxclients ; i++)
					{
						player = g_edicts + 1 + i;
						if (!player->inuse || !player->client)
							continue; 
						if (player->client->pers.team != ent->client->pers.team)
							continue;
						if (player->client->observer)
							continue;

						gsutil_centerprint (player, "%s is rewiring\na proximity grenade!\n", ent->client->pers.netname);
					}
				}
				return false;
			}

			grenades = ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))];

			if (((grenades > 50) || (ent->client->special)) && (ent->client->pers.classlevel > 2))
			{
				if ((grenades > 50) && (!ent->client->special) && (ent->health < 100))
				{
					// do nothing
				}
				else if ((ent->client->special) && ((grenades < 51) || (ent->health < 100)) )
				{
					ent->client->special = false;
					gsutil_centerprint(ent, "%s", "Self-destruct sequence aborted!\n");
				}
				else if ((ent->client->special == true) && (level.time > ent->client->specialtimer))
				{
					// suicide
					gsmod_Suicide(ent);
					ent->client->special = false;
				}
				else if ((ent->client->special == true) && (level.time > ent->client->special2timer))
				{
					if(!ent->client->showmenu)
					{
						gi.centerprintf(ent, "Self-destruct in %2.0f seconds\n", ent->client->specialtimer - ent->client->special2timer);
					}
					ent->client->special2timer += 5.0;
				}
				else if (!ent->client->special)
				{
					ent->client->special2timer = level.time + 5.0;
					ent->client->specialtimer = level.time + 30.0;
					ent->client->special = true;
					gsutil_centerprint(ent, "%s", "Self-destruct in 30 seconds\n");
				}
			}
			else
			{
				if (ent->client->special)
				{
					ent->client->special = false;
					gsutil_centerprint(ent, "%s", "Self-destruct sequence aborted!\n");
				}
			}
		}
		else if (ent->client->pers.player_class == 3)
		{
			if (level.time < ent->client->pausetimer + 2.0) 
				return false;

			if ((ucmd->forwardmove != 0) || (ucmd->sidemove != 0) || (ucmd->upmove != 0) || (ent->client->keyindex > 0))
			{
				VectorCopy (ent->s.origin, ent->client->last_position);
				ent->client->last_movement = level.time;
				if (ent->client->invisible)
				{
					ent->client->invisible = false;
					gsutil_centerprint(ent, "%s", "You are visible again!\n");
					ent->s.modelindex = 255;
				}
			}
			else
			{
				if (ent->client->pers.classlevel > 2)
					cloaktime = 2.0;
				else if (ent->client->pers.classlevel > 1)
					cloaktime = 5.0;
				else if (ent->client->pers.classlevel > 0)
					cloaktime = 8.0;
				else
					cloaktime = 10.0;

/*				if (ent->client->pers.classlevel > 0)
				{
					if (ent->light_level <= 15.0)
						cloaktime = 1.0;
					else if (ent->light_level <= 25.0)
						cloaktime = 2.0;
					else if (ent->light_level <= 35.0)
						cloaktime = 3.0;
					else if (ent->light_level <= 45.0)
						cloaktime = 4.0;
					else if (ent->light_level <= 55.0)
						cloaktime = 5.0;
					else if (ent->light_level <= 65.0)
						cloaktime = 6.0;
					else if (ent->light_level <= 75.0)
						cloaktime = 7.0;
					else if (ent->light_level <= 85.0)
						cloaktime = 8.0;
					else if (ent->light_level <= 95.0)
						cloaktime = 9.0;
					else
						cloaktime = 10.0;
				}
				else
					cloaktime = 10.0;
*/

				if (level.time > ent->client->last_movement + cloaktime)
				{
					if (!ent->client->invisible)
					{
						vec3_t		forward, wallp;	
						trace_t		tr1, tr2;

						// Setup "little look" to close wall	
						VectorCopy(ent->s.origin,wallp);

						// Cast along view angle
						AngleVectors (ent->client->v_angle, forward, NULL, NULL);	// Setup end point
						wallp[0]=ent->s.origin[0]+forward[0]*50;
						wallp[1]=ent->s.origin[1]+forward[1]*50;
						wallp[2]=ent->s.origin[2]+forward[2]*50;  	// trace
						tr1 = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

						VectorCopy(ent->s.origin,wallp);
						wallp[0]=ent->s.origin[0]-forward[0]*50;
						wallp[1]=ent->s.origin[1]-forward[1]*50;
						wallp[2]=ent->s.origin[2]-forward[2]*50;  	// trace
						tr2 = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
						
						// Line complete ? (ie. no collision)	
						if ((tr1.fraction == 1.0) && (tr2.fraction == 1.0))
						{
							ent->client->last_movement = level.time;
							// too far from a wall
						}	
						else
						{
							ent->client->invisible = true;
							gsutil_centerprint(ent, "%s", "You are now cloaked!\n");
							ent->s.modelindex = 0;
						}
					}
				}
				else
				{
					if (ent->client->invisible)
					{
						ent->client->last_movement = level.time;
						ent->client->invisible = false;
						gsutil_centerprint(ent, "%s", "You are visible again!\n");
						ent->s.modelindex = 255;
					}
				}
			}
			
			if ((ent->client->special == true) && (level.time > ent->client->specialtimer))
			{
				if (ent->client->ps.fov > 30)
				{
					ent->client->specialtimer = level.time + 0.1;
					ent->client->ps.fov -= 10;
				}
				else
					ent->client->special = false;
			}
		}
		else if (ent->client->pers.player_class == 6)
		{
			if ((ent->client->special == true) && (level.time > ent->client->specialtimer))
			{
				teamplay_ResetSkin(ent);
				gsutil_centerprint(ent, "%s", "Your disguise has worn off!\n");
			}
		}
		else if (ent->client->pers.player_class == 4)
		{
			if (ent->client->special == true)
			{
				// Check to see if the player is still within the radius to conquer
				if (ent->conquer)
				{
					VectorSubtract (ent->s.origin, ent->conquer->s.origin, v);
					len = VectorLength (v);

					distance = ent->client->pers.classlevel * 300;
					if (len > distance)
					{
						gi.bprintf (PRINT_MEDIUM,"Capture pad has been released!\n");
						ent->conquer->conquer = NULL;
						ent->conquer = NULL;
					}
				}
			}
		}
		else if (ent->client->pers.player_class == 2)
		{
			if ((ent->laserdelay) && (level.time > ent->laserdelay))
			{
				ent->laserdelay = 0;
				gsutil_centerprint(ent, "%s", "Your laser is ready!\n");
			}

			if (level.time < ent->client->pausetimer + 2.0) 
				return false;

			if ((ent->client->special == true) && (level.time > ent->client->specialtimer))
			{
				ent->client->special = false;
			}
		}
	}

	if (level.time > level.checkskin + 7.0)
	{
		for (i=0 ; i<game.maxclients ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client)
				continue; 
			if (player->client->observer)
				continue;
			if ((player->client->special) && (player->client->pers.player_class == 6) && (player->client->pers.classlevel > 1))
				continue;

			teamplay_ResetSkin(player);
		}
		level.checkskin = level.time;
	}

	// Check bleeding
	if (ent->health < 75) 
	{
		if (ent->health < 25)
			l = 25;
		if (ent->health < 50)
			l = 50;
		else
			l = 75;

		if ((rand() & l) < 2)
		{
			if ((ent->health > 0) && ((rand() & 50) < 2))
			{
				r = 1 + (rand() & 1);
				gi.sound (ent, CHAN_VOICE, gi.soundindex (va("*pain%i_%i.wav", l, r)), 1, ATTN_STATIC, 0);
			}

			SpawnDamage (TE_BLOOD, ent->s.origin, vec3_origin, 1);
		}

	}

	return true;
}


//
// Change function in p_client.c to the following:
//
//void InitClientPersistant (gclient_t *client)
//{
//	teamplay_InitClientPersistant (client);	<---
//	gsmod_InitClientPersistant (client);
//}
//
void teamplay_InitClientPersistant (gclient_t *client)
{
	gitem_t	 *item;
	char	 class_name[20];
	int		 tteam=0;
	int		 player_class=0;
	int		 classlevel;
	int		 castvote;
	int		 captures;
	int		 kills;
	int		 badsuicide;
	qboolean showhelp;
	qboolean keysbound;
	qboolean hasskins;
	qboolean startlevel;
	qboolean autoid;
	qboolean impeached;
	qboolean impeachvote;
	int		 radiochannel;
	qboolean radioon;
	qboolean warning;
	qboolean customwav;
	char	 saveskin[MAX_INFO_VALUE];
	float	 respawnpause;
	qboolean alarmmsg;
	qboolean scoutmsg;
	int		 nextplaymode;
	float	 speed_modifier;
	float	 gravity_modifier;
	qboolean fancymenu;

	if (!client)
		return;

	strcpy (saveskin, client->pers.oldskin);
	showhelp = client->pers.showhelp;
	keysbound = client->pers.keysbound;
	hasskins = client->pers.hasskins;
	autoid = client->pers.autoid;
	impeached = client->pers.impeached;
	impeachvote = client->pers.impeachvote;
	castvote = client->pers.castvote;
	radiochannel = client->pers.radiochannel;
	radioon = client->pers.radioon;
	captures = client->pers.captures;
	kills = client->pers.kills;
	badsuicide = client->pers.badsuicide;
	respawnpause = client->pers.respawnpause;
	warning = client->pers.warning;
	customwav = client->pers.customwav;
	scoutmsg = client->pers.scoutmsg;
	alarmmsg = client->pers.alarmmsg;
	speed_modifier = client->pers.speed_modifier;
	gravity_modifier = client->pers.gravity_modifier;
	fancymenu = client->pers.fancymenu;

	if (teamplay == 1)
	{
		tteam = client->pers.team;
		player_class = client->pers.player_nextclass;

		if (player_class == 1)
		{
			if (teamplay_HasLeader(tteam))
			{
				client->pers.player_nextclass = client->pers.player_class;
				player_class = client->pers.player_nextclass;
			}
		}

		nextplaymode = client->pers.nextplaymode;
		if (client->pers.playmode != nextplaymode)
		{
			if (nextplaymode == 1)
				gi.bprintf (PRINT_MEDIUM,"%s switched to offense\n", client->pers.netname);
			else
				gi.bprintf (PRINT_MEDIUM,"%s switched to defense\n", client->pers.netname);
		}

		if (client->pers.player_class != client->pers.player_nextclass)
		{
			if (player_class == 1)
				strcpy (class_name, CLASS1);
			else if (player_class == 2)
				strcpy (class_name, CLASS2);
			else if (player_class == 3)
				strcpy (class_name, CLASS3);
			else if (player_class == 4)
				strcpy (class_name, CLASS4);
			else if (player_class == 5)
				strcpy (class_name, CLASS5);
			else if (player_class == 6)
				strcpy (class_name, CLASS6);
			else if (player_class == 7)
				strcpy (class_name, CLASS7);
			else if (player_class == 8)
				strcpy (class_name, CLASS8);
			else
			{
				player_class = 2;
				strcpy (class_name, CLASS2);
			}

			if (tteam)
			{
				gi.bprintf (PRINT_MEDIUM,"%s switched classes to become a %s\n", client->pers.netname, class_name);
				
				if (client->pers.player_class != 1)
				{
					if (client->pers.classlevel > 0)
					{
						if (client->pers.classlevel == 3)
							teams.team_promos[client->pers.team-1] += 2;
						else if (client->pers.classlevel == 2)
							teams.team_promos[client->pers.team-1]++;
						client->pers.classlevel = 1;

						teamplay_CaptainMsg(client->pers.team, "Promotion points have changed\ndue to player switching classes.");
					}
					else
						client->pers.classlevel = 0;
				}
				else
					client->pers.classlevel = 0;
			}
		}
		
		classlevel = client->pers.classlevel;
		startlevel = client->pers.startlevel;
	}

	memset (&client->pers, 0, sizeof(client->pers));

	strcpy (client->pers.oldskin, saveskin);
	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	client->pers.weapon = item;
	
	// These values will be reset in PutClientInServer
	client->pers.health			= 100;
	client->pers.max_health		= 100;
	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;
	client->pers.connected		= true;
	client->pers.respawnpause	= respawnpause;

	if (teamplay == 1)
	{
		client->pers.startlevel = startlevel;
		client->pers.team = tteam;
		client->pers.player_class = player_class;
		client->pers.player_nextclass = player_class;
		client->pers.classlevel = classlevel;

		client->pers.playmode = nextplaymode;
		client->pers.nextplaymode = nextplaymode;
	}

	client->pers.showhelp = showhelp;
	client->pers.keysbound = keysbound;
	client->pers.startpause = level.time;
	client->pers.hasskins = hasskins;
	client->keyindex = 0;
	client->pers.autoid = autoid;
	client->pers.impeached = impeached;
	client->pers.impeachvote = impeachvote;
	client->pers.castvote = castvote;
	client->pers.radiochannel = radiochannel;
	client->pers.radioon = radioon;
	client->pers.captures = captures;
	client->pers.kills = kills;
	client->pers.badsuicide = badsuicide;
	client->pers.warning = warning;
	client->pers.customwav = customwav;
	client->pers.scoutmsg = scoutmsg;
	client->pers.alarmmsg = alarmmsg;
	client->pers.speed_modifier = speed_modifier;
	client->pers.gravity_modifier = gravity_modifier;
	client->pers.fancymenu = fancymenu;

	client->blindTime = 0;
	client->blindBase = 0;

	//if(!((int)teamflags->value & TEAM_KEEPTEAMS_LEVELCHANGE))
}


qboolean teamplay_CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
	//FIXME make the next line real and uncomment this block
	// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	if(!targ->client || !attacker->client)
		return false;

	if (targ->client->pers.team == attacker->client->pers.team)
		return true;
	else
		return false;
}


//
// Added function call to g_combat.c
//
//void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
//{
//	...
//	if (targ->deadflag != DEAD_DEAD)
//	{
//		//ClientObituary(self, inflictor, attacker);
//
//		teamplay_Killed(targ, inflictor, attacker, damage, point);	<---
//		targ->touch = NULL;
//		monster_death_use (targ);
//	}
//	...
//}
//
void teamplay_Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if ((!targ->client) || (!attacker->client))
		return;

	if((teamplay) && ((int)(teamflags->value) & TEAM_FRAG_PENALTY) && teamplay_CheckTeamDamage(targ, attacker))
		attacker->client->pers.kills -= 2;
}


void teamplay_DropPromote (edict_t *ent)
{
	int		index, keyindex;
	gitem_t	*it;
	edict_t	*dropped;
	
	if (!ent->client)
		return;

	if (ent->client->pers.player_class != 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Only captain has the promotion key\n");
		return;
	}
	else if (ent->client->pers.team == 1)
	{
		it = FindItemByClassname("key_promotion_blue");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key\n");
			return;
		}
	}
	else if (ent->client->pers.team == 2)
	{
		it = FindItemByClassname("key_promotion_red");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key\n");
			return;
		}
	}
	else if (ent->client->pers.team == 3)
	{
		it = FindItemByClassname("key_promotion_green");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper promotion key\n");
			return;
		}
	}
	
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		if (ent->health > 0)
			gi.cprintf (ent, PRINT_HIGH, "You're not carrying the promotion key\n");
		return;
	}
	else
	{
		if (ent->health <= 0)
			gi.bprintf (PRINT_MEDIUM,"Captain %s lost the promotion key!\n", ent->client->pers.netname);
		ent->client->pers.inventory[index] = 0;
	}

	keyindex = ent->client->pers.team;
	dropped = Drop_Item(ent, it);

	dropped->flags = keyindex;

	dropped->s.effects &= ~(EF_COLOR_SHELL);
	dropped->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	dropped->s.effects |= EF_COLOR_SHELL;

	dropped->s.renderfx |= teamkeys[keyindex-1].keyeffect;
	dropped->takedamage = 0; //no damage on flags

	teamkeys[keyindex-1].capkeydropped = true;
	teamkeys[keyindex-1].capkey = dropped;

	return;
}


void teamplay_DropKey (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	
	if (!ent->client)
		return;

	if (ent->client->keyindex == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "You're not carrying a flag\n");
		return;
	}
	else if (ent->client->keyindex == 1)
	{
		it = FindItem("Blue Team Flag");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper flag\n");
			return;
		}
	}
	else if (ent->client->keyindex == 2)
	{
		it = FindItem("Red Team Flag");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper flag\n");
			return;
		}
	}
	else if (ent->client->keyindex == 3)
	{
		it = FindItem("Green Team Key");
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "Error: Could not find a proper flag\n");
			return;
		}
	}
	
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "You're not carrying a flag\n");
		return;
	}

	it->drop (ent, it);
}

	
void teamplay_KeyStatus (edict_t *ent)
{
	int		 n, i, j;
	edict_t	 *player;
	qboolean foundplayer;
	char	 keystatus[500];

	if (!ent->client)
		return;

	if (numberteams > MAX_KEYS)
	{
		gsutil_centerprint(ent, "%s", "Too many teams. This command is only available for\ngames with 2 or 3 teams.\n");
		return;
	}
	
	teamplay_LocateKey(ent, true);

	j = sprintf(keystatus, "Flag Status:\n===========\n");
	for(n=1; n<=numberteams; n++)
	{
		if (teamkeys[n-1].isdropped == true)
		{
			j += sprintf (keystatus + j, "The %s has been dropped\n", teamkeys[n-1].keyname);
			continue;
		}

		foundplayer = false;
		for (i=0 ; i<game.maxclients ; i++)
		{
			player = g_edicts + 1 + i;
			if (!player->inuse || !player->client)
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->keyindex != n)
				continue; 

			foundplayer = true;
			j += sprintf(keystatus + j, "%s has the %s\n", player->client->pers.netname, teamkeys[n-1].keyname);

			break;
		}

		if (foundplayer == false)
			j += sprintf(keystatus + j, "The %s is at base\n", teamkeys[n-1].keyname);

		if (j > 450)
			break;
	}

	gsutil_centerprint(ent, "%s", keystatus);
}


void teamplay_Create (edict_t *ent)
{
	arena_link_t	*menulink;
	int				numcells, mincells, cellcost; 
	float			modifier;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	if ((ent->client->pers.player_class != 1) && (ent->client->pers.player_class != 8))
	{
		gsutil_centerprint(ent, "%s", "Only a Captain and Nurse can Create!\n");
		return;
	}

	if (ent->client->pers.classlevel < 1)
	{
		gsutil_centerprint(ent, "%s", "You must be level 1 to create!\n");
		return;
	}

	gsmod_Killmenu(ent);

	numcells = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];
	if (ent->client->pers.player_class == 1)
	{
		if (ent->client->pers.classlevel < 3)
		{
			mincells = 50;
			modifier = 1;
		}
		else
		{
			mincells = 25;
			modifier = 0.5;
		}

		if (numcells < mincells)
		{
			if (mincells == 50)
				gsutil_centerprint(ent, "%s", "You need at least 50 cells\nto create anything!\n");
			else
				gsutil_centerprint(ent, "%s", "You need at least 25 cells\nto create anything!\n");
			return;
		}
	}
		
	ent->client->menutype = 4;
	menulink = CreateMenu(ent, "Create a new item");

	if (ent->client->pers.player_class == 1)
	{
		if (numcells >= mincells)
		{
			cellcost = (int)(50 * modifier);
			AddMenuItem(menulink, "Shells", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Cells", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Bullets", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Grenades", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Rockets", " Cells: ", cellcost, &teamplay_MenuCallback);
		}

		if (numcells >= mincells * 2)
		{
			// Nothing
		}

		if ((numcells >= mincells * 4) && (ent->createpause < level.time))
		{
			cellcost = (int)(200 * modifier);
			AddMenuItem(menulink, "Silencer", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Rebreather", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Environment Suit", " Cells: ", cellcost, &teamplay_MenuCallback);
			AddMenuItem(menulink, "Quad Damage", " Cells: ", cellcost, &teamplay_MenuCallback);
		}
	}
	else
	{
		if (numcells < 25)
		{
			gsutil_centerprint(ent, "%s", "You need at least 25 cells\nto create anything!\n");
			return;
		}

		if ((numcells >= 25) && (ent->createpause < level.time))
		{
			cellcost = 25;
			AddMenuItem(menulink, "LargeHealth", " Cells: ", cellcost, &teamplay_MenuCallback);
		}

		if ((numcells >= 50) && (ent->createpause < level.time))
		{
			cellcost = 50;
			AddMenuItem(menulink, "MegaHealth", " Cells: ", cellcost, &teamplay_MenuCallback);
		}
	}

	AddMenuItem(menulink, "Cancel create", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}

	
void teamplay_Poison (edict_t *ent)
{
	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	if (ent->client->pers.player_class != 8)
	{
		gsutil_centerprint(ent, "%s", "Only a Nurse can Poison!\n");
		return;
	}

	if (ent->client->special2)
	{
		ent->client->special2 = false;
		gsutil_centerprint(ent, "%s", "Poison has been put away!\n");
	}
	else
	{
		ent->client->special2 = true;
		gsutil_centerprint(ent, "%s", "Poison is ready!\n");
	}
}


void teamplay_Mute (edict_t *ent)
{
	int				i;
	arena_link_t	*menulink;
	edict_t			*player;

	if (!ent->client)
		return;

	if (ent->mute1)
	{
		if (ent->mute2)
		{
			if (ent->mute3)
			{
				gsutil_centerprint(ent, "%s", "You need to unmute someone\nbefore you can mute anyone else!\n");
				return;
			}
		}
	}

	gsmod_Killmenu(ent);

	ent->client->menutype = 7;

	menulink = CreateMenu(ent, "Mute a Player");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player == ent)
			continue;
		if ((player == ent->mute1) || (player == ent->mute2) || (player == ent->mute3))
			continue;

		AddMenuItem(menulink, player->client->pers.netname, NULL, -1, &teamplay_MenuCallback);
	}
	AddMenuItem(menulink, "Cancel Mute", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}


void teamplay_Unmute (edict_t *ent)
{
	int				i;
	arena_link_t	*menulink;
	edict_t			*player;

	if (!ent->client)
		return;

	if ((!ent->mute1) && (!ent->mute2) && (!ent->mute3))
	{
		gsutil_centerprint(ent, "%s", "You do not have anyone muted!\n");
		return;
	}

	gsmod_Killmenu(ent);

	ent->client->menutype = 8;

	menulink = CreateMenu(ent, "Unmute a Player");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player == ent)
			continue;
		if ((player != ent->mute1) && (player != ent->mute2) && (player != ent->mute3))
			continue;

		AddMenuItem(menulink, player->client->pers.netname, NULL, -1, &teamplay_MenuCallback);
	}
	AddMenuItem(menulink, "Cancel Unmute", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}

	
void teamplay_TeamPromote (edict_t *ent)
{
	int				i, numpromos;
	arena_link_t	*menulink;
	edict_t			*player;
	qboolean		notalone=false, playerexists=false;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	numpromos = teams.team_promos[ent->client->pers.team-1];
	if (numpromos <= 0)
	{
		gsutil_centerprint(ent, "%s", "Your team needs to capture the flag\nbefore anyone can be promoted!\n");
		teams.team_promos[ent->client->pers.team-1] = 0;
		return;
	}
	
	if (ent->client->pers.player_class != 1)
	{
		gsutil_centerprint(ent, "%s", "Only a Captain can Promote!\n");
		return;
	}

	if (teamkeys[ent->client->pers.team-1].capkeydropped)
	{
		gsutil_centerprint(ent, "%s", "You don't have the promote key!\n");
		return;
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;
		if (player->client->resp.score < 1)
			continue;

		if ((player->client->pers.classlevel < MAX_LEVEL) && (player->client->pers.classlevel < numpromos))
			playerexists = true;
	}

	
	if (!playerexists)
	{
		if (teamplay_WholeTeam(ent, MAX_LEVEL))
			gsutil_centerprint(ent, "%s", "Your team is fully promoted!\n");
		else
			gsutil_centerprint(ent, "%s", "You cannot promote anyone right now!\n");
		return;
	}

	gsmod_Killmenu(ent);

	ent->client->menutype = 3;

	menulink = CreateMenu(ent, "Promote a Team Member");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player->client->resp.score < 1)
			continue;
		if (player == ent)
			continue;

		// Get the class of player
		if ((player->client->pers.classlevel < MAX_LEVEL) && (player->client->pers.classlevel < numpromos))
		{
			if (player->client->pers.player_class == 2)
				AddMenuItem(menulink, player->client->pers.netname, " Body: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 3)
				AddMenuItem(menulink, player->client->pers.netname, " Snip: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 4)
				AddMenuItem(menulink, player->client->pers.netname, " Sold: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 5)
				AddMenuItem(menulink, player->client->pers.netname, " Berz: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 6)
				AddMenuItem(menulink, player->client->pers.netname, " Infil: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 7)
				AddMenuItem(menulink, player->client->pers.netname, " Kami: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 8)
				AddMenuItem(menulink, player->client->pers.netname, " Nurse: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else
				AddMenuItem(menulink, player->client->pers.netname, " Error: ", player->client->pers.classlevel, &teamplay_MenuCallback);
		}

	}
	AddMenuItem(menulink, "Cancel promotion", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}


void teamplay_TeamDemote (edict_t *ent)
{
	int				i, numpromos;
	arena_link_t	*menulink;
	edict_t			*player;
	qboolean		notalone=false, playerexists=false;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	numpromos = teams.team_promos[ent->client->pers.team-1];

	if (ent->client->pers.player_class != 1)
	{
		gsutil_centerprint(ent, "%s", "Only a Captain can Demote!\n");
		return;
	}

	if (teamkeys[ent->client->pers.team-1].capkeydropped)
	{
		gsutil_centerprint(ent, "%s", "You don't have the promotion key!\n");
		return;
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;
		if (player->client->pers.classlevel > 0)
			playerexists = true;
	}

	
	if (!playerexists)
	{
		if (teamplay_WholeTeam(ent, 0))
			gsutil_centerprint(ent, "%s", "No one on your team is above level 0!\n");
		else
			gsutil_centerprint(ent, "%s", "You cannot demote anyone right now!\n");
		return;
	}

	gsmod_Killmenu(ent);

	ent->client->menutype = 9;

	menulink = CreateMenu(ent, "Demote a Team Member");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;

		// Get the class of player
		if (player->client->pers.classlevel > 0)
		{
			if (player->client->pers.player_class == 2)
				AddMenuItem(menulink, player->client->pers.netname, " Body: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 3)
				AddMenuItem(menulink, player->client->pers.netname, " Snip: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 4)
				AddMenuItem(menulink, player->client->pers.netname, " Sold: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 5)
				AddMenuItem(menulink, player->client->pers.netname, " Berz: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 6)
				AddMenuItem(menulink, player->client->pers.netname, " Infil: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 7)
				AddMenuItem(menulink, player->client->pers.netname, " Kami: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else if (player->client->pers.player_class == 8)
				AddMenuItem(menulink, player->client->pers.netname, " Nurse: ", player->client->pers.classlevel, &teamplay_MenuCallback);
			else
				AddMenuItem(menulink, player->client->pers.netname, " Error: ", player->client->pers.classlevel, &teamplay_MenuCallback);
		}

	}
	AddMenuItem(menulink, "Cancel demotion", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}


int teamplay_NumTeamPlayers(int tteam)
{
	edict_t		*player;
	int			n, numplayers;

	numplayers = 0;
	for (n=0 ; n<game.maxclients ; n++)
	{
		player = g_edicts + 1 + n;

		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != tteam)
			continue; 
		if (player->client->pers.player_class == 1)
			continue; 

		numplayers++;
	}

	return numplayers;
}


void teamplay_Impeach (edict_t *ent)
{
	edict_t		*player;
	int			n, numplayers;

	if (!ent->client)
		return;

	if (!teamplay_HasLeader(ent->client->pers.team))
	{
		gsutil_centerprint(ent, "%s", "You do not have a Captain!");
		return;
	}

	if (ent->client->pers.team == 1)
		gi.bprintf (PRINT_MEDIUM,"%i players have voted to impeach the %s Captain\n", teams.team_impeachvotes[0], team1_desc->string);
	else if (ent->client->pers.team == 2)
		gi.bprintf (PRINT_MEDIUM,"%i players have voted to impeach the %s Captain\n", teams.team_impeachvotes[1], team2_desc->string);
	else if (ent->client->pers.team == 3)
		gi.bprintf (PRINT_MEDIUM,"%i players have voted to impeach the %s Captain\n", teams.team_impeachvotes[2], team3_desc->string);
	
	if (ent->client->pers.impeachvote)
	{
		gsutil_centerprint(ent, "%s", "You have already voted!");
		return;
	}

	if (ent->client->pers.player_class == 1)
	{
		gsutil_centerprint(ent, "%s", "You cannot impeach yourself!");
		return;
	}

	startimpeach = level.time;

	ent->client->pers.impeachvote = true;
	teams.team_impeachvotes[ent->client->pers.team-1]++;

	numplayers = (int)(teamplay_NumTeamPlayers(ent->client->pers.team)*.7);
	if (teams.team_impeachvotes[ent->client->pers.team-1] >= numplayers )
	{
		for (n=0 ; n<game.maxclients ; n++)
		{
			player = g_edicts + 1 + n;
			if (!player->inuse || !player->client)
				continue; 
			if (player->client->observer)
				continue;
			if (player->client->pers.team != ent->client->pers.team)
				continue; 

			player->client->pers.impeachvote = false;

			if (player->client->pers.player_class == 1)
			{
				player->client->pers.impeached = true;
				player->client->pers.player_nextclass = 2;
				gi.bprintf (PRINT_MEDIUM,"Captain %s was impeached\n", player->client->pers.netname);
				teamplay_ClientKill(player, 0);
			}
		}

		startimpeach = 0;
		teams.team_impeachvotes[ent->client->pers.team-1] = 0;
	}
}

			
void teamplay_Heal (edict_t *ent)
{
	int				i;
	arena_link_t	*menulink;
	edict_t			*player;
	qboolean		notalone=false;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	if (ent->client->pers.player_class != 8)
	{
		gsutil_centerprint(ent, "%s", "Only a Nurse can Heal!\n");
		return;
	}

	if (ent->client->pers.classlevel < 3)
	{
		gsutil_centerprint(ent, "%s", "Only a Level 3 Nurse can\nuse healing device!\n");
		return;
	}
			
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 50)
	{
		gsutil_centerprint(ent, "%s", "You need at least 50 cells to heal!\n");
		return;
	}
	
	gsmod_Killmenu(ent);

	if (level.time < ent->client->specialtimer)
	{
		gsutil_centerprint(ent, "%s", "You must wait for the healing\ndevice to recharge!\n");
		return;
	}

	ent->client->menutype = 5;

	menulink = CreateMenu(ent, "Heal a Team Member");
	for (i=0 ; i<game.maxclients ; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;

		if (player->health < player->client->pers.max_health)
			AddMenuItem(menulink, player->client->pers.netname, " Health: ", player->health, &teamplay_MenuCallback);
	}
	AddMenuItem(menulink, "Cancel heal", NULL, -1, &teamplay_MenuCallback);

	FinishMenu(ent, menulink);
}


void teamplay_HealRadius (edict_t *ent)
{
	int			healradius, healteam, healenemy;
	edict_t		*player;

	if (!ent->client)
		return;

	if (teamplay == 0)
		return;
	
	if (ent->client->pers.player_class != 8)
	{
		gsutil_centerprint(ent, "%s", "Only a Nurse can Heal!\n");
		return;
	}

	if (ent->client->pers.classlevel < 1)
	{
		gsutil_centerprint(ent, "%s", "You must be at least Level 1 to\nuse the healing radius!\n");
		return;
	}

	if (teamplay_IsOffense(ent))
	{
		gsutil_centerprint(ent, "%s", "Only a defensive Nurse can use\nthe healing radius\n");
		return;
	}
			
	if (ent->health < 51)
	{
		gsutil_centerprint(ent, "%s", "You need at least 51 health to\nuse the healing radius!\n");
		return;
	}
	
	gsmod_Killmenu(ent);

	healradius = 75 * ent->client->pers.classlevel;
	
	player = NULL;
	healteam = 0;
	healenemy = 0;
	while ((player = findradius(player, ent->s.origin, healradius)) != NULL)
	{
		if (!player->inuse || !player->client)
			continue; 
		if (player->client->observer)
			continue;
		if (player == ent)
			continue;

		if (player->health < player->client->pers.max_health)
		{
			player->health += 100;
			if (player->health > player->client->pers.max_health)
				player->health = player->client->pers.max_health;

			if (player->client->pers.team != ent->client->pers.team)
				healenemy++;
			else
				healteam++;
		}
	}

	if (healteam || healenemy)
	{
		ent->health -= 50;

		if (!healteam)
			gsutil_centerprint(ent, "%s", "You did not heal any teammates.\n\n");
		else
		{
			if (healteam > 1)
				gsutil_centerprint(ent, "%s", "You healed some teammates!\n\n");
			else
				gsutil_centerprint(ent, "%s", "You healed a teammate!\n\n");
		}

		if (!healenemy)
			gsutil_centerprint(ent, "%s", "You did not heal any enemies.");
		else
		{
			if (healenemy > 1)
				gsutil_centerprint(ent, "%s", "You healed some enemies!");
			else
				gsutil_centerprint(ent, "%s", "You healed an enemy!");
		}
	}
	else
		gsutil_centerprint(ent, "%s", "No one was healed!\n");
}


char *teamplay_NewEntities(char *mapname, char *entities)
{
	char	entfilename[MAX_QPATH] = "";
	char	*newentities;
	int		i, islefn;
	
#ifdef _WIN32
	if (numberteams == 1)
		strcpy(entfilename, "bots\\maps\\1team\\");
	else if (numberteams == 2)
		strcpy(entfilename, "bots\\maps\\2teams\\");
	else if (numberteams == 3)
		strcpy(entfilename, "bots\\maps\\3teams\\");
#else
	if (numberteams == 1)
		strcpy(entfilename, "bots/maps/1team/");
	else if (numberteams == 2)
		strcpy(entfilename, "bots/maps/2teams/");
	else if (numberteams == 3)
		strcpy(entfilename, "bots/maps/3teams/");
#endif
	islefn = strlen(entfilename);
	for (i=0; mapname[i]; i++)
		entfilename[i + islefn] = tolower(mapname[i]);
	entfilename[i + islefn] = '\0';
	strcat(entfilename, ".ent");

	newentities = ReadTextFile(entfilename);

	if (newentities)
	{
		strcat(entities, newentities);
		newmapentities = true;
	}
	else
		newmapentities = false;

	return(entities);
}


void teamplay_TeamVoice(edict_t *ent, char *message, int msgtype)
{
	edict_t *player;
	int i;
	
	qboolean sendmsg;
	
	if (!ent->client)
		return;

	gi.sound (ent, CHAN_VOICE, gi.soundindex (message), 1, ATTN_STATIC, 0);

	for (i=0 ; i<game.maxclients ; i++)
	{
		sendmsg = false;

		player = g_edicts + 1 + i;
		if (!player->inuse || !player->client)
			continue; 
		if (player == ent)
			continue; 
		if (player->client->pers.team != ent->client->pers.team)
			continue;
		if (player->client->observer)
			continue;
		if ((ent == player->mute1) || (ent == player->mute2) || (ent == player->mute3))
			continue;

		if (player->client->pers.radioon)
		{
			if ((msgtype == 1) && ((player->client->pers.radiochannel == 2) || (player->client->pers.radiochannel == 1)) )
				sendmsg = true;
			else if ((msgtype == 2) && ((player->client->pers.radiochannel == 3) || (player->client->pers.radiochannel == 1)) )
				sendmsg = true;
			else if (msgtype == 3)
				sendmsg = true;
			else if ((msgtype == 4) && (player->client->pers.player_class == 1))
				sendmsg = true;

			if (sendmsg)
			{
				gsutil_centerprint (player, "Team message from %s!\n", ent->client->pers.netname);
				gi.sound (player, CHAN_VOICE, gi.soundindex (message), 1, ATTN_STATIC, 0);
			}
		}
	}
}


void teamplay_Voice(edict_t *ent, int msgtype, int voicetype)
{
	if (!ent->client)
		return;

	if (level.time < ent->client->voicetimer + 4.0)
	{
		if (voicetype == 1) // shout
			gi.cprintf (ent, PRINT_HIGH, "Wait to shout again...\n");
		else
			gi.cprintf (ent, PRINT_HIGH, "Radio is still in use...\n");
		return;
	}

	ent->client->voicetimer = level.time;
	
	if (voicetype == 1) // shout
	{
		if (msgtype == 1)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/guard-spot.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/guard-spot.wav"), 1, ATTN_STATIC, 0);
		}
		else if (msgtype == 2)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/cease-fire.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/cease-fire.wav"), 1, ATTN_STATIC, 0);
		}
		else if (msgtype == 3)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/you-have-flag.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/you-have-flag.wav"), 1, ATTN_STATIC, 0);
		}
		else if (msgtype == 4)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/follow-me.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/follow-me.wav"), 1, ATTN_STATIC, 0);
		}
		else if (msgtype == 5)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/medic-here.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/medic-here.wav"), 1, ATTN_STATIC, 0);
		}
		else if (msgtype == 6)
		{
			if (IsFemale(ent))
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/female/infil-open.wav"), 1, ATTN_STATIC, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("shouts/male/infil-open.wav"), 1, ATTN_STATIC, 0);
		}
	}
	else if (voicetype == 2) // team
	{
		if (msgtype == 1)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/i-have-flag.wav", 1);
			else
				teamplay_TeamVoice(ent, "radio/male/i-have-flag.wav", 1);
		}
		else if (msgtype == 2)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/off-regroup.wav", 1);
			else
				teamplay_TeamVoice(ent, "radio/male/off-regroup.wav", 1);
		}
		else if (msgtype == 3)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/get-their-key.wav", 1);
			else
				teamplay_TeamVoice(ent, "radio/male/get-their-key.wav", 1);
		}
		else if (msgtype == 4)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/perim-breach.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/perim-breach.wav", 2);
		}
		else if (msgtype == 5)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/alarm-trip.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/alarm-trip.wav", 2);
		}
		else if (msgtype == 6)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/def-backup.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/def-backup.wav", 2);
		}
		else if (msgtype == 7)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/enemy-inbound.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/enemy-inbound.wav", 2);
		}
		else if (msgtype == 8)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/cover-key.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/cover-key.wav", 2);
		}
		else if (msgtype == 9)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/enemy-has-flag.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/enemy-has-flag.wav", 2);
		}
		else if (msgtype == 10)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/lvl2-infil.wav", 2);
			else
				teamplay_TeamVoice(ent, "radio/male/lvl2-infil.wav", 2);
		}
		else if (msgtype == 11)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/im-hit.wav", 3);
			else
				teamplay_TeamVoice(ent, "radio/male/im-hit.wav", 3);
		}
		else if (msgtype == 12)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/incom-text.wav", 3);
			else
				teamplay_TeamVoice(ent, "radio/male/incom-text.wav", 3);
		}
		else if (msgtype == 13)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/key-trouble.wav", 4);
			else
				teamplay_TeamVoice(ent, "radio/male/key-trouble.wav", 4);
		}
		else if (msgtype == 14)
		{
			if (IsFemale(ent))
				teamplay_TeamVoice(ent, "radio/female/req-promote.wav", 4);
			else
				teamplay_TeamVoice(ent, "radio/male/req-promote.wav", 4);
		}
	}
}

void teamplay_Lifelink (edict_t *ent)
{
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 2)
		{
			gsutil_centerprint(ent, "%s", "Only a Bodyguard can Protect");
			return;
		}

		if (ent->client->pers.classlevel < 1)
		{
			gsutil_centerprint(ent, "%s", "You must be at least Level 1\nto Protect");
			return;
		}
	}
	else
		return;

	if (teamplay_IsOffense(ent))
	{
		gsutil_centerprint(ent, "%s", "Only a defensive Bodyguard can\nuse protect mode\n");
		return;
	}

	if (ent->client->lifelink == true)
	{
		ent->client->lifelink = false;
		gsutil_centerprint(ent, "%s", "Protect mode disabled");

		teamplay_ResetLifelink(ent);
	}
	else
	{
		ent->client->lifelink = true;
		gsutil_centerprint(ent, "%s", "Protect mode enabled");

		teams.hasprotect[ent->client->pers.team-1] = true;
	}
}

	
void teamplay_Conquer (edict_t *ent)
{
	if (!ent->client)
		return;

	if (teamplay == 1)
	{
		if (ent->client->pers.player_class != 4)
		{
			gsutil_centerprint(ent, "%s", "Only a Soldier can Conquer a pad");
			return;
		}

		if (ent->client->pers.classlevel < 1)
		{
			gsutil_centerprint(ent, "%s", "You must be at least Level 1\nto Conquer");
			return;
		}
	}
	else
		return;

	if (ent->client->special == true)
	{
		ent->client->special = false;
		if (ent->conquer)
		{
			gi.bprintf (PRINT_MEDIUM,"Capture pad has been released!\n");
			ent->conquer->conquer = NULL;
		}
		ent->conquer = NULL;
		gsutil_centerprint(ent, "%s", "Conquer mode disabled");
	}
	else
	{
		ent->client->special = true;
		gsutil_centerprint(ent, "%s", "Conquer mode enabled");
	}
}

int teamplay_VoiceCallback(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	if (!ent->client)
		return 0;

	teamplay_Voice (ent, ((menu_item_t *)selected->it)->itemvalue, ent->client->menutype);
	
	return 0;
}


void teamplay_DisplayTeamMsgs (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	gsmod_Killmenu(ent);


	ent->client->menutype = 2;
	menulink = CreateMenu(ent, "Select a Message");
	AddMenuItem(menulink, "I have the flag!", " Offense ", 1, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Offense regroup", " Offense ", 2, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Get promo key!", " Offense ", 3, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Tighten defense", " Defense ", 4, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Alarm tripped", " Defense ", 5, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Need backup", " Defense ", 6, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Heavy inbound", " Defense ", 7, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Promo key lost", " Defense ", 8, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Enemy has flag", " Defense ", 9, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Level 2 Infil", " Defense ", 10, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "I'm hit! Medic!", " Team ", 11, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Incoming text", " Team ", 12, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Help promo key", " Captain ", 13, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Requesting promo", " Captain ", 14, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Cancel Message", " ", 15, &teamplay_VoiceCallback);
	FinishMenu(ent, menulink);
}


void teamplay_DisplayShouts (edict_t *ent)
{
	arena_link_t *menulink;
	
	if (!ent->client)
		return;

	gsmod_Killmenu(ent);

	ent->client->menutype = 1;
	menulink = CreateMenu(ent, "Select a Shout");
	AddMenuItem(menulink, "Stay here and guard!", " ", 1, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Cease fire! Same team!", " ", 2, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Hey! You have their flag!", " ", 3, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "You there! Follow me!", " ", 4, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Medic! Over here!", " ", 5, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Infiltrator, open doors!", " ", 6, &teamplay_VoiceCallback);
	AddMenuItem(menulink, "Cancel Shout", " ", 7, &teamplay_VoiceCallback);
	FinishMenu(ent, menulink);
}


void teamplay_PlayMode(edict_t *ent, int playmode)
{
	if (!ent->client)
		return;

	if (playmode == 0)
	{
		if (ent->client->pers.playmode != playmode)
		{
			gsutil_centerprint(ent, "%s", "You will switch to defense\nwhen you respawn.");
		}
		else
		{
			gsutil_centerprint(ent, "%s", "You will remain on defense\nwhen you respawn.");
		}
	}
	else
	{
		if (ent->client->pers.playmode != playmode)
		{
			gsutil_centerprint(ent, "%s", "You will switch to offense\nwhen you respawn.");
		}
		else
		{
			gsutil_centerprint(ent, "%s", "You will remain on offense\nwhen you respawn.");
		}
	}

	ent->client->pers.nextplaymode = playmode;
}


void teamplay_ShowNewScoreBoard(edict_t *ent, int type)
{
	if (!ent->client)
		return;

	ent->client->showinventory = false;
	ent->client->showhelp = false;

	gsmod_Killmenu(ent);

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	ent->client->scoreboard = type;

	if (type == 1)
		teamplay_FragBoard(ent, ent);
	else
		teamplay_CaptureBoard(ent, ent);

	gi.unicast (ent, true);
}

void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0);

void teamplay_ClientCommand (edict_t *ent, char *cmd, char *parms)
{
	qboolean nonevalid=false;

	if (!ent->client)
		return;

	if (Q_stricmp (cmd, "join1") == 0)
		teamplay_ClientPickTeam (ent, 1, true);
	else if (Q_stricmp (cmd, "join2") == 0)
		teamplay_ClientPickTeam (ent, 2, true);
	else if (Q_stricmp (cmd, "join3") == 0)
		teamplay_ClientPickTeam (ent, 3, true);
	else if (Q_stricmp (cmd, "join9") == 0)
		teamplay_ClientPickTeam (ent, 9, true);
	else if (Q_stricmp (cmd, "join0") == 0)
		teamplay_ClientPickTeam (ent, 0, true);
	else if (Q_stricmp (cmd, "class1") == 0)
		teamplay_ClientPickClass (ent, 1, true);
	else if (Q_stricmp (cmd, "captain") == 0)
		teamplay_ClientPickClass (ent, 1, true);
	else if (Q_stricmp (cmd, "class2") == 0)
		teamplay_ClientPickClass (ent, 2, true);
	else if (Q_stricmp (cmd, "bodyguard") == 0)
		teamplay_ClientPickClass (ent, 2, true);
	else if (Q_stricmp (cmd, "class3") == 0)
		teamplay_ClientPickClass (ent, 3, true);
	else if (Q_stricmp (cmd, "sniper") == 0)
		teamplay_ClientPickClass (ent, 3, true);
	else if (Q_stricmp (cmd, "class4") == 0)
		teamplay_ClientPickClass (ent, 4, true);
	else if (Q_stricmp (cmd, "soldier") == 0)
		teamplay_ClientPickClass (ent, 4, true);
	else if (Q_stricmp (cmd, "class5") == 0)
		teamplay_ClientPickClass (ent, 5, true);
	else if (Q_stricmp (cmd, "berzerker") == 0)
		teamplay_ClientPickClass (ent, 5, true);
	else if (Q_stricmp (cmd, "class6") == 0)
		teamplay_ClientPickClass (ent, 6, true);
	else if (Q_stricmp (cmd, "infiltrator") == 0)
		teamplay_ClientPickClass (ent, 6, true);
	else if (Q_stricmp (cmd, "class7") == 0)
		teamplay_ClientPickClass (ent, 7, true);
	else if (Q_stricmp (cmd, "kamikazee") == 0)
		teamplay_ClientPickClass (ent, 7, true);
	else if (Q_stricmp (cmd, "class8") == 0)
		teamplay_ClientPickClass (ent, 8, true);
	else if (Q_stricmp (cmd, "nurse") == 0)
		teamplay_ClientPickClass (ent, 8, true);
	else if (Q_stricmp (cmd, "help2") == 0)
		teamplay_ShowNewScoreBoard (ent, 1);
	else if (Q_stricmp (cmd, "help3") == 0)
		teamplay_ShowNewScoreBoard (ent, 2);
	else if (Q_stricmp (cmd, "rejoin") == 0)
	{
		if ((ent->client->observer) && (ent->client->pers.team == -1))
		{
			if (gsmod_NumPlayers() >= (int)(maxplayers->value))
			{
				gsutil_centerprint(ent, "%s", "The number of active players is full.\n");
				return;
			}
		}
		
		if ((ent->client->pers.classlevel > 0) && (ent->client->pers.team))
		{
			if (ent->client->pers.classlevel == 3)
				teams.team_promos[ent->client->pers.team-1] += 3;
			else if (ent->client->pers.classlevel == 2)
				teams.team_promos[ent->client->pers.team-1] += 2;
			else if (ent->client->pers.classlevel == 1)
				teams.team_promos[ent->client->pers.team-1]++;
			teamplay_CaptainMsg(ent->client->pers.team, "Promotion points have changed\ndue to player rejoining.");
		}

		teamplay_ClientKill(ent, MOD_REJOIN);
		ent->client->pers.kills++;
		teamplay_ClientBeginDeathmatch (ent);
	}
	else
		nonevalid = true;

	if ((ent->client->pers.startlevel) && (nonevalid) && (!ent->client->observer) && (ent->health > 0))
	{
		nonevalid = false;

		if (Q_stricmp (cmd, "teamscore") == 0)
			teamplay_DisplayTeamScores (ent);

		else if (Q_stricmp (cmd, "radioon") == 0)
		{
			ent->client->pers.radioon = true;
			gsutil_centerprint(ent, "%s", "Your radio has been turned on\n");
		}
		else if (Q_stricmp (cmd, "radiooff") == 0)
		{
			ent->client->pers.radioon = false;
			gsutil_centerprint(ent, "%s", "Your radio has been turned off\n");

		}
		else if (Q_stricmp (cmd, "channelteam") == 0)
		{
			ent->client->pers.radiochannel = 1;
			gsutil_centerprint(ent, "%s", "Your radio has been switched\nto hear all team messages\n");
		}
		else if (Q_stricmp (cmd, "channeloffense") == 0)
		{
			ent->client->pers.radiochannel = 2;
			gsutil_centerprint(ent, "%s", "Your radio has been switched\nto hear offense messages\n");
		}
		else if (Q_stricmp (cmd, "channeldefense") == 0)
		{
			ent->client->pers.radiochannel = 3;
			gsutil_centerprint(ent, "%s", "Your radio has been switched\nto hear defense messages\n");
		}
		else if (Q_stricmp (cmd, "shout") == 0)
			teamplay_DisplayShouts(ent);
		else if (Q_stricmp (cmd, "team") == 0)
			teamplay_DisplayTeamMsgs(ent);

		else if (Q_stricmp (cmd, "shout1") == 0)
			teamplay_Voice(ent, 1, 1);
		else if (Q_stricmp (cmd, "shout2") == 0)
			teamplay_Voice(ent, 2, 1);
		else if (Q_stricmp (cmd, "shout3") == 0)
			teamplay_Voice(ent, 3, 1);
		else if (Q_stricmp (cmd, "shout4") == 0)
			teamplay_Voice(ent, 4, 1);
		else if (Q_stricmp (cmd, "shout5") == 0)
			teamplay_Voice(ent, 5, 1);
		else if (Q_stricmp (cmd, "shout6") == 0)
			teamplay_Voice(ent, 6, 1);

		else if (Q_stricmp (cmd, "warning") == 0)
		{
			if (ent->client->pers.player_class == 1)
			{
				if (ent->client->pers.warning)
				{
					ent->client->pers.warning = false;
					gsutil_centerprint(ent, "%s", "Promo key warning disabled\n");
				}
				else
				{
					ent->client->pers.warning = true;
					gsutil_centerprint(ent, "%s", "Promo key warning enabled\n");
				}
			}
		}

		else if (Q_stricmp (cmd, "classmenu") == 0)
			teamplay_ClassMenu(ent, false);
		else if (Q_stricmp (cmd, "teammenu") == 0)
			teamplay_TeamMenu(ent, false);
		else if (Q_stricmp (cmd, "generalmenu") == 0)
			teamplay_GeneralMenu(ent, false);
		else if (Q_stricmp (cmd, "menu") == 0)
			teamplay_MainMenu(ent);
		else if (Q_stricmp (cmd, "changeclass") == 0)
			teamplay_ClassListMenu(ent);

		else if (Q_stricmp (cmd, "team1") == 0)
			teamplay_Voice(ent, 1, 2);
		else if (Q_stricmp (cmd, "team2") == 0)
			teamplay_Voice(ent, 2, 2);
		else if (Q_stricmp (cmd, "team3") == 0)
			teamplay_Voice(ent, 3, 2);
		else if (Q_stricmp (cmd, "team4") == 0)
			teamplay_Voice(ent, 4, 2);
		else if (Q_stricmp (cmd, "team5") == 0)
			teamplay_Voice(ent, 5, 2);
		else if (Q_stricmp (cmd, "team6") == 0)
			teamplay_Voice(ent, 6, 2);
		else if (Q_stricmp (cmd, "team7") == 0)
			teamplay_Voice(ent, 7, 2);
		else if (Q_stricmp (cmd, "team8") == 0)
			teamplay_Voice(ent, 8, 2);
		else if (Q_stricmp (cmd, "team9") == 0)
			teamplay_Voice(ent, 9, 2);
		else if (Q_stricmp (cmd, "team10") == 0)
			teamplay_Voice(ent, 10, 2);
		else if (Q_stricmp (cmd, "team11") == 0)
			teamplay_Voice(ent, 11, 2);
		else if (Q_stricmp (cmd, "team12") == 0)
			teamplay_Voice(ent, 12, 2);
		else if (Q_stricmp (cmd, "team13") == 0)
			teamplay_Voice(ent, 13, 2);
		else if (Q_stricmp (cmd, "team14") == 0)
			teamplay_Voice(ent, 14, 2);

		else if (Q_stricmp (cmd, "offense") == 0)
			teamplay_PlayMode (ent, 1);
		else if (Q_stricmp (cmd, "offence") == 0)
			teamplay_PlayMode (ent, 1);
		else if (Q_stricmp (cmd, "defense") == 0)
			teamplay_PlayMode (ent, 0);
		else if (Q_stricmp (cmd, "defence") == 0)
			teamplay_PlayMode (ent, 0);

		else if (Q_stricmp (cmd, "mystatus") == 0)
			teamplay_DisplayStatus (ent, ent);
		else if (Q_stricmp (cmd, "myclass") == 0)
			teamplay_ExecScript(ent);
		else if (Q_stricmp (cmd, "playerid") == 0)
			gsmod_ShowPlayerID (ent);
		else if ((Q_stricmp (cmd, "keystatus") == 0) || (Q_stricmp (cmd, "flagstatus") == 0))
			teamplay_KeyStatus (ent);
		else if ((Q_stricmp (cmd, "dropkey") == 0) || (Q_stricmp (cmd, "dropflag") == 0))
			teamplay_DropKey (ent);
		else if (Q_stricmp (cmd, "droppromote") == 0)
			teamplay_DropPromote (ent);
		else if (Q_stricmp (cmd, "promote") == 0)
			teamplay_TeamPromote (ent);
		else if (Q_stricmp (cmd, "demote") == 0)
			teamplay_TeamDemote (ent);
		else if (Q_stricmp (cmd, "create") == 0)
			teamplay_Create (ent);
		else if (Q_stricmp (cmd, "poison") == 0)
			teamplay_Poison (ent);
		else if (Q_stricmp (cmd, "locatepromo") == 0)
			teamplay_LocatePromoKey (ent, false);
		else if ((Q_stricmp (cmd, "locatekey") == 0) || (Q_stricmp (cmd, "locateflag") == 0))
			teamplay_LocateKey (ent, false);
		else if (Q_stricmp (cmd, "heal") == 0)
			teamplay_Heal (ent);
		else if (Q_stricmp (cmd, "healradius") == 0)
			teamplay_HealRadius (ent);
		else if (Q_stricmp (cmd, "conquer") == 0)
			teamplay_Conquer (ent);
		else if (Q_stricmp (cmd, "protect") == 0)
			teamplay_Lifelink (ent);

		else if (Q_stricmp (cmd, "splitmode") == 0)
			teamplay_Toggle (ent, 4);
		else if (Q_stricmp (cmd, "split1") == 0)
			ent->client->splitmode = 1;
		else if (Q_stricmp (cmd, "split2") == 0)
			ent->client->splitmode = 2;
		else if (Q_stricmp (cmd, "split3") == 0)
			ent->client->splitmode = 3;

		else if ((Q_stricmp (cmd, "cluster") == 0) || (Q_stricmp (cmd, "clustermode") == 0))
		{
			if ((ent->client->pers.player_class == 7) && (ent->client->pers.classlevel > 1))
			{
				ent->client->weapontoggle = false;

				if (ent->client->clustermode)
				{
					ent->client->clustermode = false;
					gsutil_centerprint(ent, "%s", "Normal grenades activated\n");
				}
				else
				{
					ent->client->clustermode = true;
					gsutil_centerprint(ent, "%s", "Cluster grenades activated\n");
				}
			}
		}

		else if (Q_stricmp (cmd, "detmode") == 0)
			teamplay_Toggle (ent, 7);
		else if (Q_stricmp (cmd, "proxmode") == 0)
		{
			teamplay_Toggle (ent, 1);
			teamplay_Toggle (ent, 2);
		}
		else if (Q_stricmp (cmd, "decoymode") == 0)
			teamplay_Toggle (ent, 6);
		else if (Q_stricmp (cmd, "flashmode") == 0)
			teamplay_Toggle (ent, 8);

		else if (Q_stricmp (cmd, "disguise") == 0)
			teamplay_DisplayDisguises (ent);
		else if (Q_stricmp (cmd, "impeach") == 0)
			teamplay_Impeach (ent);
		else if (Q_stricmp (cmd, "mute") == 0)
			teamplay_Mute (ent);
		else if (Q_stricmp (cmd, "unmute") == 0)
			teamplay_Unmute (ent);
		else
			nonevalid = true;
	}

	if (nonevalid)
	{
		if (ent->health > 0)
			Cmd_Say_f (ent, false, true);
		else
			gsutil_centerprint (ent, "%s", "You're dead! Respawn if\nyou want to keep playing!");		
	}
}


// Banners
/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
//	ent->nextthink = level.time + FRAMETIME;
	ent->nextthink = level.time + .5;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
//	ent->nextthink = level.time + FRAMETIME;
	ent->nextthink = level.time + .5;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
//	ent->nextthink = level.time + FRAMETIME;
	ent->nextthink = level.time + .5;
}


void teamplay_Effects(edict_t *player)
{
	if (!player->client)
		return;

	player->s.effects &= (EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) 
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) 
		{
			player->s.effects |= EF_FLAG2;
		}
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) 
		{
			player->s.effects |= EF_FLAG1;
		}
	}

	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
	else
		player->s.modelindex3 = 0;
}
