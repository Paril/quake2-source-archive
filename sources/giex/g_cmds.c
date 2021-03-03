#include "g_local.h"
#include "m_player.h"

void printVotePrivate(edict_t *target)
{
	char type[64], more[64], desc[64];
	if (!target || !target->inuse || !target->client)
		return;

	if (level.vote_type == VOTE_TM) {
		strcpy(type, "make monsters");
		strcpy(more, "a bit");
		strcpy(desc, "tougher");
	} else if (level.vote_type == VOTE_WM) {
		strcpy(type, "make monsters");
		strcpy(more, "a bit");
		strcpy(desc, "weaker");
	} else if (level.vote_type == VOTE_FFA) {
		strcpy(type, "change game mode");
		strcpy(more, "into");
		strcpy(desc, "Free For All");
	} else if (level.vote_type == VOTE_TDM) {
		strcpy(type, "change game mode");
		strcpy(more, "into");
		strcpy(desc, "Team Deathmatch");
	} else if (level.vote_type == VOTE_PVM) {
		strcpy(type, "change game mode");
		strcpy(more, "into");
		strcpy(desc, "Players vs Monsters");
	} else if (level.vote_type == VOTE_DOM) {
		strcpy(type, "change game mode");
		strcpy(more, "into");
		strcpy(desc, "Domination");
	} else if (level.vote_type == VOTE_END) {
		strcpy(type, "end");
		strcpy(more, "the current");
		strcpy(desc, "map");
	} else if (level.vote_type == VOTE_CRAZE) {
		strcpy(type, "go into");
		strcpy(more, "craze");
		strcpy(desc, "next map");
	} else if (level.vote_type == VOTE_HUNT) {
		strcpy(type, "go into");
		strcpy(more, "monster hunt");
		strcpy(desc, "next map");
	} else if (level.vote_type == VOTE_MONSTER) {
		if (dmmonsters->value) {
			strcpy(type, "remove monsters");
		} else {
			strcpy(type, "spawn monsters");
		}
		more[0] = 0;
		desc[0] = 0;
	} else {
		return;
	}

	gi.cprintf(target, PRINT_HIGH, "A poll is running to ");
	gi.cprintf(target, PRINT_CHAT, "%s", type);
	if (more[0] && desc[0]) {
		gi.cprintf(target, PRINT_HIGH, " %s ", more);
		gi.cprintf(target, PRINT_CHAT, "%s", desc);
	}
	gi.cprintf(target, PRINT_HIGH, ".\nYou have %d seconds to place your vote.\n", (int)(level.vote_time - level.time));
}
void printVote(char *starter, char *type, char *more, char *desc) {
	gi.bprintf(PRINT_CHAT, "%s", starter);
	gi.bprintf(PRINT_HIGH, " started a poll to ");
	gi.bprintf(PRINT_CHAT, "%s", type);
	gi.bprintf(PRINT_HIGH, " %s ", more);
	gi.bprintf(PRINT_CHAT, "%s", desc);
	gi.bprintf(PRINT_HIGH, ".\nYou have %d seconds to place your vote.\n", (int)(level.vote_time - level.time));
}

void parseVote(edict_t *ent) {
	if (level.time < 8) {
		gi.cprintf(ent, PRINT_HIGH, "Please wait a while before starting a vote.\n");
		return;
	}
	if (ent->client->pers.muted) {
		gi.cprintf(ent, PRINT_HIGH, "You cannot start a vote since you are muted.\n");
		return;
	}
	if (Q_strcasecmp(gi.argv(1), "yes") == 0) {
		if (level.vote_type == 0) {
			gi.cprintf(ent, PRINT_HIGH, "No poll is in progress.\n");
			return;
		}
		ent->client->pers.vote = 2;
		gi.cprintf(ent, PRINT_HIGH, "You voted yes.\n");
	} else if (Q_strcasecmp(gi.argv(1), "no") == 0) {
		if (level.vote_type == 0) {
			gi.cprintf(ent, PRINT_HIGH, "No poll is in progress.\n");
			return;
		}
		ent->client->pers.vote = 1;
		gi.cprintf(ent, PRINT_HIGH, "You voted no.\n");
	} else if (Q_strcasecmp(gi.argv(1), "tm") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value < 1) {
			gi.cprintf(ent, PRINT_HIGH, "This is only available in cooperative play\n");
			return;
		}
		if (baseskill->value >= 5) {
			gi.cprintf(ent, PRINT_HIGH, "Enough! :)\n");
			return;
		}
		level.vote_type = VOTE_TM;
		level.vote_time = level.time + 30;
		gi.bprintf(PRINT_HIGH, "%s started a poll to make monsters tougher.\n", ent->client->pers.netname);
		gi.bprintf(PRINT_HIGH, "You have 30 seconds to place your vote.\n");
	} else if (Q_strcasecmp(gi.argv(1), "wm") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value < 1) {
			gi.cprintf(ent, PRINT_HIGH, "This is only available in cooperative play\n");
			return;
		}
		if (baseskill->value == 0) {
			gi.cprintf(ent, PRINT_HIGH, "Monsters are already as weak as they can get!\n");
			return;
		}
		level.vote_type = VOTE_WM;
		level.vote_time = level.time + 30;
		gi.bprintf(PRINT_HIGH, "%s started a poll to make monsters weaker.\n", ent->client->pers.netname);
		gi.bprintf(PRINT_HIGH, "You have 30 seconds to place your vote.\n");
	} else if (Q_strcasecmp(gi.argv(1), "ffa") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value != 0) {
			gi.cprintf(ent, PRINT_HIGH, "This is not available in cooperative play\n");
			return;
		}
		if (teams->value == 0) {
			gi.cprintf(ent, PRINT_HIGH, "Already running in FFA mode!\n");
			return;
		}
		if (game.craze == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until craze is over\n");
			return;
		}
		if (game.monsterhunt == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until monster hunt is over\n");
			return;
		}

		level.vote_type = VOTE_FFA;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "change game mode", "into", "Free For All");
//		gi.bprintf(PRINT_HIGH, "%s started a poll to change the game type to FFA.\n", ent->client->pers.netname);
//		gi.bprintf(PRINT_HIGH, "You have 30 seconds to place your vote.\n");
	} else if (Q_strcasecmp(gi.argv(1), "tdm") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value != 0) {
			gi.cprintf(ent, PRINT_HIGH, "This is not available in cooperative play\n");
			return;
		}
		if (teams->value == 1) {
			gi.cprintf(ent, PRINT_HIGH, "Already running in Team DM mode!\n");
			return;
		}
		if (game.craze == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until craze is over\n");
			return;
		}
		if (game.monsterhunt == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until monster hunt is over\n");
			return;
		}
		level.vote_type = VOTE_TDM;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "change game mode", "into", "Team Deathmatch");
/*		gi.bprintf(PRINT_CHAT, "%s", ent->client->pers.netname);
		gi.bprintf(PRINT_HIGH, " started ");
		gi.bprintf(PRINT_CHAT, "a poll");
		gi.bprintf(PRINT_HIGH, " to change the ");
		gi.bprintf(PRINT_CHAT, "game type");
		gi.bprintf(PRINT_HIGH, " to ");
		gi.bprintf(PRINT_CHAT, "Team Deathmatch.\n");
		gi.bprintf(PRINT_HIGH, "You have 30 seconds to place your vote.\n");*/
	} else if (Q_strcasecmp(gi.argv(1), "pvm") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value != 0) {
			gi.cprintf(ent, PRINT_HIGH, "This is not available in cooperative play\n");
			return;
		}
		if (teams->value == 3) {
			gi.cprintf(ent, PRINT_HIGH, "Already running in PvM mode!\n");
			return;
		}
		if (game.craze == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until craze is over\n");
			return;
		}
		if (game.monsterhunt == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until monster hunt is over\n");
			return;
		}
		level.vote_type = VOTE_PVM;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "change game mode", "into", "Players vs Monsters");
//		gi.bprintf(PRINT_HIGH, "%s started a poll to change the game type to PvM.\n", ent->client->pers.netname);
//		gi.bprintf(PRINT_HIGH, "You have 30 seconds to place your vote.\n");
	} else if (Q_strcasecmp(gi.argv(1), "dom") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value != 0) {
			gi.cprintf(ent, PRINT_HIGH, "This is not available in cooperative play\n");
			return;
		}
		if (teams->value == 2) {
			gi.cprintf(ent, PRINT_HIGH, "Already running in Domination mode!\n");
			return;
		}
		if (game.craze == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until craze is over\n");
			return;
		}
		if (game.monsterhunt == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until monster hunt is over\n");
			return;
		}
		level.vote_type = VOTE_DOM;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "change game mode", "into", "Domination");
	} else if (Q_strcasecmp(gi.argv(1), "endmap") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (!deathmatch->value) {
			gi.cprintf(ent, PRINT_HIGH, "This is only available in deathmatch modes\n");
			return;
		}
		level.vote_type = VOTE_END;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "end", "the current", "map");
	} else if (Q_strcasecmp(gi.argv(1), "craze") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (!deathmatch->value) {
			gi.cprintf(ent, PRINT_HIGH, "This is only available in deathmatch modes\n");
			return;
		}
		if (game.craze > 5) {
			gi.cprintf(ent, PRINT_HIGH, "Too close to last craze. Wait %d more maps\n", game.craze - 5);
			return;
		}
		if (game.monsterhunt > 8) {
			gi.cprintf(ent, PRINT_HIGH, "Too close to monster hunt. Wait %d more maps\n", game.monsterhunt - 8);
			return;
		}
		level.vote_type = VOTE_CRAZE;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "go into", "craze", "next map");
	} else if (Q_strcasecmp(gi.argv(1), "hunt") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (!deathmatch->value) {
			gi.cprintf(ent, PRINT_HIGH, "This is only available in deathmatch modes\n");
			return;
		}
		if (game.monsterhunt > 5) {
			gi.cprintf(ent, PRINT_HIGH, "Too close to last monster hunt. Wait %d more maps\n", game.monsterhunt - 5);
			return;
		}
		if (game.craze > 8) {
			gi.cprintf(ent, PRINT_HIGH, "Too close to craze. Wait %d more maps\n", game.craze - 8);
			return;
		}
		level.vote_type = VOTE_HUNT;
		level.vote_time = level.time + 30;
		printVote(ent->client->pers.netname, "go into", "monster hunt", "next map");
	} else if (Q_strcasecmp(gi.argv(1), "kick") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		strcpy(inp, gi.argv(2));
		target = atoi(inp);
		if ((target < 0) || (target > maxclients->value)) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
		}
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
				return;
			}
		}
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
			return;
		}
		level.vote_type = VOTE_KICK;
		level.vote_time = level.time + 30;
		level.vote_target[0] = target;

		printVote(ent->client->pers.netname, "kick", "", targ_ent->client->pers.netname);
	} else if (Q_strcasecmp(gi.argv(1), "mute") == 0) {
		char inp[32];
		int target = 0;
		edict_t *targ_ent;
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		strcpy(inp, gi.argv(2));
		target = atoi(inp);
		if (target == 0) {
			if (inp[0] != '0') {
				gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
				return;
			}
		}
		if ((target < 0) || (target > maxclients->value)) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
		}
		
		targ_ent = g_edicts + 1 + target;
		if (!targ_ent->inuse) {
			gi.cprintf(ent, PRINT_HIGH, "Bad client number, use 'playerlist' to fetch client number\n");
			return;
		}
		level.vote_type = VOTE_MUTE;
		level.vote_time = level.time + 30;
		level.vote_target[0] = target;

		printVote(ent->client->pers.netname, "mute", "", targ_ent->client->pers.netname);
	} else if (Q_strcasecmp(gi.argv(1), "monsters") == 0) {
		if (level.vote_type != 0) {
			gi.cprintf(ent, PRINT_HIGH, "A poll is already in progress.\n");
			return;
		}
		if (coop->value != 0) {
			gi.cprintf(ent, PRINT_HIGH, "This is not available in cooperative play\n");
			return;
		}
		if (game.craze == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until craze is over\n");
			return;
		}
		if (game.monsterhunt == 10) {
			gi.cprintf(ent, PRINT_HIGH, "Wait until monster hunt is over\n");
			return;
		}

		level.vote_type = VOTE_MONSTER;
		level.vote_time = level.time + 30;
		if (dmmonsters->value) {
			printVote(ent->client->pers.netname, "remove monsters", "", "");
		} else {
			printVote(ent->client->pers.netname, "spawn monsters", "", "");
		}
	} else {
		gi.cprintf(ent, PRINT_HIGH, "vote usage:\n");
		gi.cprintf(ent, PRINT_HIGH, "To place a yes or no vote in an ongoing\n");
		gi.cprintf(ent, PRINT_HIGH, "poll, type \"vote yes/no\".\n");
		gi.cprintf(ent, PRINT_HIGH, "To start a new poll, type\n");
		gi.cprintf(ent, PRINT_HIGH, "\"vote tm/wm/ffa/tdm/pvm/dom/endmap\".\n");
		gi.cprintf(ent, PRINT_HIGH, "tm     = Tougher Monsters (Coop only)\n");
		gi.cprintf(ent, PRINT_HIGH, "wm     = Weaker Monsters (Coop only)\n");
		gi.cprintf(ent, PRINT_HIGH, "ffa    = Free for all\n");
		gi.cprintf(ent, PRINT_HIGH, "tdm    = Team deathmatch\n");
		gi.cprintf(ent, PRINT_HIGH, "pvm    = Players vs. Monsters\n");
		gi.cprintf(ent, PRINT_HIGH, "dom    = Domination\n");
		gi.cprintf(ent, PRINT_HIGH, "mute X = Mute player number X\n");
		gi.cprintf(ent, PRINT_HIGH, "kick X = Kick player number X\n");
		gi.cprintf(ent, PRINT_HIGH, "craze  = Go into craze!\n");
		gi.cprintf(ent, PRINT_HIGH, "hunt  = Go hunting!\n");
		gi.cprintf(ent, PRINT_HIGH, "endmap = End the current map (DM only)\n");
		gi.cprintf(ent, PRINT_HIGH, "monsters = Toggle monsters\n");
	}
}

void castSpell(edict_t *self);
void startSpell(edict_t *ent) {
	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;
	if (ent->health < 1)
		return;
	ent->client->castspell = true;
	castSpell(ent);
}

void stopSpell(edict_t *ent) {
	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;
	if (ent->health < 1)
		return;
	ent->client->castspell = false;
	ent->client->weapon_sound = 0;
}

void Cmd_listSkills(edict_t *ent) {
	int i;
	qboolean chkspells = false;
	qboolean chkgeneral = true;
	powerupinfo_t *info;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	if (gi.argc() == 2) {
		if (strcmp(gi.argv(1), "general") == 0) {
			chkgeneral = true;
			chkspells = false;
		} else if (strcmp(gi.argv(1), "spells") == 0) {
			chkgeneral = false;
			chkspells = true;
		} else if (strcmp(gi.argv(1), "all") == 0) {
			chkgeneral = true;
			chkspells = true;
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Skills are divided into two categories, general and spells.\n");
			gi.cprintf(ent, PRINT_HIGH, "Please type \"skills\" followed by the name of the category you wish to\n");
			gi.cprintf(ent, PRINT_HIGH, "list your accumulated bonuses in, for instance \"skills spells\".\n");
			return;
		}
	}

	for (i = 0; i < GIEX_PUTYPES; i++) {
		info = getPowerupInfo(i);
		if ( ( ((info->isspell > 0) && chkspells) || ((info->isspell == 0) && chkgeneral)) && ent->client->pers.skill[i]) {
			gi.cprintf(ent, PRINT_HIGH, "%s level %d", info->name, ent->client->pers.skill[i]);
//			if ((skills->skill[i] >= info->maxlevel) && (info->maxlevel > 0))
//				gi.cprintf(ent, PRINT_HIGH, " (maxed)");
			gi.cprintf(ent, PRINT_HIGH, "\n");
		}
	}
}

void Cmd_autoPickup(edict_t *ent) {
	if (ent->client->pers.skills.autopickup == 0) {
		ent->client->pers.skills.autopickup = 1;
	} else {
		ent->client->pers.skills.autopickup = 0;
	}
	gi.cprintf (ent, PRINT_HIGH, "Auto powerup pickup is now: %s\n", ent->client->pers.skills.autopickup ? "ON" : "OFF");
}
void Cmd_autoPlugin(edict_t *ent) {
	if (ent->client->pers.skills.autoplugin == 0) {
		ent->client->pers.skills.autoplugin = 1;
	} else {
		ent->client->pers.skills.autoplugin = 0;
	}
	gi.cprintf (ent, PRINT_HIGH, "Auto powerup plugin is now: %s\n", ent->client->pers.skills.autoplugin ? "ON" : "OFF");
}
/*
void Cmd_listPowerups(edict_t *ent)
{
	int i;
	skills_t *skills;
	powerupinfo_t *info;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	skills = &ent->client->pers.skills;

	if (gi.argc() == 2) {
		i = atoi(gi.argv(1));

		//R1: check the fscking index!!!
		if (i < 0 || i > 9) {
			gi.cprintf (ent, PRINT_HIGH, "Index %d out of bounds.\n", i);
			return;
		}

		if (skills->powerup_type[i]) {
			info = getPowerupInfo(skills->powerup_type[i]);
			gi.cprintf(ent, PRINT_HIGH, "Slot %d: Level %d %s", i, skills->powerup_level[i], info->name);
			if ((skills->skill[skills->powerup_type[i]] >= info->maxlevel) && (info->maxlevel > 0))
				gi.cprintf(ent, PRINT_HIGH, " (maxed)");
			gi.cprintf(ent, PRINT_HIGH, "\n");
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Slot %d: Empty\n", i);
		}
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Powerups\n========\n");
	for (i = 0; i < 10; i++) {
		if (skills->powerup_type[i]) {
			info = getPowerupInfo(skills->powerup_type[i]);
			gi.cprintf(ent, PRINT_HIGH, "Slot %d: level %d %s", i, skills->powerup_level[i], info->name);
			if ((skills->skill[skills->powerup_type[i]] >= info->maxlevel) && (info->maxlevel > 0))
				gi.cprintf(ent, PRINT_HIGH, " (maxed)");
			gi.cprintf(ent, PRINT_HIGH, "\n");
		}
	}
}
*/
int getCombineLevel(int lvl1, int lvl2) {
	int count1, count2;
	float add;

	if (lvl1 > lvl2) {
		count1 = lvl1;
		count2 = lvl2;
	} else {
		count1 = lvl2;
		count2 = lvl1;
	}
	add = ((random() - 0.09) * (float)count2 / 1.1);
	if (add > 0)
		add = ceil(add - 0.3);
	else
		add = 1;
	if (count1 + (int) add < 1) {
		return 1;
	}
	return count1 + (int) add;
}


qboolean getPowerupLevelable(edict_t *ent, int putype, int lvl);
void pluginPowerup(edict_t *ent, int pu, int item) {
	int i, maxlevel;
	skills_t *skills;
	powerupinfo_t *puinfo;
	iteminfo_t *iteminfo;

	if (pu < 0 || pu > GIEX_PUCARRYPERCHAR) {
		gi.cprintf (ent, PRINT_HIGH, "Index %d out of bounds.\n", pu);
		return;
	}
	skills = &ent->client->pers.skills;
	puinfo = getPowerupInfo(skills->powerup_type[pu]);
	if (!puinfo->inuse) {
		gi.cprintf(ent, PRINT_HIGH, "No powerup %d\n", item);
	}
	if (item == -1) { //plug it in "body"
		if (!getPowerupLevelable(ent, ent->client->pers.skills.powerup_type[pu], 0)) {
			gi.cprintf(ent, PRINT_HIGH, "You do not meet the requirements for %s.\n", puinfo->name);
			return;
		}
		maxlevel = puinfo->maxlevel;
		if (!puinfo->isspell) {
			gi.cprintf(ent, PRINT_HIGH, "The %s powerup needs to be plugged into an item\n", puinfo->name);
			return;
		}

		for(i = 0; i < GIEX_PUPERCHAR; i++) {
			if (skills->putype[i] == skills->powerup_type[pu]) {
				int newlvl;
				if (skills->pumaxlvl[i] >= maxlevel) {
					gi.cprintf(ent, PRINT_HIGH, "Your %s powerup is already at max level\n", puinfo->name);
					return;
				}
				newlvl = getCombineLevel(skills->pumaxlvl[i], skills->powerup_level[pu]);
				if (newlvl >= maxlevel) {
					newlvl = maxlevel;
					gi.cprintf(ent, PRINT_HIGH, "Your %s powerup maxes out at level %d\n", puinfo->name, newlvl);
				} else {
					gi.cprintf(ent, PRINT_HIGH, "You combine them to level %d %s\n", newlvl, puinfo->name);
				}
				skills->pumaxlvl[i] = newlvl;
				skills->powerup_type[pu] = 0;
				skills->powerup_level[pu] = 0;
				ent->client->pers.puexpRemain[i] = getPowerupCost(ent, i);
				restructurePowerups(ent);
				return;
			}
			if (skills->putype[i] == 0) {
				skills->putype[i] = skills->powerup_type[pu];
				skills->pumaxlvl[i] = skills->powerup_level[pu];
				if (skills->pumaxlvl[i] >= maxlevel) {
					skills->pumaxlvl[i] = maxlevel;
					gi.cprintf(ent, PRINT_HIGH, "Your %s powerup maxes out at level %d\n", puinfo->name, skills->pumaxlvl[i]);
				} else {
					gi.cprintf(ent, PRINT_HIGH, "You plug in the level %d %s\n", skills->pumaxlvl[i], puinfo->name);
				}
				skills->powerup_type[pu] = 0;
				skills->powerup_level[pu] = 0;
				ent->client->pers.puexpRemain[i] = getPowerupCost(ent, i);
				restructurePowerups(ent);
				return;
			}
		}

		gi.cprintf(ent, PRINT_HIGH, "No free slots for the %s powerup\n", puinfo->name);
		return;
	}

	if (item < 0 || item > GIEX_ITEMPERCHAR) {
		gi.cprintf (ent, PRINT_HIGH, "Index %d out of bounds.\n", pu);
		return;
	}
	iteminfo = getItemInfo(skills->itemId[item]);
	if (iteminfo->name[0] == '\0') {
		gi.cprintf(ent, PRINT_HIGH, "No item %d\n", item);
	}
	{
		int puinfoslot = getItemPowerupInfoSlot(iteminfo, skills->powerup_type[pu]);
		if (puinfoslot < 0) {
			gi.cprintf(ent, PRINT_HIGH, "The %s is incompatible with the %s powerup\n", iteminfo->name, puinfo->name);
			return;
		}
		maxlevel = puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot];
		//gi.dprintf("%d + %d (%d) = %d\n", puinfo->maxlevel, iteminfo->pumaxlvl[puinfoslot], puinfoslot, maxlevel);
		for(i = 0; i < GIEX_PUPERITEM; i++) {
			if (i == iteminfo->maxpowerups) {
				break;
			}
			if (skills->item_putype[item][i] == skills->powerup_type[pu]) {
				int newlvl;
				if (skills->item_pumaxlvl[item][i] >= maxlevel) {
					gi.cprintf(ent, PRINT_HIGH, "Your %s's %s powerup is already at max level\n", iteminfo->name, puinfo->name);
					return;
				}
				newlvl = getCombineLevel(skills->item_pumaxlvl[item][i], skills->powerup_level[pu]);
				if (newlvl >= maxlevel) {
					newlvl = maxlevel;
					gi.cprintf(ent, PRINT_HIGH, "Your %s's %s powerup maxes out at level %d\n", iteminfo->name, puinfo->name, newlvl);
				} else {
					gi.cprintf(ent, PRINT_HIGH, "The %s powerup in the %s is combined to level %d\n", puinfo->name, iteminfo->name, newlvl);
				}
				skills->item_pumaxlvl[item][i] = newlvl;
				skills->powerup_type[pu] = 0;
				skills->powerup_level[pu] = 0;
				ent->client->pers.item_puexpRemain[item][i] = getItemPowerupCost(ent, item, i);
				restructurePowerups(ent);
				return;
			}
			if (skills->item_putype[item][i] == 0) {
				skills->item_putype[item][i] = skills->powerup_type[pu];
				skills->item_pumaxlvl[item][i] = skills->powerup_level[pu];
				if (skills->item_pumaxlvl[item][i] >= maxlevel) {
					skills->item_pumaxlvl[item][i] = maxlevel;
					gi.cprintf(ent, PRINT_HIGH, "Your %s's %s powerup maxes out at level %d\n", iteminfo->name, puinfo->name, skills->item_pumaxlvl[item][i]);
				} else {
					gi.cprintf(ent, PRINT_HIGH, "You plug in the level %d %s in the %s\n", skills->item_pumaxlvl[item][i], puinfo->name, iteminfo->name);
				}
				skills->powerup_type[pu] = 0;
				skills->powerup_level[pu] = 0;
				ent->client->pers.item_puexpRemain[item][i] = getItemPowerupCost(ent, item, i);
				restructurePowerups(ent);
				return;
			}
		}
		gi.cprintf(ent, PRINT_HIGH, "No free slots in the %s for the %s powerup\n", iteminfo->name, puinfo->name);
	}
}

void Cmd_plugin(edict_t *ent) {
//	int pu, item;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

//	if (gi.argc() < 2) {
		closeGiexMenu(ent);
		ent->client->showscores = true;
		ent->client->showmenu = 9;
		showGiexPlugins(ent);
//		return;
//	}
/*
	if (gi.argc() > 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: plugin [powerup number] [item number]\n");
	}
	pu = atoi(gi.argv(1));
	if (gi.argc() == 2) {
		pluginPowerup(ent, pu, -1);
		return;
	}

//(gi.argc() == 3) here
	item = atoi(gi.argv(2));
	pluginPowerup(ent, pu, item);*/
}

void Cmd_combinePowerups(edict_t *ent)
{
	int pu1, pu2;
	int count1, count2;
	int count_out;
	powerupinfo_t *info;
	skills_t *skills;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	skills = &ent->client->pers.skills;
	if (gi.argc() == 3) {
		pu1 = atoi(gi.argv(1));
		pu2 = atoi(gi.argv(2));
		if ((pu1 < 0) || (pu1 > 9) || (pu2 < 0) || (pu2 > 9)) {
			gi.cprintf(ent, PRINT_HIGH, "Usage: combine <0-9> <0-9>\n");
			return;
		}
		if (skills->powerup_type[pu1] != skills->powerup_type[pu2]) {
			gi.cprintf(ent, PRINT_HIGH, "You must combine two powerups of the same type\n");
			return;
		}
		if (skills->powerup_type[pu1] == 0) {
			gi.cprintf(ent, PRINT_HIGH, "You must combine two powerups\n");
			return;
		}
		if ((pu1 == skills->active_powerup) || (pu2 == skills->active_powerup)) {
			gi.cprintf(ent, PRINT_HIGH, "Your currently active powerup cannot be used\n");
			return;
		}
		if (pu1 == pu2) {
			gi.cprintf(ent, PRINT_HIGH, "You cannot combine a powerup with itself\n");
			return;
		}
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Usage: combine <powerup 1> <powerup 2>\n");
		return;
	}

	info = getPowerupInfo(skills->powerup_type[pu1]);
	if (skills->powerup_level[pu2] > skills->powerup_level[pu1]) {
		int tmp = pu2;
		pu2 = pu1;
		pu1 = tmp;
	}
	count1 = skills->powerup_level[pu1];
	count2 = skills->powerup_level[pu2];

	count_out = getCombineLevel(count1, count2);

	if (count_out < 1)
		count_out = 1;
	if (count_out > info->maxlevel)
		count_out = info->maxlevel;
	skills->powerup_level[pu1] = count_out;

	skills->powerup_level[pu2] = 0;
	skills->powerup_type[pu2] = 0;

	gi.cprintf(ent, PRINT_HIGH, "The powerups merged into a level %d %s powerup!\n", skills->powerup_level[pu1], info->name);
	return;
}

/*
void Cmd_activatePowerup(edict_t *ent)
{
	int num;
	skills_t *skills;
	powerupinfo_t *info;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	if (gi.argc() == 2) {
		num = atoi(gi.argv(1));
		if ((num < 0) || (num > 9)) {
			gi.cprintf(ent, PRINT_HIGH, "Usage: activate [0-9]\n");
			return;
		}
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Usage: activate X\n");
		return;
	}

	skills = &ent->client->pers.skills;
	if (skills->powerup_type[num] == 0) {
		gi.cprintf(ent, PRINT_HIGH, "Slot %d is empty, deactivating powerup\n", num);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/pupdeact.wav"), 1, ATTN_NORM, 0);
		skills->active_powerup = num;
		return;
	}
	skills->active_powerup = num;
	info = getPowerupInfo(skills->powerup_type[num]);
	gi.cprintf(ent, PRINT_HIGH, "Activating level %d %s\n", skills->powerup_level[num], info->name);
	gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/pupact.wav"), 1, ATTN_NORM, 0);
}
*/

void Cmd_chooseClass(edict_t *ent)
{
		int i;
	classinfo_t *info;

	if (ent->client->pers.spectator)
		return;
	if (gi.argc() >= 2) {
		for (i = 0; i < GIEX_NUMCLASSES; i++) {
			info = getClassInfo(i);

			//r1: portability! Q_strcasecmp, not strcasecmp.
			if ((strlen(info->name) > 0) && (!Q_strcasecmp(gi.args(), info->name))) {
				if (ent->client->pers.skills.classLevel[i] >= info->maxlevel) {
					gi.cprintf(ent, PRINT_HIGH, "Your %s level is already at max, you will not gain any exp towards your class!\n", info->name);
				}
				ent->client->pers.skills.activeClass = i;
				ent->client->pers.expRemain = getClassExpLeft(ent, ent->client->pers.skills.activeClass);
				gi.cprintf(ent, PRINT_HIGH, "%s chosen\n", info->name);
				return;
			}
		}
		return;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Usage: class <classname>\n");
		gi.cprintf(ent, PRINT_HIGH, "Available classes:\n");
		for (i = 0; i < GIEX_NUMCLASSES; i++) {
			info = getClassInfo(i);
			if (strlen(info->name) > 0) {
				gi.cprintf(ent, PRINT_HIGH, "%s\n", info->name);
			}
		}
	}
}

/*
void Cmd_activePowerup(edict_t *ent)
{
	int num;
	skills_t *skills;
	powerupinfo_t *info;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	skills = &ent->client->pers.skills;
	num = skills->active_powerup;
	info = getPowerupInfo(skills->powerup_type[num]);

	gi.cprintf(ent, PRINT_HIGH, "Active powerup: level %d %s (slot %d).\n", skills->powerup_count[num], info->name, num, skills->powerup_exp[num]);
}
*/

/*
void powerup_make_touchable (edict_t *powerup);
void Cmd_dropPowerup(edict_t *ent) {
	int num;
	edict_t *powerup;
	skills_t *skills;
	vec3_t forward, right, offset;
	trace_t trace;

	if (!ent->client)
		return;
	if (ent->client->pers.spectator)
		return;

	if (gi.argc() == 3) {
		num = atoi(gi.argv(2));
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Usage: drop powerup X\n");
		return;
	}

	if ((num < 0) || (num > 9)) {
		gi.cprintf(ent, PRINT_HIGH, "That slot does not exist!\n");
		return;
	}
	skills = &ent->client->pers.skills;
	if (num == skills->active_powerup) {
		gi.cprintf(ent, PRINT_HIGH, "You cannot drop your currently active powerup!\n");
		return;
	}
	if (skills->powerup_type[num] == 0) {
		gi.cprintf(ent, PRINT_HIGH, "No powerup in slot %d\n", num);
		return;
	}

	powerup = G_Spawn();
	powerup->classname = "powerup";
	VectorCopy (ent->s.angles, powerup->s.angles);
	powerup->movetype = MOVETYPE_TOSS;
	gi.setmodel (powerup, "models/items/keys/pyramid/tris.md2");
	powerup->think = powerup_make_touchable;
	powerup->nextthink = level.time + 1;
	powerup->solid = SOLID_TRIGGER;
	powerup->radius_dmg = skills->powerup_type[num];
	powerup->dmg_radius = skills->powerup_level[num];

	VectorSet (powerup->mins, -15, -15, -15);
	VectorSet (powerup->maxs, 15, 15, 15);
	powerup->s.effects |= EF_ROTATE;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 0, -16);
	G_ProjectSource (ent->s.origin, offset, forward, right, powerup->s.origin);
	trace = gi.trace (ent->s.origin, powerup->mins, powerup->maxs,
						powerup->s.origin, ent, CONTENTS_SOLID);
	VectorCopy (trace.endpos, powerup->s.origin);

	VectorScale (forward, 100, powerup->velocity);
	powerup->velocity[2] = 300;
	gi.linkentity (powerup);

	skills->powerup_type[num] = 0;
	skills->powerup_level[num] = 0;
	restructurePowerups(ent);
}
*/
void Cmd_showHelp(edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH, "Giex - Quake 2\n");
	gi.cprintf(ent, PRINT_HIGH, "===================================================\n");
	gi.cprintf(ent, PRINT_HIGH, "For more detailed help and information go to our\n");
	gi.cprintf(ent, PRINT_HIGH, "website at: http://www.planetquake.com/giex/\n\n");
	gi.cprintf(ent, PRINT_HIGH, "Basics:\n\n");
	gi.cprintf(ent, PRINT_HIGH, "This mod is what you could call a Lite RPG.\n");
	gi.cprintf(ent, PRINT_HIGH, "There are numerous different types of playing modes ");
	gi.cprintf(ent, PRINT_HIGH, "in Giex, but all have a common theme:\n");
	gi.cprintf(ent, PRINT_HIGH, "As you kill your opponents, you gain power ");
	gi.cprintf(ent, PRINT_HIGH, "(or exp) and with these exp comes upgrades. These");
	gi.cprintf(ent, PRINT_HIGH, "upgrades will make you more powerful, helping you ");
	gi.cprintf(ent, PRINT_HIGH, "in defeating your foes.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "Sometimes when you kill an enemy, a \"powerup\" is ");
	gi.cprintf(ent, PRINT_HIGH, "left behind. If you are interested in the bonus of that ");
	gi.cprintf(ent, PRINT_HIGH, "particular powerup, run over it and type \"pickup\". ");
	gi.cprintf(ent, PRINT_HIGH, "If you have space for it (you may only carry 10), you ");
	gi.cprintf(ent, PRINT_HIGH, "will pick it up and you can now \"activate\" that powerup. ");
	gi.cprintf(ent, PRINT_HIGH, "When fighting opponents, your active powerup (and ");
	gi.cprintf(ent, PRINT_HIGH, "only that) will charge up with power. As it gets ");
	gi.cprintf(ent, PRINT_HIGH, "sufficient power, it will \"merge\" with you, giving ");
	gi.cprintf(ent, PRINT_HIGH, "you the bonus applied to it. Observe that you will ");
	gi.cprintf(ent, PRINT_HIGH, "not gain any bonuses from any powerup until it is ");
	gi.cprintf(ent, PRINT_HIGH, "merged.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "Type \"giexcmds\" to get a list of needed commands.\n");
}

void Cmd_showCmds(edict_t *ent)
{
	gi.cprintf(ent, PRINT_HIGH, "Giex - Quake 2\n");
	gi.cprintf(ent, PRINT_HIGH, "===================================================\n");
	gi.cprintf(ent, PRINT_HIGH, "Commands:\n\n");
	gi.cprintf(ent, PRINT_HIGH, "PICKUP					Pick up any power up you are\n");
	gi.cprintf(ent, PRINT_HIGH, "                       touching.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "POWERUPS               List your current powerups.\n");
	gi.cprintf(ent, PRINT_HIGH, "                       A * next to a powerup means\n");
	gi.cprintf(ent, PRINT_HIGH, "                       that it is the active one.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "SKILLS                 List your absorbed powerups.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "ACTIVATE #             Activates powerup in slot #.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "DROP POWERUP #         Drops powerup in slot #.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "NEXTSPELL              Activates next spell.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "PREVSPELL              Activates previous spell.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "+CASTSPELL             Casts the active spell.\n\n");
	gi.cprintf(ent, PRINT_HIGH, "LOADCHAR password      Loads your character or\n");
	gi.cprintf(ent, PRINT_HIGH, "                       creates a new one if the\n");
	gi.cprintf(ent, PRINT_HIGH, "                       password/name combination\n");
	gi.cprintf(ent, PRINT_HIGH, "                       does not exist.\n");
	gi.cprintf(ent, PRINT_HIGH, "                       Use this command to enter\n");
	gi.cprintf(ent, PRINT_HIGH, "                       the game!\n");
	gi.cprintf(ent, PRINT_HIGH, "SAVECHAR               Saves your character.\n\n");
}

void Cmd_activeSpell(edict_t *ent)
{
	powerupinfo_t *info;

	if (ent->client->pers.spectator)
		return;
	if (gi.argc() >= 2) {
		int i;

		for (i = 0; i < GIEX_PUTYPES; i++) {
			info = getPowerupInfo(i);
			if (info->isspell && !Q_strcasecmp(gi.args(), info->name)) {
				ent->client->pers.selspell = i;
				ent->client->aura_refreshtime = level.time + 1.0;
				gi.cprintf(ent, PRINT_HIGH, "%s activated\n", info->name);
				return;
			}
		}
		return;
	}

	if (ent->client->pers.skill[ent->client->pers.selspell]) {
		info = getPowerupInfo(ent->client->pers.selspell);
		gi.cprintf(ent, PRINT_HIGH, "%s active\n", info->name);
	}
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2) {
	if (ent1->client->pers.spectator) {
		if (ent2->client->pers.spectator)
			return true;
		else
			return false;
	}
	if (ent2->client->pers.spectator) {
		//Since ent1 is NOT a spectator here, return false
		return false;
	}
	if (coop->value)
		return true;
	if (teams->value == 0)
		return false;

	return (ent1->count == ent2->count);
}


void selectNextMenu(edict_t *ent);
void selectPrevMenu(edict_t *ent);
void selectMenuChoice(edict_t *ent);
void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->showscores && cl->showmenu > 0) {
		selectNextMenu(ent);
		return;
	}
	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
	return;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->showscores && cl->showmenu > 0) {
		selectPrevMenu(ent);
		return;
	}
	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
	return;
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent) {
/*	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);*/
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent) {
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (!sv_cheats->value) {
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_strcasecmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_strcasecmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000, false);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "armor") == 0)
	{
		it = FindItem("Body Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = ent->client->max_armor;

		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classid = it->classid;
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup) {
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO) {
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	} else {
		it_ent = G_Spawn();
		it_ent->classid = it->classid;
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (!sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		Cmd_wieldItem(ent);
//		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		Cmd_wieldItem(ent);
//		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent) {
	int			index;
	gitem_t		*it;
	char		*s;

/*	if (!strcmp(gi.argv(1), "powerup")) {
		Cmd_dropPowerup(ent);
		return;
	}*/

	s = gi.args();
	it = FindItem(s);
	if (!it) {
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	index = ITEM_INDEX(it);
	if (ent->client->pers.inventory[index] < 1) {
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
	if (it->flags & IT_AMMO) {
		it->drop (ent, it);
		return;
	} else {
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	return;
	if (!it->drop) {
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	closeGiexMenu(ent);

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	if (ent->client->showscores && ent->client->showmenu > 0) {
		selectMenuChoice(ent);
		return;
	}
	return;
	ValidateSelectedItem (ent);
	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
qboolean canWearItem(edict_t *ent, int item);
void wearItem(edict_t *ent, int item);
void Cmd_WeapPrev_f (edict_t *ent) {
	gclient_t	*cl;
	int			i, index = ent->client->pers.skills.wornItem[GIEX_ITEM_WEAPON];
	iteminfo_t *info;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	// scan  for the next valid one
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		index--;
		if (index < 0)
			index = GIEX_ITEMPERCHAR - 1;
		if (ent->client->pers.skills.itemId[index] == 0)
			continue;
		info = getItemInfo(ent->client->pers.skills.itemId[index]);
		if ( ((info->type & 255) == GIEX_ITEM_WEAPON) && (canWearItem(ent, index))) {
			wearItem(ent, index);
			return;
		}
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent) {
	gclient_t	*cl;
	int			i, index = ent->client->pers.skills.wornItem[GIEX_ITEM_WEAPON];
	iteminfo_t *info;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	// scan  for the next valid one
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		index++;
		if (index >= GIEX_ITEMPERCHAR)
			index = 0;
		if (ent->client->pers.skills.itemId[index] == 0)
			continue;
		info = getItemInfo(ent->client->pers.skills.itemId[index]);
		if ( ((info->type & 255) == GIEX_ITEM_WEAPON) && (canWearItem(ent, index))) {
			wearItem(ent, index);
			return;
		}
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	iteminfo_t	*info;

	return;
	cl = ent->client;

	if (!cl->pers.weapon || cl->pers.lastweapon < 0)
		return;

	info = getItemInfo(ent->client->pers.skills.itemId[(int) cl->pers.lastweapon]);
	ent->client->pers.skills.wornItem[info->type & 255] = -1;
	ent->client->pers.skills.activatingItem[info->type & 255] = cl->pers.lastweapon;
	ent->client->newweapon = info->item;

/*	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);*/
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;
	int index;

	return;
	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1) {
//		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop) {
//		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (ent->client->pers.inventory[index] < 1) {
//		gi.cprintf (ent, PRINT_HIGH, "No item\n", s);
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent) {
	if (ent->client->pers.spectator)
		return;
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);

	ent->client->pers.add_exp -= pow(ent->radius_dmg, 1.5) * 20;
	if (ent->client->pers.add_exp > -1) {
		return;
	}

//TODO: Spread out exp deduction over powerups as well?

	ent->client->pers.skills.classExp[ent->client->pers.skills.activeClass] += (int) ent->client->pers.add_exp;
	ent->client->pers.expRemain -= (int) ent->client->pers.add_exp;
	ent->client->pers.add_exp -= (int) ent->client->pers.add_exp;
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showhelp = false;
	ent->client->showinventory = false;
	closeGiexMenu(ent);
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0) {
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (ent->client->pers.entering_pwd) {
		strcpy(ent->client->pers.entered_pwd, gi.argv(1));
		return;
	}
	if (ent->client->pers.muted)
		return;
	if (gi.argc () < 2 && !arg0)
		return;

	if ((!teams->value) && (!coop->value))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0) {
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	} else {
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] &&
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	logmsg(text);

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%2d: %s%s\n",
			i,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void assignToTeam(edict_t *ent) {
	int cr = 0, cb = 0, i;
	edict_t *scan;

	if (!deathmatch->value || !teams->value)
		return;

// Special teams-value that puts all players on the same team. Also, always place players on same team at start.
	if ((teams->value == 3) || (level.framenum < 150)) {
		ent->count = 1;
		gi.cprintf(ent, PRINT_HIGH, "Character loaded successfully, joining game..\n");
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	} else {
// Normal teams
		for (i=0 ; i<maxclients->value ; i++)
		{
			scan = g_edicts + 1 + i;
			if (!scan->inuse)
				continue;

			if (scan->count == 1)
				cr += scan->radius_dmg;
			else if (scan->count == 2)
				cb += scan->radius_dmg;
		}
//			cr = -1024;
		if (cr < cb) {
			ent->count = 1;
			gi.cprintf(ent, PRINT_HIGH, "Joining game on Red team\n");
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED;
		} else {
			ent->count = 2;
			gi.cprintf(ent, PRINT_HIGH, "Joining game on Green team\n");
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}
}

/*
int loadCharacter(edict_t *ent, char *password);
void saveCharacter(edict_t *ent);
void Cmd_loadchar(edict_t *ent)
{
	int i;
	edict_t	*scan;

	if (gi.argc() < 1) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: loadchar <password>\n");
		return;
	}

	if (ent->client->pers.loggedin) {
		gi.cprintf(ent, PRINT_HIGH, "You are already logged in!\n");
		return;
	}

	for (i=0; i<maxclients->value ; i++) {
		scan = g_edicts + 1 + i;
		if (!scan->inuse)
			continue;
		if (!scan->client)
			continue;
		if (strncmp(ent->client->pers.netname, scan->client->pers.skills.password + 16, 16) == 0) {
			gi.cprintf(ent, PRINT_HIGH, "This character is already logged in. Keep your password safe and don't try to cheat.\n");
			return;
		}
	}

	if (!loadCharacter(ent, gi.argv(1))) {
		ent->client->pers.spectator = true;
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 1\n");
		gi.unicast(ent, true);
		//r1: inserted missing return here
		return;
	}
// Go out of spectator mode
	ent->client->pers.spectator = false;
	gi.WriteByte (svc_stufftext);
	gi.WriteString ("spectator 0\n");
	gi.unicast(ent, true);

// Sum up upgrades
	ent->radius_dmg = 0;
	ent->count = 0;
	for (i = 0; i < GIEX_NUMCLASSES; i++)
		ent->radius_dmg += ent->client->pers.skills.classLevel[i];
	ent->client->pers.expRemain = getClassExpLeft(ent, ent->client->pers.skills.activeClass);
	ent->client->itemchanging = -1; // set client ready to equip items

	assignToTeam(ent);
	gi.cprintf(ent, PRINT_HIGH, "Character loaded successfully, joining game..\n");
}
*/
void Cmd_ChangePass(edict_t *ent) {
	if (gi.argc() < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Usage: chpwd <old_password> <new_password> <new_password>\n");
		return;
	}

	if (!ent->client->pers.loggedin) {
		gi.cprintf(ent, PRINT_HIGH, "You need to be logged in!\n");
		return;
	}

	if (strncmp(gi.argv(1), ent->client->pers.skills.password, 16)) {
		gi.cprintf(ent, PRINT_HIGH, "Old password does not match!\n");
		return;
	}
	if (strcmp(gi.argv(2), gi.argv(3))) {
		gi.cprintf(ent, PRINT_HIGH, "Please type new password twice\n");
		return;
	}
	strncpy(ent->client->pers.skills.password, gi.argv(2), 16);
	saveCharacter(ent);
	gi.cprintf(ent, PRINT_HIGH, "Character saved with the new password \"%s\"\n", gi.argv(2));
}
void Cmd_Level(edict_t *ent) {
	int i;
	gi.cprintf(ent, PRINT_HIGH, "Character level: %d\n----------------\n", ent->radius_dmg);
	for (i = 0; i < GIEX_NUMCLASSES; i++) {
		classinfo_t *info = getClassInfo(i);
		if ((strlen(info->name) > 0) && (ent->client->pers.skills.classLevel[i] > 0))
		gi.cprintf(ent, PRINT_HIGH, "%s: %d\n", info->name, ent->client->pers.skills.classLevel[i]);
	}
}
/*
=================
ClientCommand
=================
*/
void showGiexMenu (edict_t *ent);
void showGiexStats(edict_t *ent);
void AdminCommand (edict_t *ent);
void nextSpell(edict_t *self);
void prevSpell(edict_t *self);
void CreateCustomEntity(int type, int c, int sf, vec3_t pos, vec3_t dir);
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	ent->client->pers.idletime = 0;
	cmd = gi.argv(0);

	if (Q_strcasecmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_strcasecmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_strcasecmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "adm") == 0)
	{
		AdminCommand (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_strcasecmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_strcasecmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_strcasecmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_strcasecmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_strcasecmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_strcasecmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_strcasecmp (cmd, "inven") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 1;
			showGiexMenu(ent);
		}
	} else if (Q_strcasecmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_strcasecmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_strcasecmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_strcasecmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
//	else if (Q_strcasecmp (cmd, "weaplast") == 0)
//		Cmd_WeapLast_f (ent);
	else if (Q_strcasecmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_strcasecmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_strcasecmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_strcasecmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);

	else if (Q_strcasecmp(cmd, "startspell") == 0)
		startSpell(ent);
	else if (Q_strcasecmp(cmd, "stopspell") == 0)
		stopSpell(ent);
	else if (Q_strcasecmp(cmd, "prevspell") == 0)
		prevSpell(ent);
	else if (Q_strcasecmp(cmd, "nextspell") == 0)
		nextSpell(ent);

	else if (Q_strcasecmp(cmd, "chpwd") == 0)
		Cmd_ChangePass(ent);
/*	else if (Q_strcasecmp(cmd, "loadchar") == 0)
		Cmd_loadchar(ent);
	else if (Q_strcasecmp(cmd, "savechar") == 0) {
		saveCharacter(ent);
		gi.cprintf(ent, PRINT_HIGH, "Character saved\n");*/
	else if (Q_strcasecmp(cmd, "powerups") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 3;
			showGiexMenu(ent);
		}
	}/* else if (Q_strcasecmp(cmd, "plugin") == 0)
		Cmd_plugin(ent);*/
	else if ((Q_strcasecmp(cmd, "plugins") == 0) || (Q_strcasecmp(cmd, "memory") == 0)) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 9;
			showGiexPlugins(ent);
		}
	}
/*	else if (Q_strcasecmp(cmd, "activate") == 0)
		Cmd_activatePowerup(ent);
	else if (Q_strcasecmp(cmd, "active") == 0)
		Cmd_activePowerup(ent);
	else if (Q_strcasecmp(cmd, "combine") == 0)
		Cmd_combinePowerups(ent);*/
	else if (Q_strcasecmp(cmd, "skills") == 0)
		Cmd_listSkills(ent);

	else if (Q_strcasecmp(cmd, "giexhelp") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 6;
			showGiexHelpMenu(ent);
		}
		//Cmd_showHelp(ent);
	} else if (Q_strcasecmp(cmd, "giexcmds") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 11;
			showGiexCommandsMenu(ent);
		}
//		Cmd_showCmds(ent);
	} else if (Q_strcasecmp(cmd, "spell") == 0)
		Cmd_activeSpell(ent);
	else if (Q_strcasecmp(cmd, "class") == 0)
		Cmd_chooseClass(ent);
	else if (Q_strcasecmp(cmd, "level") == 0)
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->menuchoice[0] = 1;
			ent->client->showmenu = 13;
			showGiexStats(ent);
		}
//		Cmd_Level(ent);
	else if (Q_strcasecmp(cmd, "inv") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 1;
			showGiexMenu(ent);
		}
	}
	else if (Q_strcasecmp (cmd, "stats") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 13;
			showGiexMenu(ent);
		}
	} else if (Q_strcasecmp(cmd, "wield") == 0)
		Cmd_wieldItem(ent);
	else if (Q_strcasecmp(cmd, "wear") == 0)
		Cmd_wieldItem(ent);

	//r1ch: this is horribly unreliable since clientcommand is run from usercommands and
	//not per game frame. to guarantee this to work you need at LEAST 0.2. ideally it should
	//do a findradius or something.
	else if (Q_strcasecmp(cmd, "pickup") == 0)
		ent->client->get_powerup = level.time + 0.2;
	else if (Q_strcasecmp(cmd, "identify") == 0)
		ent->client->id_powerup = level.time + 0.2;
	else if (Q_stricmp (cmd, "autopickup") == 0)
		Cmd_autoPickup(ent);
	else if (Q_stricmp (cmd, "autoplugin") == 0)
		Cmd_autoPlugin(ent);
	else if (Q_strcasecmp(cmd, "vote") == 0)
		parseVote(ent);
	else if (Q_strcasecmp(cmd, "cdps_rc") == 0)
		CreateCustomEntity(0, 1, 1, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cdps_rn") == 0)
		CreateCustomEntity(0, 1, 0, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cdps_gc") == 0)
		CreateCustomEntity(0, 2, 1, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cdps_gn") == 0)
		CreateCustomEntity(0, 2, 0, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cdps_n") == 0)
		CreateCustomEntity(0, 0, 0, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cdps") == 0)
		CreateCustomEntity(0, 0, 1, ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cmpts") == 0)
		CreateCustomEntity(1, atoi(gi.argv(1)), atoi(gi.argv(2)), ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cmptm") == 0)
		CreateCustomEntity(2, atoi(gi.argv(1)), atoi(gi.argv(2)), ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cmptl") == 0)
		CreateCustomEntity(3, atoi(gi.argv(1)), atoi(gi.argv(2)), ent->s.origin, ent->s.angles);
	else if (Q_strcasecmp(cmd, "cmpth") == 0)
		CreateCustomEntity(4, atoi(gi.argv(1)), atoi(gi.argv(2)), ent->s.origin, ent->s.angles);

	else if (Q_strcasecmp(cmd, "maplist") == 0)
		printMapList(ent);
	else if (Q_strcasecmp(cmd, "nextmap") == 0)
		gi.cprintf(ent, PRINT_HIGH, "nextmap: %s\n", getNextMap(level.mapname));

	else if (Q_strcasecmp(cmd, "exp") == 0)
		gi.cprintf(ent, PRINT_HIGH, "Remaining: %d\n", ent->client->pers.expRemain);
	else if (Q_strcasecmp(cmd, "worn") == 0) {
		if (ent->client->showmenu > 0) {
			closeGiexMenu(ent);
		} else {
			closeGiexMenu(ent);
			ent->client->showscores = true;
			ent->client->showmenu = 12;
			showGiexMenu(ent);
		}
//		Cmd_wornItems(ent);
	}

	else if (Q_strcasecmp(cmd, "cheat_setskill") == 0) {
		if (!sv_cheats->value)
			gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		else
			ent->client->pers.skill[atoi(gi.argv(1))] = atoi(gi.argv(2));
	}

	else	// anything that doesn't match a command will be a chat
		gi.cprintf (ent, PRINT_HIGH, "Unknown client command: %s\n", cmd);
//		Cmd_Say_f (ent, false, true);
}
