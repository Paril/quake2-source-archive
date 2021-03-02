#include "g_local.h"
#include "p_trail.h"
#include "bot_procs.h"
#include "aj_runes.h" // AJ
#include "aj_statusbars.h" // AJ
#include "aj_menu.h" // AJ
#include "aj_scoreboard.h"

edict_t	*AddToItemList(edict_t *ent, edict_t	*head);
void	RemoveFromItemList(edict_t *ent);

typedef struct ctfgame_s
{
	int team1, team2;
	int total1, total2; // these are only set when going into intermission!
	float last_flag_capture;
	int last_capture_team;
	int team3, total3; // AJ - add support for 3TCTF
} ctfgame_t;

ctfgame_t ctfgame;
qboolean techspawn = false;

//ERASER
edict_t *flag1_ent=NULL;
edict_t *flag2_ent=NULL;
edict_t *flag3_ent=NULL; // AJ added 3TCTF support
//ERASER

cvar_t *ctf;
cvar_t *ctf_forcejoin;

// AJ added tech5
// AJ - really needs to change if having more than 5 techs...
gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4, *item_tech5;

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

//tech
"yb -129 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -102 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "
//joined overlay
"if 22 "
  "yb -104 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -75 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "
"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -24 "
  "pic 21 "
"endif "

// id view state
"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "
;
// AJ added tech5
static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4", "item_tech5",
	NULL
};

gitem_t *titems[4] = {NULL, NULL, NULL, NULL};

/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/

gitem_t *flag1_item;
gitem_t *flag2_item;

gitem_t *flag3_item; // AJ added third flag

void botGetFlag(edict_t *ent);

void CTFInit(void)
{
	ctf = gi.cvar("ctf", "0", CVAR_SERVERINFO | CVAR_LATCH);
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);

	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");
// AJ added 3rd flag
	if (!flag3_item)
		flag3_item = FindItemByClassname("item_flag_team3");
// end AJ
	memset(&ctfgame, 0, sizeof(ctfgame));
	techspawn = false;
	
	// ERASER, precache Tech powerups
	item_tech1 = FindItemByClassname("item_tech1");
	item_tech2 = FindItemByClassname("item_tech2");
	item_tech3 = FindItemByClassname("item_tech3");
	item_tech4 = FindItemByClassname("item_tech4");
// AJ added tech5
	item_tech5 = FindItemByClassname("item_tech5");
	// ERASER

}

/*--------------------------------------------------------------------------*/

char *CTFTeamName(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "RED";
	case CTF_TEAM2:
		return "BLUE";
	case CTF_TEAM3:
		return "GREEN";
	}
	return "UNKNOWN";
}

char *CTFOtherTeamName(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "BLUE";
	case CTF_TEAM2:
		return "RED";
	case CTF_TEAM3:
		return "GREEN";
	}
	return "UNKNOWN";
}

int CTFOtherTeam(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return CTF_TEAM2;
	case CTF_TEAM2:
		return CTF_TEAM1;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

//ScarFace- tell if player already has a valid LMCTF skin to keep from changing skins after death
qboolean HasRedLMCTFSkin (char *curskin, char t[64])
{
	if (strstr(t, "female"))
	{
//		gi.dprintf ("Searching red female skins...\n");
//		gi.dprintf ("Current skin is: %s\n", curskin);
		if ( strstr(curskin, "femd-r")  || strstr(curskin, "femo-r") || strstr(curskin, "femr-r")
			|| strstr(curskin, "lm-rf") || strstr(curskin, "rb-rf") )
			return true;
	}
	else if (strstr(t, "male"))
	{
//		gi.dprintf ("Searching red male skins...\n");
//		gi.dprintf ("Current skin is: %s\n", curskin);
		if ( strstr(curskin, "lm-rm")  || strstr(curskin, "maled-r") || strstr(curskin, "maleo-r")
			|| strstr(curskin, "maler-r") || strstr(curskin, "rb-rm") || strstr(curskin, "w-rm") )
			return true;
	}
//	gi.dprintf ("Invalid red LMCTF skin\n");
	return false;
}

qboolean HasBlueLMCTFSkin (char *curskin, char t[64])
{
	if (strstr(t, "female"))
	{
//		gi.dprintf ("Searching blue female skins...\n");
//		gi.dprintf ("Current skin is: %s\n", curskin);
		if ( strstr(curskin, "femd-b")  || strstr(curskin, "femo-b") || strstr(curskin, "femr-b")
			|| strstr(curskin, "lm-bf") || strstr(curskin, "rb-bf") )
			return true;
	}
	else if (strstr(t, "male"))
	{
//		gi.dprintf ("Searching blue male skins...\n");
//		gi.dprintf ("Current skin is: %s\n", curskin);
		if ( strstr(curskin, "lm-bm")  || strstr(curskin, "maled-b") || strstr(curskin, "maleo-b")
			|| strstr(curskin, "maler-b") || strstr(curskin, "rb-bm") || strstr(curskin, "w-bm") )
			return true;
	}
//	gi.dprintf ("Invalid blue LMCTF skin\n");
	return false;
}

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];
	char *curskin;
//	char *curmodel;
	char *lm_model;
	char *lm_skin;
	float r1;
	r1 = random();

	//get player's current skin name
	curskin = Info_ValueForKey (ent->client->pers.userinfo, "skin");
//	curmodel = Info_ValueForKey (ent->client->pers.userinfo, "model");

	if (!ctf->value)
		return;

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");
	switch (ent->client->resp.ctf_team)
	{
	case CTF_TEAM1:
// AJ added support for LMCTF skins (sorta)
		if (lmctf->value)
		{	
//			s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
			//ScarFace- assign skin
			if (strstr(t, "female") || strstr(t, "crakhor")) //female
			{
				lm_model = "female";
				if (r1 < 0.0909090909)
					lm_skin = "femd-r";
				else if (r1 < 0.1818181818)
					lm_skin = "femd-r2";
				else if (r1 < 0.2727272727)
					lm_skin = "femo-r";
				else if (r1 < 0.3636363636)
					lm_skin = "femo-r2";
				else if (r1 < 0.4545454545)
					lm_skin = "femr-r";
				else if (r1 < 0.5454545454)
					lm_skin = "femr-r2";
				else if (r1 < 0.6363636363)
					lm_skin = "lm-rf1";
				else if (r1 < 0.7272727272)
					lm_skin = "lm-rf2";
				else if (r1 < 0.8181818181)
					lm_skin = "rb-rf1";
				else if (r1 < 0.9090909090)
					lm_skin = "rb-rf2";
				else
					lm_skin = "rb-rf3";
			}
			else //male
			{
				lm_model = "male";
				if (r1 < 0.037037037)
					lm_skin = "lm-rm1";
				else if (r1 < 0.074074074)
					lm_skin = "lm-rm2";
				else if (r1 < 0.111111111)
					lm_skin = "lm-rm3";
				else if (r1 < 0.148148148)
					lm_skin = "lm-rm4";
				else if (r1 < 0.185185185)
					lm_skin = "lm-rm5";
				else if (r1 < 0.222222222)
					lm_skin = "lm-rm6";
				else if (r1 < 0.259259259)
					lm_skin = "lm-rm7";
				else if (r1 < 0.296296296)
					lm_skin = "maled-r";
				else if (r1 < 0.333333333)
					lm_skin = "maled-r2";
				else if (r1 < 0.370370370)
					lm_skin = "maleo-r";
				else if (r1 < 0.407407407)
					lm_skin = "maleo-r2";
				else if (r1 < 0.444444444)
					lm_skin = "maler-r";
				else if (r1 < 0.481481481)
					lm_skin = "maler-r2";
				else if (r1 < 0.518518518)
					lm_skin = "rb-rm1";
				else if (r1 < 0.555555555)
					lm_skin = "rb-rm2";
				else if (r1 < 0.592592592)
					lm_skin = "rb-rm3";
				else if (r1 < 0.629629629)
					lm_skin = "rb-rm4";
				else if (r1 < 0.666666666)
					lm_skin = "rb-rm5";
				else if (r1 < 0.703703703)
					lm_skin = "rb-rm6";
				else if (r1 < 0.740740740)
					lm_skin = "rb-rm7";
				else if (r1 < 0.777777777)
					lm_skin = "w-rm1";
				else if (r1 < 0.814814814)
					lm_skin = "w-rm2";
				else if (r1 < 0.851851851)
					lm_skin = "w-rm3";
				else if (r1 < 0.888888888)
					lm_skin = "w-rm4";
				else if (r1 < 0.925925925)
					lm_skin = "w-rm5";
				else if (r1 < 0.962962962)
					lm_skin = "w-rm6";
				else
					lm_skin = "w-rm7";
			}
			if (!HasRedLMCTFSkin (curskin, t))
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s/%s", ent->client->pers.netname, lm_model, lm_skin)); // "female", "lm-rf1"
		}
		//check if current skin name starts with "ctf_r" to allow additional CTF skins
		else if (strstr(curskin, "ctf_r"))
		{
			gi.dprintf ("Using player-selected CTF skin\n");
			//do nothing
		}
		else //force skin to red team default of "ctf_r"
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
				ent->client->pers.netname, t, CTF_TEAM1_SKIN) );
// end AJ
		break;
	case CTF_TEAM2:
// AJ added support for LMCTF skins (sorta)
		if (lmctf->value)
		{
			//ScarFace- assign random model and skin
			if (strstr(t, "female") || strstr(t, "crakhor")) //female
			{
				lm_model = "female";
				if (r1 < 0.0909090909)
					lm_skin = "femd-b";
				else if (r1 < 0.1818181818)
					lm_skin = "femd-b2";
				else if (r1 < 0.2727272727)
					lm_skin = "femo-b";
				else if (r1 < 0.3636363636)
					lm_skin = "femo-b2";
				else if (r1 < 0.4545454545)
					lm_skin = "femr-b";
				else if (r1 < 0.5454545454)
					lm_skin = "femr-b2";
				else if (r1 < 0.6363636363)
					lm_skin = "lm-bf1";
				else if (r1 < 0.7272727272)
					lm_skin = "lm-bf2";
				else if (r1 < 0.8181818181)
					lm_skin = "rb-bf1";
				else if (r1 < 0.9090909090)
					lm_skin = "rb-bf2";
				else
					lm_skin = "rb-bf3";
			}
			else //male
			{
				lm_model = "male";
				if (r1 < 0.037037037)
					lm_skin = "lm-bm1";
				else if (r1 < 0.074074074)
					lm_skin = "lm-bm2";
				else if (r1 < 0.111111111)
					lm_skin = "lm-bm3";
				else if (r1 < 0.148148148)
					lm_skin = "lm-bm4";
				else if (r1 < 0.185185185)
					lm_skin = "lm-bm5";
				else if (r1 < 0.222222222)
					lm_skin = "lm-bm6";
				else if (r1 < 0.259259259)
					lm_skin = "lm-bm7";
				else if (r1 < 0.296296296)
					lm_skin = "maled-b";
				else if (r1 < 0.333333333)
					lm_skin = "maled-b2";
				else if (r1 < 0.370370370)
					lm_skin = "maleo-b";
				else if (r1 < 0.407407407)
					lm_skin = "maleo-b2";
				else if (r1 < 0.444444444)
					lm_skin = "maler-b";
				else if (r1 < 0.481481481)
					lm_skin = "maler-b2";
				else if (r1 < 0.518518518)
					lm_skin = "rb-bm1";
				else if (r1 < 0.555555555)
					lm_skin = "rb-bm2";
				else if (r1 < 0.592592592)
					lm_skin = "rb-bm3";
				else if (r1 < 0.629629629)
					lm_skin = "rb-bm4";
				else if (r1 < 0.666666666)
					lm_skin = "rb-bm5";
				else if (r1 < 0.703703703)
					lm_skin = "rb-bm6";
				else if (r1 < 0.740740740)
					lm_skin = "rb-bm7";
				else if (r1 < 0.777777777)
					lm_skin = "w-bm1";
				else if (r1 < 0.814814814)
					lm_skin = "w-bm2";
				else if (r1 < 0.851851851)
					lm_skin = "w-bm3";
				else if (r1 < 0.888888888)
					lm_skin = "w-bm4";
				else if (r1 < 0.925925925)
					lm_skin = "w-bm5";
				else if (r1 < 0.962962962)
					lm_skin = "w-bm6";
				else
					lm_skin = "w-bm7";
			}
			if (!HasBlueLMCTFSkin (curskin, t))
				gi.configstring (CS_PLAYERSKINS+playernum, 
					va("%s\\%s/%s", ent->client->pers.netname, lm_model, lm_skin)); // "male", "lm-bm1"
		}
		//check if current skin name starts with "ctf_b" to allow additional CTF skins
		else if (strstr(curskin, "ctf_b"))
		{
			gi.dprintf ("Using player-selected CTF skin\n");
			//do nothing
		}
		else //force skin to blue team default of "ctf_b"
			gi.configstring (CS_PLAYERSKINS+playernum,
				va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN) );
// end AJ
		break;
// AJ
	case CTF_TEAM3:
		//check if current skin name starts with "ctf_g" to allow additional CTF skins
		if (strstr(curskin, "ctf_g"))
		{
			gi.dprintf ("Using player-selected CTF skin\n");
			//do nothing
		}
		else //force skin to green team default of "ctf_g"
			gi.configstring (CS_PLAYERSKINS+playernum,
				va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM3_SKIN) );
		break;
// end AJ
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi.dprintf(ent, PRINT_HIGH, "%s has been assigned skin: %s\n", ent->client->pers.netname);
}

extern int	force_team;

// ERASER, added "is_bot" flag to force assigning bot's to a team
void CTFAssignTeam(gclient_t *who, qboolean is_bot)
{
	edict_t		*player;
	int i;
// AJ added team3count
	int team1count = 0, team2count = 0, team3count = 0;

	if (!ctf->value)
		return;

	who->resp.ctf_state = CTF_STATE_START;

	if (!is_bot && !((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}

	if (force_team == CTF_TEAM1)
	{
		who->resp.ctf_team = CTF_TEAM1;
		return;
	}
	else if (force_team == CTF_TEAM2)
	{
		who->resp.ctf_team = CTF_TEAM2;
		return;
	}
// AJ - added 3rd team
	else if (force_team == CTF_TEAM3)
	{
		who->resp.ctf_team = CTF_TEAM3;
		return;
	}
// end AJ

	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team) 
		{
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
// AJ added third team
			break;
		case CTF_TEAM3:
			team3count++;
			break;
// end AJ
		}
	}
	if (team1count < team2count)
		who->resp.ctf_team = CTF_TEAM1;
	else if ((team1count < team3count) && (ttctf->value))
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = CTF_TEAM2;
	else if ((team2count < team3count) && (ttctf->value))
		who->resp.ctf_team = CTF_TEAM2;
	else if ((team3count < team1count) && (ttctf->value))
		who->resp.ctf_team = CTF_TEAM3;
	else if ((team3count < team2count) && (ttctf->value))
		who->resp.ctf_team = CTF_TEAM3;
	else if (rand() & 1)
		who->resp.ctf_team = CTF_TEAM1;
	else
		who->resp.ctf_team = CTF_TEAM2;
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if (ent->client->resp.ctf_state != CTF_STATE_START)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
// AJ added third team
	case CTF_TEAM3:
		cname = "info_player_team3";
		break;
// end AJ
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*------------------------------------------------------------------------*/
/*
CTFFragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
*/
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item, *enemy_flag_item2;
	int otherteam;
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;

	if (!ctf->value)
		return;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1) 
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
		enemy_flag_item2 = flag3_item;
	}
	else  if (targ->client->resp.ctf_team == CTF_TEAM2)
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
		enemy_flag_item2 = flag3_item;

	}
	else  //ScarFace added
	{
		flag_item = flag3_item;
		enemy_flag_item = flag1_item;
		enemy_flag_item2 = flag2_item;
	}

	// did the attacker frag the flag carrier?
	if ((targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) || (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item2)])) 
	{
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		if (!attacker->bot_client)
			gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
				CTF_FRAG_CARRIER_BONUS);

		// the the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++) {
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.ctf_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) 
	{
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		my_bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.ctf_team));
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	case CTF_TEAM3: //ScarFace added
		if (!ttctf->value)
			return;
		c = "item_flag_team3";
		break;

	default:
		return;
	}

	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL) {
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++) {
		carrier = g_edicts + i;
		if (carrier->inuse && 
			carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;
		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker)) {
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			my_bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		else
			my_bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		return;
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			my_bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
			return;
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.ctf_team == CTF_TEAM1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;

	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] &&
		targ->client->resp.ctf_team != attacker->client->resp.ctf_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}


/*------------------------------------------------------------------------*/
void	RemoveFromItemList(edict_t *ent);

void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
// AJ added third flag
	case CTF_TEAM3:
		if (!ttctf->value)
			return;
		c = "item_flag_team3";
		break;
// end AJ
	default:
		return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
		{
			RemoveFromItemList(ent);
			G_FreeEdict(ent);
		}
		else {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags(void)
{
	CTFResetFlag(CTF_TEAM1);
	CTFResetFlag(CTF_TEAM2);
	CTFResetFlag(CTF_TEAM3);
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	int i;
	edict_t *player;
	gitem_t *flag_item;
	int count;

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
// AJ added third flag
	else if (strcmp(ent->classname, "item_flag_team3") == 0)
		ctf_team = CTF_TEAM3;
// end AJ
	else {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
// AJ changed for 3tctf compat
	if (ctf_team == CTF_TEAM1)
		flag_item = flag1_item;
	else if (ctf_team == CTF_TEAM2)
		flag_item = flag2_item;
	else if (ctf_team == CTF_TEAM3) 
		flag_item = flag3_item;
// end AJ

	// ERASER
	if (other->bot_client && (other->movetarget == ent))
		other->movetarget = NULL;
	// ERASER

	if (ctf_team == other->client->resp.ctf_team) 
	{
		if (!(ent->spawnflags & DROPPED_ITEM)) 
		{
			// the flag is at home base.  if the player has the enemy
			// flag, he's just won!
			if ((other->client->pers.inventory[ITEM_INDEX(flag1_item)]) ||
				 (other->client->pers.inventory[ITEM_INDEX(flag2_item)]) ||
				 (other->client->pers.inventory[ITEM_INDEX(flag3_item)]))
			{
// AJ converted to support multiple flags
				if (other->client->pers.inventory[ITEM_INDEX(flag1_item)])
				{
					my_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTF_TEAM1));
					other->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;

					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					else if (ctf_team == CTF_TEAM2)
						ctfgame.team2++;
					else if (ctf_team == CTF_TEAM3)
						ctfgame.team3++;

					// other gets another 10 frag bonus
					other->client->resp.score += CTF_CAPTURE_BONUS;
					CTFResetFlag(CTF_TEAM1);
				}
				if (other->client->pers.inventory[ITEM_INDEX(flag2_item)])
				{
					my_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTF_TEAM2));
					other->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;

					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					else if (ctf_team == CTF_TEAM2)
						ctfgame.team2++;
					else if (ctf_team == CTF_TEAM3)
						ctfgame.team3++;

					// other gets another 10 frag bonus
					other->client->resp.score += CTF_CAPTURE_BONUS;
					CTFResetFlag (CTF_TEAM2);
				}
				if (other->client->pers.inventory[ITEM_INDEX(flag3_item)])
				{
					my_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
							other->client->pers.netname, CTFTeamName(CTF_TEAM3));
					other->client->pers.inventory[ITEM_INDEX(flag3_item)] = 0;

					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					else if (ctf_team == CTF_TEAM2)
						ctfgame.team2++;
					else if (ctf_team == CTF_TEAM3)
						ctfgame.team3++;

					// other gets another 10 frag bonus
					other->client->resp.score += CTF_CAPTURE_BONUS;
					CTFResetFlag(CTF_TEAM3);
				}
// end AJ

// AJ added support for LMCTF
				if (lmctf->value)
				{
					if (ctf_team == CTF_TEAM1)
					{
						switch (ctfgame.team1 % 3) //ScarFace- these were switched
						{	
						case 0:	
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/redscore1.wav"), 1, ATTN_NONE, 0);
							break;
						case 1:	
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/redscore2.wav"), 1, ATTN_NONE, 0);
							break;
						default:
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/redscore3.wav"), 1, ATTN_NONE, 0);
						}
					}
					else
					{
						switch (ctfgame.team1 % 3)
						{
						case 0:	
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/bluescore1.wav"), 1, ATTN_NONE, 0);
							break;
						case 1:	
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/bluescore2.wav"), 1, ATTN_NONE, 0);
							break;
						default:
							gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/bluescore3.wav"), 1, ATTN_NONE, 0);
						}
					}
				}
				else gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);
// end AJ
				// Ok, let's do the player loop, hand out the bonuses
				for (i = 1; i <= maxclients->value; i++) 
				{
					player = &g_edicts[i];
					if (!player->inuse)
						continue;

					// ERASER, if following the player, stop
					if (player->bot_client && (player->target_ent == other))
						player->target_ent = NULL;
					// ERASER

					if (player->client->resp.ctf_team != other->client->resp.ctf_team)
						player->client->resp.ctf_lasthurtcarrier = -5;
					else if (player->client->resp.ctf_team == other->client->resp.ctf_team) 
					{
						if (player != other)
							player->client->resp.score += CTF_TEAM_BONUS;
						// award extra points for capture assists
						if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time) 
						{
							my_bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
							player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
						}
						if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time) 
						{
							my_bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
							player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
						}
					}
				} 
				return false;
			}
			return false; // its at home base already
		}	
		// hey, its not home.  return it by teleporting it back
		my_bprintf(PRINT_HIGH, "%s returned the %s flag!\n", 
			other->client->pers.netname, CTFTeamName(ctf_team));
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);
		return false;
	}

	// hey, its not our flag, pick it up
	my_bprintf(PRINT_HIGH, "%s got the %s flag!\n",
		other->client->pers.netname, CTFTeamName(ctf_team));
	other->client->resp.score += CTF_FLAG_BONUS;

// AJ - end of safety as picked up the flag!
	if (other->client->safety_mode)
	{
		other->takedamage = DAMAGE_YES;
		other->client->safety_mode = false;
		other->client->ps.stats[STAT_LITHIUM_MODE] = 0;
//		other->s.effects &= 0xF7FFFFFF; // clear the yellow shell
		ent->s.effects &= EF_HALF_DAMAGE; //ScarFace- clear green shell
	}
// end AJ

	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	// ERASER: send this bot back to it's base!
	if (other->bot_client)
	{
		if (other->client->resp.ctf_team == CTF_TEAM1)
			other->movetarget = flag1_ent;
		else
			other->movetarget = flag2_ent;

		other->movetarget_want = WANT_SHITYEAH;
	}

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) 
	{
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}


//ERASER: inform bots of our location and request assistance
	count = 0;
	for (i=0; i<num_players; i++)
	{
		if (players[i]->client->resp.ctf_team == other->client->resp.ctf_team)
		{
			if (players[i]->target_ent == ent)
				players[i]->target_ent = NULL;

			if (players[i]->bot_client && (players[i] != other) &&
				(!players[i]->target_ent) &&
				(entdist(players[i], other) < 2000))
			{	// set bot to go straight to this player
				players[i]->target_ent = other;
//				if (++count >= 2)	// we have enough guardians
//					break;
			}
		}
	}
//ERASER

	return true;
}

void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if ((other == ent->owner) && 
		(  (ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
// AJ add clause to GET this time that zoid mentions but didnt code
		 ||(level.time < ent->touch_debounce_time)))
// end AJ
		return;

	Touch_Item (ent, other, plane, surf);
}

static void CTFDropFlagThink(edict_t *ent)
{

	if (ent->timestamp < (level.time - CTF_AUTO_FLAG_RETURN_TIMEOUT))
	{
		// auto return the flag
		// reset flag will remove ourselves
		if (strcmp(ent->classname, "item_flag_team1") == 0) {
			CTFResetFlag(CTF_TEAM1);
			my_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
				CTFTeamName(CTF_TEAM1));
		} else if (strcmp(ent->classname, "item_flag_team2") == 0) {
			CTFResetFlag(CTF_TEAM2);
			my_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
				CTFTeamName(CTF_TEAM2));
// AJ - added third flag test
		} else if (strcmp(ent->classname, "item_flag_team3") == 0) {
			CTFResetFlag(CTF_TEAM3);
			my_bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM3));
// end AJ
		}
	}
	else	// inform bots!
	{
		int i;

		for (i=0; i<num_players; i++)
		{
			if (!ent)
				gi.dprintf( "BUG: blah2\n");

			if ((entdist(ent, players[i]) > 1500) && !gi.inPVS(ent->s.origin, players[i]->s.origin))
				continue;

			players[i]->movetarget = ent;
		}

		ent->nextthink = level.time + 0.2;
	}
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;

	if (!ctf->value)
		return;

// AJ added 3rd flag condition
	if (!flag1_item || !flag2_item || !flag3_item)
		CTFInit();

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));

		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->timestamp = level.time;
			dropped->nextthink = level.time + 0.2;
			dropped->touch = CTFDropFlagTouch;

			CalcItemPaths(dropped);
		}
	} // AJ - removed "else" as could be carrying both
	if (self->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));

		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->timestamp = level.time;
			dropped->nextthink = level.time + 0.2;
			dropped->touch = CTFDropFlagTouch;

			CalcItemPaths(dropped);
		}
// AJ - added 3rd flag option
	}  
	if (ttctf->value && self->client->pers.inventory[ITEM_INDEX(flag3_item)]) {
		dropped = Drop_Item(self, flag3_item);
		self->client->pers.inventory[ITEM_INDEX(flag3_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM3));

		if (dropped) {
			dropped->think = CTFDropFlagThink;
			dropped->timestamp = level.time;
			dropped->nextthink = level.time + 0.2;
			dropped->touch = CTFDropFlagTouch;

			CalcItemPaths(dropped);
		}
// end AJ
	}

}

void CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
// AJ allowed (sometimes) the player to drop the flag
	edict_t *dropped = NULL;
	gitem_t	*flag;
	int flag_index;
	
	if (allow_flagdrop->value)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)]) 
		{
			flag=flag1_item;
			flag_index = ITEM_INDEX(flag1_item);
		}
// AJ - changed "else" to if statement to for extra clause
		else if (ent->client->pers.inventory[ITEM_INDEX(flag2_item)]) 
		{
			flag=flag2_item;
			flag_index = ITEM_INDEX(flag2_item);
		}
// AJ added drop third flag
		else if (ent->client->pers.inventory[ITEM_INDEX(flag3_item)]) 
		{
			flag=flag3_item;
			flag_index = ITEM_INDEX(flag3_item);
		}
// end AJ
		dropped = Drop_Item(ent, flag);

		// hack the velocity to make it bounce random
		dropped->velocity[0] = (rand() % 600) - 300;
		dropped->velocity[1] = (rand() % 600) - 300;
		dropped->think = CTFDropFlagThink;
		dropped->timestamp = level.time;
		dropped->nextthink = level.time + 0.2;
		dropped->touch_debounce_time = level.time + 1.0;
		dropped->touch = CTFDropFlagTouch;
		dropped->owner = ent; 

		CalcItemPaths(dropped);

		ent->client->pers.inventory[flag_index] = 0;
		my_bprintf(PRINT_HIGH, "%s dropped the flag!\n", 
					ent->client->pers.netname /*, CTFTeamName(CTF_TEAM1)*/);

	}
	else
	{
// end AJ
		if (rand() & 0x10) 
		{
			if (!ent->bot_client)
				gi.cprintf(ent, PRINT_HIGH, "Only losers drop flags.\n");
		}
		else
		{
			if (!ent->bot_client)
				gi.cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
		}
// AJ
	}
// end AJ
}

float	team1_rushbase_time, team2_rushbase_time;	// used by RUSHBASE command
float	team1_defendbase_time, team2_defendbase_time;

void CTFFlagThink(edict_t *ent)
{

#define CTF_DEBUG	false

	edict_t *enemy_flag;
	float dist;
	int		this_team_count, enemy_team_count;
	int		get_defenders=1, get_attackers=1;

	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);

	if (team1_rushbase_time > 0)
		team1_rushbase_time -= FRAMETIME;
	if (team2_rushbase_time > 0)
		team2_rushbase_time -= FRAMETIME;
	if (team1_defendbase_time > 0)
		team1_defendbase_time -= FRAMETIME;
	if (team2_defendbase_time > 0)
		team2_defendbase_time -= FRAMETIME;

//ERASER: make sure the flag is defended
	if (ent->last_seek_enemy < (level.time - 1))
	{
		int i, ctf_team, count=0, ideal_guarding, ctf_enemy_team;
		edict_t *plyr, *closest_guarding=NULL;

		ent->last_seek_enemy = level.time + random();	// so we try and spread out thinks

		// figure out what team this flag is
		if (strcmp(ent->classname, "item_flag_team1") == 0)
		{
			ctf_team = CTF_TEAM1;
			ctf_enemy_team = CTF_TEAM2;
			enemy_flag = flag2_ent;

			if (team1_rushbase_time > 0)
			{
				if (enemy_flag->solid == SOLID_TRIGGER)
				{
					for (i=0; i<num_players; i++)
					{
						plyr = players[i];

						if (plyr->bot_client && plyr->client->resp.ctf_team == ctf_team
							&& !CarryingFlag(plyr))
						{
							plyr->target_ent = NULL;	// don't stay guarding anything
							plyr->movetarget = enemy_flag;
							plyr->movetarget_want = 99;
						}

					}
				}

				get_defenders = false;
			}

			if (team2_defendbase_time > 0)
			{
				if (enemy_flag->solid == SOLID_TRIGGER)
				{
					for (i=0; i<num_players; i++)
					{
						plyr = players[i];

						if (plyr->bot_client && plyr->client->resp.ctf_team == ctf_enemy_team
							&& !CarryingFlag(plyr))
						{
							plyr->target_ent = enemy_flag;
							plyr->movetarget = enemy_flag;
							plyr->movetarget_want = 99;
						}

					}
				}

				get_attackers = false;
			}
		}
		else
		{
			ctf_team = CTF_TEAM2;
			ctf_enemy_team = CTF_TEAM1;
			enemy_flag = flag1_ent;

			if (team2_rushbase_time > 0)
			{
				if (enemy_flag->solid == SOLID_TRIGGER)
				{
					for (i=0; i<num_players; i++)
					{
						plyr = players[i];

						if (plyr->bot_client && plyr->client->resp.ctf_team == ctf_team
							&& !CarryingFlag(plyr))
						{
							plyr->target_ent = NULL;	// don't stay guarding anything
							plyr->movetarget = enemy_flag;
							plyr->movetarget_want = 99;
						}

					}
				}

				get_defenders = false;
			}

			if (team1_defendbase_time > 0)
			{
				if (enemy_flag->solid == SOLID_TRIGGER)
				{
					for (i=0; i<num_players; i++)
					{
						plyr = players[i];

						if (plyr->bot_client && plyr->client->resp.ctf_team == ctf_enemy_team
							&& !CarryingFlag(plyr))
						{
							plyr->target_ent = enemy_flag;
							plyr->movetarget = enemy_flag;
							plyr->movetarget_want = 99;
						}

					}
				}

				get_attackers = false;
			}
		}

		if (!get_defenders && !get_attackers)
			goto done;

		// count number of players guarding base
		this_team_count = enemy_team_count = 0;
		closest_guarding = NULL;
		for (i=0; i<num_players; i++)
		{
			plyr = players[i];

			if (plyr->client->resp.ctf_team == ctf_team)
			{
				this_team_count++;

				dist = entdist(plyr, ent);
				if (!plyr)
					gi.dprintf( "BUG: blah 1\n");

				if (	(	(plyr->target_ent == ent)
						 ||	(!plyr->bot_client)))
				{
					if (dist < BOT_GUARDING_RANGE*1.5)
					{
						closest_guarding = players[i];
						count++;
					}
				}
			}
			else if (plyr->client->resp.ctf_team == ctf_enemy_team)
			{
				enemy_team_count++;
			}
		}
/*
		if (!closest_guarding)
			ideal_guarding = 0;
		else*/ if (ent->solid != SOLID_TRIGGER)	// don't devote as many forces
			ideal_guarding = (int)ceil((1.0*(float)this_team_count)/4.0);
		else
			ideal_guarding = (int)ceil((1.0 * (float)this_team_count)/2.0);

		if (get_defenders)
		{
			if (count < ideal_guarding)
			{	// summon the closest "out of range" bot for defense
				edict_t *closest=NULL;
				float	dist, closest_dist=999999;

				for (i=0; i<num_players; i++)
				{
					plyr = players[i];

					if (	(plyr->bot_client)
						&&	(plyr->client->resp.ctf_team == ctf_team)
						&&	(plyr->target_ent != ent)
	//					&&	((dist = entdist(plyr, ent)) > BOT_GUARDING_RANGE)
						&&	(plyr->bot_fire != botBlaster)
						&&	((dist = entdist(plyr, ent)) < closest_dist))
					{
						closest = plyr;
						closest_dist = dist;
						if (!plyr)
							gi.dprintf( "BUG: summon the closest bot plyr var==NULL\n");
					}
				}

				if (closest)	// tell this bot to guard the flag!
				{
	if (CTF_DEBUG)
	gi.dprintf( "Sending %s to defend flag\n", closest->client->pers.netname);
					closest->target_ent = ent;
					closest->movetarget = ent;
					closest->movetarget_want = WANT_SHITYEAH;
				}
			}
			else if (count > ideal_guarding /*&& closest_guarding*/)
			{	// too many guarding flag, release a bot
				for (i=0; (i<num_players && count>ideal_guarding); i++)
				{
					plyr = players[i];

					if (	(plyr->bot_client)
						&&	(plyr->client->resp.ctf_team == ctf_team)
						&&	(plyr->target_ent == ent))
					{
	if (CTF_DEBUG)
	gi.dprintf( "Releasing %s from flag defense.\n", plyr->client->pers.netname);
						plyr->target_ent = NULL;
						if (plyr->movetarget == ent)
							plyr->movetarget = NULL;

						count--;
					}
				}
			}
		}

		// check for invaded base
		count = 0;
		for (i=0; i<num_players; i++)
		{
			if (	(players[i]->client->resp.ctf_team == ctf_enemy_team)
				&&	((players[i]->movetarget == ent) || !players[i]->bot_client)
				&&	(entdist(players[i], ent) < BOT_GUARDING_RANGE))
			{
				count++;
				if (!players[i])
					gi.dprintf( "BUG: invaded base check.\n");
			}
		}

		if (closest_guarding && (ent->solid == SOLID_TRIGGER))
		{
			if (count > 1)
			{	// message all teammates, INVASION! NEED HELP!
				if (ent->last_pain < (level.time - 4))
				{
					for (i=0; i<num_players; i++)
					{
						if (players[i]->client->resp.ctf_team == ctf_team)
						{	// FIXME: add to custom chat.txt
							if (!players[i]->bot_client && bot_chat->value)
								gi.cprintf(players[i], PRINT_CHAT, "%s: base under attack!\n", closest_guarding->client->pers.netname);
							else if (get_defenders && (players[i]->target_ent != ent) && !CarryingFlag(players[i]) && (entdist(players[i], ent) < 1500))
							{
if (CTF_DEBUG)
gi.dprintf( "Sending %s to defend flag\n", players[i]->client->pers.netname);
								players[i]->target_ent = ent;
								players[i]->movetarget = ent;
								players[i]->movetarget_want = WANT_SHITYEAH;
							}
						}
					}

				}

				ent->last_pain = level.time;
			}
			else if ((count == 0) && ent->last_pain && (ent->last_pain > (level.time - 7)))
			{
				ent->last_pain = 0;

				for (i=0; i<num_players; i++)
				{
					if (bot_chat->value && !players[i]->bot_client && (players[i]->client->resp.ctf_team == ctf_team))
					{	// FIXME: add to custom chat.txt
						gi.cprintf(players[i], PRINT_CHAT, "%s: base secured!\n", closest_guarding->client->pers.netname);
					}
				}
			}
		}

		if (get_attackers)
		{
			// Send enemy troops to us?
			if (count < ((int)ceil((1.0*(float)enemy_team_count)/4.0)))
			{
				for (i=0; i<num_players; i++)
				{
					if (!players[i]->bot_client)
						continue;
					if (players[i]->client->resp.ctf_team != ctf_enemy_team)
						continue;
					if (players[i]->movetarget == ent)
						continue;

					dist = entdist(players[i], ent);
					if (!players[i])
						gi.dprintf( "BUG. send enemy troops\n");

					if (dist > 1200)
					{
						if (players[i]->target_ent && (dist > 2000))
							continue;
						if ((dist > 1500) && (players[i]->bot_fire == botBlaster))
							continue;
					}
					else if (ent->solid != SOLID_TRIGGER)
					{
						continue;
					}


	if (CTF_DEBUG)
	gi.dprintf( "Sending %s to ATTACK flag\n", players[i]->client->pers.netname);
					players[i]->movetarget = ent;
					players[i]->movetarget_want = WANT_SHITYEAH;
				}
			}
			else if (ent->solid != SOLID_TRIGGER) 	// release an attacker
			{
				for (i=0; i<num_players; i++)
				{
					if (players[i]->client->resp.ctf_team == ctf_team)
						continue;

					if ((players[i]->target_ent == ent) || (players[i]->movetarget == ent))
					{
	if (CTF_DEBUG)
	gi.dprintf( "Releasing %s from attacking flag\n", players[i]->client->pers.netname);
						players[i]->target_ent = NULL;
						players[i]->enemy = NULL;
						break;
					}
				}
			}
		}

	}
//ERASER

done:

	ent->nextthink = level.time + FRAMETIME;
}

void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;
//	edict_t *flag_thinker;

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
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);	

	//ERASER
	if (strcmp(ent->classname, "item_flag_team1") == 0)
	{
		flag1_ent = ent;
		ent->count = CTF_TEAM1;
	}
// AJ added test to allow 3 flags
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
	{
		flag2_ent = ent;
		ent->count = CTF_TEAM2;
	}
// AJ added 3rd flag
	else if (strcmp(ent->classname, "item_flag_team3") == 0)
	{
		flag3_ent = ent;
		ent->count = CTF_TEAM3;
	}
// end AJ

	CalcItemPaths(ent);
	//ERASER

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;

}

void CTFEffects(edict_t *player)
{
	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) {
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			player->s.effects |= EF_FLAG1;
		}
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			player->s.effects |= EF_FLAG2;
		}
	}
	//if both red and blue flags -ScarFace
	if ((player->client->pers.inventory[ITEM_INDEX(flag1_item)]) && (player->client->pers.inventory[ITEM_INDEX(flag2_item)]))
		player->s.modelindex3 = gi.modelindex("models/flags/flag4.md2");
	//if both red and green flags
	else if ((player->client->pers.inventory[ITEM_INDEX(flag1_item)]) && (player->client->pers.inventory[ITEM_INDEX(flag3_item)]))
		player->s.modelindex3 = gi.modelindex("models/flags/flag5.md2");
	//if both blue and green flags
	else if ((player->client->pers.inventory[ITEM_INDEX(flag2_item)]) && (player->client->pers.inventory[ITEM_INDEX(flag3_item)]))
		player->s.modelindex3 = gi.modelindex("models/flags/flag6.md2");
	//red flag
	else if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		if (ttctf->value)  //use 3Team CTF model path
			player->s.modelindex3 = gi.modelindex("models/flags/flag1.md2");
		else
			player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
	//blue flag
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		if (ttctf->value)  //use 3Team CTF model path
			player->s.modelindex3 = gi.modelindex("models/flags/flag2.md2");
		else
			player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
	//green flag
	else if (player->client->pers.inventory[ITEM_INDEX(flag3_item)])
		player->s.modelindex3 = gi.modelindex("models/flags/flag3.md2");
	else
		player->s.modelindex3 = 0;
}

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	ctfgame.total1 = ctfgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			ctfgame.total1 += game.clients[i].resp.score;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			ctfgame.total2 += game.clients[i].resp.score;
// AJ added third team
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			ctfgame.total3 += game.clients[i].resp.score;
// end AJ
	}
}

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

// AJ removed the static prefix
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
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_PLAYERSKINS + (ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse)
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

void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2, p3; // AJ - added p3
	edict_t *e;

	// logo headers for the frag display
// AJ added support for LMCTF filenames - also support for 3TCTF
	if (lmctf->value)
	{
		ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("redtag");
		ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("bluetag");
	}
	else if (ttctf->value)
	{
		ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("3tctfsb1");
		ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("3tctfsb2");
		ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = gi.imageindex ("3tctfsb3");
	} else
	{
		ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
		ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");
	}
// end AJ

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) 
	{ // blink 1/8th second
		if (ttctf->value)
		{
			if (ctfgame.team1 > ctfgame.team2 && ctfgame.team1 > ctfgame.team3)
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.team2 > ctfgame.team1 && ctfgame.team2 > ctfgame.team3)
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (ctfgame.team3 > ctfgame.team1 && ctfgame.team3 > ctfgame.team2)
				ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = 0;
			else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.total2 > ctfgame.total1) 
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else 
			{ // tie game!
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM3_HEADER] = 0;
			}
		}
		else
		{
			// note that ctfgame.total[12] is set when we go to intermission
			if (ctfgame.team1 > ctfgame.team2)
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.team2 > ctfgame.team1)
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			else if (ctfgame.total2 > ctfgame.total1) 
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			else { // tie game!
				ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
				ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
			}
		}
	}

	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
// AJ added LMCTF filename support
	if (lmctf->value)
		p1 = gi.imageindex ("redlion_i");
// end AJ
/*	else if (ttctf->value) //3tctf support
	{
		if (ent->client->resp.ctf_team == CTF_TEAM1)
			p1 = gi.imageindex ("3tctfrs");
		else
			p1 = gi.imageindex ("3tctfr");
	}
*/
	else p1 = gi.imageindex ("i_ctf1");
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player, default to dropped
// AJ added LMCTF filename support
			if (lmctf->value)
				p1 = gi.imageindex ("redflagdown");
// end AJ
/*			else if (ttctf->value) //3tctf support
			{
				if (ent->client->resp.ctf_team == CTF_TEAM1)
					p1 = gi.imageindex ("3tctfrds");
				else
					p1 = gi.imageindex ("3tctfrd");
			}
*/
			else p1 = gi.imageindex ("i_ctf1d");
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)]) {
					// enemy has it
// AJ added LMCTF filename support
					if (lmctf->value)
						p1 = gi.imageindex ("redflaggone");
// end AJ
/*					else if (ttctf->value) //3tctf support
					{
						if (ent->client->resp.ctf_team == CTF_TEAM1)
							p1 = gi.imageindex ("3tctfrts");
						else
							p1 = gi.imageindex ("3tctfrt");
					}
*/
					else p1 = gi.imageindex ("i_ctf1t");
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)// must be dropped
		{
// AJ added LMCTF filename support

			if (lmctf->value)
				p1 = gi.imageindex ("redflagdown");
// end AJ
/*			else if (ttctf->value) //3tctf support
			{
				if (ent->client->resp.ctf_team == CTF_TEAM1)
					p1 = gi.imageindex ("3tctfrds");
				else
					p1 = gi.imageindex ("3tctfrd");
			}
*/
			else p1 = gi.imageindex ("i_ctf1d");
		}
	}
// AJ added LMCTF filename support
	if (lmctf->value)
		p2 = gi.imageindex ("bluewolf_i");
// end AJ
/*	else if (ttctf->value) //3tctf support
	{
		if (ent->client->resp.ctf_team == CTF_TEAM2)
			p1 = gi.imageindex ("3tctfbs");
		else
			p1 = gi.imageindex ("3tctfb");
	}
*/
	else p2 = gi.imageindex ("i_ctf2");
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player, default to dropped
// AJ added LMCTF filename support
			if (lmctf->value)
				p2 = gi.imageindex ("blueflagdown"); 
// end AJ
/*			else if (ttctf->value) //3tctf support
			{
				if (ent->client->resp.ctf_team == CTF_TEAM2)
					p1 = gi.imageindex ("3tctfbds");
				else
					p1 = gi.imageindex ("3tctfbd");
			}
*/
			else p2 = gi.imageindex ("i_ctf2d"); // default to dropped

			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)]) {
					// enemy has it
// AJ added LMCTF filename support
					if (lmctf->value)
						p2 = gi.imageindex ("blueflaggone");
// end AJ
/*					else if (ttctf->value) //3tctf support
					{
						if (ent->client->resp.ctf_team == CTF_TEAM2)
							p1 = gi.imageindex ("3tctfbts");
						else
							p1 = gi.imageindex ("3tctfbt");
					}
*/
					else p2 = gi.imageindex ("i_ctf2t");

					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM) // must be dropped
		{
// AJ added LMCTF filename support
			if (lmctf->value)
				p2 = gi.imageindex ("blueflagdown");
// end AJ
/*			else if (ttctf->value) //3tctf support
			{
				if (ent->client->resp.ctf_team == CTF_TEAM2)
					p1 = gi.imageindex ("3tctfbds");
				else
					p1 = gi.imageindex ("3tctfbd");
			}
*/
			else p2 = gi.imageindex ("i_ctf2d");
		}

	}

// AJ - add green team status tile
	if (ttctf->value)
	{
		if (ent->client->resp.ctf_team == CTF_TEAM3)
			p3 = gi.imageindex ("3tctfgs");
		else
			p3 = gi.imageindex ("3tctfg");
		e = G_Find(NULL, FOFS(classname), "item_flag_team3");
		if (e != NULL) 
		{
			if (e->solid == SOLID_NOT) 
			{
				int i;
				// not at base
				// check if on player, default to dropped
				if (ent->client->resp.ctf_team == CTF_TEAM3)
					p3 = gi.imageindex ("3tctfgds");
				else
					p3 = gi.imageindex ("3tctfgd"); 
				for (i = 1; i <= maxclients->value; i++)
					if (g_edicts[i].inuse &&
						g_edicts[i].client->pers.inventory[ITEM_INDEX(flag3_item)]) 
					{
						// enemy has it
						if (ent->client->resp.ctf_team == CTF_TEAM3)
							p3 = gi.imageindex ("3tctfgts");
						else
							p3 = gi.imageindex ("3tctfgt");
						break;
					}
			} 
			else if (e->spawnflags & DROPPED_ITEM)
				if (ent->client->resp.ctf_team == CTF_TEAM3)
					p3 = gi.imageindex ("3tctfgds");
				else
					p3 = gi.imageindex ("3tctfgd"); // must be dropped
		}

		ent->client->ps.stats[STAT_CTF_TEAM3_PIC] = p3;
	}
// end AJ

	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

	if (ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5) {
		if (ctfgame.last_capture_team == CTF_TEAM1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		else
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
	}

	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team1;
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team2;
	ent->client->ps.stats[STAT_CTF_TEAM3_CAPS] = ctfgame.team3;

	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if ((ent->client->resp.ctf_team == CTF_TEAM1 || ent->client->resp.ctf_team == CTF_TEAM3) &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] && (level.framenum & 8))
// AJ added LMCTF filename support
	{
		if (lmctf->value)
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("bluewolf_i");
	//	else if (ttctf->value)
	//		ent->client->ps.stats[STAT_CTF_FLAG_PIC]  = gi.imageindex ("3tctfb");
		else ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf2");
	}
// end AJ
	else if ((ent->client->resp.ctf_team == CTF_TEAM2 || ent->client->resp.ctf_team == CTF_TEAM3)  &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] && (level.framenum & 8))
// AJ added LMCTF filename support
	{
		if (lmctf->value)
			ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("redlion_i");
	//	else if (ttctf->value)
	//		ent->client->ps.stats[STAT_CTF_FLAG_PIC]  = gi.imageindex ("3tctfr");
		else ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf1");
	}
// end AJ
	//ScarFace- here is the missing green flag icon!!!
	else if ((ent->client->resp.ctf_team == CTF_TEAM1 || ent->client->resp.ctf_team == CTF_TEAM2)  &&
		ent->client->pers.inventory[ITEM_INDEX(flag3_item)] && (level.framenum & 8))
	{
		ent->client->ps.stats[STAT_CTF_FLAG_PIC]  = gi.imageindex ("3tctfg");
	}

	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
// AJ added LMCTF filename support
	if (lmctf->value)
	{
//		if (ent->client->resp.ctf_team == CTF_TEAM1)
//			ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
//		else if (ent->client->resp.ctf_team == CTF_TEAM2)
//			ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");
	}
	else
	{
		if (ent->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
		else if (ent->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");
	}
// end AJ

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	if (ent->client->resp.id_state)
		CTFSetIDView(ent);
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}


/*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

void KillGrappleSoundKiller(edict_t *owner)
{
// if there is a grapple sound killer attached to this grapple, kill it
	edict_t *trav=NULL;

	while (trav = G_Find(trav, FOFS(classname), "grapple_sound_killer"))
	{
		if (trav->owner == owner)
		{
			G_FreeEdict(trav);
			break;
		}
	}
}

void KillGrappleSound(edict_t *ent)
{
	gi.sound (ent->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), 1, ATTN_NORM, 0);
	G_FreeEdict(ent);
}

void SpawnGrappleSoundKiller(edict_t *owner)
{
// Fix for continuous sounds
	edict_t	*ent;

	KillGrappleSoundKiller(owner);

	ent = G_Spawn();
	ent->classname = "grapple_sound_killer";
	ent->owner = owner;
	ent->think = KillGrappleSound;
	ent->nextthink = level.time + 0.5;	// allow some time for them to get pulled
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		KillGrappleSoundKiller(self->owner);

// AJ simulated offhand grapple sound
		if (!hook_offhand->value)
			gi.sound(self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		else gi.sound(self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("misc/menu3.wav"), volume, ATTN_NORM, 0);
// end AJ

//		gi.sound (self, CHAN_RELIABLE+CHAN_ITEM, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	// AJ - added extra hook-sky condition
	if (surf && (surf->flags & SURF_SKY) && !hook_sky->value)
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage || other->client) {
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->owner->client->ctf_grapplestart = level.time;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;


// AJ quick hack to get the sounds the same as offhand... needs rework
	if (!hook_offhand->value)
		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);

// (this one wasn't taken out by AJ)
//	gi.sound (self, CHAN_RELIABLE+CHAN_ITEM, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);

// AJ quick hack to get the sounds the same as offhand... 
	if (!hook_offhand->value)
		gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);
	else gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/xpld_wat.wav"), volume, ATTN_NORM, 0); // was world/land.wav
// end AJ

	SpawnGrappleSoundKiller(self->owner);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;
	
	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 16, 16, self->owner->viewheight-8);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

if (!self->owner->bot_client)
	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) {
		CTFResetGrapple(self);
		return;
	}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			if (!hook_offhand->value)
				gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
			return;
		}
		if (!self->enemy || self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	if (!hook_offhand->value)
		CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			if (!hook_offhand->value)
				gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
//			gi.sound (self, CHAN_RELIABLE+CHAN_ITEM, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
// AJ: changed constant CTF_GRAPPLE_PULL_SPEED to hook_pullspeed->value		
		VectorScale(hookdir, hook_pullspeed->value, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFGrappleThink( edict_t *self )
{
	if (!self->owner || !self->owner->client)
	{
		G_FreeEdict(self);
		return;
	}

	if (	(self->owner->health <= 0)
		||	(self->owner->client->ctf_grapple != self))
	{
		gclient_t *cl;

		cl = self->owner->client;

		if (cl->ctf_grapple == self)
		{
			cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			KillGrappleSoundKiller(self->owner);
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), 1, ATTN_NORM, 0);
			cl->ctf_grapple = NULL;
			cl->ctf_grapplereleasetime = level.time;
			cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		}

		G_FreeEdict(self);

		return;
	}

	self->nextthink = level.time + FRAMETIME;
}


void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
// AJ - make the CTF grapple look like an expert grapple for the bots
	if (hook_offhand->value && self->bot_client)
		grapple->s.effects = EF_GREENGIB;
// end AJ

	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
// AJ added support for non-CTF grapple objects, through a cvar
	grapple->s.modelindex = gi.modelindex (grapple_mdl->string);
// end AJ


//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
	grapple->nextthink = level.time + FRAMETIME;
	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

// AJ - quick hack to get offhand-like grapple
	if (!hook_offhand->value)
		gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, 2, 0);
	else gi.sound(ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("plats/pt1_mid.wav"), volume, ATTN_NORM, 0);
// AJ: changed CTF_GRAPPLE_SPEED to hook_speed->value
	CTFFireGrapple (ent, start, forward, damage, hook_speed->value, effect);

	SpawnGrappleSoundKiller(ent);

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);

//Eraser: record this position, so we drop a grapple node here, rather than where the player is when they leave the ground
	if (!ent->bot_client)
		VectorCopy(ent->s.origin, ent->animate_org);
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

// AJ constant 10 to hook_damage->value
	damage = hook_damage->value;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->ctf_grapple) {
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, 
		CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}

void CTFTeam_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;

	if (!ctf->value)
		return;

	if (ent->classname[0] == 'c')
	{	// in CAM mode
		gi.cprintf(ent, PRINT_HIGH, "You are in CAM mode. You must reconnect or restart the game to rejoin the action.\n");
		return;
	}

	t = gi.args();
	if (!*t) {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
				CTFTeamName(ent->client->resp.ctf_team));
		return;
	}
	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;
// AJ added green team
	else if (Q_stricmp(t, "green") == 0) 
	{
		if (ttctf->value)
			desired_team = CTF_TEAM3;
		else 
			{ 
			if (!ent->bot_client)
				gi.cprintf(ent, PRINT_HIGH, "Not a 3TCTF map.\n"); 
			return;
			}
	}
// end AJ
	else {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team) {
		if (!ent->bot_client)
			gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
				CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);
	
	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(desired_team));
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);

	ent->client->resp.score = 0;

	my_bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
}

/*
==================
CTFScoreboardMessage
==================
*/
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400],ping[6];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
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

	// team one
	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 20 ",
		totalscore[0], total[0],
		totalscore[1], total[1]);
	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

#if 0 //ndef NEW_SCORE
		// set up y
		sprintf(entry, "yv %d ", 42 + i * 8);
		if (maxsize - len > strlen(entry)) {
			strcat(string, entry);
			len = strlen(string);
		}
#else
		*entry = 0;
#endif

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

// AJ removed the flag logo from LMCTF
			if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				strcat(entry, "xv 56 picn sbfctf2 ");
#else
			if (cl_ent->bot_client)
			{
				if (showbotping->value)
				{
					cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
					if (cl->ping < 0)
						cl->ping = 0;
					sprintf(ping, "%3i", cl->ping);
				}
				else strcpy(ping, "bot");
			}
			else sprintf(ping, "%3i", (cl->ping > 999) ? 999 : cl->ping);
			
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %s ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				ping);

// AJ removed the flag logo from LMCTF
			if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",
					42 + i * 8);
#endif

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 160 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);

// AJ removed the flag logo from LMCTF
			if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				strcat(entry, "xv 216 picn sbfctf1 ");

#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

// AJ removed the flag logo from LMCTF
			if (!lmctf->value && cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);
#endif
			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}
	}

	// put in spectators if we have enough room
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
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.ctf_team != CTF_NOTEAM)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			if (cl_ent->bot_client)
			{
				if (showbotping->value)
				{
					cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
					if (cl->ping < 0)
						cl->ping = 0;
					sprintf(ping, "%3i", cl->ping);
				}
				else strcpy(ping, "bot");
			}
			else sprintf(ping, "%3i", (cl->ping > 999) ? 999 : cl->ping);

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %s ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				cl->resp.score,
				ping);
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

/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2) {
		if (!who->bot_client && !use_lithiumrunes->value)
// AJ added extra clause to prevent it in lithium games...
			gi.centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech(edict_t *ent)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			return tech;
		}
		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{

	if (	(other->client->pers.inventory[ITEM_INDEX(item_tech1)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech2)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech3)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech4)])
// AJ added tech5
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech5)]))
	{
		CTFHasTech(other);

		if (other->bot_client && (other->movetarget == ent))
		{	// stop going for this item
			other->movetarget = other->goalentity = NULL;
			ent->ignore_time = level.time + 3;
		}

		return false; // has this one
	}
/*
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			other->client->pers.inventory[ITEM_INDEX(tech)]) {
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}
*/	
	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	other->client->ctf_has_tech = true;

// AJ added for HUD display of current rune/tech
	if (other->client->pers.inventory[ITEM_INDEX(item_tech1)])
		other->client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH1;
	else if (other->client->pers.inventory[ITEM_INDEX(item_tech2)])
		other->client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH2;
	else if (other->client->pers.inventory[ITEM_INDEX(item_tech3)])
		other->client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH3;
	else if (other->client->pers.inventory[ITEM_INDEX(item_tech4)])
		other->client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH4;
	else if (other->client->pers.inventory[ITEM_INDEX(item_tech5)])
		other->client->ps.stats[STAT_LITHIUM_RUNE] = CS_TECH5;
// end AJ
	
	return true;
}

// AJ - needed to pick up runes we've dropped (but ignore while dropping)
void CTFTechTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if ((other == ent->owner) && (level.time < ent->touch_debounce_time))
		return;

	Touch_Item (ent, other, plane, surf);
}
// end aj


static void SpawnTech(gitem_t *item, edict_t *spot);

static edict_t *FindTechSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

// AJ - allow runes to be spawned away from DM starts
	if (rune_spawn_dmstart->value)
	{
		while (i--)
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (!spot)
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	}
	else 
	{
		i = (int) ((trail_head-1) * random()) + 1;
		spot = trail[i];
	}
// end AJ

	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL) {
		SpawnTech(tech->item, spot);

		RemoveFromItemList(tech);
		G_FreeEdict(tech);
	} else {
// AJ: changed CTF_TECH_TIMEOUT to rune_life->value
		tech->nextthink = level.time + rune_life->value;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

// AJ prevent the from dropping runes when not set
	if (!allow_runedrop->value)
		return;
// end AJ

	tech = Drop_Item(ent, item);
// AJ: changed CTF_TECH_TIMEOUT to rune_life->value
	tech->nextthink = level.time + rune_life->value;
	tech->think = TechThink;

// AJ if allowed to pick up runes twice, change touch function
	if (allow_pickuprune->value)
	{
		tech->touch = CTFTechTouch;
		tech->touch_debounce_time = level.time + 1.0;
	}
// end AJ

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;

	// AJ added for HUD display
	ent->client->ps.stats[STAT_LITHIUM_RUNE]=CS_TECHNONE;

	// AJ added the colour_SHELL flag, and colour shell settings
//	tech->s.renderfx = RF_GLOW;
	lithium_runecolour(item, tech);
	// end AJ
}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

// AJ: changed ctf-> to use_runes->
// AJ added extra clause to prevent dropping
	if (!use_runes->value || !allow_runedrop->value)
		return;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			dropped = Drop_Item(ent, tech);
			// hack the velocity to make it bounce random
			dropped->velocity[0] = (rand() % 600) - 300;
			dropped->velocity[1] = (rand() % 600) - 300;
// AJ: changed CTF_TECH_TIMEOUT to rune_life->value
			dropped->nextthink = level.time + rune_life->value;
			dropped->think = TechThink;
			dropped->owner = NULL;
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;

			// AJ added the colour_SHELL flag, and colour shell settings
			lithium_runecolour(tech, dropped);
//			dropped->s.renderfx = RF_GLOW;
			// end AJ
		}
		i++;
	}
}

//ScarFace- this function counts the number of runes in circulation
int TechCount (void)
{
	gitem_t	*tech;
	edict_t	*cl_ent;
	edict_t	*mapent = NULL;
	int i, j;
	int count = 0;

	mapent = g_edicts+1; // skip the worldspawn
	// cycle through all ents to find techs
	for (i = 1; i < globals.num_edicts; i++, mapent++)
	{
		if (!mapent->classname)
			continue;
		if ( (!strcmp(mapent->classname, "item_tech1")) || (!strcmp(mapent->classname, "item_tech2"))
			|| (!strcmp(mapent->classname, "item_tech3")) || (!strcmp(mapent->classname, "item_tech4"))
			|| (!strcmp(mapent->classname, "item_tech5")) )
			count++;
	}
	//cycle through all players to find techs
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
		{
			j = 0;
			while (tnames[j])
			{
				if ((tech = FindItemByClassname(tnames[j])) != NULL &&
					cl_ent->client->pers.inventory[ITEM_INDEX(tech)])
					count++;
				j++;
			}
		}
	}
	return count;
}

//ScarFace- a diagnostic function to display the number of runes in circulation
void Cmd_RuneCount_f (edict_t *ent)
{
	int count;
	count = TechCount();
	gi.dprintf ("Number of runes in game: %d\n", count);
}

//ScarFace- spawn the additional runes
void SpawnMoreTechs (int oldrunecount, int newrunecount, int numrunetypes)
{
	gitem_t	*tech;
	edict_t	*spot;
	int i, j;

	//gi.dprintf ("Number of runes in loop: %d\n", numrunetypes);
	i = oldrunecount % numrunetypes; //Spawn next rune in succession of the last one spawned
	j = oldrunecount; //Start with count at old rune 
	//gi.dprintf ("Rune number to start on: %d\n", (i+1));
	while ( (j < numrunetypes) || ((j < rune_max->value) && (j < newrunecount)) )
	{
		while ( (tnames[i]) &&
				((j < numrunetypes) || ((j < rune_max->value) && (j < newrunecount))) )
		{
			if (((tech = FindItemByClassname(tnames[i])) != NULL &&
				 (spot = FindTechSpawn()) != NULL)
				&& ((int)(rune_flags->value) & (0x1 << i)))
			{
				//gi.dprintf ("Spawning tech%d\n", (i+1));
				SpawnTech(tech, spot);
				j++;
			}
			i++;
		}
		i = 0;
	}
	//gi.dprintf ("Current number of runes in game: %d\n", j);
}

//ScarFace- remove some runes
void RemoveTechs (int oldrunecount, int newrunecount, int numrunetypes)
{
	edict_t	*mapent = NULL;
	int i, j, k;
	int removed;

	//gi.dprintf ("Number of runes in loop: %d\n", numrunetypes);
	i = (oldrunecount % numrunetypes - 1) ; //Remove last rune spawned
	if (i < 0)
		i = 4;
	j = oldrunecount;
	//gi.dprintf ("Rune number to start removing on: %d\n", (i+1));
	while ((tnames[i]) && (j > newrunecount) && (j > numrunetypes)) //leave at least 1 of each rune
	{
		if ((int)(rune_flags->value) & (0x1 << i)) //if this rune is allowed in the game
		{
			removed = 0; //flag to remove only one rune per pass
			mapent = g_edicts+1; // skip the worldspawn
			for (k = 1; k < globals.num_edicts; k++, mapent++)
			{
				if (!mapent->classname)
					continue;
				if (!strcmp(mapent->classname, tnames[i]))
				{
				//	gi.dprintf ("Removing tech%d\n", (i+1));
					G_FreeEdict(mapent);
					j--;
					removed = 1;
				}
				if (removed == 1) //don't keep removing runes of this type
					break;
			}
			//If we can't find this rune in map, wait until a player drops it instead of removing others
			if (removed == 0) 
				return;
		}
		i --;
	}
//	gi.dprintf ("Current number of runes in game: %d\n", j);
}

//ScarFace- this function checks to see if we need to spawn or remove runes
void CheckNumRunes (void)
{
	edict_t	*cl_ent;
	int i, j, numclients;
	int newrunecount, numrunes, runediff;
	int numrunetypes = 0;

	//count number of rune types enabled
	for (i = 0; i < 5; i++)
		if ((int)(rune_flags->value) & (0x1 << i))
			numrunetypes++;
	
	//count num. of clients
	numclients = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
			numclients++;
	}

	newrunecount = rune_perplayer->value * numclients;
	if (newrunecount > rune_max->value) //cap at rune_max
		newrunecount = rune_max->value;
	numrunes = TechCount();
	if (newrunecount > numrunes)
	{
		//gi.dprintf ("Number of runes to spawn: %d\n", newrunecount);
		SpawnMoreTechs (numrunes, newrunecount, numrunetypes);
	}
	if ((newrunecount < numrunes) && (newrunecount > numrunetypes))
	{
	//	gi.dprintf ("Number of runes to spawn: %d\n", newrunecount);
		RemoveTechs (numrunes, newrunecount, numrunetypes);
	}
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

// AJ: changed ctf-> to use_runes->
	if (!use_runes->value)
		return;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;

// AJ added the colour_SHELL flag, and colour shell settings
	lithium_runecolour(item, ent);
// end AJ

	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

// AJ: change CTF_TECH_TIMEOUT to rune_life->value
	ent->nextthink = level.time + rune_life->value;
	ent->think = TechThink;

	gi.linkentity (ent);

	bonus_head = AddToItemList(ent, bonus_head);

	// add to the tech lookup list
	{
		int i=0;

		while (titems[i])
			i++;

		if (i < 3)
		{
			titems[i] = item;
		}
	}
}

//This function is called before any clients enter the game,
//	so it can't determine how many runes to spawn based on player count
static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i;

	i = 0;
	while (tnames[i]) {
		if (((tech = FindItemByClassname(tnames[i])) != NULL &&
			 (spot = FindTechSpawn()) != NULL)
			&& ((int)(rune_flags->value) & (0x1 << i)))
			SpawnTech(tech, spot);
		i++;
	}
}

// frees the passed edict!
void CTFRespawnTech(edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);

	RemoveFromItemList(ent);
	G_FreeEdict(ent);
}

void CTFSetupTechSpawn(void)
{
	edict_t *ent;

// AJ: changed ctf-> to use_runes->
	if (!use_runes->value || techspawn || ((int)dmflags->value & DF_CTF_NO_TECH))
		return;

	ent = G_Spawn();
	ent->nextthink = level.time + 2;
	ent->think = SpawnTechs;
	techspawn = true;
}

int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = item_tech1;
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
//AJ added sound clause (bugged?)
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex(rune_resist_sound->string), volume, ATTN_NORM, 0);
// AJ: replaced constant 2.0 with rune_resist->value
		return dmg / rune_resist->value;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = item_tech2;
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
// AJ replaced constant 2.0 with rune_strength->value
		return dmg * rune_strength->value;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	tech = item_tech2;

	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
// AJ: screwed samples (was ctf/tech2x.wav)
				gi.sound(ent, CHAN_AUTO, gi.soundindex(rune_strength_sound->string), volume, ATTN_NORM, 0);
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
// AJ: screwed samples (was ctf/tech2.wav)
				gi.sound(ent, CHAN_AUTO, gi.soundindex(rune_strength_sound->string), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

// AJ: changed ctf-> to use_runes->
	if (!use_runes->value)
		return false;
	if (!tech)
		tech = item_tech3;
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

void CTFApplyHasteSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = item_tech3;
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) 
	{
		ent->client->ctf_techsndtime = level.time + 1;
// AJ - change constant filename to cvar
		gi.sound(ent, CHAN_VOICE, gi.soundindex(rune_haste_sound->string), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = item_tech4;


	// AJ: replaced constant 150 with rune_regen_*_max (multiple times)
	if (tech && client->pers.inventory[ITEM_INDEX(tech)]) 
		{
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < rune_regen_health_max->value) {
				ent->health += 5;
				if (ent->health > rune_regen_health_max->value)
					ent->health = rune_regen_health_max->value;
				client->ctf_regentime += 0.5;
				noise = true;
			}
// AJ - added conditionality of regenerating armor
			if (rune_regen_armor->value)
			{
				index = ArmorIndex (ent);
				if (index)
				{
					if (client->pers.inventory[index] < rune_regen_armor_max->value) 
					{
						client->pers.inventory[index] += 5;
						if (client->pers.inventory[index] > rune_regen_armor_max->value)
							client->pers.inventory[index] = rune_regen_armor_max->value;
						client->ctf_regentime += 0.5;
						noise = true;
					}
				}
				else if (rune_regen_armor_always->value)	// LMCTF functionality
				{
					index = combat_armor_index; // should hopefully always be valid 
					if (index && client->pers.inventory[index] < rune_regen_armor_max->value) 
					{
						client->pers.inventory[index] += 5;
						if (client->pers.inventory[index] > rune_regen_armor_max->value)
							client->pers.inventory[index] = rune_regen_armor_max->value;
						client->ctf_regentime += 0.5;
						noise = true;
					}
				}
			} // AJ close brace from rune_regen_armor->value
		}
// end AJ (regenmax)

	
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
// AJ: added sound cvar
			gi.sound(ent, CHAN_VOICE, gi.soundindex(rune_regen_sound->string), volume, ATTN_NORM, 0);
		}
	}
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = item_tech4;
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_flag_team3",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
		strcpy(buf, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
	else
		strcat(buf, "near ");

	if (nearteam == CTF_TEAM1)
		strcat(buf, "the red ");
	else if (nearteam == CTF_TEAM2)
		strcat(buf, "the blue ");
	else
		strcat(buf, "the ");

	strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		strcpy(buf, "no armor");
}

static void CTFSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_Tech(edict_t *who, char *buf)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			who->client->pers.inventory[ITEM_INDEX(tech)]) {
			sprintf(buf, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	strcpy(buf, "no powerup");
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}


static void CTFSay_Team_Enemy(edict_t *who, char *buf)
{
	if (who->enemy && who->enemy->client)
		strcpy(buf, who->enemy->client->pers.netname);
	else
		strcpy(buf, "none");
}


void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'e':
				case 'E':
					CTFSay_Team_Enemy(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				default :
					*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

// AJ added extra clauses to support both CTF and mormal teams, and also say-all for plain DM		
		if ((ctf->value && cl_ent->client->resp.ctf_team == who->client->resp.ctf_team) ||
			(teamplay->value && OnSameTeam(cl_ent, who)))
		{
			if (!cl_ent->bot_client)
				gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
					who->client->pers.netname, outmsg);
		}
		else if (!cl_ent->bot_client)
				gi.cprintf(cl_ent, PRINT_CHAT, "%s: %s\n", 
					who->client->pers.netname, outmsg);
// end AJ
	}
}



/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;
// AJ  3TCTF
	if ((ent->spawnflags & 11) == 2) // team3
		ent->s.skinnum = 2;
// end AJ

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
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
// AJ  3TCTF
	if ((ent->spawnflags & 11) == 2) // team3
		ent->s.skinnum = 2;
// end AJ

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

	if (!ctf->value)
		return;

	PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	PutClientInServer (ent);
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
}

void CTFJoinTeam1(edict_t *ent, pmenu_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenu_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM2);
}


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

void CTFReturnToMain(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

void CTFCredits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores = true;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, NULL, NULL }, 
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Christian Antkow",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Tim Willits",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Art",							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Adrian Carmack Paul Steed",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Kevin Cloud",					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Sound",							PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Tom 'Bjorn' Klok",				PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Original CTF Art Design",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Brian 'Whaleboy' Cozzens",		PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, NULL, CTFReturnToMain }
};


pmenu_t joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, CTFChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	joinmenu[4].text = "Join Red Team";
	joinmenu[4].SelectFunc = CTFJoinTeam1;
	joinmenu[6].text = "Join Blue Team";
	joinmenu[6].SelectFunc = CTFJoinTeam2;

	if (ctf_forcejoin->string && *ctf_forcejoin->string) 
	{
		if (stricmp(ctf_forcejoin->string, "red") == 0) 
		{
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		} else if (stricmp(ctf_forcejoin->string, "blue") == 0) {
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		joinmenu[8].text = "Leave Chase Camera";
	else
		joinmenu[8].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	
	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM1;
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == CTF_TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t));
}

void CTFCredits(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t));
}

qboolean CTFStartClient(edict_t *ent)
{
	if (!ctf->value)
		return false;

	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

// AJ added support for 3TCTF
		if (ttctf->value)
			TTCTFOpenJoinMenu(ent);
		else
			CTFOpenJoinMenu(ent);
// end AJ
		return true;
	}
	return false;
}

qboolean CTFCheckRules(void)
{
	if (capturelimit->value && 
		(ctfgame.team1 >= capturelimit->value ||
		ctfgame.team2 >= capturelimit->value)) {
		my_bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}

/*--------------------------------------------------------------------------
 * just here to help old map conversions
 *--------------------------------------------------------------------------*/

static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;
	vec3_t		forward;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

//ZOID
	CTFPlayerResetGrapple(other);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->enemy->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	other->s.angles[PITCH] = 0;
	other->s.angles[YAW] = dest->s.angles[YAW];
	other->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, other->client->ps.viewangles);
	VectorCopy (dest->s.angles, other->client->v_angle);

	// give a little forward velocity
	AngleVectors (other->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, other->velocity);

	// kill anything at the destination
	if (!KillBox (other))
	{
	}

	gi.linkentity (other);
}

/*QUAKED trigger_teleport (0.5 0.5 0.5) ?
Players touching this will be teleported
*/
void SP_trigger_teleport (edict_t *ent)
{
	edict_t *s;
	int i;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->touch = old_teleporter_touch;
	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);

	// noise maker and splash effect dude
	s = G_Spawn();
	ent->enemy = s;
	for (i = 0; i < 3; i++)
		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;
	s->s.sound = gi.soundindex ("world/hum1.wav");
	gi.linkentity(s);
	
}

/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)
Point trigger_teleports at these.
*/
void SP_info_teleport_destination (edict_t *ent)
{
	ent->s.origin[2] += 16;
}






void CTFJoinTeam3(edict_t *ent, pmenu_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM3);
}



pmenu_t ttjoinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Red Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Blue Team",		PMENU_ALIGN_LEFT, NULL, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Green Team",	PMENU_ALIGN_LEFT, NULL, CTFJoinTeam3 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, CTFChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, NULL, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int TTCTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	static char team3players[32];
	int num1, num2, num3, i;

	ttjoinmenu[4].text = "Join Red Team";
	ttjoinmenu[4].SelectFunc = CTFJoinTeam1;
	ttjoinmenu[6].text = "Join Blue Team";
	ttjoinmenu[6].SelectFunc = CTFJoinTeam2;
	ttjoinmenu[8].text = "Join Green Team";
	ttjoinmenu[8].SelectFunc = CTFJoinTeam3;

	if (ctf_forcejoin->string && *ctf_forcejoin->string) 
	{
		if (stricmp(ctf_forcejoin->string, "red") == 0) 
		{
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
			joinmenu[8].text = NULL;
			joinmenu[8].SelectFunc = NULL;
		} else if (stricmp(ctf_forcejoin->string, "blue") == 0) 
		{
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
			joinmenu[8].text = NULL;
			joinmenu[8].SelectFunc = NULL;
		} else if (stricmp(ctf_forcejoin->string, "green") == 0) 
		{
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		joinmenu[10].text = "Leave Chase Camera";
	else
		joinmenu[10].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = num3 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM3)
			num3++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);
	sprintf(team3players, "  (%d players)", num3);

	joinmenu[2].text = levelname;
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	if (joinmenu[8].text)
		joinmenu[9].text = team2players;
	else
		joinmenu[9].text = NULL;
	
	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM2;
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void TTCTFOpenJoinMenu(edict_t *ent)
{
	int team;

	team = TTCTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 10;
	else if (team == CTF_TEAM1)
		team = 4;
	else if (team == CTF_TEAM2)
		team = 6;
	else team = 8;
	PMenu_Open(ent, ttjoinmenu, team, sizeof(ttjoinmenu) / sizeof(pmenu_t));
}
