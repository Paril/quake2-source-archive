#include "../g_local.h"
qboolean Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean DummyPickup (edict_t *ent, edict_t *other)
{
	return false;
}
qboolean Pickup_Star (edict_t *ent, edict_t *other);
void AddSkin (edict_t *self);

void block_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
}

void Bloolooloo (edict_t *self)
{
	self->count++;

	if (self->count == 10)
	{
		self->item->pickup = Pickup_Weapon;
		G_FreeEdict (self->owner);
		self->movetype = MOVETYPE_TOSS;  

		VectorCopy (tv(-15,-15,-15), self->mins);
		VectorCopy (tv(15,15,15), self->maxs);
		self->think = G_FreeEdict;
		self->nextthink = level.time + 10;

		return;
	}

	self->s.origin[2] += 2;

	gi.linkentity(self);
	self->nextthink = level.time + .1;
}

void GrowWeapon (edict_t *self)
{
	gitem_t *items[10] = {FindItem("Shotgun"), FindItem("Super Shotgun"), FindItem("Machinegun"), FindItem("Chaingun"), FindItem("Grenade Launcher"), FindItem("Rocket Launcher"), FindItem("Hyperblaster"), FindItem("Railgun"), FindItem("BFG10k"), FindItem("Grenades")};
	edict_t *tmpitem;

	tmpitem = G_Spawn();

	SpawnItem (tmpitem, items[rand()%10]);

	VectorCopy (self->s.origin, tmpitem->s.origin);
	
	// Temporary transition so it has time to Bloolooloo up from the block.
	tmpitem->spawnflags |= DROPPED_ITEM;
	tmpitem->item->pickup = DummyPickup;
	tmpitem->think = Bloolooloo;
	tmpitem->nextthink = level.time + .1;
	tmpitem->count = 0;
	tmpitem->owner = self;
	//tmpitem->s.modelindex = ModelIndex(tmpitem->item->world_model);
	tmpitem->classname = tmpitem->item->classname;
	gi.setmodel (tmpitem, tmpitem->item->world_model);
	tmpitem->solid = SOLID_TRIGGER;
	tmpitem->touch = Touch_Item;

	gi.linkentity (tmpitem);
}

void Bloolooloo2 (edict_t *self)
{
	self->count++;

	if (self->count == 10)
	{
		self->item->pickup = Pickup_Star;
		G_FreeEdict (self->owner);
		self->movetype = MOVETYPE_BOUNCESTAY;  

		VectorCopy (tv(-15,-15,-15), self->mins);
		VectorCopy (tv(15,15,15), self->maxs);

		self->velocity[0] += 360;
		self->velocity[1] += 360;
		self->velocity[2] += 360;
		self->think2 = AddSkin;
		self->nextthink2 = level.time + .5;
		self->movetype = MOVETYPE_BOUNCESTAY;
		self->s.origin[2] += 48;

		self->think = G_FreeEdict;
		self->nextthink = level.time + 10;

		return;
	}

	self->s.origin[2] += 2;

	gi.linkentity(self);
	self->nextthink = level.time + .1;
}

void GrowStar (edict_t *self)
{
	edict_t *tmpitem;

	tmpitem = G_Spawn();

	tmpitem->spawnflags |= DROPPED_ITEM;
	SpawnItem (tmpitem, FindItem("Starman"));

	VectorCopy (self->s.origin, tmpitem->s.origin);
	
	// Temporary transition so it has time to Bloolooloo up from the block.
	tmpitem->item->pickup = DummyPickup;
	tmpitem->think = Bloolooloo2;
	tmpitem->nextthink = level.time + .1;
	tmpitem->count = 0;
	tmpitem->owner = self;
	//tmpitem->s.modelindex = ModelIndex(tmpitem->item->world_model);
	tmpitem->classname = tmpitem->item->classname;
	gi.setmodel (tmpitem, tmpitem->item->world_model);
	tmpitem->solid = SOLID_TRIGGER;
	tmpitem->touch = Touch_Item;

	gi.linkentity (tmpitem);
}

void Bloolooloo3 (edict_t *self)
{
	self->count++;

	if (self->count == 10)
	{
		vec3_t origin;
		T_RadiusDamage (self, self->owner, 200, NULL, 200, MOD_G_SPLASH);

		VectorMA (self->s.origin, -0.02, self->velocity, origin);
		gi.WriteByte (svc_temp_entity);
		if (self->waterlevel)
		{
			if (self->groundentity)
				gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
			else
				gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		}
		else
		{
			if (self->groundentity)
				gi.WriteByte (TE_GRENADE_EXPLOSION);
			else
				gi.WriteByte (TE_ROCKET_EXPLOSION);
		}
		gi.WritePosition (origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);

		G_FreeEdict (self->tempent);
		G_FreeEdict (self);
		return;
	}

	self->s.origin[2] += 2;

	gi.linkentity(self);
	self->nextthink = level.time + .1;
}

void GrowGrenade (edict_t *self)
{
	edict_t *gren;

	gren = CreateEntity (self->s.origin, 0, MOVETYPE_NONE, 0, SOLID_NOT, EF_GRENADE, 0, 0, NULL, NULL, self->tempent);
	gren->tempent = self;
	gren->think = Bloolooloo3;
	gren->nextthink = level.time + .1;
	gren->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");

	gi.linkentity (gren);
}

void block_start (edict_t *self)
{
	trace_t tr;
	vec3_t star, end;

	// Rotate
	//if (!self->s.effects & EF_ROTATE)
	//	self->s.effects |= EF_ROTATE;
	self->s.angles[1] += 15;
	
	VectorCopy(self->s.origin, star);

	VectorMA(star, 0.2, tv(0, 0, -90), end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SHOT);

	/*gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (star);
	gi.WritePosition (end);
	gi.multicast (star, MULTICAST_PVS);*/

	// Client has to be jumping.
	if (tr.ent && tr.ent->client && tr.ent->client->ucmd.upmove > 0)
	{
		float ran = rand()%9;

		// Hopefully the trace completed and we hit a client.
		// Otherwise we have no one to claim the riches.. or the gibs.

		// Random weapon. This occurs the most often.
		if (ran == 0 || ran == 1 || ran == 2 || ran == 7)
		{
notthisone:

			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			//gi.dprintf ("Random weapon.\n");

			self->think = GrowWeapon;
		}
		// Destroy the block
		else if (ran == 3 || ran == 4)
		{
			vec3_t org;

			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/brk.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/brk.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/brk.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/brk.wav"), 1, ATTN_NORM, 0);
		//	gi.dprintf ("Destroy block.\n");

			org[0] = self->s.origin[0] + crandom() * 2;
			org[1] = self->s.origin[1] + crandom() * 2;
			org[2] = self->s.origin[2] + crandom() * 2;
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1.5, org);
			org[0] = self->s.origin[0] + crandom() * 2;
			org[1] = self->s.origin[1] + crandom() * 2;
			org[2] = self->s.origin[2] + crandom() * 2;
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1.5, org);
			org[0] = self->s.origin[0] + crandom() * 2;
			org[1] = self->s.origin[1] + crandom() * 2;
			org[2] = self->s.origin[2] + crandom() * 2;
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1.5, org);
			org[0] = self->s.origin[0] + crandom() * 2;
			org[1] = self->s.origin[1] + crandom() * 2;
			org[2] = self->s.origin[2] + crandom() * 2;
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1.5, org);

			G_FreeEdict (self);
			return;
		}
		// Sends out a grenade that blows up
		else if (ran == 5 || ran == 6 || ran == 7)
		{
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/bump.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/bump.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/bump.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/bump.wav"), 1, ATTN_NORM, 0);
			//gi.dprintf ("Grenade.\n");

			self->think = GrowGrenade;
		}
		// Very rare: STAR POWER!
		else
		{
			if (random() > 0.8)
				goto notthisone;

			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/block/up.wav"), 1, ATTN_NORM, 0);
			//gi.dprintf ("Star.\n");

			self->think = GrowStar;
		}

	}

	self->nextthink = level.time + .1;
}

void block_think (edict_t *self)
{
	self->count -= 100;

	if (self->count < 0)
	{
		self->think = block_start;
		self->nextthink = level.time + .1;
		return;
	}

	VectorScale (self->movedir, self->count, self->velocity);
	
	self->nextthink = level.time + .1;
	self->think = block_think;
}



void fire_block (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	//vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//VectorClear (rocket->mins);
	//VectorClear (rocket->maxs);
	VectorSet (rocket->mins, -12.97, -12.85, -11.33);
	VectorSet (rocket->maxs, 13.06, 12.94, 10.06);
	rocket->s.modelindex = ModelIndex ("models/proj/block/tris.md2");
	// Paril: We want to be able to jump on our own boxes.
	//rocket->owner = self;
	rocket->tempent = self;
	rocket->touch = block_touch;
	rocket->nextthink = level.time + .1;
	rocket->think = block_think;
	rocket->nextthink2 = level.time + 10;
	rocket->think2 = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->classname = "block";
	rocket->count = speed;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_BlockCannon_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 30 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_block (ent, start, forward, damage, 600, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_BlockCannon (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_BlockCannon_Fire);
}
