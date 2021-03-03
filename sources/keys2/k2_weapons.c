//Functions for alternative weapons
#include "g_local.h"


//Used for homing lasers and rockets
void K2_HomingInformDanger(edict_t *);
qboolean IsVisible(edict_t *pPlayer1, edict_t *pPlayer2);
qboolean SameTeam(edict_t *pPlayer1, edict_t *pPlayer2);
void K2_HomingThink(edict_t *self)
{
	//self is the projectile
	edict_t		*ent = NULL;

		//If we don't already have an enemy/target, try to find one
		if (!self->enemy)
		{		
			while ((ent = findradius(ent,self->s.origin,2048)) != NULL)
			{
				if (ent == self)
					continue;
				if (ent == self->owner)
					continue;
				if(!ent->takedamage)
					continue;
				if(!ent->client)
					continue;
				if(!ent->inuse)
					continue;
				if(ent->deadflag)
					continue;
				//Don't home in on teammates in CTF
				if (ctf->value)
				{
					if(ent->client->resp.ctf_team == self->owner->client->resp.ctf_team)
						continue;
					if (ent->client->resp.ctf_team == CTF_NOTEAM)
						continue;
				}

				//Dont home in on Anti_key player
				if ( ent->client->key == KEY_ANTIKEY)
					continue;
				//Dont home in on Stealth player
				if (ent->client->key == KEY_STEALTH)
					continue;
						
				//Make sure enemy is visible and infront
				if(visible(self,ent) && infront(self,ent))
				{
					self->enemy = ent;
				}

			}
		}
	

	//Home in
	if(self->enemy)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_GREENSHELL;
		self->think=K2_HomeTarget;
	}

	//Eraser
	K2_HomingInformDanger(self);

	self->nextthink = level.time + FRAMETIME;
	
}

void K2_HomeTarget(edict_t *self)
{
	vec3_t		newdir;
		
	//If enemy is dead or disconnects, or goes underwater, or is not visible anymore,
	//or gets an anti-homing key, stop homing him and try to acquire a new target
	if( self->enemy->deadflag			||
		!visible(self,self->enemy)		||
		(self->enemy->waterlevel >= 3)	||
		K2_IsAnti(self->enemy)			||
		K2_IsStealth(self->enemy)		||
		!self->enemy->inuse)
		
	{
		self->enemy = NULL;
		self->s.effects &= ~EF_COLOR_SHELL;
		self->s.renderfx &= ~RF_GREENSHELL;
		self->think=K2_HomingThink;
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	VectorSubtract(self->enemy->s.origin,self->s.origin,newdir);
	VectorNormalize(newdir);
	VectorCopy (newdir, self->movedir);
	vectoangles (newdir, self->s.angles);
	VectorScale (newdir, self->homing_speed, self->velocity);
	if (self->soundframe < level.framenum)
	{
		if (self->radius_dmg) //must be a rocket
			gi.sound(self,CHAN_AUTO,gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound(self,CHAN_AUTO,gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		self->soundframe = level.framenum + 5;
	}

	//Eraser
	K2_HomingInformDanger(self);

	self->nextthink = level.time + FRAMETIME;
}


//Explode homing rocket
void Rocket_Explode (edict_t *ent)
{
     
	vec3_t          origin;
     
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
     
	 // calculate position for the explosion entity
     VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
     
     T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_HOMING_SPLASH);
     
     gi.WriteByte (svc_temp_entity);
     if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
     else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
        gi.WritePosition (origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
     
        G_FreeEdict (ent);
}
     
//When a rocket 'dies', it blows up next frame
void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
    self->think = Rocket_Explode;
	self->nextthink = level.time + FRAMETIME;
}
     

//Flash Grenade
void Flash_Explode (edict_t *ent)
{
	vec3_t      offset, v;
    edict_t		*target;
	float Distance, BlindTimeAdd;


    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);    
    VectorAdd(ent->s.origin, offset, ent->s.origin);
	
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,MOD_FLASH);

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    target = NULL;
    while ((target = findradius(target, ent->s.origin, flash_radius->value)) != NULL)
    {
		//if (target == ent->owner)
        //  continue;       // You know when to close your eyes, don't you?
        
		if (!target->client)
           continue;       // It's not a player
        if (!visible(ent, target))
           continue;       // The grenade can't see it
        if (!infront(target, ent))
           continue;       // It's not facing it

		if (target->deadflag || !target->inuse)
			continue;

		//Don't blind proected players
		if(K2_IsProtected(target))
			continue;

		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);
		
		// Calculate blindness factor
		if ( Distance < flash_radius->value/20 )
		BlindTimeAdd = (blindtime->value*10); // Blind completely
		else
			BlindTimeAdd = 1.5 * (blindtime->value*10) * ( 1 / ( ( Distance - flash_radius->value*2 ) / (flash_radius->value*2) - 2 ) + 1 ); // Blind partially
     
		if ( BlindTimeAdd < 0 )
			BlindTimeAdd = 0; // Do not blind at all.

		// Not facing it, but still blinded a little
        if (!infront(target, ent))
            BlindTimeAdd *= .75;

		// You know when to close your eyes, don't you? Doesn't quite do the job. :)
		if (target == ent->owner)
		{
			target->client->blindTime += BlindTimeAdd * .5;
			target->client->blindBase = (blindtime->value*10);
			continue;
		}
			
		// Increment the blindness counter
        target->client->blindTime += (blindtime->value*10) * 1.5;
        target->client->blindBase = (blindtime->value*10);
		
		// Let the player know what just happened
        // (It's just as well, he won't see the message immediately!)
        if (!target->bot_client)
			safe_cprintf(target, PRINT_HIGH, 
                   "You are blinded by %s's flash grenade!!!\n",
				   ent->owner->client->pers.netname);

        // Let the owner of the grenade know it worked
        if(target != ent->owner)
			if (!ent->owner->bot_client)
			safe_cprintf(ent->owner, PRINT_HIGH, 
                   "%s is blinded by your flash grenade!\n",
                   target->client->pers.netname);

		//Eraser
		if(target->bot_client)
		{
			target->enemy = NULL;
		}
	}

    // Blow up the grenade
    BecomeExplosion1(ent);
}

void Flash_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
       return;

	// If it goes in to orbit, it's gone...
    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (ent);
        return;
	}

    // All this does is make the bouncing noises when it hits something...
    if (!other->takedamage)
    {
		if (ent->spawnflags & 1)
        {
			if (random() > 0.5)
               gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"),
                         1, ATTN_NORM, 0);
            else
               gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"),
                         1, ATTN_NORM, 0);
        }
        else
            gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"),
                      1, ATTN_NORM, 0);
    
    
		return;
	}
	
	// The ONLY DIFFERENCE between this and "Grenade_Touch"!!
    Flash_Explode (ent);    
}


void fire_rocket_drunk(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket=NULL;
	
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
	rocket->owner = self;
	rocket->touch = rocket_touch;
	
	rocket->think = Rocket_DrunkThink;
	rocket->nextthink = level.time + 0.5;
	rocket->classname = "drunk_rocket";
	rocket->dmg = damage*.75;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

	gi.linkentity (rocket);
}

void Rocket_DrunkThink (edict_t *self)
{
	vec3_t		newdir, offset;
	int			i;

	i = rand() %3;

	VectorSet(offset, 0.5,0,0);
	
	
	if(i == 1)
		VectorAdd(self->movedir, offset, newdir);
	else if (i == 2)
		VectorSubtract(self->movedir, offset, newdir);
	else
		VectorCopy(self->movedir, newdir);
	
	VectorNormalize(newdir);
	VectorCopy (newdir, self->movedir);
	vectoangles (newdir, self->s.angles);
	VectorScale (newdir, 650, self->velocity);
		
	self->nextthink = level.time + FRAMETIME;

	gi.linkentity(self);

}

void K2_Fire_Radius_Explosions(edict_t *ent)
{

	vec3_t	origin[6], offset;
	int		i;
	
	VectorSet(offset,(ent->dmg_radius/2),0,0);
	VectorAdd(ent->s.origin,offset,origin[0]);
	VectorSubtract(ent->s.origin,offset,origin[1]);

	VectorSet(offset,0,(ent->dmg_radius/2),0);
	VectorAdd(ent->s.origin,offset,origin[2]);
	VectorSubtract(ent->s.origin,offset,origin[3]);

	for(i=0;i<4;i++)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (origin[i]);
		gi.multicast (origin[i], MULTICAST_PVS);
	}

}

void K2_BurnPlayer(edict_t *ent, edict_t *attacker)
{
	edict_t	*flame=NULL;

	if(!ent->client)
		return;

	if(!ent->takedamage)
		return;

	if(!ent->inuse)
		return;

	if(ent->deadflag)
		return;

	if(K2_IsProtected(ent))
		return;

	//If frozen, unfreeze, but don't let players unfreeze themselves
	if(ent != attacker)
		ent->client->freezeTime = 0;
	
	ent->client->burntime = level.time + burntime->value;
	
	flame = G_Spawn();
	flame->owner = attacker;
	flame->spawnflags = 1;
	flame->enemy = ent;
	flame->dmg = burndamage->value;
	flame->classname = "flame";
	flame->movetype = MOVETYPE_NONE;

	VectorCopy (ent->s.origin, flame->s.origin);
	flame->solid = SOLID_NOT;
	
	flame->think = K2_FlameBurnDamage;
	flame->nextthink = level.time + 1;
}

void K2_FlameBurnDamage(edict_t *ent)
{
	edict_t	*newent = NULL;
		
	if (!ent->enemy || ent->enemy->deadflag || !ent->enemy->inuse)
	{
		G_FreeEdict(ent);
		return;
	}

	VectorCopy(ent->enemy->s.origin,ent->s.origin);
	gi.linkentity(ent);

	if (ent->enemy->client->burntime > level.time)
	{
		if(ent->enemy->client->burnframe < level.framenum)
		{
		
			//Burn sound
			if (rand()&1)
				gi.sound (ent->enemy, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent->enemy, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
	
			//Do the damage
			T_Damage(ent->enemy,ent,ent->owner,ent->enemy->velocity,ent->s.origin,vec3_origin,ent->dmg,0,0,MOD_FLAME);			
		
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION2);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			ent->enemy->client->burnframe = level.framenum + 10;
		}
		
	
		//If player comes close to another player while burning, start him on fire too :-))
		while ((newent = findradius(newent, ent->s.origin, 64)) != NULL)
				
		{
			if (!newent->client)
				continue;
			
			if (!newent->inuse)
				continue;

			if (newent->deadflag)
				continue;

			if(newent->client->burntime >= level.time)
				continue;
				
			if (!newent->takedamage)
				continue;

			//Not proected players
			if(K2_IsProtected(newent))
				return;

			K2_BurnPlayer(newent,ent->enemy);
			
		}
		
	}
	else
		G_FreeEdict(ent);

	ent->nextthink = level.time + FRAMETIME;
}
		

void K2_FireGibGun(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*gib=NULL;

	gib = G_Spawn();
	gib->clipmask = MASK_SHOT;

	if(K2_IsHoming(self))
	{
		speed = 250;
		gib->homing_speed = speed;
		VectorSet(gib->mins,-12,-5,0);
		VectorSet(gib->maxs,12,5,8);
		gib->mass=10;
		gib->health = 15;
		gib->die = Rocket_Die;
		gib->takedamage = DAMAGE_AIM;
		gib->clipmask = MASK_SOLID;
		gib->monsterinfo.aiflags = AI_NOSTEP;
		gib->think = K2_HomingThink;
		gib->nextthink = level.time + FRAMETIME;
	}
	else
	{
		gib->think=G_FreeEdict;	
		gib->nextthink = level.time + 8000/speed;
	}
	
	VectorCopy (start, gib->s.origin);
	VectorCopy (dir, gib->movedir);
	vectoangles (dir, gib->s.angles);
	VectorScale (dir, speed, gib->velocity);
	gib->movetype = MOVETYPE_FLYMISSILE;
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_ROCKET;
	gib->s.renderfx |= RF_TRANSLUCENT;
	gib->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	gib->owner = self;
	gib->touch = K2_GibGunTouch;
	gib->dmg = damage;
	gib->classname = "gibgun";		
	gib->s.sound = gi.soundindex ("world/amb10.wav");
	gi.linkentity (gib);
}


void K2_GibGunTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	
	if(other == ent->owner)
		return;
	
	if(!other->client)
	{
		G_FreeEdict(ent);
		return;
	}
	
	//Only one in a player at a time
	if(other->client->gibtime > level.time)
	{
		G_FreeEdict(ent);
		return;
	}

	//Don't stick in protected players
	if(K2_IsProtected(other))
	{
		G_FreeEdict(ent);
		return;
	}

	
	ent->s.renderfx = RF_TRANSLUCENT;
	
	VectorSet(ent->movedir,0,0,0);
	VectorSet(ent->velocity,0,0,0);
	ent->avelocity[0] = 0;
	ent->avelocity[1] = 0;
	ent->avelocity[2] = 0;

	
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.effects = 0;
	ent->s.sound = 0;
	other->client->gibtime = level.time + gibtime->value;
	VectorCopy(other->s.origin,ent->s.origin);
	ent->nextthink = level.time + 0.1;
	ent->think = K2_GibGunThink;
	ent->enemy = other;
	gi.linkentity(ent);

	
	
}

void K2_GibGunThink(edict_t *ent)
{
	int	i=0;
	
	if (ent->enemy->deadflag || !ent->enemy->inuse)
	{
		G_FreeEdict(ent);
		return;
	}

	VectorCopy(ent->enemy->s.origin,ent->s.origin);
	gi.linkentity(ent);

	if (ent->enemy->client->gibtime >= level.time)
	{
		if (ent->enemy->client->gibframe < level.framenum)  
		{
			
			gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/hyprbu1a.wav"), 1, ATTN_NORM, 0);
			T_Damage(ent->enemy,ent,ent->owner,ent->enemy->velocity,ent->s.origin,vec3_origin,ent->dmg,0,0,MOD_GIBGUN);			
			ThrowGib (ent->enemy, "models/objects/gibs/sm_meat/tris.md2", 1, GIB_ORGANIC);
			ThrowGib (ent->enemy, "models/objects/gibs/sm_meat/tris.md2", 1, GIB_ORGANIC);
			ent->enemy->client->gibframe = level.framenum + 10;
		}

	}
	else
		G_FreeEdict(ent);

	ent->nextthink = level.time + 0.1;
	
}


//Flash Grenade
void Freeze_Explode (edict_t *ent)
{
	vec3_t      offset, v;
    edict_t		*target;
	float Distance, FreezeTimeAdd=0;


    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);    
    VectorAdd(ent->s.origin, offset, ent->s.origin);
	
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,MOD_FREEZE);

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    target = NULL;
    while ((target = findradius(target, ent->s.origin, freeze_radius->value)) != NULL)
    {
		
		//if (target == ent->owner)
        //  continue;       // You know when to close your eyes, don't you?
        
		if (!target->client)
           continue;       // It's not a player
        if (!visible(ent, target))
           continue;       // The grenade can't see it
        
		if (target->deadflag || !target->inuse)
			continue;

				
		//Don't freeze protected players
		if(K2_IsProtected(target))
			continue;

		//If already frozen, continue
		if(target->client->freezeTime > level.framenum)
			continue;

		//If player is burning, put him out
		if(target != ent->owner)
			target->client->burntime = 0;

		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);
		
		// Calculate freeze factor
		if ( Distance < freeze_radius->value/20 )
		FreezeTimeAdd = (freezetime->value*10); // Blind completely
		else
			FreezeTimeAdd = 1.5 * (freezetime->value*10) * ( 1 / ( ( Distance - freeze_radius->value*2 ) / (freeze_radius->value*2) - 2 ) + 1 ); // Freeze partially
     
		if ( FreezeTimeAdd < 0 )
			FreezeTimeAdd = 0; // Do not blind at all.

		// Not facing it, but still frozen a little
        if (!infront(target, ent))
            FreezeTimeAdd *= .75;

		//Freeze the owner if he's`too close
		if (target == ent->owner)
			FreezeTimeAdd *= 0.5;

		// Increment the freeze time counter
        target->client->freezeTime = level.framenum + FreezeTimeAdd;
        		
		// Let the player know what just happened
        // (It's just as well, he won't see the message immediately!)
        if (!target->bot_client)
			safe_centerprintf(target, 
                   "You are frozen by %s's freeze grenade!!!\n",
				   ent->owner->client->pers.netname);

        // Let the owner of the grenade know it worked
        if(target != ent->owner)
			if (!ent->owner->bot_client)
			safe_cprintf(ent->owner, PRINT_HIGH, 
                   "%s is frozen by your freeze grenade!\n",
                   target->client->pers.netname);

		//Eraser
		if(target->bot_client)
		{
			target->enemy = NULL;
		}
	}

    // Blow up the grenade
    BecomeExplosion1(ent);
}

void Freeze_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
       return;

	// If it goes in to orbit, it's gone...
    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (ent);
        return;
	}

    // All this does is make the bouncing noises when it hits something...
    if (!other->takedamage)
    {
		if (ent->spawnflags & 1)
        {
			if (random() > 0.5)
               gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"),
                         1, ATTN_NORM, 0);
            else
               gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"),
                         1, ATTN_NORM, 0);
        }
        else
            gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"),
                      1, ATTN_NORM, 0);
    
    
		return;
	}
	
	// The ONLY DIFFERENCE between this and "Grenade_Touch"!!
    Freeze_Explode (ent);    
}

		
