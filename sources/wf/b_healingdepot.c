#include "g_local.h"

#define newHealingDepot self->supply
void SP_HealingDepot(edict_t *self);
void Remove_Player_Flames (edict_t *ent);
void healingdepot_explode (edict_t *self);
/*
=================
Healing Depot
=================
*/

void HealPlayer(edict_t *ent)
{
	ent->disease= 0;
	ent->lame = 0;
	ent->superslow=0;
	ent->Slower=0;
	ent->DrunkTime=level.time - 1;
	ent->client->blindBase = 0;
	ent->client->blindTime = 0;	
	ent->cantmove = 0;

	Remove_Player_Flames (ent);

}

void HealingDepotThink (edict_t *self)
{
	edict_t *other;
	int contents;
	other = NULL;

	contents = (int)gi.pointcontents(self->s.origin);

	if (contents & CONTENTS_SOLID)
	{
		safe_cprintf(self->owner, PRINT_HIGH, "Your healing depot was in a bad map position, so it was removed.\n");//5/99
		healingdepot_explode (self);
		return;
	}

	while ((other = findradius(other, self->s.origin, 16)) != NULL)
	{
		if (other->client)
		{
			HealPlayer(other);
		
			//Give some health
			if (other->health < 100) 
			{
				other->health += 5;
				if (other->health > 100) other->health = 100;
			}

    //destroy depot for bots if enemy is using it
	if ((self->wf_team != other->wf_team) && (self->owner->bot_client))
	{	healingdepot_explode (self);
		return;
    }
			if ((self->wf_team != other->wf_team) && (self->owner))
				safe_cprintf(self->owner, PRINT_HIGH, "Enemies are using your healing depot!\n"); 
		gi.sound(self, CHAN_ITEM, gi.soundindex("ctf/tech4.wav"), 1, ATTN_NORM, 0);
		}
	}
	
	self->nextthink = level.time + 0.35;
}

void healingdepot_explode (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	make_debris(self);

	self->takedamage = DAMAGE_NO;
		
	if (self->owner) safe_cprintf(self->owner, PRINT_HIGH, "Healing Depot Off.\n"); 

	if ((self->owner) && (self->owner->supply)) 
		self->owner->supply = NULL;

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_HEALINGDEPOT);
	G_FreeEdict(self);

}

void healingdepot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t *blip = NULL;
	int i;
	
//42 bot supply clear itemnode owner
if (self->owner->bot_client)
	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == self->owner)
	  {
		 if (!strcmp(blip->classname, "item_depotspot") )
		{
			blip->owner = NULL;
		}
	  }
	}
//42 end clear item owner

	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	make_debris(self);

	self->takedamage = DAMAGE_NO;
		
	if (self->owner) safe_cprintf(self->owner, PRINT_HIGH, "Healing Depot Destroyed.\n"); 

	if ((self->owner) && (self->owner->supply)) 
		self->owner->supply = NULL;

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_HEALINGDEPOT);
	G_FreeEdict(self);

}

void SP_HealingDepot(edict_t *self)
{
	/**** DEBUGGING ***/
	//safe_cprintf(self, PRINT_HIGH, "Sorry - healing depot disabled while we are testing.\n"); 
	//return;
	/**** DEBUGGING ***/

	if ( newHealingDepot ) 
	{  
		healingdepot_explode(newHealingDepot);
		return; 
	}

	// cells for laser ?
	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 10)
	{
 		safe_cprintf(self, PRINT_HIGH, "Need 10 cells for a Healing Depot.\n");
		return;
	}
	safe_cprintf(self, PRINT_HIGH, "Healing Depot on.\n");
	newHealingDepot = G_Spawn ();
	VectorCopy(self->s.origin,newHealingDepot->s.origin);
	newHealingDepot->s.origin[2] += 40;
	newHealingDepot->classname="healingdepot";
	newHealingDepot->takedamage=DAMAGE_AIM;
	newHealingDepot->movetype= MOVETYPE_TOSS;
	newHealingDepot->mass = 200;
	newHealingDepot->solid = SOLID_BBOX;
	newHealingDepot->clipmask=MASK_ALL;
	newHealingDepot->deadflag =DEAD_NO;
	newHealingDepot->clipmask = MASK_SHOT;
	newHealingDepot->model = self->model;
	newHealingDepot->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2");
	newHealingDepot->s.skinnum = 2;
	newHealingDepot->solid = SOLID_BBOX;
	newHealingDepot->noteamdamage = true;	//Don't let teammates damage it
	newHealingDepot->wf_team = self->wf_team;

	VectorSet (newHealingDepot->mins, -32, -32, -24);
	VectorSet (newHealingDepot->maxs, 32, 32, -16);
	newHealingDepot->s.frame =0;
	newHealingDepot->waterlevel = 0;
	newHealingDepot->watertype=0;
	newHealingDepot->health= 100;
	newHealingDepot->max_health =100;
	newHealingDepot->gib_health = -80;
	newHealingDepot->die = healingdepot_die;
	newHealingDepot->owner = self;
	newHealingDepot->dmg = 150;
	newHealingDepot->dmg_radius = 160;
//	newHealingDepot->touch = HealingTouch;
	newHealingDepot->think =HealingDepotThink;
	newHealingDepot->nextthink = level.time + 1;
	VectorClear (newHealingDepot->velocity);
	gi.linkentity (newHealingDepot);

	self->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]-= 10;
			
	if (self->client && !self->bot_client ) gi.centerprintf (self,"Healing Depot set!\n");
}
