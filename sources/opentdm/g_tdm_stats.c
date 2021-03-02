/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

//This file handles most stats tracking and some of the teamplayer stuff. Teamplayer
//is a structure used to record who is playing in a match at the match start, so in
//the event of a disconnection or similar, we can still have full info available.

#include "g_local.h"
#include "g_tdm.h"

static const int tdmg_weapons[] =
{
	0,
	0,
	ITEM_WEAPON_BLASTER,
	ITEM_WEAPON_SHOTGUN,
	ITEM_WEAPON_SUPERSHOTGUN,
	ITEM_WEAPON_MACHINEGUN,
	ITEM_WEAPON_CHAINGUN,
	ITEM_AMMO_GRENADES,
	ITEM_WEAPON_GRENADELAUNCHER,
	ITEM_WEAPON_ROCKETLAUNCHER,
	ITEM_WEAPON_HYPERBLASTER,
	ITEM_WEAPON_RAILGUN,
	ITEM_WEAPON_BFG,
};

//Looukp table  for meansOfDeath. MUST BE KEPT IN
//SYNC WITH MOD_*!!
static const int meansOfDeathToTDMG[] =
{
	0,
	TDMG_BLASTER,
	TDMG_SHOTGUN,
	TDMG_SSHOTGUN,
	TDMG_MACHINEGUN,
	TDMG_CHAINGUN,
	TDMG_GRENADELAUNCHER,
	TDMG_GRENADELAUNCHER,
	TDMG_ROCKETLAUNCHER,
	TDMG_ROCKETLAUNCHER,
	TDMG_HYPERBLASTER,
	TDMG_RAILGUN,
	TDMG_BFG10K,
	TDMG_BFG10K,
	TDMG_BFG10K,
	TDMG_HANDGRENADE,
	TDMG_HANDGRENADE,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
	0,
	TDMG_WORLD,
	0,
	TDMG_HANDGRENADE,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
	0,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
	TDMG_WORLD,
};

typedef struct
{
	float	percentage;
	int		index;
} statsort_t;

int TDM_PercentageSort (void const *a, void const *b)
{
	statsort_t	*a1 = (statsort_t *)a;
	statsort_t	*b1 = (statsort_t *)b;

	if (a1->percentage > b1->percentage)
		return -1;
	else if (a1->percentage < b1->percentage)
		return 1;

	return 0;
}

typedef struct
{
	int		amount;
	int		index;
} intsort_t;

int TDM_DamageSort (void const *a, void const *b)
{
	intsort_t	*a1 = (intsort_t *)a;
	intsort_t	*b1 = (intsort_t *)b;

	if (a1->amount > b1->amount)
		return -1;
	else if (a1->amount < b1->amount)
		return 1;

	return 0;
}

/*
==============
TDM_WeaponFired
==============
Client fired a weapon, 
*/
void TDM_WeaponFired (edict_t *ent)
{
	int	i;

	//ignore warmup
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	if (!ent->client->resp.teamplayerinfo)
		TDM_Error ("TDM_WeaponFired: Trying to track stats but no teamplayerinfo for client %s", ent->client->pers.netname);

	//FIXME optimize this, chaingun can call several times per frame.
	for (i = 0; i < sizeof(tdmg_weapons) / sizeof(tdmg_weapons[0]); i++)
	{
		if (tdmg_weapons[i] == ITEM_INDEX(ent->client->weapon))
		{
			ent->client->resp.teamplayerinfo->shots_fired[i]++;
			break;
		}
	}
}

/*
==============
TDM_BeginDamage
==============
Begin a sequence of possibly multiple damage, we only want to count one though for the weapon.
We are called like so - TDM_BeginDamage -> TDM_Damage(multiple) -> TDM_EndDamage (stop tracking)
*/
static qboolean	weapon_hit;

void TDM_BeginDamage (void)
{
	weapon_hit = false;
}

void TDM_Damage (edict_t *ent, edict_t *victim, edict_t *inflictor, int damage)
{
	int		weapon;

	//Ignore warmup
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	//Determine what weapon was used
	weapon = meansOfDeathToTDMG[meansOfDeath &~ MOD_FRIENDLY_FIRE];

	//Not something we care about
	if (!weapon)
		return;

	//No credit for destroying corpses in progress
	if (victim->deadflag)
		return;

	//For damage tracking we DO want to count every shot
	if (!victim->client->resp.teamplayerinfo)
		TDM_Error ("TDM_Damage: Trying to track stats but no teamplayerinfo for victim %s, attacker %s", victim->client->pers.netname, ent->client->pers.netname);

	if (!ent->client->resp.teamplayerinfo)
		TDM_Error ("TDM_Damage: Trying to track stats but no teamplayerinfo for attacker %s, victim %s", ent->client->pers.netname, victim->client->pers.netname);

	//Shooting teammates is not accurate! just count team damage
	if (ent->client->pers.team == victim->client->pers.team && ent != victim)
	{
		ent->client->resp.teamplayerinfo->team_dealt += damage;
		victim->client->resp.teamplayerinfo->team_recvd += damage;
		return;
	}

	victim->client->resp.teamplayerinfo->damage_received[weapon] += damage;
	
	//Hitting yourself doesn't count for dealt or accuracy!
	if (ent == victim)
		return;

	ent->client->resp.teamplayerinfo->damage_dealt[weapon] += damage;

	//wision: count quad damage
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->resp.teamplayerinfo->quad_dealt += damage;
		victim->client->resp.teamplayerinfo->quad_recvd += damage;
	}

	//wision: count pent damage
	if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->resp.teamplayerinfo->pent_dealt += damage;
		victim->client->resp.teamplayerinfo->pent_recvd += damage;
	}

	//Only count one hit, eg shotgun or rocket radius damage. apparently railgun should ignore this rule.
	if (weapon_hit && weapon != TDMG_RAILGUN)
		return;

	//Add stats
	ent->client->resp.teamplayerinfo->shots_hit[weapon]++;

	//Hitsound
	//G_UnicastSound (ent, gi.soundindex ("parasite/paridle1.wav"), false);

	weapon_hit = true;
}

void TDM_EndDamage (void)
{
	//so future TDM_Damages from without a begindamage are ignored
	weapon_hit = true;
}

/*
==============
TDM_Killed
==============
Someone was killed. Track weapon used for stats purposes.
*/
void TDM_Killed (edict_t *attacker, edict_t *victim, int mod)
{
	int	tdmg;

	//Ignore warmup
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return;

	//safety check, T_Damage is a scary mess of attackers and inflictors and im not
	//100% sure which ones are allowed to be null
	if (!attacker)
		return;

	//don't count suicides here
	if (attacker == victim)
		return;

	//ignore non-player weapons
	if (!attacker->client)
		return;

	tdmg = meansOfDeathToTDMG[mod];

	attacker->client->resp.teamplayerinfo->killweapons[tdmg]++;
	victim->client->resp.teamplayerinfo->deathweapons[tdmg]++;

	//wision: count quad frags/deaths
	if (attacker->client->quad_framenum > level.framenum)
	{
		attacker->client->resp.teamplayerinfo->quad_kills++;
		victim->client->resp.teamplayerinfo->quad_deaths++;
	}

	//wision: count pent frags/deaths
	if (attacker->client->invincible_framenum > level.framenum)
	{
		attacker->client->resp.teamplayerinfo->pent_kills++;
		victim->client->resp.teamplayerinfo->pent_deaths++;
	}
}

/*
==============
TDM_GetInfoForPlayer
==============
Searches matchinfo for a player matching gi.args.
*/
teamplayer_t *TDM_GetInfoForPlayer (edict_t *ent, matchinfo_t *matchinfo)
{
	teamplayer_t	*victim, *info;
	int				count;

	if (!matchinfo->teamplayers)
	{
		gi.cprintf (ent, PRINT_HIGH, "That information is not available yet.\n");
		return NULL;
	}
	
	victim = NULL;

	count = matchinfo->num_teamplayers;
	info = matchinfo->teamplayers;

	if (gi.argc() < 2)
	{
		int		i;

		//viewing chasee stats, note we can't just use chase_target->teamplayerinfo since
		//we could be looking up oldstats, so search the matchinfo teamplayers to see if
		//this player still has a link.
		if (ent->client->chase_target)
		{
			for (i = 0; i < count; i++)
			{
				if (info[i].client == ent->client->chase_target)
				{
					victim = info + i;
					return victim;
				}
			}
		}

		//checking their own stats
		for (i = 0; i < count; i++)
		{
			if (info[i].client == ent)
			{
				victim = info + i;
				return victim;
			}
		}

		//couldn't guess, ask them to specify
		if (!victim)
		{
			gi.cprintf (ent, PRINT_HIGH, "Usage: %s <name/id>\n", gi.argv(0));
			return victim;
		}
	}
	else
	{
		edict_t	*client;
		int		i;

		//match on existing player who still has a link to their stats
		if (LookupPlayer (gi.args(), &client, NULL))
		{
			for (i = 0; i < count; i++)
			{
				if (info[i].client == client)
				{
					victim = info + i;
					return victim;
				}
			}
		}

		//match on saved names
		if (!victim)
		{
			for (i = 0; i < count; i++)
			{
				if (!Q_stricmp (info[i].name, gi.args()))
				{
					victim = info + i;
					return victim;
				}
			}				
		}

		//match on partial lowered saved names
		if (!victim)
		{
			char	lowered_player[16];
			char	lowered_args[16];

			Q_strncpy (lowered_args, gi.args(), sizeof(lowered_args)-1);
			Q_strlwr (lowered_args);

			for (i = 0; i < count; i++)
			{
				strcpy (lowered_player, info[i].name);
				Q_strlwr (lowered_player);

				if (strstr (lowered_player, lowered_args))
				{
					victim = info + i;
					return victim;
				}
			}				
		}

		//well, we tried our best.
		if (!victim)
		{
			char	participants[1024];

			participants[0] = '\0';

			for (i = 0; i < count; i++)
			{
				if (participants[0])
					strcat (participants, ", ");

				strcat (participants, info[i].name);
			}

			gi.cprintf (ent, PRINT_HIGH, "No match for '%s'. Players in the match: %s\n", gi.args(), participants);
			return victim;
		}
	}

	return victim;
}

/*
==============
TDM_GetTeamFromMatchInfo
==============
Searches matchinfo for team of a player, or searches for a team based on gi.args.
*/
int TDM_GetTeamFromMatchInfo (edict_t *ent, matchinfo_t *matchinfo)
{
	teamplayer_t	*victim, *info;
	int				count;

	if (!matchinfo->teamplayers)
	{
		gi.cprintf (ent, PRINT_HIGH, "That information is not available yet.\n");
		return -1;
	}
	
	victim = NULL;

	count = matchinfo->num_teamplayers;
	info = matchinfo->teamplayers;

	if (gi.argc() < 2)
	{
		int		i;

		//viewing own stats mid-game
		if (ent->client->resp.teamplayerinfo)
			return ent->client->resp.teamplayerinfo->team;

		//viewing chasee stats - mmmmmonster deref!
		if (ent->client->chase_target && ent->client->chase_target->client->resp.teamplayerinfo)
			return ent->client->chase_target->client->resp.teamplayerinfo->team;

		//finished game, checking own stats
		for (i = 0; i < count; i++)
		{
			if (info[i].client == ent)
			{
				victim = info + i;
				return victim->team;
			}
		}

		if (!victim)
		{
			gi.cprintf (ent, PRINT_HIGH, "Usage: %s <teamname/id>\n", gi.argv(0));
			return -1;
		}
	}
	else
	{
		int		i;

		//match on a direct numeric team id or name to avoid conflicting with partial player name match below
		i = TDM_GetTeamFromArg (ent, gi.args());

		//ignore spec/invalid response
		if (i == TEAM_A || i == TEAM_B)
			return i;

		//match on saved names
		if (!victim)
		{
			for (i = 0; i < count; i++)
			{
				if (!Q_stricmp (info[i].name, gi.args()))
				{
					victim = info + i;
					return victim->team;
				}
			}				
		}

		//match on partial lowered saved names
		if (!victim)
		{
			char	lowered_player[16];
			char	lowered_args[16];

			Q_strncpy (lowered_args, gi.args(), sizeof(lowered_args)-1);
			Q_strlwr (lowered_args);

			for (i = 0; i < count; i++)
			{
				strcpy (lowered_player, info[i].name);
				Q_strlwr (lowered_player);

				if (strstr (lowered_player, lowered_args))
				{
					victim = info + i;
					return victim->team;
				}
			}				
		}

		gi.cprintf (ent, PRINT_HIGH, "Team '%s' not found. Use either the name of a player to look up their team, or use team 1 or 2.\n", gi.args());
	}

	return -1;
}

/*
==============
TDM_StatCheatCheck
==============
Returns true if someone in a match is trying to view stats of the other team. Prints
a message to ent if they are doing such.
*/
qboolean TDM_StatCheatCheck (edict_t *ent, matchinfo_t *info, unsigned team)
{
	if (info != &old_matchinfo && tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD &&
		ent->client->pers.team && ent->client->pers.team != team)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can only see stats for the other team after the match has finished.\n");
		return true;
	}

	return false;
}

/*
==============
TDM_BuildWeaponsStatsString
==============
Builds weapons stats string.
*/
char *TDM_BuildWeaponsStatsString (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info)
{
	static char		stats[1400];
	unsigned		i;
	char			frags[6], deaths[6], dealt[6], recvd[6], missed[6], acc[6];
	qboolean		basic;

	stats[0] = 0;

	basic = (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD) && ent->client->pers.team && (m_info == &current_matchinfo);

	//wision: weapon stats
	for (i = TDMG_BLASTER; i < TDMG_MAX; i++)
	{
		const gitem_t	*item;

		//don't draw some stuff
		if (!p_info->shots_fired[i] && !p_info->items_collected[tdmg_weapons[i]] && i > TDMG_BLASTER && !p_info->damage_received[i] &&
				m_info->game_mode != GAMEMODE_ITDM && !p_info->items_collected[tdmg_weapons[i]])
			continue;

		item = GETITEM (tdmg_weapons[i]);

		//don't print missed during the match
		if (basic)
			sprintf (missed, "   -");
		else
			sprintf (missed, "%4d", m_info->item_spawn_count[tdmg_weapons[i]] - p_info->items_collected[tdmg_weapons[i]]);

		// display these stats only for weapons where player fired something
		if (p_info->shots_fired[i])
		{
			sprintf (acc, "%3.0f%%", ceil (((float)p_info->shots_hit[i] / (float)p_info->shots_fired[i]) * 100));
			sprintf (dealt, "%5d", p_info->damage_dealt[i]);
			sprintf (frags, "%5d", p_info->killweapons[i]);
		}
		// else display '-' instead of 0%
		else
		{
			sprintf (acc, "   -");
			sprintf (dealt, "    -");
			sprintf (frags, "    -");
		}

		// display these stats only for weapons where we got hurt
		if (p_info->damage_received[i])
		{
			sprintf (recvd, "%5d", p_info->damage_received[i]);
			sprintf (deaths, "%5d", p_info->deathweapons[i]);
		}
		else
		{
			sprintf (recvd, "    -");
			sprintf (deaths, "    -");
		}

		strcat (stats, va ("%16.16s | %s %s  %s %s %s %4d %s\n",
					item->pickup_name,
					acc,
					frags,
					deaths,
					dealt,
					recvd,
					p_info->items_collected[tdmg_weapons[i]],
					missed
					));
	}

	return stats;
}

/*
==============
TDM_BuildTeamWeaponsStatsString
==============
Builds team weapons stats string.
*/
char *TDM_BuildTeamWeaponsStatsString (edict_t *ent, matchinfo_t *m_info, unsigned team)
{
	static char		stats[1400];
	unsigned		i, j;
	int				frags_i, deaths_i, dealt_i, recvd_i, picked_i, total_hit_i, total_shots_i;
	char			frags[6], deaths[6], dealt[6], recvd[6], missed[6], acc[6];
	teamplayer_t	*p_info;
	qboolean		basic;
	qboolean		skip;

	stats[0] = 0;

	basic = (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD) && ent->client->pers.team && (m_info == &current_matchinfo);

	//wision: weapon stats
	for (i = TDMG_BLASTER; i < TDMG_MAX; i++)
	{
		const gitem_t	*item;

		frags_i = deaths_i = dealt_i = recvd_i = picked_i = total_hit_i = total_shots_i = 0;
		skip = true;

		for (j = 0; j < m_info->num_teamplayers; j++)
		{
			p_info = &(m_info->teamplayers[j]);
			if (p_info->team == team)
			{
				//don't draw some stuff
				if (!p_info->shots_fired[i] && !p_info->items_collected[tdmg_weapons[i]] &&
						i > TDMG_BLASTER && !p_info->damage_received[i] && m_info->game_mode != GAMEMODE_ITDM &&
						!p_info->items_collected[tdmg_weapons[i]])
					continue;

				skip = false;

				total_shots_i += p_info->shots_fired[i];
				total_hit_i += p_info->shots_hit[i];
				picked_i += p_info->items_collected[tdmg_weapons[i]];
				frags_i += p_info->killweapons[i];
				deaths_i += p_info->deathweapons[i];
				dealt_i += p_info->damage_dealt[i];
				recvd_i += p_info->damage_received[i];
			}
		}

		// we don't need to draw this item
		if (skip)
			continue;

		item = GETITEM (tdmg_weapons[i]);

		//don't print missed during the match
		if (basic)
			sprintf (missed, "   -");
		else
			sprintf (missed, "%4d", m_info->item_spawn_count[tdmg_weapons[i]] - picked_i);

		// display these stats only for weapons where player fired something
		if (total_shots_i)
		{
			sprintf (acc, "%3.0f%%", ceil (((float)total_hit_i / (float)total_shots_i) * 100));
			sprintf (dealt, "%5d", dealt_i);
			sprintf (frags, "%5d", frags_i);
		}
		// else display '-' instead of 0%
		else
		{
			sprintf (acc, "   -");
			sprintf (dealt, "    -");
			sprintf (frags, "    -");
		}

		// display these stats only for weapons where we got hurt
		if (recvd_i)
		{
			sprintf (recvd, "%5d", recvd_i);
			sprintf (deaths, "%5d", deaths_i);
		}
		else
		{
			sprintf (recvd, "    -");
			sprintf (deaths, "    -");
		}

		strcat (stats, va ("%16.16s | %s %s  %s %s %s %4d %s\n",
					item->pickup_name,
					acc,
					frags,
					deaths,
					dealt,
					recvd,
					picked_i,
					missed
					));
	}

	return stats;
}

/*
==============
TDM_BuildItemsStatsString
==============
Builds items stats string.
*/
char *TDM_BuildItemsStatsString (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info)
{
	static char		stats[1400];
	unsigned		i;
	char			frags[6], deaths[6], dealt[6], recvd[6], missed[6];
	qboolean		basic;

	stats[0] = 0;

	basic = (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD) && ent->client->pers.team && (m_info == &current_matchinfo);

	//wision: items stats
	for (i = 1; i < game.num_items; i++)
	{
		const gitem_t	*item;

		//skip weapons
		if ((i >= ITEM_WEAPON_BLASTER && i <= ITEM_AMMO_SLUGS) || !p_info->items_collected[i])
			continue;

		item = GETITEM (i);

		// display damage / frags / deaths stats for quad
		if (i == ITEM_ITEM_QUAD)
		{
			sprintf (dealt, "%5d", p_info->quad_dealt);
			sprintf (recvd, "%5d", p_info->quad_recvd);
			sprintf (frags, "%5d", p_info->quad_kills);
			sprintf (deaths, "%5d", p_info->quad_deaths);
		}
		// or display damage / frags / deaths stats for invulnerability
		else if (i == ITEM_ITEM_INVULNERABILITY)
		{
			sprintf (dealt, "%5d", p_info->pent_dealt);
			sprintf (recvd, "%5d", p_info->pent_recvd);
			sprintf (frags, "%5d", p_info->pent_kills);
			sprintf (deaths, "%5d", p_info->pent_deaths);
		}
		// or don't display anything
		else
		{
			sprintf (dealt, "    -");
			sprintf (recvd, "    -");
			sprintf (frags, "    -");
			sprintf (deaths, "    -");
		}

		//don't print missed during the match
		if (basic)
			sprintf (missed, "   -");
		else
			sprintf (missed, "%4d", m_info->item_spawn_count[i] - p_info->items_collected[i]);

		strcat (stats, va ("%16.16s |    - %s  %s %s %s %4d %s\n",
					((i == ITEM_ITEM_HEALTH) ? va ("MegaHealth") : item->pickup_name),
					frags,
					deaths,
					dealt,
					recvd,
					p_info->items_collected[i],
					missed
					));
	}

	return stats;
}

/*
==============
TDM_BuildTeamItemsStatsString
==============
Builds team items stats string.
*/
char *TDM_BuildTeamItemsStatsString (edict_t *ent, matchinfo_t *m_info, unsigned team)
{
	static char		stats[1400];
	unsigned		i, j;
	int				frags_i, deaths_i, dealt_i, recvd_i, picked_i;
	char			frags[6], deaths[6], dealt[6], recvd[6], missed[6];
	teamplayer_t	*p_info;
	qboolean		skip;
	qboolean		basic;

	stats[0] = 0;

	basic = (tdm_match_status >= MM_PLAYING && tdm_match_status != MM_SCOREBOARD) && ent->client->pers.team && (m_info == &current_matchinfo);

	//wision: items stats
	for (i = 1; i < game.num_items; i++)
	{
		const gitem_t	*item;

		frags_i = deaths_i = dealt_i = recvd_i = picked_i = 0;
		skip = true;

		for (j = 0; j < m_info->num_teamplayers; j++)
		{
			p_info = &(m_info->teamplayers[j]);
			if (p_info->team == team)
			{
				if ((i >= ITEM_WEAPON_BLASTER && i <= ITEM_AMMO_SLUGS) || !p_info->items_collected[i])
					continue;

				skip = false;

				if (i == ITEM_ITEM_QUAD)
				{
					dealt_i += p_info->quad_dealt;
					recvd_i += p_info->quad_recvd;
					frags_i += p_info->quad_kills;
					deaths_i += p_info->quad_deaths;
				}
				else if (i == ITEM_ITEM_INVULNERABILITY)
				{
					dealt_i += p_info->pent_dealt;
					recvd_i += p_info->pent_recvd;
					frags_i += p_info->pent_kills;
					deaths_i += p_info->pent_deaths;
				}

				picked_i += p_info->items_collected[i];
			}
		}

		if (skip)
			continue;

		item = GETITEM (i);

		// display damage / frags / deaths stats for quad or invulnerability
		if (i == ITEM_ITEM_QUAD || i == ITEM_ITEM_INVULNERABILITY)
		{
			sprintf (dealt, "%5d", dealt_i);
			sprintf (recvd, "%5d", recvd_i);
			sprintf (frags, "%5d", frags_i);
			sprintf (deaths, "%5d", deaths_i);
		}
		// or don't display anything
		else
		{
			sprintf (dealt, "    -");
			sprintf (recvd, "    -");
			sprintf (frags, "    -");
			sprintf (deaths, "    -");
		}

		//don't print missed during the match
		if (basic)
			sprintf (missed, "   -");
		else
			sprintf (missed, "%4d", m_info->item_spawn_count[i] - picked_i);

		strcat (stats, va ("%16.16s |    - %s  %s %s %s %4d %s\n",
					((i == ITEM_ITEM_HEALTH) ? va ("MegaHealth") : item->pickup_name),
					frags,
					deaths,
					dealt,
					recvd,
					picked_i,
					missed
					));
	}

	return stats;
}

/*
==============
TDM_WeaponsStats_f
==============
Show weapons stats.
*/
/*void TDM_WeaponsStats_f (edict_t *ent, matchinfo_t *m_info)
{
	char			*extra;
	static char		stats[1024];
	teamplayer_t	*p_info;

	stats[0] = 0;

	if (tdm_match_status == MM_WARMUP)
		m_info = &old_matchinfo;

	p_info = TDM_GetInfoForPlayer (ent, m_info);

	if (!p_info)
		return;

	if (TDM_StatCheatCheck (ent, m_info, p_info->team))
		return;

	strcat (stats, va("Stats for %s:\n", p_info->name));
	strcat (stats, va ("          %s | ", TDM_SetColorText (va ("Weapon"))));
	strcat (stats, va ("%s\n", TDM_SetColorText (va ("Acc. Kills Deaths Dealt Recvd Pick Miss"))));
	strcat (stats, va ("-----------------+----------------------------------------\n"));

	extra = TDM_BuildWeaponsStatsString (ent, m_info);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}*/

/*
==============
TDM_TeamWeaponsStats_f
==============
Show team weapons stats.
*/
/*void TDM_TeamWeaponsStats_f (edict_t *ent, matchinfo_t *m_info)
{
	int				team;
	char			*extra;
	static char		stats[1024];

	stats[0] = 0;

	if (tdm_match_status == MM_WARMUP)
		m_info = &old_matchinfo;

	team = TDM_GetTeamFromMatchInfo (ent, m_info);

	if (team == -1)
		return;

	if (TDM_StatCheatCheck (ent, m_info, team))
		return;

	strcat (stats, va ("Team '%s':\n", m_info->teamnames[team]));
	strcat (stats, va ("          %s | ", TDM_SetColorText (va ("Weapon"))));
	strcat (stats, va ("%s\n", TDM_SetColorText (va ("Acc. Kills Deaths Dealt Recvd Pick Miss"))));
	strcat (stats, va ("-----------------+----------------------------------------\n"));

	extra = TDM_BuildTeamWeaponsStatsString (ent, m_info, team);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}*/

/*
==============
TDM_ItemsStats_f
==============
Show items stats.
*/
void TDM_ItemsStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info)
{
	char			*extra;
	static char		stats[1024];

	stats[0] = 0;

	strcat (stats, va("Stats for %s:\n\n", p_info->name));
	strcat (stats, va ("            %s | ", TDM_SetColorText (va ("Item"))));
	strcat (stats, va ("%s\n", TDM_SetColorText (va ("Acc. Kills Deaths Dealt Recvd Pick Miss"))));
	strcat (stats, va ("-----------------+----------------------------------------\n"));

	extra = TDM_BuildWeaponsStatsString (ent, m_info, p_info);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	extra = TDM_BuildItemsStatsString (ent, m_info, p_info);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}

/*
==============
TDM_TeamItemsStats_f
==============
Show items info.
*/
void TDM_TeamItemsStats_f (edict_t *ent, matchinfo_t *m_info, int team)
{
	char			*extra;
	static char		stats[1024];

	stats[0] = 0;

	strcat (stats, va ("Team '%s':\n\n", m_info->teamnames[team]));
	strcat (stats, va ("            %s | ", TDM_SetColorText (va ("Item"))));
	strcat (stats, va ("%s\n", TDM_SetColorText (va ("Acc. Kills Deaths Dealt Recvd Pick Miss"))));
	strcat (stats, va ("-----------------+----------------------------------------\n"));

	extra = TDM_BuildTeamWeaponsStatsString (ent, m_info, team);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	extra = TDM_BuildTeamItemsStatsString (ent, m_info, team);

	//need to flush?
	if (strlen(stats) + 1 + strlen (extra) >= sizeof(stats)-1)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
		stats[0] = 0;
	}
	
	strcat (stats, extra);

	gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}

/*
==============
TDM_GeneralStats_f
==============
Display general stats for player.
*/
void TDM_GeneralStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info, unsigned team)
{
	static char		stats[1024];
	int				i, j, p_recvd, p_dealt;
	int				team_kills, team_deaths, team_suicides, team_team_kills;
	int				team_telefrags, team_dealt, team_recvd, team_t_dealt, team_t_recvd;

	team_kills = team_deaths = team_suicides = team_team_kills = team_telefrags = 0;
	team_dealt = team_recvd = team_t_dealt = team_t_recvd = p_recvd = p_dealt = 0;
	stats[0] = 0;

	strcat (stats, va ("\n"));
	strcat (stats, TDM_SetColorText (va ("Total")));
	strcat (stats, va ("  | %s\n", TDM_SetColorText (va ("Kills Dths Sui TKs Tele Dealt Recvd TDealt TRecvd"))));
	strcat (stats, va ("-------+--------------------------------------------------\n"));

	if (p_info)
	{
		for (i = TDMG_BLASTER; i < TDMG_MAX; i++)
		{
			p_dealt += p_info->damage_dealt[i];
			p_recvd += p_info->damage_received[i];
		}

		strcat (stats, va ("Player | %5d %4d %3d %3d %4d %5d %5d %6d %6d\n",
					p_info->enemy_kills,
					p_info->deaths,
					p_info->suicides,
					p_info->team_kills,
					p_info->telefrags,
					p_dealt,
					p_recvd,
					p_info->team_dealt,
					p_info->team_recvd));
	}

	if (!m_info->is1v1)
	{
		for (j = 0; j < m_info->num_teamplayers; j++)
		{
			p_info = &(m_info->teamplayers[j]);

			if (p_info->team != team)
				continue;

			team_kills += p_info->enemy_kills;
			team_deaths += p_info->deaths;
			team_suicides += p_info->suicides;
			team_team_kills += p_info->team_kills;
			team_telefrags += p_info->telefrags;
			team_t_dealt += p_info->team_dealt;
			team_t_recvd += p_info->team_recvd;

			for (i = TDMG_BLASTER; i < TDMG_MAX; i++)
			{
				team_dealt += p_info->damage_dealt[i];
				team_recvd += p_info->damage_received[i];
			}
		}

		strcat (stats, va ("Team   | %5d %4d %3d %3d %4d %5d %5d %6d %6d\n",
					team_kills,
					team_deaths,
					team_suicides,
					team_team_kills,
					team_telefrags,
					team_dealt,
					team_recvd,
					team_t_dealt,
					team_t_recvd));
	}
	strcat (stats, va ("\n"));

	gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}

/*
==============
TDM_Stats_f
==============
Shows shitloads of useless information. Note it isn't hard to
overflow the 1024 byte cprintf limit, which is why this is
split into multiple cprints. On non-R1Q2 servers, this will
probably cause overflows!
*/
void TDM_Stats_f (edict_t *ent, matchinfo_t *m_info)
{
	int				i;
	teamplayer_t	*p_info = NULL;

	// after match stats.. don't guess the player, just pick the ent
	if (tdm_match_status == MM_SCOREBOARD)
	{
		for (i = 0; i < m_info->num_teamplayers; i++)
			if (m_info->teamplayers[i].client == ent)
				p_info = &(m_info->teamplayers[i]);

	}
	// find a player for stats
	else
	{
		if (tdm_match_status == MM_WARMUP)
			m_info = &old_matchinfo;

		p_info = TDM_GetInfoForPlayer (ent, m_info);

		if (!p_info)
			return;
	
		if (TDM_StatCheatCheck (ent, m_info, p_info->team))
			return;
	}

	TDM_ItemsStats_f (ent, m_info, p_info);
	TDM_GeneralStats_f (ent, m_info, p_info, p_info->team);
}

/*
==============
TDM_TeamStats_f
==============
Shows team statistics.
*/
void TDM_TeamStats_f (edict_t *ent, matchinfo_t *m_info)
{
	if (m_info->is1v1)
		TDM_Stats_f (ent, m_info);
	else
	{
		int				team;

		if (tdm_match_status == MM_WARMUP)
			m_info = &old_matchinfo;

		team = TDM_GetTeamFromMatchInfo (ent, m_info);

		if (team == -1)
			return;
	
		if (TDM_StatCheatCheck (ent, m_info, team))
			return;

		TDM_TeamItemsStats_f (ent, m_info, team);
		TDM_GeneralStats_f (ent, m_info, NULL, team);
	}

}

/*
==============
TDM_RemoveStatsLink
==============
A player just disconnected, so remove the pointer to their edict
from the stats info to avoid new clients taking the same ent and
messing things up.
*/
void TDM_RemoveStatsLink (edict_t *ent)
{
	int		i;

	if (current_matchinfo.teamplayers)
		for (i = 0; i < current_matchinfo.num_teamplayers; i++)
		{
			if (current_matchinfo.teamplayers[i].client == ent)
				current_matchinfo.teamplayers[i].client = NULL;
		}

	if (old_matchinfo.teamplayers)
		for (i = 0; i < old_matchinfo.num_teamplayers; i++)
		{
			if (old_matchinfo.teamplayers[i].client == ent)
				old_matchinfo.teamplayers[i].client = NULL;
		}
}

/*
==============
TDM_BuildTopBottomShotsString
==============
Builds the string for top/bottom shots.
*/
char* TDM_BuildTopBottomShotsString (edict_t *ent, int team, qboolean top_shots)
{
	static char		stats[1400];
	unsigned		i, j;
	float			accuracy;
	int				frags, deaths, hits, shots;
	char			player[16];
	matchinfo_t		*m_info;
	teamplayer_t	*p_info;
	qboolean		skip;

	m_info = &old_matchinfo;
	stats[0] = 0;

	if (!m_info->teamplayers)
	{
		gi.cprintf (ent, PRINT_HIGH, "That information is not available yet.\n");
		return stats;
	}

	if (team == -1)
		strcat (stats, va ("Total %sshots:\n\n", ((top_shots) ? "top" : "bottom")));
	else
		strcat (stats, va ("Team '%s' %sshots:\n\n", old_matchinfo.teamnames[team], ((top_shots) ? "top" : "bottom")));
	strcat (stats, va ("          %s | ", TDM_SetColorText (va ("Weapon"))));
	strcat (stats, va ("  %s\n", TDM_SetColorText (va ("Acc. Hits Shots Kills Deaths Name"))));
	strcat (stats, va ("-----------------+--------------------------------------\n"));

	for (i = TDMG_BLASTER; i < TDMG_MAX; i++)
	{
		const gitem_t	*item;

		frags = deaths = hits = shots = 0;
		skip = true;

		if (top_shots)
			accuracy = 0.0f;
		else
			accuracy = 100.0f;

		for (j = 0; j < m_info->num_teamplayers; j++)
		{
			p_info = &(m_info->teamplayers[j]);

			if ((int)p_info->team == team || team == -1)
			{
				//don't count where we didn't hit anything
				if (!p_info->shots_hit[i])
					continue;

				//this player has lower/higher accuracy.. skip him
				//check also for minimun shots fired which doesn't apply on bfg
				if ((top_shots && accuracy > (float)p_info->shots_hit[i]/(float)p_info->shots_fired[i]) ||
						(!top_shots && accuracy < (float)p_info->shots_hit[i]/(float)p_info->shots_fired[i]) ||
						shots > 10 * p_info->shots_fired[i] ||
						(p_info->shots_fired[i] < 10 && i != TDMG_BFG10K))
					continue;

				skip = false;

				shots = p_info->shots_fired[i];
				hits = p_info->shots_hit[i];
				frags = p_info->killweapons[i];
				deaths = p_info->deathweapons[i];

				//already shouldn't be more than 15!
				strcpy (player, p_info->name);
				accuracy = (float)hits/(float)shots;
			}
		}

		// we don't need to draw this weapon
		if (skip)
			continue;

		item = GETITEM (tdmg_weapons[i]);

		strcat (stats, va ("%16.16s | %5.1f%% %4d %5d %5d %6d %s\n",
					item->pickup_name,
					accuracy * 100,
					hits,
					shots,
					frags,
					deaths,
					player
					));
	}

	return stats;
}

/*
==============
TDM_TopBottomShots_f
==============
Show Top/Bottom Shots info. Only available after the match.
*/
void TDM_TopBottomShots_f (edict_t *ent, qboolean team_stats, qboolean top_shots)
{
	const char	*stats;
	int			team = -1;

	if (team_stats)
	{
		team = TDM_GetTeamFromMatchInfo (ent, &old_matchinfo);
		if (team == -1)
			return;
	}

	stats = TDM_BuildTopBottomShotsString (ent, team, top_shots);

	if (stats[0])
		gi.cprintf (ent, PRINT_HIGH, "%s", stats);
}

/*
==============
TDM_FindTeamplayerForJoinCode
==============
Get teamplayer info for a join code.
*/
teamplayer_t *TDM_FindTeamplayerForJoinCode (unsigned code)
{
	int	i;

	for (i = 0; i < current_matchinfo.num_teamplayers; i++)
	{
		if (current_matchinfo.teamplayers[i].joincode == code)
			return current_matchinfo.teamplayers + i;
	}

	return NULL;
}

void TDM_SetupTeamInfoForPlayer (edict_t *ent, teamplayer_t *info)
{
	const char *code;

	//clear old link
	if (ent->client->resp.teamplayerinfo)
	{
		//gi.dprintf ("TDM_SetupTeamInfoForPlayer: %p --> %p\n", ent->client->resp.teamplayerinfo, info );
		ent->client->resp.teamplayerinfo->client = NULL;
	}

	strcpy (info->name, ent->client->pers.netname);
	
	info->client = ent;
	info->ping = ent->client->ping;
	info->team = ent->client->pers.team;
	info->matchinfo = &current_matchinfo;

	//user has a preferred joincode they want to always use
	code = Info_ValueForKey (ent->client->pers.userinfo, "joincode");
	if (code[0])
	{
		unsigned	value;

		value = strtoul (code, NULL, 10);
		if (!value || TDM_FindTeamplayerForJoinCode (value))
			gi.cprintf (ent, PRINT_HIGH, "Your preferred join code could not be set.\n");
		else
			info->joincode = value;
	}

	if (!info->joincode)
	{
		unsigned	joincode;

		//no prefered code, they get random
		do
		{
			joincode = genrand_int31 () % 9999;
		} while (TDM_FindTeamplayerForJoinCode (joincode));

		info->joincode = joincode;
	}

	G_StuffCmd (ent, "set joincode \"%u\" u\n", info->joincode);
	gi.cprintf (ent, PRINT_HIGH, "Your join code for this match is %s\n", TDM_SetColorText(va("%u", info->joincode)));

	ent->client->resp.teamplayerinfo = info;
}

/*
==============
TDM_SetupTeamPlayers
==============
Record who is in this match for stats / scores tracking in the event a player
disconnects mid-match or something.
*/
void TDM_SetupMatchInfoAndTeamPlayers (void)
{
	int		i;
	edict_t	*ent;

	current_matchinfo.game_mode = g_gamemode->value;
	current_matchinfo.timelimit = g_match_time->value / 60;

	strcpy (current_matchinfo.mapname, level.mapname);

	strcpy (current_matchinfo.teamnames[TEAM_A], teaminfo[TEAM_A].name);
	strcpy (current_matchinfo.teamnames[TEAM_B], teaminfo[TEAM_B].name);

	current_matchinfo.num_teamplayers = teaminfo[TEAM_A].players + teaminfo[TEAM_B].players;
	current_matchinfo.teamplayers = gi.TagMalloc (current_matchinfo.num_teamplayers * sizeof(teamplayer_t), TAG_GAME);

	current_matchinfo.is1v1 = TDM_Is1V1 ();

	for (i = 0, ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
	{
		//for old servers, we may need to detect and cleanup any stale clients from map changes. we do this here
		//since we don't want to prematurely kill someone who may be taking a long time to connect (eg, map download)
		if (!ent->inuse)
		{
			if (ent->client->pers.connected)
			{
				if (!(game.server_features & GMF_WANT_ALL_DISCONNECTS))
				{
					gi.dprintf ("TDM_SetupMatchInfoAndTeamPlayers: Unclean disconnect of client %d.\n", (int)(ent - g_edicts - 1));
					memset (&ent->client->pers, 0, sizeof(ent->client->pers));
					ent->solid = SOLID_NOT;
					ent->s.modelindex = ent->s.effects = ent->s.sound = 0;
					gi.unlinkentity (ent);
				}
				else
				{
					//the server guarantees they are still connected,
					//just remove them from any team to prevent rejoining mid-game
					ent->client->pers.team = TEAM_SPEC;
				}
			}
			continue;
		}

		if (ent->client->resp.teamplayerinfo)
		{
			TDM_Error("TDM_SetupMatchInfoAndTeamPlayers: should not have teamplayerinfo");
		}
		if (ent->client->pers.team)
		{
			TDM_SetupTeamInfoForPlayer (ent, current_matchinfo.teamplayers + i);
			i++;
		}
	}

	if (teaminfo[TEAM_A].captain)
		current_matchinfo.captains[TEAM_A] = teaminfo[TEAM_A].captain->client->resp.teamplayerinfo;

	if (teaminfo[TEAM_B].captain)
		current_matchinfo.captains[TEAM_B] = teaminfo[TEAM_B].captain->client->resp.teamplayerinfo;

	if (teaminfo[TEAM_A].players > teaminfo[TEAM_B].players)
		current_matchinfo.max_players_per_team = teaminfo[TEAM_A].players;
	else
		current_matchinfo.max_players_per_team = teaminfo[TEAM_B].players;
}

/*
==============
TDM_IsTrackableItem
==============
Returns true for items we want to keep track of for stats.
*/
qboolean TDM_IsTrackableItem (edict_t *ent)
{
	if (!ent->item)
		TDM_Error ("TDM_IsTrackableItem: Got entity %d with no item", ent - g_edicts);

	//we don't track stuff during warmup
	if (tdm_match_status < MM_PLAYING || tdm_match_status == MM_SCOREBOARD)
		return false;

	//its health, but not megahealth
	if (ent->item == GETITEM(ITEM_ITEM_HEALTH) && !(ent->style & HEALTH_TIMED))
		return false;

	//useless counting this
	if (ent->item->flags & IT_AMMO)
		return false;

	//ignore tossed / dropped weapons
	if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
		return false;

	//ignore weapons if weapon stay is enabled
	if ((ent->item->flags & IT_WEAPON) && ((int)dmflags->value & DF_WEAPONS_STAY))
		return false;

	//armor shards aren't worth tracking
	if (ent->item->tag == ARMOR_SHARD)
		return false;

	//by now we should have everything else - armor, weapons, powerups and mh
	return true;
}

/*
==============
TDM_ItemSpawned
==============
An item (re)spawned, track count for stats.
*/
void TDM_ItemSpawned (edict_t *ent)
{
	//as per players requests, only items that are grabbed count towards the total
	return;

	/*
	if (!TDM_IsTrackableItem (ent))
		return;
	
	current_matchinfo.item_spawn_count[ITEM_INDEX(ent->item)]++;*/
}

/*
==============
TDM_ItemGrabbed
==============
Someone grabbed an item, do stuff for stats if needed.
*/
void TDM_ItemGrabbed (edict_t *ent, edict_t *player)
{
	//do we want to track it?
	if (!TDM_IsTrackableItem (ent))
		return;

	//as per players requests, only items that are grabbed count towards the total
	current_matchinfo.item_spawn_count[ITEM_INDEX(ent->item)]++;

	//something bad happened if this is hit!
	if (!player->client->resp.teamplayerinfo)
		TDM_Error ("TDM_ItemGrabbed: No teamplayerinfo for client %d", player - g_edicts - 1);

	//add it
	player->client->resp.teamplayerinfo->items_collected[ITEM_INDEX(ent->item)]++;
}
