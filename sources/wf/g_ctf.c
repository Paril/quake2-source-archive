/* g_ctf.c */

#include "g_local.h"
#include "wf_classmgr.h"
#include "stdlog.h"	//	StdLog - Mark Davies
//ERASER START
#include "p_trail.h"
#include "bot_procs.h"

void cmd_RemoveLaserDefense(edict_t *ent);
void readline(FILE *file, char *str, int max);
int PlayerChangeScore(edict_t *self, int points);
void WFClassCount(edict_t *self, int *classcount);
void WFEndDMLevel (char *mapname);

char classmenustring[MAX_CLASSES + 1][64];
char disguisemenustring[MAX_CLASSES + 1][64];
char menustring[24][64];
char menutitle[32];
int BannedWords(edict_t *ent, char *str);
void HealPlayer(edict_t *ent);
void Cmd_Cloak_f (edict_t *ent);

#define MAX_MENU_MAPS	25

extern int modID;

edict_t	*AddToItemList(edict_t *ent, edict_t	*head);
void	RemoveFromItemList(edict_t *ent);

//ERASER END

//WF - function prototypes
void cmd_PlasmaBombMenu(edict_t *ent);
void cmd_Alarm(edict_t *ent);
void Cmd_Feign_f (edict_t *ent);

ctfgame_t ctfgame;
qboolean techspawn = false;

//ERASER START //ACRID
edict_t *flag1_ent=NULL;
edict_t *flag2_ent=NULL;

edict_t *flagreturn1_ent=NULL;//$
edict_t *flagreturn2_ent=NULL;//$
//ERASER END

cvar_t *ctf;
cvar_t *ctf_forcejoin;

gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4;//ERASER

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
  " xv	100 "
  " anum "
  " xv	150 "
  " pic 2 "
"endif "

// armor
"if 4 "
  " xv	200 "
  " rnum "
  " xv	250 "
  " pic 4 "
"endif "

// selected item
//"if 6 "
//"	xv	296 "
//"	pic 6 "
//"endif "

"yb	-50 "

// picked up item
"if 7 "
  "xv	0 "
  "pic 7 "
  "xv	26 "
  "yb	-42 "
  "stat_string 8 "
  "yb	-50 "
"endif "

// timer
"if 9 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//  help / weapon icon
//"if 11 "
//  "xv 148 "
//  "pic 11 "
//"endif "

//  timeout icon
"if 30 "
  "xv 148 "
  "yt 100 "
  "pic 30 "
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

// weapon damage (was 75,150)
"if 28 "
  "yt 52 "
  "xr	-50 "
  "num 3 28 "
  "yt 78 "
  "xr	-24 "
  "pic 29 "
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

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

gitem_t *titems[4] = {NULL, NULL, NULL, NULL};//ERASER

void stuffcmd(edict_t *ent, char *s)
{
   	gi.WriteByte (11);
	gi.WriteString (s);
    gi.unicast (ent, true);
}

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

//static gitem_t *flag1_item;
//static gitem_t *flag2_item;
gitem_t *flag1_item;
gitem_t *flag2_item;

gitem_t *GetEnemyFlag(int teamno)
{
	if (teamno <= 0 || teamno > 2) return NULL;
	else if (teamno == 1) return flag2_item;
	else return flag1_item;
}

void botGetFlag(edict_t *ent);//ERASER

void CTFInit(void)
{
	ctf = gi.cvar("ctf", "1", CVAR_SERVERINFO | CVAR_LATCH);//ERASER ADDED | CVAR_LATCH//WF USES 1
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);

	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");
	memset(&ctfgame, 0, sizeof(ctfgame));
	techspawn = false;

//ERASER START, precache Tech powerups
	item_tech1 = FindItemByClassname("item_tech1");
	item_tech2 = FindItemByClassname("item_tech2");
	item_tech3 = FindItemByClassname("item_tech3");
	item_tech4 = FindItemByClassname("item_tech4");
//ERASER END
}

/*--------------------------------------------------------------------------*/

char *CTFTeamName(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return "RED";
	case CTF_TEAM2:
		return "BLUE";
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

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char modelname[64];
	char skinname[64];
	int	team_no = 0;


	//If not running CTF, always pick team1
	if (!ctf->value)
//		return;
		team_no = CTF_TEAM1;

	// Set model name
	if (ent->disguised)
		sprintf(modelname, "%s/",classinfo[ent->disguised].model_name);
	else
		sprintf(modelname, "%s/",classinfo[ent->client->pers.player_class].model_name);


	// Set skin name and check for disguise
	if (!team_no) team_no = ent->client->resp.ctf_team;

//gi.dprintf("Teamno = %d\n", team_no);
	switch (team_no)
	{
	case CTF_TEAM1:
		if (ent->disguised)
			strcpy(skinname, classinfo[ent->disguised].skin2);
		else
			strcpy(skinname, classinfo[ent->client->pers.player_class].skin1);
		break;
	case CTF_TEAM2:
		if (ent->disguised)
			strcpy(skinname, classinfo[ent->disguised].skin1);
		else
			strcpy(skinname, classinfo[ent->client->pers.player_class].skin2);
		break;
	default:
		//no skin?
		;
	}

	//Special case for the ref if he is using the ref skin
	if ((wf_game.show_ref_skin == 1) && (wf_game.ref_ent == ent))
	{
		sprintf(modelname, "%s/",classinfo[ent->disguised].model_name);
		switch (team_no)
		{
		case CTF_TEAM1:
			strcpy(skinname, "xref_r");
			break;
		default:
			strcpy(skinname, "xref_b");
		}
	}

	if (ent->client->player_special & SPECIAL_MERCENARY)
	{
		safe_cprintf(ent, PRINT_HIGH, "Merc - You are on the %s team\n",CTFTeamName(ent->client->resp.ctf_team));
	}

	//Now do the model/skin assignment
	switch (team_no) {
	case CTF_TEAM1:
	case CTF_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, modelname, skinname) );
//		gi.dprintf("<SKIN>: %s\\%s%s\n", ent->client->pers.netname, modelname, skinname);
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s", ent->client->pers.netname, s) );
		botDebugPrint("SKIN 9 (ACRID)\n");
		break;
	}

}
extern int	force_team;//ERASER

int FindPlayerID(gclient_t *client)
{
	edict_t *e;
	int i;

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->client)
			continue;
		if (e ->client == client)
			return i;
	}

	return (0);

}

// ERASER, added "is_bot" flag to force assigning bot's to a team
void CTFAssignTeam(gclient_t *who, qboolean is_bot)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	if (!ctf->value)
	{
		// If not running CTF, then use the player's id number as team number
		who->resp.ctf_team = FindPlayerID(who);
		who->resp.next_ctf_team = who->resp.ctf_team;
		return;
	}

	botDebugPrint("CTF ASSIGN TEAM(ACRID)\n");
	who->resp.ctf_state = CTF_STATE_START;
//ERASER ADDED IS BOT
	if (!is_bot && !((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}
//ERASER START
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
//ERASER END
	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team) {
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
		}
	}
	if (team1count < team2count)
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = CTF_TEAM2;
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

//WF - Only spawn on your own team's spawn points unless that is turned off

  if ((int)wfflags->value & WF_NO_FORT_RESPAWN)
  {
	if (ent->client->resp.ctf_state != CTF_STATE_START)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();
  }
//WF

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

//	if (ent->client->resp.ctf_team == 0)
//		gi.dprintf("DEBUG: Client has no team selected!\n");

	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
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
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *flag;
	edict_t *carrier = NULL;
	char *c;
	vec3_t v1, v2;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		if (PlayerChangeScore(attacker,CTF_FRAG_CARRIER_BONUS))
		{
			safe_cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
					CTF_FRAG_CARRIER_BONUS);


			// Log Flag Carrier Frag - MarkDavies
			sl_LogScore( &gi,
                     attacker->client->pers.netname,
                     NULL,
                     "FC Frag",
                     -1,
                     CTF_FRAG_CARRIER_BONUS);
		}

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
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) {
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		if (PlayerChangeScore(attacker,CTF_CARRIER_DANGER_PROTECT_BONUS))
		{
			my_bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
				attacker->client->pers.netname,
				CTFTeamName(attacker->client->resp.ctf_team));

			// Log Flag Danger Carrier Protect Frag - MarkDavies
			sl_LogScore( &gi,
                     attacker->client->pers.netname,
                     NULL,
                     "FC Def",
                     -1,
                     CTF_CARRIER_DANGER_PROTECT_BONUS);
		}
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
		if (PlayerChangeScore(attacker,CTF_FLAG_DEFENSE_BONUS))
		{
			if (flag->solid == SOLID_NOT)
				my_bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
					attacker->client->pers.netname,
					CTFTeamName(attacker->client->resp.ctf_team));
			else
				my_bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
					attacker->client->pers.netname,
					CTFTeamName(attacker->client->resp.ctf_team));

			// Log Flag Defense - MarkDavies
		    sl_LogScore( &gi,
                     attacker->client->pers.netname,
                     NULL,
                     "F Def",
                     -1,
                     CTF_FLAG_DEFENSE_BONUS);
		}
		return;
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {
			if (PlayerChangeScore(attacker,CTF_CARRIER_PROTECT_BONUS))
			{
				my_bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
					attacker->client->pers.netname,
					CTFTeamName(attacker->client->resp.ctf_team));

				// Log Flag Defense - MarkDavies
				sl_LogScore( &gi,
                     attacker->client->pers.netname,
                     NULL,
                     "C Prot",
                     -1,
                     CTF_CARRIER_PROTECT_BONUS);
			}
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
		flag1dropped = 0;		//clear the dropped flag flag
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		flag2dropped = 0;
		break;
	default:
		return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
		{
			RemoveFromItemList(ent);//ERASER
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
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	int i;
	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item;
	int count;//ERASER
	qboolean ret;

	if (other->client->pers.feign)//5/99
		return false;

	if ((strcmp(ent->classname, "item_flagreturn_team1") == 0)||
       (strcmp(ent->classname, "item_flagreturn_team2") == 0))
	   {botDebugPrint("pickup class %s\n",ent->classname);
		return false;
	}
	if (level.time < 30.0)
	{
		safe_cprintf(other, PRINT_HIGH, "You must wait another %d seconds to pick up flag\n", (int)(31 - level.time));
		return false;
	}

	if (wf_game.game_halted)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't pick up flag while game is suspended.\n");
		return false;
	}

	//	WF - check for unbalanced teams.  Don't allow pickup if they are
	if (WFUnbalanced_Teams(ent, other)) return false;

//WF24 SIf this is a STWF flag pickup call a different routine
	//otherwise, use the same routine
	if (((int)wfflags->value & WF_ZOID_FLAGCAP) == 0)
	{
		ret = WFPickup_Flag(ent, other);
		return(ret);
	}

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	else {
		safe_cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

//ERASER START
	if (other->bot_client && (other->movetarget == ent))
		other->movetarget = NULL;
//ERASER END NOT SURE WHAT THIS DOES

	if (ctf_team == other->client->resp.ctf_team) {

		if (!(ent->spawnflags & DROPPED_ITEM)) {
			// the flag is at home base.  if the player has the enemy
			// flag, he's just won!

			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
			{
				my_bprintf(PRINT_HIGH, "%s captured the %s flag!\n",
						other->client->pers.netname, CTFOtherTeamName(ctf_team));
				other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;
				ctfgame.last_flag_capture = level.time;
				ctfgame.last_capture_team = ctf_team;
				if (ctf_team == CTF_TEAM1)
					ctfgame.team1++;
				else
					ctfgame.team2++;

				gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);

				// other gets another 10 frag bonus
				if (PlayerChangeScore(other,CTF_CAPTURE_BONUS))
				{
					// Log Flag Capture - MarkDavies
		            sl_LogScore( &gi,
			                     other->client->pers.netname,
				                 NULL,
					             "F Capture",
						         -1,
							     CTF_CAPTURE_BONUS);
				}

				// Ok, let's do the player loop, hand out the bonuses
				for (i = 1; i <= maxclients->value; i++) {
					player = &g_edicts[i];
					if (!player->inuse)
						continue;

//ERASER START, if following the player, stop
					if (player->bot_client && (player->target_ent == other))
						player->target_ent = NULL;
//ERASER END

					if (player->client->resp.ctf_team != other->client->resp.ctf_team)
						player->client->resp.ctf_lasthurtcarrier = -5;
					else if (player->client->resp.ctf_team == other->client->resp.ctf_team) {
						if (player != other)
						{//WF34
							if (PlayerChangeScore(player,CTF_TEAM_BONUS))
							{
						    // Log Flag Capture Team Score - MarkDavies
                            sl_LogScore( &gi,
                                         player->client->pers.netname,
                                         NULL,
                                         "Team Score",
                                         -1,
                                         CTF_TEAM_BONUS);
							}
						}//WF34 END
						// award extra points for capture assists
						if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
						{
							if (PlayerChangeScore(player,CTF_RETURN_FLAG_ASSIST_BONUS))
							{
								my_bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);

		                        // Log Flag Capture Team Score - MarkDavies
			                    sl_LogScore( &gi,
                                         player->client->pers.netname,
                                         NULL,
                                         "F Return Assist",
                                         -1,
                                         CTF_RETURN_FLAG_ASSIST_BONUS);
							}
						}
						if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
						{
							if (PlayerChangeScore(player,CTF_FRAG_CARRIER_ASSIST_BONUS))
							{
								my_bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);

		                       // Log Flag Capture Team Score - MarkDavies
			                    sl_LogScore( &gi,
                                         player->client->pers.netname,
                                         NULL,
                                         "FC Frag Assist",
                                         -1,
                                         CTF_FRAG_CARRIER_ASSIST_BONUS);
							}
						}
					}
				}

				CTFResetFlags();
				return false;
			}
			return false; // its at home base already
		}
		// hey, its not home.  return it by teleporting it back
//WF USES GI.BPRINT
		if (PlayerChangeScore(other,CTF_RECOVERY_BONUS))
		{
			my_bprintf(PRINT_HIGH, "%s returned the %s flag!\n",
				other->client->pers.netname, CTFTeamName(ctf_team));

			// Log Flag Recover - MarkDavies
			sl_LogScore( &gi,
                     other->client->pers.netname,
                     NULL,
                     "F Return",
                     -1,
                     CTF_RECOVERY_BONUS);
		}

		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);

		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);
		return false;
	}

	// hey, its not our flag, pick it up
	if (PlayerChangeScore(other,CTF_FLAG_BONUS))
	{
		my_bprintf(PRINT_HIGH, "%s got the %s flag!\n",
			other->client->pers.netname, CTFTeamName(ctf_team));
		// Log Flag Pickup - MarkDavies
		sl_LogScore( &gi,
                 other->client->pers.netname,
                 NULL,
                 "F Pickup",
                 -1,
                 CTF_FLAG_BONUS);
	}

	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	//Remove player disguise
	if (other->disguised) WFRemoveDisguise(other);

//ERASER START: send this bot back to it's base!
	if (other->bot_client)
	{
if (CarryingFlag(other) && (((int)wfflags->value & WF_ZOID_FLAGCAP) == 0)
	&& (flagreturn1_ent != NULL || flagreturn2_ent != NULL)	)//$
{//$
		if (other->client->resp.ctf_team == CTF_TEAM1)//$
			other->movetarget = flagreturn1_ent;//$
		else//$
			other->movetarget = flagreturn2_ent;//$

		other->movetarget_want = WANT_SHITYEAH;//$
}//$
else//$
{//$
			if (other->client->resp.ctf_team == CTF_TEAM1)
			other->movetarget = flag1_ent;
		else
			other->movetarget = flag2_ent;

		other->movetarget_want = WANT_SHITYEAH;
}//$
	}
//ERASER END

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}


//ERASER START: inform bots of our location and request assistance
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
//ERASER END

	return true;
}
//WF24 USES STATIC
void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner &&
		ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void CTFDropFlagThink(edict_t *ent)
{

//	if (ent->timestamp < (level.time - CTF_AUTO_FLAG_RETURN_TIMEOUT))//ERASER
//	{//E //ACRID Fixes flagdrop/returntimeout bug
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
		}
//	}
//ERASER START
	else	// inform bots!
	{
		int i;

		for (i=0; i<num_players; i++)
		{
			if ((entdist(ent, players[i]) > 1500) && !gi.inPVS(ent->s.origin, players[i]->s.origin))
				continue;

			players[i]->movetarget = ent;
		}

		ent->nextthink = level.time + 0.2;
	}
}
//ERASER END
//WF34 START
int CTFHasFlag(edict_t *self)
{
	int hasflag;

	if (!flag1_item)
	{
		gi.dprintf("No flag1 item!\n");
		return 0;
	}
	if (!flag2_item)
	{
		gi.dprintf("No flag2 item!\n");
		return 0;
	}
	hasflag = 0;
	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
		hasflag = 1;
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
		hasflag = 1;

	return (hasflag);
}
//WF34 END
// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;

	if (!ctf->value)//ERASER
		return;//E

	if (!flag1_item || !flag2_item)
		CTFInit();

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
	{
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
        flag1dropped = 1;//WF34
	}
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
	{
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
		flag2dropped = 1;//WF34
	}

	if (dropped) {
		dropped->think = CTFDropFlagThink;//ACRID fixes flagdrop/returntimeout
//		dropped->timestamp = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		//dropped->nextthink = level.time + 0.2;//ERASER
	    dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
        dropped->touch = CTFDropFlagTouch;

		CalcItemPaths(dropped);//ERASER
	}






}

qboolean CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
	if (rand() & 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");
	}
	else
	{
		safe_cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
	}
	return false;
}
//ERASER START
float	team1_rushbase_time, team2_rushbase_time;	// used by RUSHBASE command
float	team1_defendbase_time, team2_defendbase_time;
//ERASER END
//WF USES STATIC
void CTFFlagThink(edict_t *ent)
{
//ERASER START
#define CTF_DEBUG	false

	edict_t *enemy_flag;
	float dist;
	int		this_team_count, enemy_team_count;
	int		get_defenders=1, get_attackers=1;//problem fixme acrid

	if (ent->solid != SOLID_NOT)//WF24 USES THIS TOO
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);//WF24 USES THIS TOO

	if (team1_rushbase_time > 0)
		team1_rushbase_time -= FRAMETIME;
	if (team2_rushbase_time > 0)
		team2_rushbase_time -= FRAMETIME;
	if (team1_defendbase_time > 0)
		team1_defendbase_time -= FRAMETIME;
	if (team2_defendbase_time > 0)
		team2_defendbase_time -= FRAMETIME;

//ERASER: make sure the flag is defended
if (!bot_melee->value)
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
			{//botDebugPrint("rushbase 1\n");
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
			{//botDebugPrint("rushbase 2\n");
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
			{//botDebugPrint("rushbase 3\n");
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
			{//botDebugPrint("rushbase 4\n");
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
			{//botDebugPrint("defending check22222\n");
				this_team_count++;

				dist = entdist(plyr, ent);

				if (	(	(plyr->target_ent == ent)
						 ||	(!plyr->bot_client)))
				{
					if (dist < (BOT_GUARDING_RANGE*1.5))
					{
//botDebugPrint("defending check4444444\n");
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
			ideal_guarding = (int)ceil((1.0*(float)this_team_count)/8.0);//was4
		else
			ideal_guarding = (int)ceil((1.0 * (float)this_team_count)/7.0);//was2

		if (get_defenders)
		{//botDebugPrint("get defenders111111111\n");
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
				//botDebugPrint("defending check3333333\n");
				count++;
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
							//if ((!players[i]->bot_client) && (bot_chat->value) && (players[i]->client) && (players[i]->client->pers.nospam_level == 0))
							//	safe_cprintf(players[i], PRINT_CHAT, "%s: base under attack!\n", closest_guarding->client->pers.netname);
							//else
							if (get_defenders && (players[i]->target_ent != ent) && !CarryingFlag(players[i]) && (entdist(players[i], ent) < 1500))
							{
							if (CTF_DEBUG) botDebugPrint( "Sending %s to defend flag\n", players[i]->client->pers.netname);
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
					//if ((bot_chat->value) && (!players[i]->bot_client) && (players[i]->client->resp.ctf_team == ctf_team) && (players[i]->client) && (players[i]->client->pers.nospam_level == 0))
					//{	// FIXME: add to custom chat.txt
					//	safe_cprintf(players[i], PRINT_CHAT, "%s: base secured!\n", closest_guarding->client->pers.netname);
					//}
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

					if (dist > 1200)
					{
						if (players[i]->target_ent && (dist > 2000))
							continue;
			//f			if ((dist > 1500) && (players[i]->bot_fire == botBlaster))
			//f				continue;
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
//ERASER END
	ent->nextthink = level.time + FRAMETIME;//WF USES THIS TOO
}

void CTFFlagSetup (edict_t *ent)//ACRID LOOK HERE
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;
//	edict_t *flag_thinker;//ERASER AND IT WAS ALREADY CO
	edict_t	*spot;//$
	spot = NULL;//$


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

//ERASER START
	if (strcmp(ent->classname, "item_flag_team1") == 0)
	{
	//home base (flag return) position
	spot = G_Find (spot, FOFS(classname), "item_flagreturn_team1");//$
	if (!spot)//$
		spot = NULL;//$
		flag1_ent = ent;
		ent->count = CTF_TEAM1;
		flagreturn1_ent = spot;//$
	}
	else
	{
	//home base (flag return) position
	spot = G_Find (spot, FOFS(classname), "item_flagreturn_team2");//$
	if (!spot)//$
		spot = NULL;//$
		flag2_ent = ent;
		ent->count = CTF_TEAM2;
		flagreturn2_ent = spot;//$
	}
if (flagreturn1_ent == NULL)
botDebugPrint("No map placed returns red\n");
if (flagreturn2_ent == NULL)
botDebugPrint("No map placed returns blue\n");
	CalcItemPaths(ent);
//ERASER END

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;

}

void CTFEffects(edict_t *player)
{
	player->s.effects &= (EF_FLAG1 | EF_FLAG2);
	if (player->health > 0) {
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
			player->s.effects |= EF_FLAG1;
		}
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
			player->s.effects |= EF_FLAG2;
		}
	}

	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
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
	}
}

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		safe_cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		safe_cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

static void CTFSetIDView(edict_t *ent)
{
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;
	trace_t tr;
	int old_armor_index;
	// TeT use a local for id
	edict_t* id;

	//Spawn Inviso Bullet

	PMenu_Close(ent);

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(end,100 , 0, 0);
	G_ProjectSource (ent->s.origin, end, forward, right, start);
	id = G_Spawn ();
	id->owner = ent;
	id->movetype = MOVETYPE_NOCLIP;
	id->solid = SOLID_NOT;
	id->classname = "id";

	//Trace a line to the target
	AngleVectors (id->owner->client->v_angle, forward, right, up);
	VectorSet(offset,24 , 6, id->owner->viewheight-7);
	G_ProjectSource (id->owner->s.origin, offset, forward, right, start);
	VectorMA(start,8192,forward,end);
	tr = gi.trace (start,NULL,NULL, end,id->owner,
		CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	if (tr.fraction != 1)
	{
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);
	}

	while (ent->client->resp.id_state == true)
	{
		//Client ID
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))  //detected client
		{
			if (tr.ent->deadflag == DEAD_DEAD)	break;
				// Enemies
				if ((tr.ent->takedamage) && (tr.ent != id->owner) && (tr.ent->wf_team != ent->wf_team)) //detected enemy
				{
					//Decoy?
					if (!tr.ent->client)
					{
						sprintf(ent->client->wfidstr[15], "[Enemy Decoy]\n");
						sprintf(ent->client->wfidstr[16], "");
						ent->client->resp.id_state = false;
						break;
					}
					else if ((strcmp(classinfo[tr.ent->client->pers.player_class].name, "Spy") == 0))
						//detected enemy spy
					{
						if (tr.ent->disguised)
						{
							if ((strcmp(classinfo[ent->client->pers.player_class].name, "Spy") == 0))
								//spy vs spy (no more secrets)
							{
								sprintf(ent->client->wfidstr[15], "[DETECTED! %s - Enemy %s]\n", tr.ent->client->pers.netname, classinfo[tr.ent->client->pers.player_class].name);
								sprintf(ent->client->wfidstr[16], "");
								ent->client->resp.id_state = false;
								break;
							}
						break; //he's disguised, don't show
						}
					}
					sprintf(ent->client->wfidstr[15], "[%s - Enemy %s]\n", tr.ent->client->pers.netname, classinfo[tr.ent->client->pers.player_class].name);
					sprintf(ent->client->wfidstr[16], "");
					ent->client->resp.id_state = false;
					break;
				}

				// Friends
				if ((tr.ent->takedamage) && (tr.ent != id->owner) && (tr.ent->wf_team == ent->wf_team)) //detected friend
				{
					//Decoy?
					if (!tr.ent->client)
					{
						sprintf(ent->client->wfidstr[15], "[Friendly Decoy]\n");
						sprintf(ent->client->wfidstr[16], "");
						ent->client->resp.id_state = false;
						break;
					}
					if ((strcmp(classinfo[tr.ent->client->pers.player_class].name, "Spy") == 0))
					{
						if (tr.ent->disguised)
						{
							if ((strcmp(classinfo[ent->client->pers.player_class].name, "Spy") == 0))
							{
								sprintf(ent->client->wfidstr[15], "[DETECTED! %s - Friendly %s]\n", tr.ent->client->pers.netname,	classinfo[tr.ent->client->pers.player_class].name);
								sprintf(ent->client->wfidstr[16], "");
								ent->client->resp.id_state = false;
								break;  //friendly spy seeing friendly spy.. show ID
							}
							break; //friendly spy disguised so don't show his id
						}
					}
					if ((strcmp(classinfo[ent->client->pers.player_class].name, "Nurse") == 0))
					{
						sprintf(ent->client->wfidstr[15], "[%s - Friendly %s]\n", tr.ent->client->pers.netname,	classinfo[tr.ent->client->pers.player_class].name);
						sprintf(ent->client->wfidstr[16], "[Health - %d]\n", tr.ent->health);
						ent->client->resp.id_state = false;
						break;
					}
					if ((strcmp(classinfo[ent->client->pers.player_class].name, "Engineer") == 0))
					{
						old_armor_index = ArmorIndex (tr.ent);
						sprintf(ent->client->wfidstr[15], "[%s - Friendly %s]\n", tr.ent->client->pers.netname,	classinfo[tr.ent->client->pers.player_class].name);
						sprintf(ent->client->wfidstr[16], "[Armor %d/%d]", tr.ent->client->pers.inventory[old_armor_index], tr.ent->client->player_armor);
						ent->client->resp.id_state = false;
						break;
					}
				sprintf(ent->client->wfidstr[15], "[%s - Friendly %s]\n", tr.ent->client->pers.netname,	classinfo[tr.ent->client->pers.player_class].name);
				sprintf(ent->client->wfidstr[16], "");
				ent->client->resp.id_state = false;
				break;
				}
		}

		//Depot ID
		if ((strcmp(tr.ent->classname, "depot") == 0)) //detected depot
		{
			if (tr.ent->wf_team != ent->wf_team)
			{
				sprintf(ent->client->wfidstr[15], "[%s's Enemy Ammo Depot]\n", tr.ent->owner->client->pers.netname);
				sprintf(ent->client->wfidstr[16], "");
				ent->client->resp.id_state = false;
				break;
			}
			if ( tr.ent->wf_team == ent->wf_team)
			{
				if (tr.ent->owner == ent)
				{
					sprintf(ent->client->wfidstr[15], "[Your Ammo Depot]\n");
					sprintf(ent->client->wfidstr[16], "");
					ent->client->resp.id_state = false;
					break;
				}
				sprintf(ent->client->wfidstr[15], "[%s's Friendly Ammo Depot]\n", tr.ent->owner->client->pers.netname);
				sprintf(ent->client->wfidstr[16], "");
				ent->client->resp.id_state = false;
				break;
			}
		}

		//Sentry ID
		if ((strcmp(tr.ent->classname, "SentryGun") == 0)) //detected sentry
		{
			if (tr.ent->wf_team != ent->wf_team)
			{
				sprintf(ent->client->wfidstr[15], "[%s's Enemy Sentry]\n", tr.ent->standowner->client->pers.netname);
				sprintf(ent->client->wfidstr[16], "");
				if ((strcmp(classinfo[ent->client->pers.player_class].name, "Spy") == 0)) //spy looking at enemy sentry
				{
					sprintf(ent->client->wfidstr[16], "[%d Bullets]\n", tr.ent->light_level); //spy seeing sentry ammo
					ent->client->resp.id_state = false;
					break;
				}
			}
			if (tr.ent->wf_team == ent->wf_team)
			{
				if(tr.ent->standowner == ent)
					sprintf(ent->client->wfidstr[15], "[Your Sentry Level:%d]\n", tr.ent->count);
				else
					sprintf(ent->client->wfidstr[15], "[%s's Friendly Sentry  Level:%d]\n", tr.ent->standowner->client->pers.netname, tr.ent->count);

				if ((strcmp(classinfo[ent->client->pers.player_class].name, "Engineer") == 0))
					sprintf(ent->client->wfidstr[16], "[Armor:%d/%d   Ammo:%d/%d]", tr.ent->health, tr.ent->max_health, tr.ent->light_level, tr.ent->gib_health);
				else
					sprintf(ent->client->wfidstr[16], "");

				ent->client->resp.id_state = false;
				break;
			}
		}
		break;
	}

	//set up the ID display
	if (ent->client->resp.id_state == false)
	{
		sprintf(ent->client->wfidstr[0], "");
		ent->client->iddisplay[0].text = ent->client->wfidstr[0];
		sprintf(ent->client->wfidstr[1], "");
		ent->client->iddisplay[1].text = ent->client->wfidstr[1];
		sprintf(ent->client->wfidstr[2], "");
		ent->client->iddisplay[2].text = ent->client->wfidstr[2];
		sprintf(ent->client->wfidstr[3], "");
		ent->client->iddisplay[3].text = ent->client->wfidstr[3];
		sprintf(ent->client->wfidstr[4], "");
		ent->client->iddisplay[4].text = ent->client->wfidstr[4];
		sprintf(ent->client->wfidstr[5], "");
		ent->client->iddisplay[5].text = ent->client->wfidstr[5];
		sprintf(ent->client->wfidstr[6], "");
		ent->client->iddisplay[6].text = ent->client->wfidstr[6];
		sprintf(ent->client->wfidstr[7], "");
		ent->client->iddisplay[7].text = ent->client->wfidstr[7];
		sprintf(ent->client->wfidstr[8], "");
		ent->client->iddisplay[8].text = ent->client->wfidstr[8];
		sprintf(ent->client->wfidstr[9], "");
		ent->client->iddisplay[9].text = ent->client->wfidstr[9];
		sprintf(ent->client->wfidstr[10], "");
		ent->client->iddisplay[10].text = ent->client->wfidstr[10];
		sprintf(ent->client->wfidstr[11], "");
		ent->client->iddisplay[11].text = ent->client->wfidstr[11];
		sprintf(ent->client->wfidstr[12], "");
		ent->client->iddisplay[12].text = ent->client->wfidstr[12];
		sprintf(ent->client->wfidstr[13], "");
		ent->client->iddisplay[13].text = ent->client->wfidstr[13];
		sprintf(ent->client->wfidstr[14], "");
		ent->client->iddisplay[14].text = ent->client->wfidstr[14];	//spacers, UGLY but works

		//id specific sprint defines 15 & 16, 16 is special for health or armor
		ent->client->iddisplay[15].text = ent->client->wfidstr[15];
		ent->client->iddisplay[15].align = PMENU_ALIGN_CENTER;
		ent->client->iddisplay[16].text = ent->client->wfidstr[16];
		ent->client->iddisplay[16].align = PMENU_ALIGN_CENTER;
		PMenu_Open(ent, ent->client->iddisplay, -1, sizeof(ent->client->iddisplay) / sizeof(pmenu_t), true, false);
	}

	//clear the menus and reset the resp id, kill the "bullet" entity
	if (ent->client->menu) ent->client->menu->MenuTimeout = level.time + 2;
	ent->client->resp.id_state = false;
	vectoangles(tr.plane.normal,id->s.angles);
	VectorCopy(tr.endpos,id->s.origin);
	gi.linkentity (id);
	G_FreeEdict (id);
	id = NULL;

}

void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2;
	edict_t *e;

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8))
	{ // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (ctfgame.team1 > ctfgame.team2)
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.team2 > ctfgame.team1)
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.total2 > ctfgame.total1)
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else
		{ // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}

	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;

    // ++TeT
	// only look for tech if they are allowed
	if (!((int)dmflags->value & DF_CTF_NO_TECH))
	{
    // -- TeT
		while (tnames[i])
		{
			if ((tech = FindItemByClassname(tnames[i])) != NULL &&
				ent->client->pers.inventory[ITEM_INDEX(tech)])
			{
				ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
				break;
			}
			i++;
		}
	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = gi.imageindex ("i_ctf1");
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT) //hidden, so someone must have it
		{
			int i;
			int someonehasit = 0;

			// not at base
			// check if on player
			p1 = gi.imageindex ("i_ctf1d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
			{
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)])
				{
					// enemy has it
					p1 = gi.imageindex ("i_ctf1t");
					someonehasit = 1;
					break;
				}
			}
			if ((someonehasit == 0) && (flag1dropped == 0))
			{
				gi.dprintf("MISSING FLAG 1 RESET BACK TO BASE\n");
				CTFResetFlag(CTF_TEAM1);
			}
		}
		else if (e->spawnflags & DROPPED_ITEM)
			p1 = gi.imageindex ("i_ctf1d"); // must be dropped
	}
	p2 = gi.imageindex ("i_ctf2");
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;
			int someonehasit = 0;

			// not at base
			// check if on player
			p2 = gi.imageindex ("i_ctf2d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
			{
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)])
				{
					// enemy has it
					p2 = gi.imageindex ("i_ctf2t");
					someonehasit = 1;
					break;
				}
			}
			if ((someonehasit == 0) && (flag2dropped == 0))
			{
				gi.dprintf("MISSING FLAG 2 RESET BACK TO BASE\n");
				CTFResetFlag(CTF_TEAM2);
			}

		}
		else if (e->spawnflags & DROPPED_ITEM)
			p2 = gi.imageindex ("i_ctf2d"); // must be dropped
	}


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

	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1 &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf2");

	else if (ent->client->resp.ctf_team == CTF_TEAM2 &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] &&
		(level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf1");

	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");

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
//ERASER START
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
//ERASER END

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
 	if ((self->owner) && (self->owner->client) && (self->owner->client->ctf_grapple)) {
//	if (self->owner->client->ctf_grapple) {//ERASER
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		KillGrappleSoundKiller(self->owner);//ERASER

		if ((self->owner) && (self->owner->client))
		{
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
			cl = self->owner->client;
			cl->ctf_grapple = NULL;
			cl->ctf_grapplereleasetime = level.time;
			cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
			cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
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

	if (surf && (surf->flags & SURF_SKY))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage || other->client) {//ERASER ADDED || other->client
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->owner->client->ctf_grapplestart = level.time;//ERASER

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	SpawnGrappleSoundKiller(self->owner);//ERASER

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
//WF
	if (!self->owner)
		return;

	if (!self->owner->client)
		return;

	if (!self->owner->client->pers.weapon)
		return;

	if (!self->owner->client->pers.weapon->classname)
		return;
//WF


if (!self->owner->bot_client)//ERASER
	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING)
	{
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
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (!self->enemy || self->enemy->deadflag) { // he died//ERASER ADDED || self->enemy->deadflag
			CTFResetGrapple(self);
			return;
		}
	}

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
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

//ERASER START
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
//ERASER END
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
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
	grapple->nextthink = level.time + FRAMETIME;//WF24 HAS THIS CO
	grapple->think = CTFGrappleThink;//WF24 HAS THIS CO
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

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

	SpawnGrappleSoundKiller(ent);//ERASER

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
//Eraser START: record this position, so we drop a grapple node here, rather than where the player is when they leave the ground
	if (!ent->bot_client)
	{	botDebugPrint("Grapple pull\n");
	VectorCopy(ent->s.origin, ent->animate_org);//fixme acrid
	}
//ERASER END
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;
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

//Determine which team to switch to
int get_mercenary_team(edict_t *ent)
{
	int team1count = 0, team2count = 0;
	int merc_team;
	int i;
	edict_t		*player;

	//Count players per team to see if we can switch
	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		switch (player->client->resp.ctf_team) {
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
		}
	}

	//Which team is loosing?
	if (ctfgame.team1 == ctfgame.team2)
		merc_team = ent->client->resp.ctf_team;	//same score, stay the same
	else if (ctfgame.team1 > ctfgame.team2)	//team 2 is loosing
	{
		if ((team2count - team1count) >= 2)	//team2 already have enough
		{
			merc_team = CTF_TEAM1;
		}
		else
		{
			merc_team = CTF_TEAM2;
		}
	}
	else									//team 1 is loosing
	{
		if ((team1count - team2count) >= 2)	//team1 already have enough
		{
			merc_team = CTF_TEAM2;
		}
		else
		{
			merc_team = CTF_TEAM1;
		}
	}

	return merc_team;

}

void CTFTeam_f (edict_t *ent)
{
	char *t, *s;
	int desired_team;
	int loosing_team;

	if (!ctf->value)
		return;

	//WF
	if (ent->solid == SOLID_NOT)
		return;
	if((level.time - ent->client->respawn_time) < 5)
	{
		safe_cprintf(ent, PRINT_HIGH, "Rapid Team switch not allowed.\n");
		return;
	}
	//WF

	t = gi.args();
	if (!*t) {
		safe_cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}
	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;
	else {
		safe_cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}

	if (ent->client->resp.ctf_team == desired_team) {
		safe_cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	//Mercenary ability only allows you to switch to loosing team
	if (ent->client->player_special & SPECIAL_MERCENARY)
	{
		loosing_team = get_mercenary_team(ent);
		if (loosing_team != desired_team)
		{
			safe_cprintf(ent, PRINT_HIGH, "Team switch not allowed at this time.\n");
			return;
		}
	}


////
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;

	//If we are playing in CTF mode, they picked team.  Otherwise it was assigned to them
	ent->client->resp.ctf_team = desired_team;
	ent->wf_team = desired_team;
	ent->client->resp.next_ctf_team = desired_team;

	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	//Remove all their old stuff lying around
	//gi.dprintf("Team changed-removing stuff.\n");
	WFPlayer_ChangeClassTeam(ent);

    sl_LogPlayerTeamChange( &gi,
                            ent->client->pers.netname,
                            CTFTeamName(ent->client->resp.ctf_team));

	if (ent->solid == SOLID_NOT) { // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		if (ctf->value) my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			ent->client->pers.netname, CTFTeamName(desired_team));
	    sl_LogPlayerTeamChange( &gi,
            ent->client->pers.netname,
                           CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	ent->health = 0;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);

	//Don't clear scores for mercenary capability
	if (!(ent->client->player_special & SPECIAL_MERCENARY))
		ent->client->resp.score = 0;

	my_bprintf(PRINT_HIGH, "%s changed to the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
    sl_LogPlayerTeamChange( &gi,
		ent->client->pers.netname,
		CTFTeamName(ent->client->resp.ctf_team));
}

/*
==================
CTFScoreboardMessage
==================
*/
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	char	classcode;
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
			classcode = getClassCode (cl);

//ACRID ADDED FOR REALISM ERASER
		if (cl_ent->bot_client)
		{
			cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
			if (cl->ping < 0)
				cl->ping = 0;
		}
//END

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %c %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score,
			(cl->ping > 999) ? 999 : cl->ping,
			classcode,
			cl->pers.netname);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				strcat(entry, "xv 56 picn sbfctf2 ");
#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
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
			classcode = getClassCode (cl);

//ACRID ADDED FOR REALISM ERASER
		if (cl_ent->bot_client)
		{
			cl->ping = (int) cl_ent->bot_stats->avg_ping + ((random() * 2) - 1) * 80;
			if (cl->ping < 0)
				cl->ping = 0;
		}
//END

#if 0 //ndef NEW_SCORE
			sprintf(entry+strlen(entry),
			"xv 160 %s \"%3d %3d %c %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score,
			(cl->ping > 999) ? 999 : cl->ping,
			classcode,
			cl->pers.netname);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				strcat(entry, "xv 216 picn sbfctf1 ");

#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
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

/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2) {
		if (!who->bot_client)//ERASER
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
//WF24 S
//	gitem_t *tech;
//	int i;
//WF24 E GOES WITH BELOW CO LINES
//ERASER START
	if (	(other->client->pers.inventory[ITEM_INDEX(item_tech1)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech2)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech3)])
		||	(other->client->pers.inventory[ITEM_INDEX(item_tech4)]))
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
	other->client->ctf_has_tech = true;//ERASER CHECK FOR DUPE
	return true;
}

static void SpawnTech(gitem_t *item, edict_t *spot);

static edict_t *FindTechSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL) {
		SpawnTech(tech->item, spot);
		RemoveFromItemList(tech);//ERASER
		G_FreeEdict(tech);
	} else {
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	//Don't drop auto-doc if player was given auto-doc on startup
	if ((strcmp(item->classname, "item_tech4") == 0) && (ent->client->player_special & SPECIAL_HEALING))
	{
		//Don't do anything in this case
	}
	else
	{
		tech = Drop_Item(ent, item);
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
		ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	}

}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

	i = 0;
	while (tnames[i])
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)])
		{
			//Don't drop auto-doc if player was given auto-doc on startup
			if ((strcmp(tnames[i], "item_tech4") == 0) && (ent->client->player_special & SPECIAL_HEALING))
			{
				//Don't do anything in this case
			}
			else
			{
				dropped = Drop_Item(ent, tech);
				// hack the velocity to make it bounce random
				dropped->velocity[0] = (rand() % 600) - 300;
				dropped->velocity[1] = (rand() % 600) - 300;
				dropped->nextthink = level.time + CTF_TECH_TIMEOUT;
				dropped->think = TechThink;
				dropped->owner = NULL;
				ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
			}
		}
		i++;
	}
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
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

	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;

	gi.linkentity (ent);
//ERASER START
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
//ERASER END

static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			(spot = FindTechSpawn()) != NULL)
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

//ERASER ADDED !CTF->VALUE
	if (!ctf->value || techspawn || ((int)dmflags->value & DF_CTF_NO_TECH))
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
		tech = FindItemByClassname("item_tech1");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return dmg / 2;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		return dmg * 2;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
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
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) {
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
//	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && client->pers.inventory[ITEM_INDEX(tech)]) {
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < 100) {
				ent->health += 5;
				if (ent->health > 100)
					ent->health = 100;
				//WF - take longer to regenerate health
//				client->ctf_regentime += 0.5;
				client->ctf_regentime += 1.7;
				noise = true;
			}


		}
		HealPlayer(ent);
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
	if (ent->health > 100)	ent->health = 100;
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
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
	{	"item_flag_team1",			0 },
	{	"item_flag_team2",			0 },
	{	"info_position",			1 },
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
	edict_t *info_pos = NULL;

	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item = NULL;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;
	qboolean isposition = false;

	while ((what = loc_findradius(what, who->s.origin, 600)) != NULL)
	{
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
		{
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		}
		if (!loc_names[i].classname)
			continue;	//Not in the list

/*
gi.dprintf("DEBUG-I see class %s\n",what->classname);
if 	(what->message && what->message[0])
  gi.dprintf("  DEBUG-I see Message = %s\n", what->message);
*/

		cansee = loc_CanSee(what, who);

		//For priority 1 items, assume you can always see them.
		if (loc_names[i].priority == 1) cansee = true;

		// something we can see get priority over something we can't
		if (cansee && !hotsee)
		{
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
//gi.dprintf("  DEBUG-Can See item selected\n");
			continue;
		}

		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;

		if (hotsee && hotindex < loc_names[i].priority)
			continue;

		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || (cansee && loc_names[i].priority < hotindex))
//		if (newdist < hotdist || loc_names[i].priority < hotindex))
		{
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = cansee;
//gi.dprintf("  DEBUG-Hot set by distance.\n" );
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


	//Is this an info_position entity?  If so, it's not an item
	if (strcmp(hot->classname, "info_position") == 0)
		isposition = true;
	else
	{
		item = FindItemByClassname(hot->classname);
		if (!item)
		{
			//strcpy(buf, "%s",vtos(who->s.origin));
			strcpy(buf, vtos(who->s.origin));
			//strcpy(buf, "nowhere.");
			return;
		}
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	if (isposition == false) //the info_possition messages are good enough
	{
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
	}

	//If this is an info_position, use the message rather than classname
	if (isposition == true)
	{
		if (hot->message && hot->message[0])
			strcat(buf, hot->message);
	}
	else
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

				default :
					*p++ = *msg;
			}
		} else
			*p++ = *msg;
	}
	*p = 0;

	if ( strlen(outmsg) > 150)	// Fix overflow with %L%L%L
		outmsg[150] = 0;	// commands

	if (BannedWords(who, outmsg))
		return;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (cl_ent->client->pers.nospam_level & SPAM_NOTEAMMSG)
			continue;	//they don't want team messages

		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			safe_cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n",
				who->client->pers.netname, outmsg);
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
	if (ent->spawnflags == 1) // team2
		ent->s.skinnum = 1;
	else if (ent->spawnflags == 2)
		ent->s.skinnum = 2;
	else if (ent->spawnflags == 3)
		ent->s.skinnum = 3;
	else if (ent->spawnflags == 4)
		ent->s.skinnum = 4;
	else if (ent->spawnflags == 5)
		ent->s.skinnum = 5;

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
	if (ent->spawnflags == 1) // team2
		ent->s.skinnum = 1;
	else if (ent->spawnflags == 2)
		ent->s.skinnum = 2;
	else if (ent->spawnflags == 3)
		ent->s.skinnum = 3;
	else if (ent->spawnflags == 4)
		ent->s.skinnum = 4;
	else if (ent->spawnflags == 5)
		ent->s.skinnum = 5;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

	PMenu_Close(ent);


	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

    // Log Join Team - MarkDavies
    sl_LogPlayerName( &gi,
                      ent->client->pers.netname,
                      CTFTeamName(ent->client->resp.ctf_team));


	PutClientInServer (ent);

	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	if (ctf->value) my_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, CTFTeamName(desired_team));
   	sl_LogPlayerTeamChange( &gi,
		ent->client->pers.netname,
		CTFTeamName(ent->client->resp.ctf_team));

	//K2
	ent->client->resp.inServer = true;
	//K2 3/99 acrid botcam

    //WF - Add welcome sound
//    gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("welcome3.wav"), 1, ATTN_STATIC, 0);
//    stuffcmd (ent, "play welcome3.wav\n");

}

void CTFJoinTeam1(edict_t *ent, pmenu_t *p)
{
	ent->client->resp.next_ctf_team = CTF_TEAM1;
	WFOpenClassMenu(ent);
	//CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenu_t *p)
{
	ent->client->resp.next_ctf_team = CTF_TEAM2;
	WFOpenClassMenu(ent);
	//CTFJoinTeam(ent, CTF_TEAM2);
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
	//CTFOpenJoinMenu(ent);
}

void CTFCredits(edict_t *ent, pmenu_t *p);

void DeathmatchScoreboard (edict_t *ent);


//WF
void WFHelp(edict_t *ent, pmenu_t *p);
void WFContinue(edict_t *ent, pmenu_t *p);
void WFShowMoreHelp(edict_t *ent, pmenu_t *p);
void WFEraserHelp(edict_t *ent, pmenu_t *p);//Acrid
void WFServer(edict_t *ent, pmenu_t *p);//Acrid
void WFRoutes(edict_t *ent, pmenu_t *p);//Acrid
void WFBindKeys(edict_t *ent, pmenu_t *p);
void WFCredits(edict_t *ent, pmenu_t *p);

pmenu_t wfcreditsmenu[] = {
	{ "*The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Gregg 'Headache' Reno",			PMENU_ALIGN_CENTER, 0, NULL },
	{ "John 'Cyrect' Rittenhouse",		PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Tom 'Tumorhead' Reno",			PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Sound",							PMENU_ALIGN_CENTER, 0, NULL },
	{ "Wayne 'Tubeblaster' Reno",		PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "Visit:",							PMENU_ALIGN_CENTER, 0, NULL },
    { WF_URL,							PMENU_ALIGN_CENTER, 0, NULL },
	{ "for details",					PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, 0, CTFReturnToMain }
};
//WF

pmenu_t creditsmenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, 0, NULL },
	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Level Design", 					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Christian Antkow",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "Tim Willits",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Art",							PMENU_ALIGN_CENTER, 0, NULL },
	{ "Adrian Carmack Paul Steed",		PMENU_ALIGN_CENTER, 0, NULL },
	{ "Kevin Cloud",					PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Sound",							PMENU_ALIGN_CENTER, 0, NULL },
	{ "Tom 'Bjorn' Klok",				PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Original CTF Art Design",		PMENU_ALIGN_CENTER, 0, NULL },
	{ "Brian 'Whaleboy' Cozzens",		PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, 0, CTFReturnToMain }
};


//WF many changed here
pmenu_t joinmenu[] = {
    { "The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
    { NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ WF_VERSION,						PMENU_ALIGN_CENTER, 0, NULL },
    { NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "*Select a Team:",				PMENU_ALIGN_LEFT, 0, NULL },
	{ "1. Join Red Team",				PMENU_ALIGN_LEFT, 0, CTFJoinTeam1 },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "2. Join Blue Team",				PMENU_ALIGN_LEFT, 0, CTFJoinTeam2 },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "3. Chase Camera",				PMENU_ALIGN_LEFT, 0, CTFChaseCam },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
//	{ "Credits: id",					PMENU_ALIGN_LEFT, 0, CTFCredits },
    { WF_URL,							PMENU_ALIGN_LEFT, 0, NULL },
//	{ "Credits: Weapons Factory",		PMENU_ALIGN_LEFT, 0, WFCredits },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "Use [ and ] to move cursor",		PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER to select",				PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC to Exit Menu",				PMENU_ALIGN_LEFT, 0, NULL },
	{ "Type WFHELP for commands",		PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
	{ "*                 ctf v" CTF_STRING_VERSION,		PMENU_ALIGN_LEFT, 0, NULL },

};
//WF

void WFClassPicked(edict_t *ent, pmenu_t *p)
{
	int i;
	int classcount[MAX_CLASSES + 1];

	int initial_class;

	i = p->arg;

	//Default to class # 1
	if (i < 1 || i > numclasses) i = 1;

	//See if there are any class limits defined
	WFClassCount(ent, classcount);

//	gi.dprintf ("class limit on %s is %d. You picked %d. Curr=%d\n",
//				classinfo[i].name,classinfo[i].limit, i, classcount[i]);

	if (classcount[i] >= classinfo[i].limit)
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, class limit on %s is %d. Pick another class.\n",
				classinfo[i].name,classinfo[i].limit);
		return;
	}

	PMenu_Close(ent);
	initial_class = ent->client->pers.next_player_class;
	ent->client->pers.next_player_class = i;

	//If team not picked, then this is the first time
	if (ent->client->resp.ctf_team == CTF_NOTEAM || initial_class == 0)
	{
		//CTFOpenJoinMenu(ent);
		ent->client->pers.player_class = ent->client->pers.next_player_class;
		if (ctf->value)
			CTFJoinTeam(ent, ent->client->resp.next_ctf_team);
		else	//death match already has team assigned
			CTFJoinTeam(ent, ent->client->resp.ctf_team);
	}
	//Otherwise, they are changing classes
	else
	{
		safe_cprintf (ent, PRINT_HIGH, "You will become a %s the next time you respawn. \n",
							classinfo[i].name);

		//safe_cprintf (ent, PRINT_HIGH, "Your class will be changed the next time you die.\n");
	}
}



void WFPickMap(edict_t *ent, pmenu_t *p);

pmenu_t wfmapvote[] = {
    { "Please Vote For Next Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};


pmenu_t wfclassmenu[] = {
    { "The Weapons Factory",		PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, 0, NULL },
    { WF_VERSION,					PMENU_ALIGN_CENTER, 0, NULL },
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { "*Select A Class:",			PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, 0, NULL },
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,							PMENU_ALIGN_LEFT, 0, NULL},
    { "Help...",					PMENU_ALIGN_LEFT, 0, WFShowHelp },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER selects, ESC exits",	PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t wfhelp[] = {
	{ "*HELP MENU",					PMENU_ALIGN_CENTER, 0, NULL },
	{ WF_VERSION,					PMENU_ALIGN_CENTER, 0, NULL },
//    { "You must download files at", PMENU_ALIGN_LEFT, 0, NULL },
    { WF_URL,						PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, 0, NULL },
    { "*Recommended Keys:",			PMENU_ALIGN_LEFT, 0, NULL },
    { "v - Hold down to fly",		PMENU_ALIGN_LEFT, 0, NULL },
    { "b - Cycle Grenades",			PMENU_ALIGN_LEFT, 0, NULL },
    { "; - Special Menu",			PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,							PMENU_ALIGN_LEFT, 0, NULL },
	{ "Continue",					PMENU_ALIGN_LEFT, 0, WFContinue },
	{ "Bind These Keys Now",		PMENU_ALIGN_LEFT, 0, WFBindKeys },
	{ "Other Commands...",			PMENU_ALIGN_LEFT, 0, WFShowMoreHelp },
	{ "Eraser Commands...",			PMENU_ALIGN_LEFT, 0, WFEraserHelp },//acrid
	{ NULL,							PMENU_ALIGN_LEFT, 0, NULL },
	{ "Press ENTER to continue",	PMENU_ALIGN_LEFT, 0, NULL },
    { "Type WFHELP to see again",   PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t wfmorehelp[] = {
	{ WF_VERSION,					PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, 0, NULL },
    { "cmd changeclass",			PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd decoy",					PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd grapple",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd grenade1",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd grenade2",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd grenade3",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd homing",					PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd showclass",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd vote",					PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd wfflags",				PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd wfhelp",					PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd wfplay xyz.wav",			PMENU_ALIGN_LEFT, 0, NULL },
    { "cmd wfplayteam xyz.wav",		PMENU_ALIGN_LEFT, 0, NULL },
	{ "Continue",					PMENU_ALIGN_LEFT, 0, WFContinue },
	{ NULL,							PMENU_ALIGN_LEFT, 0, NULL },
	{ "Press ENTER to continue",	PMENU_ALIGN_LEFT, 0, NULL },
    { "Type WFHELP to see again",   PMENU_ALIGN_LEFT, 0, NULL },
};

//Acrid
pmenu_t wferaser[] = {
	{ WF_VERSION,	                PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					        PMENU_ALIGN_CENTER, 0, NULL },
    { "group",	                    PMENU_ALIGN_LEFT, 0, NULL },
    { "disperse",	                PMENU_ALIGN_LEFT, 0, NULL },
    { "defendbase",	                PMENU_ALIGN_LEFT, 0, NULL },
	{ "rushbase",	                PMENU_ALIGN_LEFT, 0, NULL },
    { "freestyle",	                PMENU_ALIGN_LEFT, 0, NULL },
	{ "cam",	                    PMENU_ALIGN_LEFT, 0, NULL },
	{ "Server Commands...",			PMENU_ALIGN_LEFT, 0, WFServer },
	{ "Route Commands...",			PMENU_ALIGN_LEFT, 0, WFRoutes },
	{ "Continue",					PMENU_ALIGN_LEFT, 0, WFContinue },
	{ NULL,							PMENU_ALIGN_LEFT, 0, NULL },
	{ "Press ENTER to continue",	PMENU_ALIGN_LEFT, 0, NULL },
    { "Type WFHELP to see again",   PMENU_ALIGN_LEFT, 0, NULL },
};
pmenu_t wfserver[] = {
	{ WF_VERSION,						PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
    { "servcmd",  						PMENU_ALIGN_LEFT, 0, NULL },
    { "tips",							PMENU_ALIGN_LEFT, 0, NULL },
    { "sv bluebots <name1> <name2>",	PMENU_ALIGN_LEFT, 0, NULL },
    { "sv redbots  <name1> <name2>",	PMENU_ALIGN_LEFT, 0, NULL },
    { "lag <n> 0-1000",					PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_num <n> 0-32?",	            PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_name <name>",				PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_allow_client_commands 0/1",	PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_free_clients <n> 0-32?",	    PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_show_connect_info  0/1",	    PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_auto_skill  0/1",	        PMENU_ALIGN_LEFT, 0, NULL },
    { "skill <n> 1-3",					PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_drop <name>",				PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_chat  0/1",	                PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_tarzan 0/1",	                PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_melee 0/1",	                PMENU_ALIGN_LEFT, 0, NULL },
    { "botpause",						PMENU_ALIGN_LEFT, 0, NULL },
	{ "...Go Back",						PMENU_ALIGN_LEFT, 0, WFEraserHelp },
	{ "Continue",						PMENU_ALIGN_LEFT, 0, WFContinue },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "Press ENTER to continue",		PMENU_ALIGN_LEFT, 0, NULL },
    { "Type WFHELP to see again",		PMENU_ALIGN_LEFT, 0, NULL },
};
pmenu_t wfroutes[] = {
	{ WF_VERSION,						PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, 0, NULL },
    { "bot_calc_nodes 0/1",	            PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_debug_nodes 0/1",	        PMENU_ALIGN_LEFT, 0, NULL },
    { "showpath",						PMENU_ALIGN_LEFT, 0, NULL },
    { "bot_optimize 0/1",	            PMENU_ALIGN_LEFT, 0, NULL },
    { "redflag",						PMENU_ALIGN_LEFT, 0, NULL },
    { "blueflag",						PMENU_ALIGN_LEFT, 0, NULL },
    { "clearflags",						PMENU_ALIGN_LEFT, 0, NULL },
    { "flagpath",						PMENU_ALIGN_LEFT, 0, NULL },
    { "clear_flagpaths",				PMENU_ALIGN_LEFT, 0, NULL },
    { "toggle_flagpaths",				PMENU_ALIGN_LEFT, 0, NULL },
    { "ctf_item <item>",				PMENU_ALIGN_LEFT, 0, NULL },
    { "clear_items",   					PMENU_ALIGN_LEFT, 0, NULL },
    { "botpath",						PMENU_ALIGN_LEFT, 0, NULL },
    { "ctf 0/1 depends on map",			PMENU_ALIGN_LEFT, 0, NULL },
	{ "...Go Back",						PMENU_ALIGN_LEFT, 0, WFEraserHelp },
	{ "Continue",						PMENU_ALIGN_LEFT, 0, WFContinue },
	{ NULL,								PMENU_ALIGN_LEFT, 0, NULL },
	{ "Press ENTER to continue",		PMENU_ALIGN_LEFT, 0, NULL },
    { "Type WFHELP to see again",		PMENU_ALIGN_LEFT, 0, NULL },
};

void WFContinue(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	//If a team has not been picked, go to the team menu

	if (ent->client->resp.ctf_team == CTF_NOTEAM)
		CTFOpenJoinMenu(ent);

	//If a class has not been picked, go to the class menu
	else if (ent->client->pers.player_class == 0)
		WFOpenClassMenu(ent);


}

void WFShowHelp(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);

	PMenu_Open(ent, wfhelp, -1, sizeof(wfhelp) / sizeof(pmenu_t), false, true);
}

void WFShowMoreHelp(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);
	PMenu_Open(ent, wfmorehelp, -1, sizeof(wfmorehelp) / sizeof(pmenu_t), false, true);
}

void WFEraserHelp(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, wferaser, -1, sizeof(wferaser) / sizeof(pmenu_t),false, true);
}

void WFServer(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, wfserver, -1, sizeof(wfserver) / sizeof(pmenu_t),false, true);
}

void WFRoutes(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, wfroutes, -1, sizeof(wfroutes) / sizeof(pmenu_t),false, true);
}

void WFBindKeys(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	stuffcmd(ent,"bind \";\" cmd special\n");
	stuffcmd(ent,"bind v +thrust\n");
	stuffcmd(ent,"bind b cmd grenade\n");

	//If a class has not been picked, go to the class menu
	if (ent->client->pers.player_class == 0)
		WFOpenClassMenu(ent);
}


void WFFillMapNames()
{
	int i;
	int pos;

	pos = 1;
	for (i = 0; i < maplist.nummaps && pos < MAX_MENU_MAPS; ++i)
	{
		sprintf(menustring[pos],"%d. %s", pos, maplist.mapnames[i]);
		wfmapvote[pos].text = menustring[pos];
		wfmapvote[pos].SelectFunc = WFPickMap;
		wfmapvote[pos].arg = i;
		++pos;
	}

	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS)
	{
		wfmapvote[pos].text = "";
		wfmapvote[pos].SelectFunc = NULL;
		wfmapvote[pos].arg = 0;
		++pos;
	}

}

void WFMapVote(edict_t *ent)
{
	//WFFillMapNames();
	if (ent->client->menu) PMenu_Close(ent);
	PMenu_Open(ent, wfmapvote, -1, sizeof(wfmapvote) / sizeof(pmenu_t), true, true);
}

//Set up menu with special capabilities that can be picked
//from menu
void place_turret (edict_t *ent);
void UpgradeSentry(edict_t *self);
void PlaceBiosentry (edict_t *ent);
void BiosentryRepair(edict_t *self);

void WFPickSpecial(edict_t *ent, pmenu_t *p)
{
	int i;
//	char	 *s;

	i = p->arg;

	PMenu_Close(ent);

	if (!ent->client) return;

	switch (i)
	{
	case (SPECIAL_REMOTE_CAMERA):
		cmd_CameraPlace(ent);		//Place or blow up camera
		break;

	case (SPECIAL_REMOTE_CAMERA + SPECIAL_OPTION1):
		if (ent->remotecam) ent->remotecam->dmg = 0;	//Silent removal of camera
		cmd_CameraPlace(ent);
		break;

	case (SPECIAL_REMOTE_CAMERA + SPECIAL_OPTION2):
		cmd_CameraToggle(ent);		//Switch Views
		break;

	case (SPECIAL_TRIPBOMB):
		cmd_TripBomb(ent);
		break;

	case (SPECIAL_LASER_DEFENSE):
		cmd_LaserDefense(ent);
		break;

	case (SPECIAL_LASER_DEFENSE + SPECIAL_OPTION1):
		cmd_RemoveLaserDefense(ent);
		break;

	case (SPECIAL_SUPPLY_DEPOT):
		SP_SupplyDepot(ent);
		break;

	case (SPECIAL_HEALING):
		SP_HealingDepot(ent);
		break;

	case (SPECIAL_ALARMS):
		place_alarm(1, ent);
		break;

	case (SPECIAL_ALARMS + SPECIAL_OPTION1):
		alarm_remove(ent->alarm1);
		break;

	case (SPECIAL_PLASMA_BOMB):
		//cmd_PlasmaBomb(ent);
		cmd_PlasmaBombMenu(ent);
		break;

	case (SPECIAL_KAMIKAZE):
		Start_Kamikaze_Mode(ent);
		break;

	case (SPECIAL_KAMIKAZE + SPECIAL_OPTION1):
		Kamikaze_Cancel(ent);
		safe_cprintf (ent, PRINT_HIGH, "Kamikaze canceled! You are safe now!\n");
		break;

	case (SPECIAL_ANTIGRAV_BOOTS):
		if (ent->flags & FL_BOOTS)
		{
			safe_cprintf (ent, PRINT_HIGH, "Anti Gravity Boots off\n");
			ent->flags -= FL_BOOTS;
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Anti Gravity Boots on\n");
			ent->flags |= FL_BOOTS;
		}
		break;

	case (SPECIAL_SENTRY_GUN):
		place_turret (ent);
		break;
	case (SPECIAL_SENTRY_GUN + SPECIAL_OPTION1):
		UpgradeSentry(ent);
		break;
	case (SPECIAL_BIOSENTRY):
		PlaceBiosentry (ent);
		break;
//	case (SPECIAL_BIOSENTRY + SPECIAL_OPTION1):
//		BiosentryRepair(ent);
//		break;
	case (SPECIAL_MISSILE_LAUNCHER):
		place_missile(ent);
		break;
	case (SPECIAL_MISSILE_LAUNCHER + SPECIAL_OPTION1):
		UpgradeMissileTurret(ent);
		break;
	case (SPECIAL_DISGUISE):
		if (ent->disguised == 0)
		{
			if (CTFHasFlag(ent))
			{
				safe_cprintf(ent, PRINT_HIGH, "You can't disguise while holding the flag!\n");
			}
			else
			{
				ent->disguising = 0;
				WFChooseDisguise(ent);
			}
		}
		else
		{
			WFRemoveDisguise(ent);
		}
		break;

	case (SPECIAL_LASER):
		if(ent->client->pers.laseron)
			ent->client->pers.laseron=0;
		else
			ent->client->pers.laseron=1;
		break;
	case (SPECIAL_FEIGN):
		if (CTFHasFlag(ent))
			{
				safe_cprintf(ent, PRINT_HIGH, "You can't feign death while holding the flag!\n");
			}
		else
			Cmd_Feign_f (ent);
		break;
	case (SPECIAL_AUTOZOOM):
		if(ent->client->pers.autozoom)
		{
			ent->client->pers.autozoom=0;
			ent->client->ps.fov = 90;
		}
		else
			ent->client->pers.autozoom=1;
		break;
	case (SPECIAL_QUICKAIM):
		if(ent->client->pers.fastaim)
			ent->client->pers.fastaim=0;
		else
			ent->client->pers.fastaim=1;
		break;
	case (SPECIAL_CLOAK):
		Cmd_Cloak_f (ent);
		break;
	default:
		safe_cprintf(ent, PRINT_HIGH, "Sorry, '%s' hasn't been implemented yet.\n", p->text);
		break;
	}

}


void WFFillSpecial(edict_t *ent)
{
//	int i;
	int pos;
	int itemno;

	ent->client->wfspecial[0].text = "Select Special Item";
	ent->client->wfspecial[0].SelectFunc = NULL;
	ent->client->wfspecial[0].align = PMENU_ALIGN_CENTER;
	ent->client->wfspecial[0].arg = 0;

	ent->client->wfspecial[1].text = NULL;
	ent->client->wfspecial[1].SelectFunc = NULL;
	ent->client->wfspecial[1].align = PMENU_ALIGN_CENTER;
	ent->client->wfspecial[1].arg = 0;

	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 3].text = "Use [ and ] to move cursor";
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 3].SelectFunc = NULL;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 3].align = PMENU_ALIGN_LEFT;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 3].arg = 0;

	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 2].text = "ENTER to select";
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 2].SelectFunc = NULL;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 2].align = PMENU_ALIGN_LEFT;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 2].arg = 0;

	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 1].text = "ESC to Exit Menu";
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 1].SelectFunc = NULL;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 1].align = PMENU_ALIGN_LEFT;
	ent->client->wfspecial[MAX_SPECIAL_MENU_ITEMS - 1].arg = 0;

	//Clear the rest
	pos = 2;
	while (pos < (MAX_SPECIAL_MENU_ITEMS - 3))
	{
		ent->client->wfspecial[pos].text = NULL;
		ent->client->wfspecial[pos].SelectFunc = NULL;
		ent->client->wfspecial[pos].align = PMENU_ALIGN_LEFT;
		ent->client->wfspecial[pos].arg = 0;
		++pos;
	}


	pos = 2;
	itemno = 1;

	if ((ent->client->player_special & SPECIAL_FEIGN) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->client->pers.feign == 0)
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Feign Death");
		else
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Live Again");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_FEIGN;
		++pos;

		//If they are feigning, they cant do anything else from special menu
		if (ent->client->pers.feign)
			return;

	}

	if ((ent->client->player_special & SPECIAL_DISGUISE) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->disguised == 0)
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Disguise");
		else
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove Disguise");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_DISGUISE;
		++pos;
	}

	if ((ent->client->player_special & SPECIAL_PLASMA_BOMB) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Drop Plasma Bomb");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_PLASMA_BOMB;
		++pos;
	}

	if ((ent->client->player_special & SPECIAL_SENTRY_GUN) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->sentry)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Upgrade Sentry Gun");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_SENTRY_GUN + SPECIAL_OPTION1;
			++pos;

			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove Sentry Gun");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_SENTRY_GUN;
			++pos;

		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Place Sentry Gun");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_SENTRY_GUN;
			++pos;
		}

	}


	if ((ent->client->player_special & SPECIAL_BIOSENTRY) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->sentry)
		{
			/*
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Repair Biosentry");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_BIOSENTRY + SPECIAL_OPTION1;
			++pos;
			*/

			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove Biosentry");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_BIOSENTRY;
			++pos;

		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Place Biosentry");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_BIOSENTRY;
			++pos;
		}
	}


	if ((ent->client->player_special & SPECIAL_ANTIGRAV_BOOTS) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Use Anti-Grav Boots");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_ANTIGRAV_BOOTS;
		++pos;
	}


	if ((ent->client->player_special & SPECIAL_MISSILE_LAUNCHER) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->missile)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Upgrade Missile Launcher");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_MISSILE_LAUNCHER + SPECIAL_OPTION1;
			++pos;

			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove Missile Launcher");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_MISSILE_LAUNCHER;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Build Missile Launcher");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_MISSILE_LAUNCHER;
			++pos;
		}

	}


	if ((ent->client->player_special & SPECIAL_ALARMS) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->alarm1)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove Alarm");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_ALARMS  + SPECIAL_OPTION1;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Set Alarm");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_ALARMS;
			++pos;
		}
	}

	if ((ent->client->player_special & SPECIAL_KAMIKAZE) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Go Kamizaze!");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_KAMIKAZE;
		++pos;

		sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Cancel Kamizaze");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_KAMIKAZE + SPECIAL_OPTION1;
		++pos;
	}
	if ((ent->client->player_special & SPECIAL_LASER_DEFENSE) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->client->pers.active_special[ITEM_SPECIAL_LASER_DEFENSE] < MAX_SPECIAL_LASER_DEFENSE)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Build Laser Defense");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_LASER_DEFENSE;
			++pos;
		}
		if (ent->client->pers.active_special[ITEM_SPECIAL_LASER_DEFENSE] > 0)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Remove All Laser Defenses");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_LASER_DEFENSE + SPECIAL_OPTION1;
			++pos;
		}
	}

	if ((ent->client->player_special & SPECIAL_TRIPBOMB) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Build Tripbomb");
		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_TRIPBOMB;
		++pos;
	}

	if ((ent->client->player_special & SPECIAL_CLOAK) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->client->cloaking == 0)
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Cloak");
		else
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Un-Cloak");

		ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
		ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
		ent->client->wfspecial[pos].arg = SPECIAL_CLOAK;
		++pos;
	}

	if ((ent->client->player_special & SPECIAL_SUPPLY_DEPOT) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->supply)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Blow Up Supply Depot");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_SUPPLY_DEPOT;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Build Supply Depot");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_SUPPLY_DEPOT;
			++pos;
		}
	}
	if ((ent->client->player_special & SPECIAL_HEALING) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->supply)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Blow Up Healing Depot");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_HEALING;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Build Healing Depot");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_HEALING;
			++pos;
		}
	}

	if ((ent->client->player_special & SPECIAL_REMOTE_CAMERA) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{

		//If there is no camera, let them place one
		if (ent->remotecam == NULL)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Place Camera");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_REMOTE_CAMERA;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Blow Up Camera");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_REMOTE_CAMERA;
			++pos;

			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Silently Remove Camera");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_REMOTE_CAMERA + SPECIAL_OPTION1;
			++pos;
		}

		//Let user toggle camera view if a camera exists
		if (ent->remotecam)
		{
			if (ent->client->remotetoggle)	//It's already on
				sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Change View To Body");
			else
				sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Change View To Camera");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_REMOTE_CAMERA + SPECIAL_OPTION2;
			++pos;
		}
	}

	if((ent->client->player_special & SPECIAL_SNIPING) && pos <= MAX_SPECIAL_MENU_ITEMS - 4)
	{
		if (ent->client->pers.laseron)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Turn off sniping laser");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_LASER;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Turn on sniping laser");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_LASER;
			++pos;
		}
		if (ent->client->pers.autozoom)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Turn off autozoom");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_AUTOZOOM;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Turn on autozoom");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_AUTOZOOM;
			++pos;
		}
		if (ent->client->pers.fastaim)
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Switch to normal aiming");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_QUICKAIM;
			++pos;
		}
		else
		{
			sprintf(ent->client->wfmenustr[pos],"%d. %s", itemno++, "Switch to quick aiming");
			ent->client->wfspecial[pos].text = ent->client->wfmenustr[pos];
			ent->client->wfspecial[pos].SelectFunc = WFPickSpecial;
			ent->client->wfspecial[pos].arg = SPECIAL_QUICKAIM;
			++pos;
		}

	}
}

void WFSpecialMenu(edict_t *ent)
{
	if (ent->client->menu)
	{
		PMenu_Close(ent);
		return;
	}

	WFFillSpecial(ent);
	PMenu_Open(ent, ent->client->wfspecial, -1, sizeof(ent->client->wfspecial) / sizeof(pmenu_t), true, false);
}


void WFHelp(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);
	PMenu_Open(ent, wfhelp, -1, sizeof(wfhelp) / sizeof(pmenu_t), false, true);
}

void WFPickMap(edict_t *ent, pmenu_t *p)
{
	int i;

	i = p->arg;

	PMenu_Close(ent);
	if (i >= 0 && i < MAX_MENU_MAPS)
	{
		//If ref picked the map, go to it now
		if (wf_game.ref_ent == ent && wf_game.ref_picked_map == 1)
		{
            maplist.votes[i] = 999;
			wf_game.ref_picked_map = 0;
			WFEndDMLevel (maplist.mapnames[i]);
		}
		else
		{
            ++maplist.votes[i];
            ent->client->pers.HasVoted = true;
            my_bprintf(PRINT_HIGH,"%s Voted for %s\n",
                ent->client->pers.netname, maplist.mapnames[i]);
		}
	}
	else
		gi.dprintf("Bad Menu Item #%d\n",i);

}

//Fix the motd text so there are no invalid characters
/*
void FixMOTD(unsigned char *s, int len)
{
	int i;
	for (i = 0; i < len; ++i)
	{
		//Convert special characters
		if (s[i] == 128) s[i] = 40;		// (
		if (s[i] == 130) s[i] = 41;		// )
		if (s[i] == 144) s[i] = 91;		// [
		if (s[i] == 145) s[i] = 93;		// ]
		if (s[i] == 129) s[i] = 61;		// =
		if (s[i] == 133) s[i] = 46;		// .

		//Convert all other characters
		if (s[i] >= 128) s[i] = s[i] - 128;

		//get rid of other non-printable characters
		if (s[i] < 32 && s[i] > 0)
			s[i] = ' ';
		if (s[i] > 126)
			s[i] = ' ';

	}

}
*/

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

//WF
	//Load the message of the day file if available
/*
    FILE *motd_file;
	static char motd_line1[80];
	static char motd_line2[80];
	static char motd_line3[80];


	motd_line1[0] = '\0';
	motd_line2[0] = '\0';
	motd_line2[3] = '\0';

	if (motd_file = fopen("motd.txt", "r"))
	{
		readline(motd_file, motd_line1, 79);
		readline(motd_file, motd_line2, 79);
		readline(motd_file, motd_line3, 79);
		// close the file
		fclose(motd_file);
        }
	FixMOTD(motd_line1, 80);
	FixMOTD(motd_line2, 80);
	FixMOTD(motd_line3, 80);

	if (motd_line1[0] != '\0')
	{
		joinmenu[0].text = motd_line1;
		wfclassmenu[0].text = motd_line1;
	}
	if (motd_line2[0] != '\0')
	{
		joinmenu[1].text = motd_line2;
		wfclassmenu[1].text = motd_line2;
	}
	if (motd_line3[0] != '\0')
	{
		joinmenu[10].text = motd_line3;	//probably a URL
		wfhelp[2].text = motd_line3;
	}
*/
	if (wf_game.motd[0][0] != '\0')
	{
		joinmenu[0].text = wf_game.motd[0];
		wfclassmenu[0].text = wf_game.motd[0];
	}
	if (wf_game.motd[1][0] != '\0')
	{
		joinmenu[1].text = wf_game.motd[1];
		wfclassmenu[1].text = wf_game.motd[1];
	}
	if (wf_game.motd[2][0] != '\0')
	{
		joinmenu[11].text = wf_game.motd[2];	//probably a URL
		wfhelp[2].text = wf_game.motd[2];
	}


//gi.dprintf("Mod id = %d\n", modID);

	//Show if this is a WF or MOCK mod
	if (modID == MOD_ID_WF)
	{
		wfclassmenu[2].text = WF_VERSION;
		wfhelp[1].text = WF_VERSION;
		joinmenu[17].text = "*(WF Approved Config)";
	}
	else
	{
		wfclassmenu[2].text = "(MOCK Modified Game)";
		wfhelp[1].text = "(MOCK Modified Game)";
		joinmenu[17].text = "*(MOCK Modified Game)";
	}


	//Set the team names
	joinmenu[5].text = "1. Join Red Team";
	joinmenu[5].SelectFunc = CTFJoinTeam1;
	joinmenu[7].text = "2. Join Blue Team";
	joinmenu[7].SelectFunc = CTFJoinTeam2;

	if (ctf_forcejoin->string && *ctf_forcejoin->string) {
		if (stricmp(ctf_forcejoin->string, "red") == 0) {
			joinmenu[7].text = NULL;
			joinmenu[7].SelectFunc = NULL;
		} else if (stricmp(ctf_forcejoin->string, "blue") == 0) {
			joinmenu[5].text = NULL;
			joinmenu[5].SelectFunc = NULL;
		}
	}

	if (ent->client->chase_target)
		joinmenu[9].text = "3. Leave Chase Camera";
	else
		joinmenu[9].text = "3. Chase Camera";

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

	sprintf(team1players, "   (%d players)", num1);
	sprintf(team2players, "   (%d players)", num2);

	joinmenu[17].text = "*Current Map";
	joinmenu[18].text = levelname;
	if (joinmenu[5].text)
		joinmenu[6].text = team1players;
	else
		joinmenu[6].text = NULL;
	if (joinmenu[7].text)
		joinmenu[8].text = team2players;
	else
		joinmenu[8].text = NULL;

	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM1;
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void WFClassCount(edict_t *self, int *classcount)
{
	edict_t	*e;
	int i;
	int j;

	for (j = 1; j <= MAX_CLASSES; ++j)
		classcount[j] = 0;

	//Loop through all the entities to find players on the same team
	for (i = 1, e = g_edicts + i; i < game.maxclients; i++, e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//When using the menu, the team isn't set yet.  Must
		//use next team variable
//		if (e->wf_team != self->wf_team)
		if (e->client->resp.ctf_team == self->client->resp.next_ctf_team)
			continue;
		if (e->client->resp.ctf_team != CTF_TEAM1 && e->client->resp.ctf_team != CTF_TEAM2)
			continue;

		j = e->client->pers.player_class;
		if ((j >= 0) && (j <= MAX_CLASSES))
			++classcount[j];
	}
}

void WFUpdateClassMenu(edict_t *self)
{
	int i;
	int pos;


	int classcount[MAX_CLASSES + 1];

//	i = 0;
//	pos = 1 / i;

	sprintf(menutitle,"*Choose a class (%s)",classdefname);

	WFClassCount(self, classcount);

	pos = 6;
	for (i = 1; i <= numclasses && i <= MAX_CLASSES; ++i)
	{
		sprintf(classmenustring[i], "%d. %s (%d)", i, classinfo[i].name, classcount[i]);
		wfclassmenu[pos].text = classmenustring[i];
		wfclassmenu[pos].SelectFunc = WFClassPicked;
		wfclassmenu[pos].arg = i;
		++pos;
	}

	//Clear the rest of the menu
        while (pos <= MAX_CLASSES)
	{
		wfclassmenu[pos].text = "";
		wfclassmenu[pos].SelectFunc = NULL;
		wfclassmenu[pos].arg = 0;
		++pos;
	}
}

//WF
void WFOpenClassMenu(edict_t *ent)
{
	if (ent->client->menu) PMenu_Close(ent);
	WFUpdateClassMenu(ent);

	//Open class menu if classes are on.  Otherwise skip to team menu
	if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
		PMenu_Open(ent, wfclassmenu, -1, sizeof(wfclassmenu) / sizeof(pmenu_t), true, true);
//	else
//		CTFOpenJoinMenu(ent);
}
//WF

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	if (ent->client->menu) PMenu_Close(ent);
	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == CTF_TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), true, true);
}

void CTFCredits(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), false, true);
}

//WF
void WFCredits(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);
	PMenu_Open(ent, wfcreditsmenu, -1, sizeof(wfcreditsmenu) / sizeof(pmenu_t), false, true);
}
//WF

qboolean CTFStartClient(edict_t *ent)
{

//gi.dprintf("DEBUG-1 - Next class = %d\n", ent->client->pers.next_player_class);

	//If CTF is off, use class num to see if this is the first time joining
	if (!ctf->value)
	{
//	if(ent->client->resp.inServer)//acrid 3/99 botcam ,fix this?
//	return false;
//gi.dprintf("DEBUG-2 - Next class = %d\n", ent->client->pers.next_player_class);
		if (ent->client->pers.player_class)
			return false;
//gi.dprintf("DEBUG-3 - Next class = %d\n", ent->client->pers.next_player_class);

		if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
			// start as 'observer'
			ent->movetype = MOVETYPE_NOCLIP;
			ent->solid = SOLID_NOT;
			ent->svflags |= SVF_NOCLIENT;
			//ent->client->resp.ctf_team = CTF_NOTEAM;
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);
			WFOpenClassMenu(ent);
		}
		return true;
	}



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

//WF
		CTFOpenJoinMenu(ent);
		//WFOpenClassMenu(ent);
//WF

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

	//give them the frag limit warning
	if (((int)wfflags->value & WF_MAP_VOTE) &&
	(ctfgame.team1 >= (capturelimit->value - 1) || ctfgame.team2 >= (capturelimit->value-1)) &&
	(maplist.warning_given == false))
	{
		my_bprintf (PRINT_MEDIUM,"-- Capture Limit Warning (1 left): Type 'vote' to pick next map --\n");
		maplist.warning_given = true;
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

	//See if there is a team specific indicator
	if ((self->wf_team) && (self->wf_team != other->wf_team))
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

void WFDisguiseTeam(edict_t *ent, pmenu_t *p);
void WFDisguiseClassCount(edict_t *self, int *classcount);
void WFDisguiseTeamChoose(edict_t *ent, pmenu_t *p);
void WFUpdateDisguiseClassMenu(edict_t *self);
void WFDisguiseClass(edict_t *ent, pmenu_t *p);
void WFDisguise(edict_t *ent, pmenu_t *p);


pmenu_t wfdisguiseclassmenu[] = {
    { "The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
    { WF_VERSION,			PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },
    { "*Select A Class To Disguise As:",		PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },

    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { "Help...",			PMENU_ALIGN_LEFT, 0, WFShowHelp },
    { NULL,					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER selects a class",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC Exits Menu",			PMENU_ALIGN_LEFT, 0, NULL },
};
pmenu_t wfdisguiseteammenu[] = {
    { "The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
    { WF_VERSION,			PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },
    { "*Disguise as Team:",		PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },

    { "1. On",					PMENU_ALIGN_LEFT, 0, WFDisguiseTeamChoose},
    { "2. Off",					PMENU_ALIGN_LEFT, 1, WFDisguiseTeamChoose},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { "Help...",			PMENU_ALIGN_LEFT, 0, WFShowHelp },
    { NULL,					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER selects your choice",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC Exits Menu",			PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t wfdisguisemenu[] = {
    { "The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
    { WF_VERSION,			PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },
    { "*Disguise Team or Class:",		PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },

    { "1. Team",					PMENU_ALIGN_LEFT, 0, WFDisguiseTeam},
    { "2. Class",					PMENU_ALIGN_LEFT, 0, WFDisguiseClass},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { "Help...",			PMENU_ALIGN_LEFT, 0, WFShowHelp },
    { NULL,					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER selects your choice",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC Exits Menu",			PMENU_ALIGN_LEFT, 0, NULL },
};

void WFDisguiseClassCount(edict_t *self, int *classcount)
{
	edict_t	*e;
	int i;
	int j;

	for (j=1; j <= MAX_CLASSES; ++j) classcount[j] = 0;

	//Loop through all the entities to find players on the same team
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (e->client->resp.ctf_team == self->client->resp.next_ctf_team)
			continue;
		++i;

		j = e->client->pers.player_class;
                if (j >= 0 && j <= 9)  ++classcount[j];
	}
}

void WFUpdateDisguiseClassMenu(edict_t *self)
{
	int i;
	int pos;
	int classcount[MAX_CLASSES + 1];

	//sprintf(menutitle,"*Choose a class (%s)",classdefname);

	WFDisguiseClassCount(self, classcount);

	pos = 5;
    for (i = 1; i <= numclasses && i <= MAX_CLASSES; ++i)
	{
		sprintf(disguisemenustring[i], "%d. %s (%d)", i, classinfo[i].name, classcount[i]);
		wfdisguiseclassmenu[pos].text = disguisemenustring[i];
		wfdisguiseclassmenu[pos].SelectFunc = WFDisguise;
		wfdisguiseclassmenu[pos].arg = i;
		++pos;
	}

	//Clear the rest of the menu
        while (pos <= MAX_CLASSES)
	{
		wfdisguiseclassmenu[pos].text = "";
		wfdisguiseclassmenu[pos].SelectFunc = NULL;
		wfdisguiseclassmenu[pos].arg = 0;
		++pos;
	}
}

void WFChooseDisguise(edict_t *ent)
{
//	PMenu_Open(ent, wfdisguisemenu, -1, sizeof(wfdisguisemenu) / sizeof(pmenu_t), true, false);
	WFUpdateDisguiseClassMenu(ent);
	PMenu_Open(ent, wfdisguiseclassmenu, -1, sizeof(wfdisguiseclassmenu) / sizeof(pmenu_t), true, false);
}

void WFDisguiseClass(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);
	if(ent->disguisedteam)
		WFUpdateDisguiseClassMenu(ent);
	PMenu_Open(ent, wfdisguiseclassmenu, -1, sizeof(wfdisguiseclassmenu) / sizeof(pmenu_t), true, false);
}

void WFDisguiseTeam(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	PMenu_Open(ent, wfdisguiseteammenu, -1, sizeof(wfdisguiseteammenu) / sizeof(pmenu_t), true, false);
}

/*=================
CMD_Feign - command to fake death
=================*/
void Cmd_Feign_f (edict_t *ent)
{
	char    *string;

	string=gi.args();

    if (ent->health <=0)
		return;//3/99

	// Make sure player hasn't got the enemy flag
	if (CTFHasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't feign death while holding the flag!\n");
		return;
	}

	if (Q_stricmp ( string, "on") == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Feign On\n");
		ent->client->pers.feign = 1;
	}
	else if (Q_stricmp ( string, "off") == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Feign Off\n");
		ent->client->pers.feign = 0;
	}
	else	//If no "on" or "off", toggle state
	{
		if (ent->client->pers.feign == 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "Feign On\n");
			ent->client->pers.feign = 1;
		}
		else
		{
		safe_cprintf (ent, PRINT_HIGH, "Feign Off\n");
		ent->client->pers.feign = 0;
		}

		if (ent->client->pers.feign == 1)
			feign_on (ent);
		else
			feign_off (ent);
	}
}

/*=================
cmd_Disguise - command to disguise
=================*/
void cmd_Disguise (edict_t *ent)
{
	int		i;
	char	 *s;

	// Make sure player hasn't got the enemy flag
	if (CTFHasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't disguise while holding the flag!\n");
		return;
	}

	i = atoi (gi.argv(1));	//argument = class number
	if (i < 1 || i > numclasses) i = 1;

	ent->disguised = i;
	ent->disguising = i;
	ent->disguisedteam = ent->disguisingteam;
	ent->disguiseshots=1;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);
	safe_cprintf(ent, PRINT_HIGH, "You are disguised.\n");
}

void WFDisguise(edict_t *ent, pmenu_t *p)
{
	char	 *s;

	//ent->disguisetime= level.time+5;
	PMenu_Close(ent);

	// Make sure player hasn't got the enemy flag
	if (CTFHasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't disguise while holding the flag!\n");
		return;
	}

	ent->disguising = p->arg;
	if (ent->disguising)
	{
		ent->disguised = ent->disguising;
		ent->disguisedteam=ent->disguisingteam;
		ent->disguiseshots=1;	//was 4 (G.R.)
		s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
		CTFAssignSkin(ent, s);
		safe_cprintf(ent, PRINT_HIGH, "You are disguised.\n");
	}
}

void WFDisguiseTeamChoose(edict_t *ent, pmenu_t *p)
{
//	char	 *s;
	ent->disguisingteam = p->arg;
	ent->disguisetime= level.time+5;

	PMenu_Close(ent);
}

void WFRemoveDisguise(edict_t *ent)
{
	char *s;

	ent->disguised = 0;
	ent->disguising = 0;
	ent->disguisetime = 0;
	ent->disguisingteam = 0;
	ent->disguisedteam = 0;
	ent->disguiseshots = 0;

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);
	safe_cprintf(ent, PRINT_HIGH, "Disguise Off\n");
}


/*
qboolean WFPickup_Key(edict_t *ent, edict_t *other)
{
	int i;
	edict_t *player;
	gitem_t *key_item;
	qboolean ret;


	//Cant create decoy in observer/spectator mode
	if (other->solid == SOLID_NOT)
	{
		safe_cprintf(other, PRINT_HIGH, "Nice try, but observers can't pick up the flag!.\n");
		return false;
	}

	key_item = ??find key by name??


		other->client->pers.inventory[ITEM_INDEX(key_item)] = 1;
		other->client->resp.ctf_flagsince = level.time;


		// pick up the flag
		// if it's not a dropped flag, we just make is disappear
		// if it's dropped, it will be removed by the pickup caller
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			ent->flags |= FL_RESPAWN;
			ent->svflags |= SVF_NOCLIENT;
			ent->solid = SOLID_NOT;
		}
		return true;

}

static void WFDropKeyTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner &&
		ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void WFDropKeyThink(edict_t *ent)
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
	}
}

int WFHasKey(edict_t *self)
{
	int hasflag;

	hasflag = 0;
	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
		hasflag = 1;
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
		hasflag = 1;

	return (hasflag);
}

// Called from PlayerDie, to drop the flag from a dying player
void WFDeadDropKey(edict_t *self)
{
	edict_t *dropped = NULL;

	if (!flag1_item || !flag2_item)
		CTFInit();

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
	{
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
		flag1dropped = 1;
	}
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
	{
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
		flag2dropped = 1;
	}

	if (dropped) {
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
	}
}

static void WFKeyThink(edict_t *ent)
{
	ent->nextthink = level.time + FRAMETIME;
}


*/
pmenu_t wfMapHelpMenu[] = {
    { "The Weapons Factory",			PMENU_ALIGN_CENTER, 0, NULL },
    { WF_VERSION,			PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },
    { "Map Help",			PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, 0, NULL },

    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL},

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { "Help...",			PMENU_ALIGN_LEFT, 0, WFShowHelp },
    { NULL,					PMENU_ALIGN_CENTER, 0, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ENTER selects your choice",	PMENU_ALIGN_LEFT, 0, NULL },
	{ "ESC Exits Menu",			PMENU_ALIGN_LEFT, 0, NULL },
};

void WFFillMapHelp(edict_t *ent)
{
	wfMapHelpMenu[5].text = level.level_name;
	wfMapHelpMenu[7].text = game.helpmessage1;
	wfMapHelpMenu[9].text = game.helpmessage2;
}

void Cmd_MapHelp_f (edict_t *ent)
{
	if (ent->client->menu)
	{
		PMenu_Close(ent);
		return;
	}

	WFFillMapHelp(ent);
	PMenu_Open(ent, wfMapHelpMenu, -1, sizeof(wfMapHelpMenu) / sizeof(pmenu_t), true, false);
}