#include "g_local.h"
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
	}
}
static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}
static void rocket_spiral_think(edict_t *ent)
{
	vec3_t newpos, pos;

	VectorSubtract(ent->s.origin, ent->pos2, pos); 
	RotatePointAroundVector(newpos, ent->pos1, pos, 30);
	VectorAdd(ent->pos2, newpos, ent->s.origin);
	RotatePointAroundVector(ent->velocity, ent->pos1, ent->velocity, 30); 

	ent->nextthink = level.time + FRAMETIME; 
}

void fire_spiral_rocket (edict_t *self, vec3_t start, vec3_t dir, vec3_t pos1, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (pos1, rocket->pos1);
	VectorCopy (start, rocket->pos2); 
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
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + FRAMETIME;
	rocket->think = rocket_spiral_think;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void fire_streak (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod)//fast blaster effectively. looks good. that's it tho.
//it can't hurt people but i can't get the model to go through. also fucking laggy in net play.
//streakoff 1/0 to turn on / off
{
	edict_t	*bolt;
	trace_t	tr;
	int	speed = 3000;
	if (streakoff->value)
		return;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;

	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;

	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/streak/tris.md2");
	bolt->s.effects |= EF_SPHERETRANS;
	bolt->s.renderfx = RF_FULLBRIGHT;
	bolt->owner = self;
	
	bolt->touch = matrix_streak_touch;
	bolt->nextthink = level.time + 0.6;
	bolt->think = G_FreeEdict;
	bolt->dmg = 0;
	bolt->classname = "bullet";
	bolt->s.sound = gi.soundindex ("weapons/swish.wav");
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

void matrix_streak_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)//destroy streak if it hits anything.
{
	if (other == self->owner)
		return;

	G_FreeEdict (self);
}
void Matrix_SpawnDeadBullet (edict_t *self, vec3_t dir, edict_t *target)
{
	trace_t tr;
	edict_t	*bolt;
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	vec3_t	end;
	vec3_t	blag;


	int	speed = 1;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("buletstp.wav"), 1, ATTN_NORM, 0);
	VectorSubtract(target->s.origin, self->s.origin, forward);
	VectorMA(forward, 0.6, self->s.origin, end);
	VectorNormalize(forward);

	if(!target->velocity[0] || !target->velocity[1])
	SpawnShadow(target); //if standing still dodge  effect
	
		
	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (end, bolt->s.origin);
	VectorCopy (end, bolt->s.old_origin);
	VectorScale (forward, speed, bolt->velocity);
	vectoangles (forward, bolt->s.angles);
	//bolt->velocity[2] = -80;
	bolt->movetype = MOVETYPE_BOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/shell1/tris.md2");
	bolt->nextthink = level.time + 1.2;
	bolt->think = G_FreeEdict;
	bolt->classname = "bolt";
	
	gi.linkentity (bolt);

	//if (self->client)
	//	check_dodge (self, bolt->s.origin, dir, speed);
	

}
void matrix_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocketa;
	edict_t	*rocketb;
	edict_t	*rocketc;
	vec3_t	dir2;
	float	temp = -1;

	while (temp<0)
	temp = random()*1 + 1;

	rocketa = G_Spawn();
	
	rocketa->randomrocket = temp;
	VectorCopy (self->client->v_angle, dir2);
	dir2[PITCH] = self->client->v_angle[PITCH] + 8;
	AngleVectors (dir2, dir, NULL, NULL);

	VectorCopy (start, rocketa->s.origin);
	VectorCopy (dir, rocketa->movedir);
	VectorCopy(rocketa->s.origin, rocketa->s.old_origin);
	vectoangles (dir, rocketa->s.angles);
	VectorScale (dir, speed, rocketa->velocity);
	rocketa->movetype = MOVETYPE_MATRIXROCKET;
	rocketa->clipmask = MASK_SHOT;
	rocketa->solid = SOLID_BBOX;
	//rocketa->s.effects |= EF_TAGTRAIL;
	rocketa->s.effects |= EF_GRENADE;
	VectorClear (rocketa->mins);
	VectorClear (rocketa->maxs);
	rocketa->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocketa->owner = self;
	rocketa->touch = rocket_touch;
	rocketa->nextthink = level.time + 8000/speed;
	rocketa->think = G_FreeEdict;
	rocketa->dmg = damage;
	rocketa->radius_dmg = radius_damage;
	rocketa->dmg_radius = damage_radius;
	rocketa->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocketa->classname = "rocket";
	
	if (self->client)
		check_dodge (self, rocketa->s.origin, dir, speed);

	////////////////////////////////////////////////////////
	
	

	rocketb = G_Spawn();
	rocketb->randomrocket = temp;
	
	dir2[PITCH] = self->client->v_angle[PITCH] - 6;
	dir2[YAW] = self->client->v_angle[YAW] - 6;
	AngleVectors (dir2, dir, NULL, NULL);

	VectorCopy (start, rocketb->s.origin);
	VectorCopy (dir, rocketb->movedir);
	VectorCopy(rocketb->s.origin, rocketb->s.old_origin);
	vectoangles (dir, rocketb->s.angles);
	VectorScale (dir, speed, rocketb->velocity);
	rocketb->movetype = MOVETYPE_MATRIXROCKET;
	rocketb->clipmask = MASK_SHOT;
	rocketb->solid = SOLID_BBOX;
	//rocketb->s.effects |= EF_FLAG1;
	rocketb->s.effects |= EF_GRENADE;
	VectorClear (rocketb->mins);
	VectorClear (rocketb->maxs);
	rocketb->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocketb->owner = self;
	rocketb->touch = rocket_touch;
	rocketb->nextthink = level.time + 8000/speed;
	rocketb->think = G_FreeEdict;
	rocketb->dmg = damage;
	rocketb->radius_dmg = radius_damage;
	rocketb->dmg_radius = damage_radius;
	rocketb->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocketb->classname = "rocket";
	
	if (self->client)
		check_dodge (self, rocketb->s.origin, dir, speed);
	
	/////////////////////////////////////////////////
	rocketc = G_Spawn();
	
	rocketc->randomrocket = temp;
	dir2[PITCH] = self->client->v_angle[PITCH] - 6;
	dir2[YAW] = self->client->v_angle[YAW] + 6;
	AngleVectors (dir2, dir, NULL, NULL);

	VectorCopy (start, rocketc->s.origin);
	VectorCopy(rocketc->s.origin, rocketc->s.old_origin);
	VectorCopy (dir, rocketc->movedir);
	vectoangles (dir, rocketc->s.angles);
	VectorScale (dir, speed, rocketc->velocity);
	rocketc->movetype = MOVETYPE_MATRIXROCKET;
	rocketc->clipmask = MASK_SHOT;
	rocketc->solid = SOLID_BBOX;
	//rocketc->s.effects |= EF_FLAG2;
	rocketc->s.effects |= EF_GRENADE;
	VectorClear (rocketc->mins);
	VectorClear (rocketc->maxs);
	rocketc->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocketc->owner = self;
	rocketc->touch = rocket_touch;
	rocketc->nextthink = level.time + 8000/speed;
	rocketc->think = G_FreeEdict;
	rocketc->dmg = damage;
	rocketc->radius_dmg = radius_damage;
	rocketc->dmg_radius = damage_radius;
	rocketc->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocketc->classname = "rocket";
	
	if (self->client)
		check_dodge (self, rocketa->s.origin, dir, speed);

	rocketb->target_ent = rocketc;
	rocketa->target_ent = rocketb;
	rocketc->target_ent = rocketa;

	gi.linkentity (rocketb);
	gi.linkentity (rocketa);
	gi.linkentity (rocketc);
}


void VectorM (float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = scale*vecb[0];
	vecc[1] = scale*vecb[1];
	vecc[2] = scale*vecb[2];
}

void Matrix_Rocket_Think (edict_t *rocket)
{
	vec3_t	accel, accel2;
	
	VectorCopy(rocket->s.origin, rocket->s.old_origin);

	VectorSubtract(rocket->s.origin, rocket->target_ent->s.origin, accel);
	if(accel[0] > 300 || accel[0] < -300
	|| accel[1] > 300 || accel[1] < -300
	|| accel[2] > 300 || accel[2] < -300)
		return;

	VectorM(-rocket->randomrocket, accel, accel);
	VectorAdd(accel, rocket->velocity, rocket->velocity);
	

	
	
	
}

void KickBack (edict_t *ent, vec3_t dir, int kick)
{

	vec3_t	kickback;
	//ducking stops you moving
	if(ent->vertbuttons<0 && ent->groundentity)
	{
	ent->client->kick_angles[0] -= kick;
	return;
	}

	if(ent->groundentity)
	kick *= 60;
	else
	kick *= 40;
	VectorInverse(dir);
	VectorScale(dir, kick, dir);
	VectorAdd(dir, ent->velocity, ent->velocity);
	
	
	
	
}

	
void Matrix_KnifePin (edict_t *other, edict_t *attacker, vec3_t end)
{

	other->pinned_nextthink = level.framenum + 15;
	VectorCopy(end, other->pinnedpos);
	
	if(other->client)
	gi.centerprintf (other, "%s pinned your ass to the wall.\n", attacker->client->pers.netname);

}

void knife_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t          origin, dir, end;
        //      int                     n;
        
        edict_t         *dropped;
        edict_t         *knife;
                //      vec3_t          forward, right, up;
        vec3_t          move_angles;
        gitem_t         *item;
        trace_t			tr;
        
        if (other == ent->owner)
                return;
        
        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (ent);
                return;
        }
        
        if (ent->owner->client)
        {
                gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/clank.wav"), 1, ATTN_NORM, 0);
                PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
        }
        
        // calculate position for the explosion entity
        VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
        
//glass fx
        if (0 == Q_stricmp(other->classname, "func_explosive"))
          {
            // ignore it, so it can bounce
            return;
          }
        else
// ---
        if (other->takedamage)
        {
                T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, VectorLength (ent->velocity)/5, 1, MOD_KNIFE);
				VectorCopy(ent->velocity, dir);
				VectorNormalize(dir);
				VectorMA(dir, VectorLength(ent->velocity) , ent->s.origin, end);
				tr = gi.trace(ent->s.origin, vec3_origin, vec3_origin, end, ent, MASK_SOLID);
				VectorMA(dir, VectorLength(ent->velocity) * tr.fraction, ent->s.origin, end);

				if(tr.fraction<1)
					Matrix_KnifePin(other, ent->owner, end);

        }
        else
        {
            
                                                                        
                        dropped = G_Spawn();
                        item = FindItem("Gung Ho Knives");
                        dropped->classname = item->classname;
                        dropped->item = item;
                        dropped->spawnflags = DROPPED_ITEM;
                        dropped->s.effects = 0;
                        dropped->s.renderfx = RF_GLOW;
                        VectorSet (dropped->mins, -15, -15, -15);
                        VectorSet (dropped->maxs, 15, 15, 15);
                        gi.setmodel (dropped, item->world_model);
                        dropped->solid = SOLID_TRIGGER;
                        dropped->movetype = MOVETYPE_TOSS;  
                        dropped->touch = Touch_Item;
                        dropped->owner = ent;
                        dropped->gravity = 0;
                        
                        
                        vectoangles (ent->velocity, move_angles);
                        //AngleVectors (ent->s.angles, forward, right, up);
                        VectorCopy (ent->s.origin, dropped->s.origin);
                        VectorCopy (move_angles, dropped->s.angles);
                        //VectorScale (forward, 100, dropped->velocity);
                        //dropped->velocity[2] = 300;
                        
                        //dropped->think = drop_make_touchable;
                        //dropped->nextthink = level.time + 1;
                        
                        dropped->nextthink = level.time + 100;
                        dropped->think = G_FreeEdict;
                        
                        gi.linkentity (dropped);
                        
                        
                        if ( !(ent->waterlevel) )
                        {
                       
                                gi.WriteByte (svc_temp_entity);    
                                gi.WriteByte (TE_SPARKS);
                                gi.WritePosition (origin);    
                                gi.WriteDir (plane->normal);
                                gi.multicast (ent->s.origin, MULTICAST_PVS);
                        }
                        
        }
        G_FreeEdict (ent);
}

void knife_throw (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed )
{
        edict_t *knife;
//        vec3_t          forward, right, up;
                trace_t tr;

        knife = G_Spawn();

        VectorNormalize (dir);
        VectorCopy (start, knife->s.origin);
        VectorCopy (start, knife->s.old_origin);
        vectoangles (dir, knife->s.angles);
        VectorScale (dir, speed, knife->velocity);
        knife->movetype = MOVETYPE_TOSS;
                

        VectorSet (knife->avelocity, 1200, 0, 0);
        
        knife->movetype = MOVETYPE_TOSS;
        knife->clipmask = MASK_SHOT;
        knife->solid = SOLID_BBOX;
        knife->s.effects = 0; //EF_ROTATE?
        VectorClear (knife->mins);
        VectorClear (knife->maxs);
        knife->s.modelindex = gi.modelindex ("models/objects/knife/tris.md2");
        knife->owner = self;
        knife->touch = knife_touch;
        knife->nextthink = level.time + 8000/speed;
        knife->think = G_FreeEdict;
        knife->dmg = damage;
        knife->s.sound = gi.soundindex ("misc/flyloop.wav");
        knife->classname = "thrown_knife";

// used by dodging monsters, skip
//      if (self->client)
//              check_dodge (self, rocket->s.origin, dir, speed);


        tr = gi.trace (self->s.origin, NULL, NULL, knife->s.origin, knife, MASK_SHOT);

        if (tr.fraction < 1.0)
        {
                VectorMA (knife->s.origin, -10, dir, knife->s.origin);
                knife->touch (knife, tr.ent, NULL, NULL);
        }


        gi.linkentity (knife);
}

void Kuml_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent->owner, ent->owner, vec3_origin, other->s.origin, vec3_origin, ent->dmg, 0, 0, MOD_SHOTGUN);

		G_FreeEdict (ent);
		return;
	}

	ent->enemy = other;
	//Grenade_Explode (ent);
}
//shh! secret!
void fire_kuml2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	vec3_t		temp;
	float		r;
	float		u;
	int		i = 8;
	edict_t	*bolt;
	trace_t	tr;
	int	speed = 500;

	bolt = G_Spawn();

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom()*hspread;
	u = crandom()*vspread;
	VectorMA (start, 8192, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);
	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;

	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_TOSS;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;

	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/debris2/tris.md2");
	bolt->s.effects |= EF_SPHERETRANS;
	bolt->s.renderfx = RF_FULLBRIGHT;
	bolt->s.skinnum = 1;
	bolt->owner = self;
	
	bolt->touch = Kuml_Touch;
	bolt->nextthink = level.time + 3;
	bolt->think = G_FreeEdict;
	bolt->dmg = 15;
	bolt->classname = "poopoo";
	bolt->s.sound = gi.soundindex ("weapons/swish.wav");
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
void fire_kuml (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;
	edict_t	*bolt;
	trace_t	tr;
	int	speed = 500;


	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	vec3_t		temp;
	float		r;
	float		u;

	for (i = 0; i < count; i++)
	{
		void fire_kuml2 (self, start, aimdir, damage,  kick,  hspread,  vspread,  count, mod);
	}
}
