#include "g_local.h"
#include "m_player.h"
#include "stdlog.h"
#include "gslog.h"

#define	nteam	5
#define	game_loop	for (i = 0; i < maxclients->value; i++)
#define	team_loop	for (i = red; i < none; i++)
#define	_team_loop	for (i = red; i <= none; i++)
#define	map_loop	for (i = 0; i < 64; i++)
#define	far_off	100000000

#define	hook_on	0x00000001
#define	hook_in	0x00000002
#define	shrink_on	0x00000004
#define	grow_on	0x00000008
#define	motor_off	0
#define	motor_start	1
#define	motor_on	2
#define	chan_hook	5

#define	_drophook	"medic/medatck5.wav"
#define	_motorstart	"parasite/paratck2.wav"
#define	_motoron	"parasite/paratck3.wav"
#define	_motoroff	"parasite/paratck4.wav"
#define	_hooktouch	"parasite/paratck1.wav"
#define	_touchsolid	"medic/medatck3.wav"
#define	_firehook	"medic/medatck2.wav"

#define	stat_identify	18
#define	stat_red	19
#define	stat_red_arrow	23

#define	_shotgun	0x00000001 // 1
#define	_supershotgun	0x00000002 // 2
#define	_machinegun	0x00000004 // 4
#define	_chaingun	0x00000008 // 8
#define	_grenadelauncher	0x00000010 // 16
#define	_rocketlauncher	0x00000020 // 32
#define	_hyperblaster	0x00000040 // 64
#define	_railgun	0x00000080 // 128

#define	ready_help	0x00000001
#define	thaw_help	0x00000002
#define	frozen_help	0x00000004
#define	chase_help	0x00000008

#define	is_motd	0x00000001
#define	end_vote	0x00000002
#define	mapnohook	0x00000004
#define	everyone_ready 0x00000008

cvar_t*	item_respawn_time;
cvar_t*	hook_max_len;
cvar_t*	hook_rpf;
cvar_t*	hook_min_len;
cvar_t*	hook_speed;
cvar_t*	point_limit;
cvar_t*	new_team_count;
cvar_t*	frozen_time;
cvar_t*	start_weapon;
cvar_t*	start_armor;
cvar_t*	random_map;
cvar_t*	vote_percent;
cvar_t*	use_ready;
cvar_t*	grapple_wall;
static char*	freeze_skin[] = {"ctf_r", "ctf_b", "ctf_g", "ctf_y", "ctf_r"};
static char*	freeze_team_[] = {"RED", "BLUE", "GREEN", "YELLOW", "NONE"};
static char*	freeze_team__[] = {"Red", "Blu", "Grn", "Ylw", "Non"};
static int	gib_queue;
static int	team_max_count;
static int	moan[8];
static int	lame_hack;
static float	ready_time;

static qboolean floodCheck(edict_t* ent)
{
	gclient_t*	cl;
	int	i;
	if (flood_msgs->value)
	{
		cl = ent->client;
		if (level.time < cl->flood_locktill)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n", (int)(cl->flood_locktill - level.time));
			return true;
		}
		i = cl->flood_whenhead - flood_msgs->value + 1;
		if (i < 0)
			i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && level.time - cl->flood_when[i] < flood_persecond->value)
		{
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "You can't talk for %d seconds.\n", (int)flood_waitdelay->value);
			return true;
		}
		cl->flood_whenhead = (cl->flood_whenhead + 1) %(sizeof(cl->flood_when) / sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	return false;
}

static void putInventory(char* s, edict_t* ent)
{
	gitem_t*	item = NULL;
	gitem_t*	ammo = NULL;
	int	index;

	item = FindItem(s);
	if (item)
	{
		index = ITEM_INDEX(item);
		ent->client->pers.inventory[index] = 1;

		ammo = FindItem(item->ammo);
		if (ammo)
		{
			index = ITEM_INDEX(ammo);
			ent->client->pers.inventory[index] = ammo->quantity;
		}
		ent->client->newweapon = item;
	}
}

void playerWeapon(edict_t* ent)
{
	gitem_t*	item = FindItem("blaster");

	if (!ent->client->resp.spectator)
	{
		freeze[ent->client->resp.team].ready = false;
		ready_time = far_off;
	}

	item = FindItem("blaster");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	ent->client->newweapon = item;

	if (start_armor->value)
	{
		int	index = ITEM_INDEX(FindItem("jacket armor"));
		ent->client->pers.inventory[index] = (int)(start_armor->value / 2) * 2;
	}

	if (start_weapon->value)
	{
		if ((int)start_weapon->value &_shotgun)
			putInventory("shotgun", ent);
		if ((int)start_weapon->value &_supershotgun)
			putInventory("super shotgun", ent);
		if ((int)start_weapon->value &_machinegun)
			putInventory("machinegun", ent);
		if ((int)start_weapon->value &_chaingun)
			putInventory("chaingun", ent);
		if ((int)start_weapon->value &_grenadelauncher)
			putInventory("grenade launcher", ent);
		if ((int)start_weapon->value &_rocketlauncher)
			putInventory("rocket launcher", ent);
		if ((int)start_weapon->value &_hyperblaster)
			putInventory("hyperblaster", ent);
		if ((int)start_weapon->value &_railgun)
			putInventory("railgun", ent);
	}
	ChangeWeapon(ent);
}

void cmdChange(edict_t* ent)
{
	char*	s;
	int	team = none;

	s = gi.argv(1);
	if (!*s || ent->client->frozen)
	{
		gi.cprintf(ent, PRINT_HIGH, "You are on the %s team.\n", freeze_team[ent->client->resp.team]);
		return;
	}
	if (Q_stricmp(s, "red") == 0)
		team = red;
	else if (Q_stricmp(s, "blue") == 0)
		team = blue;
	else if (Q_stricmp(s, "green") == 0 && team_max_count >= 3)
		team = green;
	else if (Q_stricmp(s, "yellow") == 0 && team_max_count >= 4)
		team = yellow;
	else if (Q_stricmp(s, "none") == 0 && !ent->client->resp.spectator)
	{
		gi.WriteByte(svc_stufftext);
		gi.WriteString("spectator 1\n");
		gi.unicast(ent, true);
		freeze[ent->client->resp.team].update = true;
		return;
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", s);
		return;
	}
	if (ent->client->resp.spectator)
	{
		if (ent->client->resp.team == none)
		{
			gi.WriteByte(svc_stufftext);
			gi.WriteString("alias +hook \"hook action;wait;hook shrink\"\nalias -hook \"hook stop\"\n");
			gi.unicast(ent, true);
			gi.WriteByte(svc_stufftext);
			gi.WriteString("alias +shrink \"hook shrink\"\nalias -shrink \"hook stop\"\n");
			gi.unicast(ent, true);
			gi.WriteByte(svc_stufftext);
			gi.WriteString("alias +grow \"hook grow\"\nalias -grow \"hook stop\"\n");
			gi.unicast(ent, true);
			sl_LogPlayerName(&gi, ent->client->pers.netname, freeze_team[team], level.time);
			ent->client->pers.spectator = false;
		}
		else
			sl_LogPlayerTeamChange(&gi, ent->client->pers.netname, freeze_team[team], level.time);
		gi.WriteByte(svc_stufftext);
		gi.WriteString("spectator 0\n");
		gi.unicast(ent, true);
		freeze[ent->client->resp.team].update = true;
		ent->client->resp.team = team;
	}
	else if (ent->client->resp.team == team)
	{
		gi.cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n", freeze_team[ent->client->resp.team]);
		return;
	}
	else
	{
		freeze[ent->client->resp.team].update = true;
		ent->client->resp.team = team;
		ent->flags &= ~FL_GODMODE;
		ent->health = 0;
		meansOfDeath = MOD_SUICIDE;
		player_die(ent, ent, ent, 100000, vec3_origin);
		gi.bprintf(PRINT_HIGH, "%s changed to the %s team.\n", ent->client->pers.netname, freeze_team[team]);
		sl_LogPlayerTeamChange(&gi, ent->client->pers.netname, freeze_team[team], level.time);
	}
}

qboolean playerDamage(edict_t* targ, edict_t* attacker, int damage)
{
	if (!targ->client)
		return false;
	if (meansOfDeath == MOD_TELEFRAG)
		return false;
	if (!attacker->client)
		return false;
	if (targ->client->hookstate && random() < 0.2)
		targ->client->hookstate = 0;
	if (targ->health > 0)
	{
		if (!(lame_hack &everyone_ready))
		{
			if (!(attacker->client->resp.help &ready_help))
			{
				attacker->client->showscores = false;
				attacker->client->resp.help |= ready_help;
				gi.centerprintf(attacker, "Waiting for everyone to be ready.");
				gi.sound(attacker, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_STATIC, 0);
			}
			return true;
		}
		if (targ == attacker)
			return false;
		if (targ->client->resp.team != attacker->client->resp.team && targ->client->respawn_time + 3 < level.time)
			return false;
	}
	else
	{
		if (targ->client->frozen)
		{
			if (random() < 0.1)
				ThrowGib(targ, "models/objects/debris2/tris.md2", damage, GIB_ORGANIC);
			return true;
		}
		else
			return false;
	}
	if ((int)(dmflags->value) &DF_NO_FRIENDLY_FIRE)
		return true;
	meansOfDeath |= MOD_FRIENDLY_FIRE;
	return false;
}

qboolean freezeCheck(edict_t* ent)
{
	if (ent->deadflag)
		return false;
	if (meansOfDeath &MOD_FRIENDLY_FIRE)
		return false;
	switch (meansOfDeath)
	{
	case MOD_FALLING:
	case MOD_SLIME:
	case MOD_LAVA:
		if (random() < 0.08)
			break;
	case MOD_SUICIDE:
	case MOD_CRUSH:
	case MOD_WATER:
	case MOD_EXIT:
	case MOD_TRIGGER_HURT:
	case MOD_BFG_LASER:
	case MOD_BFG_EFFECT:
	case MOD_TELEFRAG:
		return false;
	}
	return true;
}

void freezeAnim(edict_t* ent)
{
	ent->client->anim_priority = ANIM_DEATH;
	if (ent->client->ps.pmove.pm_flags &PMF_DUCKED)
	{
		if (rand() &1)
		{
			ent->s.frame = FRAME_crpain1 - 1;
			ent->client->anim_end = FRAME_crpain1 + rand() %4;
		}
		else
		{
			ent->s.frame = FRAME_crdeath1 - 1;
			ent->client->anim_end = FRAME_crdeath1 + rand() %5;
		}
	}
	else
	{
		switch (rand() %8)
		{
		case 0:
			ent->s.frame = FRAME_run1 - 1;
			ent->client->anim_end = FRAME_run1 + rand() %6;
			break;
		case 1:
			ent->s.frame = FRAME_pain101 - 1;
			ent->client->anim_end = FRAME_pain101 + rand() %4;
			break;
		case 2:
			ent->s.frame = FRAME_pain201 - 1;
			ent->client->anim_end = FRAME_pain201 + rand() %4;
			break;
		case 3:
			ent->s.frame = FRAME_pain301 - 1;
			ent->client->anim_end = FRAME_pain301 + rand() %4;
			break;
		case 4:
			ent->s.frame = FRAME_jump1 - 1;
			ent->client->anim_end = FRAME_jump1 + rand() %6;
			break;
		case 5:
			ent->s.frame = FRAME_death101 - 1;
			ent->client->anim_end = FRAME_death101 + rand() %6;
			break;
		case 6:
			ent->s.frame = FRAME_death201 - 1;
			ent->client->anim_end = FRAME_death201 + rand() %6;
			break;
		case 7:
			ent->s.frame = FRAME_death301 - 1;
			ent->client->anim_end = FRAME_death301 + rand() %6;
			break;
		}
	}

	if (random() < 0.2 && !IsFemale(ent))
		gi.sound(ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_BODY, gi.soundindex("boss3/d_hit.wav"), 1, ATTN_NORM, 0);
	ent->client->frozen = true;
	ent->client->frozen_time = level.time + frozen_time->value;
	ent->client->resp.thawer = NULL;
	ent->client->thaw_time = far_off;
	if (random() > 0.3)
		ent->client->hookstate -= ent->client->hookstate &(grow_on | shrink_on);
	ent->deadflag = DEAD_DEAD;
	gi.linkentity(ent);
}

qboolean gibCheck()
{
	if (gib_queue > 35)
		return true;
	else
	{
		gib_queue++;
		return false;
	}
}

void gibThink(edict_t* ent)
{
	gib_queue--;
	G_FreeEdict(ent);
}

static void playerView(edict_t* ent)
{
	int	i;
	edict_t*	other;
	vec3_t	ent_origin;
	vec3_t	forward;
	vec3_t	other_origin;
	vec3_t	dist;
	trace_t	trace;
	float	dot;
	float	other_dot;
	edict_t*	best_other;

	if (level.framenum &7)
		return;

	other_dot = 0.3;
	best_other = NULL;
	VectorCopy(ent->s.origin, ent_origin);
	ent_origin[2] += ent->viewheight;
	AngleVectors(ent->s.angles, forward, NULL, NULL);

	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.spectator)
			continue;
		if (other == ent)
			continue;
		if (other->light_level < 10 && !(other->client->flashlight))
			continue;
		if (other->health <= 0 && !other->client->frozen)
			continue;
		VectorCopy(other->s.origin, other_origin);
		other_origin[2] += other->viewheight;
		VectorSubtract(other_origin, ent_origin, dist);
		if (VectorLength(dist) > 800)
			continue;
		trace = gi.trace(ent_origin, vec3_origin, vec3_origin, other_origin, ent, MASK_OPAQUE);
		if (trace.fraction != 1)
			continue;
		VectorNormalize(dist);
		dot = DotProduct(dist, forward);
		if (dot > other_dot)
		{
			other_dot = dot;
			best_other = other;
		}
	}
	if (best_other)
		ent->client->viewed = best_other;
	else
		ent->client->viewed = NULL;
}

static void playerThaw(edict_t* ent)
{
	int	i;
	edict_t*	other;
	int	j;
	vec3_t	eorg;

	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.spectator)
			continue;
		if (other == ent)
			continue;
		if (other->health <= 0)
			continue;
		if (other->client->resp.team != ent->client->resp.team)
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = ent->s.origin[j] - (other->s.origin[j] + (other->mins[j] + other->maxs[j]) * 0.5);
		if (VectorLength(eorg) > MELEE_DISTANCE)
			continue;
		if (!(other->client->resp.help &thaw_help))
		{
			other->client->showscores = false;
			other->client->resp.help |= thaw_help;
			gi.centerprintf(other, "Wait here a second to free them.");
			gi.sound(other, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_STATIC, 0);
		}
		ent->client->resp.thawer = other;
		if (ent->client->thaw_time == far_off)
		{
			ent->client->thaw_time = level.time + 3;
			gi.sound(ent, CHAN_BODY, gi.soundindex("world/steam3.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	ent->client->resp.thawer = NULL;
	ent->client->thaw_time = far_off;
}

static void playerBreak(edict_t* ent, int force)
{
	int	n;

	ent->client->respawn_time = level.time + 1;
	if (ent->waterlevel == 3)
		gi.sound(ent, CHAN_BODY, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_BODY, gi.soundindex("world/brkglas.wav"), 1, ATTN_NORM, 0);
	n = rand() %(gib_queue > 10 ? 5 : 3);
	if (rand() &1)
	{
		switch (n)
		{
		case 0:
			ThrowGib(ent, "models/objects/gibs/arm/tris.md2", force, GIB_ORGANIC);
			break;
		case 1:
			ThrowGib(ent, "models/objects/gibs/bone/tris.md2", force, GIB_ORGANIC);
			break;
		case 2:
			ThrowGib(ent, "models/objects/gibs/bone2/tris.md2", force, GIB_ORGANIC);
			break;
		case 3:
			ThrowGib(ent, "models/objects/gibs/chest/tris.md2", force, GIB_ORGANIC);
			break;
		case 4:
			ThrowGib(ent, "models/objects/gibs/leg/tris.md2", force, GIB_ORGANIC);
			break;
		}
	}
	while (n--)
		ThrowGib(ent, "models/objects/debris1/tris.md2", force, GIB_ORGANIC);
	ent->takedamage = DAMAGE_NO;
	ent->movetype = MOVETYPE_TOSS;
	ThrowClientHead(ent, force);
	ent->client->frozen = false;
	freeze[ent->client->resp.team].update = true;
	ent->client->ps.stats[STAT_CHASE] = 0;
}

static void playerUnfreeze(edict_t* ent)
{
	if (level.time > ent->client->frozen_time && level.time > ent->client->respawn_time)
	{
		playerBreak(ent, 50);
		return;
	}
	if (ent->waterlevel == 3 && !(level.framenum &3))
		ent->client->frozen_time -= 0.15;
	if (level.time > ent->client->thaw_time)
	{
		if (!ent->client->resp.thawer || !ent->client->resp.thawer->inuse)
		{
			ent->client->resp.thawer = NULL;
			ent->client->thaw_time = far_off;
		}
		else
		{
			ent->client->resp.thawer->client->resp.score++;
			ent->client->resp.thawer->client->resp.thawed++;
			sl_LogScore(&gi, ent->client->resp.thawer->client->pers.netname, NULL, "Thaw", NULL, 1, level.time, ent->client->resp.thawer->client->ping);
			freeze[ent->client->resp.team].thawed++;
			if (rand() &1)
				gi.bprintf(PRINT_HIGH, "%s thaws %s like a package of frozen peas.\n", ent->client->resp.thawer->client->pers.netname, ent->client->pers.netname);
			else
				gi.bprintf(PRINT_HIGH, "%s evicts %s from their igloo.\n", ent->client->resp.thawer->client->pers.netname, ent->client->pers.netname);
			playerBreak(ent, 100);
		}
	}
}

static void playerMove(edict_t* ent)
{
	int	i;
	edict_t*	other;
	vec3_t	forward;
	float	dist;
	int	j;
	vec3_t	eorg;

	if (ent->client->hookstate)
		return;
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.spectator)
			continue;
		if (other == ent)
			continue;
		if (!other->client->frozen)
			continue;
		if (other->client->resp.team == ent->client->resp.team)
			continue;
		if (other->client->hookstate)
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = ent->s.origin[j] - (other->s.origin[j] + (other->mins[j] + other->maxs[j]) * 0.5);
		dist = VectorLength(eorg);
		if (dist > MELEE_DISTANCE)
			continue;
		VectorScale(forward, 600, other->velocity);
		other->velocity[2] = 200;
		gi.linkentity(other);
	}
}

void freezeMain(edict_t* ent)
{
	if (!ent->inuse)
		return;
	playerView(ent);
	if (ent->client->resp.spectator)
		return;
	if (ent->client->frozen)
	{
		playerThaw(ent);
		playerUnfreeze(ent);
	}
	else if (ent->health > 0)
		playerMove(ent);
}

void freezeScore(edict_t* ent, edict_t* killer)
{
	int	i, j, k;
	edict_t*	other;
	int	team, score;
	int	total[nteam];
	int	sorted[nteam][MAX_CLIENTS];
	int	sortedscores[nteam][MAX_CLIENTS];
	int	count, best_total, best_team;
	int	x, y;
	int	move_over;
	char	string[1400];
	int	stringlength;
	char*	tag;
	char	entry[1024];
	gclient_t*	cl;

	_team_loop
		total[i] = 0;
	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.spectator)
			team = none;
		else
			team = other->client->resp.team;
		score = other->client->resp.score;
		for (j = 0; j < total[team]; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k = total[team]; k > j; k--)
		{
			sorted[team][k] = sorted[team][k - 1];
			sortedscores[team][k] = sortedscores[team][k - 1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		total[team]++;
	}

	for (;;)
	{
		count = 0;
		team_loop
			count += 2 + total[i];
		if (count <= 48)
			break;
		best_total = 0;
		team_loop
			if (total[i] > best_total)
			{
				best_total = total[i];
				best_team = i;
			}
		if (best_total)
			total[best_team]--;
	}

	x = 0;
	y = 32;

	count = 4;
	_team_loop
		if (total[i])
			count += 3 + total[i];
	move_over = (int)(count / 2) * 8;

	string[0] = 0;
	stringlength = strlen(string);

	_team_loop
	{
		if (i == red)
			tag = "k_redkey";
		else if (i == blue)
			tag = "k_bluekey";
		else if (i == green)
			tag = "k_security";
		else
			tag = "k_powercube";

		if (i == none)
			Com_sprintf(entry, sizeof(entry), "xv %d yv %d string \"%6.6s\" ", x, y, freeze_team_[i]);
		else
			Com_sprintf(entry, sizeof(entry), "xv %d yv %d if %d picn %s endif string \"%6.6s Sco%3d Tha%3d\" ", x, y, 19 + i, tag, freeze_team_[i], freeze[i].score, freeze[i].thawed);
		k = strlen(entry);
		if (stringlength + k > 1024)
			break;
		if (total[i])
		{
			strcpy(string + stringlength, entry);
			stringlength += k;
			y += 16;
		}
		else
			continue;
		for (j = 0; j < total[i]; j++)
		{
			if (y >= 224)
			{
				if (x == 0)
					x = 160;
				else
					break;
				y = 32;
			}
			cl = &game.clients[sorted[i][j]];
			Com_sprintf(entry, sizeof(entry), "ctf %d %d %d %d %d ", x, y, sorted[i][j], cl->resp.score, level.intermissiontime ? cl->resp.thawed : (cl->ping > 999 ? 999 : cl->ping));
			if (cl->frozen)
				sprintf(entry + strlen(entry), "xv %d yv %d string2 \"/\" ", x + 56, y);
			k = strlen(entry);
			if (stringlength + k > 1024)
				break;
			strcpy(string + stringlength, entry);
			stringlength += k;
			y += 8;
		}
		Com_sprintf(entry, sizeof(entry), "xv %d yv %d string \"--------------------\" ", x, y);
		k = strlen(entry);
		if (stringlength + k > 1024)
			break;
		strcpy(string + stringlength, entry);
		stringlength += k;
		if (y >= 208 || (y >= move_over && x == 0))
		{
			if (x == 0)
				x = 160;
			else
				break;
			y = 32;
		}
		else
			y += 8;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

void freezeIntermission(void)
{
	int	i, j, k;
	int	team;

	i = j = k = 0;
	team_loop
		if (freeze[i].score > j)
			j = freeze[i].score;

	team_loop
		if (freeze[i].score == j)
		{
			k++;
			team = i;
		}

	if (k > 1)
	{
		i = j = k = 0;
		team_loop
			if (freeze[i].thawed > j)
				j = freeze[i].thawed;

		team_loop
			if (freeze[i].thawed == j)
			{
				k++;
				team = i;
			}
	}
	if (k != 1)
	{
		gi.bprintf(PRINT_HIGH, "Stalemate!\n");
		return;
	}
	gi.bprintf(PRINT_HIGH, "%s team is the winner!\n", freeze_team[team]);
	team_loop
		freeze[i].win_time = level.time;
	freeze[team].win_time = far_off;
}

char* makeGreen(char* s)
{
	static char	string[16];
	int	i;

	if (!*s)
		return "";
	for (i = 0; i < 15 && *s; i++, s++)
	{
		string[i] = *s;
		string[i] |= 0x80;
	}
	string[i] = 0;
	return string;
}

static void playerHealth(edict_t* ent)
{
	int	n;

	for (n = 0; n < game.num_items; n++)
		ent->client->pers.inventory[n] = 0;

	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->flags &= ~FL_POWER_ARMOR;

	ent->health = ent->client->pers.max_health;

	ent->s.sound = 0;
	ent->client->weapon_sound = 0;
}

static void breakTeam(int team)
{
	int	i;
	edict_t*	ent;
	float	break_time;

	break_time = level.time;
	game_loop
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->client->frozen)
		{
			if (ent->client->resp.team != team && team_max_count >= 3)
				continue;
			ent->client->frozen_time = break_time;
			break_time += 0.25;
			continue;
		}
		if (ent->health > 0 && team_max_count < 3)
		{
			playerHealth(ent);
			playerWeapon(ent);
		}
	}
	freeze[team].break_time = break_time + 1;
	if (rand() &1)
		gi.bprintf (PRINT_HIGH, "%s team was run circles around by their foe.\n", freeze_team[team]);
	else
		gi.bprintf (PRINT_HIGH, "%s team was less than a match for their foe.\n", freeze_team[team]);
}

static void updateTeam(int team)
{
	int	i;
	edict_t*	ent;
	int	frozen, alive;
	char	small[32];
	int	play_sound = 0;

	frozen = alive = 0;
	game_loop
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->client->resp.spectator)
			continue;
		if (ent->client->resp.team != team)
			continue;
		if (ent->client->frozen)
			frozen++;
		if (ent->health > 0)
			alive++;
	}
	freeze[team].frozen = frozen;
	freeze[team].alive = alive;

	if (frozen && !alive)
	{
		team_loop
		{
			if (freeze[i].alive)
			{
				play_sound++;
				freeze[i].score++;
				freeze[i].win_time = level.time + 5;
				freeze[i].update = true;
			}
		}
		breakTeam(team);

		if (play_sound <= 1)
			gi.positioned_sound(vec3_origin, world, CHAN_VOICE | CHAN_RELIABLE, gi.soundindex("world/xian1.wav"), 1, ATTN_NONE, 0);
	}

	Com_sprintf(small, sizeof(small), " %s%3d/%3d", freeze_team__[team], freeze[team].score, freeze[team].alive);
//	if (!(freeze[team].alive == 1 && freeze[team].frozen))
//		makeGreen(small);
	gi.configstring(CS_GENERAL + team, small);
}

qboolean endCheck()
{
	int	i;

	if (!(level.framenum &31))
	{
		if (new_team_count->value)
		{
			int	_new_team_count = new_team_count->value;
			int	total[nteam];

			_team_loop
				total[i] = freeze[i].alive + freeze[i].frozen;

			if (total[yellow])
				team_max_count = 4;
			else if (total[red] >= _new_team_count && total[blue] >= _new_team_count)
			{
				if (total[green] >= _new_team_count)
					team_max_count = 4;
				else
					team_max_count = 3;
			}
			else if (total[green])
				team_max_count = 3;
			else
				team_max_count = 0;
		}
		else
			team_max_count = 0;
	}

	if (use_ready->value && !(lame_hack &everyone_ready))
	{
		switch ((int)(ready_time / FRAMETIME) - level.framenum)
		{
		case 150:
		case 100:
		case 50:
		case 40:
		case 30:
		case 20:
			gi.bprintf(PRINT_HIGH, "Begin in %d seconds!\n", (int)(((ready_time / FRAMETIME) - level.framenum) * FRAMETIME));
		}
		if (level.time > ready_time)
		{
			edict_t*	ent;

			lame_hack |= everyone_ready;
			gi.bprintf(PRINT_HIGH, "Begin!\n");
			game_loop
			{
				ent = g_edicts + 1 + i;
				if (!ent->inuse)
					continue;
				if (ent->client->resp.spectator)
					continue;
				if (ent->health > 0)
				{
					playerHealth(ent);
					playerWeapon(ent);
				}
			}
		}
	}
	else
		lame_hack |= everyone_ready;

	team_loop
		if (freeze[i].update && level.time > freeze[i].last_update)
		{
			updateTeam(i);
			freeze[i].update = false;
			freeze[i].last_update = level.time + 3;
		}

	if (point_limit->value)
	{
		int	_point_limit;

		_point_limit = point_limit->value;
		if (team_max_count >= 3)
			_point_limit *= 3;
		team_loop
			if (freeze[i].score >= _point_limit)
				return true;
	}
	if (lame_hack &end_vote)
		return true;

	return false;
}

void flashlightThink(edict_t* ent)
{
	vec3_t	forward, right, offset;
	vec3_t	start, end;
	trace_t	trace;

	if (!ent->owner->inuse || ent->owner->health <= 0 || !ent->owner->client->flashlight || level.intermissiontime)
	{
		G_FreeEdict(ent);
		return;
	}
	AngleVectors(ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, random() < 0.2 ? crandom() : 0, ent->owner->viewheight);
	G_ProjectSource(ent->owner->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	trace = gi.trace(start, NULL, NULL, end, ent->owner, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);
	VectorCopy(trace.endpos, ent->s.origin);
	gi.linkentity(ent);
	ent->nextthink = level.time + FRAMETIME;
}

void cmdFlashlight(edict_t* ent)
{
	edict_t*	light;

	if (ent->client->flashlight || ent->health <= 0 || ent->client->resp.spectator)
	{
		ent->client->flashlight = false;
		return;
	}

	light = G_Spawn();
	light->svflags = SVF_DEADMONSTER;
	light->movetype = MOVETYPE_NOCLIP;
	light->solid = SOLID_NOT;
	light->s.effects = EF_BFG;
	light->s.renderfx = RF_BEAM;
	light->s.modelindex = gi.modelindex("sprites/s_bubble.sp2");
	light->owner = ent;
	gi.linkentity(light);
	light->think = flashlightThink;
	light->nextthink = level.time + FRAMETIME;
	ent->client->flashlight = true;
}

void cmdMoan(edict_t* ent)
{
	if (!(ent->client->resp.help &frozen_help))
	{
		ent->client->showscores = false;
		ent->client->resp.help |= frozen_help;
		gi.centerprintf(ent, "You have been frozen.\nWait to be saved.");
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_STATIC, 0);
	}
	else if (!(ent->client->chase_target || ent->client->resp.help &chase_help))
	{
		GetChaseTarget(ent);
		ent->client->showscores = false;
		ent->client->resp.help |= chase_help;
		gi.centerprintf(ent, "Use the chase camera with\nyour inventory keys.");
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/talk1.wav"), 1, ATTN_STATIC, 0);
		return;
	}
	if (ent->client->moan_time > level.time)
		return;
	ent->client->moan_time = level.time + 2;
	if (ent->waterlevel == 3)
	{
		if (rand() &1)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("flipper/flpidle1.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(ent, CHAN_AUTO, gi.soundindex("flipper/flpsrch1.wav"), 1, ATTN_NORM, 0);
	}
	else
		gi.sound(ent, CHAN_AUTO, moan[rand() %8], 1, ATTN_NORM, 0);
}

void cmdPlay(edict_t* ent)
{
	char	*s;

	s = gi.argv(1);
	if (!*s)
	{
		gi.cprintf(ent, PRINT_HIGH, "pv <filename> : play wav file\n");
		return;
	}
	if (floodCheck(ent))
		return;
	if (strlen(s) > 32)
		return;
	gi.cprintf(ent, PRINT_HIGH, "Pv: %s\n", s);
	gi.sound(ent, CHAN_VOICE, gi.soundindex(va("%s.wav", s)), 1, ATTN_IDLE, 0);
}

void freezeRespawn(edict_t* ent, float delay)
{
	if (item_respawn_time->value)
		SetRespawn(ent, item_respawn_time->value);
	else
		SetRespawn(ent, delay);
}

void playerShell(edict_t* ent, int team)
{
	ent->s.effects |= EF_COLOR_SHELL;
	if (team == red)
		ent->s.renderfx |= RF_SHELL_RED;
	else if (team == blue)
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE;
	else if (team == green)
		ent->s.renderfx |= RF_SHELL_GREEN;
	else
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_GREEN;
}

void freezeEffects(edict_t* ent)
{
	if (level.intermissiontime)
		return;
	if (ent->client->flashlight)
		ent->s.renderfx = RF_MINLIGHT;
	if (!ent->client->frozen)
		return;
	if (!ent->client->resp.thawer || level.framenum &8)
		playerShell(ent, ent->client->resp.team);
}

void freezeSkin(edict_t* ent, char* s)
{
	char*	p;
	char	t[64];
	int	playernum = ent - g_edicts - 1;

	Com_sprintf(t, sizeof(t), "%s", s);
	if ((p = strchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->resp.team)
	{
	case red:
	case blue:
	case green:
	case yellow:
		gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s%s", ent->client->pers.netname, t, freeze_skin[ent->client->resp.team]));
		break;
	default:
		gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s", ent->client->pers.netname, s));
	}
}

enum pmenu_enum {
	align_left, align_center, align_right };

void pmenu_update(edict_t* ent)
{
	hndnode*	hnd;
	char	string[1400];
	int	i;
	pmenunode*	p;
	char*	t;
	qboolean	alt = false;
	int	x;

	if (!ent->client->menu)
		return;
	hnd = ent->client->menu;
	strcpy(string, "xv 32 yv 32 picn inventory ");
	for (i = 0, p = hnd->entries; i < hnd->num; i++, p++)
	{
		if (!p->text)
			continue;
		t = p->text;
		if (*t == '*')
		{
			alt = true;
			t++;
		}
		if (p->align == align_center)
			x = (20 - (int)(strlen(t) / 2)) * 8;
		else if (p->align == align_right)
			x = (33 - strlen(t)) * 8;
		else
			x = 56;
		sprintf(string + strlen(string), "xv %d yv %d ", x - (hnd->cur == i ? 8 : 0), 32 + i * 8);
		if (hnd->cur == i)
			sprintf(string + strlen(string), "string2 \">%s<\" ", t);
		else if (alt)
			sprintf(string + strlen(string), "string2 \"%s\" ", t);
		else
			sprintf(string + strlen(string), "string \"%s\" ", t);
		alt = false;
	}
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

void pmenu_close(edict_t* ent)
{
	if (!ent->client->menu)
		return;
	free(ent->client->menu);
	ent->client->menu = NULL;
	ent->client->showscores = false;
}

void pmenu_open(edict_t* ent, pmenunode* entries, int cur, int num)
{
	hndnode*	hnd;
	int	i;
	pmenunode*	p;

	if (ent->client->menu)
		pmenu_close(ent);
	hnd = malloc(sizeof(*hnd));
	hnd->entries = entries;
	hnd->num = num;
	if (cur < 0 || !entries[cur].selectfunc)
	{
		for (i = 0, p = entries; i < num; i++, p++)
		{
			if (p->selectfunc)
				break;
		}
	}
	else
		i = cur;
	if (i >= num)
		hnd->cur = -1;
	else
		hnd->cur = i;
	ent->client->showscores = true;
	ent->client->menu = hnd;
	pmenu_update(ent);
	gi.unicast(ent, true);
}

void pmenu_next(edict_t* ent)
{
	hndnode*	hnd;
	int	i;
	pmenunode*	p;

	if (!ent->client->menu)
		return;
	hnd = ent->client->menu;
	if (hnd->cur < 0)
		return;
	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do
	{
		i++, p++;
		if (i == hnd->num)
			i = 0, p = hnd->entries;
		if (p->selectfunc)
			break;
	} while (i != hnd->cur);
	hnd->cur = i;
	pmenu_update(ent);
	gi.unicast(ent, true);
}

void pmenu_prev(edict_t* ent)
{
	hndnode*	hnd;
	int	i;
	pmenunode*	p;

	if (!ent->client->menu)
		return;
	hnd = ent->client->menu;
	if (hnd->cur < 0)
		return;
	i = hnd->cur;
	p = hnd->entries + hnd->cur;
	do
	{
		if (i == 0)
		{
			i = hnd->num - 1;
			p = hnd->entries + i;
		}
		else
			i--, p--;
		if (p->selectfunc)
			break;
	} while (i != hnd->cur);
	hnd->cur = i;
	pmenu_update(ent);
	gi.unicast(ent, true);
}

void pmenu_select(edict_t* ent)
{
	hndnode*	hnd;
	pmenunode*	p;

	if (!ent->client->menu)
		return;
	hnd = ent->client->menu;
	if (hnd->cur < 0)
		return;
	p = hnd->entries + hnd->cur;
	if (p->selectfunc)
		p->selectfunc(ent, p);
}

static void menu_red(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	gi.WriteByte(svc_stufftext);
	gi.WriteString("team red\n");
	gi.unicast(ent, true);
}

static void menu_blue(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	gi.WriteByte(svc_stufftext);
	gi.WriteString("team blue\n");
	gi.unicast(ent, true);
}

static void menu_green(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	gi.WriteByte(svc_stufftext);
	gi.WriteString("team green\n");
	gi.unicast(ent, true);
}

static void menu_yellow(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	gi.WriteByte(svc_stufftext);
	gi.WriteString("team yellow\n");
	gi.unicast(ent, true);
}

void do_main(edict_t* ent);

static void menu_main(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	do_main(ent);
};

pmenunode	credits_menu[] = {
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"*Author", align_center, NULL, NULL}, 
	{"Darrell Bircsak", align_center, NULL, NULL}, 
	{"*GSLog", align_center, NULL, NULL}, 
	{"Mark Davies", align_center, NULL, NULL}, 
	{"*Hook Code", align_center, NULL, NULL}, 
	{"Perecli Manole", align_center, NULL, NULL}, 
	{"*Macro and Menu Code", align_center, NULL, NULL}, 
	{"Dave Kirsch", align_center, NULL, NULL}, 
	{"*Team Skins", align_center, NULL, NULL}, 
	{"Grimlock", align_center, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"Continue", align_left, NULL, menu_main}, 
	{NULL, 0, NULL, NULL}
};

static void menu_credits(edict_t* ent, pmenunode* p)
{
	pmenu_close(ent);
	pmenu_open(ent, credits_menu, -1, sizeof(credits_menu) / sizeof(pmenunode));
}

pmenunode	title_menu[] = {
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"planetquake.com/freeze", align_center, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"*Freeze Tag", align_center, NULL, NULL}, 
	{"*by Darrell Bircsak", align_center, NULL, NULL}, 
	{"*alias Doolittle", align_center, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"Join Red Team", align_left, NULL, menu_red}, 
	{"Join Blue Team", align_left, NULL, menu_blue}, 
	{NULL, align_left, NULL, NULL}, 
	{NULL, align_left, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{NULL, 0, NULL, NULL}, 
	{"Credits", align_left, NULL, menu_credits}, 
	{NULL, 0, NULL, NULL}, 
	{"*1.89", align_right, NULL, NULL}, 
	{NULL, 0, NULL, NULL}
};

pmenunode	motd_menu[] = {
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{NULL, align_center, NULL, NULL}, 
	{"Continue", align_left, NULL, menu_main}, 
	{NULL, align_center, NULL, NULL}
};

void do_main(edict_t* ent)
{
	int	total[nteam];
	int	i;
	edict_t*	other;
	int	best_total = 128, best_team = -1;

	_team_loop
		total[i] = 0;
	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.spectator)
			continue;
		total[other->client->resp.team]++;
	}

	team_loop
	{
		if (i == green && team_max_count < 3)
			continue;
		if (i == yellow && team_max_count < 4)
			continue;
		if (total[i] <= best_total)
		{
			if (total[i] == best_total && rand() &1)
				continue;
			best_total = total[i];
			best_team = i;
		}
	}

	if (team_max_count >= 3)
	{
		title_menu[14].text = "Join Green Team";
		title_menu[14].selectfunc = menu_green;
	}
	else
	{
		title_menu[14].text = "*Join Green Team";
		title_menu[14].selectfunc = NULL;
	}
	if (team_max_count >= 4)
	{
		title_menu[15].text = "Join Yellow Team";
		title_menu[15].selectfunc = menu_yellow;
	}
	else
	{
		title_menu[15].text = "*Join Yellow Team";
		title_menu[15].selectfunc = NULL;
	}

	pmenu_open(ent, title_menu, best_team + 12, sizeof(title_menu) / sizeof(pmenunode));
}

static void sayArmor(edict_t* ent, char* buf)
{
	int	power_armor_type;
	int	cells;
	int	armor_type;
	gitem_t*	item;

	*buf = 0;
	power_armor_type = PowerArmorType(ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (cells)
			sprintf(buf + strlen(buf), "%s with %d cells", power_armor_type == POWER_ARMOR_SCREEN ? "Power Screen" : "Power Shield", cells);
	}
	armor_type = ArmorIndex(ent);
	if (armor_type)
	{
		item = GetItemByIndex(armor_type);
		if (item)
		{
			if (*buf)
				strcat(buf, " and ");
			sprintf(buf + strlen(buf), "%i units of %s", ent->client->pers.inventory[armor_type], item->pickup_name);
		}
	}
	if (!*buf)
		strcpy(buf, "no armor");
}

static void sayHealth(edict_t* ent, char* buf)
{
	if (ent->health <= 0)
	{
		if (ent->client->frozen)
			strcpy(buf, "frozen");
		else
			strcpy(buf, "dead");
	}
	else
		sprintf(buf, "%i health", ent->health);
}

static void sayTech(edict_t* ent, char* buf)
{
	if (ent->client->resp.thawer && ent->client->resp.thawer->inuse)
		sprintf(buf, "%s", ent->client->resp.thawer->client->pers.netname);
	else
		strcpy(buf, "noone");
}

static void sayWeapon(edict_t* ent, char* buf)
{
	if (ent->client->pers.weapon)
		strcpy(buf, ent->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static qboolean loc_cansee(edict_t* targ, edict_t* inflictor)
{
	vec3_t	viewpoint;
	trace_t	trace;

	if (targ->movetype == MOVETYPE_PUSH)
		return false;
	if (targ->spawnflags &(DROPPED_ITEM | DROPPED_PLAYER_ITEM))
		return false;
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;
	trace = gi.trace(viewpoint, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1)
		return true;
	else
		return false;
}

static void saySight(edict_t* who, char* buf)
{
	char	s[1024], s2[1024];
	int	i;
	edict_t*	targ;
	int	n = 0;

	*s = 0;
	*s2 = 0;
	game_loop
	{
		targ = g_edicts + 1 + i;
		if (!targ->inuse || targ == who || targ->client->resp.spectator || targ->deadflag)
			continue;
		if (!loc_cansee(targ, who))
			continue;
		if (*s2)
		{
			if (strlen(s) + strlen(s2) + 3 < sizeof(s))
			{
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2 && !level.intermissiontime)
	{
		if (strlen(s) + strlen(s2) + 6 < sizeof(s))
		{
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	}
	else
		strcpy(buf, "noone");
}

static edict_t* loc_findradius(edict_t* from, vec3_t org, float rad)
{
	int	j;
	vec3_t	eorg;

	if (!from)
		from = g_edicts;
	else
		from++;
	for (; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}
	return NULL;
}

struct {
	char*	classname;
	int	priority;
} loc_names[] = {
	{"item_quad", 1}, 
	{"item_invulnerability", 1}, 
	{"weapon_bfg", 2}, 
	{"weapon_railgun", 2}, 
	{"weapon_rocketlauncher", 3}, 
	{"weapon_hyperblaster", 3}, 
	{"weapon_chaingun", 3}, 
	{"weapon_grenadelauncher", 3}, 
	{"weapon_machinegun", 3}, 
	{"weapon_supershotgun", 3}, 
	{"weapon_shotgun", 3}, 
	{"item_power_screen", 4}, 
	{"item_power_shield", 4}, 
	{"item_armor_body", 5}, 
	{"item_armor_combat", 5}, 
	{"item_armor_jacket", 5}, 
	{"item_silencer", 6}, 
	{"item_breather", 6}, 
	{"item_enviro", 6}, 
	{"item_adrenaline", 7}, 
	{"item_bandolier", 7}, 
	{"item_pack", 7}, 
	{NULL, 0}
};

static void sayLocation(edict_t* who, char* buf)
{
	edict_t*	what = NULL;
	int	i;
	qboolean	cansee;
	qboolean	hotsee = false;
	int	hotindex = 999;
	edict_t*	hot = NULL;
	vec3_t	v;
	float	hotdist = 999999;
	float	newdist;
	gitem_t*	item;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL)
	{
		for (i = 0; loc_names[i].classname; i++)
		{
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		}
		if (!loc_names[i].classname)
			continue;
		cansee = loc_cansee(what, who);
		if (cansee && !hotsee)
		{
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || (cansee && loc_names[i].priority < hotindex))
		{
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_cansee(hot, who);
		}
	}
	if (!hot || (item = FindItemByClassname(hot->classname)) == NULL)
	{
		strcpy(buf, "nowhere");
		return;
	}
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
	{
		if (v[2] > 0)
			strcat(buf, "above the ");
		else
			strcat(buf, "below the ");
	}
	else
		strcat(buf, "near the ");
	strcat(buf, item->pickup_name);
}

void sayTeam(edict_t* who)
{
	char	outmsg[1024];
	char*	p;
	char	buf[1024];
	int	i;
	edict_t*	cl_ent;
	char*	msg;

	if (floodCheck(who))
		return;
	msg = gi.args();
	outmsg[0] = 0;
	if (*msg == '\"')
	{
		msg[strlen(msg) - 1] = 0;
		msg++;
	}
	for (p = outmsg; *msg && p - outmsg < sizeof(outmsg) - 1; msg++)
	{
		if (*msg == '%')
		{
			switch (*++msg)
			{
			case 'l':
			case 'L':
				sayLocation(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'a':
			case 'A':
				sayArmor(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 't':
			case 'T':
				sayTech(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'h':
			case 'H':
				sayHealth(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'w':
			case 'W':
				sayWeapon(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'n':
			case 'N':
				saySight(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			default:
				*p++ = *msg;
			}
		}
		else
			*p++ = *msg;
	}

	*p = 0;
	if (strlen(outmsg) > 150)
		outmsg[150] = 0;
	game_loop
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.spectator)
		{
			if (!who->client->resp.spectator)
				continue;
		}
		else if (who->client->resp.spectator)
			continue;
		else if (cl_ent->client->resp.team != who->client->resp.team)
			continue;
		gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", who->client->pers.netname, outmsg);
	}
	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "(%s): %s\n", who->client->pers.netname, outmsg);
}

static void loadMessage()
{
	cvar_t*	game;
	char	name[MAX_OSPATH];
	FILE*	f;
	char	st[128];
	int	index = 0;

	lame_hack &= ~is_motd;
	game = gi.cvar("game", "", 0);
	if (!*game->string)
		sprintf(name, "baseq2/freeze.ini");
	else
		sprintf(name, "%s/freeze.ini", game->string);
	f = fopen(name, "r");
	if (!f)
		return;
	while (fgets(st, sizeof(st), f))
	{
		if (strstr(st, "[motd]"))
			break;
	}
	if (feof(f))
	{
		fclose(f);
		return;
	}
	while (fgets(st, sizeof(st), f))
	{
		if (strstr(st, "//"))
			continue;
		if (strstr(st, "###"))
			break;
		st[strlen(st) - 1] = 0;
		if (strlen(st))
		{
			motd_menu[index].text = G_CopyString(st);
			lame_hack |= is_motd;
		}
		index++;
	}
	fclose(f);
}

void pmenuBegin(edict_t* ent)
{
	if (ent->client->resp.team == none && lame_hack &is_motd)
		pmenu_open(ent, motd_menu, -1, sizeof(motd_menu) / sizeof (pmenunode));
	else
		do_main(ent);
}

void playerStat(edict_t* ent)
{
	int	i;

	if (ent->client->viewed && ent->client->viewed->inuse)
	{
		int	playernum = ent->client->viewed - g_edicts - 1;

		ent->client->ps.stats[stat_identify] = CS_PLAYERSKINS + playernum;
	}
	else
		ent->client->ps.stats[stat_identify] = 0;

	team_loop
	{
		if (((i == green && team_max_count < 3) || (i == yellow && team_max_count < 4)) || 
			(freeze[i].win_time > level.time && !(level.framenum &8)))
		{
			ent->client->ps.stats[stat_red + i] = 0;
			ent->client->ps.stats[stat_red_arrow + i] = 0;
			continue;
		}

		ent->client->ps.stats[stat_red + i] = CS_GENERAL + i;
		if (ent->client->resp.team == i && !ent->client->resp.spectator)
			ent->client->ps.stats[stat_red_arrow + i] = CS_GENERAL + 5;
		else
			ent->client->ps.stats[stat_red_arrow + i] = 0;
	}
}

static void p_projectsourcereverse(gclient_t* client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy(distance, _distance);

	if (client->pers.hand == LEFT_HANDED)
		;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	else
		_distance[1] *= -1;
	G_ProjectSource(point, _distance, forward, right, result);
}

static void drophook(edict_t* ent)
{
	ent->owner->client->hookstate = 0;
	ent->owner->client->hooker = 0;
	gi.sound(ent->owner, chan_hook, gi.soundindex(_drophook), 1, ATTN_IDLE, 0);
	G_FreeEdict(ent);
}

static void maintainlinks(edict_t* ent)
{
	float	multiplier;
	vec3_t	norm_hookvel, pred_hookpos;
	vec3_t	forward, right, offset;
	vec3_t	start, chainvec;
	vec3_t	norm_chainvec;

	multiplier = VectorLength(ent->velocity) / 22;
	VectorNormalize2(ent->velocity, norm_hookvel);
	VectorMA(ent->s.origin, multiplier, norm_hookvel, pred_hookpos);

	AngleVectors(ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8);
	p_projectsourcereverse(ent->owner->client, ent->owner->s.origin, offset, forward, right, start);
	_VectorSubtract(pred_hookpos, start, chainvec);
	VectorNormalize2(chainvec, norm_chainvec);
	VectorMA(pred_hookpos, -20, norm_chainvec, pred_hookpos);
	VectorMA(start, 10, norm_chainvec, start);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort(ent - g_edicts);
	gi.WritePosition(pred_hookpos);
	gi.WritePosition(start);
	gi.multicast(ent->s.origin, MULTICAST_PVS);
}

void hookbehavior(edict_t* ent)
{
	edict_t*	targ;
	qboolean	chain_moving;
	vec3_t	forward, right;
	vec3_t	offset, start;
	vec3_t	end;
	vec3_t	chainvec, velpart;
	float	chainlen;
	float	force;
	int	_hook_rpf = hook_rpf->value;

	if (!_hook_rpf)
		_hook_rpf = 80;

	targ = ent->owner;
	if (!targ->inuse || !(targ->client->hookstate &hook_on) || ent->enemy->solid == SOLID_NOT || 
		(targ->health <= 0 && !targ->client->frozen) || level.intermissiontime || targ->s.event == EV_PLAYER_TELEPORT)
	{
		drophook(ent);
		return;
	}

	VectorCopy(ent->enemy->velocity, ent->velocity);

	chain_moving = false;
	if (targ->client->hookstate &grow_on && ent->angle < hook_max_len->value)
	{
		ent->angle += _hook_rpf;
		if (ent->angle > hook_max_len->value)
			ent->angle = hook_max_len->value;
		chain_moving = true;
	}
	if (targ->client->hookstate &shrink_on && ent->angle > hook_min_len->value)
	{
		ent->angle -= _hook_rpf;
		if (ent->angle < hook_min_len->value)
			ent->angle = hook_min_len->value;
		chain_moving = true;
	}
	if (chain_moving)
	{
		if (ent->sounds == motor_off)
		{
			gi.sound(targ, chan_hook, gi.soundindex(_motorstart), 1, ATTN_IDLE, 0);
			ent->sounds = motor_start;
		}
		else if (ent->sounds == motor_start)
		{
			gi.sound(targ, chan_hook, gi.soundindex(_motoron), 1, ATTN_IDLE, 0);
			ent->sounds = motor_on;
		}
	}
	else if (ent->sounds != motor_off)
	{
		gi.sound(targ, chan_hook, gi.soundindex(_motoroff), 1, ATTN_IDLE, 0);
		ent->sounds = motor_off;
	}

	AngleVectors(ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8);
	p_projectsourcereverse(ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	targ = NULL;
	if (ent->enemy->client)
	{
		targ = ent->enemy;
		if (!targ->inuse || (targ->health <= 0 && !targ->client->frozen) || 
			(targ->client->buttons &4 && random() < 0.3) || targ->s.event == EV_PLAYER_TELEPORT)
		{
			drophook(ent);
			return;
		}
		VectorCopy(ent->s.origin, end);
		VectorCopy(start, ent->s.origin);
		VectorCopy(end, start);
		targ = ent->owner;
		ent->owner = ent->enemy;
		ent->enemy = targ;
	}

	_VectorSubtract(ent->s.origin, start, chainvec);
	chainlen = VectorLength(chainvec);
	if (chainlen > ent->angle)
	{
		VectorScale(chainvec, _DotProduct(ent->owner->velocity, chainvec) / _DotProduct(chainvec, chainvec), velpart);
		force = (chainlen - ent->angle) * 5;
		if (_DotProduct(ent->owner->velocity, chainvec) < 0)
		{
			if (chainlen > ent->angle + 25)
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
		}
		else
		{
			if (VectorLength(velpart) < force)
				force -= VectorLength(velpart);
			else
				force = 0;
		}
	}
	else
		force = 0;

	VectorNormalize(chainvec);
	VectorMA(ent->owner->velocity, force, chainvec, ent->owner->velocity);
	SV_CheckVelocity(ent->owner);

	if (targ)
	{
		targ = ent->enemy;
		ent->enemy = ent->owner;
		ent->owner = targ;
		VectorCopy(ent->enemy->s.origin, ent->s.origin);
	}
	else if (!ent->owner->client->resp.old_hook && 
		ent->owner->client->hookstate &shrink_on && chain_moving)
		ent->owner->velocity[2] += ent->owner->gravity * sv_gravity->value * FRAMETIME;
	maintainlinks(ent);
	ent->nextthink = level.time + FRAMETIME;
}

void hooktouch(edict_t* ent, edict_t* other, cplane_t* plane, csurface_t* surf)
{
	vec3_t	forward, right;
	vec3_t	offset, start;
	vec3_t	chainvec;

	AngleVectors(ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8);
	p_projectsourcereverse(ent->owner->client, ent->owner->s.origin, offset, forward, right, start);
	_VectorSubtract(ent->s.origin, start, chainvec);
	ent->angle = VectorLength(chainvec);
	if (surf && surf->flags &SURF_SKY)
	{
		drophook(ent);
		return;
	}
	if (other->takedamage)
		T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 100, 0, MOD_HIT);
	if (other->solid == SOLID_BBOX)
	{
		if (other->client && ent->owner->client->hooker < 2)
		{
			ent->owner->client->hooker++;
			other->s.origin[2] += 9;
			gi.sound(ent, CHAN_VOICE, gi.soundindex(_hooktouch), 1, ATTN_IDLE, 0);
		}
		else
		{
			drophook(ent);
			return;
		}
	}
	else if (other->solid == SOLID_BSP && grapple_wall->value)
	{
		if (!ent->owner->client->resp.old_hook)
			VectorClear(ent->owner->velocity);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_SHOTGUN);
		gi.WritePosition(ent->s.origin);
		if (!plane)
			gi.WriteDir(vec3_origin);
		else
			gi.WriteDir(plane->normal);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
		gi.sound(ent, CHAN_VOICE, gi.soundindex(_touchsolid), 1, ATTN_IDLE, 0);
	}
	else
	{
		drophook(ent);
		return;
	}
	VectorCopy(other->velocity, ent->velocity);
	ent->owner->client->hookstate |= hook_in;
	ent->enemy = other;
	ent->touch = NULL;
	ent->think = hookbehavior;
	ent->nextthink = level.time + FRAMETIME;
}

void hookairborne(edict_t* ent)
{
	vec3_t	chainvec;
	float	chainlen;

	_VectorSubtract(ent->s.origin, ent->owner->s.origin, chainvec);
	chainlen = VectorLength(chainvec);
	if(!(ent->owner->client->hookstate &hook_on) || chainlen > hook_max_len->value)
	{
		drophook(ent);
		return;
	}
	maintainlinks(ent);
	ent->nextthink = level.time + FRAMETIME;
}

static void firehook(edict_t* ent)
{
	int	damage;
	vec3_t	forward, right;
	vec3_t	offset, start;
	edict_t*	newhook;

	damage = 10;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight - 8);
	p_projectsourcereverse(ent->client, ent->s.origin, offset, forward, right, start);

	newhook = G_Spawn();
	newhook->svflags = SVF_DEADMONSTER;
	VectorCopy(start, newhook->s.origin);
	VectorCopy(start, newhook->s.old_origin);
	vectoangles(forward, newhook->s.angles);
	VectorScale(forward, hook_speed->value, newhook->velocity);
	VectorCopy(forward, newhook->movedir);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	VectorClear(newhook->mins);
	VectorClear(newhook->maxs);
	newhook->owner = ent;
	newhook->dmg = damage;
	newhook->sounds = 0;
	newhook->touch = hooktouch;
	newhook->think = hookairborne;
	newhook->nextthink = level.time + FRAMETIME;
	gi.linkentity(newhook);
	gi.sound(ent, chan_hook, gi.soundindex(_firehook), 1, ATTN_IDLE, 0);
}

void cmdHook(edict_t* ent)
{
	char*	s;
	int*	hookstate;

	if (!hook_rpf->value && !(lame_hack &mapnohook))
		return;
	if (hook_rpf->value && lame_hack &mapnohook)
		return;
	s = gi.argv(1);
	if (!*s)
	{
		gi.cprintf(ent, PRINT_HIGH, "hook <value> [action / stop / grow / shrink] : control hook\n");
		if (ent->client->resp.old_hook)
			ent->client->resp.old_hook = false;
		else
			ent->client->resp.old_hook = true;
		return;
	}
	if (ent->health <= 0 || ent->client->resp.spectator)
		return;
	hookstate = &ent->client->hookstate;
	if(!(*hookstate &hook_on) && Q_stricmp(s, "action") == 0)
	{
		*hookstate = hook_on;
		firehook(ent);
		return;
	}
	if (*hookstate &hook_on)
	{
		if (Q_stricmp(s, "action") == 0)
		{
			*hookstate = 0;
			return;
		}
		if (Q_stricmp(s, "stop") == 0)
		{
			if (ent->client->resp.old_hook || ent->client->hooker)
				*hookstate -= *hookstate &(grow_on | shrink_on);
			else
				*hookstate = 0;
			return;
		}
		if (Q_stricmp(s, "grow") == 0)
		{
			*hookstate |= grow_on;
			*hookstate -= *hookstate &shrink_on;
			return;
		}
		if (Q_stricmp(s, "shrink") == 0)
		{
			*hookstate |= shrink_on;
			*hookstate -= *hookstate &grow_on;
			return;
		}
	}
}

static void addmap(char* s)
{
	char*	t;
	int	i, j;

	map_loop
	{
		if (maplist[i].name)
			continue;

		t = strtok(s, ",");
		j = 0;
		while (t != NULL)
		{
			if (j == 0)
				maplist[i].name = G_CopyString(t);
			else if (j == 1)
				maplist[i].gravity = G_CopyString(t);
			else if (j == 2)
				maplist[i].light = atoi(t);
			else if (j == 3)
				maplist[i].hook = (atoi(t) == 0 ? false : true);
			else
				break;
			j++;
			t = strtok(NULL, ",");
		}
		break;
	}
}

static void loadMap()
{
	cvar_t*	game;
	char	name[MAX_OSPATH];
	FILE*	f;
	char	st[128];

	memset(maplist, 0, sizeof(maplist));
	game = gi.cvar("game", "", 0);
	if (!*game->string)
		sprintf(name, "baseq2/freeze.ini");
	else
		sprintf(name, "%s/freeze.ini", game->string);
	f = fopen(name, "r");
	if (!f)
		return;
	while (fgets(st, sizeof(st), f))
	{
		if (strstr(st, "[maplist]"))
			break;
	}
	if (feof(f))
	{
		fclose(f);
		return;
	}
	while (fgets(st, sizeof(st), f))
	{
		if (strstr(st, "//"))
			continue;
		if (strstr(st, "###"))
			break;
		st[strlen(st) - 1] = 0;
		if (strlen(st))
			addmap(st);
	}
	fclose(f);
}

static int mapCount()
{
	int	i;
	edict_t*	other;
	int	vote_count, player_count;
	int	needed;

	vote_count = player_count = 0;
	game_loop
	{
		other = g_edicts + 1 + i;
		if (!other->inuse)
			continue;
		if (other->client->resp.vote != -1)
			vote_count++;
		player_count++;
	}
	if (vote_percent->value)
		needed = (int)((vote_percent->value / 100) * player_count) - vote_count;
	else
		needed = (int)(0.50 * player_count) - vote_count;
	return (needed > 0 ? needed : 0);
}

void cmdVote(edict_t* ent)
{
	int	i;
	char*	s;

	if (!maplist[0].name)
		return;
	s = gi.argv(1);
	if (!*s || level.time < 30)
	{
		char	small[64];
		char	large[1280];

		large[0] = 0;
		map_loop
		{
			if (maplist[i].name)
			{
				Com_sprintf(small, sizeof(small), "%s\n", maplist[i].name);
				if (strlen(small) + strlen(large) > sizeof(large) - 100)
				{
					strcat(large, "...\n");
					break;
				}
				else
					strcat(large, small);
			}
			else
				break;
		}
		gi.cprintf(ent, PRINT_HIGH, "%s\nvote <map> : vote for next map\n", large);
		return;
	}

	map_loop
	{
		if (!maplist[i].name)
			break;
		if (stricmp(maplist[i].name, s) == 0)
		{
			int	needed;

			ent->client->resp.vote = i;
			needed = mapCount();
			if (needed)
				gi.bprintf(PRINT_HIGH, "%s votes for %s; %d more votes needed.\n", ent->client->pers.netname, maplist[i].name, needed);
			else
			{
				gi.bprintf(PRINT_HIGH, "%s votes for %s.\n", ent->client->pers.netname, maplist[i].name);
				lame_hack |= end_vote;
			}
			return;
		}
	}
	gi.cprintf(ent, PRINT_HIGH, "Unknown map %s.\n", s);
}

static void mapLight()
{
	int	i;

	map_loop
	{
		if (!maplist[i].name)
			break;
		if (stricmp(level.mapname, maplist[i].name) == 0)
		{
			switch (maplist[i].light)
			{
			case 1:
				gi.configstring(CS_LIGHTS + 0, "a");
				break;
			case 2:
				gi.configstring(CS_LIGHTS + 0, "b");
				break;
			case 3:
				gi.configstring(CS_LIGHTS + 0, "c");
				break;
			case 4:
				gi.configstring(CS_LIGHTS + 0, "d");
				break;
			case 5:
				gi.configstring(CS_LIGHTS + 0, "e");
				break;
			case 6:
				gi.configstring(CS_LIGHTS + 0, "f");
				break;
			case 7:
				gi.configstring(CS_LIGHTS + 0, "g");
				break;
			case 8:
				gi.configstring(CS_LIGHTS + 0, "h");
				break;
			case 9:
				gi.configstring(CS_LIGHTS + 0, "i");
				break;
			case 10:
				gi.configstring(CS_LIGHTS + 0, "j");
				break;
			default:
				gi.configstring(CS_LIGHTS + 0, "m");
				break;
			}

			if (maplist[i].hook)
				lame_hack |= mapnohook;
			else
				lame_hack &= ~mapnohook;

			if (maplist[i].gravity && stricmp(maplist[i].gravity, "0"))
				gi.cvar_set("sv_gravity", maplist[i].gravity);
			else
				gi.cvar_set("sv_gravity", "800");

			return;
		}
	}
	gi.configstring(CS_LIGHTS + 0, "m");
}

void freezeSpawn()
{
	int	i;

	loadMessage();
	loadMap();

	memset(freeze, 0, sizeof(freeze));
	team_loop
		freeze[i].update = true;
	lame_hack &= ~everyone_ready;
	ready_time = far_off;
	gib_queue = 0;

	moan[0] = gi.soundindex("insane/insane1.wav");
	moan[1] = gi.soundindex("insane/insane2.wav");
	moan[2] = gi.soundindex("insane/insane3.wav");
	moan[3] = gi.soundindex("insane/insane4.wav");
	moan[4] = gi.soundindex("insane/insane6.wav");
	moan[5] = gi.soundindex("insane/insane8.wav");
	moan[6] = gi.soundindex("insane/insane9.wav");
	moan[7] = gi.soundindex("insane/insane10.wav");

	mapLight();
	gi.configstring(CS_GENERAL + 5, ">");
}

qboolean freezeMap()
{
	int	i;

	if (!maplist[0].name)
		return false;

	if (lame_hack &end_vote)
	{
		int	j;
		edict_t*	other;
		int	vote_count;
		int	best_count, best_map;

		lame_hack &= ~end_vote;

		best_count = best_map = -1;
		map_loop
		{
			if (!maplist[i].name)
				break;
			vote_count = 0;
			for (j = 0; j < game.maxclients; j++)
			{
				other = g_edicts + 1 + j;
				if (!other->inuse)
					continue;
				if (other->client->resp.vote == i)
					vote_count++;
			}
			if (vote_count >= best_count)
			{
				if (vote_count == best_count && rand() &1)
					continue;
				best_count = vote_count;
				best_map = i;
			}
		}

		if (best_map != -1 && maplist[best_map].name)
		{
			endMapIndex = best_map;
			maplist[best_map].did = true;
			return true;
		}
	}

	if (random_map->value)
	{
		int	count = 0, n;

		map_loop
		{
			if (maplist[i].name)
			{
				if (!maplist[i].did)
					count++;
			}
			else
				break;
		}
		if (count == 0)
		{
			map_loop
				maplist[i].did = false;
			map_loop
			{
				if (maplist[i].name)
					count++;
				else
					break;
			}
		}

		n = (rand() %count) + 1;
		count = 0;
		map_loop
		{
			if (maplist[i].name)
			{
				if (maplist[i].did)
					continue;
				count++;
				if (count == n)
				{
					endMapIndex = i;
					maplist[i].did = true;
					return true;
				}
			}
			else
				break;
		}
	}

	map_loop
	{
		if (!maplist[i].name)
		{
			i--;
			break;
		}
		if (stricmp(level.mapname, maplist[i].name) == 0)
		{
			i++;
			if (i < 63)
			{
				if (!maplist[i].name)
					i = 0;
			}
			break;
		}
	}

	if (i >= 63)
		i = 0;
	if (maplist[i].name)
	{
		endMapIndex = i;
		maplist[i].did = true;
		return true;
	}
	else
		return false;
}

void cvarFreeze()
{
	item_respawn_time = gi.cvar("item_respawn_time", "20", 0);
	hook_max_len = gi.cvar("hook_max_len", "1000", 0);
	hook_rpf = gi.cvar("hook_rpf", "50", 0);
	hook_min_len = gi.cvar("hook_min_len", "40", 0);
	hook_speed = gi.cvar("hook_speed", "1000", 0);
	point_limit = gi.cvar("point_limit", "8", 0);
	new_team_count = gi.cvar("new_team_count", "5", 0);
	frozen_time = gi.cvar("frozen_time", "180", 0);
	start_weapon = gi.cvar("start_weapon", "0", 0);
	start_armor = gi.cvar("start_armor", "0", 0);
	random_map = gi.cvar("random_map", "0", 0);
	vote_percent = gi.cvar("vote_percent", "0", 0);
	use_ready = gi.cvar("use_ready", "0", 0);
	grapple_wall = gi.cvar("grapple_wall", "1", 0);

	lame_hack = 0;
}

void cmdReady(edict_t* ent)
{
	int	i;

	if (!use_ready->value || lame_hack &everyone_ready)
		return;
	if (floodCheck(ent))
		return;
	if (ent->client->resp.spectator)
		return;
	if (freeze[ent->client->resp.team].ready)
	{
		gi.bprintf(PRINT_HIGH, "%s is not ready.\n", ent->client->pers.netname);
		freeze[ent->client->resp.team].ready = false;
		ready_time = far_off;
		return;
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "%s is ready to play.\n", ent->client->pers.netname);
		freeze[ent->client->resp.team].ready = true;
	}
	team_loop
	{
		if (!freeze[i].alive)
			continue;
		if (freeze[i].ready)
			continue;
		ready_time = far_off;
		return;
	}
	ready_time = level.time + 16;
}
