#include "g_local.h"
void Remove_Player_Flames (edict_t *ent);
void HealPlayer(edict_t *ent);

void VectorRotate(vec3_t in, vec3_t angles, vec3_t out) 
{
	float	cv, sv, angle, tv;

	VectorCopy(in, out);

	angle = (-angles[PITCH]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[YAW]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[1] * sv);
	out[1] = (out[1] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[ROLL]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[1] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[1] * sv);
	out[1] = tv;
}

void VectorUnrotate(vec3_t in, vec3_t angles, vec3_t out) {
	float	cv, sv, angle, tv;

	VectorCopy(in, out);

	angle = (-angles[ROLL]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[1] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[1] * sv);
	out[1] = tv;

	angle = (-angles[YAW]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[1] * sv);
	out[1] = (out[1] * cv) + (out[0] * sv);
	out[0] = tv;

	angle = (angles[PITCH]) * M_PI / 180;
	cv = cos(angle);
	sv = sin(angle);
	tv = (out[0] * cv) - (out[2] * sv);
	out[2] = (out[2] * cv) + (out[0] * sv);
	out[0] = tv;
}

void stuck_prethink (edict_t *self)
{
	vec3_t	temp, new;
	edict_t *other;

	other = self->goalentity;

	if (!other->inuse) {
	}

	VectorRotate(self->pos1, other->s.angles, temp);
	VectorRotate(self->pos2, other->s.angles, new);

	VectorAdd(other->s.origin, temp, self->s.origin);
	VectorSubtract(new, temp, new);
	vectoangles(new, self->s.angles);
}

void Calc_StuckOffset(edict_t *self, edict_t *other) 
{
	vec3_t	forward;

	VectorSubtract(self->s.origin, other->s.origin, forward);
	VectorUnrotate(forward, other->s.angles, self->pos1);

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 64, forward, forward);
	VectorSubtract(forward, other->s.origin, forward);
	VectorUnrotate(forward, other->s.angles, self->pos2);
}

void stick(edict_t *projectile, edict_t *object) 
{
	projectile->solid = SOLID_NOT;
	projectile->movetype = MOVETYPE_FLY;
	VectorClear(projectile->velocity);
	VectorClear(projectile->avelocity);

	if (object != g_edicts) {
		Calc_StuckOffset(projectile, object);
		projectile->goalentity = object;
		projectile->prethink = stuck_prethink;
	} else
		projectile->prethink = NULL;
}

/*
=================
dart_prethink

This is a support routine for keeping an
object velocity-aligned, for I.E. arrows.
=================
*/
void dart_prethink (edict_t *ent) 
{
	vec3_t move;
	vectoangles(ent->velocity, move);
	VectorSubtract(move, ent->s.angles, move);
	move[0] = fmod((move[0] + 180), 360) - 180;
	move[1] = fmod((move[1] + 180), 360) - 180;
	move[2] = fmod((move[2] + 180), 360) - 180;
	VectorScale(move, 1/FRAMETIME, ent->avelocity);
}

/*
======================
Infected Dart Launcher
======================
*/
void infecteddart_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (!other->client)	//only infect players
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
		if (!Q_stricmp("SentryGun", other->classname))
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BLASTER);
			gi.WritePosition (self->s.origin);
			if (!plane)
				gi.WriteDir (vec3_origin);
			else
				gi.WriteDir (plane->normal);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 2, DAMAGE_ENERGY, self->mod);
		}
		//Heal person if you hit someone on the same team 
		else if (other->wf_team == self->wf_team)
		{
			if (other->disease)
			{
				safe_cprintf(other, PRINT_HIGH, "You've been cured!");
			}

			gi.sound (other, CHAN_VOICE, gi.soundindex ("items/pkup.wav"), 1, ATTN_NORM, 0);

			HealPlayer(other);

			if (other->client)
			{
				other->client->blindBase = 0;	//stop blindness
				other->client->blindTime = 0;
			}
			other->DrunkTime=level.time - 1;	//stop concusion

			if (other->health < other->max_health) 
			{
				other->health += 25;
				if (other->health > other->max_health)
					other->health = other->max_health;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_EXPLOSION);
				gi.WritePosition (other->s.origin);
				gi.multicast (other->s.origin, MULTICAST_PVS);
				if (self->mod == MOD_BIOSENTRY)
					gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/biosentry/biofriendlyhit.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(other, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), 1, ATTN_NORM, 0);
			}
		}
		else
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 2, DAMAGE_ENERGY, self->mod);
			infect_person(other,self->owner);
			if (self->mod == MOD_BIOSENTRY)
				gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/biosentry/bioenemyhit.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (other, CHAN_VOICE, gi.soundindex ("darthit.wav"), 1, ATTN_NORM, 0);
		}
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
	}

	self->think = G_FreeEdict;
	self->nextthink = level.time + 3 + 8 * crandom();

	stick(self, other);
}

void fire_infecteddart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int mod)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->mod = mod;
	bolt->wf_team = self->wf_team;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->velocity[2] += 90;
	bolt->movetype = MOVETYPE_TOSS; //JR 1/4/98 changed so it bounces
	bolt->clipmask = MASK_SHOT;
	bolt->prethink = dart_prethink; // Keeps the arrow aligned, so it arcs through the air nicely.
	bolt->gravity = 0.2;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorSet (bolt->mins,-1,-1,-2.5);
	VectorSet (bolt->maxs,1,1,4);
	bolt->s.modelindex = gi.modelindex ("models/dart/tris.md2");
	bolt->s.skinnum = 0;
	bolt->owner = self;
	bolt->touch = infecteddart_touch;
	bolt->nextthink = level.time + 3;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "infected dart"; 
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void weapon_infecteddartlauncher_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int		speed;

	damage = wf_game.weapon_damage[WEAPON_INFECTEDDART];

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	//start[0]=ent->s.origin[0] + forward[0]*1+right[0]*6;
	//start[1]=ent->s.origin[1] + forward[1]*1+right[1]*6;
	//start[2]=ent->s.origin[2] + forward[2]*1+right[2]*6+ent->viewheight-8;
	/* new stuff to fix left handedness */
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	/* new stuff to fix left handedness */

	speed = wf_game.weapon_speed[WEAPON_INFECTEDDART];
	fire_infecteddart (ent, start, forward, damage, speed, EF_GIB, MOD_INFECTEDDART);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}


void Weapon_InfectedDartLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64 ,pause_frames, fire_frames, weapon_infecteddartlauncher_fire);
}
