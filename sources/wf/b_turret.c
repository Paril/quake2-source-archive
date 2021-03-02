#include "g_local.h"
#include "m_player.h"
#include "bot_procs.h"

#define newTurret self->turret1
#define newTurret2 self->turret1->turret2
#define Level1Idle 0
#define Level1StartFirst 1
#define Level1StartEnd 2
#define Level1AttackFirst 3
#define Level1AttackEnd 6
#define Level23Idle 0
#define Level23AttackFirst 1
#define Level23AttackEnd 2
#define StatusIdle 0
#define StatusStart 1
#define StatusAttack 2

#define DIRECTION_LEFT		1
#define DIRECTION_CENTER	2
#define DIRECTION_RIGHT		3

#define LEVEL1_DAMAGE 6
#define LEVEL2_DAMAGE 8
#define LEVEL3_DAMAGE 12

#define max(a,b)    (((a) > (b)) ? (a) : (b))

void BecomeExplosion1 (edict_t *self);
void SP_Turret (edict_t *self);
//void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread);
void TurretFire1(edict_t *self);
void turret_self_remove(edict_t *ent);
void turret_remove(edict_t *ent);
void SentryReload(edict_t *ent, pmenu_t *p);
void SentryUpgrade(edict_t *ent, pmenu_t *p);
void SentryRepair(edict_t *ent, pmenu_t *p);

int PlayerChangeScore(edict_t *self, int points);


//Add or subtract ammo from sentry gun
void UpdateSentryAmmo(edict_t *ent, int change_amt)
{
	ent->light_level += change_amt;
	if (ent->light_level == 25)
	{
		safe_cprintf(ent->creator, PRINT_HIGH, "Warning: Your sentry gun is low on ammo.\n");
	}

}

void turret_fire_rocket (edict_t *ent, int direction)
{
	vec3_t forward, right, start, target, dir;

	if(ent->light_level<1)
		return;

	UpdateSentryAmmo(ent, -1);

	AngleVectors (ent->s.angles, forward, right, NULL);
	if (direction == DIRECTION_LEFT)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * -0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * -0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * -0.5 +4;
	}
	else if (direction == DIRECTION_CENTER)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 ;
		start[1] = ent->s.origin[1] + forward[1] * 3 ;
		start[2] = ent->s.origin[2] + forward[2] * 3+4;
	}
	else		// Right
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * 0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * 0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * 0.5+4;
	}

	// calc direction to where we targetd
	VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);
	VectorSubtract (target, start, dir);
	VectorNormalize (dir);

	fire_rocket (ent, start, dir, 25, 650, 25, 25, MOD_SENTRY_ROCKET);

	// send muzzle flash

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_SHOTGUN );
	gi.multicast (start, MULTICAST_PVS);
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("boss3/xfire.wav"), 1, ATTN_NORM, 0);

}



//New turret bullet firing code.  Replaces fire_bullet and fire_rail
//Fires at ent->enemy
//direction = 1 (left), 2 (center), 3 (right)
void turret_fire_bullet (edict_t *ent, int direction)
{

	vec3_t		forward, right, start, target, dir;
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;
	int			kick = 0;
	int			mod = MOD_SENTRY;
	int			damage;
	float		chance_of_hit;
	float		maxvelocity;
	float		r;

if (wfdebug) gi.dprintf("turret_fire_bullet\n");

	if(ent->light_level<1)  //do we have ammo?
		return;

	if (!ent->enemy)
		return;

	UpdateSentryAmmo(ent, -1);

	//Calculate starting position
	AngleVectors (ent->s.angles, forward, right, NULL);

	if (direction == DIRECTION_LEFT)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * -0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * -0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * -0.5 +4;
	}
	else if (direction == DIRECTION_CENTER)
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 ;
		start[1] = ent->s.origin[1] + forward[1] * 3 ;
		start[2] = ent->s.origin[2] + forward[2] * 3+4;
	}
	else		// Right
	{
		start[0] = ent->s.origin[0] + forward[0] * 3 + right[0] * 0.5;
		start[1] = ent->s.origin[1] + forward[1] * 3 + right[1] * 0.5;
		start[2] = ent->s.origin[2] + forward[2] * 3 + right[2] * 0.5+4;
	}

	// calc direction to where we targetd
	VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);

	//Adjust for height
//	target[2] += ent->enemy->viewheight/1.5;

	VectorSubtract (target, start, dir);
	VectorNormalize (dir);

	VectorMA (start, 8192, dir, end);
	VectorCopy (start, from);
	ignore = ent;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

	tr = gi.trace (from, NULL, NULL, end, ignore, mask);

	if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
	{
		mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
		water = true;
	}

	//Do damage

	//Damage based on sentry level
	chance_of_hit = 1;
	if (ent->count == 1)
	{
		damage = LEVEL1_DAMAGE;
		chance_of_hit = .85;
	}
	else if (ent->count == 2)
	{
		damage = LEVEL2_DAMAGE;
		chance_of_hit = .90;
	}
	else if (ent->count == 3)
	{
		damage = LEVEL3_DAMAGE;
		chance_of_hit = .95;
	}
	else
	{
		damage = 1;		//don't know what level it is
		chance_of_hit = 1.0;		//always hit
	}

	//Moving reduces chance of being hit
	maxvelocity = max( abs (ent->enemy->velocity[0]), abs (ent->enemy->velocity[1]) );

	if (maxvelocity >= 320)		//wierd setting for grappling
	{
		chance_of_hit = chance_of_hit  * .65;
	}
	else if (maxvelocity >= 250)		//running
	{
		chance_of_hit = chance_of_hit  * .74;
	}
	else if (maxvelocity >= 150) //walking
	{
		chance_of_hit = chance_of_hit  * .90;
	}
	r = random();

if (wfdebug) gi.dprintf("chance =%f, rnd = %f, takedamage? %d\n",	chance_of_hit, r, ent->enemy->takedamage);

	if ((ent->enemy->takedamage) && (r <= chance_of_hit))
//		T_Damage (ent->enemy, ent, ent->creator, dir, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_SENTRY);
        T_Damage (ent->enemy, ent, ent, dir, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_SENTRY);

	//Do gunshot effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GUNSHOT);
	gi.WritePosition (tr.endpos);
	gi.WriteDir (tr.plane.normal);
	gi.multicast (tr.endpos, MULTICAST_PVS);

	if (ent->client)
		PlayerNoise(ent, tr.endpos, PNOISE_IMPACT);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_SHOTGUN );
	gi.multicast (start, MULTICAST_PVS);
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("boss3/xfire.wav"), 1, ATTN_NORM, 0);
}




void TurretAnimate(edict_t *ent)
{
	if(ent->count==1)
	{
		if(ent->delay==StatusIdle)
		{
			ent->s.frame = Level1Idle;
			return;
		}
		else if(ent->delay==StatusStart)
		{
			if(ent->s.frame == Level1StartEnd)
			{
				ent->delay=StatusAttack;
				ent->s.frame=Level1AttackFirst;
				return;
			}
			else if(ent->s.frame==Level1Idle)
			{
				ent->s.frame = Level1StartFirst;
				return;
			}
			else if(ent->s.frame==Level1StartFirst)
			{
				ent->s.frame++;
				return;
			}
			else
			{
				ent->s.frame = Level1Idle;
				return;
			}
		}
		else
		{
			if(ent->delay==StatusAttack)
			{
				if(ent->s.frame==Level1AttackEnd)
				{
					ent->s.frame--;
					return;
				}
				else
				{
					ent->s.frame++;
					return;
				}
			}
		}
	}
	else
	{
		if(ent->delay ==StatusIdle)
		{
			ent->s.frame = Level23Idle;
			return;
		}
		else
		{
			if(ent->s.frame==Level23AttackEnd)
			{
				ent->s.frame = Level23AttackFirst;
				return;
			}
			ent->s.frame++;
			return;
		}
	}
}


void FireTurretLeft(edict_t *ent)
{
	float f;

if (wfdebug) gi.dprintf("Sentry Fire Left\n");

	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 10;
	if (((f * 10) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_LEFT);
	else
		turret_fire_bullet (ent, DIRECTION_LEFT);
}


void FireTurretCenter(edict_t *ent)
{
	float f;

if (wfdebug) gi.dprintf("Sentry Fire Center\n");

	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 10;
	if (((f * 10) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_CENTER);
	else
		turret_fire_bullet (ent, DIRECTION_CENTER);
}

void FireTurretRight(edict_t *ent)
{
	float f;

if (wfdebug) gi.dprintf("Sentry Fire Right\n");

	//Fire bullet.  For level 3 sentry's, fire rocket every 10th round
	f = ent->light_level / 10;
	if (((f * 10) == ent->light_level) && (ent->count == 3))
		turret_fire_rocket (ent, DIRECTION_RIGHT);
	else
		turret_fire_bullet (ent, DIRECTION_RIGHT);
}


void Turret_Think(edict_t *self)
{
	static char Sentry[32];
	static char Armor[32];
	static char Ammo[32];
	static char Level[32];
	int		range;
	int		light_level;
	int		max_dist;
	edict_t *blip;
	float dist;
	int	contents;
	int ideal_yaw = 0;
	vec3_t v;
	trace_t		tr;
//	gitem_t	*ammo;
//	int max;
	float checkyaw;
	blip = NULL;

	
	//If there is no client attached to this turret any more, it should be removed
	//from the game
	if (!self->creator)				//There is no creator
	{
		turret_self_remove(self);
	}

	if (!self->creator->client)		//Creator isn't a client
	{
		turret_self_remove(self);
	}

	//Make sure that the creator of this sentry gun also has an
	//entry that points back to this sentry
	if (!self->creator->sentry)	//creator doesn't know about this sentry
	{
		turret_self_remove(self);
	}

	if (self->creator->sentry != self)	//creator has a different sentry
	{
		turret_self_remove(self);
	}

	//If sentry or stand is in something solid, remove it
	contents = (int)gi.pointcontents(self->s.origin);

	if (contents & CONTENTS_SOLID)
	{
		safe_cprintf(self->creator, PRINT_HIGH, "Your sentry was in a bad map position, so it was removed.\n");
		turret_self_remove(self);
		return;
	}

	if (self->sentry)	//stand
	{
		contents = (int)gi.pointcontents(self->sentry->s.origin);

		if (contents & CONTENTS_SOLID)
		{
			safe_cprintf(self->creator, PRINT_HIGH, "Your sentry stand in a bad map position so it was removed.\n");
			turret_self_remove(self);
			return;
		}
	}


	/*if (self->light_level!=self->gib_health)
	{
		if (self->PlasmaDelay<level.time)
		{
			max = self->gib_health;

			if (self->count==3)
				self->PlasmaDelay = level.time +0.1;
			else if (self->count==2)
				self->PlasmaDelay = level.time +0.2;
			else
				self->PlasmaDelay = level.time +0.4;
			self->light_level++;
			if (self->light_level > max)
				self->light_level = max;
//GREGG			gi.sound(self, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
		}
	}*/

	//Play sound if we are ready
/*
	--self->turretsoundcountdown;
	if (self->turretsoundcountdown<=0)
	{
		//Play a sound
		if (self->count == 1)
			gi.sound(self, CHAN_VOICE, gi.soundindex("medic/medsrch1.wav"), 1, ATTN_NORM, 0);
		else if (self->count == 2)
			gi.sound(self, CHAN_VOICE, gi.soundindex("floater/fltatck2.wav"), 1, ATTN_NORM, 0);
		else if (self->count == 3)
			gi.sound(self, CHAN_VOICE, gi.soundindex("tank/tnkatck4.wav"), 1, ATTN_NORM, 0);
		self->turretsoundcountdown = 110; //same as 11 seconds

	}
*/

	//	if (self->turretsoundcountdown == 40)//back to ambient sound
//		self->s.sound = gi.soundindex ("world/amb3.wav");

	if (self->count == 1)
	{
		range = 500;
		light_level = 10;
		max_dist = 300;
	}
	else if (self->count == 2)
	{
		range = 800;
		light_level = 7;
		max_dist = 500;
	}

	else if (self->count == 3)
	{
		range = 900;
		light_level = 4;
		max_dist = 700;
	}

	// Don't shoot frozen, dead, or non visible enemies
	if ((self->enemy) &&
		((self->enemy->health <= 0) || (!visible(self, self->enemy)) || (self->enemy->frozen)))
	{
		self->enemy = NULL;
	}

	if (!self->enemy)
	{
		// If we are going back to idle from some other status,
		// copy the angles back to original
/*
		if (self->delay != StatusIdle)
		{
			VectorCopy (self->orig_angles , self->s.angles);
			self->sentrydelay =level.time+1;
		}
*/
		self->delay=StatusIdle;

		while (blip = findradius (blip, self->s.origin, range))
		{

			if (!blip->inuse)
				continue;

			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			{
				//allow it to shoot decoys
				if (strcmp(blip->classname, "decoy") )
					continue;	//not a decoy
			}
		    if (strcmp(blip->classname, "hook") == 0 )
				continue;	//not a grapple
			if (blip->solid == SOLID_NOT)
				continue;	//don't see observers
			if (blip->health <= 0)
				continue;
			if (blip->frozen)
				continue;
//			if (blip == self->creator)		//wont work for anarchy mode
//				continue;
			if (blip->disguised)
				continue;
			if ((blip->wf_team == self->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
//			if (blip->wf_team == self->wf_team)
				continue;
//			if (blip->light_level < light_level)
//				continue;
			tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
			if (tr.fraction != 1.0)
				continue;

			//Check the angle.
/*			VectorSubtract (blip->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			checkyaw = anglemod(self->s.angles[YAW]) - self->ideal_yaw;
			if (checkyaw < -25 || checkyaw > 25)
				continue;

			VectorSubtract (self->s.origin, blip->s.origin, v);

*/
			dist = VectorLength(v);


			if (!visible(self, blip) && dist > max_dist)
				continue;

			self->enemy = blip;
		}
	}
	else
	{
		VectorSubtract (self->enemy->s.origin, self->s.origin, v);

		self->ideal_yaw = vectoyaw(v);

		M_ChangeYaw(self);

		checkyaw = anglemod(self->s.angles[YAW])-self->ideal_yaw;

		if (checkyaw>-25 && checkyaw<25)
		{
			if (self->light_level>0)
			{
				if (visible(self, self->enemy))
				{
					if (self->s.frame != 1)
					{
						if (self->delay==StatusIdle)
							self->delay = StatusStart;

						if (self->count == 1)
						{
							if(self->s.frame == 3)
								FireTurretRight(self);

							if( (self->s.frame == 1) || (self->s.frame ==5) )
								FireTurretLeft(self);
						}
						else if (self->count == 2)
						{
							FireTurretCenter(self);
						}

						else if (self->count == 3)

						{
							//Maybe this will reduce lag
//							FireTurretRight(self);
//							FireTurretLeft(self);
							FireTurretCenter(self);
						}

					}
				}
				else
				{
					self->delay=StatusIdle;
					self->enemy=NULL;
				}
			}
		}
		else
		{
			self->delay=StatusIdle;
			self->enemy=NULL;
		}
	}


	//Move the stand? (GREGG)
	if (self->sentry)
		VectorCopy(self->s.origin,self->sentry->s.origin);

	// Move the sentry to stay with stand (TeT)
//	if (self->sentry)
//		VectorCopy(self->sentry->s.origin, self->s.origin);

	TurretAnimate(self);
	self->nextthink = level.time + 0.1;

	if(self->delay==StatusIdle)
	{
		if(self->sentrydelay<level.time)
		{
			if(self->PlasmaDelay)
				self->PlasmaDelay=0;
			else
				self->PlasmaDelay=1;
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 0.1, ATTN_NORM, 0);
			self->sentrydelay=level.time + 2;
		}
		if(self->PlasmaDelay)
			self->s.angles[YAW]+=3.5;
		else
			self->s.angles[YAW]-=3.5;
		if(self->s.angles[YAW]<0)
			self->s.angles[YAW]+=360;
		else if(self->s.angles[YAW]>360)
			self->s.angles[YAW]-=360;
	}
	//if(self->health<self->max_health)
	//	self->health++;
}


//Ent = turret entity
void turret_self_remove(edict_t *ent)
{

	//Clear client's pointer to sentry gun
	if (ent->creator)
		ent->creator->sentry = NULL;

	//first free stand
	if (ent->sentry) G_FreeEdict(ent->sentry);

	//Then free the sentry gun
	BecomeExplosion1 (ent);
//	G_FreeEdict(ent);


}

//Ent = Player  entity
void turret_remove(edict_t *ent)
{
	if (ent->sentry)
	{  //42 ebc
		safe_cprintf(ent, PRINT_HIGH, "Sentry Gun off.\n");

		//First free the stand
		if (ent->sentry->sentry)
		{
			G_FreeEdict(ent->sentry->sentry);
			ent->sentry->sentry = NULL;
		}


		//Then free the sentry gun
		BecomeExplosion1 (ent->sentry);
//		G_FreeEdict(ent->sentry);

		ent->sentry = NULL;

		if (ent->client->oldplayer)
			G_FreeEdict(ent->client->oldplayer);
		return;
	}
}

void turret_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	vec3_t origin;
	char	*name1;
	char	*name2;
//42 sebc

	edict_t *blip = NULL;
	int i;
	
//42 bot sentry clear itemnode owner
if (self->creator->bot_client)
	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == self->creator)
	  {
		 if (!strcmp(blip->classname, "item_sentryspot") )
		{
			blip->owner = NULL;
		}
	  }
	}
//42 end clear item owner



	safe_cprintf(self->creator, PRINT_HIGH, "Sentry Gun Destroyed.\n"); 

	//Give a frag to the attacker
	if (attacker->client && attacker->wf_team != self->wf_team)
	{
		PlayerChangeScore(attacker,CTF_SENTRY_POINTS);
		name1 = attacker->client->pers.netname;
		if (self->creator->client)
			name2 = self->creator->client->pers.netname;
		else
			name2 = "unknown player";
		my_bprintf(PRINT_HIGH, "%s destroyed %s's sentry gun.\n", name1, name2);

	}

	VectorCopy (self->s.origin,origin);
	origin[2]+= 0.5;
	self->takedamage = DAMAGE_NO;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (origin);
	gi.multicast (origin, MULTICAST_PVS);

	turret_self_remove(self);

}

void SentryReload(edict_t *ent, pmenu_t *p)
{	
	int max, armorfill;
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}
	if (ent->selectedsentry->light_level!=ent->selectedsentry->gib_health)
	{
		max = ent->selectedsentry->gib_health;
		armorfill=ent->selectedsentry->gib_health-ent->selectedsentry->light_level;
		if(armorfill>75)
			armorfill=75;
//		if(armorfill>25)
//			armorfill=25;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 25)
		{
 			safe_cprintf(ent, PRINT_HIGH, "You need 25 Bullets to reload the sentry gun.\n");
			if (ent->selectedsentry) gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);
			PMenu_Close(ent);
			return;
		}

		//25 player bullets will give 75 bullets to sentry gun (Gregg)
//		if(armorfill > ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))])
//			armorfill = ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];

		UpdateSentryAmmo(ent->selectedsentry, armorfill);
//		ent->selectedsentry->light_level += armorfill;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] -= 25;
		if (ent->selectedsentry->light_level > max)
			ent->selectedsentry->light_level = max;
		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
	PMenu_Close(ent);
}

void AddArmorToSentry(edict_t *ent, int amt)
{
	int max, armor;

	if (ent->health < ent->max_health)
	{
		max = ent->max_health;

		//How much armor is needed?
		armor = ent->max_health - ent->health;

		if (amt < armor) 
			armor = amt;
		else 
			armor = 0;

		ent->health+= armor;
		if (ent->health > max)
			ent->health = max;

		if (ent->selectedsentry)
			gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
}

void SentryRepair(edict_t *ent, pmenu_t *p)
{	
	int max, armor;
	int cells, maxcells, currcells;
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}
	if (ent->selectedsentry->health<ent->selectedsentry->max_health)
	{
		max = ent->selectedsentry->max_health;

		//How much armor is needed?
		armor = ent->selectedsentry->max_health - ent->selectedsentry->health;

		//Can't give more than 100 points armor in one shot
		if(armor > 100) armor=100;

		//Each cell gives 4 points armor
		currcells = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];
		maxcells = armor / 4;

		if (currcells ==0)
		{
 			safe_cprintf(ent, PRINT_HIGH, "You need Cells to repair the sentry gun.\n");
			if (ent->selectedsentry) gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);

			PMenu_Close(ent);
			return;
		}

		//If we don't have enough cells, adjust the amount of armor to give
		if(currcells < maxcells)
		{
			cells = currcells;
			armor = cells * 4;
		}
		else
		{
			cells = maxcells;
		}

		ent->selectedsentry->health+= armor;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= cells;
		if (ent->selectedsentry->health > max)
			ent->selectedsentry->health = max;
		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
	PMenu_Close(ent);
}

void SentryUpgrade(edict_t *ent, pmenu_t *p)
{
	float dist;
	vec3_t distance;

	if (!ent->selectedsentry)
		return;

	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		PMenu_Close(ent);
		return;
	}
	if (ent->selectedsentry->count == 3)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry gun already at level 3\n");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 60)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need 60 cells to upgrade sentry gun.\n");
		PMenu_Close(ent);
		return;
	}




		if (ent->selectedsentry->count < 3)
			ent->selectedsentry->count++;
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 60;
		if (ent->selectedsentry->count == 2)
		{
			ent->selectedsentry->health= 200;
			ent->selectedsentry->max_health =200;
			ent->selectedsentry -> s.modelindex	= gi.modelindex ("models/sentry/turret2/tris.md2");
			ent->selectedsentry->delay = StatusIdle;
			ent->selectedsentry->s.frame = 0;
			ent->selectedsentry->yaw_speed = 40;
			ent->selectedsentry->gib_health = 150;//Max Ammo
			ent->selectedsentry->light_level = 150;
			//ent->selectedsentry->turretsoundcountdown = 0;
			gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
		}
		if (ent->selectedsentry->count == 3)
		{
			ent->selectedsentry->health= 300;
			ent->selectedsentry->max_health = 300;
			ent->selectedsentry -> s.modelindex	= gi.modelindex ("models/sentry/turret3/tris.md2");
			ent->selectedsentry->delay = StatusIdle;
			ent->selectedsentry->s.frame = 0;			
			ent->selectedsentry->yaw_speed = 45;
			ent->selectedsentry->gib_health = 225;//Max Ammo
			ent->selectedsentry->light_level = 225;
			//ent->selectedsentry->turretsoundcountdown = 0;
			gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
		}
		PMenu_Close(ent);
}

void Sentry_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{ 
	if (other->client) other->selectedsentry = ent;

	//Skip if they already have a menu up
	if ((other->client) && (other->client->menu)) 
		return;

	if(other->sentrydelay>level.time)
		return;

	//Only show menu a max of every 1 second

	//Only allow team mates access to menu
	if (!other->bot_client)//42
	if (other->client && (other->wf_team == ent->wf_team) && (other->client->player_special & SPECIAL_SENTRY_GUN))
	{
		PMenu_Close(other);

		sprintf(other->client->wfsentrystr[0], "*%s's Sentry", ent->standowner->client->pers.netname);
		sprintf(other->client->wfsentrystr[1], "  Armor:%d/%d", ent->health, ent->max_health);
		sprintf(other->client->wfsentrystr[2], "  Ammo:%d/%d", ent->light_level,ent->gib_health);
		sprintf(other->client->wfsentrystr[3], "  Level:%d", ent->count);
		other->client->sentrymenu[0].text = other->client->wfsentrystr[0];
		other->client->sentrymenu[0].SelectFunc = NULL;
		other->client->sentrymenu[0].align = PMENU_ALIGN_CENTER;
		other->client->sentrymenu[0].arg = 0;
		other->client->sentrymenu[1].text = other->client->wfsentrystr[1];
		other->client->sentrymenu[1].SelectFunc = NULL;
		other->client->sentrymenu[1].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[1].arg = 0;
		other->client->sentrymenu[2].text = other->client->wfsentrystr[2];
		other->client->sentrymenu[2].SelectFunc = NULL;
		other->client->sentrymenu[2].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[2].arg = 0;
		other->client->sentrymenu[3].text = other->client->wfsentrystr[3];
		other->client->sentrymenu[3].SelectFunc = NULL;
		other->client->sentrymenu[3].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[3].arg = 0;
		other->client->sentrymenu[4].text = "1. Upgrade";
		other->client->sentrymenu[4].SelectFunc = SentryUpgrade;
		other->client->sentrymenu[4].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[4].arg = 0;
		other->client->sentrymenu[5].text = "2. Repair";
		other->client->sentrymenu[5].SelectFunc = SentryRepair;
		other->client->sentrymenu[5].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[5].arg = 0;
		other->client->sentrymenu[6].text = "3. Reload";
		other->client->sentrymenu[6].SelectFunc = SentryReload;
		other->client->sentrymenu[6].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[6].arg = 0;

//		other->selectedsentry = ent->standowner->sentry;
		other->selectedsentry = ent;

		PMenu_Open(other, other->client->sentrymenu, -1, sizeof(other->client->sentrymenu) / sizeof(pmenu_t), true, false);
		other->sentrydelay = level.time + 3;

		//Set timeout for menu (4 seconds)
		if (other->client->menu) other->client->menu->MenuTimeout = level.time + 4;
	}
}
//Stand think function
void Stand_Think (edict_t *ent)
{
	//For some reason, entity needs a think function
	if (ent->health <= 0)
		G_FreeEdict(ent);
	else
		ent->nextthink = level.time + 5.0;
}


//Build the turret
void place_turret (edict_t *ent)
{
	vec3_t		forward,up,right,wallp, pos,try1,try2,try3,try4;
	edict_t *blip = NULL;
	int		cells;
	trace_t		tr;
	edict_t *sentrystand;
	int	armor;
	int	armorindex;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	if (ent->sentry) 
	{  
		turret_remove(ent);

		if (ent->client->oldplayer)
			G_FreeEdict(ent->client->oldplayer);
		return; 
	}

	// cells for sentry gun ?
	if ((int)wfflags->value & WF_ANARCHY)
	{
		armor = 150;
		armorindex = ArmorIndex (ent); 
		if (ent->client->pers.inventory[armorindex] < armor)
		{
			safe_cprintf(ent, PRINT_HIGH, "You need %d points of armor to create sentry gun.\n", armor);
			return;
		}	
	}
	else
	{
		cells = 50;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < cells)
		{
			safe_cprintf(ent, PRINT_HIGH, "You need %d cells to create sentry gun.\n", cells);
			return;
		}	
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
	try1[0]=ent->s.origin[0]+forward[0]*150+right[0]*20;
	try1[1]=ent->s.origin[1]+forward[1]*150+right[1]*20;
	try1[2]=ent->s.origin[2]+forward[2]*150+30+right[2]*20; 
	try2[0]=ent->s.origin[0]+forward[0]*150+right[0]*-20;
	try2[1]=ent->s.origin[1]+forward[1]*150+right[1]*-20;
	try2[2]=ent->s.origin[2]+forward[2]*150+30+right[2]*-20; 
	try3[0]=ent->s.origin[0]+forward[0]*75+right[0]*20;
	try3[1]=ent->s.origin[1]+forward[1]*75+right[1]*20;
	try3[2]=ent->s.origin[2]+forward[2]*75+30+right[2]*20; 
	try4[0]=ent->s.origin[0]+forward[0]*75+right[0]*-20;
	try4[1]=ent->s.origin[1]+forward[1]*75+right[1]*-20;
	try4[2]=ent->s.origin[2]+forward[2]*75+30+right[2]*-20; 
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);
	
		
	// Line complete ? (ie. no collision)
	if (tr.fraction != 1.0)
	{//42 ebc
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
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try1, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try2, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try3, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}
	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, try4, ent, MASK_SOLID);
	if (tr.fraction != 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Not enough room.\n");
		return;
	}

	// Hit sky ?
	if ((tr.surface) && (tr.surface->flags & SURF_SKY))
		return;

	while (blip = findradius (blip, pos, 50))
	{
		 if ( (!strcmp(blip->classname, "item_flag_team1") )
		  || (!strcmp(blip->classname, "item_flag_team2") )
		  || (!strcmp(blip->classname, "worldspawn") )
		  || (!strcmp(blip->classname, "info_player_start") )
		  || (!strcmp(blip->classname, "info_player_deathmatch") )
		  || (!strcmp(blip->classname, "item_flagreturn_team1") )
		  || (!strcmp(blip->classname, "item_flagreturn_team2") )
		  || (!strcmp(blip->classname, "misc_teleporter_dest") )
		  || (!strcmp(blip->classname, "info_teleport_destination") ) )
		 {
		  	safe_cprintf (ent, PRINT_HIGH, "Too Close to Items.\n");
		  	return ;
		 }
	}

	if ((int)wfflags->value & WF_ANARCHY)
	{
		ent->client->pers.inventory[armorindex] -= armor;
	}
	else
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= cells;
	}


	safe_cprintf(ent, PRINT_HIGH, "Sentry Gun on.\n");
	ent->sentry = G_Spawn();

	//ent->sentry->s.sound = gi.soundindex ("world/amb3.wav");
	//ent->sentry->turretsoundcountdown = 0;

	VectorClear (ent->sentry->mins);
	VectorClear (ent->sentry->maxs);
	VectorCopy (pos, ent->sentry->s.origin);
	ent->sentry ->s.angles[0]=0;
//	ent->sentry -> movetype		= MOVETYPE_STEP;
//	ent->sentry -> clipmask		= MASK_SHOT;
	ent->sentry -> movetype		= MOVETYPE_TOSS;
	ent->sentry -> clipmask		= MASK_PLAYERSOLID;
	ent->sentry->mass = 400;
	ent->s.renderfx=RF_FRAMELERP|RF_TRANSLUCENT|RF_GLOW;
	//grenade -> solid		= SOLID_NOT;
	ent->sentry->solid = SOLID_BBOX;
//GR - reduce size of bounding box since I reduced size of model
//	VectorSet(ent->sentry->mins, -95,-44,-80);
//	VectorSet(ent->sentry->maxs, 57, 41, 24);
	VectorSet(ent->sentry->mins, -50,-20,-40);
	VectorSet(ent->sentry->maxs, 30, 21, 22);
	ent->sentry->takedamage=DAMAGE_YES;
	ent->sentry -> s.modelindex	= gi.modelindex ("models/sentry/turret1/tris.md2");
	ent->sentry -> creator = ent;	
	ent->sentry ->sentrydelay =level.time + 1;
	ent->sentry->standowner=ent;
	ent->sentry->think = Turret_Think;
//	ent->sentry->nextthink = level.time + 0.1;
	ent->sentry->nextthink = level.time + 2.0;
	ent->sentry->touch = Sentry_Touch;
	ent->sentry->die = turret_die;
	ent->sentry->health= 80;
	ent->sentry->max_health = 100;
	ent->sentry->count = 1;
	ent->sentry->s.sound = gi.soundindex ("weapons/rg_hum.wav");
	//ent->sentry->noise_index2 = gi.soundindex ("weapons/rg_hum.wav");
	ent->sentry->attenuation =1;
	ent->sentry->volume = 0.5;
	ent->sentry->classname = "SentryGun";


	ent->sentry->noteamdamage = true;	//Don't let teammates damage it
	ent->sentry->yaw_speed = 35;
	ent->sentry->gib_health = 75;//Max Ammo
	ent->sentry->light_level = 75;//Ammo Total
		
	ent->sentry->delay=StatusIdle;

	if ((int)wfflags->value & WF_ANARCHY)
	{
		ent->sentry->wf_team = 0;	//fire at anybody
		ent->sentry->count = 3;
		ent->sentry->health= 300;
		ent->sentry->max_health = 300;
		ent->sentry -> s.modelindex	= gi.modelindex ("models/sentry/turret3/tris.md2");
		ent->sentry->s.frame = 0;			
		ent->sentry->yaw_speed = 45;
		ent->sentry->gib_health = 225;//Max Ammo
		ent->sentry->light_level = 225;
		gi.sound(ent->sentry, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
	}
	else
		ent->sentry->wf_team = ent->wf_team;

	gi.linkentity (ent->sentry);

if (wfdebug)
  {
	ent->sentry->s.modelindex2	= gi.modelindex ("models/stand/tris.md2");
	ent->sentry->sentry = NULL;
  }
else
  {
	sentrystand = G_Spawn();
	VectorClear (sentrystand->mins);
	VectorClear (sentrystand->maxs);
	VectorCopy (pos,sentrystand->s.origin);
	sentrystand->s.angles[0]=0;
	sentrystand->movetype		= MOVETYPE_TOSS;
	sentrystand->mass = 400;
	//grenade -> solid		= SOLID_NOT;
	sentrystand->solid = SOLID_NOT;
	VectorSet(sentrystand->mins, -45,-15,-40);
	VectorSet(sentrystand->maxs, 25, 18, 18);
    botDebugPrint("mins: %s\n", vtos(sentrystand->mins));
	botDebugPrint("mins: %s\n", vtos(ent->sentry->s.origin));
	botDebugPrint("mins: %s\n", vtos(sentrystand->s.origin));
	botDebugPrint("mins: %s\n", vtos(ent->sentry->s.angles));
	botDebugPrint("mins: %s\n", vtos(sentrystand->s.angles));
	sentrystand->takedamage=DAMAGE_NO;
	sentrystand->health = 10;
	sentrystand->s.modelindex	= gi.modelindex ("models/stand/tris.md2");
	sentrystand->classname = "SentryStand";
	sentrystand->creator = ent->sentry;
	sentrystand->standowner = ent;
	sentrystand->wf_team = ent->wf_team;
//	sentrystand->touch = Sentry_Touch;//Touching it will then pop up a menu
	sentrystand->noteamdamage = true;	//Don't let teammates damage it
	sentrystand->think = Stand_Think;
	sentrystand->nextthink = level.time + 0.5;
		
	gi.linkentity (sentrystand);
	ent->sentry->sentry = sentrystand;
  }
}

void UpgradeSentry(edict_t *self)
{
	edict_t *blip;
	trace_t		tr;
	qboolean found;

	blip = NULL;
	found = false;
//	while (blip = findradius (blip, self->s.origin, 2048))
	while (blip = findradius (blip, self->s.origin, 128))
	{
		if (Q_stricmp("SentryGun", blip->classname))
			continue;
              if (!self->bot_client)
                if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 60)
		{//42 ebc
			safe_cprintf(self, PRINT_HIGH, "You need 60 cells to upgrade sentry gun.\n");
			return;
		}

		found = true;

		tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
		if (tr.fraction != 1.0)
			continue;
		if (blip->creator != self)
			continue;
		if (blip->count == 3)//42 sbc
 			safe_cprintf(self, PRINT_HIGH, "Sentry gun already at level 3\n");

		if (blip->count < 3)
			blip->count++;
		if(!self->bot_client)//42
                self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 60;
		if (blip->count == 2)
		{
			blip->health= 200;
			blip->max_health =200;
			blip -> s.modelindex	= gi.modelindex ("models/sentry/turret2/tris.md2");
			blip->delay = StatusIdle;
			blip->s.frame = 0;
			blip->yaw_speed = 40;
			blip->gib_health = 150;//Max Ammo
			blip->light_level = 150;//5/99 suggest removing this whole section and rework/use sentryupdate()
			//blip->turretsoundcountdown = 110;
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
		}
		if (blip->count == 3)
		{
			blip->health= 300;
			blip->max_health =300;
			blip -> s.modelindex	= gi.modelindex ("models/sentry/turret3/tris.md2");
			blip->delay = StatusIdle;
			blip->s.frame = 0;			
			blip->yaw_speed = 45;
			blip->gib_health = 225;//Max Ammo
			blip->light_level = 225;//5/99 suggest removing this whole section and rework/use sentryupdate()
			//blip->turretsoundcountdown = 110;
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/pc_up.wav"), 1, ATTN_NORM, 0);
		}
	}

	if (found == false)//42 sbc
 		safe_cprintf(self, PRINT_HIGH, "Sorry, you aren't close enough.\n");
}

//test_Sentry() - see if there is a sentry close enough to work on

//Returns 0 if sentry does not exist or is too far away
//Returns 1 if sentry exists and is close enough
int test_Sentry (edict_t *ent)
{
	float dist;
	float tdist;
	edict_t *sentry;
	vec3_t distance;
	edict_t *blip;

	if (!ent->sentry)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry not found.\n"); 
		return 0;
	}

	blip = NULL;
	dist = 0;
	sentry = NULL;
	while (blip = findradius (blip, ent->s.origin, 100))
	{
		if (strcmp(blip->classname, "SentryGun"))
			continue;	//Not a sentry gun

		distance[0]=ent->s.origin[0] - blip->s.origin[0];
		distance[1]=ent->s.origin[1] - blip->s.origin[1];
		distance[2]=ent->s.origin[2] - blip->s.origin[2];
		tdist=VectorLength(distance);
		if ((dist == 0) || (tdist < dist))
		{
			dist = tdist;
			sentry = blip;
		}
	}
	if(dist>100 || sentry == NULL)
	{
		safe_cprintf(ent, PRINT_HIGH, "Sentry too far away.\n"); 
		return 0;
	}
	else 
	{
		ent->selectedsentry = sentry;
		return 1;
	}

}

void cmd_Sentry (edict_t *ent)
{
	char    *string;

	string=gi.args();

	if (!ent->client) return;

	//argument = "build", "remove", "upgrade", "repair" and "reload"
	if (Q_stricmp ( string, "build") == 0)
	{
		if (!ent->sentry)
			place_turret(ent);
		else
			safe_cprintf(ent, PRINT_HIGH, "Sentry already exists.\n"); 
	}
	else if (Q_stricmp ( string, "remove") == 0)
	{
		if (ent->sentry) 
			turret_remove(ent);
		else
			safe_cprintf(ent, PRINT_HIGH, "Sentry not found.\n"); 
	}
	else if (Q_stricmp ( string, "upgrade") == 0)
	{
		if (test_Sentry(ent)) SentryUpgrade(ent, NULL);
	}
	else if (Q_stricmp ( string, "repair") == 0)
	{
		if (test_Sentry(ent)) SentryRepair(ent, NULL);
	}
	else if (Q_stricmp ( string, "reload") == 0)
	{
		if (test_Sentry(ent)) SentryReload(ent, NULL);
	}

	//Otherwise toggle on/off
	else if (Q_stricmp ( string, "") == 0)
	{
		place_turret(ent);
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "Invalid sentry command.\n"); 

}
