#include "../g_local.h"
#include "../m_player.h"
void target_laser_on (edict_t *self);
void target_laser_think (edict_t *self);

void KillPlayerLinks (edict_t *p)
{
	int i;
	edict_t *e;

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (e && e->classname && Q_stricmp(e->classname, "chaingrenade"))
			continue;
		if (e->owner != p)
			continue;

		// Free lasers first.
		if (e->tempent && e->tempent->inuse)
		{
			G_FreeEdict(e->tempent);
			e->tempent = NULL;
		}

		// Now, asplode the grenade
		//BecomeExplosion1 (e);
		G_Spawn_Explosion (TE_EXPLOSION1, e->s.origin, e->s.origin);
		G_FreeEdict (e);
	}

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++)
	{
		if (e && e->classname && Q_stricmp(e->classname, "laser_sh"))
			continue;
		if (e->owner != p)
			continue;
		//if (!e->tempent && !e->tempent->inuse)
		//{
			G_FreeEdict (e);
		//}
	}
}

void TraceForHurtingDamageThingy (edict_t *self)
{
	trace_t tr;
	edict_t *blip = NULL;
	vec3_t end;

	VectorMA(self->s.origin, 0, self->movedir, end);

	tr = gi.trace (self->s.origin, NULL, NULL, self->tempent2->s.origin, NULL, CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if ((tr.ent) && !(tr.ent->flags & FL_IMMUNE_LASER) && tr.ent->takedamage)
	{
		blip = tr.ent;
		T_Damage (tr.ent, self, self->owner, vec3_origin, tr.ent->s.origin, vec3_origin, 25, 0, 0, MOD_CHAINLINK); // FIXME MOD
	}

	//gi.WriteByte (svc_temp_entity);
	//gi.WriteByte (TE_DEBUGTRAIL);
	//gi.WritePosition (self->s.origin);
	//gi.WritePosition (self->tempent2->s.origin);
	//gi.multicast (self->s.origin, MULTICAST_PVS);

	self->nextthink3 = level.time + .1;
}

void pre_target_laser_think2 (edict_t *self)
{
	target_laser_on (self);

	self->think = target_laser_think;
	self->nextthink = level.time + 0.1;
	//self->think2 = G_FreeEdict;
	//self->nextthink2 = level.time + 20;

}

#define COLOR_ORANGE_AND_YELLOW_TYPE_MIX 0xe7f2ea11
#define ORANGE_AND_PALE_YELLOW 0xe7fea657
#define BEIGE_AND_BLUE 0x9a645ef1
#define DARK_TEAL 0x77777777
#define VERY_LIGHT_INVIS_RED 0xeeeeeeee
#define FLASHING_MORE_YELLOW_LESS_LIGHT_RED 0xcbdaedbd
#define RED_GREEN 0xabcdabcd


void LinkChain (edict_t *grenade, edict_t *found, float dist2)
{
//	vec3_t start;
//	vec3_t forward;
	vec3_t ang;
	float dist;
	edict_t *self;
	vec3_t start, end;
	int colors[16] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_YELLOW2, COLOR_PURPLE, 
					  COLOR_BLUE2, COLOR_GREEN2, COLOR_PURPLE2, COLOR_RED2, COLOR_ORANGE, COLOR_MIX,
					  COLOR_INNER1, COLOR_INNER2, COLOR_INNER3, COLOR_INNER4};

	// Get line between both grenades
	VectorSubtract (found->s.origin, grenade->s.origin, ang);
	dist = VectorLength (ang);


	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();

	VectorMA (start, dist, ang, end);
	
//	self->tempvec = end;
	VectorCopy (end, self->tempvec);
	self->tempent2 = found;
	self -> movetype               = MOVETYPE_NONE;
	self -> solid                  = SOLID_NOT;
	self -> s.renderfx             = RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex   = 1;                   // must be non-zero
	self -> s.sound                = SoundIndex ("world/laser.wav");
	self -> classname              = "laser_sh";
	self -> s.frame                = 5;    // beam diameter
	self -> owner                  = grenade->owner;
	self -> s.skinnum              = colors[rand()%16];
	self -> dmg                           = 25;
	self -> think                  = pre_target_laser_think2;
	self -> delay                  = level.time + 0.1;
	self->tempint = dist2;
	

	// Set origin of laser to point of contact with wall
	VectorCopy(grenade->s.origin,self->s.origin); 

	self->tempent = found;
	found->tempent = self;
	grenade->tempent = self;
	
	// convert normal at point of contact to laser angles
	vectoangles(ang, self->s.angles);
	
	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);

	VectorCopy (ang, grenade->s.angles);
	
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	// link to world
	gi.linkentity (self);
	
	// ... but make automatically come on
	self -> nextthink = level.time + 0.1;

	self->think3 = TraceForHurtingDamageThingy;
	self->nextthink3 = level.time + .1;

	//grenade->think2 = G_FreeEdict;
	//grenade->nextthink2 = level.time + 20;
	//found->think2 = G_FreeEdict;
	//found->nextthink2 = level.time + 20;

}

// Paril 2.00: Function called by a command to
// fix laser chains. It will close them in.
// Routines: Find LLG that has not been
// linked to yet, then that laser link
// grenade will find the closest grenade that has not been
// linked and link to it. This MIGHT cause a bit of confusion to
// the game, but we'll see how it works out.

// FIXME sometime.
/*void FixLaserChains (edict_t *ent)
{
	edict_t *thelinks[30];
	edict_t *unlinked;
	int loops, i, j;

	// Go through each link and copy over, just for localness.
	for (loops = 0; loops < 30; loops++)
	{
		thelinks[loops] = ent->client->laser_links[loops];
	}

	// Now, we need to go through each of these links
	// and for each link see if another of his links
	// has linked to this.. <_<

	for (i = 0; i < 30; i++)
	{
		if (thelinks[i] != NULL)
		{
			// This link has not linked with anything yet. Let's link it to
			// the closest one that 
			if (!thelinks[i]->linked_to_a_chain)
			{
				thelinks[i]->needs_to_be_linked = true;
				FindLaserChain (thelinks[i]);
				break;
			}
		}
	}
}*/

void FindLaserChain (edict_t *self)
{
	edict_t *find = NULL;
	float dist[180];
	edict_t *dist_f[180];
	int j = 0;
	vec3_t vec;
	int min = 8192;
	float closest_dist = 0;
	edict_t *closest_ent = NULL;
	int i;

	memset (dist, 0, sizeof(dist));
	memset (dist_f, 0, sizeof(dist_f));

	while (find = findradius(find, self->s.origin, 600))
	{
		if (!find)
			continue;
		if (find->tempfloat != 800)
			continue;
		// Don't link to self.
		if (find == self)
			continue;
		if (!visible(self, find))
			continue;

		VectorSubtract (find->s.origin, self->s.origin, vec);


		dist[j] = VectorLength (vec);
		dist_f[j] = find;
		j++;
	}

	if (j < 1)
		return;

	for (i = 0; i < j; i++)
	{
		if (dist[i] < min)
		{
			closest_dist = dist[i];
			closest_ent = dist_f[i];
			min = closest_dist;
		}
	}

	if (closest_ent)
	{
		// We found a chain, let's link it.
		LinkChain (self, closest_ent, closest_dist);
		//self->linked_to_a_chain = true;
		//closest_ent->linked_to_a_chain = true;
		//closest_ent->linked_with = self;
		//self->linked_with = closest_ent;

		//gi.dprintf ("Chaingrenade found a chain at %s, is %s\n", vtos(closest_ent->s.origin), closest_ent->classname);
	}

	/*for (i = 0; i < 30; i++)
	{
		if (self->owner->client->laser_links[i] != NULL)
			continue;

		// We found one that's not being used.
		if (self->owner->client->laser_links[i] == NULL)
		{
			self->owner->client->laser_links[i];
			break;
		}
	}*/
}

void CG_Stop (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->s.effects &= ~EF_GRENADE;
	self->classname = "chaingrenade";

	self->nextthink = level.time + 0.5;
	self->think = FindLaserChain;
}

void fire_chaingren (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer)
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
	//VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->nextthink = level.time + 0.1;
	grenade->think = CG_Stop;
	grenade->classname = "grenade";
	//grenade->think2 = G_FreeEdict;
	//grenade->nextthink2 = level.time + 20;
	grenade->tempfloat = 800;

	gi.linkentity (grenade);
}

void weapon_linkgrenade_fire (edict_t *ent, qboolean held)
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
	fire_chaingren (ent, start, forward, speed, 0);

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

void Weapon_LinkGrenade (edict_t *ent)
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
			//if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			//{
			//	ent->client->weapon_sound = 0;
			//	weapon_linkgrenade_fire (ent, true);
			//	ent->client->grenade_blew_up = true;
			//}

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
			weapon_linkgrenade_fire (ent, false);
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
