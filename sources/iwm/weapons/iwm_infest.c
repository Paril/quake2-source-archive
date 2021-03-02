#include "../g_local.h"
#include "../m_player.h"

static void InfestGrenade_Explode (edict_t *ent);
void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void InfestGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	InfestGrenade_Explode (ent);
}


//*************
//  INFEST HG
//*************

void fire_infestgrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_TRACKER|EF_FLIES;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	if (!ISGL(self))
		grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	else
		grenade->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = InfestGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = InfestGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	
	if (ISGL(self))
		return;

	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		InfestGrenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}
void ThrowInfestGib (edict_t *self, char *gibname, int damage, int type, edict_t *attacker);

void ThrowUpNow(edict_t *hack)
{
	edict_t *self;
	int		n;

	// use some local vector variables to work with
	vec3_t	forward, right;
	vec3_t	mouth_pos, spew_vector;

	self = hack->target_ent;

	if (!self || !self->inuse || self->takedamage == DAMAGE_NO) {
		G_FreeEdict (hack);
		return;
	}

	// set the spew vector, based on the client's view angle
	if (self->client)
		AngleVectors (self->client->v_angle, forward, right, NULL);
	else
		AngleVectors (self->s.angles, forward, right, NULL);

	// Make the spew originate from our mouth
	VectorScale (forward, 24, mouth_pos);
	VectorAdd (mouth_pos, self->s.origin, mouth_pos);
	mouth_pos[2] += self->viewheight;
	
	// Make the spew come forwards out of our mouth.
	VectorScale (forward, 24, spew_vector);


	// cough up some gibs.
	T_Damage(self, hack, hack->owner,/*forward,forward,forward*/vec3_origin, self->s.origin, vec3_origin, random()*25, 0, 0, MOD_INFESTED);

	//WARNING: may have been freed by killing player above. check self.
	if (!hack->inuse)
		return;

	ThrowInfestGib(self,"models/objects/gibs/sm_meat/tris.md2",100,GIB_ORGANIC, hack->owner);

	hack->nextthink = level.time + .5 + random()*2;

	n = (rand() + 1) % 3;
	if (n == 0)
		gi.sound (self, CHAN_VOICE, SoundIndex ("misc/gib1.wav"), 1, ATTN_NORM, 0);
	else if (n == 1)
		gi.sound (self, CHAN_VOICE, SoundIndex ("misc/gib2.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, SoundIndex ("misc/gib3.wav"), 1, ATTN_NORM, 0);

    // also do a spewing sound
	//gi.sound (self, CHAN_VOICE, SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
}

//****************
// INFEST EXPLODE
//****************

void InfestGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t		*sent = NULL;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);


	while ((sent = findradius(sent, ent->s.origin, ent->dmg_radius)) != NULL) {
		if (!sent->takedamage)
			continue;
		if (sent == ent)
			continue;

		if (CanDamage (sent, ent)) {
			edict_t *hack;
			if (deathmatch->value) {
				if (!sent->client && Q_stricmp (sent->classname, "bodyque"))
					continue;
			} else {
				if (!sent->svflags & SVF_MONSTER)
					continue;
			}

			hack = G_Spawn();
			hack->owner = hack->infester = ent->owner;
			hack->target_ent = sent;
			hack->s.effects |= EF_FLIES;
			hack->s.modelindex = ModelIndex ("sprites/null.sp2");
			hack->nextthink = level.time + crandom()*5 + 1;
			hack->think = ThrowUpNow;
			hack->think2 = Follow_Owner_Infest;
			hack->nextthink2 = level.time + .1;
			if (ent->owner->client) {
				char *victimname;
				if (sent->client) {
					victimname = sent->client->pers.netname;
				} else {
					if (sent->svflags & SVF_MONSTER)
					{
						if (sent->name)
							victimname = sent->name;
						else
							victimname = sent->classname;
					}
					else
						victimname = "a dead body";
				}
				if (sent == ent->owner)
					safe_cprintf(ent->owner,PRINT_HIGH,"You infested YOURSELF with your infest grenade!\n");
				else
					safe_cprintf(ent->owner,PRINT_HIGH,"You infested %s with your infest grenade!\n", victimname);
			}
		}
	}


	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WIDOWSPLASH);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);


	G_FreeEdict (ent);
}


//****************
// INFEST AN AREA
//****************
void Follow_Owner_Infest (edict_t *s)
{
	VectorCopy (s->target_ent->s.origin, s->s.origin);
	gi.linkentity(s);

	s->nextthink2 = level.time + .1;
}
void Infest_Area (edict_t *ent)
{
	edict_t *yuck;
	edict_t		*sent = NULL;

	while ((sent = findradius(sent, ent->s.origin, 105)) != NULL)
	{
		if (!sent->takedamage)
			continue;
		if (sent == ent)
			continue;

		if (CanDamage (sent, ent)) {
			edict_t *hack;
			qboolean valid = true;

			if (sent->health < 1)
				continue;

		/*	if (deathmatch->value) {
				if (!sent->client && Q_strncasecmp(sent->classname, "monster_", 8))
					goto cont;
				if (!sent->client && Q_stricmp (sent->classname, "bodyque"))
					continue;
			} else {
				if (!sent->client && Q_strncasecmp(sent->classname, "monster_", 8))
					continue;
			}*/

//cont:
			for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) {
				if (!yuck->inuse)
					continue;
				if (yuck->infester && yuck->target_ent == sent && yuck->owner == ent->owner) {
					valid = false;
					break;
				}
			}

			if (!valid)
				continue;

			if (sent->infest_debounce_time > level.time)
				continue;

			hack = G_Spawn();
			hack->owner = hack->infester = ent->owner;
			hack->target_ent = sent;
			hack->s.effects |= EF_FLIES;
			hack->s.modelindex = ModelIndex ("sprites/null.sp2");
			hack->nextthink = level.time + random()* 5 + 1;
			hack->think = ThrowUpNow;
			hack->think2 = Follow_Owner_Infest;
			hack->nextthink2 = level.time + .1;

			if (ent->owner->client) {
				char *victimname;
				if (sent->client)
					victimname = sent->client->pers.netname;
				else
					victimname = sent->classname;
				if (sent == ent->owner)
					safe_cprintf(ent->owner,PRINT_HIGH,"You were infested by your infests contaminated gibs!\n");
				else
					safe_cprintf(ent->owner,PRINT_HIGH,"%s was infested from contaminated gibs!\n", victimname);
			}
		}
	}

	ent->nextthink = level.time + .3 + random();

}


//****************
//  INFEST MINI
//****************
void ThrowInfestGib (edict_t *self, char *gibname, int damage, int type, edict_t *attacker)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	gib = G_Spawn();

	if (!attacker)
		gi.dprintf ("Infest with no attacker\n");


	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	gib->owner = attacker;
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel (gib, gibname);
	//gib->solid = SOLID_NOT;
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_GIB;
	gib->classname = "infestgib";
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_NO;
	gib->die = gib_die;
	gib->think = Infest_Area;
	gib->nextthink = level.time + random();
	gib->think2 = G_FreeEdict;
	gib->nextthink2 = level.time + 4;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_BOUNCE;
		gib->touch = gib_touch;
		vscale = 1.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		gib->touch = gib_touch;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

	gi.linkentity (gib);
}


void Weapon_InfestGrenade_Launcher (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_infestgrenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_InfestGrenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = SoundIndex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				Weapon_InfestGrenade_Launcher (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			Weapon_InfestGrenade_Launcher (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}
