#include "g_local.h"

/*
================
Audio Code
================
*/

#define svc_sound 9

#define MASK_VOLUME 			1
#define MASK_ATTENUATION 		2
#define MASK_POSITION 			4
#define MASK_ENTITY_CHANNEL             8
#define MASK_TIMEOFS 			16	

/*
================
unicastSound - Send a sound to one player (uni, duh).
================
*/
void unicastSound(edict_t *player, int sound_index, float volume)
{
	int mask = MASK_ENTITY_CHANNEL;

	if (volume != 1.0)
		mask |= MASK_VOLUME;

        gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
        gi.WriteByte((byte)sound_index);
	if (mask & MASK_VOLUME)
		gi.WriteByte((byte)(volume * 255));

        gi.WriteShort(((player - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
	gi.unicast (player, true);
}


/*
================
SoundToAllPlayers - what it says
================
*/
void SoundToAllPlayers (int sound_index, float volume)
{
	edict_t *player;
	int i;

	player = g_edicts + 1;

	for (i = 1; i <= game.maxclients; i++)
        {
                if (!player->inuse)
                        continue;

                unicastSound(player, sound_index, volume);

		player = player + 1;
	}
}


/*
================
sexString - find what gender directory we are using
================
*/
char *sexString(edict_t *player)
{
        if (IsFemale(player))
                return "female";
        else
                return "male";
}

#define SCOPE_LOCAL             1
#define SCOPE_TEAM              2
#define SCOPE_TEAMONLY          4
#define SCOPE_NO_POS            8

char *soundPath(edict_t *player, char *filename)
{
        //char buf[256];

        return va("radio/%s/%s.wav", sexString(player), filename);

        /*
        // Dirty - Just to make AQ2's happy...
        //strcpy(buf, "player/");
        strcpy(buf, "radio/"); 
        // Dirty - Just to make AQ2's happy...
        strcat(buf, sexString(player));
        strcat(buf, "/");
        strcat(buf, filename);
        strcat(buf, ".wav");

        return buf;
        */
}

/*
====================
SendAudio - Must specify the file, team and scope
====================
*/
void SendAudio (edict_t *ent, char *filename, int scope, int event) 
{
	edict_t *player;
        char    *final = NULL;
        int     i;
        float   volume = 0.75;

        final = soundPath(ent, filename);

        // Just in case...of err, something??
        if (!final)
                return;

	for (i = 1; i <= game.maxclients; i++)
        {
                player = &g_edicts[i];

                if (!player->inuse)
                        continue;

                if (!player->client)
                        continue;

                if (player == ent)
                {
                        if (scope & SCOPE_TEAMONLY)
                                unicastSound(player, gi.soundindex(final), 1);
                        continue;
                }

                if ((player->solid == SOLID_NOT) || (player->health < 1))
                        continue;

                if ((scope & SCOPE_TEAM) && (!OnSameTeam (ent, player)) && (!AllyCheck (player, ent)))
                        continue;

                if ((scope & SCOPE_LOCAL) && (!InRange (ent, player, 896)))
                        continue;

                //gi.cprintf(player, PRINT_HIGH, "<<%s>>\n", ent->client->pers.netname);

                if (!(scope & SCOPE_NO_POS))
                {
                        if ((player->heard < event) && (level.time - player->last_heard < 15))
                                continue;
                        else
                        {
                                VectorCopy(ent->s.origin, player->audio_pos);
                                player->last_player_heard = ent;
                                player->last_heard = level.time;
                                player->audio = true;
                        }
                }

                unicastSound(player, gi.soundindex(final), 1);
	}

        // Do it locally too unless teamonly sound...
        // Dirty - Adjust volume based on what we're doing
        if (!(scope & SCOPE_TEAMONLY))
        {
                if (ent->light_level < 7) // Trying to hide...so quieter
                        volume -= 0.35;

                if ((!ent->velocity[0]) && (!ent->velocity[1]))
                        volume -= 0.25;

                if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                        volume -= 0.1;
                
                gi.sound(ent, CHAN_AUTO, gi.soundindex(final), volume, ATTN_NORM, 0);
        }
        // Dirty
}

/*
====================
Cmd_Audio_f - Send out a sound manually...
====================
*/
void Cmd_Audio_f (edict_t *ent)
{
        char            *audioCmd;

        if ((ent->solid == SOLID_NOT) || (ent->health < 1))
                return;

        if (!max_teams)
                return;

	audioCmd = gi.args();

	// no argument  
	if (!*audioCmd)
		return;

	// it's the name of a sound, send it

        // Flood protection for those fuck heads we know are out there...
        if (!FloodCheck (ent))
                return;

        SendAudio (ent, audioCmd, (SCOPE_TEAM | SCOPE_NO_POS), -1);
}


/*
====================
NewEvent - Called everytime something happens that could change
           what you would reply...
                eg. getting shot, seeing a teammate go down...
====================
*/

void NewEvent (edict_t *ent, int event)
{
        if (!ent->event_acted)
        {
                if (ent->event < event)
                        return;
        }

        ent->event_acted = false;
        ent->event_time = level.time;
        ent->event = event;

        /*
        gi.dprintf("NewEvent - %i", ent->event);

        if (ent->event == MATE_DOWN)
                gi.dprintf(" MATE_DOWN\n");
        else if (ent->event == HURT)
                gi.dprintf(" HURT\n");
        else if (ent->event == ENEMY_DOWN)
                gi.dprintf(" ENEMY_DOWN\n");
        else if (ent->event == ENEMY_HERE)
                gi.dprintf(" ENEMY_HERE\n");
        else if (ent->event == ENEMY_SPOTTED)
                gi.dprintf(" ENEMY_SPOTTED\n");
        else if (ent->event == TAKING_FIRE)
                gi.dprintf(" TAKING_FIRE\n");
        else if (ent->event == IDLE)
                gi.dprintf(" IDLE\n");
        */
}


/*
====================
NewRequest - Called everytime someone asks something of you
                eg. I need a medic!, I need backup!...

                Unlink NewEvent, you need to include a point for which
                the person to head, if they say yes to your request
====================
*/
void NewRequest (edict_t *ent, int request, vec3_t pos)
{
        if (!ent->request_acted)
        {
                if (ent->request < request)
                        return;
        }

        ent->request_acted = false;
        ent->request_time = level.time;
        ent->request = request;

        // If the request is importantish, keep track of it...
        if (request < REQUEST_PRAISE)
        {
                //gi.dprintf ("request < REQUEST_PRAISE\n");
                VectorCopy(pos, ent->req_pos);
                ent->req = true;
        }

        /*
        gi.dprintf("NewRequest - %i", ent->request);

        if (ent->request == REQUEST_BACK_UP)
                gi.dprintf(" REQUEST_BACK_UP\n");
        else if (ent->request == REQUEST_MEDIC)
                gi.dprintf(" REQUEST_MEDIC\n");
        else if (ent->request == REQUEST_INFO)
                gi.dprintf(" REQUEST_INFO\n");
        else if (ent->request == REQUEST_PRAISE)
                gi.dprintf(" REQUEST_PRAISE\n");
        else if (ent->request == REQUEST_NOTHING)
                gi.dprintf(" REQUEST_NOTHING\n");
        */
}


/*
====================
KilledMakeEvent - Does a scan of ppl who could see this person die,
                then it tell them team/nme down
====================
*/
void KilledMakeEvent (edict_t *attacker, edict_t *targ)
{
        edict_t *player;
        int     i, rng;

        if (!max_teams)
                return;

	for (i = 1; i <= game.maxclients; i++)
        {
                player = &g_edicts[i];

                if (!player->inuse)
                        continue;

                if ((player->solid == SOLID_NOT) || (player->health < 1))
                        continue;

                rng = range (targ, player);

                if (rng > RANGE_NEAR)
                {
                        if (!point_infront (player, targ->s.origin))
                                continue;
                        if ((rng > RANGE_MID) && (targ->light_level < 6))
                                continue;
                }


                if (OnSameTeam (targ, player) || AllyCheck (targ, player))
                        NewEvent (player, MATE_DOWN);
                else
                        NewEvent (player, ENEMY_DOWN);
	}

        attacker->last_killed = level.time;
}


/*
====================
TakingFireCheck - Does a scan of ppl who could see this point,
                if so, they are considered fired apon...
====================
*/
void TakingFireCheck (edict_t *ent, vec3_t point)
{
        edict_t *player;
        int     i, rng;
        vec3_t  v;

        if (!max_teams)
                return;

	for (i = 1; i <= game.maxclients; i++)
        {
                player = &g_edicts[i];

                if (!player->inuse)
                        continue;

                if (ent == player)
                        continue;

                if ((player->movetype == MOVETYPE_NOCLIP) || player->deadflag)
                        continue;

                if (OnSameTeam (ent, player) || AllyCheck (player, ent))
                        continue;

                VectorSubtract (player->s.origin, point, v);
                rng = VectorLength (v);

                if (rng > 128)
                        continue;

                if (rng > 64)
                {
                        if (!point_infront (player, point))
                                continue;
                }

                NewEvent (player, TAKING_FIRE);
	}
}


/*
====================
Cmd_Reply_f - Report your situation or reply to a players request
                eg. Taking fire, wounded, I'm alive etc
====================
*/
void Cmd_Reply_f (edict_t *ent)
{
        char            *sfile = NULL;
        //int             i;

        if (!max_teams)
                return;

        if ((ent->solid == SOLID_NOT) || (ent->health < 1))
                return;

        if (!FloodCheck (ent))
                return;

        if (ent->client->resp.team == NULL)
                return;

        // Reply to requests first, then if we have already replied,
        // or it is way after we should, reset (event = IDLE)
        if (ent->request == REQUEST_INFO)
        {
                ent->event_acted = false;
                ent->request_acted = true;
                ent->request = REQUEST_NOTHING;
        }
        else if (ent->event_acted)
        {
                if (Wounded(ent))
                        ent->event = HURT;
                else
                        ent->event = IDLE;
                ent->event_acted = false;
        }

        if (sfile == NULL)
        {
                // Dirty - compatibility with AQ2 sounds...
                //i = 1 + (rand()&1);
                
                if (ent->event == MATE_DOWN)
                        sfile = "teamdown";
                else if (ent->event == HURT)
                        sfile = "im_hit";
                else if (ent->event == ENEMY_DOWN)
                        sfile = "enemyd";
                else if (ent->event == ENEMY_HERE)
                        sfile = "enemys";
                else if (ent->event == ENEMY_SPOTTED)
                        sfile = "enemys";
                else if (ent->event == TAKING_FIRE)
                        sfile = "taking_f";
                else
                        sfile = "reportin";
                // Dirty - compatibility with AQ2 sounds...
        }

        if (sfile)
        {
                ent->event_acted = true;
                SendAudio (ent, sfile, SCOPE_TEAM, ent->event);
        }
}


/*
====================
Cmd_Request_f - Request something of others.
                eg. Need backup, M E D I C!
====================
*/
void Cmd_Request_f (edict_t *ent)
{
        edict_t *player;
        char    *sfile = NULL;
        int     i, new_request;

        if (!max_teams)
                return;

        if (ent->client->resp.team == NULL)
                return;

        if ((ent->solid == SOLID_NOT) || (ent->health < 1))
                return;

        if (!FloodCheck (ent))
                return;

        new_request = -1;

        if (ent->request_acted || (level.time - ent->request_time > 5))
        {
                ent->request_acted = false;
                ent->request_time = level.time;
                ent->request = REQUEST_NOTHING;
        }

        // Dirty - compatibility with AQ2 sounds...
        //i = 1 + (rand()&1);
        if (ent->request < REQUEST_INFO)
        {
                //sfile = va("yes%i", i);
                sfile = "yes";
                //gi.dprintf ("request < REQUEST_INFO\n");
                VectorCopy(ent->req_pos, ent->goal_pos);
                ent->goal = true;
                ent->req = false;
                ent->request = REQUEST_NOTHING;
                ent->request_acted = true;
                ent->last_player_request = ent->last_player_heard;
                ent->last_player_heard = NULL;
        }
        else if (ent->event == HURT)
        {
                //sfile = "medic";
                sfile = "backup";
                new_request = REQUEST_MEDIC;
        }
        else if (ent->event == ENEMY_DOWN)
        {
                //sfile = va("nme_brag%i", i);
                sfile = "enemyd";
                new_request = REQUEST_PRAISE;
        }
        else if (ent->event < IDLE)
        {
                sfile = "backup";
                new_request = REQUEST_BACK_UP;
        }
        else
        {
                sfile = "treport";
                new_request = REQUEST_INFO;
        }
        // Dirty - compatibility with AQ2 sounds...

        if (sfile)
        {
                ent->request_acted = true;
                SendAudio (ent, sfile, SCOPE_TEAM|SCOPE_NO_POS, -1);
        }

        if (new_request > -1)
        {
                for (i = 1; i <= game.maxclients; i++)
                {
                        player = &g_edicts[i];

                        if (!player->inuse)
                                continue;

                        if ((player->solid == SOLID_NOT) || (player->health < 1))
                                continue;

                        if (player == ent)
                                continue;

                        if ((!OnSameTeam (ent, player)) && (!AllyCheck (player, ent)))
                                continue;

                        NewRequest (player, new_request, ent->s.origin);
                        player->last_player_heard = ent;
                        gi.cprintf(player, PRINT_HIGH, "<<%s>>\n", ent->client->pers.netname);
                }
        }
}

/*
====================
SAY_TEAM
====================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
        {       "item_flag_team1",              1 },
        {       "item_flag_team2",              1 },
        {       "item_flag_team3",              1 },
        {       "item_flag_team4",              1 },
        {       "item_quad",                    2 }, 
	{	"item_invulnerability",		2 },
        {       "weapon_bfg",                   3 },
        {       "weapon_sniper_rifle",          3 },
        {       "weapon_railgun",               4 },
	{	"weapon_rocketlauncher",	4 },
        {       "weapon_hyperblaster",          4 },
        {       "weapon_rifle",                 4 },
        {       "weapon_chaingun",              4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
        {       "weapon_shotgun",               4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
        {       "item_armor_body",              6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
        {       "item_silencer",                7 },
        {       "item_breather",                7 },
        {       "item_enviro",                  7 },
        {       "item_adrenaline",              7 },
        {       "item_bandolier",               8 },
        {       "item_pack",                    8 },
	{ NULL, 0 }
};

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


static void Say_Team_Location(edict_t *who, char *buf)
{
        edict_t         *what = NULL;
        edict_t         *hot = NULL;
        edict_t         *flag;
        gitem_t         *item;
        teams_t         *team = NULL;
        qboolean        hotsee = false;
        qboolean        cansee;
        float           hotdist = 999999, newdist;
        int             hotindex = 999;
        int             nearteam = -1;
        int             i, count;
        vec3_t          v;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL)
        {
                // Ignore dropped items...
                if (what->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                        continue;

		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
                {
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
                }

                if (!loc_names[i].classname)
                        continue;

		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee)
                {
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
                if (newdist < hotdist || (cansee && loc_names[i].priority < hotindex))
                {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot)
        {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
        //if ((ctf->value) && (hot->item) && (hot->item->pickup != CTFPickup_Flag))
        if (ctf->value)
        {
                what = NULL;
                while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL)
                {
                        if (what == hot)
                                continue;
                        // if we are here, there is more than one, find out if hot
                        // is closer to red flag or blue flag
                        hotdist = 4096;

                        count = max_teams;
                        while (count > 0)
                        {
                                flag = G_Find(NULL, FOFS(classname), FlagClassname(GetTeamByIndex(count)));

                                // Ignore dropped flags...
                                //if (flag->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
                                //        continue;

                                if (flag)
                                {
                                        VectorSubtract(hot->s.origin, flag->s.origin, v);
                                        newdist = VectorLength(v);
                                        if (newdist < hotdist)
                                        {
                                                hotdist = newdist;
                                                team = GetTeamByIndex(count);
                                        }
                                }
                                count--;
                        }
                        break;
                }
        }

        if (((item = FindItemByClassname(hot->classname)) == NULL) || (!hot->item))
        {
		strcpy(buf, "nowhere");
		return;
	}

        // in water? in a dark area?
	if (who->waterlevel)
        {
                if ((who->light_level < 6) && (!((int)bflags->value & BF_LIGHTS_OUT)))
                        strcpy(buf, "in the dark water ");
                else
                        strcpy(buf, "in the water ");
        }
        else if ((who->light_level < 6) && (!((int)bflags->value & BF_LIGHTS_OUT)))
                strcpy(buf, "in the dark area ");
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

        if (ctf->value)
        {
                // If near flag, leave it at that...
                if ((hot->item) && (hot->item->pickup == CTFPickup_Flag))
                {
                        team = GetTeamByIndex(hot->item->quantity);
                        strcat(buf, team->name);
                        strcat(buf, "'s flag");
                        return;
                }
                else if (team != NULL)
                {
                        strcat(buf, team->name);
                        strcat(buf, "'s ");
                }
        }
        strcat(buf, hot->item->pickup_name);
}

static void Say_Team_Armor(edict_t *who, char *buf)
{
	*buf = 0;

        if (who->client->pers.armor)
        {
                if (*buf)
                        strcat(buf, "and ");
                sprintf(buf+strlen(buf), "I am wearing an %s", who->client->pers.armor->pickup_name);
        }

	if (!*buf)
		strcpy(buf, "no armor");
}

static void Say_Team_Health(edict_t *who, char *buf)
{
        int wound_level, i;

	if (who->health <= 0)
        {
		strcpy(buf, "dead");
                return;
        }

	*buf = 0;

        i = 0;

        wound_level = Wounded (who);

        // GRIM - I am <whatever> and have...?
                //strcat(buf, ", ");
        if (wound_level > 10)
                sprintf(buf+strlen(buf), "severly wounded and have ");
        else if (wound_level > 5)
                sprintf(buf+strlen(buf), "reasonably wounded and have ");
        else if (wound_level > 0)
                sprintf(buf+strlen(buf), "wounded and have ");
        else
                sprintf(buf+strlen(buf), "not wounded and have ");

        sprintf(buf+strlen(buf), "%i health", who->health);
}

/* GRIM - Fuck it.
static void Say_Team_Tech(edict_t *who, char *buf)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
        while (tnames[i])
        {
                if ((tech = FindItemByClassname(tnames[i])) != NULL && who->client->pers.inventory[ITEM_INDEX(tech)])
                {
			sprintf(buf, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	strcpy(buf, "no powerup");
}
*/

static void Say_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
                strcpy(buf, who->client->pers.weapon->pickup_name);
	else
        {
		strcpy(buf, "none");
                return;
        }

        // GRIM - God, don't you just love two weapons :)
        if ((who->client->pers.weapon_handling) && (who->client->pers.weapon2))
        {
                strcat(buf, " and a ");
                strcat(buf, who->client->pers.weapon2->pickup_name);
        }
}

static void Say_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++)
        {
		targ = g_edicts + i;
                if (!targ->inuse || targ == who || !loc_CanSee(targ, who))
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

	if (*s2)
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
                strcpy(buf, "no one");
}

void Say_Team(edict_t *who, char *msg)
{
	edict_t *cl_ent;
	char outmsg[1024];
	char buf[1024];
	char *p;
	int i;
        qboolean        observers_only = false;

        if (!max_teams)
                return;

        if (turns_on)
        {
                if ((level.match_state < MATCH_START) || (level.match_state == MATCH_FINISHED))
                        observers_only = false;
                else if (who->client->resp.state < CS_PLAYING)
                        observers_only = true;
        }

	outmsg[0] = 0;

	if (*msg == '\"')
        {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++)
        {
		if (*msg == '%')
                {
			switch (*++msg)
                        {
				case 'l' :
				case 'L' :
                                        Say_Team_Location(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'a' :
				case 'A' :
                                        Say_Team_Armor(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
				case 'h' :
				case 'H' :
                                        Say_Team_Health(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
                                /* Fuck it
				case 't' :
				case 'T' :
                                        Say_Team_Tech(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;
                                */
				case 'w' :
				case 'W' :
                                        Say_Team_Weapon(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				case 'n' :
				case 'N' :
                                        Say_Team_Sight(who, buf);
					strcpy(p, buf);
					p += strlen(buf);
					break;

				default :
					*p++ = *msg;
			}
                }
                else
                        *p++ = *msg;
	}
	*p = 0;

        // GRIM - Big thanks to L-Fire for this
        if (strlen(outmsg) > 150)
                outmsg[150] = 0;
        // GRIM - Big thanks to L-Fire for this

	for (i = 0; i < maxclients->value; i++)
        {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

                // GRIM
                if ((!OnSameTeam (cl_ent, who)) && (!AllyCheck (who, cl_ent)))
			continue;

                if (observers_only && (cl_ent->client->resp.state == CS_PLAYING))
                        continue;

                if (observers_only)
                        gi.cprintf(cl_ent, PRINT_CHAT, "([NON]%s): %s\n", who->client->pers.netname, outmsg);
                else
                        gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", who->client->pers.netname, outmsg);
                // GRIM
	}
}
