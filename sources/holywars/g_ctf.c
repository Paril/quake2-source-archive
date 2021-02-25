// heavily modified for HOLY WARS

#include "g_local.h"
#include "g_ctf.h"

#define MENU_WIDTH	28

static void menu_id(edict_t *ent, pmenu_t *p);
static void menu_inv(edict_t *ent, pmenu_t *p);
static void menu_board(edict_t *ent, pmenu_t *p);
static void menu_spec(edict_t *ent, pmenu_t *p);

static const char *hw_motd_file = "holywars/config/motd.txt";
static char menu_map[MENU_WIDTH + 1];
static char menu_level[MENU_WIDTH + 1];
static int motdsize;
pmenu_t *motdmenu = NULL;

// this structure *MUST* agree with the indices below
static pmenu_t mainmenu[] =
{
	{ "*Holy Wars 2.22 SE",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ menu_map,				PMENU_ALIGN_CENTER, NULL, NULL },
	{ menu_level,			PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Message of the Day",	PMENU_ALIGN_LEFT, NULL, OpenMOTDMenu },
	{ "Inventory",			PMENU_ALIGN_LEFT, NULL, menu_inv },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, menu_spec },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Preferences:",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, menu_id },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, menu_board },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL }
};

// these indices *MUST* agree with the structure above
#define MENU_MAP	1
#define MENU_LEVEL	2
#define MENU_SPEC	7
#define MENU_ID		10
#define MENU_SBRD	11


/*--------------------------------------------------------------------------*/
// code borrowed from ZOID's CTF
/*--------------------------------------------------------------------------*/


static void menu_id(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	// toggle ID mode
	CTFID_f(ent);

	// store persistent preference
	gi.cvar_set("use_player_id", (ent->client->resp.id_state) ? "1" : "0");

	OpenMainMenu(ent);
}

static void menu_inv(edict_t *ent, pmenu_t *p)
{
	int i;

	PMenu_Close(ent);

	// show inventory
	ent->client->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (ent->client->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

void OpenMOTDMenu(edict_t *ent, pmenu_t *p)
{
	FILE *fp;
	int line;
	char buf[256];

	PMenu_Close(ent);

	// initialize once only
	if (motdmenu == NULL)
	{
		fp = fopen(hw_motd_file, "rt");
		if (fp == NULL)
			return;

		// count lines
		motdsize = 0;
		while(!feof(fp))
		{
			if (fgets(buf, 255, fp) == NULL)
				break;
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			motdsize++;
		}

		// allocate menu
		line = 0;
		motdsize += 2;		// make room for title
		motdmenu = malloc(motdsize * sizeof(pmenu_t));
		if (motdmenu == NULL)
			return;
		memset(motdmenu, 0, motdsize * sizeof(pmenu_t));	// paranoia
		motdmenu[line].align = PMENU_ALIGN_CENTER;
		motdmenu[line++].text = "*Message of the Day";
		motdmenu[line].align = PMENU_ALIGN_CENTER;
		motdmenu[line++].text = "*(press TAB to continue)";

		// read message
		if (fseek(fp, 0L, SEEK_SET) != 0)
			return;
		while (!feof(fp) && line < motdsize)
		{
			if (fgets(buf, 255, fp) == NULL)
				break;
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			motdmenu[line].align = PMENU_ALIGN_CENTER;
			motdmenu[line].text = malloc(strlen(buf));
			strcpy(motdmenu[line++].text, buf);
		}
		motdsize = line;	// paranoia
		fclose(fp);
	}

	PMenu_Open(ent, motdmenu, -1, motdsize);
}

static void menu_spec(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	ent->client->update_chase = true;

	// toggle spectator mode
	if (ent->client->pers.spectator)
	{
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 0\n");
		gi.unicast(ent, true);
	}
	else
	{
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 1\n");
		gi.unicast(ent, true);
	}
}

static void menu_board(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	// toggle scoreboard mode
	gi.cvar_set("hw_scoreboard", hw_scoreboard->value ? "0" : "1");

	OpenMainMenu(ent);
}

void OpenMainMenu(edict_t *ent)
{
	// map name
	strcpy(menu_map, "map ");
	strncat(menu_map, level.mapname, MENU_WIDTH);

	// level name
	strncpy(menu_level, level.level_name, MENU_WIDTH);

	// spectator mode prompt
	if (ent->client->resp.spectator)
		mainmenu[MENU_SPEC].text = "Leave Spectator Mode";
	else
		mainmenu[MENU_SPEC].text = "Spectator Mode";

	// player ID prompt
	if (ent->client->resp.id_state)
		mainmenu[MENU_ID].text = "  player ID  \x10""active  \x11";
	else
		mainmenu[MENU_ID].text = "  player ID  \x10""inactive\x11";

	// scoreboard prompt
	if (hw_scoreboard->value)
		mainmenu[MENU_SBRD].text = "  scoreboard \x10""detailed\x11";
	else
		mainmenu[MENU_SBRD].text = "  scoreboard \x10""standard\x11";

	PMenu_Open(ent, mainmenu, ent->client->inmenu, sizeof(mainmenu) / sizeof(pmenu_t));
}


/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
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

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identification display.\n");
		ent->client->resp.id_state = false;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identification display.\n");
		ent->client->resp.id_state = true;
	}
}

void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client && !tr.ent->deadflag && !tr.ent->client->resp.spectator) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse || who->deadflag || who->client->resp.spectator)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90)
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (best - g_edicts - 1);
}
