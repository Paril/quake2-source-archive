#include "g_local.h"


void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
void bfg_explode (edict_t *self);


//////////////////////////////////////////////////////////////////////
// ======================================
// Quake and Doom Firing Weapon Rountines - Skid
// =====================================
////////////////////////////////////////////////////////////////////////

/*
=============
q1_fire_axe
=============
*/

void q1_fire_axe ( edict_t *self, vec3_t start, vec3_t dir, int damage)
{    
    trace_t tr; 
    vec3_t end;
	
    VectorMA (start, 36 , dir, end);  
	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
    
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
               T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, damage, 0, MOD_Q1_AXE);
			   if (!(tr.ent->svflags & SVF_MONSTER) && tr.ent->client)
			   {
				   if(tr.ent->client->resp.player_class == CLASS_Q1)
				   {
				   		tr.ent->pain_debounce_time = level.time;
						gi.sound (tr.ent, CHAN_VOICE, gi.soundindex("q1weap/axe/axhit1.wav") , 1.0 , ATTN_NORM, 0);
				   }
			   }
				   gi.WriteByte (svc_temp_entity);
				   gi.WriteByte (TE_MOREBLOOD);
				   gi.WritePosition (tr.endpos);
				   gi.WriteDir (tr.plane.normal);
				   gi.multicast (tr.endpos, MULTICAST_PVS);
            }        
            else
			{
				tr.endpos[2] +=5;

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SHOTGUN);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				gi.sound (self, CHAN_AUTO, gi.soundindex("q1weap/axe/axhit2.wav") , 0.8 , ATTN_NORM, 0);
			}
        }
    }
    return;
}  


/*
=================
fire_Nail

Fires a Nail, Used by Q1 NG and SNG
=================
*/
void nail_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
			mod = MOD_Q1_SNG;
		else
			mod = MOD_Q1_NG;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_BULLET, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (self->s.origin);
		gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);	

		//not if online, too laggy
		if(!deathmatch->value)
		{
			float sound= random();
			if (sound < 0.3)
			{
				if (sound < 0.1)
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weap/nails/ric3.wav"), 1, ATTN_STATIC, 0);
				else if (sound < 0.2)
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weap/nails/ric2.wav"), 1, ATTN_STATIC, 0);
				else 
					gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weap/nails/ric1.wav"), 1, ATTN_STATIC, 0);
			}
			else if (sound < 0.5)
				gi.sound (self, CHAN_WEAPON, gi.soundindex ("q1weap/nails/tink1.wav"), 1, ATTN_STATIC, 0);
		}

	}
	G_FreeEdict (self);
}



void q1_fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean sng)
{
	edict_t	*nail;
	trace_t	tr;

	VectorNormalize (dir);

	nail = G_Spawn();
	VectorCopy (start, nail->s.origin);
	VectorCopy (start, nail->s.old_origin);
	vectoangles (dir, nail->s.angles);
	VectorScale (dir, speed, nail->velocity);
	nail->movetype = MOVETYPE_FLYMISSILE;
	nail->clipmask = MASK_SHOT;
	nail->solid = SOLID_BBOX;
	nail->svflags = SVF_DEADMONSTER;
	nail->s.renderfx = RF_FULLBRIGHT;
	VectorClear (nail->mins);
	VectorClear (nail->maxs);
	nail->s.modelindex = gi.modelindex ("models/objects/q1nail/tris.md2");
	nail->owner = self;
	nail->touch = nail_touch;
	nail->nextthink = level.time + 8000/speed; 
	nail->think = G_FreeEdict;
	nail->dmg = damage;
	nail->classname = "nail";
	if (sng)
		nail->spawnflags = 1;
	gi.linkentity (nail);

	if (self->client)
		check_dodge (self, nail->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, nail->s.origin, nail, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (nail->s.origin, -10, dir, nail->s.origin);
		nail->touch (nail, tr.ent, NULL, NULL);
	}
}


//////////////////////////
//////////////////////////
// Q1 GRENADE
//////////////////////////
//////////////////////////

void q1_explode (edict_t *self)
{
	if (self->s.frame == 5)
	{
		G_FreeEdict (self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

void q1_grenade_explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_Q1_GL);
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_Q1_GL_SPLASH);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.sound (ent, CHAN_AUTO, gi.soundindex ("q1weap/rocket/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// explosion sprite
	gi.unlinkentity (ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy( origin, ent->s.origin);
	VectorCopy( origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0; 
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}


static void q1_grenade_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;
	
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	
	if (!other->takedamage || 
		(other->solid == SOLID_BSP))
	{
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex ("q1weap/grenades/bounce.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent->enemy = other;
	q1_grenade_explode (ent);
}


void q1_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + (random() - 0.5) * 20.0, up, grenade->velocity);
	VectorMA (grenade->velocity, (random() - 0.5) * 20.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/q1gren/tris.md2"); 
	grenade->owner = self;
	grenade->touch = q1_grenade_touch;
	grenade->nextthink = level.time + timer;
	grenade->think = q1_grenade_explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}



//////////////////////////
//////////////////////////
// Q1 ROCKET
//////////////////////////
//////////////////////////

void q1_rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_Q1_RL);
	}
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_Q1_RL_SPLASH);
	gi.sound (ent,CHAN_AUTO , gi.soundindex ("q1weap/rocket/r_exp3.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION); 
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

//	if(ent->target_ent)
//		G_FreeEdict(ent->target_ent);

	// explosion sprite
	gi.unlinkentity (ent);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorCopy( origin, ent->s.origin);
	VectorCopy( origin, ent->s.old_origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_explod.sp2"); 
	ent->s.frame = 0; 
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST; 
	ent->think = q1_explode; 
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}




void q1rocketTrail_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_FreeEdict (ent);
}

//Rocket Trails 
//needed to give the yellow part of the Rocket trail

void q1rocket_trail (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*bolt;
	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, 950, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects = EF_BLASTER;
	bolt->svflags |= SVF_DEADMONSTER;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->owner = self;
	bolt->touch = q1rocketTrail_touch;
	bolt->nextthink = level.time + 8000/950;
	bolt->think = G_FreeEdict;
	bolt->classname = "bolt";
	gi.linkentity (bolt);

}

	
void q1_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_GRENADE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = q1_rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	
	q1rocket_trail (rocket, start, dir);
	
	gi.linkentity (rocket);
}


//////////////////////////
//////////////////////////
// Q1 LIGHTNING
//////////////////////////
//////////////////////////

void q1_fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	vec3_t end;
	trace_t  tr;

	
	VectorNormalize(dir);
	VectorMA(start,600,dir,end);

	//Initial Trace - damage if close enough
	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		if(tr.ent && (tr.ent !=self) && (tr.ent->takedamage))
		T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LG);
	    return;
	}
	else
	{
		//trace 2
		tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	
		if(self->client->chasetoggle)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
			gi.WriteShort (self->client->oldplayer - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos); 
			gi.multicast (self->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
			gi.WriteShort (self - g_edicts);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos); 
			gi.multicast (self->s.origin, MULTICAST_PVS);
		}

		if ((tr.ent != self) && (tr.ent->takedamage))
			T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_Q1_LG);
	}
}




//====================================
//====================================
// Doom
//====================================
//====================================


/*
=============
d_fire_punch
=============
*/

void d_fire_punch ( edict_t *self, vec3_t start, vec3_t dir, int damage)
{    
    trace_t tr; 
    vec3_t end;
	
    VectorMA (start, 32 , dir, end);  
	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
    
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
			gi.sound (self, CHAN_VOICE, gi.soundindex("dweap/punch.wav") , 1.0 , ATTN_NORM, 0);
            if (tr.ent->takedamage)            
            {
               T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, damage, 0, MOD_DOOM_FISTS);
			   			   
			   gi.WriteByte (svc_temp_entity);
			   gi.WriteByte (TE_MOREBLOOD);
			   gi.WritePosition (tr.endpos);
			   gi.WriteDir (tr.plane.normal);
			   gi.multicast (tr.endpos, MULTICAST_PVS);
            }        
            else
			{
				tr.endpos[2] +=5;

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SHOTGUN);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}
        }
    }
} 


//////////////////////////
//////////////////////////
// DOOM CHAINSAW
//////////////////////////
//////////////////////////

void d_fire_saw ( edict_t *self, vec3_t start, vec3_t dir, int damage)
{    
    trace_t tr; 
    vec3_t end;
	// PULL  
//	vec3_t forward;  
	qboolean hit=false;
    	
	VectorMA (start, 30 , dir, end);
//	VectorAdd(dir,forward,forward);

	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
    
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)         
            {
//Get Stuck		// Simulate Pull
				if(tr.ent->health >0  && (tr.ent->solid != SOLID_BSP))
				{
//					VectorScale(forward, -25, forward);  
//					VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);

					if(tr.ent->client)
					{
						if((tr.ent->client->v_dmg_pitch > 0)
							&& (tr.ent->client->v_dmg_pitch < 6.0))
							tr.ent->client->v_dmg_pitch -= 1.0;
						else
							tr.ent->client->v_dmg_pitch += 1.0;
				
						if(random() > 0.5)
							tr.ent->client->v_dmg_roll = random() * 4.0f;
						else
							tr.ent->client->v_dmg_roll = -1.0 *(random() * 4);
						tr.ent->client->v_dmg_time = level.time + (DAMAGE_TIME*2);
//gi.dprintf("CLIENT BEING CHAINSAWED\n"); 
					}
				}
				
				// Bobbing ?
							
				if((self->client->v_dmg_pitch > 0)
					&& (self->client->v_dmg_pitch < 8.0))
					self->client->v_dmg_pitch -= 1.0;
				else
					self->client->v_dmg_pitch += 1.0;
				
				if(random() > 0.5)
					self->client->v_dmg_roll = (float)(random() * 5);
				else
					self->client->v_dmg_roll = -1.0 *(random() * 5);

				  T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_DOOM_SAW); // kick 0
				  gi.WriteByte (svc_temp_entity);
				  gi.WriteByte (TE_MOREBLOOD);
				  gi.WritePosition (tr.endpos);
				  gi.WriteDir (tr.plane.normal);
				  gi.multicast (tr.endpos, MULTICAST_PVS);
				  self->client->oldweapon = level.time + 0.4;
            }
			else
			{
				tr.endpos[2] +=10;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SHOTGUN);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}
			hit=true;
        }
    }

	if(hit==true) 
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("dweap/sawhit.wav") , 1.0 , ATTN_NORM, 0);
	}
	else if(self->client->oldweapon < level.time) 
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("dweap/sawon.wav") , 1.0 , ATTN_NORM, 0);
		self->client->oldweapon = level.time + 1.2;
	}
	
    return;
} 


/*
=================
fire_plasma

Fires a plasma blob
=================
*/

//explode anim
void d_plasma_explode (edict_t *self)
{
	if (self->s.frame == 3)
	{
		G_FreeEdict(self);
		return;
	}
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}
 

void plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t origin;
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	VectorMA (self->s.origin, -0.02, self->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_DOOM_PG);
	}
	
	gi.sound (self, CHAN_AUTO, gi.soundindex ("dweap/phit.wav"), 1, ATTN_NORM, 0);
	
	gi.unlinkentity(self);		
	
	// explosion sprite 
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy(origin,self->s.origin);
	VectorCopy(origin,self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/Pexp.sp2"); 
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->s.renderfx |= RF_TRANSLUCENT;
	self->think = d_plasma_explode;  
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;
	gi.linkentity (self);
}



void d_fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*plasma;
	trace_t	tr;

	VectorNormalize (dir);

	plasma = G_Spawn();
	VectorCopy (start, plasma->s.origin);
	VectorCopy (start, plasma->s.old_origin);
	vectoangles (dir, plasma->s.angles);
	VectorScale (dir, speed, plasma->velocity);
	plasma->svflags = SVF_DEADMONSTER;
	plasma->movetype = MOVETYPE_FLYMISSILE;
	plasma->clipmask = MASK_SHOT;
	plasma->solid = SOLID_BBOX;
	plasma->s.effects |= (EF_BLUEHYPERBLASTER|EF_ANIM_ALLFAST); 

	VectorClear (plasma->mins);
	VectorClear (plasma->maxs);
	plasma->s.modelindex = gi.modelindex ("sprites/plasma.sp2"); 
	plasma->owner = self;
	plasma->touch = plasma_touch;
	plasma->nextthink = level.time + 8000/speed;
	plasma->think = G_FreeEdict;
	plasma->dmg = damage;
	plasma->classname = "plasma";
	
	gi.linkentity (plasma);

	if (self->client)
		check_dodge (self, plasma->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, plasma->s.origin, plasma, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (plasma->s.origin, -12, dir, plasma->s.origin);
		plasma->touch (plasma, tr.ent, NULL, NULL);
	}
}
 

//////////////////////////
//////////////////////////
// DOOM ROCKET
//////////////////////////
//////////////////////////    

void d_rocket_explode (edict_t *self)
{
	if (self->s.frame == 2)
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}


void d_rocket_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	VectorMA (self->s.origin, -0.02, self->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, MOD_DOOM_RL);
	}


	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_DOOM_RL_SPLASH);
	gi.sound (self, CHAN_AUTO, gi.soundindex ("dweap/rockexp.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// explosion sprite 
	gi.unlinkentity(self);		
	
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy(origin,self->s.origin);
	VectorCopy(origin,self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/Drexp.sp2"); 
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->s.effects |= EF_FLAG1;


	self->think = d_rocket_explode;  
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

void d_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_HYPERBLASTER;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/drocket/tris.md2");
	rocket->owner = self;
	rocket->touch = d_rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}




/*
//===================================
//===================================
Doom fire_bfg
//===================================
//===================================
*/

qboolean CanDamage (edict_t *targ, edict_t *inflictor);

void d_bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;
		
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_DOOM_BFG_BLAST);

//T_RadiusDamage(self, self->owner, 200, other, 100, MOD_DOOM_BFG_BLAST);
//void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)

	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL)
	{
		if (ent == other)
			continue;
		if (!ent->takedamage)
			continue;
		if (ent==self->owner)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (self->s.origin, v, v);
		points = 200 - 0.5 * VectorLength (v);
		
		if (points > 0)
		{
			if (CanDamage (ent, self) && CanDamage(ent,self->owner))
			{
				VectorSubtract (ent->s.origin, self->s.origin, dir);
				T_Damage (ent, self, self->owner, dir, self->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_DOOM_BFG_BLAST);
			}
		}
	}


	gi.sound (self, CHAN_VOICE, gi.soundindex ("dweap/bfgexp.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}



void d_bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && 
				!(tr.ent->flags & FL_IMMUNE_LASER) && 
				(tr.ent != self->owner))
			{
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_DOOM_BFG_LASER);
			}

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}
	}

	self->nextthink = level.time + FRAMETIME;
}



void d_fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*doombfg;

	doombfg = G_Spawn();
	VectorCopy (start, doombfg->s.origin);
	VectorCopy (dir, doombfg->movedir);
	vectoangles (dir, doombfg->s.angles);
	VectorScale (dir, speed, doombfg->velocity);
	doombfg->movetype = MOVETYPE_FLYMISSILE;
	doombfg->clipmask = MASK_SHOT;
	doombfg->solid = SOLID_BBOX;
	doombfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (doombfg->mins);
	VectorClear (doombfg->maxs);
	doombfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	doombfg->owner = self;
	doombfg->touch = d_bfg_touch;
	doombfg->nextthink = level.time + 8000/speed;
	doombfg->think = G_FreeEdict;
	doombfg->radius_dmg = damage;
	doombfg->dmg_radius = damage_radius;
	doombfg->classname = "bfg blast";
	doombfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

//	doombfg->think = d_bfg_think;
//	doombfg->nextthink = level.time + FRAMETIME;
	doombfg->teammaster = doombfg;
	doombfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, doombfg->s.origin, dir, speed);

	gi.linkentity (doombfg);
}


//===================================
//===================================
//Wolfy Bazooka
//===================================
//===================================

void w_rocket_explode (edict_t *self)
{
	if (self->s.frame == 4)
	{
		G_FreeEdict(self);
		return;
	}
	
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

void w_rocket_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	VectorMA (self->s.origin, -0.04, self->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, 0, MOD_WOLF_RL);
	}

	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_WOLF_RL_SPLASH);
	gi.sound (self, CHAN_AUTO, gi.soundindex ("wweap/explode.wav"), 1.0, ATTN_NORM, 0);	

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// explosion sprite 
	gi.unlinkentity(self);		
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy(origin,self->s.origin);
	VectorCopy(origin,self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/wrexp.sp2"); 
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST; 
	self->s.effects |= EF_FLAG1;
	self->think = w_rocket_explode;  
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}

void w_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.modelindex = gi.modelindex ("models/objects/wrocket/tris.md2");
	rocket->owner = self;
	rocket->touch = w_rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*
======================================
Wolfy Flamethrower
======================================
*/


//explode anim
void w_flame_explode (edict_t *self)
{
	if (self->s.frame == 2)
	{
		G_FreeEdict(self);
		return;
	}
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
}
 

void flame_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t origin;
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	VectorMA (self->s.origin, -0.02, self->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_WOLF_FLAME);
	}
	T_RadiusDamage(self, self->owner, 40, other, 40, MOD_WOLF_FLAMEBURN);
	
	gi.sound (self, CHAN_VOICE, gi.soundindex ("wweap/fhit.wav"), 1, ATTN_NORM, 0);
	
	gi.unlinkentity(self);		
	
	// explosion sprite 
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorCopy(origin,self->s.origin);
	VectorCopy(origin,self->s.old_origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/wfexp.sp2"); 
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects = EF_FLAG1;
	self->s.renderfx = RF_TRANSLUCENT;
	self->think = w_flame_explode;  
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;
	gi.linkentity (self);
}


void Flame_think(edict_t *self)
{
	if(self->s.frame >= 6)
	{
		G_FreeEdict(self);
		return;
	}
	if(self->waterlevel > 1)
	{
		G_FreeEdict(self);
		return;
	}
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;
	T_RadiusDamage(self, self->owner, 32, self->owner, 40, MOD_WOLF_FLAMEBURN);
}



void w_fire_flame (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*flame;
	trace_t	tr;

	VectorNormalize (dir);

	flame = G_Spawn();
	VectorCopy (start, flame->s.origin);
	VectorCopy (start, flame->s.old_origin);
	vectoangles (dir, flame->s.angles);
	VectorScale (dir, speed, flame->velocity);
	flame->svflags = SVF_DEADMONSTER;
	flame->movetype = MOVETYPE_FLYMISSILE;
	flame->clipmask = MASK_SHOT;
	flame->solid = SOLID_BBOX;
	flame->s.effects = EF_FLAG1;
	flame->s.renderfx = RF_TRANSLUCENT;

	VectorClear (flame->mins);
	VectorClear (flame->maxs);
//	VectorSet (flame->mins, -16, -16, -16);
//	VectorSet (flame->maxs, 16, 16, 16);

	flame->s.modelindex = gi.modelindex ("sprites/wflame.sp2");
	flame->owner = self;
	flame->touch = flame_touch;
	flame->nextthink = level.time + FRAMETIME;
	flame->think = Flame_think;
	flame->dmg = damage;
	flame->classname = "flame";
	
	gi.linkentity (flame);

	if (self->client)
		check_dodge (self, flame->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, flame->s.origin, flame, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (flame->s.origin, -12, dir, flame->s.origin);
		flame->touch (flame, tr.ent, NULL, NULL);
	}
}


/*
=============
q1_fire_axe
=============
*/

void w_fire_knife ( edict_t *self, vec3_t start, vec3_t dir, int damage)
{    
    trace_t tr; 
    vec3_t end;
	
    VectorMA (start, 32 , dir, end);  
	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
    
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
               T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, damage, 0, MOD_WOLF_KNIFE);
			   if (!(tr.ent->svflags & SVF_MONSTER) && tr.ent->client)
			   {
				   if(tr.ent->client->resp.player_class == CLASS_Q1)
				   {
				   		tr.ent->pain_debounce_time = level.time;
						gi.sound (tr.ent, CHAN_VOICE, gi.soundindex("q1weap/axe/axhit1.wav") , 1.0 , ATTN_NORM, 0);
				   }
			   }
				   gi.WriteByte (svc_temp_entity);
				   gi.WriteByte (TE_MOREBLOOD);
				   gi.WritePosition (tr.endpos);
				   gi.WriteDir (tr.plane.normal);
				   gi.multicast (tr.endpos, MULTICAST_PVS);
            }        
            else
			{
				tr.endpos[2] +=4;

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SHOTGUN);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				gi.sound (self, CHAN_AUTO, gi.soundindex("q1weap/axe/axhit2.wav") , 0.8 , ATTN_NORM, 0);
			}
        }
    }
    return;
}  

