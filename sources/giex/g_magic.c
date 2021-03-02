#include "g_local.h"

int getFreeAuraSlot(edict_t *ent) {
	int i;
	if (!ent->client)
		return -1;
	for (i = 0; i < 5; i++)
		if (ent->client->aura_type[i] == 0)
			return i;

	return -1;
}

int getAuraSlot(edict_t *ent, int aura_skillnum) {
	int i;
	if (!ent->client)
		return -1;
	for (i = 0; i < 5; i++)
		if (ent->client->aura_type[i] == aura_skillnum)
			return i;

	return -1;
}
int getAuraLevel(edict_t *ent, int aura_skillnum) {
	int slot = getAuraSlot(ent, aura_skillnum);
	if (slot == -1) {
		return 0;
	}
	return (ent->client->aura_level[slot]);
}

void giveAura(edict_t *caster, edict_t *target, int aura_skillnum, int auralevel, float auratime) {
	int slot = getAuraSlot(target, aura_skillnum);
	if (slot == -1) {
		slot = getFreeAuraSlot(target);
		if (slot == -1) {
			return;
		}
	} else {
		if ((auralevel <= target->client->aura_level[slot]) && (target->client->aura_caster[slot] != caster)) {
			return;
		}
	}
	if ((target != caster) && ((target->client->aura_type[slot] != aura_skillnum) || (target->client->aura_level[slot] != auralevel))) {
		powerupinfo_t *info = getPowerupInfo(aura_skillnum);
		gi.cprintf(target, PRINT_HIGH, "You go under %s's level %d %s\n", caster->client->pers.netname, auralevel, info->name);
	}
	target->client->aura_type[slot] = aura_skillnum;
	target->client->aura_level[slot] = auralevel;
	target->client->aura_caster[slot] = caster;
	target->client->aura_time[slot] = auratime;
}

void checkAuras(edict_t *ent) {
	int i;
	if (!ent->client)
		return;
	for (i = 0; i < 5; i++) {
		if (ent->client->aura_type[i] == 0)
			continue;
//Check both aura time and whether caster of aura still has same aura active
		if ((ent->client->aura_time[i] < level.time) || (ent->client->aura_caster[i]->client->pers.selspell != ent->client->aura_type[i])) {
			ent->client->aura_type[i] = 0;
			ent->client->aura_level[i] = 0;
			ent->client->aura_caster[i] = NULL;
			ent->client->aura_time[i] = 0;
		}
	}
}

float getMagicBonuses(edict_t *ent, int spell) {
	powerupinfo_t *info = getPowerupInfo(spell);
	float bonus = 1;

	if ((ent->client->quad_framenum > level.framenum) && ((info->isspell & 64) != 64)) { // quad for non-auras
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
		bonus *= 3;
	}
	if ((info->isspell & 1) == 1) { // Mage level bonus
		bonus += 0.002 * pow(ent->client->pers.skills.classLevel[3], 1.5);
		bonus += 0.05 * ent->client->pers.skill[25]; // Only mage-spells get bonus from Spell affinity
	}
	if ((info->isspell & 2) == 2) { // Cleric level bonus
		bonus += 0.015 * pow(ent->client->pers.skills.classLevel[2], 1.1);
	}
//Technicians get very high damage bonus to his "spells", but instead their base damage is fairly low.
	if ((info->isspell & 4) == 4) { // Technician level bonus
		bonus += 0.025 * pow(ent->client->pers.skills.classLevel[1], 1.1);
	}
	if ((info->isspell & 8) == 8) { // Vampire level bonus
		bonus += 0.015 * pow(ent->client->pers.skills.classLevel[4], 1.1);
	}

	if (ent->pain_debounce_time < level.time) {
		if ((ent->client->silencer_shots > 4) && (ent->client->pers.skill[58] > 1)) {
			ent->client->silencer_shots -= 5;
		} else {
			if (bonus >= 1.75)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/amconv2.wav"), 1, ATTN_NORM, 0);
			else if (bonus >= 1.4)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/amconv1.wav"), 1, ATTN_NORM, 0);
		}
		ent->pain_debounce_time = level.time + 0.4;
	}

	return bonus;
}

/*
	-----
	Inferno
	-----
*/

void thinkInferno(edict_t *ent) {
	int dmg;
	int	lvl = ent->owner->client->pers.skill[51];

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (ent->count * 4);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (5);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	dmg = ent->radius_dmg - ((10 - ent->count) * 0.05 * ent->radius_dmg);
	if (dmg > ent->radius_dmg)
		dmg = ent->radius_dmg;
	ent->health -= T_RadiusDamage (NULL, ent, ent->owner, dmg, dmg * 0.5, ent->owner, ent->dmg_radius, true, MOD_INFERNO | MOD_MAGIC);

	ent->velocity[0] += crandom()*(32 - lvl*0);
	ent->velocity[1] += crandom()*(32 - lvl*0);
	ent->velocity[2] += crandom()*(32 - lvl*0);
	ent->velocity[0] *= 0.92;
	ent->velocity[1] *= 0.92;
	ent->velocity[2] *= 0.92;
	ent->count--;
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_NUKE2);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	if ((ent->count < 1) || (ent->health < 1))
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void castInferno(edict_t *self) {
	int	lvl = self->client->pers.skill[51];
	int cost = 20 + 1.5 * lvl;
	edict_t *ent;
	vec3_t	forward, right;
	float bonus;
	if (lvl > 40) {
		cost += 1.5 * (lvl - 40);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 51);

	ent = G_Spawn();
	ent->classid = CI_INFERNO;
	ent->classname = "inferno";

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 750 + 5 * lvl, ent->velocity);
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->svflags = SVF_DEADMONSTER;
	ent->clipmask = MASK_SOLID;
	ent->solid = SOLID_BBOX;
	ent->count = 10;
	ent->radius_dmg = (35 + 0.6 * lvl) * bonus;
	ent->health = ent->radius_dmg * 2;
	ent->dmg_radius = 95;// + 0.2 * lvl;
	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
//		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/infcast.wav"), 1, ATTN_NORM, 0);
		self->client->weapon_sound = gi.soundindex ("giex/spells/infcast.wav");
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
	}
	ent->owner = self;
	ent->think = thinkInferno;
	ent->nextthink = level.time + FRAMETIME;
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->s.effects |= EF_GRENADE;
	PlayerNoise(self, self->s.origin, PNOISE_WEAPON);

	self->client->spelltime = level.time + 0.2;
	self->client->magregentime = level.time + 0.2;

	check_dodge (self, ent->s.origin, forward, 320, 800, 0.4, 0.0);
}

/*
	-----
	Firebolt
	-----
*/

void touchFirebolt(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_FIREBOLT | MOD_MAGIC);
//	T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_FIREBALL);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLASTER);
	gi.WritePosition (ent->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (ent->s.origin, MULTICAST_PVS);


	G_FreeEdict (ent);
}

void thinkFirebolt(edict_t *ent)
{
/*	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (15);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (5);
	gi.multicast (ent->s.origin, MULTICAST_PVS);*/

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void castFirebolt(edict_t *self) {
	int	lvl = self->client->pers.skill[47];
	int cost = 8 + 1.9 * lvl;
	int speed = 750 + 18 * lvl;
	edict_t *ent;
	vec3_t	forward, right;
	float bonus;
	if (lvl > 40)
		cost += 1.9 * (lvl - 40);
	if (speed > 1800)
		speed = 1800;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 47);

	ent = G_Spawn();
	ent->classid = CI_FIREBOLT;
	ent->classname = "firebolt";
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, speed, ent->velocity);
	ent->s.effects |= EF_ROCKET;
	ent->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	ent->count = 30;
	ent->dmg = (45 + 1.75 * lvl) * bonus;
	ent->owner = self;
	ent->touch = touchFirebolt;
	ent->think = thinkFirebolt;
	ent->nextthink = level.time + FRAMETIME;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/fbcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 0.4;
	self->client->magregentime = level.time + 0.4;

	check_dodge (self, ent->s.origin, forward, 800, 0, 0.5, 0.3);
}

/*
	-----
	Fireball
	-----
*/

void touchFireball(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_FIREBALL | MOD_MAGIC);
	T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, other, ent->dmg_radius, true, MOD_FIREBALL | MOD_MAGIC);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void thinkFireball(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (15);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (5);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void castFireball(edict_t *self) {
	int	lvl = self->client->pers.skill[52];
	int cost = 30 + 5.5 * lvl;
	int speed = 700 + 7 * lvl;
	edict_t *ent;
	vec3_t	forward, right;
	float bonus;
	if (lvl > 40)
		cost += 5.4 * (lvl - 40);
	if (speed > 1800)
		speed = 1800;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 52);

	ent = G_Spawn();
	ent->classid = CI_FIREBALL;
	ent->classname = "fireball";
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, speed, ent->velocity);
//	ent->s.effects |= EF_TRACKER;
	ent->s.effects |= EF_ROCKET;
	ent->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	ent->s.effects |= EF_COLOR_SHELL;
	ent->s.renderfx |= RF_SHELL_RED;
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	ent->count = 30;
	ent->dmg = (105 + 4.5 * lvl) * bonus;
	ent->radius_dmg = (110 + 4.3 * lvl) * bonus;
	ent->dmg_radius = 120; // + 0.4 * lvl;
	ent->owner = self;
	ent->touch = touchFireball;
	ent->think = thinkFireball;
	ent->nextthink = level.time + FRAMETIME;
	ent->s.effects |= EF_GIB;
	ent->s.effects |= EF_ROCKET;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 3;
	} else {
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/fbcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 0.9;
	self->client->magregentime = level.time + 0.9;

	check_dodge_rocket(self, ent->s.origin, forward, 700, 0, ent->dmg_radius, 0.6, 0.5);
}

/*
	-----
	Plague bomb
	-----
*/

void thinkPlague2(edict_t *ent) {
	int dmg = 0;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (25);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (6);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_NUKE1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	dmg = T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, NULL, ent->dmg_radius, true, MOD_PLAGUEBOMB | MOD_MAGIC);
	ent->health -= dmg;

	//ent->radius_dmg /= 1.1;
	ent->dmg_radius += ceil(ent->count * 0.1);
	ent->count--;
	if ((ent->count < 1) || (ent->health < 1))
		ent->think = G_FreeEdict;
//	if (!visible(ent, ent->owner))
//		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void touchPlague(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->velocity[0] = 0;
	ent->velocity[1] = 0;
	ent->velocity[2] = 0;
	ent->movedir[0] = 0;
	ent->movedir[1] = 0;
	ent->movedir[2] = 0;

	//ent->s.effects |= EF_TRACKER;
	ent->s.effects |= EF_TRACKERTRAIL;
	ent->count = 10;
	ent->touch = NULL;
	ent->think = thinkPlague2;
	ent->nextthink = level.time + FRAMETIME;
}

void thinkPlague(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (25);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (6);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_NUKE1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void castPlague(edict_t *self)
{
	int	lvl = self->client->pers.skill[53];
	int cost = 32 + 4 * lvl;
	edict_t *ent;
	vec3_t	forward, right;
	float bonus;

	if (lvl > 30) {
		cost += 6 * (lvl - 30);
	}
	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 53);

	ent = G_Spawn();
	ent->classid = CI_PLAGUEBOMB;
	ent->classname = "plaguebomb";

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 850, ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	ent->count = 40;
	ent->dmg = (25 + 0.6 * lvl) * bonus;
	ent->radius_dmg = (25 + 0.6 * lvl) * bonus;
	ent->dmg_radius = 120 + 0.2 * lvl;
	if (ent->dmg_radius > 180)
		ent->dmg_radius = 180;
	ent->health = (120 + 4 * lvl) * bonus; //Amount of damage this bomb may deal.
	ent->owner = self;
	ent->touch = touchPlague;
	ent->think = thinkPlague;
	ent->nextthink = level.time + FRAMETIME;
	//ent->s.effects |= EF_TRACKER;
	//ent->s.effects |= EF_TRACKERTRAIL;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 3;
	} else {
		ent->s.sound = gi.soundindex ("giex/spells/pbfly.wav");
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/pbcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 1.1;
	self->client->magregentime = level.time + 1.1;

	check_dodge_rocket(self, ent->s.origin, forward, 480, 0, ent->dmg_radius * 2, 0.7, 0.0);
}

/*
	-----
	Corpse explosion
	-----
*/

edict_t *findreticlecorpse (edict_t *ent, double limit) {
	edict_t *found = NULL, *from;
	float fdot = -100, dot;
	vec3_t vec, forward, offset;

	VectorCopy(ent->s.origin, offset);
	offset[2] += ent->viewheight - 8;
	for (from = g_edicts ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		if (!from->takedamage)
			continue;
		if ((from->health > 0) && (from->classid != CI_M_INSANE))
			continue;
		if (from->gib_health >= 0)
			continue;
		if (!visible(ent, from) || !infront(ent, from))
			continue;

		// is this edict "better" than the last one?
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (from->s.origin, offset, vec);
		VectorNormalize (forward);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if ((dot > limit) && (dot > fdot)) {
			fdot = dot;
			found = from;
		}
	}

	return found;
}

void touchCE(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;
	if (ent->velocity[2] > 0.1)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

//	if (other->takedamage)
//		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_CORPSEEXPLOSION | MOD_MAGIC);
	T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, other, ent->dmg_radius, true, MOD_CORPSEEXPLOSION | MOD_MAGIC);
	gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

void thinkCE(edict_t *self) {
	self->solid = SOLID_BBOX;
	self->clipmask = MASK_SHOT;
	self->touch = touchCE;
	self->nextthink = level.time + 5000;
	self->think = G_FreeEdict;
}

void castCE(edict_t *self) {
	int dmg, i;
	int	lvl = self->client->pers.skill[54];
	int cost = 20 + 5 * lvl;
	int count;
	//float mult;
	edict_t *blip = NULL;
	edict_t *gib = NULL;
	vec3_t eorg;
	float bonus;
	if (lvl > 40) {
		cost += 5 * (lvl - 40);
	}

	if ((blip = findreticlecorpse(self, 0.89)) != NULL)
	{
		for (i=0 ; i<3 ; i++)
			eorg[i] = self->s.origin[i] - (blip->s.origin[i] + (blip->mins[i] + blip->maxs[i])*0.5);
		if (VectorLength(eorg) <= 1024) {
//			int n;
			if (self->client->magic < cost) {
				gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
				return;
			}
			self->client->magic -= cost;

			bonus = getMagicBonuses(self, 54);
			count = 6;
//			mult = (0.3 + 0.002 * lvl) * bonus;
//			dmg = (90 - (blip->gib_health * 0.2)) * mult;
//			gi.dprintf("mult: %f, gib: %d, dmg: %d\n", mult, blip->gib_health, dmg);
			dmg = (70 + 1.8 * lvl) * bonus;
//			gi.dprintf("bonus: %f, dmg: %d\n", bonus, dmg);

			gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//			for (n= 0; n < count; n++) {
				gib = ThrowGib (blip, "models/objects/gibs/sm_meat/tris.md2", 200, GIB_ORGANIC);
				gib->s.origin[2] -= 20;
				gib->velocity[0] = 0;
				gib->velocity[1] = 0; //Only fly straight up
				gib->owner = self;
				gib->gravity = 1.5;
				gib->enemy = blip;
				gib->takedamage = false;
				gib->svflags = SVF_DEADMONSTER;
				gib->dmg = dmg * 3;
				gib->radius_dmg = dmg * 3;
				gib->dmg_radius = 220;
				gib->nextthink = level.time + 0.3;
				gib->think = thinkCE;
				gib->s.effects |= EF_COLOR_SHELL;
				gib->s.renderfx |= RF_SHELL_RED;
				gib->s.sound = gi.soundindex ("giex/spells/gibfly.wav");
				gib->s.effects |= 0x04000000;
				gib->classid = CI_CORPSEEXPLOSION;
				gib->classname = "explosionspore";
//			}

			if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
				self->client->silencer_shots -= 2;
			} else {
				gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/cecast.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
				PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
			}

			check_dodge_area(blip, blip->s.origin, 256, 0.2);

			blip->health = blip->gib_health + 1;
			T_Damage (blip, self, self, vec3_origin, blip->s.origin, vec3_origin, (int) -(blip->gib_health * 10), 1, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
//			T_RadiusDamage (NULL, blip, self, gib->dmg / 1.5, gib->dmg / 3, NULL, gib->dmg_radius, true, MOD_CORPSEEXPLOSION | MOD_MAGIC);
//			BecomeExplosion2(blip);
			self->client->spelltime = level.time + 1.2;
			self->client->magregentime = level.time + 1.2;

		}
	}
}

/*
	-----
	Corpse drain
	-----
*/

void castCD(edict_t *self) {
	int lvl = self->client->pers.skill[55];
	int cost = (15 + 0.15 * lvl) * (self->client->max_magic / 750.0);
	int gain = 0, maxgain = 30 + 0.75 * lvl, i;
	float mult = 0.1 + 0.002 * lvl;
	float maxh = 1.5 + 0.01 * lvl;
	edict_t *blip = NULL;
	vec3_t eorg;
	float bonus;
	char msg[256];
	if (lvl > 40) {
		cost += 5 * (lvl - 40);
	}


	if ((blip = findreticlecorpse(self, 0.89)) != NULL) {
		// check for radius
		for (i=0 ; i<3 ; i++)
			eorg[i] = self->s.origin[i] - (blip->s.origin[i] + (blip->mins[i] + blip->maxs[i])*0.5);
		if (VectorLength(eorg) <= 256) {
			if (self->health >= (self->max_health * maxh - 10)) {
				gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
				return;
			}
			if (self->client->magic < cost) {
				gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
				return;
			}

			bonus = getMagicBonuses(self, 55);
			mult *= bonus;
			self->client->magic -= cost;

//			gi.dprintf("Draining, health: %i, gib_health: %i\n", blip->health, blip->gib_health);
			if ((blip->health - blip->gib_health) < maxgain) {
				gain = (blip->health - blip->gib_health);
			} else {
				gain = maxgain;
			}
			if ((self->health + gain * mult) < self->health) {
				sprintf(msg, "Corpse drain tried to give %s negative health\nHealth: %d - Corpse health: %d - Corpse gibhealth: %d - mult: %f\n", self->client->pers.netname, self->health, blip->health, blip->gib_health, mult);
				logmsg(msg);
				return;
			}
			self->health += gain * mult;
			if (self->health >= self->max_health * maxh) {
				self->health = self->max_health * maxh;
			}

			gi.sound (blip, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 0.5, ATTN_NORM, 0);
			ThrowGib (blip, "models/objects/gibs/sm_meat/tris.md2", gain * 2, GIB_ORGANIC);
			T_Damage(blip, self, self, eorg, blip->s.origin, vec3_origin, -(blip->gib_health / 10), 0, 0, 0);
			if (blip->health <= blip->gib_health) {
				int n;
				T_Damage(blip, self, self, eorg, blip->s.origin, vec3_origin, 5000, 0, 0, 0);
				for (n= 0; n < 2; n++) {
					ThrowGib (blip, "models/objects/gibs/sm_meat/tris.md2", gain * 2, GIB_ORGANIC);
				}
			}


			if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
				self->client->silencer_shots -= 1;
			} else {
				gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/cdcast.wav"), 0.5, ATTN_NORM, 0);
				PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
			}
			self->client->spelltime = level.time + 0.1;
			self->client->magregentime = level.time + 0.5;
		}
	}
}

/*
	-----
	Create health
	-----
*/

void castCHealth(edict_t *self)
{
	int amount;
	int	lvl = self->client->pers.skill[56];
	int cost = 25 + 5 * lvl;
	gitem_t *item;
	edict_t *pack;
	float bonus;
	if (lvl > 40) {
		cost += 5 * (lvl - 40);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	bonus = getMagicBonuses(self, 56);
	self->client->magic -= cost;

	amount = (50 + 10 * lvl) * bonus;

	item = FindItem("Health");
	item->world_model = "models/items/healing/large/tris.md2";
	pack = Drop_Item(self, item);
//	gi.setmodel (pack, "models/items/healing/large/tris.md2");
	pack->count = amount;
	pack->style = 1; //HEALTH_IGNORE_MAX;
	pack->owner = self;
	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/chcast.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
	}
	self->client->spelltime = level.time + 1.5 - 0.008 * lvl;
	self->client->magregentime = level.time + 1.5;
}

/*
	-----
	Create armor
	-----
*/

void castCArmor(edict_t *self) {
	int amount;
	int	lvl = self->client->pers.skill[57];
	int cost = 25 + 5 * lvl;
	gitem_t *item;
	edict_t *pack;
	float bonus = getMagicBonuses(self, 57);
	if (lvl > 40) {
		cost += 5 * (lvl - 40);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	amount = (50 + 10 * lvl) * bonus;

	item = FindItem("Body Armor");
	pack = Drop_Item(self, item);
	gi.setmodel (pack, "models/items/armor/jacket/tris.md2");
	pack->count = amount;
	pack->owner = self;
	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/cacast.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
	}
	self->client->spelltime = level.time + 1.5 - 0.008 * lvl;
	self->client->magregentime = level.time + 1.5;
}

/*
	-----
	Silencer
	-----
*/

void castSilence(edict_t *self) {
	int	lvl = self->client->pers.skill[58];
	int cost = 22 + 4 * lvl;

	edict_t *ent = NULL;

	float bonus = getMagicBonuses(self, 58);
	int amount = (200 + 50 * lvl) * bonus;
	int radius = 256 + 25 * lvl;

	if (lvl > 8) {
		cost += 4 * lvl;
	}

	self->client->spelltime = level.time + 0.3;
	self->client->magregentime = level.time + 0.3;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	if (deathmatch->value && !teams->value) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/silcast.wav"), 1, ATTN_NORM, 0);
		self->client->silencer_shots += amount;
		return;
	}
	while ((ent = findradius(ent, self->s.origin, radius)) != NULL)
	{
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;
		if (!coop->value && teams->value && (ent->count != self->count))
			continue;
		if (ent->health < 1)
			continue;

		ent->client->silencer_shots += amount;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("giex/spells/silcast.wav"), 1, ATTN_NORM, 0);
	}
}



/*
	-----
	LaserMine
	-----
*/

void touchLaserGrid(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->takedamage)
		return;
	if (surf) {
/*		if (ent->enemy && ent->enemy->inuse) {
			G_FreeEdict (ent->enemy);
			ent->enemy = NULL;
		}
		if (!ent->oldenemy || !ent->oldenemy->inuse) {
			G_FreeEdict (ent);
			return;
		}*/
		ent->count = 50;
		ent->velocity[0] = 0;
		ent->velocity[1] = 0;
		ent->velocity[2] = 0;
	}
}

void thinkLaserGrid(edict_t *ent)
{
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	trace_t	tr;
	int i;

	if (ent->owner->health <= 0) {
		G_FreeEdict(ent);
		return;
	}

	if (ent->enemy) {
		if (!ent->enemy->inuse)
			ent->enemy = NULL;
		else if (visible(ent, ent->enemy)) {
			VectorMA (ent->enemy->absmin, 0.5, ent->enemy->size, point);

			VectorSubtract (point, ent->s.origin, dir);
			VectorNormalize (dir);

			ignore = ent;
			VectorCopy (ent->s.origin, start);

			i = 10;
			while(i > 0)
			{
				i--;
				tr = gi.trace (start, NULL, NULL, ent->enemy->s.origin, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

				if (!tr.ent)
					break;

				// hurt it if we can
				if ((tr.ent->takedamage) && (tr.ent != ent->owner)/* && (tr.ent->nextthink >= level.time - 0.5)*/) {
					T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, ent->dmg, 1, DAMAGE_ENERGY, MOD_LASERMINE | MOD_MAGIC);
					ent->radius_dmg--; //Dealing damage zaps their battery
				}

				// if we hit something that's not a monster or player we're done
				if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_LASER_SPARKS);
					gi.WriteByte (4);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (ent->s.skinnum);
					gi.multicast (tr.endpos, MULTICAST_PVS);
					break;
				}
				ignore = tr.ent;
				VectorCopy (tr.endpos, start);
			}

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (ent->s.origin);
			gi.WritePosition (ent->enemy->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	}

	ent->count--;
	if ((ent->count < 1) || (ent->radius_dmg < 1)) {
		ent->think = G_FreeEdict;
	}
	ent->nextthink = level.time + FRAMETIME;
}

void castLaserGrid(edict_t *self) {
	int	lvl = self->client->pers.skill[59];
	int cost = 25 + 2 * lvl;
	edict_t *ent, *scan;
	vec3_t	forward, right, v;
	float bonus = getMagicBonuses(self, 59);
	if (lvl > 40) {
		cost += 2 * (lvl - 40);
	}
	cost *= 1.0 / (float) ceil(self->client->pers.skills.classLevel[1] * 0.2);

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	ent = G_Spawn();
	ent->classid = CI_LASERMINE;
	ent->classname = "laser_mine";

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 1500 /*+ 6.5 * lvl*/, ent->velocity);
	ent->count = 30;
	ent->dmg = (6 + 0.45 * lvl) * bonus;
	ent->radius_dmg = 10; // They can deal damage 10 times.

	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = 0;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	ent->takedamage = false;
	ent->owner = self;
	ent->touch = touchLaserGrid;
	ent->think = thinkLaserGrid;
	ent->nextthink = level.time + FRAMETIME;

	ent->enemy = NULL;
	ent->oldenemy = NULL;
	for (scan = g_edicts ; scan < &g_edicts[globals.num_edicts]; scan++) {
//		gi.dprintf("%s at %s\n", scan->classname, vtos(scan->s.origin));
		if (!scan->inuse)
			continue;
		if (scan->solid == SOLID_NOT)
			continue;
		if (scan->classid != CI_LASERMINE)
			continue;
		if (scan == ent)
			continue;
		if (scan->owner != self)
			continue;
		if (ent->enemy && (ent->enemy->count > scan->count))
			continue;

		VectorSubtract (ent->s.origin, scan->s.origin, v);
//		gi.dprintf("len: %f", VectorLength(v));
		if (VectorLength(v) > 1024)
			continue;

		ent->enemy = scan;
		scan->oldenemy = ent;
	}

//	if (ent->enemy)
//		gi.dprintf("mine at %s found %s at %s\n", vtos(ent->s.origin), ent->enemy->classname, vtos(ent->enemy->s.origin));

//	ent->s.effects |= EF_TRACKER;
//	ent->s.effects |= EF_TRACKERTRAIL;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 3;
	} else {
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/lmcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
		ent->s.sound = gi.soundindex ("giex/spells/lmfly.wav");
	}

	self->client->spelltime = level.time + 0.5;
	self->client->magregentime = level.time + 0.5;

	check_dodge (self, ent->s.origin, forward, 350, 0, 0.6, 0.0);
}



/*
	-----
	LifeDrain
	-----
*/

void thinkLifeDrain2(edict_t *ent) {
	int	lvl = ent->owner->client->pers.skill[60];
	int cost = 8 + 0.6 * lvl;
	float bonus;
	int damage;
	vec3_t dir, own, ene;
	if (lvl > 40) {
		cost += 0.5 * (lvl - 40);
	}

	ent->nextthink = level.time + FRAMETIME;
	ent->owner->client->spelltime = level.time + 0.5;
	ent->owner->client->magregentime = level.time + 0.5;

	if (ent->owner->client->magic < cost) {
		VectorMA (ent->owner->s.origin, -1, ent->enemy->s.origin, dir);
		T_Damage (ent->owner, ent, ent->owner, dir, ent->owner->s.origin, vec3_origin, 100000, 1, DAMAGE_NO_PROTECTION, MOD_DRAIN | MOD_MAGIC);
		ent->owner->client->weapon_sound = 0;
		ent->think = G_FreeEdict;
		return;
	}
	bonus = getMagicBonuses(ent->owner, 60);
	ent->owner->client->magic -= cost;

	VectorMA (ent->enemy->s.origin, -1, ent->owner->s.origin, dir);
//	if (ent->enemy->client)
	damage = (int) ceil((6.5 + 0.3 * lvl) * bonus);
	T_Damage (ent->enemy, ent, ent->owner, dir, ent->enemy->s.origin, vec3_origin, damage, 0, DAMAGE_NO_ARMOR, MOD_DRAIN | MOD_MAGIC);
//	else
//		T_Damage (ent->enemy, ent, ent->owner, dir, ent->enemy->s.origin, vec3_origin, (int) ceil(2 + 0.6 * lvl), 0, DAMAGE_NO_ARMOR, MOD_DRAIN);
	if (ent->owner->health < 2 * ent->owner->max_health)
		ent->owner->health += (int) ceil((3.25 + 0.15 * lvl) * bonus);
	if (ent->enemy->health < 1) {
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->enemy->s.origin, vec3_origin, (int) -(ent->enemy->gib_health * 10), 1, DAMAGE_NO_PROTECTION, MOD_DRAIN | MOD_MAGIC);
		ent->owner->client->weapon_sound = 0;
		ent->think = G_FreeEdict;
		return;
	}

	if ((visible(ent->owner, ent->enemy)) && (infront(ent->owner, ent->enemy)) && (ent->owner->health > 0)) {
		VectorCopy(ent->owner->s.origin, own);
		own[2] += ent->owner->viewheight - 8;
		VectorCopy(ent->enemy->s.origin, ene);
		ene[2] += ent->enemy->viewheight - 8;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (ent - g_edicts);
		gi.WritePosition (own);
		gi.WritePosition (ene);
		gi.multicast (ent->owner->s.origin, MULTICAST_PVS);
	} else {
		ent->owner->client->weapon_sound = 0;
		ent->think = G_FreeEdict;
	}
}

void thinkLifeDrain(edict_t *ent)
{
	vec3_t own, ene;
	ent->owner->client->spelltime = level.time + 2;
	ent->owner->client->magregentime = level.time + 2;
	ent->nextthink = level.time + FRAMETIME;

	if (ent->count < 1) {
		ent->think = thinkLifeDrain2;
		ent->owner->client->weapon_sound = gi.soundindex("giex/spells/ldfly.wav");
		return;
	}
	if ((visible(ent->owner, ent->enemy)) && (ent->owner->health > 0)) {
		VectorCopy(ent->owner->s.origin, own);
		own[2] += ent->owner->viewheight - 8;
		VectorCopy(ent->enemy->s.origin, ene);
		ene[2] += ent->enemy->viewheight - 8;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (ent - g_edicts);
		gi.WritePosition (own);
		gi.WritePosition (ene);
		gi.multicast (ent->owner->s.origin, MULTICAST_PVS);

		ent->count--;
	} else {
		ent->think = G_FreeEdict;
	}
}

void castLifeDrain(edict_t *self)
{
	int	lvl = self->client->pers.skill[60];
	edict_t *ent;
	trace_t tr;
	vec3_t	forward, offset, end;

	if (self->client->magic < 2 + 0.2 * lvl) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= 2 + 0.2 * lvl;

	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, offset);
	offset[2] += self->viewheight - 8;
	VectorMA(offset, 256, forward, end);

	self->client->spelltime = level.time;
	self->client->magregentime = level.time;
	tr = gi.trace (offset, NULL, NULL, end, self, CONTENTS_MONSTER);
	if (teams->value && tr.ent->client && (tr.ent->count == self->count)) {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (self - g_edicts);
		gi.WritePosition (offset);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		return;
	}
	if ((tr.fraction < 1.0) && (tr.ent->takedamage) && (tr.ent->health > 0)) {

		if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
			self->client->silencer_shots -= 3;
		} else {
			gi.sound(tr.ent, CHAN_ITEM, gi.soundindex("giex/spells/ldcast.wav"), 1, ATTN_NORM, 0);
		}
		ent = G_Spawn();
		ent->classid = CI_LIFEDRAIN;
		ent->classname = "lifedrain";
		ent->count = 10;
		ent->think = thinkLifeDrain;
		ent->nextthink = level.time + FRAMETIME;
		ent->owner = self;
		ent->enemy = tr.ent;
		self->client->spelltime = level.time + 2;
		self->client->magregentime = level.time + 2;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (offset);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


/*
	-----
	Corpse spores
	-----
*/
edict_t *findreticletarget (edict_t *ent, double limit) {
	edict_t *found = NULL, *from;
	float fdot = -100, dot;
	vec3_t vec, forward, offset;

	VectorCopy(ent->s.origin, offset);
	offset[2] += ent->viewheight - 8;
	for (from = g_edicts ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		if (!from->takedamage)
			continue;
		if ((from->client) && (coop->value || (teams->value && (from->count == ent->count))))
			continue;
		if (from->health <= 0)
			continue;
		if (!visible(ent, from) || !infront(ent, from))
			continue;

		// is this edict "better" than the last one?
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (from->s.origin, offset, vec);
		VectorNormalize (forward);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if ((dot > limit) && (dot > fdot)) {
			fdot = dot;
			found = from;
		}
	}

	return found;
}

void touchSpores(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (other == ent->owner)
		return;
	if (other == ent->goalentity)
		return;
	if (other && (other->classid == CI_CORPSESPORE))
		return;

//	if (other)
//		gi.dprintf("%s hit %s\n", ent->classname, other->classname);

	if (surf && !other) {
		if (surf->flags & SURF_SKY)
			G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		if (ent->classid == CI_CORPSESPORE)
			T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SPORE | MOD_MAGIC);
		else if (ent->classid == CI_GIBRAIN)
			T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_GIBRAIN | MOD_MAGIC);
		else
			T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BURST | MOD_MAGIC);
		gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		G_FreeEdict(ent);
	}
}

void thinkSpores(edict_t *ent) {
	vec3_t	start;
	vec3_t	end;
	vec3_t	aim;
//	edict_t *scan = NULL;
	ent->touch = touchSpores;
	ent->solid = SOLID_BBOX;
	ent->clipmask = MASK_SHOT;
	ent->nextthink = level.time + 0.1;

	ent->count--;
	if (ent->count < 0) {
		G_FreeEdict(ent);
		return;
	}
/*	if ((ent->enemy) && ((ent->enemy->health <= 0) || (!ent->enemy->inuse) || (!visible(ent->owner, ent->enemy))))
		ent->enemy = NULL;
	// scan through all edicts to find a target
	while (!ent->enemy && (scan = findradius(scan, ent->s.origin, 2048)) != NULL) {
		if (!scan->inuse)
			continue;
		if (!scan->takedamage)
			continue;
		if (scan == ent->owner)
			continue;
		if (scan->health <= 0)
			continue;
		if (scan->client) {
			if (coop->value)
				continue;
			if (teams->value && (ent->owner->count == scan->count))
				continue;
		}
		if (!visible(ent->owner, scan))
			continue;
		if (!infront_aim(ent->owner, scan))
			continue;
		ent->enemy = scan;
	}*/

	ent->enemy = findreticletarget(ent->owner, 0.89);
	if (!ent->enemy)
		return;

	VectorCopy (ent->s.origin, start);
	VectorCopy (ent->enemy->s.origin, end);
	end[2] += ent->enemy->viewheight;

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);
	VectorCopy (ent->velocity, end);
	VectorNormalize (end);
	VectorMA(end, 0.8, aim, aim);
	VectorNormalize (aim);
	VectorScale(aim, ent->radius_dmg, ent->velocity);
	ent->radius_dmg *= 1.01;
}

void castSpores(edict_t *self) {
	int	lvl = self->client->pers.skill[61];
	int cost = 40 + 6 * lvl;
	float bonus;
	int count = 10;
	int dmg = 18 + 1.8 * lvl;
	int spd = 600;
	edict_t *blip = NULL;
	vec3_t eorg;
	if (lvl > 30) {
		cost += 6 * (lvl - 30);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}

	if ((blip = findreticlecorpse(self, 0.89)) != NULL)
	{
		int i;
		for (i=0 ; i<3 ; i++)
			eorg[i] = self->s.origin[i] - (blip->s.origin[i] + (blip->mins[i] + blip->maxs[i])*0.5);
		if (VectorLength(eorg) <= 1024) {
			edict_t *gib;
			int n;
			bonus = getMagicBonuses(self, 61);
			dmg *= bonus;

			gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
			for (n = 0; n < count; n++) {
				gib = ThrowGib(blip, "models/objects/gibs/sm_meat/tris.md2", 350, GIB_ORGANIC);
				gib->owner = self;
				gib->dmg = dmg;
				gib->radius_dmg = spd; //store the "attack speed" in radius_dmg
				gib->think = thinkSpores;
				gib->nextthink = level.time + 0.5 + 0.5 * random();
				gib->enemy = NULL;
				gib->count = 40;
				gib->s.sound = gi.soundindex ("giex/spells/gibfly.wav");
				gib->classid = CI_CORPSESPORE;
				gib->classname = "corpsespore";
			}
			
			if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
				self->client->silencer_shots -= 3;
			} else {
				gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/cscast.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
				PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
			}
			T_Damage(blip, self, self, eorg, blip->s.origin, vec3_origin, 5000, 0, 0, 0);
			self->client->magic -= cost;
			self->client->spelltime = level.time + 1.2;
			self->client->magregentime = level.time + 1.2;
		}
	}
}


/*
	-----
	Gib rain
	-----
*/
void thinkGibRain(edict_t *ent) {
	int	lvl = ent->owner->client->pers.skill[62];

	edict_t *gib;
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t	aim;
	vec3_t	dir;
	vec3_t	end;
	float	u;
	int cost = 4 + 0.8 * lvl;
	float bonus = getMagicBonuses(ent->owner, 62);
	if (lvl > 40) {
		cost += 0.8 * (lvl - 40);
	}

	ent->count--;
	if ((ent->count < 0) || (ent->enemy->health <= 0) || (ent->owner->health <= 0)) {
		G_FreeEdict(ent);
		return;
	}

	if (ent->owner->client->magic < cost) {
		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		G_FreeEdict(ent);
		return;
	}
	ent->owner->client->magic -= cost;

	gib = ThrowGib (ent->owner, "models/objects/gibs/sm_meat/tris.md2", 50, GIB_ORGANIC);
	gib->owner = ent->owner;
	gib->dmg = ent->dmg * bonus;
	gib->clipmask = MASK_SHOT;
	gib->solid = SOLID_BBOX;
	gib->touch = touchSpores;
	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 2;
	gib->s.sound = gi.soundindex("giex/spells/gibfly.wav");
	gib->classid = CI_GIBRAIN;
	gib->classname = "gibrain";

	VectorCopy (gib->s.origin, start);
	VectorCopy (ent->enemy->s.origin, end);
	end[2] += ent->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, right, up);

	u = (int) ceil((VectorLength(aim) + 1.2 * aim[2]) * 0.4);

	VectorMA (start, 8192, forward, end);
	VectorMA (end, u, up, end);

	VectorSubtract (end, start, aim);
	VectorNormalize (aim);
	AngleVectors (ent->owner->client->v_angle, forward, NULL, NULL);
	VectorNormalize (forward);
	VectorAdd (aim, forward, aim);
	VectorNormalize (aim);
	VectorScale(aim, 1500, gib->velocity);
	AngleVectors (aim, NULL, NULL, up);
	VectorMA(aim, 200, up, aim);

	gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/grcast.wav"), 1, ATTN_NORM, 0);
	ent->nextthink = level.time + FRAMETIME;
	ent->owner->client->spelltime = level.time + 0.8;
	ent->owner->client->magregentime = level.time + 1;

	check_dodge (ent->owner, gib->s.origin, forward, (int) ent->radius_dmg, (int) (ent->radius_dmg / 1.5), 0.05, 0.3);
}

void castGibRain(edict_t *self) {
	int	lvl = self->client->pers.skill[62];
	edict_t *ent;
	trace_t tr;
	vec3_t	forward, offset, end;

	if (self->client->magic < 20) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= 2;

	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, offset);
	offset[2] += self->viewheight - 8;
	VectorMA(offset, 2048, forward, end);

	self->client->spelltime = level.time;
	self->client->magregentime = level.time;
	tr = gi.trace (offset, NULL, NULL, end, self, CONTENTS_MONSTER);
	if (teams->value && tr.ent->client && (tr.ent->count == self->count)) {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (self - g_edicts);
		gi.WritePosition (offset);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		return;
	}
	if ((tr.fraction < 1.0) && (tr.ent->takedamage) && (tr.ent->health > 0)) {
		ent = G_Spawn();
		ent->classid = CI_GIBRAINSPAWNER;
		ent->classname = "gibrain_spawner";
		ent->count = 50;
		ent->think = thinkGibRain;
		ent->nextthink = level.time + FRAMETIME;
		ent->owner = self;
		ent->enemy = tr.ent;
		ent->dmg = 16 + 0.7 * lvl;
		self->client->spelltime = level.time + 2;
		self->client->magregentime = level.time + 2;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (offset);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

/*
	-----
	Blizzard
	-----
*/

void castBlizzard(edict_t *self) {
//	int	lvl = self->client->pers.skill[61];
//	int cost = 15 + 5 * lvl;
	int i;
//	edict_t *ent;
	vec3_t	offset, end;
	vec3_t	forward, right, up;
	trace_t tr;
	float r, u;
	float bonus = getMagicBonuses(self, self->client->pers.selspell);

	AngleVectors(self->client->v_angle, forward, right, up);
	VectorCopy(self->s.origin, offset);
	offset[2] += self->viewheight - 8;
	VectorMA(offset, 14, forward, offset);

	for (i = 0; i < 5; i++) {
		VectorMA(offset, 256, forward, end);
		r = crandom()*180;
		u = crandom()*40;
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (offset);
		gi.WritePosition (end);
		gi.multicast (offset, MULTICAST_PVS);

		tr = gi.trace (offset, NULL, NULL, end, self, CONTENTS_MONSTER);
		if ((tr.fraction < 1.0) && (tr.ent->takedamage) && (tr.ent->health > 0)) {
			vec3_t	dir;
			VectorMA (offset, -1, tr.ent->s.origin, dir);
			T_Damage (tr.ent, self, self, dir, self->s.origin, vec3_origin, 2, 1, DAMAGE_NO_ARMOR, MOD_LEECH | MOD_MAGIC);
			if (tr.ent->speed > -0.8)
				tr.ent->speed -= 0.02 * bonus;
		}
	}

	self->client->spelltime = level.time + 0.1;
	self->client->magregentime = level.time + 0.1;
}



/*
	-----
	Black hole
	-----
*/
/*
void thinkBlackHole2(edict_t *ent) {
	edict_t *scan = NULL;
	vec3_t v;
	float points, dist;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->dmg_radius += ent->count/2;

	if (!visible(ent, ent->owner) || (ent->owner->health <= 0)) {
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	while ((scan = findradius(scan, ent->s.origin, ent->dmg_radius)) != NULL)
	{
		if (scan == ent)
			continue;
//		if (scan == ent->owner)
//			continue;
		if (coop->value && scan->client && (ent->owner != scan))
			continue;
		if (teams->value && scan->client && (ent->owner->count == scan->count) && (ent->owner != scan))
			continue;
		if (!scan->takedamage)
			continue;

		VectorAdd (scan->mins, scan->maxs, v);
		VectorMA (scan->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		dist = VectorLength(v);
		points = ent->radius_dmg * ( (float)(ent->dmg_radius - dist) / (float)ent->dmg_radius);
		if (dist < 64)
			points += 10 * ent->dmg;
		else if (dist < 256)
			points += 10 * ent->dmg * ( (float)(256 - dist) / (float)256);

		if (points > 0)
		{
			if (CanDamage (scan, ent))
			{
//				edict_t *tmp = scan->groundentity;
				scan->groundentity = NULL;
				VectorSubtract (scan->s.origin, ent->s.origin, v);
				T_Damage (scan, ent, ent->owner, v, ent->s.origin, vec3_origin, (int)ceil(0.1 * points), (int)-(0.2 * ceil(points)), DAMAGE_RADIUS, MOD_BLACKHOLE);
//				scan->groundentity = tmp;
			}
		}
	}

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void touchBlackHole(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->velocity[0] = 0;
	ent->velocity[1] = 0;
	ent->velocity[2] = 0;
	ent->movedir[0] = 0;
	ent->movedir[1] = 0;
	ent->movedir[2] = 0;

	ent->touch = NULL;
	ent->think = thinkBlackHole2;
	ent->nextthink = level.time + FRAMETIME;
}

void thinkBlackHole(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (25);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (6);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_NUKE1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	ent->nextthink = level.time + FRAMETIME * 5;
}
*/
void castBlackHole(edict_t *self)
{
//	edict_t *ent;
	//vec3_t	offset, start;
//	vec3_t	forward, right;
	//float r, u;
//	int count;

	gi.cprintf(self, PRINT_HIGH, "This spell has been disabled for now\n");
	return;
	/*if (self->client->magic < 25) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	count = (self->client->magic - 25) / 4;
	self->client->magic = 0;

	ent = G_Spawn();

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 480, ent->velocity);
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	ent->count = 40;
	ent->dmg = 75 + 0.5 * count;
	ent->radius_dmg = 2 + 0.1 * count;
	ent->dmg_radius = 64 + 0.5 * count;
	ent->s.sound = gi.soundindex ("weapons/rockfly.wav");
	ent->owner = self;
	ent->touch = touchBlackHole;
	ent->think = thinkBlackHole;
	ent->nextthink = level.time + FRAMETIME;
	ent->s.effects |= EF_TRACKER;
	ent->s.effects |= EF_TRACKERTRAIL;

	gi.sound(self, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(self, self->s.origin, PNOISE_WEAPON);

	self->client->spelltime = level.time + 15.0;
	self->client->magregentime = level.time + 15.0;*/
}

/*
 ---------
 Lightning
 ---------
*/
/*
void checkLightning(edict_t *ent)
{
	int	lvl = ent->owner->client->pers.skill[50];
	int cost = 6 + 0.5 * lvl;
	float count;
	float bonus = getMagicBonuses(ent->owner, 50);
	int damage;
	vec3_t	dir;

	if (!ent->enemy)
		return;
	if (ent->owner->client->magic < cost) {
		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	ent->owner->client->magic -= cost;

	count = (float) ent->dmg * 0.25;
	damage = ((1 + count * count) * (8 + 0.4 * lvl)) * bonus;
	ent->dmg = 1;
	VectorMA (ent->owner->s.origin, -1, ent->enemy->s.origin, dir);
	T_Damage (ent->enemy, ent, ent->owner, dir, ent->enemy->s.origin, vec3_origin, damage, 0, DAMAGE_NO_ARMOR, MOD_LIGHTNING);
	gi.sound(ent->enemy, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 1, ATTN_NORM, 0);
	if (ent->enemy->health <= 0)
		ent->enemy = NULL;
}
*/
void thinkLightning(edict_t *ent) {
	int	lvl = ent->owner->client->pers.skill[50];
	int cost = 8 + 0.3 * lvl;
	int i, damage;
	trace_t tr;
	vec3_t	forward, offset, end, aim, dir;
	float bonus = getMagicBonuses(ent->owner, 50);
	if (lvl > 40) {
		cost += 0.3 * (lvl - 40);
	}
	cost *= (1 + 0.07 * ent->count);

	ent->nextthink = level.time + FRAMETIME;
	ent->owner->client->spelltime = level.time + 0.5;
	ent->owner->client->magregentime = level.time + 0.5;

	if (ent->owner->health <= 0) {
		ent->think = G_FreeEdict;
		return;
	}
	if (ent->owner->client->magic < cost) {
		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		ent->think = G_FreeEdict;
		return;
	}
	ent->owner->client->magic -= cost;

	AngleVectors(ent->owner->client->v_angle, forward, NULL, NULL);
	VectorCopy(ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	if (ent->enemy) {
		VectorSubtract(ent->enemy->s.origin, offset, aim);
		VectorNormalize(forward);
		VectorNormalize(aim);
		VectorMA(forward, 0.7, aim, forward);
		VectorNormalize(forward);
	}
	VectorMA(offset, 1024, forward, end);

	tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
	for (i = 0; i < 5; i++) {
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (offset);
		gi.WritePosition (tr.endpos);
		gi.multicast (ent->owner->s.origin, MULTICAST_PVS);
	}
	if (tr.fraction >= 1.0) {
		ent->dmg = 0;
		ent->enemy = NULL;
	} else if (!ent->enemy || (ent->enemy && (tr.ent != ent->enemy))) {
		ent->dmg = 0;
		if (tr.ent->takedamage) {
			ent->enemy = tr.ent;
		} else {
			ent->enemy = NULL;
		}
	}

	if (ent->enemy) {
		ent->dmg++;
		damage = ceil((float) (19 + lvl * 0.7) * pow((ent->dmg / 6.0), 1.1) * bonus);
		if (ent->enemy->svflags & SVF_MONSTER)
			damage *= 0.5;
//		gi.dprintf("%d %d %d\n", ent->dmg, damage, cost);
		VectorMA (ent->owner->s.origin, -1, tr.endpos, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, tr.endpos, vec3_origin, damage, 0, DAMAGE_75_PIERCE, MOD_LIGHTNING | MOD_MAGIC);
		gi.sound(ent->enemy, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 1, ATTN_NORM, 0);
		if (ent->enemy->health <= 0) {
			ent->enemy = NULL;
			ent->dmg = 0;
		}
	}
	ent->count++;

	if (!ent->owner->client->castspell) {
		ent->think = G_FreeEdict;
		return;
	}
	check_dodge (ent->owner, offset, forward, 10000, 1024, 0.2, 0.0);
}

void castLightning(edict_t *self) {
	int	lvl = self->client->pers.skill[50];
	edict_t *ent;
	//trace_t tr;
	//vec3_t	forward, start, end;

	if (self->client->magic < 10 + 0.5 * lvl) {
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
//	self->client->magic -= 2 + 0.2 * lvl; //MP deducted in think-function

/*	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight - 8;
	VectorMA(start, 512, forward, end);

	tr = gi.trace (start, NULL, NULL, end, self, CONTENTS_MONSTER);
	if (tr.fraction >= 1.0) {
		return;
	}*/
	ent = G_Spawn();
	ent->classid = CI_LIGHTNING;
	ent->classname = "lightning";
//	VectorCopy (self->s.origin, ent->s.origin);
	ent->think = thinkLightning;
	ent->nextthink = level.time + FRAMETIME;
	//ent->s.sound = gi.soundindex("giex/spells/ligcast.wav");
	ent->owner = self;
	ent->dmg = 1;
	//ent->enemy = tr.ent;
	self->client->spelltime = level.time + 3.0;
	self->client->magregentime = level.time + 3.0;
}

/*
 ------
 Flight
 ------
*/
void castFlight(edict_t *self) {
//	vec3_t	forward;
	int	lvl = self->client->pers.skill[49];
	int cost = (int) ((float)self->client->max_magic / 50.0);
	float grav = self->gravity * sv_gravity->value * FRAMETIME;
	float speed = 0.65 + 0.09 * lvl;
	if (speed > 1.0)
		speed = 1.0;
	if (cost < 5)
		cost = 5;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	self->client->spelltime = level.time + 0.1;
	self->client->magregentime = level.time + 0.1;

	self->velocity[0] *= speed;
	self->velocity[1] *= speed;
	if (self->groundentity)
		self->velocity[2] += 150;
	if (self->velocity[2] < grav + 140 + 32 * lvl)
		self->velocity[2] += grav + 50 + 9 * lvl;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 2)) {
		self->client->silencer_shots -= 1;
	} else {
		self->client->weapon_sound = gi.soundindex ("giex/spells/flifly.wav");
	}
}

/*
	-----
	Adrenaline rush
	-----
*/

void thinkARush2(edict_t *ent) {
	ent->nextthink = level.time + FRAMETIME;

	if (ent->count < 1) {
		ent->think = G_FreeEdict;
		return;
	}
	if (ent->owner->health > 50) {
		ent->owner->client->pers.add_health -= ((float)ent->dmg / 14.0);
		ent->count--;
	} else {
		ent->think = G_FreeEdict;
	}
}

void thinkARush(edict_t *ent) {
	ent->nextthink = level.time + FRAMETIME;

	if (ent->count < 1) {
		ent->nextthink = level.time + 50 * FRAMETIME;
		ent->think = thinkARush2;
		ent->count = 300;
		return;
	}
	if (ent->owner->health > 0) {
		ent->owner->client->pers.add_health += ent->dmg;
		ent->count--;
	} else {
		ent->think = G_FreeEdict;
	}
}

void castARush(edict_t *self) {
	edict_t *ent;
	int	lvl = self->client->pers.skill[48];
	int cost = 50 + 5 * lvl;
	float bonus;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	if (self->health > self->max_health) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	bonus = getMagicBonuses(self, 48);
	self->client->magic -= cost;

	ent = G_Spawn();
	ent->classid = CI_ADRENRUSH;
	ent->classname = "adrenrush";
	ent->count = 20;
	ent->dmg = (20 + 0.5 * lvl) * bonus;
	ent->think = thinkARush;
	ent->nextthink = level.time + FRAMETIME;
	ent->owner = self;
	self->client->spelltime = level.time + 4.5;
	self->client->magregentime = level.time + 4.5;
	gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/arcast.wav"), 1, ATTN_NORM, 0);

/*	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (offset);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PVS);*/
}


/*******
 The almighty PIZZA SPELL!
*******/

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void ThrowVomit (edict_t *ent, vec3_t mouth_pos, vec3_t forward, vec3_t right)
{
	edict_t *gib;

	gib = G_Spawn();
	gib->classid = CI_PIZZAGIB;
	gib->classname = "pizza_gib";

	gi.setmodel (gib, "models/objects/gibs/sm_meat/tris.md2");

	gib->solid = SOLID_TRIGGER;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;
	gib->owner = ent;

	gib->movetype = MOVETYPE_TOSS;
	gib->touch = gib_touch;

	// start the gib from out mouth, moving at a forwards velocity
	VectorCopy (mouth_pos, gib->s.origin);
	VectorScale (forward, 200 + random() * 80.0, gib->velocity);
//	VectorAdd (player_vel, gib->velocity, gib->velocity);

	// add a random left-right component to the vomit velocity
	VectorMA (gib->velocity, (random() * 48.0) - 24.0, right, gib->velocity);
	gib->velocity[2] = 120 + random() * 80;

/*	gib->avelocity[0] = random()*50.0;
	gib->avelocity[1] = random()*50.0;
	gib->avelocity[2] = random()*50.0;*/

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 2.0 + random();

	gi.linkentity (gib);
}


void castPizza(edict_t *self, qboolean first) {
	vec3_t	forward, right;
	vec3_t	mouth_pos, spew_vector;
	int i;

	AngleVectors (self->client->v_angle, forward, right, NULL);

	VectorScale (forward, 24, mouth_pos);
	VectorAdd (mouth_pos, self->s.origin, mouth_pos);
	mouth_pos[2] += self->viewheight;

	VectorScale (forward, 24, spew_vector);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLOOD);
	gi.WritePosition (mouth_pos);
	gi.WriteDir (spew_vector);
	gi.multicast (mouth_pos, MULTICAST_PVS);

	for (i = 0; i < 10; i++) {
		ThrowVomit(self, mouth_pos, forward, right);
	}
	gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/picast.wav"), 1, ATTN_NORM, 0);
	if (first) {
		if (self->client->pers.skill[64])
			gi.bprintf(PRINT_HIGH, "%s la en pizza! Usch!\n", self->client->pers.netname);
		self->client->spelltime = level.time + 3;
		self->client->magregentime = level.time + 0.2;
	}
}

/*
	-----
	Spark
	-----
*/

void thinkSpark(edict_t *ent) {
	int i;
	vec3_t	offset, end;
	vec3_t	forward, right, up;
	trace_t tr;
	float r, u;

	ent->nextthink = level.time + FRAMETIME;
	if (!ent->owner->inuse || ent->owner->health < 1) {
		ent->think = G_FreeEdict;
		ent->owner->client->spelltime = level.time + 0.6;
		ent->owner->client->magregentime = level.time + 1.4;
		return;
	}
	AngleVectors(ent->owner->client->v_angle, forward, right, up);
	VectorCopy(ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	VectorMA(offset, 14, forward, offset);

	for (i = 0; i < 4; i++) {
		VectorMA(offset, 320, forward, end);
		r = crandom()*70;
		u = crandom()*70;
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
		if ((tr.fraction < 1.0) && (tr.ent->takedamage) && (tr.ent->health > 0)) {
			vec3_t	dir;
			int damage = ent->dmg;

/*			if (tr.ent->client) {
				damage *= 0.85;
			}*/

			VectorMA (ent->owner->s.origin, -1, tr.endpos, dir);
			VectorNormalize(dir);
			T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, damage, 0, DAMAGE_75_PIERCE, MOD_SPARK | MOD_MAGIC);
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (offset);
		gi.WritePosition (tr.endpos);
		gi.multicast (offset, MULTICAST_PVS);
	}

	check_dodge (ent->owner, offset, forward, 10000, 256, 0.9, 0.0);

	ent->count--;
	if (ent->count < 0) {
//		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
		ent->think = G_FreeEdict;
	}
}
void castSpark(edict_t *self) {
	int	lvl = self->client->pers.skill[67];
	edict_t *ent;
	float bonus;
	int cost = 20 + 4 * lvl;
	if (lvl > 40) {
		cost += 4 * (lvl - 40);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 67);

	ent = G_Spawn();
	ent->classid = CI_SPARKSPAWNER;
	ent->classname = "spark_spawner";
	ent->count = 3;
	ent->think = thinkSpark;
	ent->nextthink = level.time + FRAMETIME;
	ent->owner = self;
	ent->dmg = (10 + 0.85 * lvl) * bonus;

	gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
	self->client->spelltime = level.time + 1.5;
	self->client->magregentime = level.time + 1.5;
}

/*
	----
	Bolt
	----
*/

void thinkLaser(edict_t *ent) {
	ent->nextthink = level.time + FRAMETIME;

	ent->count--;
	if (ent->count < 0) {
		ent->think = G_FreeEdict;
		return;
	}
	VectorCopy (ent->pos2, ent->s.old_origin);
}

/*		self->s.skinnum = 0xf2f2f0f0; //red
		self->s.skinnum = 0xd0d1d2d3; //green
		self->s.skinnum = 0xf3f3f1f1; //blue
		self->s.skinnum = 0xdcdddedf; //yellow
		self->s.skinnum = 0xe0e1e2e3; //orange*/

void createLaser(vec3_t start, vec3_t end, int frames, int width, int color) {
	edict_t *ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	ent->s.modelindex = 1;			// must be non-zero

	// set the beam diameter
	ent->s.frame = width;
	// set the color
	ent->s.skinnum = color;

	VectorSet (ent->mins, -8, -8, -8);
	VectorSet (ent->maxs, 8, 8, 8);
	VectorCopy (start, ent->s.origin);
	VectorCopy (end, ent->s.old_origin);
	VectorCopy (end, ent->pos2);
	gi.linkentity (ent);

	ent->count = frames;
	ent->think = thinkLaser;
	ent->nextthink = level.time + FRAMETIME;
}

void thinkBolt(edict_t *ent) {
	int i;
	vec3_t	offset, end;
	vec3_t	forward, right, up;
	trace_t tr;
	float r, u;

	ent->nextthink = level.time + FRAMETIME;
	if (!ent->owner->inuse || ent->owner->health < 1) {
		ent->think = G_FreeEdict;
		ent->owner->client->spelltime = level.time + 0.6;
		ent->owner->client->magregentime = level.time + 1.4;
		return;
	}
	AngleVectors(ent->owner->client->v_angle, forward, right, up);
	VectorCopy(ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	VectorMA(offset, 14, forward, offset);

	if (ent->count > 0) {
		for (i = 0; i < 4; i++) {
			VectorMA(offset, 200 + (6 - ent->count) * 100, forward, end);
			r = crandom() * (ent->count / 6.0) * 350.0;
			u = crandom() * (ent->count / 6.0) * 350.0;
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);

			tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BUBBLETRAIL);
			gi.WritePosition (offset);
			gi.WritePosition (tr.endpos);
			gi.multicast (offset, MULTICAST_PVS);
		}
		check_dodge (ent->owner, offset, forward, 10000, 0, 0.4, 0.0);
	}
	ent->count--;
	if (ent->count < 0) {
		if (ent->owner->client->castspell) {
			int cost = 0.1 * ent->owner->client->pers.skill[68] * ent->radius_dmg;
			if (ent->owner->client->magic < cost) {
				gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
				ent->think = G_FreeEdict;
				ent->owner->client->spelltime = level.time + 0.6;
				ent->owner->client->magregentime = level.time + 1.4;
				return;
			}
			ent->owner->client->magic -= cost;
			ent->count = 0;
			ent->radius_dmg++;
			ent->owner->client->spelltime = level.time + 1.0;
			ent->owner->client->magregentime = level.time + 2.0;
		} else {
			gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
			VectorMA(offset, 4096, forward, end);
			tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
			if ((tr.fraction < 1.0) && (tr.ent->takedamage) && (tr.ent->health > 0)) {
				int damage = ent->dmg * (1 + 0.01 * ent->radius_dmg);
				vec3_t	dir;

/*				if (tr.ent->svflags & SVF_MONSTER) {
					damage *= 1.2;
				}*/

				VectorMA (ent->owner->s.origin, -1, tr.endpos, dir);
				VectorNormalize(dir);
				T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, damage, 0, DAMAGE_75_PIERCE, MOD_BOLT | MOD_MAGIC);
			}
			for (i = 0; i < 6; i++) {
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BUBBLETRAIL);
				gi.WritePosition (offset);
				gi.WritePosition (tr.endpos);
				gi.multicast (offset, MULTICAST_PVS);
			}

			createLaser(offset, tr.endpos, 1, 2, 0xdcdddedf); //0xdcdddedf
			ent->think = G_FreeEdict;
			ent->owner->client->spelltime = level.time + 0.6;
			ent->owner->client->magregentime = level.time + 1.4;
		}
	}
}
void castBolt(edict_t *self) {
	int	lvl = self->client->pers.skill[68];
	edict_t *ent;
	float bonus;
	int cost = 40 + 6 * lvl;
	if (lvl > 40) {
		cost += 6 * (lvl - 40);
	}

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 68);

	ent = G_Spawn();
	ent->classid = CI_BOLTSPAWNER;
	ent->classname = "bolt_spawner";
	ent->count = 6;
	ent->think = thinkBolt;
	ent->nextthink = level.time + FRAMETIME;
	ent->owner = self;
	ent->dmg = (80 + 8.5 * lvl) * bonus;

	gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/ligcast.wav"), 1, ATTN_NORM, 0);
	self->client->spelltime = level.time + 1.0;
	self->client->magregentime = level.time + 2.0;
}


/*
	----------
	Laser beam
	----------
*/
void freeLaserBeam(edict_t *ent) {
	gi.dprintf("free\n");
	G_FreeEdict(ent->enemy);
	G_FreeEdict(ent);
}
void dummyLaserBeam(edict_t *ent) {
	ent->nextthink = level.time + FRAMETIME;
	gi.dprintf("dummy\n");
}
void thinkLaserBeam(edict_t *ent) {
//	int i;
	vec3_t	offset, end;
	vec3_t	forward, right, up;
	trace_t tr;
//	float r, u;
	int	lvl = ent->owner->client->pers.skill[72];
	int cost = 8 + 0.8 * lvl;

	ent->nextthink = level.time + FRAMETIME;
	if (lvl > 40) {
		cost += 1.2 * (lvl - 40);
	}
	cost *= 1.0 / (1.0 + ent->owner->client->pers.skills.classLevel[1] * 0.035);
	if (ent->count > 40) {
		cost *= 1.0 + ((float) (ent->count - 40) / 6.0);
	}

	if (ent->owner->client->magic < cost) {
		ent->think = G_FreeEdict;
		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		ent->owner->client->weapon_sound = 0;
		return;
	}
	ent->owner->client->magic -= cost;

	if (!ent->owner->inuse || ent->owner->health < 1) {
		ent->think = G_FreeEdict;
		ent->owner->client->spelltime = level.time + 0.6;
		ent->owner->client->magregentime = level.time + 1.4;
		ent->owner->client->weapon_sound = 0;
		return;
	}
	AngleVectors(ent->owner->client->v_angle, forward, right, up);
	VectorCopy(ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 4;
	VectorMA(offset, 14, forward, offset);

	ent->count++;

	ent->owner->client->spelltime = level.time + 0.6;
	ent->owner->client->magregentime = level.time + 1.4;
	if (!ent->owner->client->castspell) {
		ent->think = G_FreeEdict;
		ent->owner->client->weapon_sound = 0;
		return;
	}

//	gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
	VectorMA(offset, ent->dmg_radius, right, offset);
	VectorMA(offset, 4096, forward, end);
	tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
	if ((tr.fraction < 1.0) && (tr.ent->takedamage)) {
		int damage = ent->dmg;
		int c = ent->count;
		vec3_t	dir;

		if (tr.ent->svflags & SVF_MONSTER) {
			damage *= 1.2;
		}
		if (c > 10)
			c = 10;
		damage *= ((double) (c + 1) / 11.0);

		VectorMA (ent->owner->s.origin, -1, tr.endpos, dir);
		VectorNormalize(dir);
		T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, damage, 0, 0, MOD_LASERBEAM | MOD_MAGIC);
	}

	VectorCopy (offset, ent->s.origin);
	VectorCopy (tr.endpos, ent->s.old_origin);
	gi.linkentity(ent);
}

void castLaserBeam(edict_t *self) {
	int	lvl = self->client->pers.skill[72];
	edict_t *ent, *ent2;
	float bonus;
	int cost = 8 + 0.8 * lvl;
	bonus = getMagicBonuses(self, 72);

	if (lvl > 40) {
		cost += 1.2 * (lvl - 40);
	}
	cost *= 1.0 / (1.0 + self->client->pers.skills.classLevel[1] * 0.035);

	if (self->client->magic < cost * 5) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent = G_Spawn();
	
	ent->think = thinkLaserBeam;
	ent->nextthink = level.time + FRAMETIME;
	ent->owner = self;
	ent->dmg = (5.0 + 0.25 * lvl) * bonus;
	ent->dmg_radius = 7;
	ent->classid = CI_LASERBEAM;
	ent->classname = "laserbeam";
	ent->count = 0;

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	ent->s.modelindex = 1;			// must be non-zero
	ent->s.frame = 2;
	ent->s.skinnum = 0xf2f2f0f0;

	VectorSet (ent->mins, -8, -8, -8);
	VectorSet (ent->maxs, 8, 8, 8);
	gi.linkentity (ent);

	ent2 = G_Spawn();
	
	ent2->owner = self;
	ent2->enemy = ent;
	ent->enemy = ent2;
	ent2->dmg = (5.0 + 0.2 * lvl) * bonus;
	ent2->dmg_radius = -7;
	ent2->classid = CI_LASERBEAM;
	ent2->classname = "laserbeam";
	ent2->think = thinkLaserBeam;
	ent2->nextthink = level.time + FRAMETIME;

	ent2->movetype = MOVETYPE_NONE;
	ent2->solid = SOLID_NOT;
	ent2->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	ent2->s.modelindex = 1;			// must be non-zero
	ent2->s.frame = 2;
	ent2->s.skinnum = 0xf2f2f0f0;

	VectorSet (ent2->mins, -8, -8, -8);
	VectorSet (ent2->maxs, 8, 8, 8);
	gi.linkentity (ent2);

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 2)) {
		self->client->silencer_shots -= 5;
	} else {
		self->client->weapon_sound = gi.soundindex ("giex/spells/flifly.wav");
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/ligcast.wav"), 1, ATTN_NORM, 0);
	}
	self->client->spelltime = level.time + 1.1;
	self->client->magregentime = level.time + 2.0;
}



/*
	----
	Bite
	----
*/

void thinkBite(edict_t *ent) {
	vec3_t	start, end, dir;
	int dmg;

	ent->nextthink = level.time + FRAMETIME;
	if (!ent->owner->inuse) {
		ent->think = G_FreeEdict;
		return;
	}
	if ((ent->owner->health < 1) || (!infront(ent->owner, ent->enemy))) {
		ent->think = G_FreeEdict;
		ent->owner->client->spelltime = level.time + 0.6;
		ent->owner->client->magregentime = level.time + 1.4;
		return;
	}

	VectorCopy(ent->owner->s.origin, start);
	start[2] += ent->owner->viewheight - 8;
	VectorCopy(ent->enemy->s.origin, end);
	end[2] += ent->enemy->viewheight - 8;

	VectorSubtract(start, end, dir);
	if ((VectorLength(dir) > 250) || !infront(ent->owner, ent->enemy)) {
		ent->think = G_FreeEdict;
		return;
	}
	VectorNormalize(dir);
	dmg = T_Damage (ent->enemy, ent, ent->owner, dir, end, vec3_origin, ent->dmg, 0, DAMAGE_75_PIERCE | DAMAGE_50_MANABURN | DAMAGE_200_MANALEECH, MOD_BITE | MOD_MAGIC);
	if (dmg > 0) {
		if (ent->enemy->health < 0) {
			if (ent->owner->health < ent->owner->max_health * 2) {
				int gain = -ent->enemy->health;
				if (gain > dmg * 2)
					gain = dmg * 2;
				ent->owner->health += (int) ceil(gain * 0.5);
				if (ent->owner->health > ent->owner->max_health * 2) {
					ent->owner->health = ent->owner->max_health * 2;
				}
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (ent->enemy-g_edicts);
				gi.WriteByte (MZ_NUKE1);
				gi.multicast (ent->enemy->s.origin, MULTICAST_PVS);
			}
		} else {
			ent->owner->client->magic += dmg;
		}
	}

	ent->count--;
	if (ent->count < 0) {
//		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
		ent->think = G_FreeEdict;
	}
}
void castBite(edict_t *self) {
	vec3_t	offset, end;
	vec3_t	forward;
	trace_t	tr;

	int	lvl = self->client->pers.skill[73];
	float bonus;
	int cost = 20 + 1 * lvl;

	if (lvl > 40) {
		cost += 1 * (lvl - 40);
	}

//	if (self->client->weaponstate == WEAPON_FIRING) {
	if ((self->client->latched_buttons|self->client->buttons) & BUTTON_ATTACK) {
		return;
	}
/*	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}*/
	bonus = getMagicBonuses(self, 73);

	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorCopy(self->s.origin, offset);
	offset[2] += self->viewheight - 8;
	VectorMA(offset, 14, forward, offset);
	VectorMA(offset, 200, forward, end);

	tr = gi.trace (offset, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.fraction < 1.0) && (tr.ent->takedamage)) {
		edict_t	*ent;

		ent = G_Spawn();
		ent->classid = CI_BITE;
		ent->classname = "bite";
		ent->count = 8;
		ent->think = thinkBite;
		ent->nextthink = level.time + FRAMETIME;
		ent->owner = self;
		ent->enemy = tr.ent;
		ent->dmg = (6 + 0.35 * lvl) * bonus;

		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/ldfly.wav"), 0.8, ATTN_NORM, 0);
//		self->client->magic -= cost;
		self->client->spelltime = level.time + 1.2;
	} else {
		gi.sound(self, CHAN_VOICE, gi.soundindex("parasite/paratck4.wav"), 0.8, ATTN_NORM, 0);
//		self->client->magic -= (int) ceil((float) cost / 10.0);
		self->client->spelltime = level.time + 0.2;
	}
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (offset);
	gi.WritePosition (tr.endpos);
	gi.multicast (offset, MULTICAST_PVS);

	check_dodge (self, offset, forward, 10000, 150, 0.9, 0.0);

	self->client->magregentime = level.time + 1.5;
}






/*
 ----
 Rush
 ----
*/
void thinkRush(edict_t *ent) {
	vec3_t	forward;
	float speed = 50 + 2 * ent->dmg;
	int maxspeed = 600 + 5 * ent->dmg;
	float grav = ent->owner->gravity * sv_gravity->value * FRAMETIME;

	ent->nextthink = level.time + FRAMETIME;

	if (!ent->owner->client->castspell) {
		ent->think = G_FreeEdict;
		return;
	}

	if (VectorLength(ent->owner->velocity) < maxspeed) {
		AngleVectors(ent->owner->client->v_angle, forward, NULL, NULL);
		VectorNormalize(forward);

		ent->owner->velocity[0] += forward[0] * speed;
		ent->owner->velocity[1] += forward[1] * speed;
		ent->owner->velocity[2] += forward[2] * speed;
	}
	ent->owner->velocity[2] += grav;

	ent->count--;
	if (ent->count < 0)
		ent->think = G_FreeEdict;
}
void castRush(edict_t *self) {
	edict_t	*ent;
	int	lvl = self->client->pers.skill[74];
	int cost = 20 + 4 * lvl;

	if (self->client->spelltime > level.time - 0.7) {
		return;
	}

	cost *= ((float) (self->client->max_magic + 2000.0)) / 2000.0;
	
	if (self->client->magic < cost) {
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}

	self->client->magic -= cost;
	self->client->spelltime = level.time + 0.1;
	self->client->magregentime = level.time + 1.5;

	ent = G_Spawn();
	ent->classid = CI_RUSH;
	ent->classname = "rush";
	ent->count = 5;
	ent->think = thinkRush;
	ent->nextthink = level.time + FRAMETIME;
	ent->owner = self;
	ent->dmg = lvl;

	if (abs(self->velocity[2]) < 5)
		self->velocity[2] += 250;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 2)) {
		self->client->silencer_shots -= 1;
	} else {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/flifly.wav"), 1, ATTN_NORM, 0);
	}
}




/*
 -----
 Drone
 -----
*/

void killedDrone (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
//TODO: Anything to do here?
}

void dronerocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_DRONE | MOD_MAGIC);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, other, ent->dmg_radius, true, MOD_DRONE | MOD_MAGIC);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void touchDrone(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (surf) {
		VectorSet(ent->velocity, 0, 0, 0);
//		G_FreeEdict(ent);
	}
}

void thinkDrone(edict_t *ent) {
	vec3_t	offset, end, dir;
	vec3_t	forward;
	trace_t	tr;
	edict_t *rocket;
	float bonus = getMagicBonuses(ent->owner, 75);

	ent->nextthink = level.time + FRAMETIME;
	if (ent->owner->health < 1) {
		ent->think = G_FreeEdict;
		return;
	}

// Start check course correction
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorMA(ent->s.origin, 256, forward, end);
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
	if (tr.fraction < 1.0) { // Something in the way!
		VectorMA(forward, 0.5, tr.plane.normal, dir);
		VectorCopy (dir, ent->movedir);
		vectoangles (dir, ent->s.angles);
		VectorScale (dir, 150, ent->velocity);
	}
// End check course correction

// Start fire rocket stuff
	if ((ent->count < 160) && (ent->count % 5) == 0) {
		AngleVectors(ent->owner->client->v_angle, forward, NULL, NULL);
		VectorCopy(ent->owner->s.origin, offset);
		offset[2] += ent->owner->viewheight - 8;
		VectorMA(offset, 14, forward, offset);
		VectorMA(offset, 4096, forward, end);

		tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
		VectorSubtract(tr.endpos, ent->s.origin, dir);
		VectorNormalize(dir);
		rocket = fire_rocket(ent, ent->s.origin, dir, (int) (ent->dmg * bonus), 1500, 90, (int) (ent->dmg * bonus) + 20);
		rocket->owner = ent->owner;
		rocket->touch = dronerocket_touch;
	}
// End fire rocket stuff

	ent->count--;
	if (ent->count < 0)
		ent->think = G_FreeEdict;
//	if (!visible(ent, ent->owner))
//		ent->think = G_FreeEdict;
	ent->owner->client->magregentime = level.time + 0.5;
}

void castDrone(edict_t *self) {
	int	lvl = self->client->pers.skill[75];
	int cost = self->client->max_magic;
	edict_t *ent;
	vec3_t	forward, right;
	cost *= 1.0 / (1.0 + self->client->pers.skills.classLevel[1] * 0.025);
	if (cost < (self->client->max_magic * 0.25)) {
		cost = (self->client->max_magic * 0.25);
	}

	if (self->client->magic > self->client->max_magic)
		self->client->magic = self->client->max_magic;
	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	ent = G_Spawn();
	ent->classid = CI_TECHDRONE;
	ent->classname = "techdrone";
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 150, ent->velocity);
//	ent->s.effects |= 0x04000000;
	ent->s.effects |= EF_ROCKET;
	//ent->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	ent->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = 0; //MASK_SHOT;
	ent->count = 150;
	ent->solid = SOLID_BBOX;
	ent->dmg = 40 + 2.0 * lvl;
	ent->owner = self;
	ent->touch = touchDrone;
	ent->think = thinkDrone;
	ent->nextthink = level.time + FRAMETIME;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/fbcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 1.4;
	self->client->magregentime = level.time + 3.0;

//	check_dodge (self, ent->s.origin, forward, 800, 0, 0.5, 0.3);
}


/*
 ----------
 Redemption
 ----------
*/

void touchRedeem(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (surf) {
		VectorSet(ent->velocity, 0, 0, 0);
	}
}

void thinkRedeem(edict_t *ent) {
	vec3_t	offset, end, dir;
	vec3_t	forward;
	trace_t	tr;
	edict_t *gib;
	int i;
	float bonus;
	
	if (!ent->owner || !ent->owner->inuse) {
		ent->think = NULL;
		return;
	}

	ent->nextthink = level.time + FRAMETIME;

	if (ent->owner->health < 1) {
		T_Damage (ent, ent->owner, ent->owner, vec3_origin, ent->s.origin, vec3_origin, (int) -(ent->gib_health * 10), 1, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
		return;
	}

	bonus = getMagicBonuses(ent->owner, 76);

	//VectorSet(ent->velocity, 0, 0, 0);

	AngleVectors(ent->owner->client->v_angle, forward, NULL, NULL);
	VectorCopy(ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	VectorMA(offset, 14, forward, offset);
	VectorMA(offset, 4096, forward, end);

	tr = gi.trace (offset, NULL, NULL, end, ent->owner, MASK_SHOT);
	VectorSubtract(tr.endpos, ent->s.origin, dir);
	VectorNormalize(dir);

	for (i = 0; i < 2; i++) {
		gib = ThrowGib (ent, "models/objects/gibs/sm_meat/tris.md2", 50, GIB_ORGANIC);
		gib->owner = ent->owner;
		gib->goalentity = ent;
		gib->dmg = ent->dmg * (0.5 + bonus * 0.5);

		VectorCopy (dir, gib->movedir);
		vectoangles (dir, gib->s.angles);
		gib->velocity[0] = (1100 + 200 * random()) * dir[0];
		gib->velocity[1] = (1100 + 200 * random()) * dir[1];
		gib->velocity[2] = (1100 + 200 * random()) * dir[2];
//		VectorScale (dir, 1200, gib->velocity);
		gib->movetype = MOVETYPE_FLYMISSILE;
		gib->clipmask = MASK_SHOT;
		gib->solid = SOLID_BBOX;

		gib->touch = touchSpores;
		gib->think = G_FreeEdict;
		gib->nextthink = level.time + 2;
		gib->s.sound = gi.soundindex("giex/spells/gibfly.wav");
		gib->classid = CI_CORPSEBURST;
		gib->classname = "corpseburst";
	}

	ent->count--;
	if (ent->count < 0) {
		T_Damage (ent, ent->owner, ent->owner, vec3_origin, ent->s.origin, vec3_origin, (int) -(ent->gib_health * 10), 1, DAMAGE_NO_PROTECTION, MOD_UNKNOWN);
		//ent->think = G_FreeEdict;
	}
	ent->owner->client->magregentime = level.time + 0.5;
}

void castRedeem(edict_t *self) {
	int	lvl = self->client->pers.skill[76];
	int cost = 40 + 8 * lvl;
	edict_t *blip = NULL;
//	edict_t *gib = NULL;
	vec3_t eorg;
	if (lvl > 40) {
		cost += 8 * (lvl - 40);
	}

	if ((blip = findreticlecorpse(self, 0.89)) != NULL) {
		int i;
		for (i=0 ; i<3 ; i++)
			eorg[i] = self->s.origin[i] - (blip->s.origin[i] + (blip->mins[i] + blip->maxs[i])*0.5);
		if ((VectorLength(eorg) <= 1024) && (blip->deadflag == DEAD_DEAD)) {
			if (self->client->magic < cost) {
				gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
				return;
			}

			self->client->magic -= cost;
			blip->dmg = 40 + 0.5 * lvl;
			blip->think = thinkRedeem;
			blip->nextthink = level.time + FRAMETIME * 10;
			blip->owner = self;
			blip->count = 50;

			blip->movetype = MOVETYPE_FLYMISSILE;
			blip->clipmask = 0; //MASK_SHOT;
			blip->velocity[2] = 50;
			blip->solid = SOLID_BBOX;
			blip->touch = touchRedeem;

			if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
				self->client->silencer_shots -= 2;
			} else {
				gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/cecast.wav"), 1, ATTN_NORM, 0);//Play the spell sound!
				PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
			}

			self->client->spelltime = level.time + 1.2;
			self->client->magregentime = level.time + 1.2;

		}
	}
}


/*
	-----
	Blaze
	-----
*/

void touchBlaze(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BLAZE | MOD_MAGIC);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLASTER);
	gi.WritePosition (ent->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (ent->s.origin, MULTICAST_PVS);


	G_FreeEdict (ent);
}

void thinkBlaze(edict_t *ent) {
	vec3_t	forward, end, dir, dir2, offset;
	trace_t tr;

	if (ent->count > 28) {
		VectorCopy(ent->velocity, dir);
		VectorNormalize(dir);
		VectorMA(dir, 0.5, ent->pos2, dir);
		VectorScale (dir, ent->speed, ent->velocity);
	}
	AngleVectors (ent->owner->client->v_angle, forward, NULL, NULL);

	VectorCopy (ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	VectorMA(offset, 5000, forward, end);
	tr = gi.trace (offset, NULL, NULL, end, ent->owner, CONTENTS_SOLID|CONTENTS_MONSTER);
	if (tr.fraction < 1.0)
		VectorCopy(tr.endpos, end);

	VectorSubtract(end, ent->s.origin, dir);
	VectorNormalize(dir);
	VectorCopy(ent->velocity, dir2);
	VectorNormalize(dir2);
	VectorMA(dir2, 0.5, dir, dir2);
	VectorScale (dir2, ent->speed, ent->velocity);

//		VectorMA(dir, 0.4, ent->pos2, dir);
//		VectorScale (dir, ent->speed, ent->velocity);

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;

	ent->nextthink = level.time + FRAMETIME;
}

void castBlaze(edict_t *self) {
	int	lvl = self->client->pers.skill[78];
	int cost = 2 + 0.5 * lvl;
	int speed = 1000;
	float r,u;
	edict_t *ent;
	vec3_t	forward, right, up, aim;
	float bonus;
	if (lvl > 40)
		cost += 1.9 * (lvl - 40);
	if (speed > 1800)
		speed = 1800;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 78);

	ent = G_Spawn();
	ent->classid = CI_BLAZE;
	ent->classname = "blaze";

	AngleVectors (self->client->v_angle, forward, right, up);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;

	VectorMA(ent->s.origin, 512, forward, aim);
	r = crandom() * 128;
	u = crandom() * 16;
	if (r > 0)
		r += 128;
	else
		r -= 128;
	if (u > 0)
		u += 16;
	else
		u -= 16;
	VectorMA (aim, r * 0.15, right, aim);
	VectorMA (aim, u * 0.15, up, aim);
	VectorSubtract(aim, ent->s.origin, aim);
	VectorNormalize(aim);
	VectorCopy(aim, ent->pos1);

	VectorMA(ent->s.origin, 512, forward, aim);
	VectorMA (aim, r, right, aim);
	VectorMA (aim, u, up, aim);
	VectorSubtract(aim, ent->s.origin, aim);
	VectorNormalize(aim);
	VectorCopy(aim, ent->pos2);

	VectorCopy (ent->pos1, ent->movedir);
	VectorScale (ent->pos1, speed, ent->velocity);
	ent->speed = speed;
	ent->s.effects |= EF_GREENGIB;
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	ent->count = 30;
	ent->dmg = (15 + 0.5 * lvl) * bonus;
	ent->owner = self;
	ent->touch = touchBlaze;
	ent->think = thinkBlaze;
	ent->nextthink = level.time + FRAMETIME;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
//		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
//		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/fbcast.wav"), 1, ATTN_NORM, 0);
		self->client->weapon_sound = gi.soundindex ("giex/spells/infcast.wav");
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 0.1;
	self->client->magregentime = level.time + 0.1;

	check_dodge (self, ent->s.origin, forward, 800, 0, 0.5, 0.3);
}




/*
	-----
	Swarm
	-----
*/

void thinkSwarm(edict_t *ent) {
	int dmg = 0;
	vec3_t	forward, end, dir, dir2, offset;
	trace_t tr;

	if (ent->count > 28) {
		VectorCopy(ent->velocity, dir);
		VectorNormalize(dir);
		VectorMA(dir, 0.5, ent->pos2, dir);
		VectorScale (dir, ent->speed, ent->velocity);
	}
	AngleVectors (ent->owner->client->v_angle, forward, NULL, NULL);

	VectorCopy (ent->owner->s.origin, offset);
	offset[2] += ent->owner->viewheight - 8;
	VectorMA(offset, 5000, forward, end);
	tr = gi.trace (offset, NULL, NULL, end, ent->owner, CONTENTS_SOLID|CONTENTS_MONSTER);
	if (tr.fraction < 1.0)
		VectorCopy(tr.endpos, end);

	VectorSubtract(end, ent->s.origin, dir);
	VectorNormalize(dir);
	VectorCopy(ent->velocity, dir2);
	VectorNormalize(dir2);
	VectorMA(dir2, 0.3, dir, dir2);
	VectorScale (dir2, ent->speed, ent->velocity);

	if (!ent->owner || !ent->owner->inuse) {
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	dmg = T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg, ent->owner, ent->dmg_radius, false, MOD_SWARM | MOD_MAGIC);
	ent->health -= dmg;

	//ent->radius_dmg /= 1.1;
//	ent->dmg_radius += ceil(ent->count * 0.1);
	ent->count--;
	if (!visible(ent, ent->owner)) {
		ent->count--;
	}
	if ((ent->count < 1) || (ent->health < 1)) {
		ent->think = G_FreeEdict;
	}
	ent->nextthink = level.time + FRAMETIME;
}

void touchSwarm(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (other->takedamage)
		return;
	return;
}

void castSwarm(edict_t *self) {
	int	lvl = self->client->pers.skill[79];
	int cost = 32 + 4 * lvl;
	int speed = 450;
	edict_t *ent;
	vec3_t	forward, right;
	float bonus;

	if (lvl > 30) {
		cost += 6 * (lvl - 30);
	}
	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;
	bonus = getMagicBonuses(self, 79);

	ent = G_Spawn();
	ent->classid = CI_SWARM;
	ent->classname = "swarm";

	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy(forward, ent->movedir);
	vectoangles(forward, ent->s.angles);
	VectorScale(forward, speed, ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYRICOCHET;
	ent->clipmask = 0;
	ent->solid = SOLID_BBOX;
	ent->count = 40;
	ent->speed = speed;
	ent->radius_dmg = (25 + 0.7 * lvl) * bonus;
	ent->dmg_radius = 75;
	ent->health = (120 + 5.5 * lvl) * bonus; //Amount of damage this swarm may deal.
	ent->owner = self;
	ent->touch = touchSwarm;
	ent->think = thinkSwarm;
	ent->nextthink = level.time + FRAMETIME;
	//ent->s.effects |= EF_TRACKER;
	ent->s.effects |= EF_TRACKERTRAIL;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 3;
	} else {
		ent->s.sound = gi.soundindex ("infantry/inflies1.wav");
		gi.sound(self, CHAN_ITEM, gi.soundindex("giex/spells/pbcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 1.0;
	self->client->magregentime = level.time + 0.9;

	check_dodge_rocket(self, ent->s.origin, forward, 480, 0, ent->dmg_radius * 2, 0.7, 0.0);
}



/*******
 Storm
*******/

void thinkStorm(edict_t *ent) {
	int	lvl = ent->owner->client->pers.skill[81];
//	int cost = 10 + ceil(0.25 * lvl);
	int i;
	vec3_t	end;
	trace_t	tr;

	ent->nextthink = level.time + FRAMETIME;
	if (ent->owner->health < 1) {
		ent->think = G_FreeEdict;
		return;
	}

/*	if (lvl > 40) {
		cost += 0.25 * (lvl - 40);
	}
	if (ent->owner->client->magic < cost) {
		ent->think = G_FreeEdict;
		gi.sound(ent->owner, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	ent->owner->client->magic -= cost;*/

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_STEAM);
	gi.WriteShort(-1); // "sustain"
	gi.WriteByte (50); // count
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir); // ent->movedir
	gi.WriteByte (14); // color
	gi.WriteShort(100); // magnitude
	gi.multicast (ent->s.origin, MULTICAST_PVS);

// Start lightning stuff
	if ((ent->count % 8) == 0) {
		vec3_t offset, forward;
		vec3_t box1, box2;
		edict_t *target;// = findreticletarget(ent->owner, 0.94);

		AngleVectors(ent->owner->client->v_angle, forward, NULL, NULL);
		VectorCopy(ent->owner->s.origin, offset);
		offset[2] += ent->owner->viewheight - 8;
		VectorMA(offset, 14, forward, offset);
		VectorMA(offset, 4096, forward, end);

		tr = gi.trace (offset, NULL, NULL, end, ent->owner, CONTENTS_MONSTER);
		if ((tr.fraction >= 1.0) || (!tr.ent) || (!tr.ent->takedamage)) {
			VectorSet(box1, -30, -30, -30);
			VectorSet(box2, 30, 30, 30);

			tr = gi.trace (offset, box1, box2, end, ent->owner, CONTENTS_MONSTER);
		}
		target = tr.ent;
		if (target && target->takedamage && (target->health > 0)) {
			float bonus = getMagicBonuses(ent->owner, 81);
			if (!target->client || !OnSameTeam(ent->owner, target)) {
				VectorCopy(target->s.origin, end);
				end[2] += target->viewheight;
				tr = gi.trace (ent->s.origin, NULL, NULL, end, ent->owner, MASK_SHOT);

				if ((tr.fraction < 1.0) && (tr.ent) && (tr.ent->takedamage) && (tr.ent->health > 0)) {
					vec3_t	dir;

					VectorMA (ent->s.origin, -1, tr.endpos, dir);
					VectorNormalize(dir);
					T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, ent->dmg * bonus, 0, DAMAGE_75_PIERCE, MOD_STORM | MOD_MAGIC);
				}

				for (i = 0; i < 6; i++) {
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_BUBBLETRAIL);
					gi.WritePosition (ent->s.origin);
					gi.WritePosition (tr.endpos);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/spells/lighit.wav"), 0.8, ATTN_NORM, 0);
				createLaser(ent->s.origin, tr.endpos, 1, 2, 0xdcdddedf); //0xdcdddedf
				ent->health--;
			}
		}
	}
// End lightning stuff

	if (ent->health < 0)
		ent->think = G_FreeEdict;
//	if (!visible(ent, ent->owner))
//		ent->think = G_FreeEdict;

	ent->count--;
	if (ent->count < 1) {
		ent->think = G_FreeEdict;
		return;
	}
	ent->owner->client->magregentime = level.time + 0.5;
}

void castStorm(edict_t *self) {
	int	lvl = self->client->pers.skill[81];
	int cost = self->client->max_magic * 0.48;
//	int cost = 20 + 2 * lvl;
	edict_t *ent;
	vec3_t	forward, right;

	if (lvl > 50) {
		cost = self->client->max_magic * 0.24;
	} else if (lvl > 50) {
		cost = self->client->max_magic * 0.32;
	}
	if (self->client->magic > self->client->max_magic)
		self->client->magic = self->client->max_magic;
	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	ent = G_Spawn();
	ent->classid = CI_STORM;
	ent->classname = "storm";
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorCopy (self->s.origin, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	VectorScale (forward, 150, ent->velocity);
	ent->s.effects |= EF_FLAG2;
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYRICOCHET;
	ent->clipmask = 0; //MASK_SHOT;
	ent->count = 150;
	ent->health = 8 + ceil(0.05 * lvl);
	ent->solid = SOLID_BBOX;
	ent->dmg = 40 + 1.0 * lvl;
	ent->owner = self;
	//ent->touch = touchDrone;
	ent->think = thinkStorm;
	ent->nextthink = level.time + FRAMETIME;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		ent->s.sound = gi.soundindex ("giex/spells/firefly.wav");
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/ligcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 1.3;
	self->client->magregentime = level.time + 3.0;
}


/*******
 Spawn Idiot
*******/

void ED_CallSpawn (edict_t *ent);
void castSpawnIdiot(edict_t *self) {
	int	lvl = self->client->pers.skill[83];
	int cost = 200 + 200 * lvl;
	edict_t *ent;
	vec3_t	forward, right;

	if (self->client->magic < cost) {
		gi.sound(self, CHAN_VOICE, gi.soundindex("giex/spells/nomagic.wav"), 1, ATTN_NORM, 0);
		return;
	}
	self->client->magic -= cost;

	ent = G_Spawn();
	ent->classid = CI_M_INSANE;
	ent->classname = "misc_insane";
	ent->spawnflags &= 32; //always stand
	AngleVectors (self->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, self->client->kick_origin);

	VectorMA(self->s.origin, 128, forward, ent->s.origin);
	//ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	ED_CallSpawn (ent);
	ent->gib_health = -2000;
	ent->owner = self;

	if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 0)) {
		self->client->silencer_shots -= 2;
	} else {
		gi.sound(self, CHAN_AUTO, gi.soundindex("giex/spells/ligcast.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, self->s.origin, PNOISE_WEAPON);
	}

	self->client->spelltime = level.time + 2.0;
	self->client->magregentime = level.time + 3.0;
}



/*******
 Generic aura
*******/

void castGenericAura(edict_t *self, int aura) {
	giveAura(self, self, aura, self->client->pers.skill[aura], level.time + 5);
	if (coop->value || (deathmatch->value && teams->value)) {
		int i;
		edict_t *ent;
		for (i = 0 ; i < maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse)
				continue;
			if (!ent->client->pers.loggedin)
				continue;
			if (ent->count != self->count)
				continue;
			if (ent->health < 1)
				continue;
			if (!visible(ent, self))
				continue;
			giveAura(self, ent, aura, self->client->pers.skill[aura], level.time + 5);
		}
	}
}
void castSelfAura(edict_t *self, int aura) {
	giveAura(self, self, aura, self->client->pers.skill[aura], level.time + 5);
}



/*****
Detpipes
*****/

void Grenade_Explode (edict_t *ent);
void castDetpipes(edict_t *self) {
	int	i, lvl = self->client->pers.skill[80];
	vec3_t dir;
	edict_t *scan;

	scan = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, scan++) {
		if (!scan->inuse)
			continue;
		if ((scan->classid != CI_GRENADE) && (scan->classid != CI_HGRENADE))
			continue;
		if (scan->owner != self)
			continue;
		VectorSubtract(self->s.origin, scan->s.origin, dir);
		if (VectorLength(dir) > lvl * 100)
			continue;
		Grenade_Explode(scan);
		//scan->nextthink = level.time + FRAMETIME;
	}
	self->client->spelltime = level.time + 0.1;
}

/*****
Kill Drones
*****/

void castKillDrones(edict_t *self) {
	edict_t *scan = NULL;

	while ((scan = G_FindClassId(scan, CI_TECHDRONE)) != NULL) {
		if (scan->owner != self)
			continue;
		scan->count = 0;
	}
	scan = NULL;
	while ((scan = G_FindClassId(scan, CI_STORM)) != NULL) {
		if (scan->owner != self)
			continue;
		scan->health = -1;
	}
	self->client->spelltime = level.time + 0.1;
}



/*************
----------------
Spell calling functions
----------------
*************/

void castSpell(edict_t *self) {
	if (!self->client)
		return;
	if (self->health < 1)
		return;
	if (self->client->spelltime > level.time)
		return;
//	if (self->client->weaponstate == WEAPON_FIRING)
//		return;

	switch(self->client->pers.selspell) {
	case 47:
		if (self->client->pers.skill[47])
			castFirebolt(self);
		break;
	case 48:
		if (self->client->pers.skill[48])
			castARush(self);
		break;
	case 49:
		if (self->client->pers.skill[49])
			castFlight(self);
		break;
	case 50:
		if (self->client->pers.skill[50])
			castLightning(self);
		break;
	case 51:
		if (self->client->pers.skill[51])
			castInferno(self);
		break;
	case 52:
		if (self->client->pers.skill[52])
			castFireball(self);
		break;
	case 53:
		if (self->client->pers.skill[53])
			castPlague(self);
		break;
	case 54:
		if (self->client->pers.skill[54])
			castCE(self);
		break;
	case 55:
		if (self->client->pers.skill[55])
			castCD(self);
		break;
	case 56:
		if (self->client->pers.skill[56])
			castCHealth(self);
		break;
	case 57:
		if (self->client->pers.skill[57])
			castCArmor(self);
		break;
	case 58:
		if (self->client->pers.skill[58])
			castSilence(self);
		break;
	case 59:
		if (self->client->pers.skill[59])
			castLaserGrid(self);
		break;
	case 60:
		if (self->client->pers.skill[60])
			castLifeDrain(self);
		break;
	case 61:
		if (self->client->pers.skill[61])
			castSpores(self);
		break;
	case 62:
		if (self->client->pers.skill[62])
			castGibRain(self);
		break;
	case 63:
		if (self->client->pers.skill[63])
			castBlizzard(self);
		break;
	case 64:
		if (self->client->pers.skill[64])
			castPizza(self, true);
		break;
	case 67:
		if (self->client->pers.skill[67])
			castSpark(self);
		break;
	case 68:
		if (self->client->pers.skill[68])
			castBolt(self);
		break;
	case 72:
		if (self->client->pers.skill[72])
			castLaserBeam(self);
		break;
	case 73:
		if (self->client->pers.skill[73])
			castBite(self);
		break;
	case 74:
		if (self->client->pers.skill[74])
			castRush(self);
		break;
	case 75:
		if (self->client->pers.skill[75])
			castDrone(self);
		break;
	case 76:
		if (self->client->pers.skill[76])
			castRedeem(self);
		break;
	case 78:
		if (self->client->pers.skill[78])
			castBlaze(self);
		break;
	case 79:
		if (self->client->pers.skill[79])
			castSwarm(self);
		break;
	case 80:
		if (self->client->pers.skill[80])
			castDetpipes(self);
		break;
	case 81:
		if (self->client->pers.skill[81])
			castStorm(self);
		break;
	case 82:
		castKillDrones(self);
		break;
	case 83:
		if (self->client->pers.skill[83])
			castSpawnIdiot(self);
		break;
	default:
		break;
	}
}
void castAura(edict_t *self) {
	if (!self->client)
		return;
	if (self->health < 1)
		return;
	if (self->client->spelltime > level.time)
		return;
	if (self->client->aura_refreshtime > level.time)
		return;

	switch(self->client->pers.selspell) {
	case 65: //Regen aura
		if (self->client->pers.skill[65])
			castGenericAura(self, 65);
		break;
	case 66: //Sanc aura
		if (self->client->pers.skill[66])
			castGenericAura(self, 66);
		break;
	case 77: //Might
		if (self->client->pers.skill[77])
			castSelfAura(self, 77);
		break;
	case 84: //Anti resist
		if (self->client->pers.skill[84])
			castSelfAura(self, 84);
		break;
	default:
		break;
	}
}
void prevSpell(edict_t *self) {
	int newspell = self->client->pers.selspell - 1;
	powerupinfo_t *info;

	if (!self->client)
		return;
	if (self->client->castspell)
		return;

	while (newspell != self->client->pers.selspell) {
		info = getPowerupInfo(newspell);
		if (info->isspell && self->client->pers.skill[newspell])
			break;

		newspell--;
		if (newspell < 0)
			newspell = GIEX_PUTYPES;
	}
	if (newspell != self->client->pers.selspell)
		self->client->aura_refreshtime = level.time + 1.0;
	self->client->pers.selspell = newspell;
	if (self->client->pers.skill[self->client->pers.selspell]) {
		info = getPowerupInfo(self->client->pers.selspell);
		gi.cprintf(self, PRINT_HIGH, "%s selected\n", info->name);
	}
}

void nextSpell(edict_t *self) {
	int newspell = self->client->pers.selspell + 1;
	powerupinfo_t *info;

	if (!self->client)
		return;
	if (self->client->castspell)
		return;

	while (newspell != self->client->pers.selspell) {
		info = getPowerupInfo(newspell);
		if (info->isspell && (self->client->pers.skill[newspell]) && (newspell < GIEX_PUTYPES))
			break;

		newspell++;
		if (newspell > GIEX_PUTYPES)
			newspell = 0;
	}
	if (newspell != self->client->pers.selspell)
		self->client->aura_refreshtime = level.time + 1.0;
	self->client->pers.selspell = newspell;

	if (self->client->pers.skill[self->client->pers.selspell]) {
		info = getPowerupInfo(self->client->pers.selspell);
		gi.cprintf(self, PRINT_HIGH, "%s selected\n", info->name);
	}
}
