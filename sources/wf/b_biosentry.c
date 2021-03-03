#include "g_local.h"
#include "m_player.h"
#define newBio self->Bio1
#define newBio2 self->Bio1->Bio2
#define IdleStart 0
#define IdleEnd 2
#define AttackStart 3
#define AttackEnd 7
#define AttackFire 7
#define StatusIdle 0
#define StatusAttack 1

void BecomeExplosion1 (edict_t *self);
void SP_Biosentry (edict_t *self);
//void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread);
void BiosentryFire1(edict_t *self);
void Biosentry_self_remove(edict_t *ent);
void Biosentry_remove(edict_t *ent);
void BiosentryReload(edict_t *ent, pmenu_t *p);
void BiosentryRepair(edict_t *ent, pmenu_t *p);
void fire_infecteddart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int mod);


void BiosentryAnimate(edict_t *ent)
{
	if(ent->delay==StatusIdle)
	{
		if (ent->s.frame >= IdleEnd)
		{
			ent->s.frame = IdleStart;
			return;
		}
	}
	else if (ent->delay==StatusAttack) 
	{
		if(ent->s.frame < AttackStart)
		{
			ent->s.frame = AttackStart;
			return;
		}
		if(ent->s.frame >= AttackEnd)
		{
			ent->s.frame = IdleStart;
			ent->delay = StatusIdle;
			ent->enemy = NULL;
			return;
		}
	}
	ent->s.frame++;
	return;
}

//Add or subtract ammo from biosentry
void UpdateBiosentryAmmo(edict_t *ent, int change_amt)
{
	ent->light_level += change_amt;
	if (ent->light_level == 25)
	{
		safe_cprintf(ent->creator, PRINT_HIGH, "Warning: Your biosentry is low on ammo.\n"); 
	}

}

void FireBiosentry(edict_t *ent)
{
	vec3_t forward, right, start, target, dir;
	vec3_t temp_vel;

	temp_vel[0] = 0;	
	temp_vel[1] = 0;	
	temp_vel[2] = 0;	
	
	if(ent->light_level<=0)
	{
		Biosentry_self_remove(ent);
		return;
	}

	UpdateBiosentryAmmo(ent, -1);

	AngleVectors (ent->s.angles, forward, right, NULL);

	start[0] = ent->s.origin[0] + forward[0] * 3 ;
	start[1] = ent->s.origin[1] + forward[1] * 3 ;
	start[2] = ent->s.origin[2] + forward[2] * 3+4;

	// calc direction to where we targetd
//	VectorMA (ent->enemy->s.origin, -0.05, ent->enemy->velocity, target);
	VectorMA (ent->enemy->s.origin, -0.05, temp_vel, target);
	
	//Adjust for height
//	target[2] += ent->enemy->viewheight/1.5;
	
	VectorSubtract (target, start, dir);
	VectorNormalize (dir);
	
	//fire dart
	fire_infecteddart (ent, start, dir, 20, 500, EF_GIB, MOD_BIOSENTRY);

	// send muzzle flash
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_SHOTGUN );
	gi.multicast (start, MULTICAST_PVS);
//	gi.sound(ent, CHAN_VOICE, gi.soundindex("boss3/xfire.wav"), 1, ATTN_NORM, 0);
}

void Biosentry_Think(edict_t *self)
{
	static char Bio[32];
	static char Armor[32];
	static char Ammo[32];
	static char Level[32];
	int	contents;
	edict_t *blip;
	float dist;
	vec3_t v;
	trace_t		tr;
//	gitem_t	*ammo;
//	int max;
	float checkyaw;
	blip = NULL;

	//gi.error("john");

	//If there is no client attached to this Bio any more, it should be removed
	//from the game
	if (!self->creator)				//There is no creator
	{
		Biosentry_self_remove(self);
	}

	if (!self->creator->client)		//Creator isn't a client
	{
		Biosentry_self_remove(self);
	}

	//Make sure that the creator of this biosentry also has an
	//entry that points back to this Bio
	if (!self->creator->sentry)	//creator doesn't know about this Bio
	{
		Biosentry_self_remove(self);
	}

	if (self->creator->sentry != self)	//creator has a different Bio
	{
		Biosentry_self_remove(self);
	}

	contents = (int)gi.pointcontents(self->s.origin);

	if (contents & CONTENTS_SOLID)
	{
		safe_cprintf(self->creator, PRINT_HIGH, "Your biosentry was in a bad map position, so it was removed.\n");//5/99
		Biosentry_self_remove(self);
		return;
	}


	if(self->enemy)
	{
		if(self->enemy->health<=0)
		{
			self->enemy= NULL;
		}
	}

	//Play sound if we are ready
//	--self->turretsoundcountdown;

	if(!self->enemy)
		{
			self->delay=StatusIdle;
			//Reduce range 
			while (blip = findradius (blip, self->s.origin, 500))
			{
				if (!blip->client)
					continue;	//not a player
 				if (blip->wf_team != self->wf_team)	//shoot only at team mates
					continue;
				if (blip->solid == SOLID_NOT)
					continue;	//don't see observers
				if (blip->disguised)
					continue;
				if (blip->health <= 0 || blip->health >= 100)
					continue;
				tr = gi.trace (self->s.origin, NULL, NULL, blip->s.origin, self, MASK_SOLID);
				if (tr.fraction != 1.0)
					continue;
				VectorSubtract (self->s.origin, blip->s.origin, v);
			
				dist = VectorLength(v);

				if (!visible(self, blip) && dist > 300)
					continue;
			
				self->enemy = blip;
				self->delay = StatusAttack;
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
						if (self->s.frame == AttackFire)
						{
							FireBiosentry(self);
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

	BiosentryAnimate(self);
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


//Ent = Bio entity
void Biosentry_self_remove(edict_t *ent)
{

	//Clear client's pointer to Biosentry
	if (ent->creator)
		ent->creator->sentry = NULL;


	//Then free the biosentry
	BecomeExplosion1 (ent);
//	G_FreeEdict(ent);
}

//Ent = Player  entity
void Biosentry_remove(edict_t *ent)
{
	if (ent->sentry) 
	{  //42 ebc
		safe_cprintf(ent, PRINT_HIGH, "biosentry off.\n"); 


		//free the Biosentry
		BecomeExplosion1 (ent->sentry);
//		G_FreeEdict(ent->sentry);    

		ent->sentry = NULL;

		if (ent->client->oldplayer)
			G_FreeEdict(ent->client->oldplayer);
		return; 
	}
}

void Biosentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	edict_t *blip = NULL;
	int i;
	vec3_t origin;
	
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

//42 scbc
	safe_cprintf(self->creator, PRINT_HIGH, "biosentry Destroyed.\n"); 

	//Give a frag to the attacker
	if (attacker->client && attacker->wf_team != self->wf_team)
	{
		attacker->client->resp.score++;
	}

	VectorCopy (self->s.origin,origin);
	origin[2]+= 0.5;
	self->takedamage = DAMAGE_NO;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (origin);
	gi.multicast (origin, MULTICAST_PVS);

	Biosentry_self_remove(self);

}

void BiosentryReload(edict_t *ent, pmenu_t *p)
{	
	int max, armorfill;
	float dist;
	vec3_t distance;
	if (!ent->selectedsentry)
	{
		safe_cprintf(ent, PRINT_HIGH, "No bio-sentry selected\n");
	}
	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Biosentry too far away.\n"); 
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
		//42 ebc
		if (!ent->bot_client && ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 25)
		{
 			safe_cprintf(ent, PRINT_HIGH, "You need 25 Bullets to reload the biosentry.\n");
			if (ent->selectedsentry) gi.sound(ent->selectedsentry, CHAN_VOICE, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);
			PMenu_Close(ent);
			return;
		}

		//25 player bullets will give 75 bullets to biosentry (Gregg)
//		if(armorfill > ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))])
//			armorfill = ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];

		UpdateBiosentryAmmo(ent->selectedsentry, armorfill);
//		ent->selectedsentry->light_level += armorfill;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] -= 25;
		if (ent->selectedsentry->light_level > max)
			ent->selectedsentry->light_level = max;
		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
	PMenu_Close(ent);
}

void AddArmorToBiosentry(edict_t *ent, int amt)
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
		gi.sound(ent->selectedsentry, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1, ATTN_NORM, 0);
	}
}

void BiosentryRepair(edict_t *ent, pmenu_t *p)
{	
	int max, armor;
	int cells, maxcells, currcells;
	float dist;
	vec3_t distance;
	if (!ent->selectedsentry)
	{//42 ebc
		safe_cprintf(ent, PRINT_HIGH, "No bio-sentry selected\n");
	}
	distance[0]=ent->s.origin[0] - ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1] - ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2] - ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Biosentry too far away.\n"); 
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
		{//42 ebc
 			safe_cprintf(ent, PRINT_HIGH, "You need Cells to repair the biosentry.\n");
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


void Biosentry_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{ 
	if (other->bot_client)
        return;//42

	if (other->client) other->selectedsentry = ent;

	//Skip if they already have a menu up
	if ((other->client) && (other->client->menu)) 
		return;

	if(other->sentrydelay>level.time)
		return;

	//Only show menu a max of every 1 second

	//Only allow team mates access to menu
//if(!other->bot_client)
	if (other->client && (other->wf_team == ent->wf_team) && (other->client->player_special & SPECIAL_BIOSENTRY))
	{
		PMenu_Close(other);

		sprintf(other->client->wfsentrystr[0], "*%s's Bio", ent->creator->client->pers.netname);
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
		other->client->sentrymenu[5].text = "1. Repair";
		other->client->sentrymenu[5].SelectFunc = BiosentryRepair;
		other->client->sentrymenu[5].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[5].arg = 0;
		other->client->sentrymenu[6].text = "2. Reload";
		other->client->sentrymenu[6].SelectFunc = BiosentryReload;
		other->client->sentrymenu[6].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[6].arg = 0;

//		other->selectedsentry = ent->creator->sentry;
		other->selectedsentry = ent;

		PMenu_Open(other, other->client->sentrymenu, -1, sizeof(other->client->sentrymenu) / sizeof(pmenu_t), true, false);
		other->sentrydelay = level.time + 3;

		//Set timeout for menu (4 seconds)
		if (other->client->menu) other->client->menu->MenuTimeout = level.time + 4;
	}
}


//Build the Biosentry
void PlaceBiosentry (edict_t *ent)
{
	vec3_t		forward,up,right,wallp, pos,try1,try2,try3,try4;

	trace_t		tr;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	/**** DEBUGGING ***/
	//safe_cprintf(ent, PRINT_HIGH, "Sorry - biosentry disabled while we are testing.\n"); 
	//return;
	/**** DEBUGGING ***/

	if (ent->sentry) 
	{  
		Biosentry_remove(ent);

		if (ent->client->oldplayer)
			G_FreeEdict(ent->client->oldplayer);
		return; 
	}

	// cells for biosentry ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 50)
	{//42 ebc
		safe_cprintf(ent, PRINT_HIGH, "You need 50 cells to create biosentry.\n");
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
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;
        ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 50;


	safe_cprintf(ent, PRINT_HIGH, "biosentry on.\n");
	ent->sentry = G_Spawn();

//	ent->sentry->turretsoundcountdown = 0;

	VectorClear (ent->sentry->mins);
	VectorClear (ent->sentry->maxs);
	VectorCopy (pos, ent->sentry->s.origin);
	ent->sentry ->s.angles[0]=ent->s.angles[0];
	ent->sentry -> movetype		= MOVETYPE_STEP;
	ent->sentry -> clipmask		= MASK_PLAYERSOLID;
	ent->sentry->mass = 400;
	ent->s.renderfx=RF_FRAMELERP|RF_TRANSLUCENT|RF_GLOW;
	//grenade -> solid		= SOLID_NOT;
	ent->sentry->solid = SOLID_BBOX;

	VectorSet(ent->sentry->mins, -45,-15,-35);
	VectorSet(ent->sentry->maxs, 25, 18, 18);
	
	ent->sentry->takedamage=DAMAGE_YES;
	ent->sentry -> s.modelindex	= gi.modelindex ("models/biosentry/tris.md2");
	ent->sentry -> creator = ent;	
	ent->sentry ->sentrydelay =level.time+1;
	ent->sentry->creator=ent;
	ent->sentry->think = Biosentry_Think;
	ent->sentry->nextthink = level.time + 0.1;
//	ent->sentry->touch = Biosentry_Touch;
	ent->sentry->die = Biosentry_die;
	ent->sentry->health= 80;
	ent->sentry->max_health = 100;
	ent->sentry->count = 1;
	ent->sentry->s.sound = gi.soundindex ("weapons/biosentry/bioidle.wav");
	ent->sentry->attenuation =1;
	ent->sentry->volume = 0.5;
	ent->sentry->classname = "biosentry";
	ent->sentry->wf_team = ent->wf_team;
	ent->sentry->noteamdamage = true;	//Don't let teammates damage it
	ent->sentry->yaw_speed = 35;
	ent->sentry->gib_health = 75;//Max Ammo
	ent->sentry->light_level = 75;//Ammo Total
		
	ent->sentry->delay=StatusIdle;
	gi.linkentity (ent->sentry);
}


//test_Bio() - see if there is a Bio close enough to work on

//Returns 0 if Bio does not exist or is too far away
//Returns 1 if Bio exists and is close enough
int test_Biosentry (edict_t *ent)
{
	float dist;
	float tdist;
	edict_t *Bio;
	vec3_t distance;
	edict_t *blip;
	if (!ent->sentry)
	{
		safe_cprintf(ent, PRINT_HIGH, "Biosentry not found.\n"); 
		return 0;
	}

	blip = NULL;
	dist = 0;
	Bio = NULL;
	while (blip = findradius (blip, ent->s.origin, 100))
	{
		if (strcmp(blip->classname, "biosentry"))
			continue;	//Not a biosentry

		distance[0]=ent->s.origin[0] - blip->s.origin[0];
		distance[1]=ent->s.origin[1] - blip->s.origin[1];
		distance[2]=ent->s.origin[2] - blip->s.origin[2];
		tdist=VectorLength(distance);
		if ((dist == 0) || (tdist < dist))
		{
			dist = tdist;
			Bio = blip;
		}
	}
	if(dist>100 || Bio == NULL)
	{
		safe_cprintf(ent, PRINT_HIGH, "Bio too far away.\n"); 
		return 0;
	}
	else 
	{
		ent->selectedsentry = Bio;
		return 1;
	}

}

void cmd_Biosentry (edict_t *ent)
{
	char    *string;

	string=gi.args();

	if (!ent->client) return;

	//argument = "build", "remove", "repair" and "reload"
	if (Q_stricmp ( string, "build") == 0)
	{
		if (!ent->sentry)
		PlaceBiosentry(ent);
		else
			safe_cprintf(ent, PRINT_HIGH, "Bio already exists.\n"); 
	}
	else if (Q_stricmp ( string, "remove") == 0)
	{
		if (ent->sentry) 
			Biosentry_remove(ent);
		else
			safe_cprintf(ent, PRINT_HIGH, "Bio not found.\n"); 
	}
	else if (Q_stricmp ( string, "repair") == 0)
	{
		if (test_Biosentry(ent)) BiosentryRepair(ent, NULL);
	}
	else if (Q_stricmp ( string, "reload") == 0)
	{
		if (test_Biosentry(ent)) BiosentryReload(ent, NULL);
	}

	//Otherwise toggle on/off
	else if (Q_stricmp ( string, "") == 0)
	{
		PlaceBiosentry(ent);
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "Invalid Bio command.\n"); 

}
