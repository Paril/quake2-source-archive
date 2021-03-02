#include "g_local.h"

static gitem_t *flag1_item;
static gitem_t *flag2_item;
static gitem_t *touchdown1;
static gitem_t *touchdown2;
static int team1_captures;
static int team1_scores;
static int team2_scores;
static int team2_captures;

void SP_misc_teleporter_dest (edict_t *ent);
void CTFResetFlags(void);
static void CTFFlagThink(edict_t *ent);
void CTFFlagSetup (edict_t *ent);

#define CTF_CAPTURE_BONUS		level.FLAG_CAPTURE_BONUS	// what you get for capture
#define CTF_TEAM_BONUS			level.FLAG_TEAM_BONUS	// what your team gets for capture
#define CTF_RECOVERY_BONUS		level.FLAG_RECOVERY_BONUS	// what you get for recovery
#define CTF_FLAG_BONUS			level.FLAG_TAKE_BONUS	// what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS	5	// what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME	40	// seconds until auto return
#define CTF_CARRIER_DANGER_PROTECT_BONUS	2	// bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			1	// bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS				1	// bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS		1	// awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS		2	// award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS			400	// the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS			400	// the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10

#define CTF_AUTO_FLAG_RETURN_TIMEOUT		30	// number of seconds before dropped flag auto-returns

#define CTF_TECH_TIMEOUT					60  // seconds before techs spawn again

#define CTF_GRAPPLE_SPEED					650 // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED				650	// speed player is pulled at

void InitFragSystem (void)
{
	CTF_CAPTURE_BONUS = 5;
	CTF_TEAM_BONUS = 10;
	CTF_RECOVERY_BONUS = 5;
	CTF_FLAG_BONUS = 0;
}

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


char *CTFTeamName(int team)
{
	switch (team) {
	case  1:
		return "RED";
	case 2:
		return "BLUE";
	}
	return "UKNOWN";
}

char *CTFOtherTeamName(int team)
{
	switch (team) {
	case 1:
		return "BLUE";
	case 2:
		return "RED";
	}
	return "UKNOWN";
}

int CTFOtherTeam(int team)
{
	switch (team) {
	case 1:
		return 2;
	case 2:
		return 1;
	}
	return -1; // invalid value
}

//////////////////////////////////////////////////////////////
///
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	otherteam = CTFOtherTeam(targ->client->resp.s_team);

	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.s_team == 1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
			CTF_FRAG_CARRIER_BONUS);

		// the the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++) {
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.s_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) {
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		gi.bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.s_team));
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.s_team) {
	case 1:
		c = "item_flag_team1";
		break;
	case 2:
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
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.s_team));
		else
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.s_team));
		return;
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.s_team));
			return;
		}
	}
}

char *Your_Flag (int i)
{
	if (i == 1)
		return "Red Flag";
	else if (i == 2)
		return "Blue Flag";
	else
		return "Flag";
}

char *Enemy_Flag (int i)
{
	if (i == 1)
		return "Blue Flag";
	else if (i == 2)
		return "Red Flag";
	else
		return "Flag";
}

void Print_Msg (edict_t *who, char *msg)
{
	char outmsg[1024];
	char *p;
	edict_t *ent = who;

    outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
		if (*msg == '%')
		{
			switch (*++msg)
			{
				case 'f' :
				case 'F' :
					strcpy(p, Your_Flag(ent->client->resp.s_team));
					p += strlen(Your_Flag(ent->client->resp.s_team));
					break;
				case 'o' :
				case 'O' :
					strcpy(p, Enemy_Flag(ent->client->resp.s_team));
					p += strlen(Enemy_Flag(ent->client->resp.s_team));
					break;
				case 't' :
				case 'T' :
					strcpy(p, CTFTeamName(ent->client->resp.s_team));
					p += strlen (CTFTeamName(ent->client->resp.s_team));
					break;
				case 'e' :
				case 'E' :
					strcpy(p, CTFOtherTeamName(ent->client->resp.s_team));
					p += strlen (CTFOtherTeamName(ent->client->resp.s_team));
					break;
				case 'n' :
				case 'N' :
					strcpy (p, ent->client->pers.netname);
					p += strlen(ent->client->pers.netname);
					break;

				default :
					*p++ = *msg;
			}
		}
		else
			*p++ = *msg;
	}
	*p = 0;

	strcat (outmsg, "\n");
	gi.cprintf(who, PRINT_HIGH, outmsg);
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.s_team == 1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;

	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] &&
		targ->client->resp.s_team != attacker->client->resp.s_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}
/////////////////////////////////////////////////////////

void Flag_Check (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item;
	int ctf_team, i;

	if (other->client)
	{
		// figure out what team this flag is
		
		if ((stricmp(ent->classname, "info_goal_team1") == 0) || (stricmp(ent->classname, "info_touch_team1") == 0))
			ctf_team = 1;
		else if ((stricmp(ent->classname, "info_goal_team2") == 0) || (stricmp(ent->classname, "info_touch_team2") == 0))
			ctf_team = 2;
		else
		{
			gi.cprintf(other, PRINT_HIGH, "Don't know what team the touchdown is on.\n");
			return;
		}

		// same team, if the flag at base, check to he has the enemy flag
		if (ctf_team == 1)
		{
			flag_item = flag1_item;
			enemy_flag_item = flag2_item;
		}
		else
		{
			flag_item = flag2_item;
			enemy_flag_item = flag1_item;
		}

		if (ctf_team == other->client->resp.s_team || ent->mode == 1 /* Have flag */)
		{
			// if the player has the enemy flag, he's just won!

			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
			{

		if (level.capturer_message)
			tprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.capturer_message);
		else
		    tprintf (other, NULL, MOD_CENTRAL, false, "%s captured the enemy's flag!\n",other->client->pers.netname);

        if (level.capture_message)
		    eprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.capture_message);
        else
		    eprintf (other, NULL, MOD_CENTRAL, false, "%s captured your flag!\n",other->client->pers.netname);

		if (ent->target)
		{
			edict_t *target;

			target = G_Find (NULL, FOFS(targetname), ent->target);

			if (target != NULL)
				target->speed = 1;
		}

		if (!ent->mode)
			other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;
		else
			ent->mode = 0;

		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("teams/flagcap.wav"), 1, ATTN_NONE, 0);

		if (other->client->resp.s_team == 1)
			team1_captures ++;
		else if (other->client->resp.s_team == 2)
			team2_captures ++;

		// other gets another 10 frag bonus
		other->client->resp.score += CTF_CAPTURE_BONUS;

		// Ok, let's do the player loop, hand out the bonuses
		for (i = 1; i <= maxclients->value; i++)
		{
			player = &g_edicts[i];
			if (!player->inuse)
				continue;

	     	if (player->client->resp.s_team != other->client->resp.s_team)
				player->client->resp.ctf_lasthurtcarrier = -5;
			else if (player->client->resp.s_team == other->client->resp.s_team)
			{
				player->client->resp.score += CTF_TEAM_BONUS;
				// award extra points for capture assists
				if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
				{
						gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
						player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
				}
				if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
				{
					gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
					player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
				}
			}
		} 

		CTFResetFlags();
        return;
	}
}
	}
	else
		return;
}

static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}

void SP_info_goal_team1 (edict_t *self)
{
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, self->mins);
	v = tv(15,15,15);
	VectorCopy (v, self->maxs);

	self->s.skinnum = 0;
	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_TOSS;  
	self->s.effects = EF_TRANSPARENT;
	self->touch = Flag_Check;
	self->think = CTFFlagThink;
	self->nextthink = level.time + FRAMETIME;

	if (self->model)
		gi.setmodel (self, self->model);

	gi.linkentity (self);
	return;
}

void SP_info_goal_team2 (edict_t *self)
{
       SP_info_goal_team1 (self);
}

void CTFInit(void)
{
   if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");

   if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");

}


void CTFResetFlag (int ctf_steam)
{
	char *c;
	edict_t *ent;

	switch (ctf_steam)
	{
		case 1:
		c = "item_flag_team1";
		break;

		case 2:
		c = "item_flag_team2";
		break;

		default:
			return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL)
	{
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags(void)
{
	CTFResetFlag(1);
	CTFResetFlag(2);
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	int i;
	edict_t *player, *touchdown;
	gitem_t *flag_item, *enemy_flag_item;

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = 1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = 2;
	else {
		gi.cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == 1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
		touchdown = G_Find (NULL, FOFS(classname), "info_goal_team1");
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
		touchdown = G_Find (NULL, FOFS(classname), "info_goal_team2");
	}

	if (ctf_team == other->client->resp.s_team)
	{

		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			// the flag is at home base.  if the player has the enemy
			// flag, he's just won!
		
			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
			{
					if (touchdown == NULL)
					{
						if (level.capturer_message)
							tprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n", other->client->pers.netname, level.capturer_message);
						else
							tprintf (other, NULL, MOD_CENTRAL, false, "%s captured the enemy's flag!\n",other->client->pers.netname);

						if (level.capture_message)
							eprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.capture_message);
						else
							eprintf (other, NULL, MOD_CENTRAL, false, "%s captured your flag!\n",other->client->pers.netname);

						other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

						if (other->client->resp.s_team == 1)
							team1_captures ++;
						else if (other->client->resp.s_team == 2)
							team2_captures ++;

						gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("teams/flagcap.wav"), 1, ATTN_NONE, 0);

						// other gets another 10 frag bonus
						other->client->resp.score += CTF_CAPTURE_BONUS;

						// Ok, let's do the player loop, hand out the bonuses

						for (i = 1; i <= maxclients->value; i++)
						{
							player = &g_edicts[i];

							if (!player->inuse)
								continue;

							if (player->client->resp.s_team != other->client->resp.s_team)
								player->client->resp.ctf_lasthurtcarrier = -5;
							else if (player->client->resp.s_team == other->client->resp.s_team)
							{
								player->client->resp.score += CTF_TEAM_BONUS;
								// award extra points for capture assists
								
								if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
								{
									gi.bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
									player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
								}

								if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
								{
									gi.bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
									player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
								}
							}
						}
						
						CTFResetFlags();
						return false;
					}

					return false; // its at home base already
			}
		}
		return false;
	}

	// hey, its not our flag, pick it up
	if (level.taker_message)
		tprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.taker_message);
	else
	    tprintf (other, NULL, MOD_CENTRAL, false, "%s got the enemy's flag!\n",other->client->pers.netname);

	if (level.take_message)
	    eprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.take_message);
    else
	    eprintf (other, NULL, MOD_CENTRAL, false, "%s got your flag!\n",other->client->pers.netname);

	if (ent->message)
		gi.centerprintf (other, ent->message);

	gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("world/x_alarm.wav"), 1, ATTN_NONE, 0);
	other->client->resp.score += CTF_FLAG_BONUS;

	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}

void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner && 
		ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		return;

	Touch_Item (ent, other, plane, surf);
}

void CTFDropFlagThink(edict_t *ent)
{
	// auto return the flag
	// reset flag will remove ourselves
	if (ent->classname == "item_flag_team1")
	{
		CTFResetFlag(1);

		if (level.return_message)
			gi.bprintf (PRINT_HIGH, "The %s %s\n", CTFTeamName(1), level.return_message);
		else
			gi.bprintf (PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(1));
	}
    else if (ent->classname == "item_flag_team2")
	{
		CTFResetFlag(2);

		if (level.return_message)
			gi.bprintf (PRINT_HIGH, "The %s %s\n", CTFTeamName(2), level.return_message);
		else
			gi.bprintf (PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(2));
	}
	else
	{
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

// Called from PlayerDie, to drop the flag from a dying player

void CTFFlagSetup (edict_t *ent)
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
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);

	if (ent->model == NULL)
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = CTFFlagThink;
	}
}

void CTFEffects(edict_t *player)
{
	player->s.effects &= (EF_FLAG1 | EF_FLAG2);

	if (player->health > 0)
	{
		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			player->s.effects |= EF_FLAG1;
		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			player->s.effects |= EF_FLAG2;

		if (player->mode)
			player->s.effects |= (EF_FLAG1 | EF_FLAG2);
	}

	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
		player->s.modelindex3 = gi.modelindex ("players/male/flag1.md2");
	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");
	// wait
    else
		player->s.modelindex3 = 0;
}

void Flag_StatusReport (edict_t *self)
{
	edict_t *flag, *other;
	int i;

	flag = G_Find (NULL, FOFS(classname), "item_flag");

    if (!self->client)
		return;
	else if (!self->inuse)
		return;
	else if (!self->client->resp.s_team)
		return;
	else if (flag != NULL)
		return;

    if (self->client->resp.s_team == 1)
		flag = flag1_entity;
	else if (self->client->resp.s_team == 2)
		flag = flag2_entity;

    for_each_player (other, i)
	{
		if (flag)
		{
			if (flag->spawnflags & DROPPED_ITEM)
			{
				if (level.flag_lying)
       				Print_Msg (self, level.flag_lying);
				else
					gi.cprintf (self, PRINT_HIGH, level.flag_lying);
			}
            else if (!(flag->spawnflags & DROPPED_ITEM) & !(other->client->pers.inventory[ITEM_INDEX(FindItemByClassname(flag->classname))]))
			{
				if (level.flag_at_base)
					Print_Msg (self, level.flag_at_base);
				else
       				gi.cprintf (self, PRINT_HIGH, "Your flag is at the base\n");
			}
			else if (other->client->pers.inventory[ITEM_INDEX(FindItemByClassname(flag->classname))])
			{
				if (level.flag_own)
					Print_Msg (self, level.flag_own);
				else
					gi.cprintf (self, PRINT_HIGH, "Your flag is being carried by %s\n", other->client->pers.netname);
			}
			else 
			{
				if (level.flag_corrupt)
					Print_Msg (self, level.flag_corrupt);
				else
					gi.cprintf (self, PRINT_HIGH, "Your flag is corrupt\n");
			}
		}
		else
		    gi.cprintf (self, PRINT_HIGH, "This map doesn't have the %s\n",Your_Flag (self->client->resp.s_team));
	}
}

void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;

	if (!self->client->resp.s_team)
		return;

	if (self->client->pers.inventory[ITEM_INDEX(FindItem(Enemy_Flag(self->client->resp.s_team)))])
	{
		if (!self->mode)
		{
			dropped = Drop_Item(self, FindItem(Enemy_Flag(self->client->resp.s_team)));
			self->client->pers.inventory[ITEM_INDEX(FindItem(Enemy_Flag(self->client->resp.s_team)))] = 0;
      	    gi.bprintf(PRINT_HIGH, "%s lost the %s!\n", self->client->pers.netname, Enemy_Flag(self->client->resp.s_team));
		}
		else
		{
			dropped = Drop_Item(self, FindItem("Flag"));
			self->mode = 0;
      	    gi.bprintf(PRINT_HIGH, "%s lost the Flag!\n", self->client->pers.netname);
		}
 	}

	if (dropped)
	{
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
	}
}

void RipDropFlag (edict_t *ent, gitem_t *item)
{
	int		index;
	entity *dropped;
 
    if (!((int)ripflags->value & RF_DROP_FLAG))
           return;

	index = ITEM_INDEX (item);
	if (!ent->mode)
		gi.bprintf(PRINT_HIGH, "%s dropped the %s!\n", ent->client->pers.netname, Enemy_Flag(ent->client->resp.s_team));
	else
		gi.bprintf(PRINT_HIGH, "%s dropped the Flag!\n", ent->client->pers.netname);

    dropped = Drop_Item(ent, item);
	ent->client->pers.inventory[index] = 0;

	if (dropped)
	{
		ent->mode = 0;
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
	}
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
	int		len;
    int	    score, total[2], totalscore[2];
    int	    last[2];
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;


	if (ent -> client -> showscores)
	{

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.s_team == 2)
			team = 0;
		else if (game.clients[i].resp.s_team == 1)
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
	sprintf(string, "if 27 xv 8 yv 8 pic 27 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 30 "
		"if 28 xv 168 yv 8 pic 28 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
        "xv 256 yv 12 num 2 31 ",
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
		if (i < total[0])
		{
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

#if 0 //ndef NEW_SCORE

			sprintf(entry+strlen(entry),
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else
			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);

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
			"xv 0 %s \"%3d %3d %-12.12s\" ",
			(cl_ent == ent) ? "string2" : "string",
			cl->resp.score, 
			(cl->ping > 999) ? 999 : cl->ping, 
			cl->pers.netname);
#else

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				cl->resp.score,
				cl->ping > 999 ? 999 : cl->ping);
#endif
			if (maxsize - len > strlen(entry))
			{
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
				cl_ent->client->resp.s_team != 0)
				continue;

			if (!k)
			{
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

// added ...
	}
	else
		*string = 0;

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
/*
	fp = fopen ("stats.html", "w");

    fprintf (fp, "<html>\n");
    fprintf (fp, "<head>\n");
    fprintf (fp, "<meta http-equiv=\"Content-Type\"\n");
    fprintf (fp, "content= \"text/html; charset=windows-1251\"");
	fprintf (fp, ">\n");
    fprintf (fp, "<meta name=\"GENERATOR\"\n");
    fprintf (fp, "content=\"Quake 2\"");
	fprintf (fp, ">\n");
	fprintf (fp, "<title>Current Statistics</title>\n");
	fprintf (fp, "</head>\n");
	fprintf (fp, "\n");
    fprintf (fp, "<body bgcolor=\"#000000\"\n");
    fprintf (fp, "text=\"#FFFFFF\"");
    fprintf (fp, ">\n");
    fprintf (fp, "<h1 align=\"center\"");
    fprintf (fp, ">\n");
    fprintf (fp, "<strong>Current Statistics</strong></h1>\n");
    fprintf (fp, "<p align=\"center\"");
    fprintf (fp, ">");
    fprintf (fp, "<font color =\"#0000FF\"");
    fprintf (fp, ">\n");
    fprintf (fp, "<strong>Blue Team: %4d/%-3d </strong></font></p>\n", totalscore[0], total[0]);
    fprintf (fp, "<p align=\"center\"");
    fprintf (fp, ">");
	fprintf (fp, "<font color =\"#FF0000\"");
    fprintf (fp, ">\n");
    fprintf (fp, "<strong>Red Team: %4d/%-3d </strong></font></p>", totalscore[1], total[1]);
    fprintf (fp, "\n</body>\n");
	fprintf (fp, "</html>");
	fclose (fp); */
}

char *ctf_statusbar =
"yb	-24 "

"if 1 "
// health
"xv	0 "
"hnum "
"endif "
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
"if 24 "
"xr -33 "
"yt 1 "
"pic 24 "
"endif "

"xr	-50 "
"yt 34 "
"num 3 14 "

// have flag graph
"if 16 "
  "yt 67 "
  "xr -23 "
  "pic 16 "
"endif "

"if 17 "
"	yb	-120 "		// New
"	xr	-75 "		// New
"	num	3	17 "	// New. Display 2-digits with value from stat-array at index 17
"	xr	-24 "		// New
"	pic	0 "		// New. Display icon
"endif "

"if 18 "
"	yb	-48 "		// New
"	xr	-75 "		// New
"	num	3	18 "	// New. Display 2-digits with value from stat-array at index 17
"	xr	-24 "		// New
"	pic	19 "		// New. Display icon
"endif "

"if 20 "
"	yb	-72 "		// New
"	xr	-75 "		// New
"	num	3	20 "	// New. Display 2-digits with value from stat-array at index 17
"	xr	-24 "		// New
"	pic	21 "		// New. Display icon
"endif "

"if 22 "
"	yb	-96 "		// New
"	xr	-75 "		// New
"	num	3	22 "	// New. Display 2-digits with value from stat-array at index 17
"	xr	-24 "		// New
"	pic	23 "		// New. Display icon
"endif "

"if 25 "
" yb -174 "		// New
" xr -75 "		// New
" num 3	25 "	// New. Display 2-digits with value from stat-array at index 17
" xr -24 "		// New
" pic 26 "		// New. Display icon
"endif "

"if 29 "
" yb -144 "		// New
" xr -19"		// New
" num 1	29 "	// New. Display 2-digits with value from stat-array at index 17
"endif "
;

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct
{
	char *classname;
	int priority;
}

loc_names[] = 
{
	{   "info_position",            1 },
	{	"item_flag_team1",			2 },
	{	"item_flag_team2",		    2 },
	{	"item_quad",				3 },
	{	"item_invulnerability",		4 },
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

vec_t VectorLengthSqr(vec3_t v)
{
	int		i;
	float	length;
	
	length = 0.0f;
	for (i=0; i<3; i++)
		length += v[i]*v[i];
	
	return length;
}

static void CTFSay_Team_Location(edict_t * who, char *buf)
{
    edict_t *what = NULL;
    edict_t *hot = NULL;
    float hotdist = 222222222.2, newdist;
    vec3_t v;
    int hotindex = 999;
    int i;
    gitem_t *item;
    int nearteam = -1;
    edict_t *flag1, *flag2;
    qboolean hotsee = false;
    qboolean cansee;
    int ibuf = 0;
	
    while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (!strcmp(what->classname, loc_names[i].classname))
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
				hotdist = VectorLengthSqr(v);
				continue;
			}
			// if we can't see this, but we have something we can see, skip it
			if (hotsee && !cansee)
				continue;
			if (hotsee && hotindex < loc_names[i].priority)
				continue;
			VectorSubtract(what->s.origin, who->s.origin, v);
			newdist = VectorLengthSqr(v);
			if (newdist < hotdist ||
				(cansee && loc_names[i].priority < hotindex)) {
				hot = what;
				hotdist = newdist;
				hotindex = i;
				hotsee = loc_CanSee(hot, who);
			}
    }
	
    if (!hot) {
		memcpy(buf, "nowhere", 8);
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
			hotdist = VectorLengthSqr(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLengthSqr(v);
			if (hotdist < newdist)
				nearteam = 1;
			else if (hotdist > newdist)
				nearteam = 2;
		}
		break;
    }
	
    if ((item = FindItemByClassname(hot->classname)) == NULL) {
		memcpy(buf, "nowhere", 8);
		return;
    }
	
    // in water?
    if (who->waterlevel) {
		memcpy(buf, "in the water ", 13);
		ibuf = 13;
    }
    else
		*buf = 0;
	
    // near or above
    VectorSubtract(who->s.origin, hot->s.origin, v);
    if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1])) {
		if (v[2] > 0) {
			memcpy(buf + ibuf, "above ", 6);
			ibuf += 6;
		}
		else {
			memcpy(buf + ibuf, "below ", 6);
			ibuf += 6;
		}
    }
    else {
		memcpy(buf + ibuf, "near ", 5);
		ibuf += 5;
    }
	
    if (nearteam == 1) {
		memcpy(buf + ibuf, "the red ", 8);
		ibuf += 8;
    }
    else if (nearteam == 2) {
		memcpy(buf + ibuf, "the blue ", 9);
		ibuf += 9;
    }
    else {
		memcpy(buf + ibuf, "the ", 4);
		ibuf += 4;
    }
	
    strcpy(buf + ibuf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;
	
	*buf = 0;
	
	power_armor_type = PowerArmorType(who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
			(power_armor_type == POWER_ARMOR_SCREEN) ?
			"Power Screen" : "Power Shield", cells);
	}
	
	index = ArmorIndex(who);
	if (index)
	{
		item = GetItemByIndex(index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}
	
    if (!*buf)
		memcpy(buf, "no armor", 9);
}

static void CTFSay_Team_Health(edict_t * who, char *buf)
{
    if (who->health <= 0)
		memcpy(buf, "dead", 5);
    else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_SightEnemy(edict_t *who, char *buf)
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
			!loc_CanSee(targ, who) || who->client->resp.s_team == targ->client->resp.s_team)
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
		strcpy(buf, "no enemy");
}

static void CTFSay_Team_SightTeammate(edict_t *who, char *buf)
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
			!loc_CanSee(targ, who) || who->client->resp.s_team != targ->client->resp.s_team)
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
		strcpy(buf, "nobody");
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		memcpy(buf, "none", 5);
}
static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];
	
	*s = *s2 = 0;
	for (i = 1; i <= game.maxclients; i++) {
		targ = g_edicts + i;
		if (!targ->inuse ||
			targ->svflags & SVF_NOCLIENT ||
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
	}
	else
		memcpy(buf, "no one", 7);
}

void CTFParseTeamMessage(edict_t * who, char *msg, char *outmsg, int size)
{
    char buf[1024];
    char *p;
	int len;
	
    outmsg[0] = 0;
	
    if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
    }
	
    for (p = outmsg; *msg && (p - outmsg) < size - 1; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
			case 'l':
			case 'L':
				CTFSay_Team_Location(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 'a':
			case 'A':
				CTFSay_Team_Armor(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 'h':
			case 'H':
				CTFSay_Team_Health(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 't':
			case 'T':
				CTFSay_Team_SightTeammate(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 'e':
			case 'E':
				CTFSay_Team_SightEnemy(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 'w':
			case 'W':
				CTFSay_Team_Weapon(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case 'n':
			case 'N':
				CTFSay_Team_Sight(who, buf);
				len = strlen(buf);
				if (p-outmsg+len < size) {
					memcpy(p, buf, len);
					p += len;
				}
				break;
			case '%':
				*p++ = '%';
				break;
			case '\0':
				--msg;
				break;
			default:
				break;
			}
		}
		else
			*p++ = *msg;
    }
    *p = 0;
}


void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	int i;
	edict_t *cl_ent;
	//flood protection code
    int time_left=0; 

// Begin - Message Flooding Protection Code. 

// if not already in a timer situation then.. 
	if (!(who->client->flood_timer > PRESENT_TIME)) 
	// if attempt to post more than 3 msgs in 1 unit of time. 
	if ((who->client->flood_num_msgs >= 3) && (who->client->flood_post_time + 1 > PRESENT_TIME))
	{
		who->client->flood_timer = PRESENT_TIME + 30; // Start 30 sec Timer. 
		who->client->flood_post_time = PRESENT_TIME; // Reset First Post Time. 
		who->client->flood_num_msgs = 0;
	} // Reset Num Messages posted 

	// Timer is running so NO POSTS ALLOWED. Just printf and exit.. 

    if (who->client->flood_timer > PRESENT_TIME)
	{
		time_left = (int)(who->client->flood_timer-PRESENT_TIME); 
		gi.cprintf(who, PRINT_HIGH, "You can't talk for %i more seconds..\n",time_left); 
		return;
	} 

// End - Message Flood Protection 

	CTFParseTeamMessage(who, msg, outmsg, sizeof (outmsg));

	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (!cl_ent->client)
			continue;

		if (cl_ent->client->resp.s_team == who->client->resp.s_team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

qboolean sentry_valid (edict_t *ent)
{
	return (G_EntExists(ent->sentry) && ent->playerclass == 6);
}

void RIP_GetStats (edict_t *ent)
{
    int	    score, total[2], totalscore[2];
    int	    last[2];
	int		i, j, k;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
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
		if (game.clients[i].resp.s_team == 2)
			team = 0;
		else if (game.clients[i].resp.s_team == 1)
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

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("team1");
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("team2");

	if (sentry_valid(ent))
	{
		ent->client->ps.stats[STAT_SENTRY_HEALTH] = ent->sentry->health;
		ent->client->ps.stats[STAT_SENTRY_SLUGS] = ent->sentry->slugs;
		ent->client->ps.stats[STAT_SENTRY_ROCKETS] = ent->sentry->rockets;
		ent->client->ps.stats[STAT_SENTRY_BULLETS] = ent->sentry->bullets;
		ent->client->ps.stats[STAT_SENTRY_LEVEL] = ent->sentry->playerclass;
	}
	else
	{
		if (ent->playerclass == 9)
			ent->client->ps.stats[STAT_SENTRY_SLUGS] = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		else
		    ent->client->ps.stats[STAT_SENTRY_SLUGS] = 0;

		ent->client->ps.stats[STAT_SENTRY_HEALTH] = 0;
		ent->client->ps.stats[STAT_SENTRY_ROCKETS] = 0;
	    ent->client->ps.stats[STAT_SENTRY_BULLETS] = 0;
      	ent->client->ps.stats[STAT_SENTRY_LEVEL] = 0;
	}
	
	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (totalscore[0] > totalscore[1]) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (totalscore[1] > totalscore[0]) 
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}

	ent->client->ps.stats[STAT_HAVE_FLAG] = 0;

	if (ent->client->resp.s_team)
	{
		gitem_t *fl_ind = FindItem(Enemy_Flag(ent->client->resp.s_team));

		if (level.framenum & 8)
		{
			if (ent->client->pers.inventory[ITEM_INDEX(fl_ind)])
				ent->client->ps.stats[STAT_HAVE_FLAG] = gi.imageindex(fl_ind->icon);
		}
	}

	ent->client->ps.stats[STAT_FRAGS_ICON] = ent->playerclass;

	if (ent->playerclass == 6)
		ent->client->ps.stats[STAT_SENTRY_SLUGS_PIC] = gi.imageindex("a_slugs");
	else
		ent->client->ps.stats[STAT_SENTRY_SLUGS_PIC] = gi.imageindex("a_cells");

	ent->client->ps.stats[STAT_SENTRY_ROCKETS_PIC] = gi.imageindex("a_rockets");
	ent->client->ps.stats[STAT_SENTRY_BULLETS_PIC] = gi.imageindex("a_bullets");
	ent->client->ps.stats[STAT_DMG] = ent->client->SonicDamage;
	ent->client->ps.stats[STAT_DMG_ICON] = gi.imageindex("w_railgun");
	ent->client->ps.stats[STAT_TEAM1_CAPTURES] = team2_captures;
	ent->client->ps.stats[STAT_TEAM2_CAPTURES] = team1_captures;
}

void Captures_Reached (void){
	if (capturelimit->value == team1_captures || capturelimit->value == team2_captures)
	{
		gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		EndDMLevel ();
		return;
	}
}

/*
===========================
Concussion Grenades
===========================
*/
void Concussion_Explode (edict_t *ent)
{
    vec3_t      offset,v;
    edict_t *target;
	float Distance, DrunkTimeAdd;	

    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);    
    VectorAdd(ent->s.origin, offset, ent->s.origin);

    if (ent->owner->client)
       PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    target = NULL;

    while ((target = findradius(target, ent->s.origin, 520)) != NULL)
    {
		if (!target->client)
			continue;
		if (target->client->resp.s_team == ent->owner->client->resp.s_team)
			continue;

        if (!loc_CanSee(ent, target))
            continue;       // The player can't see it

		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);
		// Calculate drunk factor
		if(Distance < 520/10)
			DrunkTimeAdd = 20; //completely drunk
        else
            DrunkTimeAdd = 1.5 * 20 * ( 1 / ( ( Distance - 520*2 ) / (520*2) - 2 ) + 1 ); //partially drunk
        if ( DrunkTimeAdd < 0 )
            DrunkTimeAdd = 0; // Do not make drunk at all.
    
              
        // Increment the drunk time
        if(target->DrunkTime < level.time)
			target->DrunkTime = DrunkTimeAdd+level.time;
		else
			target->DrunkTime += DrunkTimeAdd;               
	}

   // Blow up the grenade
   G_PointEntity(TE_BOSSTPORT, ent->s.origin, MULTICAST_PHS);
   
   G_FreeEdict (ent);
}

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

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;
}