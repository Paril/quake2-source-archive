#include "../g_local.h"
#include "../m_player.h"
void Nuke_Quake (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 0.75, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict (self);
}
void beam_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;
	int j;
	edict_t *temp;
	int ha = 0;

	ignore = self->owner;

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		VectorCopy (self->movedir, last_movedir);
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}

	ignore = self;
	VectorCopy (self->s.origin, start);

	VectorMA (start, 8192, self->movedir, end);

	j = 0;
	tr = gi.trace (start, NULL, NULL, end, self->owner, MASK_SHOT);
	while(j == 0)
	{
		tr = gi.trace (start, NULL, NULL, end, self->owner, MASK_SHOT);

		if (!tr.ent)
			break;
		if (self->s.skinnum == COLOR_GREEN)
			break;
		//if (!tr.ent->client)
		//	break;
		//if (tr.ent->deadflag || tr.ent->health < 1)
		//	break;
		if (self->owner->client->ps.gunframe != 5)
			break;

			if (self->s.frame > 0 && self->s.frame < 6)
					ha = 1;
			if (self->s.frame == 1)
				T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 8, 0, 0, MOD_BEAMCANNON); // Fixme
			if (self->s.frame == 2)
				T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 12, 0, 0, MOD_BEAMCANNON); // Fixme
			if (self->s.frame == 3)
				T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 16, 0, 0, MOD_BEAMCANNON); // Fixme
			if (self->s.frame == 4)
				T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 20, 0, 0, MOD_BEAMCANNON); // Fixme
			if (self->s.frame == 5)
				T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 24, 0, 0, MOD_BEAMCANNON); // Fixme

		
			if (self->s.frame == 6)
			{
				temp = G_Spawn();
				VectorCopy (tr.endpos, temp->s.origin);
				//T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 20, 0, 0, MOD_BLASTER); // Fixme
				G_Spawn_Explosion (TE_EXPLOSION1, tr.endpos, tr.endpos);
				T_RadiusDamage (temp, self->owner, 70, NULL, 70, MOD_BEAMCANNON2); // FIXME
				G_FreeEdict (temp);
				ha = 1;
			}
			if (self->s.frame == 7)
			{
				temp = G_Spawn();
				VectorCopy (tr.endpos, temp->s.origin);
				//T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 20, 0, 0, MOD_BLASTER); // Fixme
				G_Spawn_Explosion (TE_EXPLOSION1, tr.endpos, tr.endpos);
				G_Spawn_Explosion (TE_EXPLOSION1, tr.endpos, tr.endpos);
				T_RadiusDamage (temp, self->owner, 90, NULL, 90, MOD_BEAMCANNON2); // FIXME
				G_FreeEdict (temp);
				ha = 1;
			}
			if (self->s.frame > 7 && self->s.frame < 16)
			{
				temp = G_Spawn();
				VectorCopy (tr.endpos, temp->s.origin);
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_DEBUGTRAIL);
				gi.WritePosition (self->s.origin);
				gi.WritePosition (tr.endpos);
				gi.multicast (self->s.origin, MULTICAST_PVS);

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_NUKEBLAST);
				gi.WritePosition (tr.endpos);
				gi.multicast (tr.endpos, MULTICAST_ALL);

				// QuakeEarth
				if (self->s.frame > 10)
				{
					edict_t *nuke;
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_WIDOWBEAMOUT);
					gi.WriteShort (25555);
					gi.WritePosition (tr.endpos);
					gi.multicast (tr.endpos, MULTICAST_PHS);

					nuke = G_Spawn();
					// become a quake
					nuke->svflags |= SVF_NOCLIENT;
					nuke->noise_index = SoundIndex ("world/rumble.wav");
					nuke->think = Nuke_Quake;
					nuke->speed = 80;
					nuke->timestamp = level.time + 2.5;
					nuke->nextthink = level.time + .1;
					nuke->last_move_time = 0;

					gi.linkentity (nuke);
				}
	
				T_RadiusDamage (temp, self->owner, self->s.frame * 15, NULL, self->s.frame * 15, MOD_BEAMCANNON3); // FIXME
				gi.sound (temp, CHAN_AUTO, SoundIndex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
				gi.sound (temp, CHAN_AUTO, SoundIndex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
				gi.sound (temp, CHAN_AUTO, SoundIndex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
				gi.sound (temp, CHAN_AUTO, SoundIndex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
				G_FreeEdict (temp);
				ha = 1;
			}
			if (self->s.frame == 16 || self->s.frame == 17)
			{
				MakeFreezeExplosion (self->owner, tr.endpos);
				ha = 1;
			}
			else if (self->s.frame > 17)
			{
				MakeNukeExplosionAndCloudBecauseItsAVeryBigWeaponAndDoesALotofDamage (self->owner, tr.endpos, 0);
				self->owner->client->pers.womded = 1;
			}

		j = 1;

	}

	VectorCopy (tr.endpos, self->s.old_origin);

	self->nextthink = level.time + FRAMETIME;
}

void beam_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	beam_think (self);
}

void beam_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void beam_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	if (self->spawnflags & 1)
		beam_off (self);
	else
		beam_on (self);
}


void pre_target_beam_think (edict_t *self)
{

  beam_on (self);

  self->think = beam_think;
	self->think2 = G_FreeEdict;
	self->nextthink2 = level.time + .1;
	self->nextthink = level.time + .1;
}

void Beam_Fire (edict_t *ent, vec3_t g_offset, int diameter)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
////	vec3_t fwd, end;
	edict_t *self;
//	trace_t tr;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();
	
	self -> movetype               = MOVETYPE_NONE;
	self -> solid                  = SOLID_NOT;
	self -> s.renderfx             = RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex   = 1;                   // must be non-zero
	self -> s.sound                = SoundIndex ("world/laser.wav");
	self -> classname              = "laser_yaya";
	self -> s.frame                = diameter;    // beam diameter
	self -> owner                  = ent;
	if (ent->client->ps.gunframe == 4)
		self -> s.skinnum              = COLOR_BLUE;
	else
		self -> s.skinnum              = COLOR_GREEN;
	self -> dmg                           = 25;
	self -> think                  = pre_target_beam_think;
	self -> delay                  = level.time + 30;
	
	// Set orgin of laser to point of contact with wall
	VectorCopy(start,self->s.origin); 
	
	// convert normal at point of contact to laser angles
	vectoangles(forward,self -> s.angles);
	
	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);
	
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	// link to world
	gi.linkentity (self);
	
	// ... but make automatically come on
	self -> nextthink = level.time;

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_BeamCannon_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
//	int			damage;
//	int			kick;
//	vec3_t		v;

	if (is_quad)
	{
//		damage *= 4;
//		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;
	
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (!ent->client->beam_diam)
		ent->client->beam_diam = 1;

	if (ent->client->beam_diam > 20)
		ent->client->beam_diam = 20;

	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 1)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Beam\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}
	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 6)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Explosion\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}
	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 8)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Slug Cannon\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}
	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 11)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Slug Cannon EarthQuake\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}
	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 16)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Freeze Nuke\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}
	if (ent->client->beam_diamcount == 2 && ent->client->beam_diam == 18)
	{
		safe_cprintf (ent, PRINT_HIGH, "BEAM CANNON EFFECT: Nuke\n");
		gi.sound (ent, CHAN_WEAPON, SoundIndex("weapons/RAILGR1A.WAV"), 0.4, ATTN_NORM, 0);
	}

	Beam_Fire (ent, forward, ent->client->beam_diam);
	//gi.dprintf ("%i\n", ent->client->beam_diam);
	
	if (ent->client->ps.gunframe == 4)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1;
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1;
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->beam_diam = 0;
}


void Weapon_BeamCannon (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 18, 56, 61, pause_frames, fire_frames, Weapon_BeamCannon_Fire);

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) && ent->client->ps.gunframe == 4)
	{
		if (!ent->client->beam_diamcount)
			ent->client->beam_diamcount = 1;

			if (ent->client->beam_diam > 16 && ent->client->pers.womded)
			{
				if ( (int)dmflags->value & DF_INFINITE_AMMO )
				{
//					gi.dprintf ("Stop?\n");
					goto cont;
				}
			}

		ent->client->beam_diamcount++;

		if (ent->client->beam_diamcount == 5)
		{
				ent->client->beam_diam++;
				ent->client->beam_diamcount = 1;
		}
cont:
		ent->client->ps.gunframe--;
	}
}
