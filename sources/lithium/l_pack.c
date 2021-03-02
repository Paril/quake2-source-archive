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

lvar_t *use_packs;

lvar_t *pack_spawn;
lvar_t *pack_life;

lvar_t *pack_health;
lvar_t *pack_armor;
lvar_t *pack_bullets;
lvar_t *pack_shells;
lvar_t *pack_rockets;
lvar_t *pack_grenades;
lvar_t *pack_cells;
lvar_t *pack_slugs;

void Pack_InitGame(void) {
	use_packs = lvar("use_packs", "1", "^", VAR_USE);

	pack_spawn = lvar("pack_spawn", "0.03", "#.##", VAR_PACK);
	pack_life = lvar("pack_life", "25", "##", VAR_PACK);

	pack_health = lvar("pack_health", "120", "###", VAR_PACK);
	pack_armor = lvar("pack_armor", "250", "###", VAR_PACK);
	pack_bullets = lvar("pack_bullets", "360", "###", VAR_PACK);
	pack_shells = lvar("pack_shells", "180", "###", VAR_PACK);
	pack_rockets = lvar("pack_rockets", "90", "###", VAR_PACK);
	pack_grenades = lvar("pack_grenades", "90", "###", VAR_PACK);
	pack_cells = lvar("pack_cells", "360", "###", VAR_PACK);
	pack_slugs = lvar("pack_slugs", "90", "###", VAR_PACK);
}

//===============================
// packs

void Pack_MaybeSpawn(vec3_t origin) {
	edict_t	*pack;

	if(!use_packs->value)
		return;

	if(random() > pack_spawn->value)
		return;

	pack = G_Spawn();
	if(!pack)
		return;

	VectorCopy(origin, pack->s.origin);
	gi.setmodel(pack, "models/items/pack/tris.md2");
	pack->velocity[0] = -200 + (random() * 400);
	pack->velocity[1] = -200 + (random() * 400);
	pack->velocity[2] = 150 + (random() * 150);
	pack->movetype = MOVETYPE_BOUNCE;
	pack->solid = SOLID_TRIGGER;
	pack->clipmask = MASK_SOLID;
	pack->s.frame = 0;
	pack->flags = 0;
	pack->classname = "pack";
	pack->s.effects = EF_ROTATE;
	pack->s.renderfx = RF_GLOW;

	pack->spawnflags = DROPPED_ITEM;

	VectorSet(pack->mins, -16, -16, -12);
	VectorSet(pack->maxs, 16, 16, 24);

	pack->item = FindItem("pack");

	pack->touch = Pack_Touch;
	pack->think = Pack_Remove;
	pack->nextthink = level.time + pack_life->value;

	gi.linkentity(pack);
}

qboolean Pickup_Pack(edict_t *ent, edict_t *other);
void Pack_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	gclient_t *client;

	if(strcmp(other->classname, "player"))
		return; // only players can have packs

	if(other->health < 1)
		return;

	if(level.intermissiontime)
		return;

	if(!other->client->has_pack) {
		client = other->client;

		if(other->max_health < pack_health->value)
			other->max_health = pack_health->value;
		if(other->max_armor < pack_armor->value)
			other->max_armor = pack_armor->value;
		if(client->pers.max_health < pack_health->value)
			client->pers.max_health = pack_health->value;
		if(client->pers.max_bullets < pack_bullets->value)
			client->pers.max_bullets = pack_bullets->value;
		if(client->pers.max_shells < pack_shells->value)
			client->pers.max_shells = pack_shells->value;
		if(client->pers.max_rockets < pack_rockets->value)
			client->pers.max_rockets = pack_rockets->value;
		if(client->pers.max_grenades < pack_grenades->value)
			client->pers.max_grenades = pack_grenades->value;
		if(client->pers.max_cells < pack_cells->value)
			client->pers.max_cells = pack_cells->value;
		if(client->pers.max_slugs < pack_slugs->value)
			client->pers.max_slugs = pack_slugs->value;

		client->has_pack = true;
	}

	Pickup_Pack(self, other);

	gi.sound(other, CHAN_ITEM, gi.soundindex("items/pkup.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict(self);
}

void Pack_Remove(edict_t *self) {
	G_FreeEdict(self);
}

