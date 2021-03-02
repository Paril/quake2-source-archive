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

lvar_t *use_runes;
lvar_t *rune_minbound;
lvar_t *rune_life;
lvar_t *rune_perplayer;
lvar_t *rune_spawnpersecond;
lvar_t *rune_min;
lvar_t *rune_max;

lvar_t *rune_resist;
lvar_t *rune_strength;
lvar_t *rune_regen;
lvar_t *rune_regenmax;
lvar_t *rune_regencap;
lvar_t *rune_vampire;
lvar_t *rune_vampiremax;

lvar_t *rune_flags;

lvar_t *rune_strength_sound;
lvar_t *rune_resist_sound;
lvar_t *rune_haste_sound;
lvar_t *rune_regen_sound;
lvar_t *rune_vampire_sound;

#define RUNE_RESIST 1
#define	RUNE_STRENGTH 2
#define RUNE_HASTE 4
#define RUNE_REGEN 8
#define RUNE_VAMPIRE 16

int rune_count[NUM_RUNES] = { 0, 0, 0, 0, 0 };
int rune_total = 0;

void UseRunesChanged(void) {
	if(!use_runes->value)
		Rune_RemoveAll();
//	StatusBar_UpdateAll();
}

void Rune_InitGame(void) {
	use_runes = lvar("use_runes", "1", "^", VAR_USE);
	use_runes->func = UseRunesChanged;

	rune_minbound = lvar("rune_minbound", "-4", "-##", VAR_NONE);

	rune_flags = lvar("rune_flags", "31", "##", VAR_RUNE);
	rune_perplayer = lvar("rune_perplayer", "0.6", "#.##", VAR_RUNE);
	rune_spawnpersecond = lvar("rune_spawnpersecond", "1", "#", VAR_RUNE);
	rune_life = lvar("rune_life", "20", "##", VAR_RUNE);
	rune_min = lvar("rune_min", "3", "##", VAR_RUNE);
	rune_max = lvar("rune_max", "12", "##", VAR_RUNE);
	rune_resist = lvar("rune_resist", "2.0", "#.##", VAR_RUNE);
	rune_strength = lvar("rune_strength", "2.0", "#.##", VAR_RUNE);
	rune_regen = lvar("rune_regen", "0.25", "#.##", VAR_RUNE);
	rune_regenmax = lvar("rune_regenmax", "200", "###", VAR_RUNE);
	rune_regencap = lvar("rune_regencap", "3.5", "#.###", VAR_RUNE);
	rune_vampire = lvar("rune_vampire", "0.5", "#.##", VAR_RUNE);
	rune_vampiremax = lvar("rune_vampiremax", "200", "###", VAR_RUNE);

	rune_resist_sound = lvar("rune_resist_sound", "world/force2.wav", "wav", VAR_NONE);
	rune_strength_sound = lvar("rune_strength_sound", "items/damage3.wav", "wav", VAR_NONE);
	rune_haste_sound = lvar("rune_haste_sound", "world/x_light.wav", "wav", VAR_NONE);
	rune_regen_sound = lvar("rune_regen_sound", "items/s_health.wav", "wav", VAR_NONE);
	rune_vampire_sound = lvar("rune_vampire_sound", "makron/pain2.wav", "wav", VAR_NONE);
}

void Rune_InitLevel(void) {
	Rune_Reset();

	gi.soundindex(rune_resist_sound->string);
	gi.soundindex(rune_strength_sound->string);
	gi.soundindex(rune_haste_sound->string);
	gi.soundindex(rune_regen_sound->string);
	gi.soundindex(rune_vampire_sound->string);
}

void Rune_Reset(void) {
	int i;
	for(i = 0; i < NUM_RUNES; i++)
		rune_count[i] = 0;
	rune_total = 0;
}

void Rune_RunFrame(void) {
	edict_t *ent;
	int i, j, lowest, x;
	static int counter = -10;

	// only check once per second
	if(counter++ < 10)
		return;
	counter = 0;

	// does a rune need to be spawned?
	if(!use_runes->value)
		return;
	if(!rune_flags->value)
		return;
	if(rune_flags->value >= 1 << NUM_RUNES)
		return;

	for(x = 0; x < rune_spawnpersecond->value; x++) {
		if(rune_total >= rune_max->value)
			return;
		if(rune_total >= rune_min->value && rune_total >= (int)((float)countplayers() * rune_perplayer->value))
			return;

		// find an entity to sprout from
		while(1) {
			ent = &g_edicts[(int)(random() * globals.num_edicts)];
			if(!ent->inuse)
				continue;
			if(ent->client)
				continue;
			break;
		}

		// find which rune there are fewest of
		lowest = -1;
		for(i = 0; i < NUM_RUNES; i++)
			if((int)rune_flags->value & 1 << i && (lowest == -1 || rune_count[i] < lowest))
				lowest = rune_count[i];

		i = j = rand() % NUM_RUNES;
		do {
			if((lowest == -1 || rune_count[i] == lowest) && (int)rune_flags->value & 1 << i) {
				Rune_Spawn(ent->s.origin, 1 << i);
				rune_count[i]++;
				rune_total++;
				break;
			}

			i++;
			if(i == NUM_RUNES)
				i = 0;
		}
		while(i != j);
	}
}

edict_t *Rune_Spawn(vec3_t origin, int type) {
	edict_t	*rune;

	vec3_t	mins = { -20, -20, rune_minbound->value };
	vec3_t	maxs = { 20, 20, rune_minbound->value + 40 };

	if(!use_runes->value)
		return NULL;

	rune = G_Spawn();
	if(!rune)
		return NULL;

	VectorCopy(origin, rune->s.origin);
	gi.setmodel(rune, "models/items/keys/pyramid/tris.md2");
	rune->velocity[0] = -200 + (random() * 400);
	rune->velocity[1] = -200 + (random() * 400);
	rune->velocity[2] = 150 + (random() * 150);
	rune->movetype = MOVETYPE_BOUNCE;
	rune->solid = SOLID_TRIGGER;
	rune->clipmask = MASK_SOLID;
	rune->s.frame = 0;
	rune->flags = 0;
	rune->classname = "rune";
	rune->s.effects = EF_ROTATE | EF_COLOR_SHELL;

	VectorCopy(mins, rune->mins);
	VectorCopy(maxs, rune->maxs);

	if(type & RUNE_RESIST)
		rune->s.renderfx = RF_SHELL_BLUE;
	else if(type & RUNE_STRENGTH)
		rune->s.renderfx = RF_SHELL_RED;
	else if(type & RUNE_HASTE) {
		if(qver >= 3.19f)
			rune->s.renderfx = RF_SHELL_DOUBLE;
		else
			rune->s.renderfx = RF_SHELL_RED | RF_SHELL_GREEN;
	}
	else if(type & RUNE_REGEN)
		rune->s.renderfx = RF_SHELL_GREEN;
	else if(type & RUNE_VAMPIRE)
		rune->s.renderfx = RF_SHELL_RED | RF_SHELL_BLUE;

	rune->touch = Rune_Touch;
	rune->think = Rune_Remove;
	rune->nextthink = level.time + rune_life->value;

	rune->rune = type;

	gi.linkentity(rune);

	return rune;
}

void Rune_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	int i;

	// this can be NULL somehow, if +set deathmatch 1 and no map specified
	if(!other->classname)
		return;

	// only players can have runes
	if(strcmp(other->classname, "player"))
		return;

	if(other->health < 1)
		return;

	if(level.intermissiontime)
		return;

	// don't pick up a rune the player just dropped
	if(self == other->client->drop_rune && other->client->drop_rune_time > level.time)
		return;

	// prevent player from touching a rune TOO much, if they already have one
	if(other->rune) {
        if(self == other->client->last_rune && other->client->last_rune_time > level.time)
			return;

        other->client->last_rune = self;
        other->client->last_rune_time = level.time + 3.0;
        return;
	}

	other->rune = self->rune;
	other->client->regen_remainder = 0;

	other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex("k_pyramid");
	for(i = 0; i < NUM_RUNES; i++)
		if(self->rune & 1 << i)
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_RUNE1 + i;
	other->client->pickup_msg_time = level.time + 3.0;

	gi.sound(other, CHAN_AUTO, gi.soundindex("items/pkup.wav"), 1, ATTN_NORM, 0);

	LNet_Rune(other, other->rune);

	G_FreeEdict(self);
}

void Rune_Remove(edict_t *self) {
	int i;
	for(i = 0; i < NUM_RUNES; i++)
		if(self->rune & 1 << i) {
			rune_count[i]--;
			rune_total--;
		}
	G_FreeEdict(self);
}

void Rune_Drop(edict_t *player) {
	edict_t *rune;

	if(!player->rune)
		return;

	rune = Rune_Spawn(player->s.origin, player->rune);

	player->client->drop_rune = rune;
	player->client->drop_rune_time = level.time + 2;

	player->rune = 0;

	LNet_Rune(player, player->rune);
}

int Rune_AdjustDamage(edict_t *targ, edict_t *attacker, int damage) {
	if(targ->rune & RUNE_RESIST)
		damage /= rune_resist->value;
	if(attacker->rune & RUNE_STRENGTH)
		damage *= rune_strength->value;
	return damage;
}

void body_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void Rune_TDamage(edict_t *targ, edict_t *attacker, int damage) {
	if(targ == attacker || !(targ->client || targ->die == body_die))
		return;

	if(level.time > targ->last_sound_time) {
		if(targ->rune & RUNE_RESIST && !targ->deadflag)
			gi.sound(targ, CHAN_ITEM, gi.soundindex(rune_resist_sound->string), 1, ATTN_NORM, 0);
		if(attacker->rune & RUNE_VAMPIRE)
			gi.sound(targ, CHAN_ITEM, gi.soundindex(rune_vampire_sound->string), 1, ATTN_NORM, 0);
		targ->last_sound_time = level.time;
	}

	if(attacker->rune & RUNE_VAMPIRE) {
		int add, vdamage;

		if(attacker->health > rune_vampiremax->value)
			return;

		vdamage = damage;
		if(targ->health < -40)
			vdamage -= -40 - targ->health;

		add = (int)((float)vdamage * rune_vampire->value + 0.5);

		if(add > 0) {
			attacker->health += add;
			if(attacker->health > rune_vampiremax->value)
				attacker->health = rune_vampiremax->value;
		}
	}
}

void Rune_ClientFrame(edict_t *player) {
	if(level.intermissiontime)
		return;

	// take away a rune that has been turned off
	if(player->rune && !(player->rune & (int)rune_flags->value)) {
		player->rune = 0;
		gi.cprintf(player, PRINT_HIGH, "Admin has disabled the rune you have.\n");
	}

	if(player->rune & RUNE_REGEN && player->health < rune_regenmax->value) {
		float health = player->health + player->client->regen_remainder;
		float newhealth = rune_regenmax->value / health;
		if(newhealth > rune_regencap->value)
			newhealth = rune_regencap->value;
		newhealth *= rune_regen->value * 0.625;
		health += newhealth;
		player->health = (int)health;
		player->client->regen_remainder = health - (float)player->health;

		if(level.framenum % 25 == 0)
			gi.sound(player, CHAN_AUTO, gi.soundindex(rune_regen_sound->string), 1, ATTN_NORM, 0);
	}
}

int Rune_HasHaste(edict_t *player) {
	return player->rune & RUNE_HASTE;
}

int Rune_HasRegen(edict_t *player) {
	return player->rune & RUNE_REGEN;
}

void Rune_RemoveAll(void) {
	edict_t *ent;

	while((ent = G_Find(NULL, FOFS(classname), "rune")))
		G_FreeEdict(ent);

	ent = NULL;
	while((ent = G_Find(ent, FOFS(classname), "player")))
		ent->rune = 0;
}

void Rune_FireSound(edict_t *ent) {
	if(ent->rune == RUNE_STRENGTH)
		gi.sound(ent, CHAN_ITEM, gi.soundindex(rune_strength_sound->string), 1, ATTN_NORM, 0);
	if(ent->rune == RUNE_HASTE)
		gi.sound(ent, CHAN_ITEM, gi.soundindex(rune_haste_sound->string), 1, ATTN_NORM, 0);
}

qboolean Rune_Give(edict_t *ent, char *name) {
	if(!Q_stricmp(name, "resist"))
		ent->rune = RUNE_RESIST;
	else if(!Q_stricmp(name, "strength"))
		ent->rune = RUNE_STRENGTH;
	else if(!Q_stricmp(name, "haste"))
		ent->rune = RUNE_HASTE;
	else if(!Q_stricmp(name, "regen"))
		ent->rune = RUNE_REGEN;
	else if(!Q_stricmp(name, "vampire"))
		ent->rune = RUNE_VAMPIRE;
	else
		return false;

	return true;
}

void CTFSay_Team_Rune(edict_t *who, char *buf, unsigned int buflen) {
	if(who->rune & RUNE_RESIST)
		strlcpy(buf, "the Resist rune", buflen);
	else if(who->rune & RUNE_STRENGTH)
		strlcpy(buf, "the Strength rune", buflen);
	else if(who->rune & RUNE_HASTE)
		strlcpy(buf, "the Haste rune", buflen);
	else if(who->rune & RUNE_REGEN)
		strlcpy(buf, "the Regen rune", buflen);
	else if(who->rune & RUNE_VAMPIRE)
		strlcpy(buf, "the Vampire rune", buflen);
	else
		strlcpy(buf, "no rune", buflen);
}
