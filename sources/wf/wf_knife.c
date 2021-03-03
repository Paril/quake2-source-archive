#include "g_local.h"
/*
==========================
Knife
==========================
*/
void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void drop_make_touchable (edict_t *ent);
void WFRemoveDisguise(edict_t *ent);

void ChuckAmmoFromSentry(edict_t *ent)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	gitem_t *item;
	int count;

	if (ent->light_level <= 0) return;//out of ammo

	item = FindItem("Bullets");

	if (item == NULL)
	{
		//gi.dprintf("Cant find bullets item\n");
		return;
	}

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	AngleVectors (ent->s.angles, forward, right, NULL);
	VectorCopy (ent->s.origin, dropped->s.origin);

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	//Do 100 bullets at a time
	if (ent->light_level > 100)
		count = 100;
	else
		count = ent->light_level;

	dropped->count = count;
	ent->light_level -= count;

	gi.linkentity (dropped);

}
/*
=================
fire_stab
=================
*/
void fire_stab (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod)
{
	trace_t		tr;
	edict_t		*ignore;
//	int			mask;
//	qboolean	water;
	vec3_t		forward, wallp;
    int         content_mask = MASK_PLAYERSOLID |MASK_MONSTERSOLID |MASK_SHOT;

	int		bodypos;	//Where on the body was the hit?

//	VectorMA (start, 8192, aimdir, end);
//	VectorCopy (start, from);
	ignore = self;
	// Setup "little look" to close wall
	VectorCopy(self->s.origin,wallp);         

	// Cast along view angle
	AngleVectors (self->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=self->s.origin[0]+forward[0]*50;
	wallp[1]=self->s.origin[1]+forward[1]*50;
	wallp[2]=self->s.origin[2]+forward[2]*50;  

	// trace
	tr = gi.trace (self->s.origin, NULL, NULL, wallp, self, content_mask);

	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
		//gi.dprintf ("Too far from entity.\n");
		//A complete miss will not remove your disguise
		//if ((self->disguiseshots <= 0) && (self->disguised)) self->disguiseshots = 1;
		return;
	}

//gi.dprintf("Knife Hit %s\n", tr.ent->classname);

	//Location damage!  Was it a head or leg shot?
	if (tr.ent->client)
	{
		if (self->disguised) WFRemoveDisguise(self);

		if (tr.endpos[2] < (tr.ent->s.origin[2] - 10))
		{
			bodypos = 1;  // leg shot
		}
		else if (tr.endpos[2] > ((tr.ent->s.origin[2] + 20)))
		{
			bodypos = 2;
		}
		else 
		{
			bodypos = 0;	//Body shot
			//See if it was in the back!
			if (!infront(tr.ent, self))
			{
				bodypos = 3;	//in the back!
				damage = damage * 5;
			}
		}

		if (tr.ent->wf_team != self->wf_team)
		{
			//Play a hit sound
			if (bodypos == 3)
				gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/knifescream.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/knifehit.wav"), 1, ATTN_NORM, 0);
		}
		else
			damage = 0;

	}
	else
	{
		//For non-clients
//gi.dprintf("Hit item: %s\n",tr.ent->classname);

		//If it is a sentry gun, remove some ammo
		if (strcmp(tr.ent->classname, "SentryGun") == 0) 
		{
		  if ((tr.ent->wf_team != self->wf_team) && (self->client->player_special & SPECIAL_DISGUISE))
		  {
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/knifehit.wav"), 1, ATTN_NORM, 0);
			ChuckAmmoFromSentry(tr.ent);			
			damage = damage * .1;

			//Hitting the sentry gun will not remove your disguise
			//if (self->disguiseshots <= 0) self->disguiseshots = 1;
		  }
		  else
			  damage = 0;
		}

		//If it is a decoy, don't do much
		else if (strcmp(tr.ent->classname, "decoy") == 0) 
		{
			//gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/disembowel.wav"), 1, ATTN_NORM, 0);
		}


		//Otherwise, make it look like a bullet shot.
		else
		{
			//damage = 0;
			if (self->disguised) WFRemoveDisguise(self);
			fire_bullet (self, start, aimdir, 0, kick, 0, 0, MOD_SNIPERRIFLE);
		}
	}

	if ((damage) && (CanDamage(tr.ent, self)) )
		T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);


//	if (self->client)
//		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}



void weapon_knife_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = wf_game.weapon_damage[WEAPON_KNIFE];
	int			kick = 4;


	if (ent->client->ps.gunframe == 11)
	{
		//Play a slicing cound
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/knifeswish.wav"), 1, ATTN_NORM, 0);

		ent->client->ps.gunframe++;
		return;
	}

	//Must be frame 13 here then
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

	fire_stab (ent, start, forward, damage, kick, MOD_KNIFE);

	// send muzzle flash
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_SHOTGUN | is_silenced);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_IMPACT);

	//does not take any ammo
//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Knife (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {11, 13, 0};	//Call fire frame on 8 and 13

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_knife_fire);
}

