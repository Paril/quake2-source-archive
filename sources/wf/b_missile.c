#include "g_local.h"
#include "m_player.h"
#define newTurret self->turret1
#define newTurret2 self->turret1->turret2


void BecomeExplosion1 (edict_t *self);

// WF & CCH: New think function for homing missiles

/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void mhoming_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
	vec_t   speed;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
		{
			//The only non-client to track is turret grenades and sentry guns
			if ( (strcmp(blip->classname, "turret") != 0) &&
				 (strcmp(blip->classname, "SentryGun") != 0)&&
				 (strcmp(blip->classname, "MissileTurret") != 0))
			{
				continue;
			}
		}
		if (strcmp(blip->classname, "hook") == 0 )
			continue;	//not a grapple 5/99 
		if (blip == ent->creator)
			continue;

		if (blip->disguised)
			continue;
		//dont aim at same team unless friendly fire is on
  	    if ((blip->wf_team == ent->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}
                    
	if (target != NULL)
	{
		 // target acquired, nudge our direction toward it
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.28, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);

		//is this the first time we locked in? sound warning for the target
		if (ent->homing_lock == 0)
		{
			gi.sound (target, CHAN_AUTO, gi.soundindex ("homelock.wav"), 1, ATTN_NORM, 0);
//			gi.sound (target, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
			ent->homing_lock = 1;
		}
	}
     
	//ent->nextthink = level.time + .1;
}

void fire_turretrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->creator = self;
	rocket->owner = self;
	rocket->touch = rocket_touch;

//WF & CCH - Add homing lock status
//	rocket->nextthink = level.time + 8000/speed;
//	rocket->think = G_FreeEdict;

	//Set the team of the rocket
	rocket->wf_team = self->wf_team;
	
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;

	rocket->nextthink = level.time + .1;
	rocket->think = mhoming_think;

    //reduce damage of homing rockets to 1/2 of normal rocket
    rocket->dmg = damage / 2;
//  rocket->radius_dmg = radius_damage / 2;


//WF

	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	gi.linkentity (rocket);
}


void MTurret_Think(edict_t *self)
{
	edict_t *blip;
	float dist;
	vec3_t v;
	vec3_t start,forward;
	trace_t		tr;
	int max;
	blip = NULL;
	
	//gi.error("john");

	if(self->enemy)
	{
		if(self->enemy->health<=0)
			self->enemy= NULL;
	}
	if (self->light_level!=self->gib_health)
	{
		if (self->PlasmaDelay<level.time)
		{
			max = self->gib_health;
				
			if (self->count==3)
				self->PlasmaDelay = level.time +1;
			else if (self->count==2)
				self->PlasmaDelay = level.time +2;
			else
				self->PlasmaDelay = level.time +3;
			self->light_level++;
			if (self->light_level > max)
				self->light_level = max;
			gi.sound(self, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
		}
	}

	if (self->count == 1)
	{
		if(self->delay<level.time)
		{
			if(self->light_level)
				self->s.frame=1;
			else
				self->s.frame=0;
		}
		else
			self->s.frame=0;
		if(!self->enemy)
		{
			while (blip = findradius (blip, self->s.origin, 1024))
			{
		
				//shoot monsters, decoys or players
				if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				{
					//allow it to shoot decoys
					if (strcmp(blip->classname, "decoy") )
						continue;	//not a decoy
				}
		        if (strcmp(blip->classname, "hook") == 0 )
				    continue;	//not a grapple 5/99
				if (blip->health <= 0)
					continue;
				if (blip == self->creator)
					continue;
				if (blip->disguised)
					continue;
		  	    if ((blip->wf_team == self->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
					continue;
				tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
				if (tr.fraction != 1.0)
					continue;
				VectorSubtract (self->s.origin, blip->s.origin, v);
			
				dist = VectorLength(v);

				if (!visible(self, blip) && dist > 400)
					continue;
			
				self->enemy = blip;
			}
		}
		else
		{
			VectorSubtract (self->enemy->s.origin, self->s.origin, v);
		
			self->ideal_yaw = vectoyaw(v);
		
		
			M_ChangeYaw(self);
		
			if (self->light_level>0)
			{
				if ((visible(self, self->enemy))&& (self->delay<level.time))
				{
					AngleVectors (self->s.angles, forward, NULL, NULL);
					start[0] = self->s.origin[0] + forward[0] * 25;
					start[1] = self->s.origin[1] + forward[1] * 25;
					start[2] = self->s.origin[2] + forward[2] * 25+10;


//					fire_turretrocket (self, start, forward, 30, 700, 150, 50);
					fire_rocket (self, start, forward, 30, 750, 150, 50, MOD_MISSILE);
					self->light_level-=1;
					self->delay = level.time + 1.0;
				}
				else
				{	
					self->enemy=NULL;
				}
			}
			else
			{
				self->enemy=NULL;
			}
		}
	}

	else if (self->count == 2)
	{
	
		self->s.frame = self->light_level;
		if(!self->enemy)
		{
			while (blip = findradius (blip, self->s.origin, 2048))
			{
		
				//shoot monsters, decoys or players
				if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				{
					//allow it to shoot decoys
					if (strcmp(blip->classname, "decoy") )
						continue;	//not a decoy
				}
		        if (strcmp(blip->classname, "hook") == 0 )
				    continue;	//not a grapple 5/99
				if (blip->health <= 0)
					continue;
				if (blip == self->creator)
					continue;
				if (blip->disguised)
					continue;
		  	    if ((blip->wf_team == self->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
					continue;
				tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
				if (tr.fraction != 1.0)
					continue;
				VectorSubtract (self->s.origin, blip->s.origin, v);
			
				dist = VectorLength(v);

				if (!visible(self, blip) && dist > 800)
					continue;
			
				self->enemy = blip;
			}
		}
		else
		{
			VectorSubtract (self->enemy->s.origin, self->s.origin, v);
		
			self->ideal_yaw = vectoyaw(v);
		
		
			M_ChangeYaw(self);
		
			if (self->light_level>0)
			{
				if ((visible(self, self->enemy))&&(self->delay<level.time))
				{

					AngleVectors (self->s.angles, forward, NULL, NULL);
					start[0] = self->s.origin[0] + forward[0] * 25;
					start[1] = self->s.origin[1] + forward[1] * 25;
					start[2] = self->s.origin[2] + forward[2] * 25+10;


//					fire_turretrocket (self, start, forward, 30, 700, 150, 50);
					fire_rocket (self, start, forward, 30, 700, 150, 50, MOD_ROCKET);
					self->light_level-=1;
					self->delay = level.time + 0.3;
					
				
				}
				else
				{
					self->enemy=NULL;
				}
			}
			else
			{
				self->enemy=NULL;
			}
		}
	}
	else if (self->count == 3)
	{
		self->s.frame =self->light_level;
		if(!self->enemy)
		{

			while (blip = findradius (blip, self->s.origin, 2048))
			{
		
				//shoot monsters, decoys or players
				if (!(blip->svflags & SVF_MONSTER) && !blip->client)
				{
					//allow it to shoot decoys
					if (strcmp(blip->classname, "decoy") )
						continue;	//not a decoy
				}
		        if (strcmp(blip->classname, "hook") == 0 )
				    continue;	//not a grapple 5/99
				if (blip->health <= 0)
					continue;
				if (blip == self->creator)
					continue;
				if (blip->disguised)
					continue;
		  	    if ((blip->wf_team == self->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
					continue;
				tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
				if (tr.fraction != 1.0)
					continue;
				VectorSubtract (self->s.origin, blip->s.origin, v);
			
				dist = VectorLength(v);

				if (!visible(self, blip) && dist > 1400)
					continue;
			
				self->enemy = blip;
			}
		}
		else
		{
			VectorSubtract (self->enemy->s.origin, self->s.origin, v);
		
			self->ideal_yaw = vectoyaw(v);
		
		
			M_ChangeYaw(self);
		
			if (self->light_level>0)
			{
				if ((visible(self, self->enemy))&&(self->delay<level.time))
				{
					AngleVectors (self->s.angles, forward, NULL, NULL);
					start[0] = self->s.origin[0] + forward[0] * 25;
					start[1] = self->s.origin[1] + forward[1] * 25;
					start[2] = self->s.origin[2] + forward[2] * 25+10;


//					fire_turretrocket (self, start, forward, 30, 700,  150, 50);
					fire_rocket (self, start, forward, 30, 700, 150, 50, MOD_ROCKET);
					self->light_level-=1;
					self->delay = level.time + 0.1;
				}
			
				else
				{

					self->enemy=NULL;
				}
			}
			else
			{

				self->enemy = NULL;
			}
		}
	}
	VectorCopy(self->s.origin,self->missile->s.origin);
	self->nextthink = level.time + 0.1;
	if(self->health<self->max_health)
		self->health++;
}	

//self = mturret entity
void mturret_remove(edict_t *self)
{
	if (self->missile)
	{
		G_FreeEdict(self->missile);
	}

	//Clear client's pointer to missile gun
	if (self->creator)
		self->creator->missile = NULL;

	G_FreeEdict (self);
}

void mturret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t origin;	
	VectorCopy (self->s.origin,origin);
	origin[2]+= 0.5;
	self->takedamage = DAMAGE_NO;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (origin);
	gi.multicast (origin, MULTICAST_PVS);
	//G_FreeEdict(self->turret2);

	//Remove stand (which is the missile of the missile gun)
	mturret_remove(self);
	gi.bprintf (PRINT_HIGH, "Missile Gun Destroyed.\n"); 
}

void place_missile (edict_t *ent)
{
	
	vec3_t		forward,up,right,wallp, pos;

	trace_t		tr;
	edict_t *missilestand;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	if (ent->missile) 
	{  
		//First free the stand
		if (ent->missile->missile)
		{
			G_FreeEdict(ent->missile->missile);
			ent->missile->missile = NULL;
		}

		//Then free the missile gun
		G_FreeEdict(ent->missile);    
		ent->missile = NULL;
		gi.bprintf (PRINT_HIGH, "Missile Turret off.\n"); 
		if (ent->client->oldplayer)
			G_FreeEdict(ent->client->oldplayer);
		return; 
	}

	// cells for missile gun ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 25)
	{
 		safe_cprintf(ent, PRINT_HIGH, "You need 25 cells to create Missile Turret\n");
		return;
	}

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);         

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, right, up);

	// Setup end point
	pos[0]=ent->s.origin[0]+forward[0]*75;
	pos[1]=ent->s.origin[1]+forward[1]*75;
	pos[2]=ent->s.origin[2]+forward[2]*75+30;  
	wallp[0]=ent->s.origin[0]+forward[0]*150;
	wallp[1]=ent->s.origin[1]+forward[1]*150;
	wallp[2]=ent->s.origin[2]+forward[2]*150+30; 
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
		
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	wallp[2]+=22;
	tr = gi.trace (pos, NULL, NULL, wallp, ent, MASK_SOLID);
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room. Try aiming lower\n");
		return;
	}
	wallp[2]-=40;
		tr = gi.trace (pos, NULL, NULL, wallp, ent, MASK_SOLID);
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room. Try aiming higher\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 25;


//		if (ent->missile) 
//		{  
//			G_FreeEdict(ent->missile);    
//			ent->missile = NULL;
//			gi.bprintf (PRINT_HIGH, "missile Gun off.\n"); 
//			return; 
//		}
	gi.bprintf (PRINT_HIGH, "Missile Turret on.\n");
	ent->missile = G_Spawn();
	VectorClear (ent->missile->mins);
	VectorClear (ent->missile->maxs);
	VectorCopy (pos, ent->missile->s.origin);
	ent->missile ->s.angles[0]=ent->s.angles[0];
//	ent->missile -> movetype		= MOVETYPE_STEP;
//	ent->missile -> clipmask		= MASK_SHOT;
	ent->missile -> movetype		= MOVETYPE_STEP;
	ent->missile -> clipmask		= MASK_PLAYERSOLID;
	ent->missile->mass = 400;
	//grenade -> solid		= SOLID_NOT;
	ent->missile->solid = SOLID_BBOX;
//GR - reduce size of bounding box since I reduced size of model
//	VectorSet(ent->missile->mins, -95,-44,-80);
//	VectorSet(ent->missile->maxs, 57, 41, 24);
	VectorSet(ent->missile->mins, -50,-20,-40);
	VectorSet(ent->missile->maxs, 30, 21, 22);
	ent->missile->takedamage=DAMAGE_YES;
	ent->missile -> s.modelindex	= gi.modelindex ("models/missileb/missile1/tris.md2");
	ent->missile -> creator = ent;	
	ent->missile->think = MTurret_Think;
	ent->missile->nextthink = level.time + 0.1;
	ent->missile->die = mturret_die;
	ent->missile->health= 100;
	ent->missile->max_health =100;
	ent->missile->count = 1;
	ent->missile->classname = "MissileTurret";
	ent->missile->wf_team = ent->wf_team;
	ent->missile->noteamdamage = true;	//Don't let teammates damage it
	ent->missile->yaw_speed = 10;
	ent->missile->gib_health = 3;//Max Ammo
	ent->missile->light_level = 3;//Ammo Total
		
	gi.linkentity (ent->missile);
	missilestand = G_Spawn();
	VectorClear (missilestand->mins);
	VectorClear (missilestand->maxs);
	VectorCopy (pos,missilestand->s.origin);
	missilestand->s.angles[0]=ent -> s.angles[0];
	missilestand->movetype		= MOVETYPE_NONE;
	missilestand->mass = 400;
	//grenade -> solid		= SOLID_NOT;
	missilestand->solid = SOLID_BBOX;
	VectorSet(missilestand->mins, -45,-15,-35);
	VectorSet(missilestand->maxs, 25, 18, 18);
	missilestand->takedamage=DAMAGE_NO;
	missilestand-> s.modelindex	= gi.modelindex ("models/stand/tris.md2");
	missilestand-> creator = ent->missile;
	missilestand->wf_team = ent->wf_team;
	missilestand->noteamdamage = true;	//Don't let teammates damage it
		
	gi.linkentity (missilestand);
	ent->missile->missile = missilestand;
}

void UpgradeMissileTurret(edict_t *self)
{
	edict_t *blip;
	trace_t		tr;
	blip = NULL;
	while (blip = findradius (blip, self->s.origin, 2048))
	{

		if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 60)
		{
 			safe_cprintf(self, PRINT_HIGH, "You need 60 cells to upgrade\n");
			return;
		}

		if (Q_stricmp("MissileTurret", blip->classname))
			continue;

		if (Q_stricmp("MissileTurret", blip->classname))
			continue;

		tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;
		if (blip->creator != self)
			continue;
		if (blip->count == 3)
 			safe_cprintf(self, PRINT_HIGH, "missile gun already at level 3\n");

		if (blip->count < 3)
			blip->count++;
		self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 60;
		if (blip->count == 2)
		{
			blip->health= 150;
			blip->max_health =150;
			blip -> s.modelindex	= gi.modelindex ("models/missileb/missile2/tris.md2");
			//blip->delay = StatusIdle;
			blip->s.frame = 0;
			blip->yaw_speed = 15;
			blip->gib_health = 4;//Max Ammo
		}
		if (blip->count == 3)
		{
			blip->health= 200;
			blip->max_health =200;
			blip -> s.modelindex	= gi.modelindex ("models/missileb/missile3/tris.md2");
			//blip->delay = StatusIdle;
			blip->s.frame = 0;			
			blip->yaw_speed = 20;
			blip->gib_health = 12;//Max Ammo
		}
	}
}