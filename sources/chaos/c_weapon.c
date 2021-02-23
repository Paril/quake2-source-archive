#include "g_local.h"
#include "c_base.h"
#include "c_weapon.h"
#include "m_player.h"
#include "c_botai.h"

#define GRENADE_TIMER				3.0
#define GRENADE_MINSPEED			400
#define GRENADE_MAXSPEED			800

void FakeDeath(edict_t *self);
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
int Valid_Target( edict_t *ent, edict_t *blip );

/*
 * Chainsaw
 */
void weapon_chainsaw_fire (edict_t *ent)
{
	int			damage;
	vec3_t		start, end, forward, bloodvec, mins = {-3, -3, -3},maxs = {3, 3, 3};
	trace_t		tr;

	damage = 10 + (int)(random() * 3.0);

	if (is_quad)
		damage *= 4;
	
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);	
	VectorNormalize(forward);

	VectorCopy(ent->s.origin, start);
	start[2] += 18;

	VectorMA(start, 80, forward, end);
	end[2]+=18;

	tr = gi.trace(start, mins, maxs, end, ent, MASK_PLAYERSOLID);

	if ((tr.fraction != 1) || tr.startsolid)
	{
		if (tr.ent && tr.ent->takedamage)
		{
			VectorNegate(forward, bloodvec);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BLOOD);
			gi.WritePosition (end);
			gi.WriteDir (bloodvec);
			gi.multicast (end, MULTICAST_PVS);

			VectorClear(tr.ent->velocity);
			T_Damage (tr.ent, ent, ent, tr.ent->velocity, tr.ent->s.origin, tr.ent->velocity, damage, 30, DAMAGE_ENERGY, MOD_CHAINSAW);
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SHOTGUN);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
	}
}

void Weapon_Chainsaw (edict_t *ent)
{
	// 0 -11 :  putup:  Einschaltsequenz
	// 12-19  cuton:  Hochfahren bzw vorstrecken zum Angriff
	// 20-27   cut   :  vorgestreckter Angriff als beständiger Loop...aber beim stoppen immer noch bis fram 19 laufen lassen!
	// 28-35    cutoff :  Säge zurück in Trageposition
	// 36-55   idle  : rumdragen halt  pött-pött-pött  :)
	// 56-60   putdown  : runternehmen


	if (ent->client->weaponstate == WEAPON_ENDFIRE)
	{
		if (ent->client->ps.gunframe == 35) // FRAME_ENDFIRE_LAST
		{
			ent->client->weapon_sound = gi.soundindex("weapons/chainsw/chainidl.wav");
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 36;//FRAME_IDLE_FIRST;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == 60) // FRAME_DEACTIVATE_LAST
		{
			ent->client->weapon_sound = 0;
			ChangeWeapon (ent);
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == 0) // FRAME_ACTIVATE_FIRST
		{
			ent->client->weapon_sound = 0;
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsw/chainstr.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->client->ps.gunframe == 11) // FRAME_ACTIVATE_LAST
		{
			ent->client->weapon_sound = gi.soundindex("weapons/chainsw/chainidl.wav");
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 36;//FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsw/chainend.wav"), 1, ATTN_NORM, 0);
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 56; //FRAME_DEACTIVATE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ENDFIRE)
	{
		if (ent->client->ps.gunframe == 35) // FRAME_ENDFIRE_LAST
		{
			ent->client->weapon_sound = gi.soundindex("weapons/chainsw/chainidl.wav");
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 36;//FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_STARTFIRE)
	{
		if (ent->client->ps.gunframe == 19) // FRAME_STARTFIRE_LAST
		{
			ent->client->weapon_sound = gi.soundindex("weapons/chainsw/chaincu2.wav");
			
			//PLAYERNOISE
			PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);

			ent->client->weaponstate = WEAPON_FIRING;
			ent->client->ps.gunframe = 20;//FRAME_FIRE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->weapon_sound = 0;
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsw/chaincu1.wav"), 1, ATTN_NORM, 0);
				ent->client->ps.gunframe = 12;//FRAME_STARTFIRE_FIRST;
				ent->client->weaponstate = WEAPON_STARTFIRE;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					ent->client->weapon_sound = 0;
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == 55)//FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = 36;//FRAME_IDLE_FIRST;
				return;
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			weapon_chainsaw_fire (ent);

			if (ent->client->ps.gunframe == 27)//FRAME_FIRE_LAST)
			{
				ent->client->ps.gunframe = 20;//FRAME_FIRE_FIRST;
				return;
			}
			ent->client->ps.gunframe++;
		}
		else
		{
			ent->client->weapon_sound = 0;
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/chainsw/chaincu3.wav"), 1, ATTN_NORM, 0);
			ent->client->weaponstate = WEAPON_ENDFIRE;
			ent->client->ps.gunframe = 28;//FRAME_ENDFIRE_FIRST;
		}
	}
}

//----------------------------------------------------------------------------------------------
// Automatic Turrets
//----------------------------------------------------------------------------------------------

void Turret_Rocket_Think (edict_t *ent)
{
	ent->velocity[0] += crandom() * 40;
	ent->velocity[1] += crandom() * 40;
	ent->velocity[2] += crandom() * 40;

	ent->nextthink = level.time + .2;
}

void Turret_Explode (edict_t *ent)
{
	int			i;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	for(i = 0; i < 2;  i++)
	{
		edict_t *chunk;

		chunk = G_Spawn();
		VectorCopy (ent->s.origin, chunk->s.origin);
		gi.setmodel (chunk, "models/objects/debris2/tris.md2");
		VectorCopy(tv(crandom() * 500,crandom() * 500, random() * 500), chunk->velocity);
		chunk->movetype = MOVETYPE_BOUNCE;
		chunk->solid = SOLID_NOT;
		chunk->avelocity[0] = crandom()*300;
		chunk->avelocity[1] = crandom()*300;
		chunk->avelocity[2] = crandom()*300;
		chunk->think = G_FreeEdict;
		chunk->nextthink = level.time + 1 + random() * 2;
		chunk->classname = "debris";
		chunk->takedamage = DAMAGE_NO;

		chunk->s.effects |= EF_GRENADE;

		gi.linkentity (chunk);
	}

	if (ent->other)
		G_FreeEdict (ent->other);

	//MATTHIAS
	for (i=0; i<numturrets; i++)
		if (turrets[i] == ent)
			break;

	i++;
	for (; i<numturrets; i++)
		turrets[i-1] = turrets[i];

	turrets[i-1] = NULL;
	//MATTHIAS
	
	numturrets--;

	// FWP Don't free the ent till we're done with it

	G_FreeEdict (ent);


}

void Turret_Die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (attacker->client)
	{
		if (attacker != ent->owner)
		{
			attacker->client->resp.score += 1;
			bprintf2 (PRINT_MEDIUM, "%s receives an extra frag for killing %s's turret.\n", attacker->client->pers.netname, ent->owner->client->pers.netname);
		}
		else
			bprintf2 (PRINT_HIGH, "%s killed his own turret!\n", attacker->client->pers.netname);
	}

	Turret_Explode (ent);
}

void Turret_Pain(edict_t *ent, edict_t *other, float kickback, int damage)
{
	vec3_t	blipdir, blipangles;

	if (ent != other)
	{
		VectorSubtract(other->s.origin, ent->s.origin, blipdir);
		vectoangles(blipdir, blipangles);

		if ((blipangles[PITCH] > -70 && blipangles[PITCH] <= 0)
			|| (blipangles[PITCH] > -360 && blipangles[PITCH] < -310))
		{
			ent->enemy    = other;
		}
	}
}

void fire_turretrocket (edict_t *self, vec3_t start, vec3_t dir, float speed)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_PLAYERSOLID;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_GRENADE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/t_rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 0.1;
	rocket->think = Turret_Rocket_Think;
	rocket->dmg = 30;
	rocket->radius_dmg = 40;
	rocket->dmg_radius = 160;
	rocket->s.sound = gi.soundindex ("weapons/turret/rockfly.wav");
	rocket->classname = "turret_rocket";

	gi.linkentity (rocket);
}

void fire_lead3 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	edict_t		*oldowner;
			
	
	oldowner = self->owner;
	self->owner = NULL;


	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, oldowner, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}

	self->owner = oldowner;
}

void LaserTurret_Think (edict_t *ent)
{
	edict_t			*blip = NULL;
	vec3_t			blipdir, blipangles,forigin, forward, right, side;

	ent->nextthink = level.time + 0.1;
	ent->touch = NULL;

	if (!ent->groundentity)
	{
		ent->nextthink = 1;
		ent->think = Turret_Explode;
	}

	if (ent->count == -1)	//activate state
	{
		edict_t	*base;

		VectorClear(ent->velocity);
		VectorClear(ent->avelocity);

		base = G_Spawn();
		VectorCopy (ent->s.origin, base->s.origin);
		base->s.origin[2] -= 22;
		vectoangles (ent->s.angles, base->s.angles);
		base->movetype = MOVETYPE_NONE;
		base->clipmask = MASK_SHOT;
		base->solid = SOLID_BBOX;
		VectorClear (base->mins);
		VectorClear (base->maxs);
		VectorSet (base->mins, -2, -2, 0);
		VectorSet (base->maxs, 2, 2, 22);
		base->s.modelindex = gi.modelindex ("models/objects/t_base/tris.md2");
		base->classname = "turret_base";
		base->mass = 1;
		base->health = 600;
		base->die = Turret_Die;
		base->flags |= FL_NO_KNOCKBACK;
		base->takedamage = DAMAGE_YES;
		base->other = ent;
		base->owner = ent->owner;

		gi.linkentity (base);

		ent->other = base;
		VectorSet (ent->mins, -16, -16, -22);
		VectorSet (ent->maxs, 16, 16, 15);
		gi.setmodel (ent, "models/objects/lturret/tris.md2");

		if (defence_turret_ammo->value > 0)
			ent->count = defence_turret_ammo->value;
		else
			ent->count = 1000;

		ent->delay = level.time;	//for fire freq
		ent->s.sound = gi.soundindex ("weapons/turret/online.wav");
	}
	else
	{
		//search enemy
		while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
		{
			if (blip->client 
				|| Q_stricmp(blip->classname, "proxmine") == 0
				|| Q_stricmp(blip->classname, "laser_turret") == 0
				|| Q_stricmp(blip->classname, "rocket_turret") == 0)
			{
				if(!visible(ent,blip))
					continue;
				if(!infront(ent,blip))
					continue;
				if(blip->health <= 0)
					continue;
				if(blip == ent->owner)
					continue;
				if (blip->owner == ent->owner)
					continue;
				if (TeamMembers(ent->owner, blip))
					continue;
				if (TeamMembers(ent->owner, blip->owner))
					continue;
				if (blip->client && blip->client->invisible_framenum > level.framenum)	//invisible
					continue;

				VectorSubtract(tv(blip->s.origin[0],blip->s.origin[1], blip->s.origin[2] - 8), ent->s.origin, blipdir);
				vectoangles(blipdir, blipangles);

				if ((blipangles[PITCH] > -70 && blipangles[PITCH] <= 0)
					|| (blipangles[PITCH] > -360 && blipangles[PITCH] < -310))
				{
					ent->enemy = blip;
					break;
				}
			}
		}

		if(ent->enemy && ent->enemy->health < -40)
			ent->enemy = NULL;

		//shot at enemy
		if (ent->enemy && (visible(ent, ent->enemy)))
		{
			VectorSubtract(ent->enemy->s.origin, ent->s.origin, blipdir);
			vectoangles(blipdir, blipangles);

			ent->s.angles[YAW] = blipangles[YAW];
			ent->s.angles[PITCH] = blipangles[PITCH];

			VectorCopy(ent->s.origin, forigin);

			AngleVectors (ent->s.angles, forward, right, NULL);

			VectorScale (right, 9, side);
			VectorAdd (forigin, side, forigin);
			forigin[2] += 7;
			VectorSubtract(ent->enemy->s.origin,forigin, blipdir);
			blipdir[0] += crandom() * 20;
			blipdir[1] += crandom() * 20;
			blipdir[2] += crandom() * 10;
			if (Q_stricmp(ent->enemy->classname, "bot") == 0)
				fire_lead3 (ent, forigin, blipdir, 2, 0, TE_GUNSHOT, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_TURRET);
			else
				fire_lead3 (ent, forigin, blipdir, 3, 3, TE_GUNSHOT, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_TURRET);
			
			VectorScale (right, -18, side);
			VectorAdd (forigin, side, forigin);
			VectorSubtract(ent->enemy->s.origin,forigin, blipdir);
			if (Q_stricmp(ent->enemy->classname, "bot") == 0)
				fire_lead3 (ent, forigin, blipdir, 2, 0, TE_GUNSHOT, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_TURRET);
			else
				fire_lead3 (ent, forigin, blipdir, 3, 3, TE_GUNSHOT, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_TURRET);
			
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte ((MZ_CHAINGUN1));
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			ent->count -= 2;
			if(ent->count <= 0)
			{
				ent->nextthink = 1;
				ent->think = Turret_Explode;
			}

			if (!(blipangles[PITCH] > -70 && blipangles[PITCH] <= 0)
				&& !(blipangles[PITCH] > -360 && blipangles[PITCH] < -310))
			{
				ent->enemy = 0;
			}
		}
		else	//rotate
		{
			ent->enemy = NULL;
			ent->s.angles[YAW] += 8;
			ent->s.angles[PITCH] = 0;
		}
	}
}

void RocketTurret_Think (edict_t *ent)
{
	edict_t			*blip = NULL;
	vec3_t			blipdir, blipangles,forigin, forward, right, side;

	ent->nextthink = level.time + 0.1;
	ent->touch = NULL;

	if (!ent->groundentity)
	{
		ent->nextthink = 1;
		ent->think = Turret_Explode;
	}

	if (ent->count == -1)	//activate state
	{
		edict_t	*base;

		VectorClear(ent->velocity);
		VectorClear(ent->avelocity);

		base = G_Spawn();
		VectorCopy (ent->s.origin, base->s.origin);
		base->s.origin[2] -= 22;
		vectoangles (ent->s.angles, base->s.angles);
		base->movetype = MOVETYPE_NONE;
		base->clipmask = MASK_SHOT;
		base->solid = SOLID_BBOX;
		VectorSet (base->mins, -2, -2, 0);
		VectorSet (base->maxs, 2, 2, 22);
		base->s.modelindex = gi.modelindex ("models/objects/t_base/tris.md2");
		base->classname = "turret_base";
		base->mass = 1;
		base->health = 600;
		base->die = Turret_Die;
		base->flags |= FL_NO_KNOCKBACK;
		base->takedamage = DAMAGE_YES;
		base->other = ent;
		base->owner = ent->owner;

		gi.linkentity (base);

		ent->other = base;
		VectorSet (ent->mins, -16, -16, -22);
		VectorSet (ent->maxs, 16, 16, 15);
		gi.setmodel (ent, "models/objects/rturret/tris.md2");

		if (rocket_turret_ammo->value > 0)
			ent->count = rocket_turret_ammo->value;
		else
			ent->count = 90;

		ent->delay = level.time;	//for fire freq
		ent->s.sound = gi.soundindex ("weapons/turret/online.wav");
	}
	else
	{
		//search enemy
		while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
		{
			if (blip->client 
				|| Q_stricmp(blip->classname, "proxymine") == 0
				|| Q_stricmp(blip->classname, "laser_turret") == 0
				|| Q_stricmp(blip->classname, "rocket_turret") == 0)
			{
				if(!visible(ent,blip))
					continue;
				if(!infront(ent,blip))
					continue;
				if(blip->health <= 0)
					continue;
				if(blip == ent->owner)
					continue;
				if (blip->owner == ent->owner)
					continue;
				if (TeamMembers(ent->owner, blip))
					continue;
				if (TeamMembers(ent->owner, blip->owner))
					continue;
				if (blip->client && blip->client->invisible_framenum > level.framenum)	//invisible
					continue;

				VectorSubtract(tv(blip->s.origin[0],blip->s.origin[1], blip->s.origin[2] - 10) , ent->s.origin, blipdir);
				vectoangles(blipdir, blipangles);

				if ((blipangles[PITCH] > -70 && blipangles[PITCH] <= 0)
					|| (blipangles[PITCH] > -360 && blipangles[PITCH] < -310))
				{
					ent->enemy = blip;
					break;
				}
			}
		}

		if(ent->enemy && ent->enemy->health < -40)
			ent->enemy = NULL;

		//shot at enemy
		if (ent->enemy && (visible(ent, ent->enemy)))
		{
  			if (level.time > ent->delay)
			{
				VectorSubtract(ent->enemy->s.origin, ent->s.origin, blipdir);
				vectoangles(blipdir, blipangles);

				
				ent->s.angles[YAW] = blipangles[YAW];
				ent->s.angles[PITCH] = blipangles[PITCH];

				AngleVectors (ent->s.angles, forward, right, NULL);

				VectorCopy(ent->s.origin, forigin);
				forigin[2] += 11;
				fire_turretrocket (ent, forigin, forward, 1100);

				VectorCopy(ent->s.origin, forigin);
				forigin[2] += 9;
				VectorScale (right, -2, side);
				VectorAdd (forigin, side, forigin);
				fire_turretrocket (ent, forigin, forward, 1000);

				VectorCopy(ent->s.origin, forigin);
				forigin[2] += 9;
				VectorScale (right, 2, side);
				VectorAdd (forigin, side, forigin);
				fire_turretrocket (ent, forigin, forward, 900);

				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/turret/rockshot.wav"), 1, ATTN_NORM, 0);

				ent->delay = level.time + 1;
				ent->count -= 3;
				if(ent->count == 0)
				{
					ent->nextthink = 1;
					ent->think = Turret_Explode;
				}

				if (!(blipangles[PITCH] > -70 && blipangles[PITCH] < 0)
					&& !(blipangles[PITCH] > -360 && blipangles[PITCH] < -332))
				{
					ent->enemy = 0;
				}
			}
		}
		else	//rotate
		{
			ent->enemy = NULL;
			ent->s.angles[YAW] += 8;
			ent->s.angles[PITCH] = 0;
		}
	}
}

void Turret_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!other->takedamage)
	{
		VectorClear(ent->velocity);
		ent->movetype = MOVETYPE_NONE;
		ent->count	= -1;	//state
		ent->nextthink = level.time + 1;
		if (Q_stricmp(ent->classname, "laser_turret") == 0)
			ent->think = LaserTurret_Think;
		else if (Q_stricmp(ent->classname, "rocket_turret") == 0)
			ent->think = RocketTurret_Think;
		ent->touch = NULL;
	}
}

void fire_lturret (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*turret;
    
	turret = G_Spawn();
	VectorCopy (start, turret->s.origin);
	VectorCopy (dir, turret->movedir);
	vectoangles (dir, turret->s.angles);
	VectorScale (dir, speed, turret->velocity);
	turret->movetype = MOVETYPE_STEP;
	turret->clipmask = MASK_PLAYERSOLID;
	turret->solid = SOLID_BBOX;
	VectorSet (turret->mins, -19, -19, -22);
	VectorSet (turret->maxs, 19, 19, 15);
	turret->s.modelindex = gi.modelindex ("models/objects/ltrthrow/tris.md2");
	turret->touch = Turret_Touch;
	turret->nextthink = level.time + 30;
	turret->think = G_FreeEdict;
	turret->classname = "laser_turret";
	turret->mass = 1;
	turret->health = 500;
	turret->die = Turret_Die;
	turret->pain = Turret_Pain;
	turret->flags |= FL_NO_KNOCKBACK;
	turret->takedamage = DAMAGE_YES;
	turret->random = 0; //used for left/right fire
	turret->owner = self;

	gi.linkentity (turret);

	turrets[numturrets] = turret;
	numturrets++;
}

void fire_rturret (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*turret;

	turret = G_Spawn();
	VectorCopy (start, turret->s.origin);
	VectorCopy (dir, turret->movedir);
	vectoangles (dir, turret->s.angles);
	VectorScale (dir, speed, turret->velocity);
	turret->movetype = MOVETYPE_STEP;
	turret->clipmask = MASK_SHOT;
	turret->solid = SOLID_BBOX;
	VectorSet (turret->mins, -19, -19, -22);
	VectorSet (turret->maxs, 19, 19, 15);
	turret->s.modelindex = gi.modelindex ("models/objects/rtrthrow/tris.md2");
	turret->touch = Turret_Touch;
	turret->nextthink = level.time + 30;
	turret->think = G_FreeEdict;
	turret->classname = "rocket_turret";
	turret->mass = 1;
	turret->health = 500;
	turret->die = Turret_Die;
	turret->pain = Turret_Pain;
	turret->flags |= FL_NO_KNOCKBACK;
	turret->takedamage = DAMAGE_YES;
	turret->owner = self;

	gi.linkentity (turret);

	turrets[numturrets] = turret;
	numturrets++;
}

void weapon_lturret_fire (edict_t *ent)
{
	vec3_t	offset, forward, right, start, endpos;
	trace_t	tr;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	

	// we try to throw a turret
	endpos[0] = start[0] + forward[0] * 40;
	endpos[1] = start[1] + forward[1] * 40;
	endpos[2] = start[2] + forward[2] * 40;  

	tr = gi.trace (ent->s.origin, NULL, NULL, endpos, ent, MASK_SOLID);

	if (tr.startsolid || tr.fraction != 1.0)
	{
		cprintf2(ent, PRINT_HIGH, "To close to a wall!\n");
		return;
	}
	else if (numturrets >= 3)
	{
		cprintf2(ent, PRINT_HIGH, "Only 3 Turrets can be active at the same time...try later!\n");
		return;
	}

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/turret/throw.wav"), 1, ATTN_IDLE, 0);
	fire_lturret (ent, start, forward, 300);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	NoAmmoWeaponChange (ent);
}

void weapon_rturret_fire (edict_t *ent)
{
	vec3_t	offset, forward, right, start, endpos;
	trace_t	tr;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// we try to throw a turret
	endpos[0] = start[0] + forward[0] * 40;
	endpos[1] = start[1] + forward[1] * 40;
	endpos[2] = start[2] + forward[2] * 40;  

	tr = gi.trace (ent->s.origin, NULL, NULL, endpos, ent, MASK_SOLID);
	
	if (tr.startsolid || tr.fraction != 1.0)
	{
		cprintf2(ent, PRINT_HIGH, "To close to a wall!\n");
		return;
	}
	else if (numturrets >= 3)
	{
		cprintf2(ent, PRINT_HIGH, "Only 3 Turrets can be active at the same time...try later!\n");
		return;
	}

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/turret/throw.wav"), 1, ATTN_IDLE, 0);
	fire_rturret (ent, start, forward, 300);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	NoAmmoWeaponChange (ent);
}

void Weapon_LaserTurret (edict_t *ent)
{
	it_lturret = FindItem("automatic defence turret");	//bugfix

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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
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
			weapon_lturret_fire (ent);
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
	it_lturret = FindItem("automatic defence turret");	//bugfix
}

void Weapon_RocketTurret (edict_t *ent)
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
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
			weapon_rturret_fire (ent);
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

//----------------------------------------------------------------------------------------------
// Bastard Sword
//----------------------------------------------------------------------------------------------

void weapon_sword_fire (edict_t *ent)
{
	int			damage;
	edict_t		*blip = NULL;
	vec3_t		forward, start, end, mins = {-3, -3, -3},maxs = {3, 3, 3};
	trace_t		tr;

	damage = 60 + (int)(random() * 60);

	if (is_quad)
		damage *= 4;

	// DAMAGE
	while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
	{
		if (blip->takedamage)
		{
			if (blip == ent)
				continue;
			if (!visible(ent, blip))
				continue;
			if (!infront(ent, blip))
				continue;

			if (blip->client && blip->client->pers.weapon == it_sword && infront (blip, ent))
			{
				if (blip->client->weaponstate != WEAPON_FIRING)
				{
					// 90% blocked
					if (random() < 0.9)
						gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/sword/blocked.wav"), 1, ATTN_NORM, 0);
					else
						T_Damage (blip, ent, ent, blip->velocity, blip->s.origin, blip->velocity, damage, 1, DAMAGE_ENERGY, MOD_SWORD);
				}
				else
				{
					// 50% blocked
					if (random() < 0.5)
						gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/sword/blocked.wav"), 1, ATTN_NORM, 0);
					else
						T_Damage (blip, ent, ent, blip->velocity, blip->s.origin, blip->velocity, damage, 1, DAMAGE_ENERGY, MOD_SWORD);
				}
			}
			else
				T_Damage (blip, ent, ent, blip->velocity, blip->s.origin, blip->velocity, damage, 1, DAMAGE_ENERGY, MOD_SWORD);
		}
	}

	// WALLSPARKS
	
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorNormalize(forward);

	VectorCopy(ent->s.origin, start);
	start[2] +=15;

	VectorMA(start, 100, forward, end);
	end[2] += 24;

	tr = gi.trace(start, mins, maxs, end, ent, MASK_SOLID);

	if ((tr.fraction != 1) || tr.startsolid)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/sword/hitwall.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);
	}
}

void Weapon_Sword (edict_t *ent)
{
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == 43)	//DEACTIVATE_LAST
		{
			ChangeWeapon (ent);
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == 4)	//FRAME_ACTIVATE_LAST
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = 5;	//FRAME_IDLE_FIRST
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 39;	//FRAME_DEACTIVATE_FIRST
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			
			if (random() < 0.5)
			{
				ent->client->swordstate = 1;
				ent->client->ps.gunframe = 15;	//FRAME_FIRE_FIRST
			}
			else
			{
				ent->client->swordstate = 0;
				ent->client->ps.gunframe = 27;	//FRAME_FIRE_FIRST

			}
			ent->client->weaponstate = WEAPON_FIRING;

			// start the animation
			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1-1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1-1;
				ent->client->anim_end = FRAME_attack8;
			}
		}
		else
		{
			if (ent->client->ps.gunframe == 14)	//FRAME_IDLE_LAST
			{
				ent->client->ps.gunframe = 5;	//FRAME_IDLE_FIRST
				return;
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if ((ent->client->ps.gunframe == 15) || (ent->client->ps.gunframe == 27))
		{
			if (!CTFApplyStrengthSound(ent))

			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

			if (ent->client->swordstate == 1)
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/sword/swingl.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/sword/swingr.wav"), 1, ATTN_NORM, 0);

			PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		}
		
		if ((ent->client->ps.gunframe == 20) || (ent->client->ps.gunframe == 32))
		{	
			weapon_sword_fire (ent);
		}

		if (ent->client->swordstate == 1)
		{
			if (ent->client->ps.gunframe >= 26)	//FRAME_FIRE_LAST+1
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = 5;
			}
		}
		else
		{
			if (ent->client->ps.gunframe >= 38)	//FRAME_FIRE_LAST+1
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = 5;
			}
		}
		ent->client->ps.gunframe++;
	}
}


//----------------------------------------------------------------------------------------------
// Airfist
//----------------------------------------------------------------------------------------------

void airstuff_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	self->nextthink = level.time + .1;
	self->think = G_FreeEdict;
	self->movetype = MOVETYPE_NONE;
}

void fire_air (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*stuff,*blip = NULL;
	vec3_t  blipdir, addvect,kick,forward, right;
	vec_t   dist;
	int     i;

	AngleVectors (self->client->v_angle, forward, right, NULL);

	VectorNegate(forward,kick);
	VectorNormalize(kick);
	VectorScale(kick,500,kick);
	VectorAdd(self->velocity, kick, self->velocity);

	while ((blip = findradius2(blip, self->s.origin, 600)) != NULL)
	{
		if ((blip->client && !blip->client->camera)
			|| blip->item
			|| Q_stricmp(blip->classname, "bolt") == 0
			|| Q_stricmp(blip->classname, "arrow") == 0
			|| Q_stricmp(blip->classname, "poison_arrow") == 0
			|| Q_stricmp(blip->classname, "explosive_arrow") == 0
			|| Q_stricmp(blip->classname, "grenade") == 0
			|| Q_stricmp(blip->classname, "hgrenade") == 0
			|| Q_stricmp(blip->classname, "flashgrenade") == 0
			|| Q_stricmp(blip->classname, "lasermine") == 0
			|| Q_stricmp(blip->classname, "poisongrenade") == 0
			|| Q_stricmp(blip->classname, "proxymine") == 0
			|| Q_stricmp(blip->classname, "rocket") == 0
			|| Q_stricmp(blip->classname, "turret_rocket") == 0
			|| Q_stricmp(blip->classname, "homing") == 0
			|| Q_stricmp(blip->classname, "buzz") == 0
			|| Q_stricmp(blip->classname, "bfg blast") == 0
			|| Q_stricmp(blip->classname, "item_flag_team1") == 0
			|| Q_stricmp(blip->classname, "item_flag_team2") == 0
			|| Q_stricmp(blip->classname, "bodyque") == 0)
		{
			if (blip == self)
				continue;
			if (!visible(self, blip))
				continue;
			if (!infront(self, blip))
				continue;

			if (Q_stricmp(blip->classname, "item_flag_team1") == 0
				|| Q_stricmp(blip->classname, "item_flag_team2") == 0)
				continue;

			VectorSubtract(blip->s.origin, self->s.origin, blipdir);
			dist = VectorLength(blipdir);
			VectorCopy(blipdir,addvect);
			VectorNormalize(addvect);

			VectorScale(addvect, 1.8 * (600 - dist),addvect);
			addvect[2] = 700 - dist;

			if (dist < 400 && blip->takedamage)
				T_Damage (blip, self, self, blip->velocity, blip->s.origin, blip->velocity, 15, 1, DAMAGE_ENERGY, MOD_AIRFIST);

			blip->s.origin[2] += 5;
			VectorAdd(blip->velocity, addvect, blip->velocity);

			if (blip->client && blip->client->camera)
					blip->client->b_waittime = level.time + 3;
		}
	}

	for (i = 0; i < 4; i++)
	{
		stuff = G_Spawn();
		VectorCopy (start, stuff->s.origin);
		VectorCopy (dir, stuff->movedir);
		vectoangles (dir, stuff->s.angles);
		VectorScale (dir, 1800, stuff->velocity);
		stuff->velocity[0] += crandom() * 150;
		stuff->velocity[1] += crandom() * 150;
		stuff->velocity[2] += crandom() * 10;
		stuff->movetype = MOVETYPE_FLYMISSILE;
		stuff->clipmask = MASK_SHOT;
		stuff->solid = SOLID_BBOX;
		stuff->s.effects |= EF_GRENADE;
		VectorClear (stuff->mins);
		VectorClear (stuff->maxs);
		stuff->s.modelindex = gi.modelindex ("models/objects/dummy/tris.md2");
		stuff->owner = self;
		stuff->touch = airstuff_touch;
		stuff->nextthink = level.time + 0.3;
		stuff->think = G_FreeEdict;
		stuff->s.sound = gi.soundindex ("weapons/air/fly.wav");
		stuff->classname = "airstuff";

		gi.linkentity (stuff);
	}
}

void Weapon_Airfist_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right, end;
	int     i;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->waterlevel == 3)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/air/agwater.wav"), 1, ATTN_NORM, 0);

		for (i = 0; i < 10;i++)
		{
			VectorNormalize(forward);
			VectorScale(forward,100,forward);
			forward[0] += random() * 20 -10;
			forward[1] += random() * 20 -10;
			forward[2] += 3;
			VectorAdd(ent->s.origin, forward, end);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BUBBLETRAIL);
			gi.WritePosition (ent->s.origin);
			gi.WritePosition (end);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	}
	else
	{
		if (random() > 0.1)
		{
			fire_air (ent, start, forward);
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/air/agfire.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/air/agfail.wav"), 1, ATTN_NORM, 0);
		}
	}

	ent->client->v_dmg_pitch = -20;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Airfist (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Airfist_Fire);
}

//----------------------------------------------------------------------------------------------
//  Explosive Shotgun & Explosive Super Shotgun
//----------------------------------------------------------------------------------------------
void fire_lead2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (Q_stricmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{			
					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_ROCKET_EXPLOSION);
			gi.WritePosition (tr.endpos);
			gi.multicast (tr.endpos, MULTICAST_PHS);

			T_RadiusDamage2 (self,tr.endpos, ((int) damage*1.75), 120, MOD_ESSHOT_SPLASH);
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}

void fire_eshotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead2 (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}

void weapon_esupershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int		damage = 15;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 3;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_eshotgun (ent, start, forward, damage, kick, 300, 300, 1, MOD_ESSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW];
	AngleVectors (v, forward, NULL, NULL);
	fire_eshotgun (ent, start, forward, damage, kick, 300, 300, 1, MOD_ESSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 3;
	AngleVectors (v, forward, NULL, NULL);
	fire_eshotgun (ent, start, forward, damage, kick, 300, 300, 1, MOD_ESSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_ExplosiveSuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_esupershotgun_fire);
}

//----------------------------------------------------------------------------------------------
//  Crossbow
//----------------------------------------------------------------------------------------------

void arrow_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
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
		self->s.sound = 0;

		if (other->client && other->client->pers.weapon == it_sword && infront (other, self))
		{
			// 60% blocked
			if (random() < 0.6)
			{
				self->s.sound = 0;
				self->movetype = MOVETYPE_BOUNCE;
				gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
				self->movetype = MOVETYPE_BOUNCE;
				self->velocity[0] += crandom() * 300;
				self->velocity[1] += crandom() * 300;
				self->velocity[2] += crandom() * 500;
				vectoangles (self->velocity, self->s.angles);
			}
			else
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_ARROW);
				gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/crossbow/catch.wav"), 1, ATTN_NORM, 0);
				G_FreeEdict (self);
			}
		}
		else
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_ARROW);
			gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/crossbow/catch.wav"), 1, ATTN_NORM, 0);

			G_FreeEdict (self);
		}
	}
	else if ((Q_stricmp(other->classname, "func_door") == 0) || (Q_stricmp(other->classname, "func_plat") == 0)) //door or plat
	{
		self->s.sound = 0;
		self->movetype = MOVETYPE_BOUNCE;
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
		vectoangles (self->velocity, self->s.angles);
	}
	else
	{
		if (random() > 0.2)
		{
			self->movetype = MOVETYPE_NONE;
			self->solid = SOLID_NOT;
			self->s.sound = 0;
			self->think = G_FreeEdict;
			self->nextthink = level.time + 30;
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit1.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			self->s.sound = 0;
			self->movetype = MOVETYPE_BOUNCE;
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
			vectoangles (self->velocity, self->s.angles);
		}
	}
}

void fire_arrow (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*arrow;

	arrow = G_Spawn();
	VectorCopy (start, arrow->s.origin);
	VectorCopy (dir, arrow->movedir);
	vectoangles (dir, arrow->s.angles);
	VectorScale (dir, speed, arrow->velocity);
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->clipmask = MASK_SHOT;
	arrow->solid = SOLID_BBOX;
	arrow->s.effects = 0;
	VectorClear (arrow->mins);
	VectorClear (arrow->maxs);
	arrow->s.modelindex = gi.modelindex ("models/objects/arrow/tris.md2");
	arrow->owner = self;
	arrow->touch = arrow_touch;
	arrow->nextthink = level.time + 5;
	arrow->think = G_FreeEdict;
	arrow->dmg = damage;
	arrow->s.sound = gi.soundindex ("weapons/crossbow/fly.wav");
	arrow->classname = "arrow";

	gi.linkentity (arrow);
}

void Weapon_Crossbow_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	damage = 40 + (int)(random() * 20.0);

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_arrow (ent, start, forward, damage, 1600);

	if (!is_silenced)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/crossbow/release1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Crossbow (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Crossbow_Fire);
}

//----------------------------------------------------------------------------------------------
//  Poison Crossbow
//----------------------------------------------------------------------------------------------

void parrow_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
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
		self->s.sound = 0;

		if (other->client && other->client->pers.weapon == it_sword && infront (other, self))
		{
			// 60% blocked
			if (random() < 0.6)
			{
				self->s.sound = 0;
				self->movetype = MOVETYPE_BOUNCE;
				gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
				self->movetype = MOVETYPE_BOUNCE;
				self->velocity[0] += crandom() * 300;
				self->velocity[1] += crandom() * 300;
				self->velocity[2] += crandom() * 500;
				vectoangles (self->velocity, self->s.angles);
			}
			else
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_PARROW);
				gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/crossbow/catch.wav"), 1, ATTN_NORM, 0);
				G_FreeEdict (self);
			}
		}
		else
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_PARROW);
			gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/crossbow/catch.wav"), 1, ATTN_NORM, 0);

			G_FreeEdict (self);
		}


		if (other->client)
		{
			if (!(other->client->invincible_framenum > level.framenum)
				&& !(other->flags & FL_GODMODE)) // invulnerable or god
			{
				other->client->PoisonTime += 10;
				G_FreeEdict (self);
			}
		}
	}
	else if ((Q_stricmp(other->classname, "func_door") == 0) || (Q_stricmp(other->classname, "func_plat") == 0)) //door or plat
	{
		self->s.sound = 0;
		self->movetype = MOVETYPE_BOUNCE;
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if (random() > 0.2)
		{
			self->movetype = MOVETYPE_NONE;
			self->solid = SOLID_NOT;
			self->s.sound = 0;
			self->think = G_FreeEdict;
			self->nextthink = level.time + 30;
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit1.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			self->movetype = MOVETYPE_BOUNCE;
			self->s.sound = 0;
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/crossbow/hit3.wav"), 1, ATTN_NORM, 0);
		}
	}
}

void fire_poisonarrow (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*arrow;

	arrow = G_Spawn();
	VectorCopy (start, arrow->s.origin);
	VectorCopy (dir, arrow->movedir);
	vectoangles (dir, arrow->s.angles);
	VectorScale (dir, speed, arrow->velocity);
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->clipmask = MASK_SHOT;
	arrow->solid = SOLID_BBOX;
	arrow->s.effects = 0;
	VectorClear (arrow->mins);
	VectorClear (arrow->maxs);
	arrow->s.modelindex = gi.modelindex ("models/objects/parrow/tris.md2");
	arrow->owner = self;
	arrow->touch = parrow_touch;
	arrow->nextthink = level.time + 5;
	arrow->think = G_FreeEdict;
	arrow->dmg = damage;
	arrow->s.sound = gi.soundindex ("weapons/crossbow/fly.wav");
	arrow->classname = "poison_arrow";

	gi.linkentity (arrow);
}

void Weapon_PoisonCrossbow_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	damage = 20 + (int)(random() * 20.0);

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_poisonarrow (ent, start, forward, damage, 1600);

	if (!is_silenced)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/crossbow/release1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_PoisonCrossbow (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_PoisonCrossbow_Fire);
}

//----------------------------------------------------------------------------------------------
//  Explosive Crossbow
//----------------------------------------------------------------------------------------------

void earrow_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	Grenade_Explode(self);
}

void fire_explosivearrow (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*arrow;

	arrow = G_Spawn();
	VectorCopy (start, arrow->s.origin);
	VectorCopy (dir, arrow->movedir);
	vectoangles (dir, arrow->s.angles);
	VectorScale (dir, speed, arrow->velocity);
	arrow->movetype = MOVETYPE_FLYMISSILE;
	arrow->clipmask = MASK_SHOT;
	arrow->solid = SOLID_BBOX;
	arrow->s.effects = 0;
	VectorClear (arrow->mins);
	VectorClear (arrow->maxs);
	arrow->s.modelindex = gi.modelindex ("models/objects/earrow/tris.md2");
	arrow->owner = self;
	arrow->touch = earrow_touch;
	arrow->nextthink = level.time + 5;
	arrow->think = G_FreeEdict;
	arrow->dmg = damage; 
	arrow->dmg_radius = ex_arrow_radius->value;
	arrow->s.sound = gi.soundindex ("weapons/crossbow/fly.wav");
	arrow->classname = "explosive_arrow";

	gi.linkentity (arrow);
}

void Weapon_ExplosiveCrossbow_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	damage = ex_arrow_damage->value;

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_explosivearrow (ent, start, forward, damage, 1000);

	if (!is_silenced)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/crossbow/release1.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_ExplosiveCrossbow (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_ExplosiveCrossbow_Fire);
}

//----------------------------------------------------------------------------------------------
//  Homing Missile Launcher
//----------------------------------------------------------------------------------------------

void homing_think (edict_t *ent)
{
	edict_t	*target = NULL;
	vec3_t	targetdir, playerdir;
	vec_t	speed;
	int		i;

	if (level.time >= ent->delay)
	{
		G_FreeEdict(ent);
		return;
	}

	for (i = 0; i < numturrets; i++)
	{
		if (!turrets[i]->inuse)
			continue;
		if (turrets[i]->owner == ent->owner)
			continue;
		if (turrets[i]->health <= 0)
			continue;
		if (TeamMembers(ent->owner, turrets[i]->owner))
			continue;
		if (!visible(ent, turrets[i]))
			continue;

		VectorSubtract(turrets[i]->s.origin, ent->s.origin, playerdir);
		playerdir[2] += 10;
		if ((target == NULL) || (VectorLength(playerdir) < VectorLength(targetdir)))
		{
			target = turrets[i];
			VectorCopy(playerdir, targetdir);
		}
	}

	if (!target)
	{
		for (i = 0; i < numplayers; i++)
		{
			if (!players[i]->client)
				continue;
			if (players[i] == ent->owner)
				continue;
			if (players[i]->health <= 0)
				continue;
			if (players[i]->client->camera)
				continue;
			if (TeamMembers(ent->owner, players[i]))
				continue;
			if (!visible(ent, players[i]))
				continue;

			VectorSubtract(players[i]->s.origin, ent->s.origin, playerdir);
			playerdir[2] += 16;
			if ((target == NULL) || (VectorLength(playerdir) < VectorLength(targetdir)))
			{
				target = players[i];
				VectorCopy(playerdir, targetdir);
			}
		}
	}
		
	if (target != NULL)
	{
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.5, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);
	}

	ent->nextthink = level.time + .1;
}


void fire_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/homing/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + .2;
	rocket->think = homing_think;
	rocket->delay = level.time + 10;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "homing";

	gi.linkentity (rocket);
}

void Weapon_HomingLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 50 + (int)(random() * 20.0);
	radius_damage = 50;
	damage_radius = 100;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_homing (ent, start, forward, damage, 650, damage_radius, radius_damage);

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

void Weapon_HomingLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_HomingLauncher_Fire);
}

//----------------------------------------------------------------------------------------------
//  Buzzsaw
//----------------------------------------------------------------------------------------------

void buzz_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (VectorLength(self->velocity) < 5)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && self->movetype != MOVETYPE_BOUNCE)
	{
		if (other->client && other->client->pers.weapon == it_sword && infront (other, self))
		{
			// 60% blocked
			if (random() < 0.6)
			{
				self->movetype = MOVETYPE_BOUNCE;
				self->nextthink = level.time + 5;
				self->think = G_FreeEdict;
				self->velocity[0] += crandom() * 300;
				self->velocity[1] += crandom() * 300;
				self->velocity[2] += crandom() * 500;
				vectoangles (self->velocity, self->s.angles);
				gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/sword/zblock.wav"), 1, ATTN_NORM, 0);
			}
			else
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_BUZZ);
				gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/buzz/buzzflsh.wav"), 1, ATTN_NORM, 0);
				G_FreeEdict (self);
			}
		}
		else
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_BUZZ);
			gi.sound(other, CHAN_WEAPON, gi.soundindex("weapons/buzz/buzzflsh.wav"), 1, ATTN_NORM, 0);
			G_FreeEdict (self);
		}
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPARKS);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (self->velocity);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/buzz/buzzrico.wav"), 1, ATTN_NORM, 0);
	}
}

void buzz_think (edict_t	*ent)
{
	ent->movetype = MOVETYPE_BOUNCE;
	ent->s.sound = 0;
	ent->nextthink = level.time + 5;
	ent->think = G_FreeEdict;
}

void fire_buzz (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*buzz;

	buzz = G_Spawn();
	VectorCopy (start, buzz->s.origin);
	VectorCopy (dir, buzz->movedir);
	vectoangles (dir, buzz->s.angles);
	VectorScale (dir, speed, buzz->velocity);
	buzz->movetype = MOVETYPE_FLYRICOCHET;
	buzz->clipmask = MASK_SHOT;
	buzz->solid = SOLID_BBOX;
	buzz->s.effects = 0;
	VectorClear (buzz->mins);
	VectorClear (buzz->maxs);
	buzz->s.modelindex = gi.modelindex ("models/objects/buzz/tris.md2");
	buzz->owner = self;
	buzz->touch = buzz_touch;
	buzz->nextthink = level.time + 1.8;
	buzz->think = buzz_think;
	buzz->dmg = damage;
	buzz->s.sound = gi.soundindex ("weapons/buzz/buzzwhrl.wav");
	buzz->classname = "buzz";

	gi.linkentity (buzz);
}

void Weapon_Buzzsaw_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	damage = 90 + (int)(random() * 20.0);

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_buzz (ent, start, forward, damage, 1800);

	if (!is_silenced)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/buzz/buzzfire.wav"), 1, ATTN_NORM, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Buzzsaw (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Buzzsaw_Fire);
}

//----------------------------------------------------------------------------------------------
// Flash Grenade
//----------------------------------------------------------------------------------------------



void FlashGrenade_Explode (edict_t *ent)
{
	vec3_t		offset, v;
	edict_t		*target;
	float		Distance, BlindTimeAdd;

	VectorSet(offset, 0, 0, 10);
	VectorAdd(ent->s.origin, offset, ent->s.origin);

	if (ent->owner->client)
		 PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	target = NULL;

	while ((target = findradius(target, ent->s.origin, FLASH_RADIUS)) != NULL) 
	{
		if (!target->client)
			 continue;
		if (!visible(ent, target))
			 continue;
		if (target->client->camera)
			continue;
		if (target->client->invincible_framenum > level.framenum) // invulnerable
			continue;
		if (target->flags & FL_GODMODE) // god
			continue;

		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);

		if ( Distance < FLASH_RADIUS/10 )
			BlindTimeAdd = blindtime->value;
		else
			BlindTimeAdd = 1.5 * blindtime->value * ( 1 / ( ( Distance - FLASH_RADIUS*2 ) / (FLASH_RADIUS*2) - 2 ) + 1 ); // Blind partially
		
		if ( BlindTimeAdd < 0 )
			BlindTimeAdd = 0;

		if (!infront(target, ent))
			   BlindTimeAdd *= .5;

		if (target == ent->owner)
		{
		   target->client->BlindTime = BlindTimeAdd * .75;
		   target->client->BlindBase = blindtime->value;
		   continue;
		}

		target->client->BlindTime = BlindTimeAdd * 1.5;
		target->client->BlindBase = blindtime->value;

		T_Damage (target, ent, ent, target->velocity, target->s.origin, target->velocity, 20, 1, DAMAGE_ENERGY, MOD_FGRENADE);

		if (ent->owner && ent->owner->client)
		{
			cprintf2(target, PRINT_HIGH, "You are blinded by %s's flash grenade!\n", ent->owner->client->pers.netname);
			cprintf2(ent->owner, PRINT_HIGH, "%s has been blinded by your flash grenade!\n", target->client->pers.netname);
		}
		else
		{
			cprintf2(target, PRINT_HIGH, "You are blinded by a flash grenade!\n");
		}
	}
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	BecomeExplosion1(ent);
}

void FlashGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	FlashGrenade_Explode (ent);
}

void fire_flashgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->s.effects = 0;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/gflash/tris.md2");
	grenade->owner = self;
	grenade->touch = FlashGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = FlashGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "flashgrenade";

	gi.linkentity (grenade);
}

void fire_flashgrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->s.effects = 0;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/hgflash/tris.md2");
	grenade->owner = self;
	grenade->touch = FlashGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = FlashGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "flashgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		FlashGrenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void weapon_flashgrenade_fire (edict_t *ent, qboolean held)
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_flashgrenade2 (ent, start, forward, damage, speed, timer, radius, held);

	//vwep
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else if(ent->s.modelindex != 255 && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_FlashGrenade (edict_t *ent)
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_flashgrenade_fire (ent, true);
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
			weapon_flashgrenade_fire (ent, false);
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

//----------------------------------------------------------------------------------------------
// Laser Mine
//----------------------------------------------------------------------------------------------

void pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);
	self->think = target_laser_think;
}

void LaserMine_Explode (edict_t *ent)
{

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
	/*	T_RadiusDamage (ent, ent, 60, NULL, 60,MOD_TARGET_LASER);  */

        if (ent->other)
           G_FreeEdict (ent->other);

	G_FreeEdict (ent);


}

void LaserMine_Die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
   LaserMine_Explode (ent);
}

void LaserMine_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	 int laser_colour[] = {0xf2f2f0f0,             // red
                           0xd0d1d2d3,             // green
                           0xf3f3f1f1,             // blue
                           0xdcdddedf,             // yellow
                           0xe0e1e2e3              // bitty yellow strobe
                           };

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (Q_stricmp(other->classname,"info_player_deathmatch") == 0
		|| Q_stricmp(other->classname,"info_player_start") == 0
		|| Q_stricmp(other->classname, "func_door") == 0
		|| Q_stricmp(other->classname, "func_plat") == 0)
	{
		T_RadiusDamage (ent, ent, 60, NULL, 60,MOD_TARGET_LASER);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition(ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage && plane) // FWP Funky lasermine/corpse race condition pseudo hack fix 
	{
		edict_t *self;

		ent->movetype = MOVETYPE_NONE;

		ent->s.sound  = 0;

		// FWP Killable laser mines
          
                if (lasermine_health->value == 0) {
			ent->clipmask = MASK_SHOT;   
			ent->solid	  = SOLID_NOT;
			VectorClear (ent->mins);
			VectorClear (ent->maxs); 
                }
                else {    
		  ent->clipmask = MASK_SHOT;
		  ent->solid = SOLID_BBOX;
		  ent->takedamage = DAMAGE_YES;
		  ent->health = lasermine_health->value;
		  VectorSet (ent->mins, -5, -5,-5);
		  VectorSet (ent->maxs, 5, 5, 5);
                  ent->die = LaserMine_Die;
                }
		vectoangles(plane->normal,ent->s.angles);
		
		if (lasertime->value > 0)
			ent->nextthink	= level.time + lasertime->value + 2;
		else
			ent->nextthink	= level.time + 62;

		ent->think      = G_FreeEdict;

		self = G_Spawn();

		self->movetype		= MOVETYPE_NONE;
		self->solid			= SOLID_NOT;
		self->s.renderfx	= RF_BEAM|RF_TRANSLUCENT;
		self->s.modelindex	= 1;                    // must be non-zero
		self->s.sound		= gi.soundindex ("world/laser.wav");
		self->classname		= "chaoslaser";
		self->s.frame		= 2;    // beam diameter
		self->owner			= ent;
		self->s.skinnum		= laser_colour[((int) (random() * 1000)) % 5];
		self->dmg           = 200;
        self->think         = pre_target_laser_think;

		if (lasertime->value > 0)
			self->delay = level.time + lasertime->value;
		else
			self->delay = level.time + 60;

		VectorCopy(ent->s.origin,self->s.origin);
		vectoangles(plane->normal,self->s.angles);
		G_SetMovedir (self->s.angles, self->movedir);

		VectorSet (self->mins, -8, -8, -8);
		VectorSet (self->maxs, 8, 8, 8);

		// link to world
		gi.linkentity (self);

                if (lasermine_health->value > 0)
                   ent->other=self;
                
		target_laser_off (self);
		self->nextthink = level.time + 2;	//must be after target_laser_off
	}
}

void fire_lasermine2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->s.effects = 0;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/hglaser/tris.md2");
	grenade->owner = self;
	grenade->touch = LaserMine_Touch;
	grenade->nextthink = level.time + 30;
	grenade->think = G_FreeEdict;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "lasermine";
	
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (grenade);
}

void weapon_lasergrenade_fire (edict_t *ent, qboolean held)
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_lasermine2 (ent, start, forward, damage, speed, timer, radius, held);

	//vwep
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else if(ent->s.modelindex != 255 && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_LaserGrenade (edict_t *ent)
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_lasergrenade_fire (ent, true);
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
			weapon_lasergrenade_fire (ent, false);
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

//----------------------------------------------------------------------------------------------
// Poison Grenade
//----------------------------------------------------------------------------------------------

void PoisonGrenade_Explode (edict_t *ent)
{
	vec3_t		offset, v;
	edict_t		*target;
	float		Distance, PoisonTimeAdd;

	VectorSet(offset, 0, 0, 10);
	VectorAdd(ent->s.origin, offset, ent->s.origin);

	if (ent->owner->client)
		 PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	target = NULL;

	while ((target = findradius(target, ent->s.origin, POISON_RADIUS)) != NULL) 
	{
		if (!target->client)
			 continue;
		if (!visible(ent, target))
			 continue;
		if (target->client->camera)
			continue;
		if (target->client->invincible_framenum > level.framenum) // invulnerable
			continue;
		if (target->flags & FL_GODMODE) // god
			continue;

		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);

		if ( Distance < POISON_RADIUS/10 )
			PoisonTimeAdd = poisontime->value;
		else
			PoisonTimeAdd = 1.5 * poisontime->value * ( 1 / ( ( Distance - POISON_RADIUS*2 ) / (POISON_RADIUS*2) - 2 ) + 1 ); // Blind partially
		
		if ( PoisonTimeAdd < 0 )
			PoisonTimeAdd = 0;

		target->client->PoisonTime = PoisonTimeAdd * 1.5;
		target->client->PoisonBase = poisontime->value;
		target->s.angles[YAW] = (rand() % 360); // Whee!

		if (target->client && target->client->fakedeath != 0)	//target is fakedead
		{
			FakeDeath(target); //unfake him
		}

		T_Damage (target, ent, ent, target->velocity, target->s.origin, target->velocity, 20, 1, DAMAGE_ENERGY, MOD_PGRENADE);

		if (ent->owner && ent->owner->client)
		{
			cprintf2(target, PRINT_HIGH, "You are poisoned by %s's poison grenade!\n", ent->owner->client->pers.netname);
			cprintf2(ent->owner, PRINT_HIGH, "%s has been poisoned by your poison grenade!\n", target->client->pers.netname);
		}
		else
		{
			cprintf2(target, PRINT_HIGH, "You are poisoned by a poison grenade!\n");
		}
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void PoisonGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	PoisonGrenade_Explode (ent);
}

void fire_poisongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->s.effects = 0;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/gpoison/tris.md2");
	grenade->owner = self;
	grenade->touch = PoisonGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = PoisonGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "poisongrenade";

	gi.linkentity (grenade);
}

void fire_poisongrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	grenade->s.effects = 0;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/hgpoison/tris.md2");
	grenade->owner = self;
	grenade->touch = PoisonGrenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = PoisonGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "poisongrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		PoisonGrenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void weapon_poisongrenade_fire (edict_t *ent, qboolean held)
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_poisongrenade2 (ent, start, forward, damage, speed, timer, radius, held);

	//vwep
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else if(ent->s.modelindex != 255 && ent->health > 0)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_PoisonGrenade (edict_t *ent)
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_poisongrenade_fire (ent, true);
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
			weapon_poisongrenade_fire (ent, false);
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

//----------------------------------------------------------------------------------------------
// Proximity Mine
//----------------------------------------------------------------------------------------------

void Proxy_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void Proxy_Explode (edict_t *ent)
{
	float	points;
	edict_t	*blip = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((blip = findradius(blip, ent->s.origin, 200)) != NULL)
	{
		if (!blip->takedamage)
			continue;
		if (blip->die == Proxy_Die)
			continue;

		VectorAdd (blip->mins, blip->maxs, v);
		VectorMA (blip->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = 100 - 0.5 * VectorLength (v);
		if (blip == ent->owner)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (blip, ent))
			{
				VectorSubtract (blip->s.origin, ent->s.origin, dir);
				T_Damage (blip, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_PROXYMINE);
			}
		}
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void Proxy_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 0.1;
	self->think = Proxy_Explode;
}

void Proxy_Think (edict_t *ent)
{
	edict_t	*blip = NULL, *target = NULL;
	vec3_t  blipdir, bestdir, bestangles;
	vec_t	dist, bestdist = 9999;

	ent->nextthink = level.time + 0.2;

	if ( ent->owner && 
	     ent->owner->client && 
	     ent->owner->client->grenadesactive == 0)
		return;

	if (level.time > ent->delay)
	{
		Grenade_Explode(ent);
	}
	
	while ((blip = findradius2(blip, ent->s.origin, 600)) != NULL)
	{
		if( !Valid_Target(ent, blip) )
		  continue;


		VectorSubtract(blip->s.origin, ent->s.origin, blipdir); 
		dist = VectorLength(blipdir);

		if (dist < 60)
		{
			Grenade_Explode(ent);
			return;
		}

		if (dist < bestdist)
		{
			target = blip;
			bestdist = dist;
			VectorSubtract(blip->s.origin, ent->s.origin, bestdir);                         
		}
	}



	if (target && ent->groundentity)
	{
	  /*	        gi.dprintf("Acquired target: %s Dist %f\n", target->classname, bestdist);  */
		VectorSubtract(target->s.origin, ent->s.origin, blipdir);

		vectoangles(bestdir, bestangles);
		ent->s.angles[YAW] = bestangles[YAW] - 90;
		
		/*		  *
		  * Check to see if we are an evil proxy
		   
			if( target && 
			    ( target == ent->owner ||
			      target->owner == ent->owner ||
			      TeamMembers(ent->owner, target) ||
			      TeamMembers(ent->owner, target->owner) ) )
			{
				gi.setmodel (ent, "models/objects/hgevilpr/tris.md2");
				ent->s.event = EV_ITEM_RESPAWN;
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/evilproxy.wav"), 1, ATTN_NORM, 0);
			}
			else // Not an evil proxy
			{
			  	gi.setmodel (ent, "models/objects/hgproxy/tris.md2");
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/proxy.wav"), 1, ATTN_NORM, 0);
			}
		}
		*/


		M_walkmove (ent, bestangles[YAW], 30);
		
		dist = VectorLength(bestdir);
		VectorNormalize(bestdir);
		if (dist > 700)
			dist = 700;
		if (dist < 150)
			dist = 150;
		VectorScale(bestdir, 1.6 * dist, bestdir);

		bestdir[2] = 300 + random() * 200;

		VectorCopy(bestdir, ent->velocity);
	}
	else if (random() < 0.1 && ent->groundentity) // No enemy there so do other things !!
	{
		vec3_t	vangles;

		ent->velocity[0] = crandom() * 400;
		ent->velocity[1] = crandom() * 400;
		ent->velocity[2] = 200 + random() * 300;

		vectoangles(ent->velocity, vangles);

		ent->s.angles[YAW] = vangles[YAW] - 90;

		if (random() < 0.2)
		{
			float rn;

			rn = random();

			if (rn < 0.1)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle1.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.1 && rn < 0.2)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle2.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.2 && rn < 0.3)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle3.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.3 && rn < 0.4)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle4.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.4 && rn < 0.5)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle5.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.5 && rn < 0.6)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle6.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.6 && rn < 0.7)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle7.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.7 && rn < 0.8)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle8.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.8 && rn < 0.9)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle9.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.9)
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/giggle10.wav"), 1, ATTN_NORM, 0);
			} 
	} 
}

void EvilProxy_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	vec3_t  blipdir, blipangles;
	vec_t	dist, bestdist = 9999;
	int	i;

	ent->nextthink = level.time + 0.1;

	if (level.time > ent->delay)
	{
		Grenade_Explode(ent);
	}
	
	if (ent->owner && ent->owner->client && ent->owner->client->grenadesactive == 0)
		return;

	for (i = 0; i < numplayers; i++)
	{
		if (!players[i]->client)
			continue;
		if (!visible(ent, players[i]))
			continue;
		if (players[i]->health <= 0)
			continue;
		if (players[i]->client->camera)
			continue;
		if (players[i]->client->invisible_framenum > level.framenum)	//invisible
			continue;

		VectorSubtract(players[i]->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);

		if (dist < 60)
		{
			Grenade_Explode(ent);
			return;
		}

		if (dist < bestdist)
		{
			target = players[i];
			bestdist = dist;
		}
	}

	for (i = 0; i < numturrets; i++)
	{
		if (!turrets[i]->inuse)
			continue;
		if (!visible(ent, turrets[i]))
			continue;
		if (turrets[i]->health <= 0)
			continue;

		VectorSubtract(turrets[i]->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);

		if (dist < 60)
		{
			Grenade_Explode(ent);
			return;
		}

		if (dist < bestdist)
		{
			target = turrets[i];
			bestdist = dist;
		}
	}

	if (target && ent->groundentity)
	{
		VectorSubtract(target->s.origin, ent->s.origin, blipdir);

		vectoangles(blipdir, blipangles);
		ent->s.angles[YAW] = blipangles[YAW] - 90;
		
		if (random() < 0.1)
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/evilproxy.wav"), 1, ATTN_NORM, 0);

		M_walkmove (ent, blipangles[YAW], 30);
		
		dist = VectorLength(blipdir);
		VectorNormalize(blipdir);
		if (dist > 700)
			dist = 700;
		if (dist < 150)
			dist = 150;
		VectorScale(blipdir, 1.6 * dist, blipdir);

		blipdir[2] = 300 + random() * 200;

		VectorCopy(blipdir, ent->velocity);
	}
	else if (random() < 0.05  && ent->groundentity) // No enemy there so do other things !!
	{
		vec3_t	vangles;

		ent->velocity[0] = crandom() * 400;
		ent->velocity[1] = crandom() * 400;
		ent->velocity[2] = 200 + random() * 300;

		vectoangles(ent->velocity, vangles);

		ent->s.angles[YAW] = vangles[YAW] - 90;

		if (random() < 0.2)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/evilproxy.wav"), 1, ATTN_NORM, 0);
		}
	}
}

void Proxy_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	ent->s.effects = EF_GREENGIB;

	if (!other->takedamage)
	{
	  	/*
		 * This is now handled in the Proxy_Think
		if (random() < 0.1 && ent->owner && (Q_stricmp(ent->owner->classname, "bot") !=0))	//evil proxy
		{
			gi.setmodel (ent, "models/objects/hgevilpr/tris.md2");
			ent->s.event = EV_ITEM_RESPAWN;
			ent->think = EvilProxy_Think;
		}
		else
		*/

		ent->think = Proxy_Think;
		ent->nextthink = level.time +1;
		ent->touch = NULL;
		
	}
	else
	{
		Grenade_Explode(ent);
	}
}

void fire_proxymine (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->movetype = MOVETYPE_STEP;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects = 0;
	VectorSet (grenade->mins, -2, -2, -2);
	VectorSet (grenade->maxs, 2, 2, 2);
	grenade->s.modelindex = gi.modelindex ("models/objects/hgproxy/tris.md2");
	grenade->owner = self;
	grenade->touch = Proxy_Touch;
	grenade->viewheight	= 40;

	if (proxytime->value > 0)
		grenade->delay = level.time + proxytime->value;
	else
		grenade->delay = level.time + 100;

	grenade->nextthink = level.time + 30;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "proxymine";
	grenade->mass = 10;
	grenade->health = 120;
	grenade->die = Proxy_Die;
	grenade->flags |= FL_NO_KNOCKBACK;
	grenade->takedamage = DAMAGE_YES;


	gi.linkentity (grenade);
}

//----------------------------------------------------------------------------------------------
// Flash Grenade Launcher
//----------------------------------------------------------------------------------------------

void weapon_flashgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_flashgrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_FlashGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_flashgrenadelauncher_fire);
}

//----------------------------------------------------------------------------------------------
// Poison Grenade Launcher
//----------------------------------------------------------------------------------------------

void weapon_poisongrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_poisongrenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_PoisonGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_poisongrenadelauncher_fire);
}

//----------------------------------------------------------------------------------------------
// Proxy Mine Launcher
//----------------------------------------------------------------------------------------------

void weapon_proxyminelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_proxymine (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_ProxyMineLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_proxyminelauncher_fire);
}

//----------------------------------------------------------------------------------------------
// Gravity Vortex
//----------------------------------------------------------------------------------------------

#define	VORTEX_INACTIVE		0
#define	VORTEX_FIRED		1
#define	VORTEX_STARTING		2
#define	VORTEX_SPAWNSTUFF	3
#define	VORTEX_ACTIVE		4

void Vortex_Free (edict_t *ent)
{
	vortex_pointer = NULL;
	vortexstate = VORTEX_INACTIVE;
	G_FreeEdict(ent);
}

void BlackHole_Think (edict_t *ent)
{
	if (level.time > ent->delay)
	{
		G_FreeEdict(ent);
		return;
	}

	ent->nextthink = level.time + .1;

	//animate black hole
	ent->s.frame += 1;
	if (ent->s.frame > 19)
		ent->s.frame = 0;
}

/*
 * Validate Target
 * This is a general ussage function that validates if the
 * target "blip" is a valid target for the hunter "ent"
 */
int Valid_Target( edict_t *ent, edict_t *blip )
{
int dummy;

  	/*
	 * For every item we never allow it to legally target itself
	 */
	if (blip == ent)
	  return false;

	/* 
	 * The Vortex doesn't care who launched it or who is one what team,
	 * it just eats everything in it's path
	 */
	if(Q_stricmp(ent->classname, "vortex") /*== 0)*/)
	  /*
	{
	  if (blip->item || blip->client)
	  	  return true;
	}
	else // These are the quick rules for Turret's, Proxies, ect...
	*/
	{
		/*
		 * Can See
		 * FIXME:  The whole "Can See" concept should be a
		 * sub-function that automaticly handles infront, visible,
		 * invisible, and darkness.  And of corse, the vortex
		 * shouldn't ever need to use it, but turrets, proxies, even
		 * havok bot's should likely use the same function to decide
		 * if a target is visible.
		 */
	  /*		if( !visible(ent,blip) ||
		    !infront(ent,blip) ||
		    blip->health <= 0 )
		  return false;
 
	  */
	          if (blip->health <= 0)
                      return false;

		/*
		 * Turret's and Proxies should only be attacking other
		 * Turrets, proxies, and other players, i.e. items that take
		 * damage.
		 */
		if( !blip->takedamage )
		  return false;


		/* Check for invisible?  */
		if( blip->client && blip->health > 0 && 
		    blip->client->invisible)
		  return false;


		/*		if( blip == ent->owner ||
		    blip->owner == ent->owner ||
		    TeamMembers(ent->owner, blip) ||
		    TeamMembers(ent->owner, blip->owner) ) 
		  return false;
		  
		*/


		/*
		 * Here we need to do a check to settup Evil Proxies
		 * If we are a proxymine we roll the dice for an evil proxy,
		 * if the roll fails then we fall back to the turret/proxy
		 * rules, else we are an evil proxy and that's life. */


		if( Q_stricmp(ent->classname, "proxymine") == 0 )
		{


                  dummy = 0;

		  if(( Q_stricmp(blip->classname, "rocket_turret") == 0 ) &&
                              (blip->owner != ent->owner))   
		  return true; 
                  

		if (( Q_stricmp(blip->classname, "laser_turret") == 0 ) &&
                             (blip->owner != ent->owner))
		  return true;


                if ((Q_stricmp(blip->classname, "player") == 0) &&
                     (blip != ent->owner) && !TeamMembers(ent->owner, blip->owner))
                   return true;


                if ((Q_stricmp(blip->classname, "bot") == 0) &&
                     (blip != ent->owner) && !TeamMembers(ent->owner, blip->owner))
                   return true;

                if (Q_stricmp(blip->classname, "proxymine") == 0) 
		{
                    if (blip->owner == ent->owner)
		    { 
			 return (false);
		    }
		    else
		    {
			return (true);
                    }
		}

		  /* Roll for an EvilProxy */
                	if ( random() < 0.05 && 
			     ent->owner && 
			     (Q_stricmp(ent->owner->classname, "bot") != 0) )
			{
			  	if( blip == ent->owner ||
			       	    blip->owner == ent->owner ||
			       	    TeamMembers(ent->owner, blip) ||
			       	    TeamMembers(ent->owner, blip->owner) ) 
				  return true;
			}

		}

		//		else	// Something other then Evil Proxies, at the time of
		//{ 	// this writting this was only Proxies and Turrets
		
		// }
		if(blip->client && blip->client->camera)
			return false;
 
		return false;
	} else {

	  if(blip->client) {
	    return true;
	  }

	/*
	 * This part of the search takes up alot of CPU time
	 * so we use a switch statement to make a general match w/
	 * as little CPU overhead as possible and then get into the
	 * fun stuff.
	 *
	 * Note:  This currently has no support for Proxies and Turrets
	 * 	  and I dobt it will ever really be needed for them.
	 */
	switch(blip->classname[0])
	{
	  	case 'a':
			if( Q_stricmp(blip->classname, "arrow") == 0 )
			  return true;
			break;
	  	case 'b':
			if( Q_stricmp(blip->classname, "bolt") == 0
			 || Q_stricmp(blip->classname, "buzz") == 0
			 || Q_stricmp(blip->classname, "bfg blast") == 0
			 || Q_stricmp(blip->classname, "blackholestuff") == 0 )
			  return true;
			break;

		case 'e':
			if( Q_stricmp(blip->classname, "explosive_arrow") == 0 )
			  return true;
			break;

		case 'f':
			if( Q_stricmp(blip->classname, "flashgrenade") == 0 )
			  return true;
			break;

		case 'g':
			if( Q_stricmp(blip->classname, "grenade") == 0 
			 || Q_stricmp(blip->classname, "gib") == 0 )
			  return true;
			break;

		case 'h':
			if( Q_stricmp(blip->classname, "hgrenade") == 0 
			 || Q_stricmp(blip->classname, "homing") == 0 )
			  return true;
			break;
		case 'i':
			if( Q_stricmp(blip->classname, "item_flag_team1") == 0
 			 || Q_stricmp(blip->classname, "item_flag_team2") == 0
 			 || Q_stricmp(blip->classname, "item_tech1") == 0
 			 || Q_stricmp(blip->classname, "item_tech2") == 0
 			 || Q_stricmp(blip->classname, "item_tech3") == 0
 			 || Q_stricmp(blip->classname, "item_tech4") == 0)
  			return false;

		case 'l':
			if( Q_stricmp(blip->classname, "lasermine") == 0 
			 || Q_stricmp(blip->classname, "laser_turret") == 0 )
			  return true;
			break;

		case 'p':
			if( Q_stricmp(blip->classname, "poisongrenade") == 0 
			 || Q_stricmp(blip->classname, "poison_arrow") == 0 
			 || Q_stricmp(blip->classname, "proxymine") == 0 )
			  return true;
			break;
		case 'r': 
			if( Q_stricmp(blip->classname, "rocket") == 0 
			 || Q_stricmp(blip->classname, "rocket_turret") == 0 )
			  return true;
			break;
		case 't': 
			if( Q_stricmp(blip->classname, "turret_rocket") == 0 )
			  return true;
			break;
		default:
			return false;
			break;
	}
	}

	/*
	 * If we didn't match up a rule by now then just assume it's
	 * false
	 */
	return false;
}

void Vortex_Think (edict_t *ent)
{
	edict_t			*stuff, *blip = NULL;
	vec3_t			blipdir;
	vec_t			dist;
	static	edict_t	* black;

	ent->nextthink = level.time + .1;

	if (vortexstate == VORTEX_FIRED)
	{
		VectorClear(ent->velocity);
		VectorClear(ent->avelocity);
		vortexstate = VORTEX_STARTING;
	}
	else if (vortexstate == VORTEX_STARTING)
	{
		if (level.time > ent->delay)
		{
			VectorClear(ent->velocity);
			VectorClear(ent->avelocity);
			ent->movetype = MOVETYPE_FLYMISSILE;
			vortexstate = VORTEX_SPAWNSTUFF;
		}

		//animate rings
		ent->velocity[2] = 15;
		ent->s.frame += 1;
		if (ent->s.frame > 19)
			ent->s.frame = 0;

	}
	else if (vortexstate == VORTEX_SPAWNSTUFF)
	{
		int		i;

		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/vortex/storm.wav"), 1, ATTN_NORM, 0);

		// spawn black hole
		black = G_Spawn();
		VectorCopy (ent->s.origin, black->s.origin);
		black->movetype = MOVETYPE_NONE;
		black->clipmask = MASK_SHOT;
		black->solid = SOLID_NOT;
		VectorClear (black->mins);
		VectorClear (black->maxs);
		black->s.modelindex = gi.modelindex ("models/objects/avortex/tris.md2");
		black->owner = ent->owner;
		black->nextthink = level.time + 0.1;
		black->think = BlackHole_Think;
		black->classname = "blackhole";
		black->delay = level.time + 15;

		gi.linkentity (black);

		// spawn black hole stuff
		for (i = 0; i < 7; i++)
		{
			stuff = G_Spawn();
			VectorCopy (ent->s.origin, stuff->s.origin);
			stuff->velocity[0] += crandom () * 400;
			stuff->velocity[1] += crandom () * 400;
			stuff->velocity[2] += crandom () * 400;
			stuff->movetype = MOVETYPE_FLYMISSILE;
			stuff->clipmask = MASK_SHOT;
			stuff->solid = SOLID_NOT;
			VectorClear (stuff->mins);
			VectorClear (stuff->maxs);

			if (i < 2)
				stuff->s.effects |= EF_FLAG1;
			else if (i == 2 || i == 3)
				stuff->s.effects |= EF_FLAG2;
			else if (i == 4)
				stuff->s.effects |= EF_TELEPORTER;
			else if (i == 5)
				stuff->s.effects |= EF_ROCKET;
			else if (i == 6)
				stuff->s.effects |= EF_BFG;


			stuff->s.modelindex = gi.modelindex ("models/objects/dummy/tris.md2");
			stuff->owner = ent;
			stuff->nextthink = level.time + 15;
			stuff->think = G_FreeEdict;
			stuff->classname = "blackholestuff";

			gi.linkentity (stuff);
		}

		ent->delay = level.time + 15;
		vortexstate	= VORTEX_ACTIVE;
	}
	else if (vortexstate == VORTEX_ACTIVE)
	{
		//animate rings
		ent->s.frame += 1;
		if (ent->s.frame > 19)
			ent->s.frame = 0;

		// move
		VectorCopy(ent->s.origin, black->s.origin);

		ent->velocity[0] += crandom() * 20;
		ent->velocity[1] += crandom() * 20;
		ent->velocity[2] += crandom() * 20;

		VectorNormalize(ent->velocity);
		VectorScale(ent->velocity, 50, ent->velocity);

		VectorCopy(ent->velocity, black->velocity);

		//suck
		while ((blip = findradius2(blip, ent->s.origin, 600)) != NULL)
		{
		  	if( !Valid_Target(ent, blip) )
			  continue;

			VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
			dist = VectorLength(blipdir);
			VectorNormalize(blipdir);
			
			if (dist < 100)
			{
				if (blip->takedamage)
				{
					T_Damage (blip, ent, ent->owner, blip->velocity, blip->s.origin, blip->velocity, 1000, 1, DAMAGE_ENERGY, MOD_VORTEX);
				}
				if (blip->item)
				{
					if (!(blip->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
					{
						SetRespawn (blip, random() * 30 + 10);
					}
					else
					{
						G_FreeEdict (blip);
					}
				}
			}
			VectorMA(blip->velocity, -1.4 *(600 - dist), blipdir, blip->velocity);
			blip->velocity[0] += crandom() * 60;
			blip->velocity[1] += crandom() * 60;
			blip->velocity[2] += random() * 60;
		}

		if (level.time > ent->delay)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_BIGEXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			Vortex_Free(ent);
			return;
		}
	}
}

void Vortex_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		Vortex_Free(ent);
		return;
	}

	if (!other->takedamage)
	{
		ent->delay = level.time + 3;
		ent->solid = SOLID_NOT;
		ent->movetype = MOVETYPE_FLYMISSILE;
		ent->nextthink = level.time + 0.1;
		ent->think = Vortex_Think;
		ent->touch = NULL;
	}
}

void fire_vortex (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*vortex;

	vortex = G_Spawn();
	VectorCopy (start, vortex->s.origin);
	VectorCopy (dir, vortex->movedir);
	vectoangles (dir, vortex->s.angles);
	VectorScale (dir, speed, vortex->velocity);
	vortex->movetype = MOVETYPE_STEP;
	vortex->clipmask = MASK_SHOT;
	vortex->solid = SOLID_BBOX;
	vortex->s.effects |= EF_GRENADE;
	VectorClear (vortex->mins);
	VectorClear (vortex->maxs);
	vortex->s.modelindex = gi.modelindex ("models/objects/arngs/tris.md2");
	vortex->owner = self;
	vortex->touch = Vortex_Touch;
	vortex->nextthink = level.time + 30;
	vortex->think = Vortex_Free;
	vortex->classname = "vortex";

	gi.linkentity (vortex);

	vortex_pointer = vortex;
	vortexstate = VORTEX_FIRED;
}

void weapon_vortex_fire (edict_t *ent)
{
	vec3_t	offset, forward, right, start;
	float   timer, speed;

	if (vortexstate != VORTEX_INACTIVE)
	{
		cprintf2(ent, PRINT_HIGH, "Only one Gravity Vortex can be active at the same time...try later!");
		return;
	}

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	speed *= 1.5;
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/vortex/throw.wav"), 1, ATTN_IDLE, 0);
	fire_vortex (ent, start, forward, speed);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	NoAmmoWeaponChange (ent);
}

void Weapon_Vortex (edict_t *ent)
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
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
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
			weapon_vortex_fire (ent);
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
