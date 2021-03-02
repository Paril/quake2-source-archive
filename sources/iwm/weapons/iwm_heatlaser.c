#include "../g_local.h"
#include "../m_player.h"

/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) ;

void laser_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
//		G_Spawn_Sparks (TE_TUNNEL_SPARKS, self->s.origin, vec3_origin, self->s.origin);
		//G_Spawn_Splash(TE_TUNNEL_SPARKS, 6, COLOR_RED, self->s.origin, vec3_origin, self->s.origin);

//		gi.sound (self, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);
	}

	G_FreeEdict (self);
}	

void heatlaser_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;
	int j, water;

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
	if (self->owner->waterlevel < 2)
	{
		VectorMA (start, 8192, self->movedir, end);
		water = 0;
	}
	else
	{
		VectorMA (start, 0, self->movedir, end);
		water = 1;
	}

	j = 0;
	tr = gi.trace (start, NULL, NULL, end, self->owner, MASK_SHOT);
	while(j == 0)
	{
		tr = gi.trace (start, NULL, NULL, end, self->owner, MASK_SHOT);

		if (self->owner->waterlevel > 1)
		{
			VectorCopy (self->owner->s.origin, end);
			//end = ent->owner->s.origin;
		}

		if (tr.contents & CONTENTS_WATER || self->owner->waterlevel > 1)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_HEATBEAM_SPARKS);
			//			gi.WriteByte (50);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			//			gi.WriteByte (8);
			//			gi.WriteShort (60);
			gi.multicast (tr.endpos, MULTICAST_PVS);
			water = 1;
			j = 1;
			break;
		}

		if (!tr.ent)
			break;
		if (!tr.ent->takedamage)
			break;
		if (tr.ent->deadflag || tr.ent->health < 1)
			break;

		if (tr.ent->burning_framenum < level.framenum)
			tr.ent->burning_framenum = level.framenum + 5;
		else
			tr.ent->burning_framenum += 5;

		tr.ent->last_heater = self->owner;

		tr.ent->burner_entity = self->owner;
		//ignore = tr.ent;
		//VectorCopy (tr.endpos, start);	
		j = 1;
	}

	VectorCopy (tr.endpos, self->s.old_origin);

	if (!water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_HEATBEAM_STEAM);
		//			gi.WriteByte (50);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		//			gi.WriteByte (8);
		//			gi.WriteShort (60);
		gi.multicast (tr.endpos, MULTICAST_PVS);
	}

	self->nextthink = level.time + FRAMETIME;
}

void heatlaser_on (edict_t *self)
{
	if (!self->activator)
		self->activator = self;
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	heatlaser_think (self);
}

void heatlaser_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

void heatlaser_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->activator = activator;
	if (self->spawnflags & 1)
		heatlaser_off (self);
	else
		heatlaser_on (self);
}


void pre_target_laser_think (edict_t *self)
{

  heatlaser_on (self);

  self->think = heatlaser_think;
	self->think2 = G_FreeEdict;
	self->nextthink2 = level.time + .1;
}

// Paril's Found Colors!
#define COLOR_ORANGE_AND_YELLOW_TYPE_MIX 0xe7f2ea11
#define ORANGE_AND_PALE_YELLOW 0xe7fea657
#define BEIGE_AND_BLUE 0x9a645ef1
#define DARK_TEAL 0x77777777
#define VERY_LIGHT_INVIS_RED 0xeeeeeeee
#define FLASHING_MORE_YELLOW_LESS_LIGHT_RED 0xcbdaedbd
#define RED_GREEN 0xabcdabcd


void HeatLaser_Fire (edict_t *ent, vec3_t g_offset)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset, end;
//	vec3_t fwd, end;
	edict_t *self;
	trace_t tr;

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
	self -> s.frame                = 5;    // beam diameter
	self -> owner                  = ent;
	self -> s.skinnum              = 0xe7f2ea11;
	self -> dmg                           = 25;
	self -> think                  = pre_target_laser_think;
	self -> delay                  = level.time + 30;
	
	// Set orgin of laser to point of contact with wall
	VectorCopy(start,self->s.origin); 
	
	// convert normal at point of contact to laser angles
	VectorMA (start, 8192, forward, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	vectoangles(forward,self -> s.angles);
	//VectorCopy(forward,self -> s.angles); 

	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);
	
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	// link to world
	gi.linkentity (self);
	
	// ... but make automatically come on
	self -> nextthink = level.time + 0.1;

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

}

void Weapon_HeatLaser_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = SoundIndex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = 0;
			offset[1] = 0;
			offset[2] = 0;

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			HeatLaser_Fire (ent, offset);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HeatLaser (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HeatLaser_Fire);
}
