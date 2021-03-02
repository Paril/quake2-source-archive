/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"

//===============================
// obituary

// colored text
char *colored(char *str) {
	static char ret[64];
	char *c = str;
	char *d = ret;
	while(*c)
		*d++ = *c++ + 128;
	*d = '\0';
	return ret;
}

void bobit(edict_t *ent1, edict_t *ent2, char *text) {
	int i;
	edict_t *ent;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		if(ent == ent1 || ent == ent2 ||
			ent->client->chase_target == ent1 || ent->client->chase_target == ent2)
			continue;

		gi.cprintf(ent, PRINT_MEDIUM, text);
	}

	if(dedicated->value)
		gi.dprintf(text);
}

void cobit(edict_t *self, char *text) {
	int i;
	edict_t *ent;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		if(ent == self || ent->client->chase_target == self)
			gi.cprintf(ent, PRINT_MEDIUM, text);
	}
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	char		*message3;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";
		message3 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			message3 = "Suicide";
			break;
		case MOD_FALLING:
			message = "cratered";
			message3 = "Fell";
			break;
		case MOD_CRUSH:
			message = "was squished";
			message3 = "Squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			message3 = "Drowned";
			break;
		case MOD_SLIME:
			message = "melted";
			message3 = "Slime";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			message3 = "Lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			message3 = "Exploded";
			break;
		case MOD_EXIT:
			message = "found a way out";
			message3 = "Exit";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			message3 = "Laser";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			message3 = "Blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			message3 = "Bomb";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				message3 = "Hand Grenade";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				message3 = "Grenade Launcher";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				message3 = "Rocket Launcher";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				message3 = "BFG10k";
				break;
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				message3 = "Unknown";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;

			//WF
			GSLog_Score(self->client->pers.netname, "", "Suicide", message3, -1, (int)level.time, self->client->ping);
			LNet_Score(self, self, mod, -1, (int)level.time);

			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				message3 = "Blaster";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				message3 = "Shotgun";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				message3 = "Super Shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				message3 = "Machinegun";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				message3 = "Chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				message3 = "Grenade Launcher";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				message3 = "Grenade Launcher";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				message3 = "Rocket Launcher";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				message3 = "Rocket Launcher";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				message3 = "Hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				message3 = "Railgun";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				message3 = "BFG10k";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				message3 = "BFG10k";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				message3 = "BFG10k";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				message3 = "Hand Grenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				message3 = "Hand Grenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				message3 = "Hand Grenade";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				message3 = "Telefrag";
				break;
//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID
			case MOD_HOOK:
				message = "was disemboweled by";
				message2 = "'s grappling hook";
				message3 = "Grappling Hook";
			}
			if (message)
			{
				// inform everyone except self and attacker
				bobit(self, attacker, va("%s %s %s%s.\n",
					self->client->pers.netname, message, attacker->client->pers.netname, message2));

				// inform self, attacker name in green
				cobit(self, va("%s %s %s%s.\n",
					self->client->pers.netname, message, colored(attacker->client->pers.netname), message2));

				// inform attacker, self name in green
				cobit(attacker, va("%s %s %s%s.\n", 
					colored(self->client->pers.netname), message, attacker->client->pers.netname, message2));

//				if (ff)
//					attacker->client->resp.score--;
//				else
					attacker->client->resp.score++;

				GSLog_Score(attacker->client->pers.netname, self->client->pers.netname, "Kill", 
					message3, 1, (int)level.time, attacker->client->ping);
				LNet_Score(attacker, self, mod, 1, (int)level.time);

				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);

	self->client->resp.score--;
}
