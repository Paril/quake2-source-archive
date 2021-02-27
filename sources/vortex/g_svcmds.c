#include "g_local.h"
#include <sys/stat.h>

//#if defined(_WIN32) || defined(WIN32)
//#include <windows.h>
//#endif

//Function prototypes required for this .c file:
void dom_spawnflag (void);
void boss_spawn_tank (edict_t *ent);

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct
{
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			safe_cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numipfilters ; i++)
		if ( (in & ipfilters[i].mask) == ipfilters[i].compare)
			return (int)filterban->value;

	return (int)!filterban->value;
}


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f (void)
{
	int		i;
	
	if (gi.argc() < 3) {
		safe_cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			safe_cprintf (NULL, PRINT_HIGH, "IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	if (!StringToFilter (gi.argv(2), &ipfilters[i]))
		ipfilters[i].compare = 0xffffffff;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f (void)
{
	ipfilter_t	f;
	int			i, j;

	if (gi.argc() < 3) {
		safe_cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
		return;
	}

	if (!StringToFilter (gi.argv(2), &f))
		return;

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipfilters ; j++)
				ipfilters[j-1] = ipfilters[j];
			numipfilters--;
			safe_cprintf (NULL, PRINT_HIGH, "Removed.\n");
			return;
		}
	safe_cprintf (NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f (void)
{
	int		i;
	byte	b[4];

	safe_cprintf (NULL, PRINT_HIGH, "Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		safe_cprintf (NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;
	cvar_t	*game;

	game = gi.cvar("game", "", 0);

	if (!*game->string)
		sprintf (name, "%s/listip.cfg", GAMEVERSION);
	else
		sprintf (name, "%s/listip.cfg", game->string);

	safe_cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		safe_cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}

void	Svcmd_Test_f (void)
{
	safe_cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

//GHz START
#define ALLOW_ADMIN		1 // whether admin cmds are available

#if ALLOW_ADMIN

void SVCmd_AddExp_f (void)
{
	int		points;
	char	*name;
	edict_t *e;

	name = gi.argv(2);

	if ((e = FindPlayer(name)) != NULL)
	{
		points = atoi(gi.argv(3));

		if (points < 1)
		{
			gi.dprintf("Cannot subtract experience.\n");
			return;
		}

		e->myskills.experience += points;
		e->client->resp.score += points;
		safe_cprintf(NULL, PRINT_HIGH, "Gave %d experience to %s.\n", points, e->client->pers.netname);
		check_for_levelup(e);
		WriteToLogfile(e, va("Experience was modified. (amount = %d)\n", points));
		return;
	}
	
	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);
}

void SVCmd_AddCredits_f (void)
{
	int		points;
	char	*name;
	edict_t *e;

	name = gi.argv(2);

	if ((e = FindPlayer(name)) != NULL)
	{
		points = atoi(gi.argv(3));
		e->myskills.credits += points;
		safe_cprintf(NULL, PRINT_HIGH, "Gave %d credits to %s.\n", points, e->client->pers.netname);
		WriteToLogfile(e, va("Credits were modified. (amount = %d)\n", points));
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);
}

void SVCmd_MakeAdmin_f (void)
{
	int		lvl;
	char	*name;
	edict_t *e;

	name = gi.argv(2);
	lvl = atoi(gi.argv(3));

	if ((e = FindPlayer(name)) != NULL)
	{
		e->myskills.administrator = lvl;
		if (lvl < 0)
			lvl = 0;
		if (!lvl)
			safe_cprintf(NULL, PRINT_HIGH, "%s's administrator flag was reset.\n", e->client->pers.netname);
		else
			safe_cprintf(NULL, PRINT_HIGH, "%s's admin flag set to level %d.\n", e->client->pers.netname, lvl);
		WriteToLogfile(e, "Administrator flag was modified.\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);
}

void SVCmd_AddAbilityPoints_f (void)
{
	char	*name;
	edict_t *e;

	name = gi.argv(2);

	if ((e = FindPlayer(name)) != NULL)
	{
		e->myskills.speciality_points += atoi(gi.argv(3));
		safe_cprintf(NULL, PRINT_HIGH, "Ability points added.\n");
		WriteToLogfile(e, "Ability points were modified.\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);
}

void SVCmd_AddTalentPoints_f (void)
{
	char	*name;
	edict_t *e;

	name = gi.argv(2);

	if ((e = FindPlayer(name)) != NULL)
	{
		e->myskills.talents.talentPoints += atoi(gi.argv(3));
		safe_cprintf(NULL, PRINT_HIGH, "Talent points added.\n");
		WriteToLogfile(e, "Talent points were modified.\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);
}

void SVCmd_AddWeaponPoints_f (void)
{
	char	*name;
	edict_t *e;

	name = gi.argv(2);

	if ((e = FindPlayer(name)) != NULL)
	{
		e->myskills.weapon_points += atoi(gi.argv(3));
		safe_cprintf(NULL, PRINT_HIGH, "Weapon points added.\n");
		WriteToLogfile(e, "Weapon points were modified.\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", name);

}

int GetLevelValue (edict_t *player);

void SVCmd_Teams_f (void)
{
	int		i, lvl, levels;
	edict_t *cl_ent;

	if (!ptr->value && !domination->value && !ctf->value)
		return;
	if (level.time < pregame_time->value)
		return;

	gi.dprintf("Not Assigned:\n");
	levels = 0;
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != 0)
			continue;
		lvl = GetLevelValue(cl_ent);
		gi.dprintf("%d. %s (%d/%d/%d)\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		levels += lvl;
	}
	gi.dprintf("%d levels.\n", levels);

	gi.dprintf("Team 1:\n");
	gi.dprintf("Caps: %d\n", CTF_GetTeamCaps(1));
	levels = 0;
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != 1)
			continue;
		lvl = GetLevelValue(cl_ent);
		if (cl_ent->solid == SOLID_NOT)
			gi.dprintf("%d. %s (%d/%d/%d) SPECTATOR\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		else if (cl_ent->deadflag == DEAD_DEAD)
			gi.dprintf("%d. %s (%d/%d/%d) DEAD\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		else
			gi.dprintf("%d. %s (%d/%d/%d)\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		levels += lvl;
	}
	gi.dprintf("%d levels.\n", levels);

	gi.dprintf("Team 2:\n");
	gi.dprintf("Caps: %d\n", CTF_GetTeamCaps(2));
	levels = 0;
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != 2)
			continue;
		lvl = GetLevelValue(cl_ent);
		if (cl_ent->solid == SOLID_NOT)
			gi.dprintf("%d. %s (%d/%d/%d) SPECTATOR\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		else if (cl_ent->deadflag == DEAD_DEAD)
			gi.dprintf("%d. %s (%d/%d/%d) DEAD\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		else
			gi.dprintf("%d. %s (%d/%d/%d)\n", i+1, cl_ent->client->pers.netname, cl_ent->myskills.level, cl_ent->myskills.inuse, lvl);
		levels += lvl;
	}
	gi.dprintf("%d levels.\n", levels);
}

void SVCmd_BalanceTeams_f (void)
{
	OrganizeTeams(true);
}

void SVCmd_SpawnFlag_f (void)
{
	int		i;
	edict_t	*e, *cl_ent;

	if (!domination->value)
		return;

	DEFENSE_TEAM = 0;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!G_EntIsAlive(cl_ent))
			continue;
		// remove flag from carrier
		if (cl_ent->client->pers.inventory[flag_index])
			cl_ent->client->pers.inventory[flag_index] = 0;
	}

	if ((e = G_Find(NULL, FOFS(classname), "flag")) != NULL)
		G_FreeEdict(e); // this should cause the flag to auto-respawn
	else
		dom_spawnflag();
}

void SVCmd_EntityCount_f (void)
{
	PrintNumEntities(true);
}

/*
#define MAX_NODES	16384
void SVCmd_GenerateNodes_f (void)
{
	int		i, j, k, l=0;
	vec3_t	start, end, mins, maxs;
	trace_t	tr;

	VectorSet(mins, -32, -32, -32);
	VectorSet(maxs, 32, 32, 32);

	for (i=-4096; i<4096; i+=32) {
		for (j=-4096; j<4096; j+=32) {
			for (k=-4096; k<4096; k+=32) {
				if (l >= MAX_NODES)
				{
					gi.dprintf("ERROR: Maximum nodes reached.\n");
					return;
				}
				VectorSet(start, i, j, k);
				if (gi.pointcontents(start) != 0)
					continue;
				tr = gi.trace(start, mins, maxs, start, NULL, MASK_SOLID);
				if (tr.allsolid || tr.startsolid || (tr.fraction < 1))
					continue;
				if (tr.contents & MASK_SOLID)
					continue;
				VectorCopy(start, end);
				end[2] -= 96;
				tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
				if (tr.fraction == 1)
					continue;
				VectorCopy(start, nodes[l]);
				l++;
			}
		}
	}

	total_nodes = l;
	gi.dprintf("%d nodes generated\n", l);
}
*/
void SVCmd_SpawnBoss_f (void)
{
	edict_t *e;
	edict_t *m_worldspawn = NULL;

	// try to locate the world monster spawning entity
	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && e->mtype == M_WORLDSPAWN)
		{
			m_worldspawn = e;
			break;
		}
	}

	// if we can't find it, use world instead
	if (!m_worldspawn)
		m_worldspawn = world;

	if (!strcmp(gi.argv(2), "tank"))
		SpawnDrone(m_worldspawn, 30, true);
	else if (!strcmp(gi.argv(2), "supertank"))
		SpawnDrone(m_worldspawn, 31, true);
	else if (!strcmp(gi.argv(2), "jorg"))
		SpawnDrone(m_worldspawn, 32, true);
	else
		safe_cprintf(NULL, PRINT_HIGH, "Invalid boss type. Usage: sv spawnboss <type>\n");
}

void SVCmd_MakeBoss_f (void)
{
	edict_t *p;

	if ((p = FindPlayer(gi.argv(2))) != NULL)
	{
		if (!strcmp(gi.argv(3), "tank"))
			boss_spawn_tank(p);
		else if (!strcmp(gi.argv(3), "makron"))
			boss_makron_spawn(p);
		else
			safe_cprintf(NULL, PRINT_HIGH, "Invalid boss type. Usage: sv makeboss <player> <type>\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Couldn't find %s\n", gi.argv(2));
}

void SVCmd_ChangeClass_f (void)
{
	char	*playername = gi.argv(2);
	int		newclass = getClassNum(gi.argv(3));
	edict_t *p;

	if ((newclass < 1) || (newclass > CLASS_MAX))
	{
		safe_cprintf(NULL, PRINT_HIGH, "Invalid class number: %d\n", newclass);
		return;
	}

	if ((p = FindPlayer(playername)) != NULL)
		ChangeClass(p->client->pers.netname, newclass, 1);
	else
		safe_cprintf(NULL, PRINT_HIGH, "Couldn't find %s\n", playername);
}

void SVCmd_ExpHole_f()
{
	char *pname = gi.argv(2);
	edict_t *p;
	int value = atoi(gi.argv(3));

	if ((value < 0) || (value > 1000000) || (strlen(pname) < 1))
	{
		safe_cprintf(NULL, PRINT_HIGH, "cmd: exphole <playername> <value>.\n", pname);
		return;
	}

	if ((p = FindPlayer(pname)) != NULL)
	{
		p->myskills.nerfme = value;
		safe_cprintf(NULL, PRINT_HIGH, "%s's experience hole has been set to %d\n", p->client->pers.netname, value);
		WriteToLogfile(p, "Experience hole was modified.\n");
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", pname);
}

void SVCmd_SetTeam_f()
{
	char *pname = gi.argv(2);
	edict_t *p;
	int value = atoi(gi.argv(3));

	if ((value < 0) || (strlen(pname) < 0))
	{
		safe_cprintf(NULL, PRINT_HIGH, "cmd: setteam <playername> <value>.\n", pname);
		return;
	}
	
	if ((p = FindPlayer(pname)) != NULL)
	{
		p->teamnum = value;
		safe_cprintf(NULL, PRINT_HIGH, "%s's team has been set to %d (%s)\n", p->client->pers.netname, value, CTF_GetTeamString(value));
		return;
	}

	safe_cprintf(NULL, PRINT_HIGH, "Can't find %s.\n", pname);
}

void SVCmd_DeleteCharacter_f()
{
	char		*pname = gi.argv(2);
	char		*reason = gi.argv(3);
	char		buf[100];
	struct	stat file;

	if (strlen(pname) < 1 || strlen(reason) < 1)
	{
		safe_cprintf(NULL, PRINT_HIGH, "cmd: delchar <playername> <reason>\n", pname);
		return;
	}

	if (strlen(reason) > 50)
	{
		safe_cprintf(NULL, PRINT_HIGH, "Reason string too long (> 50 characters).\n");
		return;
	}

	// did the file we want download successfully?
	sprintf(buf, "%s\\%s.vrx", save_path->string, pname);
	if(!stat(buf, &file) && (file.st_size > 1))
	{

		
		WriteToLogFile(pname, va("Character deleted by an administrator (Reason: %s).\n", reason));
		gi.bprintf(PRINT_HIGH, "%s's character was deleted by an administrator (reason: %s)\n", pname, reason);
		sprintf(buf, "del %s\\\"%s.vrx\"", save_path->string, V_FormatFileName(pname));
		system(buf);
	}
}

#endif
//GHz END

//Ticamai START
void SV_SaveAllCharacters (void)
{
	int i;
	edict_t *ent;

	for_each_player(ent, i)
	{
		SaveCharacter(ent);
	}
	gi.dprintf("INFO: All players saved.\n");
}
//Ticamai END

// az begin

void DoMaplistFilename(int mode, char* filename);

void SV_AddMapToMaplist()
{
	int mode = atoi(gi.argv(2));
	char* map = gi.argv(3);
	char filename[256];
	qboolean IsAppend = true;
	FILE* fp;

	if (gi.argc() < 4)
	{
		safe_cprintf(NULL, PRINT_HIGH, "Missing arguments!\n");
		return;
	}

	DoMaplistFilename(mode, &filename[0]);

	if (!(fp = fopen(filename, "a")))
	{
		safe_cprintf(NULL, PRINT_HIGH, "%s: file couldn't be opened... opening in write mode.\n", filename);
		fp = fopen(filename, "w");
		IsAppend = false;
		if (!fp)
			safe_cprintf(NULL, PRINT_HIGH, "Nope. Not in writing mode neither.\n");
	}
	
	fprintf(fp, "%s\n", map);	
	fclose(fp);
}


// az end

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);

	// JABot[start]
	if (BOT_ServerCommand ())
		return;
	// [end]

	if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f ();
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f ();
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f ();
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();
	else if (Q_stricmp(cmd, "maplist") == 0)
		Cmd_Maplist_f(NULL);
	// az begin
	else if (Q_stricmp(cmd, "avglevel"))
		gi.dprintf("Average level: %d\n", AveragePlayerLevel());
#if (!defined GDS_NOMULTITHREADING) && (!defined NO_GDS)
	else if (Q_stricmp (cmd, "connectgds") == 0)
		V_GDS_StartConn();
	// az end
#endif
//GHz START
#if ALLOW_ADMIN
	else if (Q_stricmp (cmd, "addexp") == 0) 
        SVCmd_AddExp_f ();
	else if (Q_stricmp (cmd, "addtalentpoints") == 0) 
        SVCmd_AddTalentPoints_f ();
	else if (Q_stricmp (cmd, "addcredits") == 0) 
        SVCmd_AddCredits_f ();
	else if (Q_stricmp (cmd, "makeadmin") == 0) 
        SVCmd_MakeAdmin_f ();
	else if (Q_stricmp (cmd, "makeboss") == 0) 
        SVCmd_MakeBoss_f ();
	else if (Q_stricmp (cmd, "ssv pawnboss") == 0) 
		SVCmd_SpawnBoss_f();
	else if (Q_stricmp (cmd, "changeclass") == 0) 
        SVCmd_ChangeClass_f();
	else if (Q_stricmp (cmd, "addabilitypoints") == 0) 
        SVCmd_AddAbilityPoints_f ();
	else if (Q_stricmp (cmd, "addweaponpoints") == 0) 
        SVCmd_AddWeaponPoints_f ();
	else if (Q_stricmp (cmd, "teams") == 0) 
        SVCmd_Teams_f ();
	else if (Q_stricmp (cmd, "entities") == 0) 
        SVCmd_EntityCount_f ();
	else if (Q_stricmp (cmd, "eventeams") == 0)
		SVCmd_BalanceTeams_f();
//	else if (Q_stricmp (cmd, "nodes") == 0)
//		SVCmd_GenerateNodes_f();
	else if (Q_stricmp (cmd, "spawnflag") == 0)
		SVCmd_SpawnFlag_f();
	else if (Q_stricmp (cmd, "exphole") == 0)
		SVCmd_ExpHole_f();
	else if (Q_stricmp (cmd, "resetctf") == 0)
		CTF_Init();
	else if (Q_stricmp (cmd, "setteam") == 0)
		SVCmd_SetTeam_f();
		else if (Q_stricmp (cmd, "delchar") == 0)
		SVCmd_DeleteCharacter_f();
	else if (Q_stricmp (cmd, "loadmaplists") == 0)
	{
		//3.0 Load the custom map lists
		if(v_LoadMapList(MAPMODE_PVP) && v_LoadMapList(MAPMODE_PVM) 
			&& v_LoadMapList(MAPMODE_DOM) && v_LoadMapList(MAPMODE_CTF) 
			&& v_LoadMapList(MAPMODE_FFA) && v_LoadMapList(MAPMODE_INV)
			&& v_LoadMapList(MAPMODE_TRA) && v_LoadMapList(MAPMODE_INH) && v_LoadMapList(MAPMODE_VHW)
			&& v_LoadMapList(MAPMODE_TBI))
			safe_cprintf(NULL, PRINT_HIGH, "INFO: Vortex Custom Map Lists loaded successfully\n");
	}
	else if (Q_stricmp (cmd, "addtomaplist") == 0)
		SV_AddMapToMaplist ();
#endif
//GHz END
//Ticamai START
else if (Q_stricmp (cmd, "saveplayers") == 0)
	SV_SaveAllCharacters ();

//Ticamai END
	else
		safe_cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

